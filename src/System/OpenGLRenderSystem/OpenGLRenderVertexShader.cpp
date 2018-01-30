#   include "OpenGLRenderVertexShader.h"

#   include "OpenGLRenderError.h"

#   include "Logger/Logger.h"


namespace Menge
{
    //////////////////////////////////////////////////////////////////////////
    OpenGLRenderVertexShader::OpenGLRenderVertexShader()
        : m_serviceProvider( nullptr )
        , m_shaderId( 0 )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    OpenGLRenderVertexShader::~OpenGLRenderVertexShader()
    {
        if( m_shaderId != 0 )
        {
            GLCALL( m_serviceProvider, glDeleteShader, (m_shaderId) );
            m_shaderId = 0;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void OpenGLRenderVertexShader::setServiceProvider( ServiceProviderInterface * _serviceProvider )
    {
        m_serviceProvider = _serviceProvider;
    }
    //////////////////////////////////////////////////////////////////////////
    ServiceProviderInterface * OpenGLRenderVertexShader::getServiceProvider() const
    {
        return m_serviceProvider;
    }
    //////////////////////////////////////////////////////////////////////////
    const ConstString & OpenGLRenderVertexShader::getName() const
    {
        return m_name;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenGLRenderVertexShader::initialize( const ConstString & _name, const MemoryInterfacePtr & _memory )
    {
        m_name = _name;
        m_memory = _memory;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenGLRenderVertexShader::compile()
    {
        GLuint shaderId;		
        GLCALLR( m_serviceProvider, shaderId, glCreateShader, (GL_VERTEX_SHADER) );

        if( shaderId == 0 )
        {
            LOGGER_ERROR( m_serviceProvider )("OpenGLRenderVertexShader::initialize %s invalid create shader"
                , m_name.c_str()
                );

            return false;
        }

        const char * str_source = m_memory->getMemory();
        GLint str_size = (GLint)m_memory->getSize();

        GLCALL( m_serviceProvider, glShaderSource, ( shaderId, 1, &str_source, &str_size ) );
        GLCALL( m_serviceProvider, glCompileShader, ( shaderId ) );

        GLint status;
        GLCALL( m_serviceProvider, glGetShaderiv, ( shaderId, GL_COMPILE_STATUS, &status ) );

        if( status == GL_FALSE )
        {
            GLchar errorLog[1024];
            GLCALL( m_serviceProvider, glGetShaderInfoLog, ( shaderId, sizeof(errorLog) - 1, nullptr, errorLog ) );

            LOGGER_ERROR( m_serviceProvider )("OpenGLRenderVertexShader::initialize '%s' compilation shader error '%s'"
                , m_name.c_str()
                , errorLog
                );

            return false;
        }

        m_shaderId = shaderId;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void OpenGLRenderVertexShader::attach( GLuint _program )
    { 
        GLCALL( m_serviceProvider, glAttachShader, ( _program, m_shaderId ) );
    }
}	// namespace Menge
