#	include "XmlToBinDecoder.h"

#   include "Interface/StringizeInterface.h"
#   include "Interface/ArchiveInterface.h"

#	include "Utils/Logger/Logger.h"

#	include "metabuf/Metabuf.hpp"

#	include "xml2metabuf/Xml2Metabuf.hpp"
#	include "xml2metabuf/Xml2Metacode.hpp"

#   include <stdio.h>

namespace Menge
{
    //////////////////////////////////////////////////////////////////////////
    static bool s_write_wstring( Metabuf::Xml2Metabuf * _metabuf, const char * _value, void * _user )
    {
        (void)_user;

        size_t utf8_size = strlen( _value ); 

        _metabuf->write( utf8_size );
        
        _metabuf->writeCount( &_value[0], utf8_size );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    static bool s_write_wchar_t( Metabuf::Xml2Metabuf * _metabuf, const char * _value, void * _user )
    {
        (void)_user;

        size_t utf8_size = strlen( _value ); 

        _metabuf->write( utf8_size );

        _metabuf->writeCount( &_value[0], utf8_size );

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	XmlToBinDecoder::XmlToBinDecoder()
        : m_serviceProvider(NULL)
        , m_stream(NULL)
	{
	}
    //////////////////////////////////////////////////////////////////////////
    ServiceProviderInterface * XmlToBinDecoder::getServiceProvider() const
    {
        return m_serviceProvider;
    }
    //////////////////////////////////////////////////////////////////////////
    InputStreamInterfacePtr XmlToBinDecoder::getStream() const
    {
        return m_stream;
    }
	//////////////////////////////////////////////////////////////////////////
	bool XmlToBinDecoder::setOptions( CodecOptions * _options )
	{
		m_options = *static_cast<XmlCodecOptions *>(_options);

        return true;
	}
	//////////////////////////////////////////////////////////////////////////
	const XmlCodecDataInfo* XmlToBinDecoder::getCodecDataInfo() const
	{
		return 0;
	}
	//////////////////////////////////////////////////////////////////////////
	bool XmlToBinDecoder::initialize( ServiceProviderInterface * _serviceProvider, const InputStreamInterfacePtr & _stream )
	{
        m_serviceProvider = _serviceProvider;
        m_stream = _stream;

		return true;
	}
	////////////////////////////////////////////////////////////////////////////
	unsigned int XmlToBinDecoder::decode( unsigned char* _buffer, unsigned int _bufferSize )
	{
        (void)_buffer;
        (void)_bufferSize;

        ConstString dev = Helper::stringizeStringSize( m_serviceProvider, "dev", 3 );

        InputStreamInterfacePtr protocol_stream = FILE_SERVICE(m_serviceProvider)
            ->openInputFile( dev, m_options.pathProtocol );

		//FILE * file_protocol = _wfopen( unicode_pathProtocol.c_str(), L"rb" );

        if( protocol_stream == NULL )
        {
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error open protocol %s"
                , m_options.pathProtocol.c_str()
                );

            return 0;
        }

		size_t protocol_size = protocol_stream->size();

		TBlobject protocol_buf(protocol_size);
        protocol_stream->read( &protocol_buf[0], protocol_size );
        protocol_stream = nullptr;

        Metabuf::XmlProtocol xml_protocol;

		if( xml_protocol.readProtocol( &protocol_buf[0], protocol_size ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error read protocol %s error:\n%s"
                , m_options.pathProtocol.c_str()
                , xml_protocol.getError().c_str()
                );

            return 0;
        }
        	
        InputStreamInterfacePtr xml_stream = FILE_SERVICE(m_serviceProvider)
            ->openInputFile( dev, m_options.pathXml );

        if( xml_stream == NULL )
        {
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error open xml %s"
                , m_options.pathXml.c_str()
                );

            return 0;
        }

        size_t xml_size = xml_stream->size();

        if( xml_size == 0 )
        {
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error open xml %s (file size == 0)"
                , m_options.pathXml.c_str()
                );

            return 0;
        }

        TBlobject xml_buf(xml_size);
        xml_stream->read( &xml_buf[0], xml_size );
        xml_stream = nullptr;

        OutputStreamInterfacePtr bin_stream = FILE_SERVICE(m_serviceProvider)
            ->openOutputFile( dev, m_options.pathBin );

        if( bin_stream == NULL )
        {
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error create bin %s"
                , m_options.pathBin.c_str()
                );

            return 0;
        }

		Metabuf::Xml2Metabuf xml_metabuf(&xml_protocol);

        LOGGER_INFO(m_serviceProvider)( "Xml2BinDecoder::decode:\nxml %s\nbin %s"
            , m_options.pathXml.c_str()
            , m_options.pathBin.c_str()
            );

        xml_metabuf.initialize();

        xml_metabuf.addSerializator( "wstring", &s_write_wstring, (void*)m_serviceProvider );
        xml_metabuf.addSerializator( "wchar_t", &s_write_wchar_t, (void*)m_serviceProvider );
		
        TBlobject header_buf(Metabuf::header_size);

        size_t header_size;
        if( xml_metabuf.header( &header_buf[0], 16, header_size ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error header %s error:\n%s"
                , m_options.pathXml.c_str()
                , xml_metabuf.getError().c_str()
                );

            return 0;
        }

        bin_stream->write( &header_buf[0], Metabuf::header_size );

        TBlobject bin_buf(xml_size * 2);

        size_t bin_size;
		if( xml_metabuf.convert( &bin_buf[0], xml_size * 2, &xml_buf[0], xml_size, bin_size ) == false )
		{
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error convert %s error:\n%s"
                , m_options.pathXml.c_str()
                , xml_metabuf.getError().c_str()
                );

			return 0;
		}

        size_t bound_size = ARCHIVE_SERVICE(m_serviceProvider)
            ->compressBound( bin_size );

        TBlobject compress_buf(bound_size);
        
        size_t compress_size;
        if( ARCHIVE_SERVICE(m_serviceProvider)
            ->compress( &compress_buf[0], bound_size, compress_size, &bin_buf[0], bin_size ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "Xml2BinDecoder::decode: error compress bin %s"
                , m_options.pathBin.c_str()
                );

            return 0;
        }
        
        bin_stream->write( &bin_size, sizeof(bin_size) );
        bin_stream->write( &compress_size, sizeof(compress_size) );
        bin_stream->write( &compress_buf[0], compress_size );
		
		return bin_size;
	}
}