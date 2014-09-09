#	include "Amplifier.h"

#   include "Interface/ResourceInterface.h"
#   include "Interface/StringizeInterface.h"

#	include "Playlist.h"

#	include "ResourcePlaylist.h"

#   include "Logger/Logger.h"

#	include <s3eAudio.h>

#	include <cmath>

//////////////////////////////////////////////////////////////////////////
SERVICE_FACTORY(Amplifier, Menge::AmplifierInterface, Menge::Amplifier);
//////////////////////////////////////////////////////////////////////////
namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	Amplifier::Amplifier()
		: m_serviceProvider(nullptr)
		, m_currentPlayList(nullptr)
		, m_volume(1.f)
		, m_volumeOverride(1.f)
		, m_play(false)
        , m_turn(true)
		, m_currentSoundPosition(0.f)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	Amplifier::~Amplifier()
	{
		//_release();
		this->stop();

		for( TMapPlayList::iterator 
			it = m_mapPlayLists.begin(),
			it_end = m_mapPlayLists.end();
		it != it_end;
		++it)
		{
			Playlist * playlist = it->second;
			
			delete playlist;
		}
	}
    //////////////////////////////////////////////////////////////////////////
    void Amplifier::setServiceProvider( ServiceProviderInterface * _serviceProvider )
    {
        m_serviceProvider = _serviceProvider;
    }
    //////////////////////////////////////////////////////////////////////////
    ServiceProviderInterface * Amplifier::getServiceProvider() const
    {
        return m_serviceProvider;
    }
	//////////////////////////////////////////////////////////////////////////
	static int32 s_Amplifier_AudioCallback_Stop( s3eAudioCallbackData * _data, Amplifier * _amplifier )
	{
		(void)_data;

		printf("s_Amplifier_AudioCallback_Stop %d\n"
			, _data->m_ChannelID
			);

		_amplifier->onSoundStop();

		return 0;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Amplifier::initialize()
	{
		if( s3eAudioGetInt( S3E_AUDIO_AVAILABLE ) == 0 )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::initialize: not support"
				);

			return false;
		}

		s3eResult result = s3eAudioRegister( S3E_AUDIO_STOP, (s3eCallback)&s_Amplifier_AudioCallback_Stop, this );

		if( result != S3E_RESULT_SUCCESS )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::initialize: invalid register callback %d"
				, result
				);

			return false;
		}

		SOUND_SERVICE(m_serviceProvider)
			->addSoundVolumeProvider( this );

#	define AMPLIFIER_SUPPORT_CODEC( Codec )\
	LOGGER_WARNING(m_serviceProvider)("Amplifier::initialize: " #Codec " %s"\
		, s3eAudioIsCodecSupported( Codec ) ? "support" : "unsupport!" )

		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_MIDI );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_MP3 );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_AAC );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_AACPLUS );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_QCP );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_PCM );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_SPF );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_AMR );
		AMPLIFIER_SUPPORT_CODEC( S3E_AUDIO_CODEC_MP4 );

#	undef AMPLIFIER_SUPPORT_CODEC
		
		return true;			
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::finalize()
	{
		SOUND_SERVICE(m_serviceProvider)
			->removeSoundVolumeProvider( this );

		s3eAudioUnRegister( S3E_AUDIO_STOP, (s3eCallback)&s_Amplifier_AudioCallback_Stop );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Amplifier::loadPlayList_( const ConstString& _playlistResource )
	{
		TMapPlayList::iterator it = m_mapPlayLists.find( _playlistResource );

		if( it == m_mapPlayLists.end() )
		{			
			Playlist * playlist = new Playlist( m_serviceProvider );

            if( playlist->initialize( _playlistResource ) == false )
            {
                LOGGER_ERROR(m_serviceProvider)("Amplifier: no found playlist with name '%s'"
                    , _playlistResource.c_str()
                    );

                delete playlist;

                return false;
            }

			it = m_mapPlayLists.insert( std::make_pair( _playlistResource, playlist ) ).first;
		}

		m_currentPlaylistName = _playlistResource;
		m_currentPlayList = it->second;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Amplifier::playTrack( const ConstString& _playlistResource, size_t _index, float _pos, bool _looped )
	{
		if( this->loadPlayList_( _playlistResource ) == false )
		{
			return false;
		}

		m_currentPlayList->setLooped1( _looped );
		m_currentPlayList->setTrack( _index );

		if( m_play == true )
		{
			return true;
		}

		if( this->preparePlay_( _pos ) == false )
		{
			return false;
		}

		m_play = true;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	size_t Amplifier::getNumTracks() const
	{
		if( m_currentPlayList == nullptr )
		{
			return 0;
		}

        size_t numTracks = m_currentPlayList->numTracks();

		return numTracks;
	}
	//////////////////////////////////////////////////////////////////////////
	size_t Amplifier::getCurrentTrack() const
	{
		if( m_currentPlayList == nullptr )
		{
			return 0;
		}

		size_t index = m_currentPlayList->currentTrackIndex();

		return index;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Amplifier::preparePlay_( float _pos )
	{
		if( m_currentPlayList == nullptr )
		{
			return false;
		}

		const TrackDesc * track = m_currentPlayList->getTrack();

		if( track == nullptr )
		{
			return false;
		}

		const ConstString & category = m_currentPlayList->getCategory();
		if( this->play_( category, track->path, track->codec, _pos ) == false )
		{
			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::shuffle( const ConstString & _playlist )
	{
		if( this->loadPlayList_( _playlist ) == false )
		{
			return;
		}

		m_currentPlayList->shuffle();
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::stop()
	{
		m_play = false;

		s3eAudioStop();				
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::pause()
	{
		m_play = false;

		s3eResult result = s3eAudioPause();

		if( result != S3E_RESULT_SUCCESS )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::pause invalid %d"
				, result
				);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::resume()
	{
        m_play = true;

		s3eResult result = s3eAudioResume();

		if( result != S3E_RESULT_SUCCESS )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::resume invalid %d"
				, result
				);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::onSoundStop()
	{
		if( m_play == false )
		{
			return;
		}

        m_currentPlayList->next();

		const TrackDesc * track = m_currentPlayList->getTrack();
		
        if( track != nullptr )
        {
            const ConstString & category = m_currentPlayList->getCategory();
            
			if( this->play_( category, track->path, track->codec, 0.f ) == false )
			{
				return;
			}
        }
	}
	//////////////////////////////////////////////////////////////////////////
	bool Amplifier::play_( const ConstString& _pakName, const FilePath& _filePath, const ConstString& _codec, float _pos )
	{
		printf("Amplifier::play_ %s:%s %f\n"
			, _pakName.c_str()
			, _filePath.c_str()
			, _pos
			);

		this->stop();

		int32 s3e_pos = (int32)_pos;

		s3eResult result = s3eAudioSetInt( S3E_AUDIO_POSITION, s3e_pos );

		if( result != S3E_RESULT_SUCCESS )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::play_: can't '%s:%s' set pos %d"
				, _pakName.c_str()
				, _filePath.c_str()
				, s3e_pos
				);

			return false;
		}
				
		MemoryInputPtr memory = FILE_SERVICE(m_serviceProvider)
			->openInputFileInMemory( _pakName, _filePath );

		if( memory == nullptr )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::play_: invalid open sound '%s:%s'"
				, _pakName.c_str()
				, _filePath.c_str()
				);

			return false;
		}

		size_t size;
		const void * buffer = memory->getMemory( size );

		result = s3eAudioPlayFromBuffer( const_cast<void *>(buffer), size, 1 );
				
		if( result != S3E_RESULT_SUCCESS )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::play_: can't play sound '%s:%s'"
				, _pakName.c_str()
				, _filePath.c_str()
				);

			return false;
		}	

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	const ConstString& Amplifier::getPlayTrack() const
	{
		return m_currentPlaylistName;
	}
	//////////////////////////////////////////////////////////////////////////
	float Amplifier::getPosMs() const
	{
		int32 s3e_pos = s3eAudioGetInt( S3E_AUDIO_POSITION );

		float pos = (float)s3e_pos;

		return pos;
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::setPosMs( float _posMs )
	{
		printf("Amplifier::setPosMs %f\n"
			, _posMs
			);

		int32 s3e_pos = (int32)_posMs;
		 
		s3eResult result = s3eAudioSetInt( S3E_AUDIO_POSITION, s3e_pos );

		if( result != S3E_RESULT_SUCCESS )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::setPosMs invalid set S3E_AUDIO_POSITION %d"
				, s3e_pos
				);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Amplifier::onSoundChangeVolume( float _sound, float _music, float _voice )
	{
		printf("Amplifier::onSoundChangeVolume %f %f %f\n"
			, _sound
			, _music
			, _voice
			);

		int32 s3e_volume = (int32)(_music * S3E_AUDIO_MAX_VOLUME);

		s3eResult result = s3eAudioSetInt( S3E_AUDIO_VOLUME, s3e_volume );

		if( result != S3E_RESULT_SUCCESS )
		{
			LOGGER_ERROR(m_serviceProvider)("Amplifier::onSoundChangeVolume invalid set S3E_AUDIO_VOLUME %d"
				, s3e_volume
				);
		}
	}
}