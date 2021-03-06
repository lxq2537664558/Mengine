#include "ThreadTaskPrefetchStream.h"

#include "Interface/CodecInterface.h"
#include "Interface/StringizeServiceInterface.h"

#include "Kernel/Stream.h"
#include "Kernel/Logger.h"
#include "Kernel/Document.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    ThreadTaskPrefetchStream::ThreadTaskPrefetchStream()
        : m_magicNumber( 0 )
        , m_magicVersion( 0 )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    ThreadTaskPrefetchStream::~ThreadTaskPrefetchStream()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    void ThreadTaskPrefetchStream::setArchivator( const ArchivatorInterfacePtr & _archivator )
    {
        m_archivator = _archivator;
    }
    //////////////////////////////////////////////////////////////////////////
    const ArchivatorInterfacePtr & ThreadTaskPrefetchStream::getArchivator() const
    {
        return m_archivator;
    }
    //////////////////////////////////////////////////////////////////////////
    void ThreadTaskPrefetchStream::setMagicNumber( uint32_t _magicNumber )
    {
        m_magicNumber = _magicNumber;
    }
    //////////////////////////////////////////////////////////////////////////
    uint32_t ThreadTaskPrefetchStream::getMagicNumber() const
    {
        return m_magicNumber;
    }
    //////////////////////////////////////////////////////////////////////////
    void ThreadTaskPrefetchStream::setMagicVersion( uint32_t _magicVersion )
    {
        m_magicVersion = _magicVersion;
    }
    //////////////////////////////////////////////////////////////////////////
    uint32_t ThreadTaskPrefetchStream::getMagicVersion() const
    {
        return m_magicVersion;
    }
    //////////////////////////////////////////////////////////////////////////
    const MemoryInterfacePtr & ThreadTaskPrefetchStream::getMemory() const
    {
        return m_memory;
    }
    //////////////////////////////////////////////////////////////////////////
    bool ThreadTaskPrefetchStream::_onRun()
    {
        if( ThreadTaskPrefetch::_onRun() == false )
        {
            return false;
        }

        m_stream = m_fileGroup->createInputFile( m_filePath, false, MENGINE_DOCUMENT_FUNCTION );

        if( m_stream == nullptr )
        {
            LOGGER_ERROR( "can't create input file '%s'"
                , this->getFileGroup()->getName().c_str()
            );

            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool ThreadTaskPrefetchStream::_onMain()
    {
        if( m_fileGroup->openInputFile( m_filePath, m_stream, 0, 0, false ) == false )
        {
            LOGGER_ERROR( "invalide open file '%s':'%s'"
                , this->getFileGroup()->getName().c_str()
                , this->getFilePath().c_str()
            );

            return false;
        }

        MemoryInterfacePtr memory = Helper::loadStreamArchiveMagicMemory( m_stream, m_archivator, m_magicNumber, m_magicVersion, "ThreadTaskPrefetchStream", __FILE__, __LINE__ );

        if( memory == nullptr )
        {
            LOGGER_ERROR( "invalide stream archive magic memory '%s':'%s'"
                , this->getFileGroup()->getName().c_str()
                , this->getFilePath().c_str()
            );

            return false;
        }

        m_memory = memory;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void ThreadTaskPrefetchStream::_onComplete( bool _successful )
    {
        ThreadTaskPrefetch::_onComplete( _successful );

        m_archivator = nullptr;
    }
}
