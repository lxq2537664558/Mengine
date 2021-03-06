#include "Meshget.h" 

#include "Kernel/ResourceImage.h"

#include "Kernel/Logger.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    Meshget::Meshget()
        : m_invalidateVerticesWM( true )
        , m_invalidateVerticesColor( true )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    Meshget::~Meshget()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool Meshget::_compile()
    {
        if( m_surface == nullptr )
        {
            LOGGER_ERROR( "Meshget::_compile '%s' can`t setup surface"
                , this->getName().c_str()
            );

            return false;
        }

        if( m_surface->compile() == false )
        {
            LOGGER_ERROR( "Meshget::_compile '%s' can`t compile surface '%s'"
                , this->getName().c_str()
                , m_surface->getName().c_str()
            );

            return false;
        }

        this->invalidateVerticesWM();
        this->invalidateVerticesColor();

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::_release()
    {
        m_surface->release();
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::setSurface( const SurfacePtr & _surface )
    {
        if( m_surface == _surface )
        {
            return;
        }

        this->recompile( [this, _surface]() {m_surface = _surface; } );
    }
    //////////////////////////////////////////////////////////////////////////
    const SurfacePtr & Meshget::getSurface() const
    {
        return m_surface;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Meshget::setVertices( const pybind::list & _positions, const pybind::list & _uv, const pybind::list & _colors, const pybind::list & _indices )
    {
        uint32_t positions_count = _positions.size();
        uint32_t uvs_count = _uv.size();
        uint32_t colors_count = _colors.size();

        if( positions_count != uvs_count || positions_count != colors_count )
        {
            return false;
        }

        m_positions.resize( positions_count );
        m_uvs.resize( uvs_count );
        m_colors.resize( colors_count );

        for( uint32_t i = 0; i != positions_count; ++i )
        {
            m_positions[i] = _positions[i];
            m_uvs[i] = _uv[i];
            m_colors[i] = _colors[i];
        }

        uint32_t indices_count = _indices.size();

        m_indices.resize( indices_count );

        for( uint32_t i = 0; i != indices_count; ++i )
        {
            m_indices[i] = _indices[i];
        }

        m_verticesWM.resize( positions_count );

        this->invalidateVerticesColor();
        this->invalidateVerticesWM();

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::_update( const UpdateContext * _context )
    {
        EVENTABLE_METHOD( EVENT_MESHGET_UPDATE )
            ->onMeshgetUpdate( _context );
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::render( const RenderContext * _state ) const
    {
        if( m_positions.empty() == true )
        {
            return;
        }

        uint32_t vertexCount = (uint32_t)m_positions.size();
        uint32_t indicesCount = (uint32_t)m_indices.size();

        const VectorRenderVertex2D & vertices = this->getVerticesWM();
        const RenderMaterialInterfacePtr & material = m_surface->getMaterial();

        const RenderVertex2D * vertices_buff = &vertices[0];
        const RenderIndex * indices_buff = &m_indices[0];

        const mt::box2f * bb = this->getBoundingBox();

        this->addRenderObject( _state, material, nullptr, vertices_buff, vertexCount, indices_buff, indicesCount, bb, false );
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::_updateBoundingBox( mt::box2f & _boundingBox, mt::box2f ** _boundingBoxCurrent ) const
    {
        if( m_positions.empty() == true )
        {
            *_boundingBoxCurrent = nullptr;

            return;
        }

        const VectorRenderVertex2D & vertices = this->getVerticesWM();

        mt::reset( _boundingBox, vertices[0].position.x, vertices[0].position.y );

        VectorPosition::size_type vertexCount = m_positions.size();

        for( VectorPosition::size_type i = 1; i != vertexCount; ++i )
        {
            const mt::vec3f & v = m_positions[i];

            mt::add_internal_point( _boundingBox, v.x, v.y );
        }

        *_boundingBoxCurrent = &_boundingBox;
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::_invalidateWorldMatrix()
    {
        Node::_invalidateWorldMatrix();

        this->invalidateVerticesWM();
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::invalidateVerticesWM()
    {
        m_invalidateVerticesWM = true;

        this->invalidateBoundingBox();
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::_invalidateColor()
    {
        this->invalidateVerticesColor();
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::invalidateVerticesColor()
    {
        m_invalidateVerticesColor = true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::updateVerticesColor() const
    {
        m_invalidateVerticesColor = false;

        Color color;
        this->calcTotalColor( color );

        const Color & textureColor = m_surface->getColor();
        color *= textureColor;

        VectorPosition::size_type vertexCount = m_positions.size();

        for( VectorPosition::size_type i = 0; i != vertexCount; ++i )
        {
            const mt::vec4f & c = m_colors[i];

            Color v_color( c.x, c.y, c.z, c.w );
            v_color *= color;

            uint32_t argb = v_color.getAsARGB();

            m_verticesWM[i].color = argb;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Meshget::updateVerticesWM() const
    {
        m_invalidateVerticesWM = false;

        const mt::mat4f & wm = this->getWorldMatrix();

        VectorPosition::size_type vertexCount = m_positions.size();

        for( VectorPosition::size_type i = 0; i != vertexCount; ++i )
        {
            const mt::vec3f & pos = m_positions[i];
            const mt::vec2f & uv = m_uvs[i];

            mt::vec3f & wm_pos = m_verticesWM[i].position;
            mt::mul_v3_v3_m4( wm_pos, pos, wm );

            m_surface->correctUV( 0, uv, m_verticesWM[i].uv + 0 );
            m_surface->correctUV( 1, uv, m_verticesWM[i].uv + 1 );
        }
    }
}
