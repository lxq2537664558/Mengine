#include "Amplifier.h"

#include "Interface/ResourceServiceInterface.h"
#include "Interface/StringizeServiceInterface.h"
#include "Interface/FileServiceInterface.h"
#include "Interface/SoundServiceInterface.h"

#include "Engine/ResourceMusic.h"

#include "Kernel/Logger.h"
#include "Kernel/Document.h"

#include <cmath>

//////////////////////////////////////////////////////////////////////////
SERVICE_FACTORY( Amplifier, Mengine::Amplifier );
//////////////////////////////////////////////////////////////////////////
namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    Amplifier::Amplifier()
        : m_play( false )
        , m_pause( false )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    Amplifier::~Amplifier()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool Amplifier::_initializeService()
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Amplifier::_finalizeService()
    {
        m_soundEmitter = nullptr;
    }
    //////////////////////////////////////////////////////////////////////////
    class Amplifier::MyMusicSoundListener
        : public Factorable
        , public SoundListenerInterface
    {
    public:
        MyMusicSoundListener( const AmplifierMusicCallbackInterfacePtr & _callback )
            : m_callback( _callback )
        {
        }

        ~MyMusicSoundListener() override
        {
        }

    protected:
        void onSoundPause( const SoundIdentityInterfacePtr & _emitter ) override
        {
            (void)_emitter;

            m_callback->onMusicPause();
        }

        void onSoundResume( const SoundIdentityInterfacePtr & _emitter ) override
        {
            (void)_emitter;

            m_callback->onMusicResume();
        }

        void onSoundStop( const SoundIdentityInterfacePtr & _emitter ) override
        {
            (void)_emitter;

            m_callback->onMusicStop();
        }

        void onSoundEnd( const SoundIdentityInterfacePtr & _emitter ) override
        {
            (void)_emitter;

            m_callback->onMusicEnd();
        }

    protected:
        AmplifierMusicCallbackInterfacePtr m_callback;
    };
    //////////////////////////////////////////////////////////////////////////
    bool Amplifier::playMusic( const ConstString & _resourceMusic, float _pos, bool _looped, const AmplifierMusicCallbackInterfacePtr & _callback )
    {
        if( m_play == true )
        {
            this->stopMusic();
        }

        ResourceMusicPtr resourceMusic = RESOURCE_SERVICE()
            ->getResourceReference( _resourceMusic );

        if( resourceMusic == nullptr )
        {
            LOGGER_ERROR( "can't found resource '%s'"
                , _resourceMusic.c_str()
            );

            return false;
        }

        const FileGroupInterfacePtr & fileGroup = resourceMusic->getFileGroup();
        const FilePath & filePath = resourceMusic->getFilePath();
        const ConstString & codecType = resourceMusic->getCodecType();
        bool external = resourceMusic->isExternal();
        float volume = resourceMusic->getVolume();

        SoundBufferInterfacePtr buffer;

        if( fileGroup->isPacked() == false || external == false )
        {
            buffer = SOUND_SERVICE()
                ->createSoundBufferFromFile( fileGroup, filePath, codecType, true );
        }
        else
        {
            const FileGroupInterfacePtr & defaultFileGroup = FILE_SERVICE()
                ->getDefaultFileGroup();

            buffer = SOUND_SERVICE()
                ->createSoundBufferFromFile( defaultFileGroup, filePath, codecType, true );
        }

        if( buffer == nullptr )
        {
            LOGGER_ERROR( "can't load sample '%s'"
                , filePath.c_str()
            );

            return false;
        }

        SoundIdentityInterfacePtr soundEmitter = SOUND_SERVICE()
            ->createSoundIdentity( false, buffer, ES_SOURCE_CATEGORY_MUSIC, true
                , MENGINE_DOCUMENT( "Amplifier::playMusic resource '%s'", _resourceMusic.c_str() ) );

        if( soundEmitter == nullptr )
        {
            LOGGER_ERROR( "can't create sound source '%s'"
                , filePath.c_str()
            );

            return false;
        }

        if( _callback != nullptr )
        {
            soundEmitter->setSoundListener( Helper::makeFactorableUnique<Amplifier::MyMusicSoundListener>( _callback ) );
        }

        if( SOUND_SERVICE()
            ->setSourceVolume( soundEmitter, volume, 0.f, false ) == false )
        {
            LOGGER_ERROR( "can't set sound '%s' volume '%f'"
                , filePath.c_str()
                , volume
            );

            return false;
        }

        if( SOUND_SERVICE()
            ->setPosMs( soundEmitter, _pos ) == false )
        {
            LOGGER_ERROR( "can't set sound '%s' pos '%f'"
                , filePath.c_str()
                , _pos
            );

            return false;
        }

        if( SOUND_SERVICE()
            ->setLoop( soundEmitter, _looped ) == false )
        {
            LOGGER_ERROR( "can't set sound '%s' lood '%d'"
                , filePath.c_str()
                , _looped
            );

            return false;
        }

        if( SOUND_SERVICE()
            ->playEmitter( soundEmitter ) == false )
        {
            LOGGER_ERROR( "sound emitter '%s' invalid play %d"
                , filePath.c_str()
                , soundEmitter->getId()
            );

            return false;
        }

        m_soundEmitter = soundEmitter;

        m_play = true;
        m_pause = false;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Amplifier::stopMusic()
    {
        m_play = false;
        m_pause = false;

        if( m_soundEmitter != nullptr )
        {
            SoundIdentityInterfacePtr keep_sourceEmitter = m_soundEmitter;
            m_soundEmitter = nullptr;

            SOUND_SERVICE()
                ->stopEmitter( keep_sourceEmitter );

            SOUND_SERVICE()
                ->releaseSoundSource( keep_sourceEmitter );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool Amplifier::pauseMusic()
    {
        if( m_soundEmitter == nullptr )
        {
            return false;
        }

        m_play = true;
        m_pause = true;

        SOUND_SERVICE()
            ->pauseEmitter( m_soundEmitter );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Amplifier::resumeMusic()
    {
        if( m_soundEmitter == nullptr )
        {
            return false;
        }

        m_play = true;
        m_pause = false;

        SOUND_SERVICE()
            ->resumeEmitter( m_soundEmitter );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    float Amplifier::getDuration() const
    {
        if( m_soundEmitter == nullptr )
        {
            return 0.f;
        }

        float pos = SOUND_SERVICE()
            ->getDuration( m_soundEmitter );

        return pos;
    }
    //////////////////////////////////////////////////////////////////////////
    float Amplifier::getPosMs() const
    {
        if( m_soundEmitter == nullptr )
        {
            return 0.f;
        }

        float pos = SOUND_SERVICE()
            ->getPosMs( m_soundEmitter );

        return pos;
    }
    //////////////////////////////////////////////////////////////////////////
    void Amplifier::setPosMs( float _posMs )
    {
        if( m_soundEmitter == nullptr )
        {
            return;
        }

        SOUND_SERVICE()
            ->setPosMs( m_soundEmitter, _posMs );
    }
    //////////////////////////////////////////////////////////////////////////
    void Amplifier::_stopService()
    {
        this->stopMusic();
    }
    //////////////////////////////////////////////////////////////////////////
}
