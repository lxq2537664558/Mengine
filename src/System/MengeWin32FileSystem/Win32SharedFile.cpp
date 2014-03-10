#	include "Win32SharedFile.h"

#	include "Interface/LogSystemInterface.h"
#	include "Interface/UnicodeInterface.h"

#   include "Logger/Logger.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	Win32SharedFile::Win32SharedFile()
		: m_serviceProvider(nullptr)
        , m_filenamelen(0)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	Win32SharedFile::~Win32SharedFile()
	{        
	}
    //////////////////////////////////////////////////////////////////////////
    void Win32SharedFile::setServiceProvider( ServiceProviderInterface * _serviceProvider )
    {
        m_serviceProvider = _serviceProvider;
    }
	//////////////////////////////////////////////////////////////////////////
	bool Win32SharedFile::initialize( const FilePath & _folder, const FilePath & _dir, const char * _filename, size_t _filenamelen )
	{
		m_folder = _folder;
		m_dir = _dir;

		memcpy( m_filename, _filename, _filenamelen );
		m_filenamelen = _filenamelen;
				
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	InputStreamInterfacePtr Win32SharedFile::createFileStream()
	{
		Win32InputStream * stream = m_factoryWin32InputStream.createObjectT();

		stream->setServiceProvider( m_serviceProvider );

		return stream;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Win32SharedFile::openFileStream( const InputStreamInterfacePtr & _stream, size_t _offset, size_t _size )
	{
		Win32InputStream * stream = stdex::intrusive_get<Win32InputStream>(_stream);

		if( stream->openRange( m_folder, m_dir, m_filename, m_filenamelen, _offset, _size ) == false )
		{
			LOGGER_ERROR(m_serviceProvider)("Win32SharedFile::openFileStream invalid share file"
				);

			return false;
		}

		return true;
	}
}	// namespace Menge