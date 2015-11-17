#	pragma once

#   include "Interface/PlayerInterface.h"

#	include "Core/ConstString.h"
#	include "Core/Resolution.h"

#   include "Factory/FactoryStore.h"

#	include "Math/vec3.h"

#	include "pybind/types.hpp"

#   include "MousePickerSystem.h"
#   include "GlobalHandleSystem.h"
#   include "Kernel/ScheduleManager.h"


namespace Menge
{	
    class Game;
	class Scene;
	class Arrow;
	class TextField;

	class Node;
	class Join;

	class RenderViewport;
	class Camera2D;

	class MousePickerSystemInterface;
	class GlobalHandleSystem;

	class ScheduleManagerInterface;

	class EventManager;

	class Player
		: public ServiceBase<PlayerServiceInterface>
	{
	public:
		Player();
		~Player();

	public:
        bool _initialize() override;
		void _finalize() override;

	public:
		bool setCurrentScene( Scene * _scene, bool _destroyOld, const SceneChangeCallbackInterfacePtr & _cb ) override;
		bool restartCurrentScene( const SceneChangeCallbackInterfacePtr & _cb ) override;
		bool removeCurrentScene( const SceneChangeCallbackInterfacePtr & _cb ) override;

		Scene * getCurrentScene() override;

		bool isChangedScene() const override;

	public:
		void updateChangeScene() override;

    protected:        
		void updateSwitchScene_();
		void updateRemoveScene_();
		void updateRestartScene_();

	public:
		void setArrow( Arrow * _arrow ) override;
		Arrow * getArrow() const override;

	public:
		void calcGlobalMouseWorldPosition( const mt::vec2f & _screenPoint, mt::vec2f & _worldPoint ) override;
		void calcGlobalMouseWorldDelta( const mt::vec2f & _screenPoint, const mt::vec2f & _screenDeltha, mt::vec2f & _worldDeltha ) override;

	public:
		ScheduleManagerInterface * createSchedulerManager() override;
		bool destroySchedulerManager( ScheduleManagerInterface * _scheduler ) override;

    public:
        MousePickerSystemInterface * getMousePickerSystem() const override;
        GlobalHandleSystemInterface * getGlobalHandleSystem() const override;

    public:
		ScheduleManagerInterface * getScheduleManager() const override;
		ScheduleManagerInterface * getScheduleManagerGlobal() const override;

	public:
		ScheduleManagerInterface * createTimingManager() override;
		bool destroyTimingManager( ScheduleManagerInterface * _timing ) override;

    public:
		ScheduleManagerInterface * getTimingManager() const override;
		ScheduleManagerInterface * getTimingManagerGlobal() const override;

	public:
		Affectorable * getAffectorable() const override;
		Affectorable * getAffectorableGlobal() const override;

	public:
		void setRenderCamera( RenderCameraInterface * _camera) override;
		const RenderCameraInterface * getRenderCamera() const override;

	public:
		void setRenderViewport( RenderViewportInterface * _viewport ) override;
		const RenderViewportInterface * getRenderViewport() const override;

	public:
		void setRenderClipplane( RenderClipplaneInterface * _clipplane ) override;
		const RenderClipplaneInterface * getRenderClipplane() const override;

    public:
		void initializeRenderResources() override;
		void finalizeRenderResources() override;

    public:
		void tick( float _timing );
		bool update() override;


	public:
		float getTime() const override;

	public:
		void render() override;
		
	public:
		bool handleKeyEvent( const InputKeyEvent & _event ) override;

	public:
		bool handleMouseButtonEvent( const InputMouseButtonEvent & _event ) override;
		bool handleMouseButtonEventBegin( const InputMouseButtonEvent & _event ) override;
		bool handleMouseButtonEventEnd( const InputMouseButtonEvent & _event ) override;
		bool handleMouseMove( const InputMouseMoveEvent & _event ) override;
		bool handleMouseWheel( const InputMouseWheelEvent & _event ) override;

	public:
		void onFocus( bool _focus ) override;

		void onAppMouseLeave( const InputMousePositionEvent & _event ) override;
		void onAppMouseEnter( const InputMousePositionEvent & _event ) override;
		void onAppMousePosition( const InputMousePositionEvent & _event ) override;

		void onFullscreen( const Resolution & _resolution, bool _fullscreen ) override;
		void onFixedContentResolution( const Resolution & _resolution, bool _fixed ) override;
	
	protected:
		void renderArrow_( unsigned int _debugMask );

	private:
		Scene * m_scene;
		Arrow * m_arrow;
		
		Camera2D * m_camera2D;
		RenderViewport * m_viewport2D;

		Camera2D * m_arrowCamera2D;
		
		RenderViewportInterface * m_renderViewport;
		RenderCameraInterface * m_renderCamera;		
		RenderClipplaneInterface * m_renderClipplane;
		RenderTargetInterface * m_renderTarget;

        MousePickerSystem * m_mousePickerSystem;
        GlobalHandleSystem * m_globalHandleSystem;

		ScheduleManagerInterface * m_scheduleManager;
		ScheduleManagerInterface * m_scheduleManagerGlobal;

		typedef stdex::vector<ScheduleManagerInterface *> TVectorUserScheduler;
		TVectorUserScheduler m_schedulers;

        FactoryDefaultStore<ScheduleManager> m_factoryScheduleManager;

		ScheduleManagerInterface * m_timingManager;
		ScheduleManagerInterface * m_timingManagerGlobal;

		typedef stdex::vector<ScheduleManagerInterface *> TVectorUserTiming;
		TVectorUserTiming m_timingers;

        FactoryDefaultStore<ScheduleManager> m_factoryTimingManager;

		Affectorable * m_affectorable;
		Affectorable * m_affectorableGlobal;

		bool m_arrowHided;
		
		Scene * m_switchSceneTo;

		bool m_switchScene;		
		bool m_restartScene;
		bool m_removeScene;

		bool m_destroyOldScene;
				
		SceneChangeCallbackInterfacePtr m_changeSceneCb;
		SceneChangeCallbackInterfacePtr m_removeSceneCb;

		float m_time;

		uint32_t m_fps;

//#	ifndef MENGINE_MASTER_RELEASE
	public:
		void toggleDebugText() override;

	protected:
		uint32_t m_showDebugText;
		TextField * m_debugText;
		Camera2D * m_debugCamera2D;
//#	endif
	};
}
