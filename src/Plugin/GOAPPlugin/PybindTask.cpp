#   include "PybindTask.h"
#   include "PybindTaskGenerator.h"

namespace Menge
{
    //////////////////////////////////////////////////////////////////////////
    PybindTask::PybindTask()
        : m_generator(nullptr)
    {
    }
    //////////////////////////////////////////////////////////////////////////
    PybindTask::~PybindTask()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::setGenerator( PybindTaskGenerator * _generator )
    {
        m_generator = _generator;
    }
    //////////////////////////////////////////////////////////////////////////
    PybindTaskGenerator * PybindTask::getGenerator() const
    {
        return m_generator;
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::setScriptObject( const pybind::object & _object )
    {
        m_object = _object;
    }
    //////////////////////////////////////////////////////////////////////////
    const pybind::object & PybindTask::getScriptObject() const
    {
        return m_object;
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onInitialize()
    {
        return m_generator->callEventInitialize( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::_onFinalize()
    {
        m_generator->callEventFinalize( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onValidate() const
    {
        return m_generator->callEventValidate( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onCheck() const
    {
        return m_generator->callEventCheck( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onRun()
    {
        return m_generator->callEventRun( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onSkipable() const
    {
        return m_generator->callEventSkipable( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::_onSkipNoSkiped()
    {
        m_generator->callEventSkipNoSkiped( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onSkipBlock()
    {
        return m_generator->callEventSkipBlock( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::_onComplete()
    {
        m_generator->callEventComplete( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::_onSkip()
    {
        m_generator->callEventSkip( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::_onCancel()
    {
        m_generator->callEventCancel( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindTask::_onFinally()
    {
        m_generator->callEventFinally( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onCheckRun() const
    {
        return m_generator->callEventCheckRun( m_object );
    }
    //////////////////////////////////////////////////////////////////////////
    bool PybindTask::_onCheckSkip() const
    {
        return m_generator->callEventCheckSkip( m_object );
    }
}