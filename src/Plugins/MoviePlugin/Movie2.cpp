#include "Movie2.h"

#include "Interface/TimelineServiceInterface.h"
#include "Interface/NodeInterface.h"
#include "Interface/StringizeInterface.h"
#include "Interface/PrototypeServiceInterface.h"
#include "Interface/ResourceServiceInterface.h"

#include "Plugins/AstralaxParticlePlugin/UnknownParticleEmitterInterface.h"
#include "Plugins/VideoPlugin/VideoUnknownInterface.h"

#include "Engine/SurfaceImage.h"
#include "Engine/SurfaceSound.h"
#include "Engine/SurfaceTrackMatte.h"

#include "Kernel/Materialable.h"
#include "Kernel/Layer.h"

#include "Kernel/NodeHelper.h"
#include "Kernel/NodeRenderHelper.h"
#include "Kernel/ResourceImage.h"

#include "Kernel/Logger.h"

#include "Config/Stringstream.h"

#include "math/quat.h"

#include <algorithm>

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    Movie2::Movie2()
        : m_composition( nullptr )
        , m_duration( 0.f )
        , m_frameDuration( 0.f )
        , m_hasBounds( false )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    Movie2::~Movie2()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::setResourceMovie2( const ResourceMovie2Ptr & _resourceMovie2 )
    {
        if( m_resourceMovie2 == _resourceMovie2 )
        {
            return;
        }

        this->recompile( [this, _resourceMovie2]() {m_resourceMovie2 = _resourceMovie2; } );
    }
    //////////////////////////////////////////////////////////////////////////
    const ResourceMovie2Ptr & Movie2::getResourceMovie2() const
    {
        return m_resourceMovie2;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::setCompositionName( const ConstString & _compositionName )
    {
        if( m_compositionName == _compositionName )
        {
            return true;
        }

        m_compositionName = _compositionName;

        if( m_resourceMovie2 == nullptr )
        {
            return true;
        }

        this->destroyCompositionLayers_();

        if( this->createCompositionLayers_() == false )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    const ConstString & Movie2::getCompositionName() const
    {
        return m_compositionName;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::setTextAliasEnvironment( const ConstString & _aliasEnvironment )
    {
        if( m_aliasEnvironment == _aliasEnvironment )
        {
            return;
        }

        m_aliasEnvironment = _aliasEnvironment;

        for( MapTexts::value_type & value : m_texts )
        {
            const TextFieldPtr & text = value.second;
            
            text->setTextAliasEnvironment( m_aliasEnvironment );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    const ConstString & Movie2::getTextAliasEnvironment() const
    {
        return m_aliasEnvironment;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::checkInterruptElement() const
    {
        for( const SurfacePtr & surface : m_surfaces )
        {
            AnimationInterface * animation = surface->getAnimation();

            if( animation != nullptr )
            {
                if( animation->isInterrupt() == true )
                {
                    return false;
                }
            }
        }

        for( const MapParticleEmitter2s::value_type & value : m_particleEmitters )
        {
            const NodePtr & particle = value.second;

            AnimationInterface * animation = particle->getAnimation();

            if( animation != nullptr )
            {
                if( animation->isInterrupt() == true )
                {
                    return false;
                }
            }
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::createCompositionLayers_()
    {
        const ResourceMovie2CompositionDesc * composition = m_resourceMovie2->getCompositionDesc( m_compositionName );

        if( composition == nullptr )
        {
            Stringstream ss;

            m_resourceMovie2->foreachCompositionDesc( [&ss]( const ConstString & _name, const ResourceMovie2CompositionDesc & _desc )
            {
                if( _desc.master == false )
                {
                    return;
                }
                
                ss << _name.c_str() << ", ";
            } );

            LOGGER_ERROR( "movie '%s' resource '%s' path '%s' invalid get composition name '%s' in [%s]"
                , this->getName().c_str()
                , this->getResourceMovie2()->getName().c_str()
                , this->getResourceMovie2()->getFilePath().c_str()
                , m_compositionName.c_str()
                , ss.str().c_str()
            );         

            return false;
        }

        m_duration = composition->duration * 1000.f;
        m_frameDuration = composition->frameDuration * 1000.f;
        m_hasBounds = composition->has_bounds;
        m_bounds = composition->bounds;

        for( const ResourceMovie2CompositionLayer & layer : composition->layers )
        {
            if( layer.type == STRINGIZE_STRING_LOCAL( "TextField" ) )
            {
                TextFieldPtr node = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "TextField" ) );

                if( node == nullptr )
                {
                    return false;
                }

                node->setName( layer.name );
                node->setExternalRender( true );

                node->setTextID( layer.name );
                node->setTextAliasEnvironment( m_aliasEnvironment );

                node->setPixelsnap( false );

                this->addText_( layer.index, node );

                MatrixProxyPtr matrixProxy = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MatrixProxy" ) );

                if( matrixProxy == nullptr )
                {
                    return false;
                }

                matrixProxy->setName( layer.name );

                matrixProxy->setProxyMatrix( layer.matrix );
                matrixProxy->setLocalColor( layer.color );

                matrixProxy->addChild( node );

                this->addChild( matrixProxy );

                this->addMatrixProxy_( matrixProxy );
            }
            else if( layer.type == STRINGIZE_STRING_LOCAL( "Movie2Slot" ) )
            {
                Movie2SlotPtr node = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "Movie2Slot" ) );

                if( node == nullptr )
                {
                    return false;
                }

                node->setName( layer.name );

                RenderInterface * render = node->getRender();
                render->setExternalRender( true );

                node->setMovie( this );

                this->addSlot_( layer.index, node );

                MatrixProxyPtr matrixProxy = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MatrixProxy" ) );

                if( matrixProxy == nullptr )
                {
                    return false;
                }

                matrixProxy->setName( layer.name );

                matrixProxy->setProxyMatrix( layer.matrix );
                matrixProxy->setLocalColor( layer.color );

                matrixProxy->addChild( node );

                this->addChild( matrixProxy );

                this->addMatrixProxy_( matrixProxy );
            }
            else if( layer.type == STRINGIZE_STRING_LOCAL( "HotSpotPolygon" ) )
            {
                HotSpotPolygonPtr node = NODE_SERVICE()
                    ->createNode( STRINGIZE_STRING_LOCAL( "HotSpotPolygon" ) );

                if( node == nullptr )
                {
                    return false;
                }

                node->setName( layer.name );

                this->addSocket_( layer.index, node );

                MatrixProxyPtr matrixProxy = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MatrixProxy" ) );

                if( matrixProxy == nullptr )
                {
                    return false;
                }

                matrixProxy->setName( layer.name );

                matrixProxy->setProxyMatrix( layer.matrix );
                matrixProxy->setLocalColor( layer.color );

                matrixProxy->addChild( node );

                this->addChild( matrixProxy );

                this->addMatrixProxy_( matrixProxy );
            }
            else if( layer.type == STRINGIZE_STRING_LOCAL( "ParticleEmitter2" ) )
            {
                NodePtr node = NODE_SERVICE()
                    ->createNode( STRINGIZE_STRING_LOCAL( "ParticleEmitter2" ) );

                if( node == nullptr )
                {
                    return false;
                }

                node->setName( layer.name );

                RenderInterface * render = node->getRender();
                render->setExternalRender( true );

                this->addParticle_( layer.index, node );

                MatrixProxyPtr matrixProxy = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MatrixProxy" ) );

                if( matrixProxy == nullptr )
                {
                    return false;
                }

                matrixProxy->setName( layer.name );

                matrixProxy->setProxyMatrix( layer.matrix );
                matrixProxy->setLocalColor( layer.color );

                matrixProxy->addChild( node );

                this->addChild( matrixProxy );

                this->addMatrixProxy_( matrixProxy );
            }
            else if( layer.type == STRINGIZE_STRING_LOCAL( "ShapeQuadFixed" ) )
            {
                ResourceImagePtr resourceImage = RESOURCE_SERVICE()
                    ->getResourceReference( layer.name );

                if( resourceImage == nullptr )
                {
                    LOGGER_ERROR( "Movie2 '%s' resource '%s' composition '%s' layer '%s' invalid get resource for image %s"
                        , this->getName().c_str()
                        , this->getResourceMovie2()->getName().c_str()
                        , this->getCompositionName().c_str()
                        , layer.name.c_str()
                        , layer.name.c_str()
                    );

                    return false;
                }

                SurfaceImagePtr surface = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Surface" ), STRINGIZE_STRING_LOCAL( "SurfaceImage" ) );

                if( surface == nullptr )
                {
                    return false;
                }

                surface->setResourceImage( resourceImage );

                ShapeQuadFixedPtr node = NODE_SERVICE()
                    ->createNode( layer.type );

                if( node == nullptr )
                {
                    LOGGER_ERROR( "Movie::createMovieSprite_ '%s' resource '%s' composition '%s' layer '%s' invalid create 'Sprite'"
                        , this->getName().c_str()
                        , this->getResourceMovie2()->getName().c_str()
                        , this->getCompositionName().c_str()
                        , layer.name.c_str()
                    );

                    return false;
                }

                node->setName( layer.name );
                node->setExternalRender( true );

                node->setSurface( surface );

                this->addSprite_( layer.index, node );

                MatrixProxyPtr matrixProxy = PROTOTYPE_SERVICE()
                    ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MatrixProxy" ) );

                if( matrixProxy == nullptr )
                {
                    return false;
                }

                matrixProxy->setName( layer.name );

                matrixProxy->setProxyMatrix( layer.matrix );
                matrixProxy->setLocalColor( layer.color );

                matrixProxy->addChild( node );

                this->addChild( matrixProxy );

                this->addMatrixProxy_( matrixProxy );
            }
        }

        for( const ResourceMovie2CompositionSubComposition & subcomposition : composition->subcompositions )
        {
            Movie2SubCompositionPtr node = PROTOTYPE_SERVICE()
                ->generatePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "Movie2SubComposition" ) );

            node->setMovie( this );

            node->setSubMovieCompositionName( subcomposition.name );

            this->addSubMovieComposition_( subcomposition.name, node );
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::destroyCompositionLayers_()
    {
        m_slots.clear();
        m_sockets.clear();
        m_texts.clear();
        m_particleEmitters.clear();
        m_sprites.clear();

        m_matrixProxies.clear();
    }
    //////////////////////////////////////////////////////////////////////////
    float Movie2::getDuration() const
    {
        return m_duration;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::setWorkAreaFromEvent( const ConstString & _eventName )
    {
        if( m_composition == nullptr )
        {
            LOGGER_ERROR( "Movie2::setWorkAreaFromEvent '%s' invalid setup '%s' not compile"
                , this->getName().c_str()
                , _eventName.c_str()
            );

            return false;
        }

        float a, b;
        ae_bool_t ok = ae_get_movie_composition_node_in_out_time( m_composition, _eventName.c_str(), AE_MOVIE_LAYER_TYPE_EVENT, &a, &b );

        if( ok == AE_FALSE )
        {
            return false;
        }

        ae_set_movie_composition_work_area( m_composition, a, b );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::removeWorkArea()
    {
        if( m_composition == nullptr )
        {
            LOGGER_ERROR( "Movie2::removeWorkArea '%s' not compile"
                , this->getName().c_str()
            );

            return false;
        }

        ae_remove_movie_composition_work_area( m_composition );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasCompositionBounds() const
    {
        return m_hasBounds;
    }
    //////////////////////////////////////////////////////////////////////////
    const Viewport & Movie2::getCompositionBounds() const
    {
        return m_bounds;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasSubComposition( const ConstString & _name ) const
    {
        MapSubCompositions::const_iterator it_found = m_subCompositions.find( _name );

        if( it_found == m_subCompositions.end() )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    const Movie2SubCompositionPtr & Movie2::getSubComposition( const ConstString & _name ) const
    {
        MapSubCompositions::const_iterator it_found = m_subCompositions.find( _name );

        if( it_found == m_subCompositions.end() )
        {
            return Movie2SubCompositionPtr::none();
        }

        const Movie2SubCompositionPtr & subComposition = it_found->second;

        return subComposition;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::setEnableMovieLayers( const ConstString & _name, bool _enable )
    {
        if( m_composition == nullptr )
        {
            LOGGER_ERROR( "Movie2::setEnableMovieLayers '%s' invalid get layer '%s' not compile"
                , this->getName().c_str()
                , _name.c_str()
            );

            return;
        }

        ae_set_movie_composition_nodes_enable_any( m_composition, _name.c_str(), _enable ? AE_TRUE : AE_FALSE );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::foreachRenderSlots( const RenderContext * _context, const LambdaMovieRenderSlot & _lambda )
    {
        const mt::mat4f & wm = this->getWorldMatrix();

        ae_voidptr_t composition_camera_data = ae_get_movie_composition_camera_data( m_composition );
        (void)composition_camera_data;

        ae_uint32_t compute_movie_mesh_iterator = 0;

        aeMovieRenderMesh mesh;
        while( ae_compute_movie_mesh( m_composition, &compute_movie_mesh_iterator, &mesh ) == AE_TRUE )
        {
            RenderContext context;

            if( mesh.camera_data != nullptr )
            {
                Movie2::Camera * camera = reinterpret_cast<Movie2::Camera *>(mesh.camera_data);

                context.camera = camera->projection;
                context.viewport = camera->viewport;
                context.transformation = _context->transformation;
                context.scissor = _context->scissor;
                context.target = _context->target;
            }
            else
            {
                context = *_context;
            }

            if( mesh.viewport != nullptr )
            {
                Movie2ScissorPtr scissor = new FactorableUnique<Movie2Scissor>();

                scissor->setViewport( wm, mesh.viewport );

                context.scissor = scissor;
            }
            else
            {
                context.scissor = _context->scissor;
            }

            if( mesh.track_matte_data == nullptr )
            {
                switch( mesh.layer_type )
                {
                case AE_MOVIE_LAYER_TYPE_SLOT:
                    {
                        Movie2Slot * node = reinterpret_node_cast<Movie2Slot *>(mesh.element_data);

                        _lambda( node, &context );
                    }break;
                case AE_MOVIE_LAYER_TYPE_SOCKET:
                    {
                        HotSpotPolygon * node = reinterpret_node_cast<HotSpotPolygon *>(mesh.element_data);

                        _lambda( node, &context );
                    }break;
                case AE_MOVIE_LAYER_TYPE_SPRITE:
                    {
                        ShapeQuadFixed * node = reinterpret_node_cast<ShapeQuadFixed *>(mesh.element_data);

                        _lambda( node, &context );
                    }break;
                case AE_MOVIE_LAYER_TYPE_TEXT:
                    {
                        TextField * node = reinterpret_node_cast<TextField *>(mesh.element_data);

                        _lambda( node, &context );
                    }break;
                case AE_MOVIE_LAYER_TYPE_PARTICLE:
                    {
                        Node * node = reinterpret_node_cast<Node *>(mesh.element_data);

                        _lambda( node, &context );
                    }break;
                default:
                    break;
                }
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::_play( uint32_t _enumerator, float _time )
    {
        if( this->isCompile() == false )
        {
            return false;
        }

        ae_play_movie_composition( m_composition, 0.f );

        EVENTABLE_METHOD( this, EVENT_ANIMATION_PLAY )
            ->onAnimationPlay( _enumerator, _time );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::_restart( uint32_t _enumerator, float _time )
    {
        EVENTABLE_METHOD( this, EVENT_ANIMATION_RESTART )
            ->onAnimationRestart( _enumerator, _time );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_pause( uint32_t _enumerator )
    {
        EVENTABLE_METHOD( this, EVENT_ANIMATION_PAUSE )
            ->onAnimationPause( _enumerator );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_resume( uint32_t _enumerator, float _time )
    {
        EVENTABLE_METHOD( this, EVENT_ANIMATION_RESUME )
            ->onAnimationResume( _enumerator, _time );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_stop( uint32_t _enumerator )
    {
        ae_stop_movie_composition( m_composition );

        EVENTABLE_METHOD( this, EVENT_ANIMATION_STOP )
            ->onAnimationStop( _enumerator );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_end( uint32_t _enumerator )
    {
        EVENTABLE_METHOD( this, EVENT_ANIMATION_END )
            ->onAnimationEnd( _enumerator );
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::_interrupt( uint32_t _enumerator )
    {
        (void)_enumerator;

        ae_interrupt_movie_composition( m_composition, AE_FALSE );

        EVENTABLE_METHOD( this, EVENT_ANIMATION_INTERRUPT )
            ->onAnimationInterrupt( _enumerator );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_bool_t __movie_composition_camera_provider( const aeMovieCameraProviderCallbackData * _callbackData, ae_voidptrptr_t _cd, ae_voidptr_t _ud )
    {
        Movie2 * movie2 = reinterpret_node_cast<Movie2 *>(_ud);

        ConstString c_name = Helper::stringizeString( _callbackData->name );

        Movie2::Camera * old_camera;
        if( movie2->getCamera( c_name, &old_camera ) == true )
        {
            *_cd = old_camera;

            return AE_TRUE;
        }

        RenderCameraProjectionPtr renderCameraProjection = NODE_SERVICE()
            ->createNode( STRINGIZE_STRING_LOCAL( "RenderCameraProjection" ) );

        renderCameraProjection->setName( c_name );

        mt::vec3f cameraTarget;
        cameraTarget.from_f3( _callbackData->target );
        mt::vec3f cameraPosition;
        cameraPosition.from_f3( _callbackData->position );

        mt::vec3f cameraDirection;
        mt::norm_v3_v3( cameraDirection, cameraTarget - cameraPosition );

        float aspect = _callbackData->width / _callbackData->height;

        renderCameraProjection->setCameraPosition( cameraPosition );
        renderCameraProjection->setCameraDirection( cameraDirection );
        renderCameraProjection->setCameraFOV( _callbackData->fov );
        renderCameraProjection->setCameraAspect( aspect );

        RenderViewportPtr renderViewport = NODE_SERVICE()
            ->createNode( STRINGIZE_STRING_LOCAL( "RenderViewport" ) );

        renderViewport->setName( c_name );

        Viewport vp;
        vp.begin.x = 0.f;
        vp.begin.y = 0.f;

        vp.end.x = _callbackData->width;
        vp.end.y = _callbackData->height;

        renderViewport->setViewport( vp );

        Movie2::Camera * new_camera = movie2->addCamera( c_name, renderCameraProjection, renderViewport );

        *_cd = new_camera;

        return AE_TRUE;
    }
    //////////////////////////////////////////////////////////////////////////
    static void __movie_composition_camera_deleter( const aeMovieCameraDeleterCallbackData * _callbackData, ae_voidptr_t _data )
    {
        Movie2 * movie2 = reinterpret_node_cast<Movie2 *>(_data);

        ConstString c_name = Helper::stringizeString( _callbackData->name );

        movie2->removeCamera( c_name );
    }
    //////////////////////////////////////////////////////////////////////////
    static void __movie_composition_camera_update( const aeMovieCameraUpdateCallbackData * _callbackData, ae_voidptr_t _data )
    {
        (void)_data;

        Movie2::Camera * camera = reinterpret_cast<Movie2::Camera *>(_callbackData->camera_data);

        //camera
        mt::vec3f cameraTarget;
        cameraTarget.from_f3( _callbackData->target );
        mt::vec3f cameraPosition;
        cameraPosition.from_f3( _callbackData->position );

        mt::vec3f cameraDirection;
        mt::norm_v3_v3( cameraDirection, cameraTarget - cameraPosition );

        camera->projection->setCameraPosition( cameraPosition );
        camera->projection->setCameraDirection( cameraDirection );
    }
    //////////////////////////////////////////////////////////////////////////
    namespace Detail
    {
        //////////////////////////////////////////////////////////////////////////
        static EMaterialBlendMode getMovieBlendMode( ae_blend_mode_t _ae_blend_mode )
        {
            EMaterialBlendMode blend_mode = EMB_NORMAL;

            switch( _ae_blend_mode )
            {
            case AE_MOVIE_BLEND_ADD:
                blend_mode = EMB_ADD;
                break;
            case AE_MOVIE_BLEND_SCREEN:
                blend_mode = EMB_SCREEN;
                break;
            case AE_MOVIE_BLEND_MULTIPLY:
                blend_mode = EMB_MULTIPLY;
                break;
            default:
                break;
            };

            return blend_mode;
        }
        //////////////////////////////////////////////////////////////////////////
        static EMaterialBlendMode getMovieLayerBlendMode( const aeMovieLayerData * _layer )
        {
            ae_blend_mode_t layer_blend_mode = ae_get_movie_layer_data_blend_mode( _layer );

            EMaterialBlendMode blend_mode = getMovieBlendMode( layer_blend_mode );

            return blend_mode;
        }
        //////////////////////////////////////////////////////////////////////////
        static void updateMatrixProxy( const NodePtr & _node, ae_bool_t _immutable_matrix, const float * _matrix, ae_bool_t _immutable_color, const ae_color_t & _color, ae_color_channel_t _opacity )
        {
            Node * nodeParent = _node->getParent();

            MatrixProxy * matrixProxy = static_node_cast<MatrixProxy *>(nodeParent);

            if( _immutable_matrix == AE_FALSE )
            {
                mt::mat4f mp;
                mp.from_f12( _matrix );
                matrixProxy->setProxyMatrix( mp );
            }

            if( _immutable_color == AE_FALSE )
            {
                matrixProxy->setLocalColorRGBA( _color.r, _color.g, _color.b, _opacity );
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    ae_bool_t Movie2::__movie_composition_node_provider( const aeMovieNodeProviderCallbackData * _callbackData, ae_voidptrptr_t _nd, ae_voidptr_t _ud )
    {
        Movie2 * movie2 = (Movie2 *)_ud;

        const aeMovieLayerData * layer = _callbackData->layer;

        ae_uint32_t node_index = _callbackData->index;
        const ae_char_t * layer_name = ae_get_movie_layer_data_name( layer );

        ae_bool_t is_track_matte = ae_is_movie_layer_data_track_mate( layer );

        if( is_track_matte == AE_TRUE )
        {
            *_nd = AE_NULL;

            return AE_TRUE;
        }

        aeMovieLayerTypeEnum type = ae_get_movie_layer_data_type( layer );

        switch( type )
        {
        case AE_MOVIE_LAYER_TYPE_SPRITE:
            {
                const ShapeQuadFixedPtr & node = movie2->getSprite_( node_index );

                if( node == nullptr )
                {
                    LOGGER_ERROR( "Movie::createMovieSprite_ '%s' resource '%s' composition '%s' layer '%s' invalid create 'Sprite'"
                        , movie2->getName().c_str()
                        , movie2->getResourceMovie2()->getName().c_str()
                        , movie2->getCompositionName().c_str()
                        , layer_name
                    );

                    return AE_FALSE;
                }

                const SurfacePtr & surface = node->getSurface();

                EMaterialBlendMode blend_mode = Detail::getMovieLayerBlendMode( layer );

                surface->setBlendMode( blend_mode );

                *_nd = node.get();

                return AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_TEXT:
            {
                const TextFieldPtr & node = movie2->getText_( node_index );

                if( node == nullptr )
                {
                    return AE_FALSE;
                }

                Detail::updateMatrixProxy( node, AE_FALSE, _callbackData->matrix, AE_FALSE, _callbackData->color, _callbackData->opacity );

                if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'h', 'r' ) ) == AE_TRUE )
                {
                    node->setHorizontalRightAlign();
                }

                if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'h', 'c' ) ) == AE_TRUE )
                {
                    node->setHorizontalCenterAlign();
                }

                if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'h', 'l' ) ) == AE_TRUE )
                {
                    node->setHorizontalLeftAlign();
                }

                if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'v', 't' ) ) == AE_TRUE )
                {
                    node->setVerticalTopAlign();
                }

                if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'v', 'c' ) ) == AE_TRUE )
                {
                    node->setVerticalCenterAlign();
                }

                if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'v', 'b' ) ) == AE_TRUE )
                {
                    node->setVerticalBottomAlign();
                }

                *_nd = node.get();

                return AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SLOT:
            {
                const Movie2SlotPtr & node = movie2->getSlot_( node_index );

                if( node == nullptr )
                {
                    return AE_FALSE;
                }

                *_nd = node.get();

                return AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SOCKET:
            {
                const HotSpotPolygonPtr & node = movie2->getSocket_( node_index );

                if( node == nullptr )
                {
                    return AE_FALSE;
                }

                const ae_polygon_t * polygon;
                if( ae_get_movie_layer_data_socket_polygon( _callbackData->layer, 0, &polygon ) == AE_FALSE )
                {
                    return false;
                }

                Polygon p;
                for( ae_uint32_t index = 0; index != polygon->point_count; ++index )
                {
                    const ae_vector2_t * v = polygon->points + index;

                    mt::vec2f v2( (*v)[0], (*v)[1] );

                    p.append( v2 );
                }

                node->setPolygon( p );

                *_nd = node.get();

                return AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_PARTICLE:
            {
                const NodePtr & node = movie2->getParticle_( node_index );

                if( node == nullptr )
                {
                    return AE_FALSE;
                }

                UnknownParticleEmitterInterfacePtr unknownParticleEmitter2 = node->getUnknown();

                if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', '\0', 't' ) ) == AE_TRUE )
                {
                    unknownParticleEmitter2->setEmitterPositionRelative( true );
                    unknownParticleEmitter2->setEmitterCameraRelative( false );
                    unknownParticleEmitter2->setEmitterTranslateWithParticle( false );
                }
                else
                {
                    unknownParticleEmitter2->setEmitterPositionRelative( false );
                    unknownParticleEmitter2->setEmitterCameraRelative( false );
                    unknownParticleEmitter2->setEmitterTranslateWithParticle( true );
                }

                Resource * resourceParticle = reinterpret_node_cast<Resource *>(ae_get_movie_layer_data_resource_data( _callbackData->layer ));

                unknownParticleEmitter2->setResourceParticle( resourceParticle );

                unknownParticleEmitter2->setEmitterPositionProviderOriginOffset( -mt::vec3f( 1024.f, 1024.f, 0.f ) );

                //EMaterialBlendMode blend_mode = getMovieBlendMode( layer );

                AnimationInterface * animation = node->getAnimation();

                ae_float_t layer_stretch = ae_get_movie_layer_data_stretch( _callbackData->layer );
                animation->setStretch( layer_stretch );
                animation->setLoop( _callbackData->incessantly );

                *_nd = node.get();

                return AE_TRUE;
            }break;
        default:
            break;
        };

        if( _callbackData->track_matte_layer != AE_NULL )
        {
            switch( type )
            {
            case AE_MOVIE_LAYER_TYPE_IMAGE:
                {
                    SurfaceTrackMattePtr surfaceTrackMatte = PROTOTYPE_SERVICE()
                        ->generatePrototype( STRINGIZE_STRING_LOCAL( "Surface" ), STRINGIZE_STRING_LOCAL( "SurfaceTrackMatte" ) );

                    if( surfaceTrackMatte == nullptr )
                    {
                        return AE_FALSE;
                    }

                    ConstString c_name = Helper::stringizeString( layer_name );
                    surfaceTrackMatte->setName( c_name );

                    ResourceImage * resourceImage = reinterpret_node_cast<ResourceImage *>(ae_get_movie_layer_data_resource_data( _callbackData->layer ));
                    ResourceImage * resourceTrackMatteImage = reinterpret_node_cast<ResourceImage *>(ae_get_movie_layer_data_resource_data( _callbackData->track_matte_layer ));

                    surfaceTrackMatte->setResourceImage( resourceImage );
                    surfaceTrackMatte->setResourceTrackMatteImage( resourceTrackMatteImage );

                    ae_track_matte_mode_t track_matte_mode = ae_get_movie_layer_data_track_matte_mode( _callbackData->layer );

                    switch( track_matte_mode )
                    {
                    case AE_MOVIE_TRACK_MATTE_ALPHA:
                        {
                            surfaceTrackMatte->setTrackMatteMode( ESTM_MODE_ALPHA );
                        }break;
                    case AE_MOVIE_TRACK_MATTE_ALPHA_INVERTED:
                        {
                            surfaceTrackMatte->setTrackMatteMode( ESTM_MODE_ALPHA_INVERTED );
                        }break;
                    default:
                        break;
                    }

                    EMaterialBlendMode blend_mode = Detail::getMovieLayerBlendMode( layer );

                    surfaceTrackMatte->setBlendMode( blend_mode );

                    movie2->addSurface( surfaceTrackMatte );

                    *_nd = surfaceTrackMatte.get();

                    return AE_TRUE;
                }break;
            default:
                {
                }break;
            }
        }
        else
        {
            switch( type )
            {
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                    SurfacePtr surface = PROTOTYPE_SERVICE()
                        ->generatePrototype( STRINGIZE_STRING_LOCAL( "Surface" ), STRINGIZE_STRING_LOCAL( "SurfaceVideo" ) );

                    if( surface == nullptr )
                    {
                        return AE_FALSE;
                    }

                    ConstString c_name = Helper::stringizeString( layer_name );
					surface->setName( c_name );

                    Resource * resourceVideo = reinterpret_node_cast<Resource *>(ae_get_movie_layer_data_resource_data( _callbackData->layer ));

					UnknownVideoSurfaceInterfacePtr unknownVideoSurface = surface->getUnknown();

					unknownVideoSurface->setResourceVideo( resourceVideo );

                    EMaterialBlendMode blend_mode = Detail::getMovieLayerBlendMode( layer );

					AnimationInterface * surface_animation = surface->getAnimation();

					surface_animation->setLoop( _callbackData->incessantly );

					surface->setBlendMode( blend_mode );
					surface->setPremultiplyAlpha( true );

                    movie2->addSurface(surface);

                    *_nd = surface.get();

                    return AE_TRUE;
                }break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                {
                    SurfaceSoundPtr surfaceSound = PROTOTYPE_SERVICE()
                        ->generatePrototype( STRINGIZE_STRING_LOCAL( "Surface" ), STRINGIZE_STRING_LOCAL( "SurfaceSound" ) );

                    if( surfaceSound == nullptr )
                    {
                        return AE_FALSE;
                    }

                    ConstString c_name = Helper::stringizeString( layer_name );
                    surfaceSound->setName( c_name );

                    surfaceSound->setLoop( _callbackData->incessantly );
                    surfaceSound->setInterpolateVolume( false );

                    if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'v', 'o' ) ) == AE_TRUE )
                    {
                        surfaceSound->setSoundCategory( ES_SOURCE_CATEGORY_VOICE );
                    }

                    if( ae_has_movie_layer_data_option( layer, AE_OPTION( '\0', '\0', 'm', 'u' ) ) == AE_TRUE )
                    {
                        surfaceSound->setSoundCategory( ES_SOURCE_CATEGORY_MUSIC );
                    }

                    ResourceSound * resourceSound = reinterpret_node_cast<ResourceSound *>(ae_get_movie_layer_data_resource_data( _callbackData->layer ));

                    surfaceSound->setResourceSound( resourceSound );

                    movie2->addSurface( surfaceSound );

                    *_nd = surfaceSound.get();

                    return AE_TRUE;
                }break;
            default:
                break;
            }
        }

        *_nd = AE_NULL;

        return AE_TRUE;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_composition_node_deleter( const aeMovieNodeDeleterCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        Movie2 * movie2 = (Movie2 *)_ud;
        (void)movie2;

        ae_bool_t is_track_matte = ae_is_movie_layer_data_track_mate( _callbackData->layer );

        if( is_track_matte == AE_TRUE )
        {
            return;
        }

        aeMovieLayerTypeEnum type = ae_get_movie_layer_data_type( _callbackData->layer );

        if( _callbackData->track_matte_layer != AE_NULL )
        {
            switch( type )
            {
            case AE_MOVIE_LAYER_TYPE_IMAGE:
                {
                    //SurfaceTrackMatte * surfaceTrackMatte = (SurfaceTrackMatte *)_callbackData->element;

                    //movie2->removeSurface( surfaceTrackMatte );
                }break;
            default:
                {
                }break;
            }
        }
        else
        {
            switch( type )
            {
            case AE_MOVIE_LAYER_TYPE_PARTICLE:
                {
                    //ParticleEmitter2 * particleEmitter = (ParticleEmitter2 *)_callbackData->element;

                    //movie2->removeParticle( particleEmitter );
                }break;
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                    //SurfaceVideo * surfaceVideo = (SurfaceVideo *)_callbackData->element;

                    //movie2->removeSurface( surfaceVideo );
                }break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                {
                    //SurfaceSound * surfaceSound = (SurfaceSound *)_callbackData->element;

                    //movie2->removeSurface( surfaceSound );
                }break;
            default:
                break;
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_composition_node_update( const aeMovieNodeUpdateCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_ud;

        aeMovieLayerTypeEnum layer_type = ae_get_movie_layer_data_type( _callbackData->layer );

        switch( layer_type )
        {
        case AE_MOVIE_LAYER_TYPE_PARTICLE:
            {
                Node * particle = reinterpret_node_cast<Node *>(_callbackData->element);

                Detail::updateMatrixProxy( particle, _callbackData->immutable_matrix, _callbackData->matrix, _callbackData->immutable_color, _callbackData->color, _callbackData->opacity );

                AnimationInterface * particle_animation = particle->getAnimation();

                switch( _callbackData->state )
                {
                case AE_MOVIE_STATE_UPDATE_BEGIN:
                    {
                        float time = TIMELINE_SERVICE()
                            ->getTime();

                        particle_animation->setTime( _callbackData->offset * 1000.f );

                        if( _callbackData->loop == AE_TRUE )
                        {
                            if( particle_animation->isPlay() == false )
                            {
                                if( particle_animation->play( time ) == 0 )
                                {
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if( particle_animation->play( time ) == 0 )
                            {
                                return;
                            }
                        }
                    }break;
                case AE_MOVIE_STATE_UPDATE_PROCESS:
                    {
                    }break;
                case AE_MOVIE_STATE_UPDATE_INTERRUPT:
                    {
                        particle_animation->interrupt();
                    }break;
                case AE_MOVIE_STATE_UPDATE_STOP:
                    {
                        particle_animation->stop();
                    }break;
                case AE_MOVIE_STATE_UPDATE_END:
                    {
                        if( _callbackData->loop == AE_FALSE )
                        {
                            particle_animation->stop();
                        }
                    }break;
                case AE_MOVIE_STATE_UPDATE_PAUSE:
                    {
                        particle_animation->pause();
                    }break;
                case AE_MOVIE_STATE_UPDATE_RESUME:
                    {
                        float time = TIMELINE_SERVICE()
                            ->getTime();

                        particle_animation->resume( time );
                    }break;
                default:
                    {
                    }break;
                }
            }break;
        case AE_MOVIE_LAYER_TYPE_VIDEO:
            {
                Surface * surface = reinterpret_node_cast<Surface *>(_callbackData->element);

                AnimationInterface * surface_animation = surface->getAnimation();

                switch( _callbackData->state )
                {
                case AE_MOVIE_STATE_UPDATE_BEGIN:
                    {
                        float time = TIMELINE_SERVICE()
                            ->getTime();

                        if( _callbackData->loop == AE_TRUE && surface_animation->isLoop() == true )
                        {
                            if( surface_animation->isPlay() == false )
                            {
                                if( surface_animation->play( time ) == 0 )
                                {
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if( surface_animation->play( time ) == 0 )
                            {
                                return;
                            }
                        }

						surface_animation->setTime( _callbackData->offset * 1000.f );
                    }break;
                case AE_MOVIE_STATE_UPDATE_PROCESS:
                    {
                    }break;
                case AE_MOVIE_STATE_UPDATE_INTERRUPT:
                    {
                        surface_animation->interrupt();
                    }break;
                case AE_MOVIE_STATE_UPDATE_STOP:
                    {
                        surface_animation->stop();
                    }break;
                case AE_MOVIE_STATE_UPDATE_END:
                    {
                        if( _callbackData->loop == AE_FALSE )
                        {
							surface_animation->stop();
                        }
                    }break;
                case AE_MOVIE_STATE_UPDATE_PAUSE:
                    {
						surface_animation->pause();
                    }break;
                case AE_MOVIE_STATE_UPDATE_RESUME:
                    {
                        float time = TIMELINE_SERVICE()
                            ->getTime();

						surface_animation->resume( time );
                    }break;
                default:
                    {
                    }break;
                }
            }break;
        case AE_MOVIE_LAYER_TYPE_SOUND:
            {
                SurfaceSound * surface = reinterpret_node_cast<SurfaceSound *>(_callbackData->element);

                AnimationInterface * surface_animation = surface->getAnimation();

                surface->setVolume( _callbackData->volume );

                switch( _callbackData->state )
                {
                case AE_MOVIE_STATE_UPDATE_BEGIN:
                    {
                        float time = TIMELINE_SERVICE()
                            ->getTime();

                        surface_animation->setTime( _callbackData->offset * 1000.f );

                        if( _callbackData->loop == AE_TRUE )
                        {
                            if( surface_animation->isPlay() == false )
                            {
                                if( surface_animation->play( time ) == 0 )
                                {
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if( surface_animation->play( time ) == 0 )
                            {
                                return;
                            }
                        }
                    }break;
                case AE_MOVIE_STATE_UPDATE_PROCESS:
                    {
                    }break;
                case AE_MOVIE_STATE_UPDATE_INTERRUPT:
                    {
                        surface_animation->interrupt();
                    }break;
                case AE_MOVIE_STATE_UPDATE_STOP:
                    {
                        surface_animation->stop();
                    }break;
                case AE_MOVIE_STATE_UPDATE_END:
                    {
                        if( _callbackData->loop == AE_FALSE )
                        {
                            surface_animation->stop();
                        }
                    }break;
                case AE_MOVIE_STATE_UPDATE_PAUSE:
                    {
                        surface_animation->pause();
                    }break;
                case AE_MOVIE_STATE_UPDATE_RESUME:
                    {
                        float time = TIMELINE_SERVICE()
                            ->getTime();

                        surface_animation->resume( time );
                    }break;
                default:
                    {
                    }break;
                }
            }break;
        case AE_MOVIE_LAYER_TYPE_SLOT:
            {
                Movie2Slot * node = reinterpret_node_cast<Movie2Slot *>(_callbackData->element);

                Detail::updateMatrixProxy( node, _callbackData->immutable_matrix, _callbackData->matrix, _callbackData->immutable_color, _callbackData->color, _callbackData->opacity );
            }break;
        case AE_MOVIE_LAYER_TYPE_SPRITE:
            {
                ShapeQuadFixed * node = reinterpret_node_cast<ShapeQuadFixed *>(_callbackData->element);

                Detail::updateMatrixProxy( node, _callbackData->immutable_matrix, _callbackData->matrix, _callbackData->immutable_color, _callbackData->color, _callbackData->opacity );
            }break;
        case AE_MOVIE_LAYER_TYPE_TEXT:
            {
                TextField * node = reinterpret_node_cast<TextField *>(_callbackData->element);

                Detail::updateMatrixProxy( node, _callbackData->immutable_matrix, _callbackData->matrix, _callbackData->immutable_color, _callbackData->color, _callbackData->opacity );
            }break;
        case AE_MOVIE_LAYER_TYPE_SOCKET:
            {
                HotSpotPolygon * node = reinterpret_node_cast<HotSpotPolygon *>(_callbackData->element);

                Detail::updateMatrixProxy( node, _callbackData->immutable_matrix, _callbackData->matrix, _callbackData->immutable_color, _callbackData->color, _callbackData->opacity );
            }break;
        default:
            {
            }break;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    struct TrackMatteDesc
    {
        mt::mat4f matrix;
        aeMovieRenderMesh mesh;
        ae_track_matte_mode_t mode;
    };
    //////////////////////////////////////////////////////////////////////////
    static ae_bool_t __movie_composition_track_matte_provider( const aeMovieTrackMatteProviderCallbackData * _callbackData, ae_voidptrptr_t _tmp, ae_voidptr_t _ud )
    {
        (void)_ud;

        TrackMatteDesc * desc = new TrackMatteDesc;

        desc->matrix.from_f12( _callbackData->matrix );
        desc->mesh = *_callbackData->mesh;
        desc->mode = _callbackData->track_matte_mode;

        *_tmp = desc;

        return AE_TRUE;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_composition_track_matte_update( const aeMovieTrackMatteUpdateCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_ud;

        switch( _callbackData->state )
        {
        case AE_MOVIE_STATE_UPDATE_BEGIN:
            {
                TrackMatteDesc * desc = reinterpret_cast<TrackMatteDesc *>(_callbackData->track_matte_data);

                desc->matrix.from_f12( _callbackData->matrix );
                desc->mesh = *_callbackData->mesh;
            }break;
        case AE_MOVIE_STATE_UPDATE_PROCESS:
            {
                TrackMatteDesc * desc = reinterpret_cast<TrackMatteDesc *>(_callbackData->track_matte_data);

                desc->matrix.from_f12( _callbackData->matrix );
                desc->mesh = *_callbackData->mesh;
            }break;
        default:
            break;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_composition_track_matte_deleter( const aeMovieTrackMatteDeleterCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_ud;

        TrackMatteDesc * desc = reinterpret_cast<TrackMatteDesc *>(_callbackData->element);

        delete desc;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_bool_t __movie_composition_shader_provider( const aeMovieShaderProviderCallbackData * _callbackData, ae_voidptrptr_t _sd, ae_voidptr_t _ud )
    {
        (void)_callbackData;
        (void)_sd;
        (void)_ud;

        return AE_TRUE;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_composition_shader_property_update( const aeMovieShaderPropertyUpdateCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_callbackData;
        (void)_ud;

    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_composition_event( const aeMovieCompositionEventCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_callbackData;
        (void)_ud;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_composition_state( const aeMovieCompositionStateCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        Movie2 * m2 = reinterpret_node_cast<Movie2 *>(_ud);

        if( _callbackData->state == AE_MOVIE_COMPOSITION_END )
        {
            m2->end();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_bool_t __movie_composition_extra_interrupt( const aeMovieCompositionExtraInterruptCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_callbackData;

        Movie2 * m2 = reinterpret_node_cast<Movie2 *>(_ud);

        if( m2->checkInterruptElement() == false )
        {
            return AE_FALSE;
        }

        return AE_TRUE;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_bool_t __movie_scene_effect_provider( const aeMovieCompositionSceneEffectProviderCallbackData * _callbackData, ae_voidptrptr_t _sed, ae_voidptr_t _ud )
    {
        (void)_sed;

        Movie2 * m2 = reinterpret_node_cast<Movie2 *>(_ud);

        Layer * parent_layer = Helper::findParentNodeT<Layer *>( m2 );

        mt::vec3f anchor_point;
        anchor_point.from_f2( _callbackData->anchor_point, 0.f );
        parent_layer->setOrigin( anchor_point );

        mt::vec3f position;
        position.from_f2( _callbackData->position, 0.f );
        parent_layer->setLocalPosition( position );

        mt::vec3f scale;
        scale.from_f2( _callbackData->scale, 1.f );
        parent_layer->setScale( scale );

        mt::quatf q;
        q.x = 0.f;
        q.y = 0.f;
        q.z = _callbackData->quaternion[0];
        q.w = _callbackData->quaternion[1];
        float angle = quatzw_to_angle( q );

        parent_layer->setOrientationX( angle );

        ColourValue color( _callbackData->color.r, _callbackData->color.g, _callbackData->color.b, _callbackData->opacity );

        RenderInterface * parent_layer_render = parent_layer->getRender();
        parent_layer_render->setLocalColor( color );

        *_sed = parent_layer;

        return AE_TRUE;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_scene_effect_update( const aeMovieCompositionSceneEffectUpdateCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_ud;

        Layer * parent_layer = reinterpret_node_cast<Layer *>(_callbackData->scene_effect_data);

        mt::vec3f anchor_point;
        anchor_point.from_f2( _callbackData->anchor_point, 0.f );
        parent_layer->setOrigin( anchor_point );

        mt::vec3f position;
        position.from_f2( _callbackData->position, 0.f );
        parent_layer->setLocalPosition( position );

        mt::vec3f scale;
        scale.from_f2( _callbackData->scale, 1.f );
        parent_layer->setScale( scale );

        mt::quatf q;
        q.x = 0.f;
        q.y = 0.f;
        q.z = _callbackData->quaternion[0];
        q.w = _callbackData->quaternion[1];
        float angle = quatzw_to_angle( q );

        parent_layer->setOrientationX( angle );

        ColourValue color( _callbackData->color.r, _callbackData->color.g, _callbackData->color.b, _callbackData->opacity );

        RenderInterface * parent_layer_render = parent_layer->getRender();
        parent_layer_render->setLocalColor( color );
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_bool_t __movie_subcomposition_provider( const aeMovieSubCompositionProviderCallbackData * _callbackData, ae_voidptrptr_t _scd, ae_voidptr_t _ud )
    {
        Movie2 * m2 = reinterpret_node_cast<Movie2 *>(_ud);

        const aeMovieLayerData * layer = _callbackData->layer;
        
        const ae_char_t * layer_name = ae_get_movie_layer_data_name( layer );

        ConstString c_name = Helper::stringizeString( layer_name );

        const Movie2SubCompositionPtr & subcomposition = m2->getSubComposition( c_name );
    
        if( subcomposition == nullptr )
        {
            return AE_FALSE;
        }

        *_scd = subcomposition.get();

        return AE_TRUE;
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_subcomposition_deleter( const aeMovieSubCompositionDeleterCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_callbackData;
        (void)_ud;

        //Empty
    }
    //////////////////////////////////////////////////////////////////////////
    static ae_void_t __movie_subcomposition_state( const aeMovieSubCompositionStateCallbackData * _callbackData, ae_voidptr_t _ud )
    {
        (void)_ud;

        Movie2SubComposition * m2sc = reinterpret_node_cast<Movie2SubComposition *>(_callbackData->subcomposition_data);

        if( _callbackData->state == AE_MOVIE_COMPOSITION_END )
        {
            m2sc->end();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    Movie2::Camera * Movie2::addCamera( const ConstString & _name, const RenderCameraProjectionPtr & _projection, const RenderViewportPtr & _viewport )
    {
        this->addChild( _projection );
        this->addChild( _viewport );

        Camera c;
        c.projection = _projection;
        c.viewport = _viewport;

        MapCameras::iterator it_found = m_cameras.emplace( _name, c ).first;

        Camera * new_camera = &it_found->second;

        return new_camera;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::removeCamera( const ConstString & _name )
    {
        MapCameras::iterator it_found = m_cameras.find( _name );

        if( it_found == m_cameras.end() )
        {
            return false;
        }

        Camera & c = it_found->second;

        c.projection = nullptr;
        c.viewport = nullptr;

        m_cameras.erase( it_found );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasCamera( const ConstString & _name ) const
    {
        MapCameras::const_iterator it_found = m_cameras.find( _name );

        if( it_found == m_cameras.end() )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::getCamera( const ConstString & _name, Camera ** _camera )
    {
        MapCameras::iterator it_found = m_cameras.find( _name );

        if( it_found == m_cameras.end() )
        {
            return false;
        }

        Camera & camera = it_found->second;

        *_camera = &camera;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::_compile()
    {
        if( m_resourceMovie2 == nullptr )
        {
            LOGGER_ERROR( "Movie2::_compile: '%s' can't setup resource"
                , this->getName().c_str()
            );

            return false;
        }

        if( m_resourceMovie2.compile() == false )
        {
            LOGGER_ERROR( "Movie2::_compile '%s' resource %s not compile"
                , m_name.c_str()
                , m_resourceMovie2->getName().c_str()
            );

            return false;
        }

        const aeMovieData * movieData = m_resourceMovie2->getMovieData();

        const aeMovieCompositionData * compositionData = m_resourceMovie2->getCompositionData( m_compositionName );

        if( compositionData == nullptr )
        {
            LOGGER_ERROR( "Movie2::_compile '%s' resource %s not found composition '%s'"
                , m_name.c_str()
                , m_resourceMovie2->getName().c_str()
                , m_compositionName.c_str()
            );

            return false;
        }

        aeMovieCompositionProviders providers;
        ae_clear_movie_composition_providers( &providers );

        providers.camera_provider = &__movie_composition_camera_provider;
        providers.camera_deleter = &__movie_composition_camera_deleter;
        providers.camera_update = &__movie_composition_camera_update;

        providers.node_provider = &Movie2::__movie_composition_node_provider;
        providers.node_deleter = &__movie_composition_node_deleter;
        providers.node_update = &__movie_composition_node_update;

        providers.track_matte_provider = &__movie_composition_track_matte_provider;
        providers.track_matte_update = &__movie_composition_track_matte_update;
        providers.track_matte_deleter = &__movie_composition_track_matte_deleter;

        providers.shader_provider = &__movie_composition_shader_provider;
        providers.shader_property_update = &__movie_composition_shader_property_update;

        providers.composition_event = &__movie_composition_event;
        providers.composition_state = &__movie_composition_state;
        providers.composition_extra_interrupt = &__movie_composition_extra_interrupt;

        providers.scene_effect_provider = &__movie_scene_effect_provider;
        providers.scene_effect_update = &__movie_scene_effect_update;

        providers.subcomposition_provider = &__movie_subcomposition_provider;
        providers.subcomposition_deleter = &__movie_subcomposition_deleter;
        providers.subcomposition_state = &__movie_subcomposition_state;

        const aeMovieComposition * composition = ae_create_movie_composition( movieData, compositionData, AE_TRUE, &providers, this );

        if( composition == nullptr )
        {
            LOGGER_ERROR( "Movie2::_compile '%s' resource %s invalid create composition '%s'"
                , m_name.c_str()
                , m_resourceMovie2->getName().c_str()
                , m_compositionName.c_str()
            );

            return false;
        }

        aeMovieCompositionRenderInfo info;
        ae_calculate_movie_composition_render_info( composition, &info );

        //if( info.max_render_node != 0 )
        //{
        //    m_meshes.resize( info.max_render_node );
        //}

        if( info.max_vertex_count != 0 )
        {
            m_vertices.resize( info.max_vertex_count );
        }

        if( info.max_vertex_count != 0 )
        {
            m_vertices.resize( info.max_vertex_count );
        }

        if( info.max_index_count != 0 )
        {
            m_indices.resize( info.max_index_count );
        }

        bool loop = this->isLoop();

        ae_set_movie_composition_loop( composition, loop ? AE_TRUE : AE_FALSE );

        bool play = this->isPlay();

        if( play == true )
        {
            float timing = this->getTime();

            ae_play_movie_composition( composition, timing * 0.001f );
        }

        m_composition = composition;

        float animationSpeedFactor = this->getAnimationSpeedFactor();
        this->updateAnimationSpeedFactor_( animationSpeedFactor );

        for( MapSubCompositions::value_type & value : m_subCompositions )
        {
            if( value.second->initialize( m_composition ) == false )
            {
                return false;
            }
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_release()
    {
        ae_delete_movie_composition( m_composition );
        m_composition = nullptr;

        m_cameras.clear();

        for( const SurfacePtr & surface : m_surfaces )
        {
            surface->release();
        }

        m_surfaces.clear();

        m_resourceMovie2.release();

        Node::_release();
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::_activate()
    {
        if( Node::_activate() == false )
        {
            return false;
        }

        ae_vector3_t anchorPoint;
        if( ae_get_movie_composition_anchor_point( m_composition, anchorPoint ) == AE_TRUE )
        {
            mt::vec3f origin;

            origin.from_f3( anchorPoint );

            this->setOrigin( origin );
        }

        for( const SurfacePtr & surface : m_surfaces )
        {
            surface->activate();
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_deactivate()
    {
        Node::_deactivate();

        for( const SurfacePtr & surface : m_surfaces )
        {
            surface->deactivate();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_changeParent( Node * _oldParent, Node * _newParent )
    {
        (void)_oldParent;
        (void)_newParent;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_addChild( const NodePtr & _node )
    {
        (void)_node;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_removeChild( const NodePtr & _node )
    {
        (void)_node;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_afterActivate()
    {
        Node::_afterActivate();

        bool autoPlay = this->isAutoPlay();

        if( autoPlay == true )
        {
            float time = TIMELINE_SERVICE()
                ->getTime();

            if( this->play( time ) == 0 )
            {
                LOGGER_ERROR( "Movie2::_afterActivate '%s' resource '%s' auto play return 0"
                    , this->getName().c_str()
                    , this->m_resourceMovie2->getName().c_str()
                );

                return;
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_setLoop( bool _value )
    {
        if( this->isCompile() == false )
        {
            return;
        }

        ae_set_movie_composition_loop( m_composition, _value );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_setTime( float _time )
    {
        if( this->isCompile() == false )
        {
            LOGGER_ERROR( "Movie2::_setTiming '%s' invalid compile"
                , this->getName().c_str()
            );

            return;
        }

        ae_set_movie_composition_time( m_composition, _time * 0.001f );
    }
    //////////////////////////////////////////////////////////////////////////
    float Movie2::_getTime() const
    {
        if( this->isCompile() == false )
        {
            return 0.f;
        }

        float timing = ae_get_movie_composition_time( m_composition );

        return timing * 1000.f;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_setFirstFrame()
    {
        this->setTime( 0.f );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_setLastFrame()
    {
        if( this->isCompile() == false )
        {
            LOGGER_ERROR( "Movie._setLastFrame: '%s' not activate"
                , this->getName().c_str()
            );

            return;
        }

        const aeMovieCompositionData * compositionData = ae_get_movie_composition_composition_data( m_composition );

        float duration = ae_get_movie_composition_data_duration( compositionData );
        float frameDuration = ae_get_movie_composition_data_frame_duration( compositionData );

        this->setTime( duration * 1000.f - frameDuration * 1000.f );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_setAnimationSpeedFactor( float _factor )
    {
        if( this->isCompile() == false )
        {
            return;
        }

        this->updateAnimationSpeedFactor_( _factor );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::updateAnimationSpeedFactor_( float _factor )    
    {
        for( const SurfacePtr & surface : m_surfaces )
        {
            AnimationInterface * animation = surface->getAnimation();

            if( animation == nullptr )
            {
                continue;
            }

            animation->setAnimationSpeedFactor( _factor );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_update( const UpdateContext * _context )
    {
        //if( this->isPlay() == false )
        //{
        //    return;
        //}

        if( m_composition == nullptr )
        {
            return;
        }

        float totalTime = this->calcTotalTime( _context );

        ae_update_movie_composition( m_composition, totalTime * 0.001f );

        for( const SurfacePtr & surface : m_surfaces )
        {
            surface->update( _context );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_render( const RenderContext * _context )
    {
        uint32_t vertex_iterator = 0;
        uint32_t index_iterator = 0;

        RenderVertex2D * vertices_buffer = m_vertices.empty() == false ? &m_vertices.front() : nullptr;
        RenderIndex * indices_buffer = m_indices.empty() == false ? &m_indices.front() : nullptr;

        const mt::mat4f & wm = this->getWorldMatrix();

        ColourValue total_color;
        this->calcTotalColor( total_color );

        float total_color_r = total_color.getR();
        float total_color_g = total_color.getG();
        float total_color_b = total_color.getB();
        float total_color_a = total_color.getA();

        ae_voidptr_t composition_camera_data = ae_get_movie_composition_camera_data( m_composition );
        (void)composition_camera_data;

        ae_uint32_t compute_movie_mesh_iterator = 0;

        aeMovieRenderMesh mesh;
        while( ae_compute_movie_mesh( m_composition, &compute_movie_mesh_iterator, &mesh ) == AE_TRUE )
        {
            Resource * resource_reference = reinterpret_node_cast<Resource *>(mesh.resource_data);

            RenderContext context;

            if( mesh.camera_data != nullptr )
            {
                Movie2::Camera * camera = reinterpret_cast<Movie2::Camera *>(mesh.camera_data);

                context.camera = camera->projection;
                context.viewport = camera->viewport;
                context.transformation = _context->transformation;
                context.scissor = _context->scissor;
                context.target = _context->target;
            }
            else
            {
                context = *_context;
            }

            if( mesh.viewport != nullptr )
            {
                Movie2ScissorPtr scissor = new FactorableUnique<Movie2Scissor>();

                scissor->setViewport( wm, mesh.viewport );

                context.scissor = scissor;
            }
            else
            {
                context.scissor = _context->scissor;
            }

            ColourValue_ARGB total_mesh_color = Helper::makeARGB( total_color_r * mesh.color.r, total_color_g * mesh.color.g, total_color_b * mesh.color.b, total_color_a * mesh.opacity );

            if( mesh.track_matte_data == nullptr )
            {
                switch( mesh.layer_type )
                {
                case AE_MOVIE_LAYER_TYPE_SLOT:
                    {
                        Movie2Slot * node = reinterpret_node_cast<Movie2Slot *>(mesh.element_data);

                        RenderInterface * render = node->getRender();
                        render->renderWithChildren( &context, true );
                    }break;
                case AE_MOVIE_LAYER_TYPE_SOCKET:
                    {
                        //HotSpotPolygon * node = reinterpret_node_cast<HotSpotPolygon *>(mesh.element_data);

                        //RenderInterfacePtr render = node->getRender();

                        //render->render( &state );
                    }break;
                case AE_MOVIE_LAYER_TYPE_SPRITE:
                    {
                        ShapeQuadFixed * node = reinterpret_node_cast<ShapeQuadFixed *>(mesh.element_data);

                        RenderInterface * render = node->getRender();
                        render->renderWithChildren( &context, true );
                    }break;
                case AE_MOVIE_LAYER_TYPE_TEXT:
                    {
                        TextField * node = reinterpret_node_cast<TextField *>(mesh.element_data);

                        RenderInterface * render = node->getRender();
                        render->renderWithChildren( &context, true );
                    }break;
                case AE_MOVIE_LAYER_TYPE_PARTICLE:
                    {
                        Node * node = reinterpret_node_cast<Node *>(mesh.element_data);

                        RenderInterface * render = node->getRender();
                        render->renderWithChildren( &context, true );
                    }break;
                case AE_MOVIE_LAYER_TYPE_SHAPE:
                    {
                        if( mesh.vertexCount == 0 )
                        {
                            continue;
                        }

                        RenderVertex2D * vertices = vertices_buffer + vertex_iterator;
                        vertex_iterator += mesh.vertexCount;

                        for( ae_uint32_t index = 0; index != mesh.vertexCount; ++index )
                        {
                            RenderVertex2D & v = vertices[index];

                            mt::vec3f vp;
                            vp.from_f3( &mesh.position[index][0] );

                            mt::mul_v3_v3_m4( v.position, vp, wm );

                            v.uv[0].x = 0.f;
                            v.uv[0].y = 0.f;
                            v.uv[1].x = 0.f;
                            v.uv[1].y = 0.f;

                            v.color = total_mesh_color;
                        }

                        RenderIndex * indices = indices_buffer + index_iterator;
                        index_iterator += mesh.indexCount;

                        stdex::memorycopy_pod( indices, 0, mesh.indices, mesh.indexCount );

                        EMaterialBlendMode blend_mode = Detail::getMovieBlendMode( mesh.blend_mode );

                        const RenderMaterialInterfacePtr & material = Helper::makeTextureMaterial( nullptr, 0, ConstString::none(), blend_mode, false, false, false );

                        this->addRenderObject( &context, material, nullptr, vertices, mesh.vertexCount, indices, mesh.indexCount, nullptr, false );
                    }break;
                case AE_MOVIE_LAYER_TYPE_SOLID:
                    {
                        if( mesh.vertexCount == 0 )
                        {
                            continue;
                        }

                        RenderVertex2D * vertices = vertices_buffer + vertex_iterator;
                        vertex_iterator += mesh.vertexCount;

                        for( ae_uint32_t index = 0; index != mesh.vertexCount; ++index )
                        {
                            RenderVertex2D & v = vertices[index];

                            mt::vec3f vp;
                            vp.from_f3( &mesh.position[index][0] );

                            mt::mul_v3_v3_m4( v.position, vp, wm );

                            v.uv[0].x = 0.f;
                            v.uv[0].y = 0.f;
                            v.uv[1].x = 0.f;
                            v.uv[1].y = 0.f;

                            v.color = total_mesh_color;
                        }

                        RenderIndex * indices = indices_buffer + index_iterator;
                        index_iterator += mesh.indexCount;

                        stdex::memorycopy_pod( indices, 0, mesh.indices, mesh.indexCount );

                        EMaterialBlendMode blend_mode = Detail::getMovieBlendMode( mesh.blend_mode );

                        const RenderMaterialInterfacePtr & material = Helper::makeTextureMaterial( nullptr, 0, ConstString::none(), blend_mode, false, false, false );

                        this->addRenderObject( &context, material, nullptr, vertices, mesh.vertexCount, indices, mesh.indexCount, nullptr, false );
                    }break;
                case AE_MOVIE_LAYER_TYPE_SEQUENCE:
                case AE_MOVIE_LAYER_TYPE_IMAGE:
                    {
                        if( mesh.vertexCount == 0 )
                        {
                            continue;
                        }

                        ResourceImage * resource_image = reinterpret_node_cast<ResourceImage *>(resource_reference);
                        
                        RenderVertex2D * vertices = vertices_buffer + vertex_iterator;
                        vertex_iterator += mesh.vertexCount;

                        for( ae_uint32_t index = 0; index != mesh.vertexCount; ++index )
                        {
                            RenderVertex2D & v = vertices[index];

                            mt::vec3f vp;
                            const float * vp3 = mesh.position[index];
                            vp.from_f3( vp3 );

                            mt::mul_v3_v3_m4( v.position, vp, wm );

                            v.color = total_mesh_color;
                        }

                        if( mesh.uv_cache_data == AE_NULL )
                        {
                            for( ae_uint32_t index = 0; index != mesh.vertexCount; ++index )
                            {
                                RenderVertex2D & v = vertices[index];

                                mt::vec2f uv;
                                const float * uv2 = mesh.uv[index];
                                uv.from_f2( uv2 );

                                resource_image->correctUVImage( v.uv[0], uv );
                                resource_image->correctUVAlpha( v.uv[1], uv );
                            }
                        }
                        else
                        {
                            const mt::vec2f * uvs = reinterpret_cast<const mt::vec2f *>(mesh.uv_cache_data);

                            for( ae_uint32_t index = 0; index != mesh.vertexCount; ++index )
                            {
                                RenderVertex2D & v = vertices[index];

                                const mt::vec2f & uv = uvs[index];

                                v.uv[0] = uv;
                                v.uv[1] = uv;
                            }
                        }

                        RenderIndex * indices = indices_buffer + index_iterator;
                        index_iterator += mesh.indexCount;

                        stdex::memorycopy_pod( indices, 0, mesh.indices, mesh.indexCount );

                        EMaterialBlendMode blend_mode = Detail::getMovieBlendMode( mesh.blend_mode );

                        const RenderMaterialInterfacePtr & material = Helper::makeImageMaterial( resource_image, ConstString::none(), blend_mode, false, false );

                        this->addRenderObject( &context, material, nullptr, vertices, mesh.vertexCount, indices, mesh.indexCount, nullptr, false );
                    }break;
                case AE_MOVIE_LAYER_TYPE_VIDEO:
                    {
                        if( mesh.vertexCount == 0 )
                        {
                            continue;
                        }

                        Surface * surface = reinterpret_node_cast<Surface *>(mesh.element_data);
                        
                        RenderVertex2D * vertices = vertices_buffer + vertex_iterator;
                        vertex_iterator += mesh.vertexCount;

                        for( ae_uint32_t index = 0; index != mesh.vertexCount; ++index )
                        {
                            RenderVertex2D & v = vertices[index];

                            mt::vec3f vp;
                            vp.from_f3( &mesh.position[index][0] );

                            mt::mul_v3_v3_m4( v.position, vp, wm );

                            mt::vec2f uv;
                            uv.from_f2( &mesh.uv[index][0] );

							surface->correctUV( 0, v.uv[0], uv );
							surface->correctUV( 1, v.uv[1], uv );

                            v.color = total_mesh_color;
                        }

                        RenderIndex * indices = indices_buffer + index_iterator;
                        index_iterator += mesh.indexCount;

                        stdex::memorycopy_pod( indices, 0, mesh.indices, mesh.indexCount );

                        const RenderMaterialInterfacePtr & material = surface->getMaterial();

                        this->addRenderObject( &context, material, nullptr, vertices, mesh.vertexCount, indices, mesh.indexCount, nullptr, false );
                    }break;
                default:
                    break;
                }
            }
            else
            {
                switch( mesh.layer_type )
                {
                case AE_MOVIE_LAYER_TYPE_SEQUENCE:
                case AE_MOVIE_LAYER_TYPE_IMAGE:
                    {
                        if( mesh.vertexCount == 0 )
                        {
                            continue;
                        }

                        const SurfaceTrackMatte * surfaceTrackMatte = reinterpret_node_cast<const SurfaceTrackMatte *>(mesh.element_data);
                        
                        RenderVertex2D * vertices = vertices_buffer + vertex_iterator;
                        vertex_iterator += mesh.vertexCount;

                        const ResourceImagePtr & resourceImage = surfaceTrackMatte->getResourceImage();
                        const ResourceImagePtr & resourceTrackMatteImage = surfaceTrackMatte->getResourceTrackMatteImage();

                        const TrackMatteDesc * track_matte_desc = reinterpret_cast<const TrackMatteDesc *>(mesh.track_matte_data);

                        const aeMovieRenderMesh * track_matte_mesh = &track_matte_desc->mesh;

                        for( uint32_t index = 0; index != mesh.vertexCount; ++index )
                        {
                            RenderVertex2D & v = vertices[index];

                            mt::vec3f vp;
                            vp.from_f3( mesh.position[index] );

                            mt::mul_v3_v3_m4( v.position, vp, wm );

                            mt::vec2f uv;
                            uv.from_f2( &mesh.uv[index][0] );

                            resourceImage->correctUVImage( v.uv[0], uv );

                            mt::vec2f uv_track_matte = mt::calc_point_uv(
                                mt::vec2f( track_matte_mesh->position[0] ), mt::vec2f( track_matte_mesh->position[1] ), mt::vec2f( track_matte_mesh->position[2] ),
                                mt::vec2f( track_matte_mesh->uv[0] ), mt::vec2f( track_matte_mesh->uv[1] ), mt::vec2f( track_matte_mesh->uv[2] ),
                                vp.to_vec2f()
                            );

                            resourceTrackMatteImage->correctUVImage( v.uv[1], uv_track_matte );

                            v.color = total_mesh_color;
                        }

                        RenderIndex * indices = indices_buffer + index_iterator;
                        index_iterator += mesh.indexCount;

                        stdex::memorycopy_pod( indices, 0, mesh.indices, mesh.indexCount );

                        const RenderProgramVariableInterfacePtr & programVariable = surfaceTrackMatte->getProgramVariable();

                        float bb[4];
                        bb[0] = std::numeric_limits<float>::max();
                        bb[1] = std::numeric_limits<float>::max();
                        bb[2] = std::numeric_limits<float>::lowest();
                        bb[3] = std::numeric_limits<float>::lowest();

                        for( uint32_t index = 0; index != track_matte_mesh->vertexCount; ++index )
                        {
                            mt::vec2f uv;
                            uv.from_f2( &track_matte_mesh->uv[index][0] );

                            mt::vec2f uv_correct;
                            resourceTrackMatteImage->correctUVImage( uv_correct, uv );

                            if( bb[0] > uv_correct.x )
                            {
                                bb[0] = uv_correct.x;
                            }

                            if( bb[2] < uv_correct.x )
                            {
                                bb[2] = uv_correct.x;
                            }

                            if( bb[1] > uv_correct.y )
                            {
                                bb[1] = uv_correct.y;
                            }

                            if( bb[3] < uv_correct.y )
                            {
                                bb[3] = uv_correct.y;
                            }
                        }

                        programVariable->setPixelVariableFloats( 0, bb, 4 );

                        const RenderMaterialInterfacePtr & material = surfaceTrackMatte->getMaterial();

                        this->addRenderObject( &context, material, programVariable, vertices, mesh.vertexCount, indices, mesh.indexCount, nullptr, false );
                    }break;
                default:
                    break;
                }
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addSurface( const SurfacePtr & _surface )
    {
        _surface->compile();

        m_surfaces.emplace_back( _surface );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::removeSurface( const SurfacePtr & _surface )
    {
        _surface->release();

        VectorSurfaces::iterator it_found = std::find( m_surfaces.begin(), m_surfaces.end(), _surface );

        m_surfaces.erase( it_found );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addSprite_( uint32_t _index, const ShapeQuadFixedPtr & _sprite )
    {
        m_sprites.emplace( _index, _sprite );
    }
    //////////////////////////////////////////////////////////////////////////
    const ShapeQuadFixedPtr & Movie2::getSprite_( uint32_t _index ) const
    {
        MapSprites::const_iterator it_found = m_sprites.find( _index );

        if( it_found == m_sprites.end() )
        {
            return ShapeQuadFixedPtr::none();
        }

        const ShapeQuadFixedPtr & sprite = it_found->second;

        return sprite;
    }
    //////////////////////////////////////////////////////////////////////////
    const ShapeQuadFixedPtr & Movie2::findSprite( const ConstString & _name ) const
    {
        for( const MapSprites::value_type & value : m_sprites )
        {
            const ShapeQuadFixedPtr & sprite = value.second;

            if( sprite->getName() != _name )
            {
                continue;
            }

            return sprite;
        }

        return ShapeQuadFixedPtr::none();
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasSprite( const ConstString & _name ) const
    {
        for( const MapSprites::value_type & value : m_sprites )
        {
            const ShapeQuadFixedPtr & sprite = value.second;

            if( sprite->getName() != _name )
            {
                continue;
            }

            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addParticle_( uint32_t _index, const NodePtr & _particleEmitter )
    {
        m_particleEmitters.emplace( _index, _particleEmitter );
    }
    //////////////////////////////////////////////////////////////////////////
    const NodePtr & Movie2::getParticle_( uint32_t _index ) const
    {
        MapParticleEmitter2s::const_iterator it_found = m_particleEmitters.find( _index );

        if( it_found == m_particleEmitters.end() )
        {
            return NodePtr::none();
        }

        const NodePtr & particleEmitter = it_found->second;

        return particleEmitter;
    }
    //////////////////////////////////////////////////////////////////////////
    const NodePtr & Movie2::findParticle( const ConstString & _name ) const
    {
        for( const MapParticleEmitter2s::value_type & value : m_particleEmitters )
        {
            const NodePtr & particle = value.second;

            if( particle->getName() != _name )
            {
                continue;
            }

            return particle;
        }

        return NodePtr::none();
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasParticle( const ConstString & _name ) const
    {
        for( const MapParticleEmitter2s::value_type & value : m_particleEmitters )
        {
            const NodePtr & particle = value.second;

            if( particle->getName() != _name )
            {
                continue;
            }

            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addSlot_( uint32_t _index, const Movie2SlotPtr & _slot )
    {
        m_slots.emplace( _index, _slot );
    }
    //////////////////////////////////////////////////////////////////////////
    const Movie2SlotPtr & Movie2::getSlot_( uint32_t _index ) const
    {
        MapSlots::const_iterator it_found = m_slots.find( _index );

        if( it_found == m_slots.end() )
        {
            return Movie2SlotPtr::none();
        }

        const Movie2SlotPtr & slot = it_found->second;

        return slot;
    }
    //////////////////////////////////////////////////////////////////////////
    const Movie2SlotPtr & Movie2::findSlot( const ConstString & _name ) const
    {
        for( const MapSlots::value_type & value : m_slots )
        {
            const Movie2SlotPtr & slot = value.second;

            if( slot->getName() != _name )
            {
                continue;
            }

            return slot;
        }

        return Movie2SlotPtr::none();
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasSlot( const ConstString & _name ) const
    {
        for( const MapSlots::value_type & value : m_slots )
        {
            const Movie2SlotPtr & slot = value.second;

            if( slot->getName() != _name )
            {
                continue;
            }

            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::visitSlots( const VisitorMovie2LayerInterfacePtr & _visitor )
    {
        for( const MapSlots::value_type & value : m_slots )
        {
            _visitor->visitMovieLayer( this, value.first, value.second );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addSocket_( uint32_t _index, const HotSpotPolygonPtr & _hotspot )
    {
        m_sockets.emplace( _index, _hotspot );
    }
    //////////////////////////////////////////////////////////////////////////
    const HotSpotPolygonPtr & Movie2::getSocket_( uint32_t _index ) const
    {
        MapSockets::const_iterator it_found = m_sockets.find( _index );

        if( it_found == m_sockets.end() )
        {
            return HotSpotPolygonPtr::none();
        }

        const HotSpotPolygonPtr & hotspot = it_found->second;

        return hotspot;
    }
    //////////////////////////////////////////////////////////////////////////
    const HotSpotPolygonPtr & Movie2::findSocket( const ConstString & _name ) const
    {
        for( const MapSockets::value_type & value : m_sockets )
        {
            const HotSpotPolygonPtr & socket = value.second;

            if( socket->getName() != _name )
            {
                continue;
            }

            return socket;
        }

        return HotSpotPolygonPtr::none();
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasSocket( const ConstString & _name ) const
    {
        for( const MapSockets::value_type & value : m_sockets )
        {
            const HotSpotPolygonPtr & socket = value.second;

            if( socket->getName() != _name )
            {
                continue;
            }

            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::visitSockets( const VisitorMovie2LayerInterfacePtr & _visitor )
    {
        for( const MapSockets::value_type & value : m_sockets )
        {
            _visitor->visitMovieLayer( this, value.first, value.second );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addText_( uint32_t _index, const TextFieldPtr & _text )
    {
        m_texts.emplace( _index, _text );
    }
    //////////////////////////////////////////////////////////////////////////
    const TextFieldPtr & Movie2::getText_( uint32_t _index ) const
    {
        MapTexts::const_iterator it_found = m_texts.find( _index );

        if( it_found == m_texts.end() )
        {
            return TextFieldPtr::none();
        }

        const TextFieldPtr & text = it_found->second;

        return text;
    }
    //////////////////////////////////////////////////////////////////////////
    const TextFieldPtr & Movie2::findText( const ConstString & _name ) const
    {
        for( const MapTexts::value_type & value : m_texts )
        {
            const TextFieldPtr & text = value.second;

            if( text->getName() != _name )
            {
                continue;
            }

            return text;
        }

        return TextFieldPtr::none();
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie2::hasText( const ConstString & _name ) const
    {
        for( const MapTexts::value_type & value : m_texts )
        {
            const TextFieldPtr & text = value.second;

            if( text->getName() != _name )
            {
                continue;
            }

            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::visitTexts( const VisitorMovie2LayerInterfacePtr & _visitor )
    {
        for( const MapTexts::value_type & value : m_texts )
        {
            _visitor->visitMovieLayer( this, value.first, value.second );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addSubMovieComposition_( const ConstString & _name, const Movie2SubCompositionPtr & _subComposition )
    {
        m_subCompositions.emplace( _name, _subComposition );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::addMatrixProxy_( const MatrixProxyPtr & _matrixProxy )
    {
        m_matrixProxies.emplace_back( _matrixProxy );
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie2::_destroy()
    {
        this->destroyCompositionLayers_();

        Node::_destroy();
    }
}