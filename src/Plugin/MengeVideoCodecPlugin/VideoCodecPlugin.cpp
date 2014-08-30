#	include "VideoCodecPlugin.h"

//#   include "VideoDecoderFFMPEG.h"
#   include "VideoDecoderGVF.h"
//#   include "VideoDecoderVPX.h"

#   include "Codec/DecoderFactory.h"

#   include "Interface/StringizeInterface.h"

extern "C" // only required if using g++
{
	//////////////////////////////////////////////////////////////////////////
	bool initPluginMengeVideoCodec( Menge::PluginInterface ** _plugin )
	{
		*_plugin = new Menge::VideoCodecPlugin();

		return true;
	}
	////////////////////////////////////////////////////////////////////////////
	__declspec(dllexport) bool dllCreatePlugin( Menge::PluginInterface ** _plugin )
	{
		return initPluginMengeVideoCodec( _plugin );
	}
}
//////////////////////////////////////////////////////////////////////////
namespace Menge
{
    ////////////////////////////////////////////////////////////////////////////
    //static void s_ffmpeg_log( void* _impl, int _level, const char* _format, va_list _args )
    //{
    //    (void)_impl;

    //    if( _level > AV_LOG_ERROR )
    //    {
    //        return;
    //    }

    //    printf("VideoCodecPlugin::s_ffmpeg_log %d\n"
    //        , _level
    //        );

    //    vprintf( _format, _args );
    //}
	//////////////////////////////////////////////////////////////////////////
	VideoCodecPlugin::VideoCodecPlugin()
		: m_serviceProvider(nullptr)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	bool VideoCodecPlugin::initialize( ServiceProviderInterface * _serviceProvider )
	{
		m_serviceProvider = _serviceProvider;

        //avcodec_register_all();
        //av_register_all();

        //av_log_set_callback( &s_ffmpeg_log );

		//m_decoders.push_back( new DecoderFactory<VideoDecoderFFMPEG>(m_serviceProvider, CONST_STRING_LOCAL(m_serviceProvider, "ffmpegVideo")) );
        m_decoders.push_back( new DecoderFactory<VideoDecoderGVF>(m_serviceProvider, Helper::stringizeString(m_serviceProvider, "gvfVideo")) );
        //m_decoders.push_back( new DecoderFactory<VideoDecoderVPX>(m_serviceProvider, Helper::stringizeString(m_serviceProvider, "vpxVideo")) );
		
		for( TVectorVideoDecoders::iterator
			it = m_decoders.begin(),
			it_end = m_decoders.end();
		it != it_end;
		++it )
		{			
			const ConstString & name = (*it)->getName();

			CODEC_SERVICE(m_serviceProvider)
				->registerDecoder( name, (*it) );
		}
        
        return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void VideoCodecPlugin::finalize()
	{
		for( TVectorVideoDecoders::iterator
			it = m_decoders.begin(),
			it_end = m_decoders.end();
		it != it_end;
		++it )
		{
			const ConstString & name = (*it)->getName();

			CODEC_SERVICE(m_serviceProvider)
				->unregisterDecoder( name );
		}

		m_decoders.clear();
	}
	//////////////////////////////////////////////////////////////////////////
	void VideoCodecPlugin::destroy()
	{
		delete this;
	}
}