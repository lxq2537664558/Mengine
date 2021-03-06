#pragma once

#include "Interface/GameServiceInterface.h"
#include "Interface/ApplicationInterface.h"
#include "Interface/AccountInterface.h"
#include "Interface/SoundSystemInterface.h"

#include "Kernel/ServiceBase.h"
#include "Kernel/Scriptable.h"

#include "Kernel/BaseEventation.h"
#include "Kernel/Resolution.h"
#include "Kernel/String.h"

namespace Mengine
{

    //////////////////////////////////////////////////////////////////////////
    class GameService
        : public ServiceBase<GameServiceInterface>
        , public BaseEventation
    {
        DECLARE_EVENTABLE( GameEventReceiver );

    public:
        GameService();
        ~GameService() override;

    public:
        bool _initializeService() override;
        void _finalizeService() override;

    public:
        void run() override;

    public:
        void update() override;
        void tick( const UpdateContext * _context ) override;

    public:
        void render() override;

    public:
        void initializeRenderResources() override;
        void finalizeRenderResources() override;

    public:
        bool loadPersonality() override;

    public:
        void setCursorMode( bool _mode ) override;

    public:
        bool handleKeyEvent( const InputKeyEvent & _event ) override;
        bool handleTextEvent( const InputTextEvent & _event ) override;

    public:
        bool handleMouseButtonEvent( const InputMouseButtonEvent & _event ) override;
        bool handleMouseButtonEventBegin( const InputMouseButtonEvent & _event ) override;
        bool handleMouseButtonEventEnd( const InputMouseButtonEvent & _event ) override;
        bool handleMouseMove( const InputMouseMoveEvent & _event ) override;
        bool handleMouseWheel( const InputMouseWheelEvent & _event ) override;

    public:
        void mousePosition( const InputMousePositionEvent & _event ) override;
        void mouseEnter( const InputMousePositionEvent & _event ) override;
        void mouseLeave( const InputMousePositionEvent & _event ) override;

    public:
        void setFocus( bool _focus ) override;
        void setFullscreen( const Resolution & _resolution, bool _fullscreen ) override;
        void setFixedContentResolution( const Resolution & _resolution, bool _fixed ) override;
        void setFixedDisplayResolution( const Resolution & _resolution, bool _fixed ) override;
        void setRenderViewport( const Viewport & _viewport, const Resolution & _contentResolution ) override;
        void setGameViewport( const Viewport & _viewport, float _aspect ) override;

        bool close() override;

        void userEvent( const ConstString & _id, const MapWParams & _params ) override;

        void turnSound( bool _turn ) override;

    public:
        bool getParam( const ConstString & _paramName, Char * _param ) const override;
        bool hasParam( const ConstString & _paramName ) const override;

    public:
        float getTimeFactor() const override;
        void setTimeFactor( float _timingFactor ) override;

    protected:
        ConstString m_currentPackName;
        String m_currentResourcePath;

        float m_timingFactor;

        FilePath m_iconPath;

        MapParams m_params;

        struct UserEvent
        {
            ConstString id;
            MapWParams params;
        };

        typedef Vector<UserEvent> VectorUserEvents;
        VectorUserEvents m_userEventsAdd;
        VectorUserEvents m_userEvents;
    };
}
