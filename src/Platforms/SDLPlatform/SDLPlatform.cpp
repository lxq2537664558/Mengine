#include "Config/Config.h"

#include "SDLPlatform.h"

#include "Interface/LoggerInterface.h"
#include "Interface/FileServiceInterface.h"
#include "Interface/UnicodeSystemInterface.h"
#include "Interface/InputServiceInterface.h"
#include "Interface/RenderServiceInterface.h"
#include "Interface/OptionsInterface.h"
#include "Interface/TimeServiceInterface.h"

#ifdef WIN32
#	include "Environment/Windows/WindowsIncluder.h"
#endif

#include "SDLDynamicLibrary.h"

#include "Kernel/FileLogger.h"
#include "Kernel/IniUtil.h"
#include "Kernel/FilePath.h"
#include "Kernel/FilePathHelper.h"
#include "Kernel/AssertionNotImplemented.h"
#include "Kernel/AssertionFactory.h"
#include "Kernel/AssertionMemoryPanic.h"

#include "Kernel/FactorableUnique.h"
#include "Kernel/FactoryDefault.h"
#include "Kernel/FactoryPool.h"

#include "Kernel/Logger.h"
#include "Kernel/Document.h"

#ifdef __APPLE__
#	include "TargetConditionals.h"
#endif

#include <cstdio>
#include <clocale>

#include <ctime>
#include <algorithm>

#include <sstream>
#include <iomanip>

#include <sys/stat.h>


#ifndef SDL_IPHONE_MAX_GFORCE
#define SDL_IPHONE_MAX_GFORCE 5.0f
#endif

//////////////////////////////////////////////////////////////////////////
SERVICE_FACTORY( Platform, Mengine::SDLPlatform );
//////////////////////////////////////////////////////////////////////////
namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    SDLPlatform::SDLPlatform()
        : m_window( nullptr )
        , m_accelerometer( nullptr )
        , m_glContext( nullptr )
        , m_sdlInput( nullptr )
        , m_icon( 0 )
        , m_shouldQuit( false )
        , m_running( false )
        , m_pause( false )
        , m_touchpad( false )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    SDLPlatform::~SDLPlatform()
    {
    }
    //////////////////////////////////////////////////////////////////////////    
    size_t SDLPlatform::getCurrentPath( Char * _path ) const
    {
#ifdef WIN32
        WChar unicode_path[MENGINE_MAX_PATH];
        DWORD len = (DWORD)::GetCurrentDirectory( MENGINE_MAX_PATH, unicode_path );

        if( len == 0 )
        {
            return 0;
        }

        unicode_path[len] = L'/';
        unicode_path[len + 1] = L'\0';

        Helper::pathCorrectBackslashToW( unicode_path, unicode_path );

        size_t pathLen;
        if( Helper::unicodeToUtf8( unicode_path, _path, MENGINE_MAX_PATH, &pathLen ) == false )
        {
            return 0;
        }

        return pathLen;
#elif TARGET_OS_IPHONE
        const char deploy_ios_data[] = "deploy-ios-data/";
        strcpy( _path, deploy_ios_data );

        return sizeof( deploy_ios_data ) - 1;
#else  
        _path[0] = L'\0';

        return 0;
#endif
    }
    //////////////////////////////////////////////////////////////////////////
    size_t SDLPlatform::getUserPath( Char * _path ) const
    {
        bool developmentMode = HAS_OPTION( "dev" );
        bool roamingMode = HAS_OPTION( "roaming" );
        bool noroamingMode = HAS_OPTION( "noroaming" );

        FilePath userPath;
        if( (developmentMode == true && roamingMode == false) || noroamingMode == true )
        {
            Char currentPath[MENGINE_MAX_PATH];
            size_t currentPathLen = this->getCurrentPath( currentPath );

            if( MENGINE_MAX_PATH <= currentPathLen + 5 )
            {
                return 0;
            }

            strcpy( _path, currentPath );
            strcat( _path, "User" );
            strcat( _path, "/" );

            size_t pathLen = strlen( _path );

            return pathLen;
        }
        else
        {
            String Project_Company = CONFIG_VALUE( "Project", "Company", "NONAME" );
            String Project_Name = CONFIG_VALUE( "Project", "Name", "UNKNOWN" );

            char * sdl_prefPath = SDL_GetPrefPath( Project_Company.c_str(), Project_Name.c_str() );

            size_t sdl_prefPathLen = strlen( sdl_prefPath );

            if( sdl_prefPathLen >= MENGINE_MAX_PATH )
            {
                SDL_free( sdl_prefPath );

                return 0;
            }

            Helper::pathCorrectBackslashToA( _path, sdl_prefPath );

            SDL_free( sdl_prefPath );

            return sdl_prefPathLen;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    float SDLPlatform::getJoystickAxis( uint32_t _index ) const
    {
        if( m_accelerometer == nullptr )
        {
            return 0.f;
        }

        float axis = SDL_JoystickGetAxis( m_accelerometer, _index );

        const float inv_maxint32f = 1.f / 32767.f;

        axis *= inv_maxint32f;

#ifdef TARGET_OS_IPHONE
        axis *= SDL_IPHONE_MAX_GFORCE;
#endif
        return axis;
    }
    //////////////////////////////////////////////////////////////////////////
    size_t SDLPlatform::getShortPathName( const Char * _path, Char * _short, size_t _len ) const
    {
        size_t pathSize = strlen( _path );

        if( _len == pathSize )
        {
            strcpy( _short, _path );
        }

        return pathSize;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::getMaxClientResolution( Resolution & _resolution ) const
    {
        SDL_Rect rect;
        SDL_GetDisplayBounds( 0, &rect );

        LOGGER_WARNING( "SDLPlatform::getMaxClientResolution w %d h %d"
            , rect.w
            , rect.h
        );

        _resolution = Resolution( (uint32_t)rect.w, (uint32_t)rect.h );
    }
    //////////////////////////////////////////////////////////////////////////
    static void MySDL_LogOutputFunction( void * _userdata, int _category, SDL_LogPriority _priority, const char * _message )
    {
        (void)_userdata;
        (void)_category;

        EMessageLevel level = LM_INFO;

        switch( _priority )
        {
        case SDL_LOG_PRIORITY_VERBOSE:
            level = LM_INFO;
            break;
        case SDL_LOG_PRIORITY_DEBUG:
            level = LM_WARNING;
            break;
        case SDL_LOG_PRIORITY_INFO:
            level = LM_INFO;
            break;
        case SDL_LOG_PRIORITY_WARN:
            level = LM_WARNING;
            break;
        case SDL_LOG_PRIORITY_ERROR:
            level = LM_ERROR;
            break;
        case SDL_LOG_PRIORITY_CRITICAL:
            level = LM_CRITICAL;
            break;
        default:
            break;
        }

        size_t messageLen = strlen( _message );

        LOGGER_SERVICE()
            ->logMessage( level, 0, _message, messageLen );
    }
    //////////////////////////////////////////////////////////////////////////
    static int RemoveMouse_EventFilter( void *userdata, SDL_Event * event )
    {
        (void)userdata;

        switch( event->type )
        {
        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            return 0;
        default: break;
        };

        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::_initializeService()
    {
        if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 )
        {
            LOGGER_CRITICAL( "SDL initialization failed" );
            return false;
        }

        const Char* sdlPlatform = SDL_GetPlatform();
        const int sdlRam = SDL_GetSystemRAM();

        if( strcmp( sdlPlatform, "Windows" ) == 0 )
        {
            m_touchpad = false;
            m_platformName.addTag( Helper::stringizeString( "PC" ) );
        }
        else if( strcmp( sdlPlatform, "Mac OS X" ) == 0 )
        {
            m_touchpad = false;
            m_platformName.addTag( Helper::stringizeString( "MAC" ) );
        }
        else if( strcmp( sdlPlatform, "Android" ) == 0 )
        {
            m_touchpad = true;
            m_platformName.addTag( Helper::stringizeString( "ANDROID" ) );
            SDL_SetEventFilter( RemoveMouse_EventFilter, nullptr );
        }
        else if( strcmp( sdlPlatform, "iOS" ) == 0 )
        {
            m_touchpad = true;
            m_platformName.addTag( Helper::stringizeString( "IOS" ) );
            SDL_SetEventFilter( RemoveMouse_EventFilter, nullptr );
        }

        if( HAS_OPTION( "touchpad" ) == true )
        {
            m_touchpad = true;
        }

        LOGGER_WARNING( "Device info:"
        );

        LOGGER_WARNING( "Platform: %s"
            , sdlPlatform
        );

        LOGGER_WARNING( "RAM: %d MB"
            , sdlRam
        );

        SDL_LogSetOutputFunction( &MySDL_LogOutputFunction, nullptr );
        SDL_LogSetAllPriority( SDL_LOG_PRIORITY_WARN );


        m_sdlInput = new FactorableUnique<SDLInput>();

        if( m_sdlInput->initialize() == false )
        {
            return false;
        }

        // Search accelerometer device among joysticks
        int numJoysticks = SDL_NumJoysticks();

        for( int deviceIndex = 0; deviceIndex < numJoysticks; deviceIndex++ )
        {
            SDL_Joystick * joystick = SDL_JoystickOpen(deviceIndex);

            if (!joystick)
            {
                continue;
            }

            const Char * joystickName = SDL_JoystickName(joystick);

            bool isAccelerometer = false;

            if( strcmp( joystickName, "Android Accelerometer" ) == 0 )
            {
                isAccelerometer = true;
            }
            else if( strcmp( joystickName, "IOS Accelerometer" ) == 0 )
            {
                isAccelerometer = true;
            }

            if (isAccelerometer)
            {
                LOGGER_WARNING("Accelerometer found: %s"
                    , joystickName
                );

                m_accelerometer = joystick;
                break;
            }
        }

        if ( m_accelerometer == nullptr )
        {
            LOGGER_WARNING("Accelerometer not found"
            );
        }

        m_factoryDynamicLibraries = new FactoryPool<SDLDynamicLibrary, 8>();

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::_finalizeService()
    {
        this->destroyWindow_();

        SDL_Quit();

        if( m_sdlInput != nullptr )
        {
            m_sdlInput->finalize();
            m_sdlInput = nullptr;
        }

        m_platformName.clear();

        MENGINE_ASSERTION_FACTORY_EMPTY( m_factoryDynamicLibraries );

        m_factoryDynamicLibraries = nullptr;

        if (SDL_JoystickGetAttached(m_accelerometer)) {
            SDL_JoystickClose(m_accelerometer);
            m_accelerometer = nullptr;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::update()
    {
        TIME_SERVICE()
            ->resetDeltaTime();

        while( true )
        {
            float frameTime = TIME_SERVICE()
                ->getDeltaTime();

            if( m_pause == true )
            {
                SDL_Delay( 100 );

                continue;
            }

            bool quitRequest = this->processEvents();

            if( quitRequest == true )
            {
                break;
            }

            bool updating = APPLICATION_SERVICE()
                ->beginUpdate();

            if( updating == true )
            {
                APPLICATION_SERVICE()
                    ->tick( frameTime );

                SDL_Delay( 0 );
            }
            else
            {
                SDL_Delay( 100 );
            }

            if( APPLICATION_SERVICE()
                ->isFocus() == true )
            {
                if( APPLICATION_SERVICE()
                    ->render() == true )
                {
                    APPLICATION_SERVICE()
                        ->flush();

                    SDL_GL_SwapWindow( m_window );
                }
            }
            else
            {
                SDL_Delay( 100 );
            }

            APPLICATION_SERVICE()
                ->endUpdate();

            SDL_Delay( 1 );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::openUrlInDefaultBrowser( const Char * _url )
    {
        MENGINE_UNUSED( _url );

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::stopPlatform()
    {
        m_running = false;
        m_shouldQuit = true;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::setIcon( uint32_t _icon )
    {
        m_icon = _icon;
    }
    //////////////////////////////////////////////////////////////////////////
    uint32_t SDLPlatform::getIcon() const
    {
        return m_icon;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::setProjectTitle( const Char * _projectTitle )
    {
        m_projectTitle = _projectTitle;
    }
    //////////////////////////////////////////////////////////////////////////
    size_t SDLPlatform::getProjectTitle( Char * _projectTitle ) const
    {
        strcpy( _projectTitle, m_projectTitle.c_str() );

        String::size_type projectTitleLen = m_projectTitle.size();

        return (size_t)projectTitleLen;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::createWindow( const Resolution & _resolution, bool _fullscreen )
    {
        LOGGER_WARNING( "SDLPlatform::createWindow w %d h %d fullscreen %d"
            , _resolution.getWidth()
            , _resolution.getHeight()
            , _fullscreen
        );

        if( this->createWindow_( _resolution, _fullscreen ) == false )
        {
            return false;
        }

        m_glContext = SDL_GL_CreateContext( m_window );

        if( m_glContext == nullptr )
        {
            SDL_DestroyWindow( m_window );
            m_window = nullptr;

            return false;
        }

        int dw;
        int dh;
        SDL_GL_GetDrawableSize( m_window, &dw, &dh );

        m_sdlInput->updateSurfaceResolution( static_cast<float>(dw),
            static_cast<float>(dh) );

        ;

#if TARGET_OS_IPHONE
        APPLICATION_SERVICE()
            ->changeWindowResolution( Resolution( dw, dh ) );
#endif

#if defined(__ANDROID__)
        APPLICATION_SERVICE()
            ->changeWindowResolution( Resolution( dw, dh ) );
#endif

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    Pointer SDLPlatform::getWindowHandle() const
    {
        return m_window;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::hasPlatformTag( const ConstString & _tag ) const
    {
        return m_platformName.hasTag( _tag );
    }
    //////////////////////////////////////////////////////////////////////////
    const Tags & SDLPlatform::getPlatformTags() const
    {
        return m_platformName;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::hasTouchpad() const
    {
        return m_touchpad;
    }
    //////////////////////////////////////////////////////////////////////////
    DynamicLibraryInterfacePtr SDLPlatform::loadDynamicLibrary( const Char * _dynamicLibraryName )
    {
        LOGGER_INFO( "load dynamic library '%s'"
            , _dynamicLibraryName
        );

        SDLDynamicLibraryPtr dynamicLibrary = m_factoryDynamicLibraries->createObject( MENGINE_DOCUMENT_FUNCTION );

        MENGINE_ASSERTION_MEMORY_PANIC( dynamicLibrary, nullptr )("can't create dynamic library '%s'"
            , _dynamicLibraryName
            );

        dynamicLibrary->setName( _dynamicLibraryName );

        if( dynamicLibrary->load() == false )
        {
            LOGGER_ERROR( "can't load '%s' plugin [invalid load]"
                , _dynamicLibraryName
            );

            return nullptr;
        }

        return dynamicLibrary;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::getDesktopResolution( Resolution & _resolution ) const
    {
#if TARGET_OS_IPHONE
        if( m_window == nullptr )
        {
            return false;
        }

        int dw;
        int dh;
        SDL_GL_GetDrawableSize( m_window, &dw, &dh );

        _resolution.setWidth( static_cast<uint32_t>(dw) );
        _resolution.setHeight( static_cast<uint32_t>(dh) );
#elif defined(__ANDROID__)
        if( m_window == nullptr )
        {
            return false;
        }

        int dw;
        int dh;
        SDL_GL_GetDrawableSize( m_window, &dw, &dh );

        _resolution.setWidth( static_cast<uint32_t>(dw) );
        _resolution.setHeight( static_cast<uint32_t>(dh) );
#else
        SDL_DisplayMode dm;
        if( SDL_GetDesktopDisplayMode( 0, &dm ) != 0 )
        {
            LOGGER_ERROR( "SDLPlatform::getDesktopResolution failed %s"
                , SDL_GetError()
            );

            return false;
        }

        _resolution.setWidth( static_cast<uint32_t>(dm.w) );
        _resolution.setHeight( static_cast<uint32_t>(dm.h) );
#endif

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::getNoFullscreen() const
    {
        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::getAlwaysFullscreen() const
    {
#if TARGET_OS_IPHONE
        return true;
#elif defined(__ANDROID__)
        return true;
#endif

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::setProcessDPIAware()
    {
        MENGINE_ASSERTION_NOT_IMPLEMENTED();

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::minimizeWindow()
    {
        SDL_MinimizeWindow( m_window );
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::setCursorPosition( const mt::vec2f & _pos )
    {
        Resolution resolution;
        if( this->getDesktopResolution( resolution ) == false )
        {
            return;
        }

        uint32_t width = resolution.getWidth();
        uint32_t height = resolution.getHeight();

        const int wndPosX = static_cast<int>(_pos.x * width);
        const int wndPosY = static_cast<int>(_pos.y * height);

        // ! This function generates a mouse motion event !
        SDL_WarpMouseInWindow( m_window, wndPosX, wndPosY );
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::showKeyboard()
    {
        SDL_StartTextInput();
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::hideKeyboard()
    {
        SDL_StopTextInput();
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::notifyWindowModeChanged( const Resolution & _resolution, bool _fullscreen )
    {
        if( m_window == nullptr )
        {
            return;
        }

        Uint32 flags = SDL_GetWindowFlags( m_window );

        if( _fullscreen == true && !(flags & SDL_WINDOW_FULLSCREEN) )
        {
            this->changeWindow_( _resolution, _fullscreen );
        }
        else if( _fullscreen == false && (flags & SDL_WINDOW_FULLSCREEN) )
        {
            this->changeWindow_( _resolution, _fullscreen );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::notifyVsyncChanged( bool _vsync )
    {
        MENGINE_UNUSED( _vsync );

        if( _vsync == false )
        {
            SDL_GL_SetSwapInterval( 0 );
        }
        else
        {
            SDL_GL_SetSwapInterval( 1 );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::notifyCursorModeChanged( bool _mode )
    {
        if( _mode == true )
        {
            SDL_ShowCursor( SDL_ENABLE );
        }
        else
        {
            SDL_ShowCursor( SDL_DISABLE );
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::notifyCursorIconSetup( const ConstString & _name, const FilePath & _path, const MemoryInterfacePtr & _buffer )
    {
        MENGINE_UNUSED( _name );
        MENGINE_UNUSED( _path );
        MENGINE_UNUSED( _buffer );
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::onEvent( const ConstString & _event, const MapWParams & _params )
    {
        MENGINE_UNUSED( _event );
        MENGINE_UNUSED( _params );
    }
    //////////////////////////////////////////////////////////////////////////
    static bool s_createDurectoryFullpath( const Char * _fullpath )
    {
#if defined(__APPLE__)
        int status = mkdir( _fullpath, 0700 );

        if( status != 0 )
        {
            LOGGER_WARNING( "SDLPlatform::createDirectory '%s' alredy exists"
                , _fullpath
            );

            return false;
        }

#elif defined(__ANDROID__)
        int status = mkdir( _fullpath, 0700 );

        if( status != 0 )
        {
            LOGGER_WARNING( "SDLPlatform::createDirectory '%s' alredy exists"
                , _fullpath
            );

            return false;
        }

#elif defined(WIN32)
        WChar unicode_fullpath[MENGINE_MAX_PATH];
        Helper::utf8ToUnicode( _fullpath, unicode_fullpath, MENGINE_MAX_PATH );

        BOOL successful = CreateDirectoryW( unicode_fullpath, NULL );

        if( successful == FALSE )
        {
            DWORD err = GetLastError();

            switch( err )
            {
            case ERROR_ALREADY_EXISTS:
                {
                    LOGGER_WARNING( "SDLPlatform::createDirectory '%s' alredy exists"
                        , _fullpath
                    );

                    return false;
                }break;
            case ERROR_PATH_NOT_FOUND:
                {
                    LOGGER_WARNING( "SDLPlatform::createDirectory '%s' not found"
                        , _fullpath
                    );

                    return false;
                }break;
            default:
                {
                    LOGGER_WARNING( "SDLPlatform::createDirectory '%s' unknown error %d"
                        , _fullpath
                        , err
                    );
                }break;
            }

            return false;
        }
#endif

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    static bool s_isDirectoryFullpath( const Char * _fullpath )
    {
        struct stat sb;
        if( stat( _fullpath, &sb ) == 0 && ((sb.st_mode)& S_IFMT) == S_IFDIR )
        {
            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::existDirectory( const Char * _path ) const
    {
        Char fullPath[MENGINE_MAX_PATH];
        Helper::pathCorrectBackslashToA( fullPath, _path );

        Helper::pathRemoveFileSpecA( fullPath );

        size_t len = strlen( fullPath );

        if( len == 0 )	// current dir
        {
            return true;
        }

        if( fullPath[len - 1] == ':' )	// root dir
        {
            return true;	// let it be
        }

        bool exist = s_isDirectoryFullpath( fullPath );

        return exist;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::createDirectory( const Char * _path )
    {
        Char fullPath[MENGINE_MAX_PATH];
        Helper::pathCorrectBackslashToA( fullPath, _path );

        Helper::pathRemoveFileSpecA( fullPath );

        if( s_isDirectoryFullpath( fullPath ) == true )
        {
            return true;
        }

        Helper::pathRemoveBackslashA( fullPath );

        VectorString paths;

        for( ;; )
        {
            paths.push_back( fullPath );

            if( Helper::pathRemoveFileSpecA( fullPath ) == false )
            {
                break;
            }

            Helper::pathRemoveBackslashA( fullPath );

            if( s_isDirectoryFullpath( fullPath ) == true )
            {
                break;
            }
        }

        for( VectorString::reverse_iterator
            it = paths.rbegin(),
            it_end = paths.rend();
            it != it_end;
            ++it )
        {
            const String & path = *it;

            if( s_createDurectoryFullpath( path.c_str() ) == false )
            {
                return false;
            }
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::existFile( const Char * _path )
    {
        Char userPath[MENGINE_MAX_PATH];
        this->getUserPath( userPath );

        Char pathCorrect[MENGINE_MAX_PATH];
        Helper::pathCorrectBackslashToA( pathCorrect, _path );

        Char fullPath[MENGINE_MAX_PATH];
        strcpy( fullPath, userPath );
        strcat( fullPath, pathCorrect );

        struct stat sb;
        if( stat( fullPath, &sb ) == 0 && ((sb.st_mode)& S_IFMT) != S_IFDIR )
        {
            return true;
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::removeFile( const Char * _path )
    {
        Char userPath[MENGINE_MAX_PATH];
        this->getUserPath( userPath );

        Char pathCorrect[MENGINE_MAX_PATH];
        Helper::pathCorrectBackslashToA( pathCorrect, _path );

        Char fullPath[MENGINE_MAX_PATH];
        strcpy( fullPath, userPath );
        strcat( fullPath, pathCorrect );

        int result = ::remove( fullPath );

        if( result != 0 )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
#ifdef WIN32
    //////////////////////////////////////////////////////////////////////////
    static bool s_listDirectoryContents( const WChar * _dir, const WChar * _mask, const WChar * _path, const LambdaFilePath & _lambda )
    {
        {
            WChar sPath[MENGINE_MAX_PATH];
            wsprintf( sPath, L"%s%s%s", _dir, _path, _mask );

            WIN32_FIND_DATA fdFile;
            HANDLE hFind = FindFirstFile( sPath, &fdFile );

            if( hFind != INVALID_HANDLE_VALUE )
            {
                do
                {
                    if( wcscmp( fdFile.cFileName, L"." ) == 0
                        || wcscmp( fdFile.cFileName, L".." ) == 0 )
                    {
                        continue;
                    }

                    if( fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                    {
                        continue;
                    }

                    WChar unicode_filepath[MENGINE_MAX_PATH];
                    wsprintf( unicode_filepath, L"%s%s", _path, fdFile.cFileName );

                    Char utf8_filepath[MENGINE_MAX_PATH];
                    if( Helper::unicodeToUtf8( unicode_filepath, utf8_filepath, MENGINE_MAX_PATH ) == false )
                    {
                        FindClose( hFind );

                        return false;
                    }

                    FilePath fp = Helper::stringizeFilePath( utf8_filepath );

                    _lambda( fp );

                } while( FindNextFile( hFind, &fdFile ) != FALSE );
            }

            FindClose( hFind );
        }

        {
            WChar sPath[MENGINE_MAX_PATH];
            wsprintf( sPath, L"%s%s*.*", _dir, _path );

            WIN32_FIND_DATA fdFile;
            HANDLE hFind = FindFirstFile( sPath, &fdFile );

            if( hFind == INVALID_HANDLE_VALUE )
            {
                return true;
            }

            do
            {
                if( wcscmp( fdFile.cFileName, L"." ) == 0
                    || wcscmp( fdFile.cFileName, L".." ) == 0 )
                {
                    continue;
                }

                if( (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
                {
                    continue;
                }

                WChar nextPath[2048];
                wsprintf( nextPath, L"%s%s/", _path, fdFile.cFileName );

                s_listDirectoryContents( _dir, _mask, nextPath, _lambda );

            } while( FindNextFile( hFind, &fdFile ) != FALSE );

            FindClose( hFind );
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
#endif
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::findFiles( const Char * _base, const Char * _path, const Char * _mask, const LambdaFilePath & _lambda ) const
    {
#ifdef WIN32
        WChar unicode_base[MENGINE_MAX_PATH];
        if( Helper::utf8ToUnicode( _base, unicode_base, MENGINE_MAX_PATH ) == false )
        {
            return false;
        }

        WChar unicode_mask[MENGINE_MAX_PATH];
        if( Helper::utf8ToUnicode( _mask, unicode_mask, MENGINE_MAX_PATH ) == false )
        {
            return false;
        }

        WChar unicode_path[MENGINE_MAX_PATH];
        if( Helper::utf8ToUnicode( _path, unicode_path, MENGINE_MAX_PATH ) == false )
        {
            return false;
        }

        if( s_listDirectoryContents( unicode_base, unicode_mask, unicode_path, _lambda ) == false )
        {
            return false;
        }

        return true;
#else
        MENGINE_ASSERTION_NOT_IMPLEMENTED();

        return false;
#endif
    }
    //////////////////////////////////////////////////////////////////////////
#ifdef WIN32
    //////////////////////////////////////////////////////////////////////////
    static time_t s_FileTimeToUnixTime( const FILETIME * filetime )
    {
        uint32_t a0;			/* 16 bit, low    bits */
        uint32_t a1;			/* 16 bit, medium bits */
        uint32_t a2;			/* 32 bit, high   bits */

        uint32_t carry;		/* carry bit for subtraction */
        int negative;		/* whether a represents a negative value */

                            /* Copy the time values to a2/a1/a0 */
        a2 = filetime->dwHighDateTime;
        a1 = ((uint32_t)filetime->dwLowDateTime) >> 16;
        a0 = ((uint32_t)filetime->dwLowDateTime) & 0xffff;

        /* Subtract the time difference */
        if( a0 >= 32768 )
            a0 -= 32768, carry = 0;
        else
            a0 += (1 << 16) - 32768, carry = 1;

        if( a1 >= 54590 + carry )
            a1 -= 54590 + carry, carry = 0;
        else
            a1 += (1 << 16) - 54590 - carry, carry = 1;

        a2 -= 27111902 + carry;

        /* If a is negative, replace a by (-1-a) */
        negative = (a2 >= ((uint32_t)1) << 31);
        if( negative )
        {
            /* Set a to -a - 1 (a is a2/a1/a0) */
            a0 = 0xffff - a0;
            a1 = 0xffff - a1;
            a2 = ~a2;
        }

        /* Divide a by 10000000 (a = a2/a1/a0), put the rest into r.
        Split the divisor into 10000 * 1000 which are both less than 0xffff. */
        a1 += (a2 % 10000) << 16;
        a2 /= 10000;
        a0 += (a1 % 10000) << 16;
        a1 /= 10000;
        a0 /= 10000;

        a1 += (a2 % 1000) << 16;
        a2 /= 1000;
        a0 += (a1 % 1000) << 16;
        a1 /= 1000;
        a0 /= 1000;

        /* If a was negative, replace a by (-1-a) and r by (9999999 - r) */
        if( negative )
        {
            /* Set a to -a - 1 (a is a2/a1/a0) */
            a0 = 0xffff - a0;
            a1 = 0xffff - a1;
            a2 = ~a2;
        }

        /* Do not replace this by << 32, it gives a compiler warning and it does
        not work. */
        return ((((time_t)a2) << 16) << 16) + ((time_t)a1 << 16) + a0;
    }
#endif
    //////////////////////////////////////////////////////////////////////////
    uint64_t SDLPlatform::getFileTime( const Char * _filePath ) const
    {
#ifdef WIN32	
        WChar unicode_filePath[MENGINE_MAX_PATH];
        Helper::utf8ToUnicode( _filePath, unicode_filePath, MENGINE_MAX_PATH );

        HANDLE handle = ::CreateFile( unicode_filePath, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

        FILETIME creation;
        FILETIME access;
        FILETIME write;

        if( ::GetFileTime( handle, &creation, &access, &write ) == FALSE )
        {
            ::CloseHandle( handle );

            return 0U;
        }

        time_t time = s_FileTimeToUnixTime( &write );

        ::CloseHandle( handle );

        return time;
#else
        return 0U;
#endif		
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::createDirectoryUserPicture( const Char * _path, const Char * _file, const void * _data, size_t _size )
    {
        MENGINE_UNUSED( _path );
        MENGINE_UNUSED( _file );
        MENGINE_UNUSED( _data );
        MENGINE_UNUSED( _size );

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::createDirectoryUserMusic( const Char * _path, const Char * _file, const void * _data, size_t _size )
    {
        MENGINE_UNUSED( _path );
        MENGINE_UNUSED( _file );
        MENGINE_UNUSED( _data );
        MENGINE_UNUSED( _size );

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::getErrorMessage( uint32_t _messageId, Char * _out ) const
    {
        MENGINE_UNUSED( _messageId );
        MENGINE_UNUSED( _out );

        MENGINE_ASSERTION_NOT_IMPLEMENTED();

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::sleep( uint32_t _ms )
    {
        MENGINE_UNUSED( _ms );

        MENGINE_ASSERTION_NOT_IMPLEMENTED();
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::cmd( const Char * _command )
    {
        MENGINE_UNUSED( _command );

        MENGINE_ASSERTION_NOT_IMPLEMENTED();

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::abort()
    {
        ::abort();
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::changeWindow_( const Resolution & _resolution, bool _fullscreen )
    {
#if defined(TARGET_OS_IPHONE)
        RENDER_SYSTEM()
            ->onWindowChangeFullscreen( _fullscreen );
#elif defined(__ANDROID__)
        RENDER_SYSTEM()
            ->onWindowChangeFullscreen( _fullscreen );
#else
        //RENDER_SERVICE()
        //    ->destroyRenderWindow();

        //SDL_Window * old_window = m_window;
        //SDL_HideWindow( old_window );
        //SDL_GL_MakeCurrent( old_window, nullptr );

        RENDER_SERVICE()
            ->onDeviceLostPrepare();

        RENDER_SYSTEM()
            ->onWindowChangeFullscreenPrepare( _fullscreen );

        this->destroyWindow_();
        this->createWindow( _resolution, _fullscreen );

        RENDER_SERVICE()
            ->onDeviceLostRestore();

        RENDER_SYSTEM()
            ->onWindowChangeFullscreen( _fullscreen );
#endif
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::createWindow_( const Resolution & _resolution, bool _fullscreen )
    {
        //SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
        SDL_GL_SetAttribute( SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1 );

        Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

#if defined(TARGET_OS_IPHONE)
        windowFlags |= SDL_WINDOW_FULLSCREEN;
        windowFlags |= SDL_WINDOW_BORDERLESS;
        windowFlags |= SDL_WINDOW_ALLOW_HIGHDPI;

        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

        //SDL_SetHint( SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight" );
        SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );

#elif defined(__ANDROID__)
        //windowFlags |= SDL_WINDOW_FULLSCREEN;

        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

        SDL_SetHint( SDL_HINT_ORIENTATIONS, "Portrait" );
        //SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );

#else
        if( _fullscreen == true )
        {
            windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
#endif

#if defined(TARGET_OS_IPHONE)
        SDL_SetHint( SDL_HINT_IOS_HIDE_HOME_INDICATOR, "1" );

        m_window = SDL_CreateWindow( m_projectTitle.c_str()
            , SDL_WINDOWPOS_UNDEFINED
            , SDL_WINDOWPOS_UNDEFINED
            , -1
            , -1
            , windowFlags );

#elif defined(__ANDROID__)
        m_window = SDL_CreateWindow( m_projectTitle.c_str()
            , SDL_WINDOWPOS_UNDEFINED
            , SDL_WINDOWPOS_UNDEFINED
            , -1
            , -1
            , windowFlags );

#else

        int width = static_cast<int>(_resolution.getWidth());
        int height = static_cast<int>(_resolution.getHeight());

        m_window = SDL_CreateWindow( m_projectTitle.c_str()
            , SDL_WINDOWPOS_CENTERED
            , SDL_WINDOWPOS_CENTERED
            , width
            , height
            , windowFlags
        );
#endif

        if( m_window == nullptr )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::destroyWindow_()
    {
        if( m_glContext != nullptr )
        {
            SDL_GL_DeleteContext( m_glContext );
            m_glContext = nullptr;
        }

        if( m_window != nullptr )
        {
            SDL_DestroyWindow( m_window );
            m_window = nullptr;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool SDLPlatform::processEvents()
    {
        bool quitRequest = m_shouldQuit;

        if( m_shouldQuit == true )
        {
            return true;
        }

        Uint32 windowID = SDL_GetWindowID( m_window );

        SDL_Event sdlEvent;
        while( SDL_PollEvent( &sdlEvent ) != 0 && !quitRequest )
        {
            m_sdlInput->handleEvent( sdlEvent );

            switch( sdlEvent.type )
            {
            case SDL_KEYDOWN:
                {
                    switch( sdlEvent.key.keysym.sym )
                    {
                    case SDLK_RETURN:
                        {
                            if( sdlEvent.key.keysym.mod & KMOD_ALT )
                            {
                                bool fullscreen = APPLICATION_SERVICE()
                                    ->getFullscreenMode();

                                APPLICATION_SERVICE()
                                    ->setFullscreenMode( !fullscreen );
                            }
                        }break;
                    }
                }break;
            case SDL_WINDOWEVENT:
                {
                    if( sdlEvent.window.windowID != windowID )
                    {
                        continue;
                    }

                    SDL_WindowEventID windowEventId = (SDL_WindowEventID)sdlEvent.window.event;

                    switch( windowEventId )
                    {
                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_RESTORED:
                        {
                            this->setActive_( true );
                        }break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        {
                            this->setActive_( false );
                        }break;
                    case SDL_WINDOWEVENT_CLOSE:
                        {
                            SDL_Event newEvent;
                            newEvent = sdlEvent;
                            newEvent.type = SDL_QUIT;

                            SDL_PushEvent( &newEvent );
                        }break;
                    default:
                        break;
                    }
                }break;
            case SDL_QUIT:
                {
                    return true;
                }break;
            }
        }

        return quitRequest;
    }
    //////////////////////////////////////////////////////////////////////////
    void SDLPlatform::setActive_( bool _active )
    {
        APPLICATION_SERVICE()
            ->setFocus( _active );

        INPUT_SERVICE()
            ->onFocus( _active );

        APPLICATION_SERVICE()
            ->turnSound( _active );
    }
}
