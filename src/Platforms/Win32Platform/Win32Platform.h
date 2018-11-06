#pragma once

#include "Interface/PlatformInterface.h"

#include "Kernel/ServiceBase.h"

#include "Win32MouseEvent.h"
#include "Win32AlreadyRunningMonitor.h"
#include "Win32FPSMonitor.h"

namespace Mengine
{
    class Win32Platform
        : public ServiceBase<PlatformInterface>
    {
    public:
        Win32Platform();
        ~Win32Platform() override;

    public:
        bool _initializeService() override;
        void _finalizeService() override;

    public:
        void update() override;
        void stopPlatform() override;

    public:
        void setIcon( uint32_t _icon ) override;
        uint32_t getIcon() const override;

        void setProjectTitle( const WString & _projectTitle ) override;
        const WString & getProjectTitle() const override;

    public:
        bool createWindow( const Resolution & _resolution, bool _fullscreen ) override;
        Pointer getWindowHandle() const override;

    protected:
        ATOM registerClass_( WNDPROC _wndProc, int _clsExtra, int _wndExtra
            , HINSTANCE _hInstance, DWORD _hIcon, HBRUSH _hbrBackground
            , const WChar * _className );

    public:
        const Tags & getPlatformTags() const override;
        bool hasPlatformTag( const ConstString & _tag ) const override;
        bool hasTouchpad() const override;

    public:
        bool getDesktopResolution( Resolution & _resolution ) const override;

        bool getNoFullscreen() const override;
        bool getAlwaysFullscreen() const override;

        size_t getCurrentPath( WChar * _path, size_t _len ) const override;
        size_t getUserPath( WChar * _path, size_t _len ) const override;

        void minimizeWindow() override;

        void setCursorPosition( const mt::vec2f & _pos ) override;

        void showKeyboard() override;
        void hideKeyboard() override;

        void notifyWindowModeChanged( const Resolution & _resolution, bool _fullscreen ) override;
        void notifyVsyncChanged( bool _vsync ) override;
        void notifyCursorModeChanged( bool _mode ) override;
        bool notifyCursorIconSetup( const ConstString & _name, const FilePath & _path, const MemoryInterfacePtr & _buffer ) override;

    public:
        void onEvent( const ConstString & _event, const MapWParams & _params ) override;

    public:
        float getJoystickAxis( uint32_t _index ) const override;

    public:
        size_t getShortPathName( const WString & _path, WChar * _short, size_t _len ) const override;

    public:
        void getMaxClientResolution( Resolution & _resolution ) const override;

    public:
        bool openUrlInDefaultBrowser( const WString & _url ) override;

    public:
        bool existDirectory( const WString & _path ) const override;
        bool createDirectory( const WString & _path ) override;

    public:
        bool existFile( const WChar * _path ) override;
        bool removeFile( const WChar * _path ) override;

    public:
        uint64_t getFileTime( const WString & _path ) const override;

    public:
        bool createDirectoryUserPicture( const WString & _path, const WString & _file, const void * _data, size_t _size ) override;
        bool createDirectoryUserMusic( const WString & _path, const WString & _file, const void * _data, size_t _size ) override;

    protected:
        bool createDirectoryUser_( const WChar * _userPath, const WString & _path, const WString & _file, const void * _data, size_t _size );

    protected:
        bool getErrorMessage( uint32_t _messageId, WString & _out ) const override;

    protected:
        void sleep( uint32_t _ms ) override;
        bool cmd( const WString & _command ) override;

    protected:
        bool setProcessDPIAware() override;

    public:
        LRESULT wndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
        bool wndProcInput( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & _result );

    protected:
        bool calcCursorPosition_( mt::vec2f & _point ) const;
        DWORD getWindowStyle_( bool _fullsreen ) const;
        RECT getWindowsRect_( const Resolution & _resolution, bool _fullsreen ) const;

    protected:
        void setActive_( bool _active );

    protected:
        HINSTANCE m_hInstance;

        HWND m_hWnd;

        Win32AlreadyRunningMonitor * m_alreadyRunningMonitor;

        Win32FPSMonitor * m_fpsMonitor;

        Win32MouseEvent m_mouseEvent;

        uint32_t m_icon;
        WString m_projectTitle;

        Tags m_platformTags;
        Resolution m_windowResolution;

        float m_activeFrameTime;
        float m_inactiveFrameTime;

        bool m_active;
        bool m_update;
        bool m_close;

        float m_pauseUpdatingTime;

        bool m_vsync;

        bool m_cursorInArea;
        bool m_clickOutArea;
        bool m_cursorMode;

        HCURSOR m_cursor;

        typedef Map<ConstString, HCURSOR> MapCursors;
        MapCursors m_cursors;

        bool m_lastMouse;
        int m_lastMouseX;
        int m_lastMouseY;

        bool m_isDoubleClick;
        bool m_touchpad;
    };
}