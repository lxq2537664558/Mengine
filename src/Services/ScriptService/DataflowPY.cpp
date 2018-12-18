#include "DataflowPY.h"

#include "Interface/ScriptServiceInterface.h"

#include "ScriptCodeData.h"

#include "Kernel/MemoryHelper.h"
#include "Kernel/FactoryPool.h"
#include "Kernel/AssertionFactory.h"
#include "Kernel/AssertionMemoryPanic.h"
#include "Kernel/Logger.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    DataflowPY::DataflowPY()
        : m_kernel( nullptr )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    DataflowPY::~DataflowPY()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    void DataflowPY::setKernel( pybind::kernel_interface * _kernel )
    {
        m_kernel = _kernel;
    }
    //////////////////////////////////////////////////////////////////////////
    pybind::kernel_interface * DataflowPY::getKernel() const
    {
        return m_kernel;
    }
    //////////////////////////////////////////////////////////////////////////
    bool DataflowPY::initialize()
    {
        m_factoryScriptCodeData = new FactoryPool<ScriptCodeData, 128>();

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void DataflowPY::finalize()
    {
        MENGINE_ASSERTION_FACTORY_EMPTY( m_factoryScriptCodeData );
        m_factoryScriptCodeData = nullptr;
    }
    //////////////////////////////////////////////////////////////////////////
    DataInterfacePtr DataflowPY::create()
    {
        ScriptCodeDataPtr data = m_factoryScriptCodeData->createObject();

        MENGINE_ASSERTION_MEMORY_PANIC( data, nullptr );

        return data;
    }
    //////////////////////////////////////////////////////////////////////////
    bool DataflowPY::load( const DataInterfacePtr & _data, const InputStreamInterfacePtr & _stream )
    {
        ScriptCodeData * data = stdex::intrusive_get<ScriptCodeData *>( _data );

        MemoryInterfacePtr source_memory = Helper::createMemoryCacheStreamExtraSize( _stream, 2, "DataflowPY", __FILE__, __LINE__ );

        MENGINE_ASSERTION_MEMORY_PANIC( source_memory, false );

        Char * source_buffer = source_memory->getBuffer();

        MENGINE_ASSERTION_MEMORY_PANIC( source_buffer, false );

        size_t stream_size = _stream->size();

        source_buffer[stream_size] = '\n';
        source_buffer[stream_size + 1] = '\0';

        PyObject * code = m_kernel->code_compile_file( source_buffer, "DataflowPY" );

        if( code == nullptr )
        {
            LOGGER_ERROR( "invalid marshal get object" );

            return nullptr;
        }

        if( m_kernel->code_check( code ) == false )
        {
            LOGGER_ERROR( "marshal get object not code"
            );

            return nullptr;
        }

        data->setScriptCode( pybind::make_borrowed_t( m_kernel, code ) );

        return true;
    }
}