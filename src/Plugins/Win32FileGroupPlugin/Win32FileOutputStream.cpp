#include "Win32FileOutputStream.h"

#include "Interface/UnicodeSystemInterface.h"

#include "Win32FileHelper.h"

#include "Kernel/Logger.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    Win32FileOutputStream::Win32FileOutputStream()
        : m_hFile( INVALID_HANDLE_VALUE )
        , m_size( 0 )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    Win32FileOutputStream::~Win32FileOutputStream()
    {
        if( m_hFile != INVALID_HANDLE_VALUE )
        {
            ::CloseHandle( m_hFile );
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool Win32FileOutputStream::open( const FilePath & _relationPath, const FilePath & _folderPath, const FilePath& _filePath )
    {
#ifdef MENGINE_DEBUG
        m_relationPath = _relationPath.c_str();
        m_folder = _folderPath.c_str();
        m_fileName = _filePath.c_str();
#endif

        WChar fullPath[MENGINE_MAX_PATH];
        if( Helper::Win32ConcatenateFilePathW( _relationPath, _folderPath, _filePath, fullPath, MENGINE_MAX_PATH ) == false )
        {
            LOGGER_ERROR( "Win32OutputStream::open invlalid concatenate filePath '%s':'%s'"
                , _folderPath.c_str()
                , _filePath.c_str()
            );

            return false;
        }

        m_hFile = Helper::Win32CreateFile(
            fullPath
            , GENERIC_WRITE
            , FILE_SHARE_READ | FILE_SHARE_WRITE
            , CREATE_ALWAYS
        );

        if( m_hFile == INVALID_HANDLE_VALUE )
        {
            LOGGER_ERROR( "Win32OutputStream::open %ls invalid open"
                , fullPath
            );

            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    size_t Win32FileOutputStream::write( const void * _data, size_t _size )
    {
        DWORD bytesWritten = 0;
        BOOL result = ::WriteFile( m_hFile, _data, (DWORD)_size, &bytesWritten, NULL );

        if( result == FALSE )
        {
            LOGGER_ERROR( "Win32OutputStream::write invalid %d"
                , _size
            );

            return 0;
        }

        m_size += bytesWritten;

        return (size_t)bytesWritten;
    }
    //////////////////////////////////////////////////////////////////////////
    size_t Win32FileOutputStream::size() const
    {
        return m_size;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Win32FileOutputStream::flush()
    {
        if( ::FlushFileBuffers( m_hFile ) == FALSE )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
}
