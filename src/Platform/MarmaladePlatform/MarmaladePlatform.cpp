#	include "Config/Config.h"

#	include "MarmaladePlatform.h"

#	include "Interface/LogSystemInterface.h"
#	include "Interface/FileSystemInterface.h"
#	include "Interface/UnicodeInterface.h"
#	include "Interface/InputSystemInterface.h"

#	include <cstdio>
#	include <clocale>

//#	include "FPSMonitor.h"
//#	include "AlreadyRunningMonitor.h"
//#	include "CriticalErrorsMonitor.h"

#	include "Core/FileLogger.h"
#	include "Core/IniUtil.h"

//#	include "resource.h"

#	include <ctime>
#	include <algorithm>

#	include <sstream>
#	include <iomanip>

//#	include <mhook.h>
#	include <s3eDevice.h>
#	include <s3eWindow.h>
#	include <s3e.h>

#ifdef _MSC_VER
#	define snprintf _snprintf
#endif

SERVICE_PROVIDER_EXTERN( ServiceProvider );

SERVICE_EXTERN( Application );
SERVICE_EXTERN( StringizeService );
SERVICE_EXTERN( LoggerService );
SERVICE_EXTERN( MarmaladeLayer );

SERVICE_EXTERN( ArchiveService );

SERVICE_EXTERN( ThreadSystem );
SERVICE_EXTERN( ThreadService );

SERVICE_EXTERN( ParticleSystem2 );
SERVICE_EXTERN( ParticleService2 );

SERVICE_EXTERN( RenderSystem );
SERVICE_EXTERN( RenderSystemES1 );
SERVICE_EXTERN( RenderService );
SERVICE_EXTERN( RenderTextureManager );
SERVICE_EXTERN( RenderMaterialManager );

SERVICE_EXTERN( PhysicSystem );

SERVICE_EXTERN( UnicodeSystem );
SERVICE_EXTERN( UnicodeService );

SERVICE_EXTERN( FileService );

SERVICE_EXTERN( NotificationService );
SERVICE_EXTERN( ScriptService );

SERVICE_EXTERN( SoundSystem );
SERVICE_EXTERN( SilentSoundSystem );
SERVICE_EXTERN( SoundService );

SERVICE_EXTERN( InputService );
SERVICE_EXTERN( CodecService );
SERVICE_EXTERN( PluginService );

SERVICE_EXTERN( ModuleService );
SERVICE_EXTERN( DataService );
SERVICE_EXTERN( MemoryService );
SERVICE_EXTERN( ConfigService );
SERVICE_EXTERN( PrefetcherService );


PLUGIN_EXPORT( MengeImageCodec );
PLUGIN_EXPORT( MengeSoundCodec );
PLUGIN_EXPORT( MengeVideoCodec );
PLUGIN_EXPORT( MengeAmplifier );
PLUGIN_EXPORT( MengeZip );
PLUGIN_EXPORT( MengeLZ4 );
PLUGIN_EXPORT( MengeSpine );
PLUGIN_EXPORT( MengeOggVorbis );
PLUGIN_EXPORT( MengeSpine );

PLUGIN_EXPORT( MarmaladeFileGroupPlugin );

PLUGIN_EXPORT( PathFinder );

namespace Menge
{
    //////////////////////////////////////////////////////////////////////////
	MarmaladePlatform::MarmaladePlatform()
        : m_loggerConsole(nullptr)
        , m_fileLog(nullptr)
        , m_timer(nullptr)
		, m_marmaladeInput(nullptr)
		, m_running(false)
		, m_active(false)
		, m_developmentMode(false)
    {
    }
    //////////////////////////////////////////////////////////////////////////
	MarmaladePlatform::~MarmaladePlatform()
    {
    }
    //////////////////////////////////////////////////////////////////////////    
	size_t MarmaladePlatform::getCurrentPath( WChar * _path, size_t _len ) const
    {
		if( _len > 0 )
		{
			wcscpy( _path, L"" );
		}

        return 0;
    }
    //////////////////////////////////////////////////////////////////////////
	size_t MarmaladePlatform::getShortPathName( const WString & _path, WChar * _short, size_t _len ) const
    {
		size_t pathSize = _path.size();

		if( _len == pathSize )
		{
			wcscpy( _short, _path.c_str() );
		}
		
		return _path.size();
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeFileEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Inititalizing File Service..." );
		       
		m_fileService->registerFileGroupFactory( Helper::stringizeString(m_serviceProvider, "dir"), new FactorableUnique< FactoryDefault<MarmaladeFileGroupDirectory> >() );
        
        // mount root		
        ConstString c_dir = Helper::stringizeString(m_serviceProvider, "dir");
        if( m_fileService->mountFileGroup( ConstString::none(), m_currentPath, c_dir ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "WinApplication::setupFileService: failed to mount application directory %ls"
                , m_currentPath.c_str()
                );

            return false;
        }

        //m_tempPath.clear();
        m_userPath.clear();

        m_userPath += L"User";
        m_userPath += L"/";

        String utf8_userPath;
        if( Helper::unicodeToUtf8( m_serviceProvider, m_userPath, utf8_userPath ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "WinApplication: failed user directory %ls convert to ut8f"
                , m_userPath.c_str()
                );

            return false;
        }

        FilePath userPath = Helper::stringizeString( m_serviceProvider, utf8_userPath );

        // mount user directory
        if( m_fileService->mountFileGroup( Helper::stringizeString(m_serviceProvider, "user"), userPath, Helper::stringizeString(m_serviceProvider, "dir") ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "WinApplication: failed to mount user directory %ls"
                , m_userPath.c_str()
                );

            return false;
        }

		if( m_fileService->existDirectory( Helper::stringizeString(m_serviceProvider, "user"), ConstString::none() ) == false )
		{
			m_fileService->createDirectory( Helper::stringizeString(m_serviceProvider, "user"), ConstString::none() );
		}

        FilePath logFilename = Helper::stringizeString( m_serviceProvider, "log.log" );

        OutputStreamInterfacePtr fileLogInterface = 
            m_fileService->openOutputFile( Helper::stringizeString(m_serviceProvider, "user"), logFilename );

        if( fileLogInterface != nullptr )
        {
            m_fileLog = new FileLogger();

            m_logService->registerLogger( m_fileLog );
        }

#	ifndef MENGINE_MASTER_RELEASE
		ConstString c_dev = Helper::stringizeString( m_serviceProvider, "dev" );
		// mount root		
		if( m_fileService->mountFileGroup( c_dev, ConstString::none(), c_dir ) == false )
		{
			LOGGER_ERROR(m_serviceProvider)( "WinApplication::setupFileService: failed to mount dev directory %ls"
				, m_currentPath.c_str()
				);

			return false;
		}
#   endif

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::getApplicationPath_( const char * _section, const char * _key, ConstString & _path )
	{
		FilePath applicationPath = STRINGIZE_STRING_LOCAL( m_serviceProvider, "application.ini" );

		InputStreamInterfacePtr applicationInputStream = 
			FILE_SERVICE(m_serviceProvider)->openInputFile( ConstString::none(), applicationPath, false );

		if( applicationInputStream == nullptr )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ Invalid open application settings %s"
				, applicationPath.c_str()
				);

			return false;
		}

		IniUtil::IniStore ini;
		if( IniUtil::loadIni( ini, applicationInputStream, m_serviceProvider ) == false )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ Invalid load application settings %s"
				, applicationPath.c_str()
				);

			return false;
		}

		const char * gameIniPath = ini.getSettingValue( _section, _key );

		if( gameIniPath == nullptr )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ Not found Game Path %s"
				, applicationPath.c_str()
				);

			return false;
		}

		_path = Helper::stringizeString( m_serviceProvider, gameIniPath );

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::initializeConfigEngine_()
	{
		LOGGER_WARNING(m_serviceProvider)("Inititalizing Config Manager..." );

		FilePath gameIniPath;
		if( this->getApplicationPath_( "Game", "Path", gameIniPath ) == false )
		{
			return false;
		}

		if( CONFIG_SERVICE(m_serviceProvider)
			->loadConfig( ConstString::none(), gameIniPath ) == false )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ invalid load config %s"				
				, gameIniPath.c_str()
				);

			return false;
		}

		return true;
	}
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeLogEngine_()
    {
        m_loggerConsole = new MarmaladeLogger();

        LOGGER_SERVICE(m_serviceProvider)
			->registerLogger( m_loggerConsole );

        EMessageLevel m_logLevel;

        m_logLevel = LM_LOG;

        String logLevel;
        Helper::s_getOption( " -log:", m_commandLine, &logLevel );

        if( logLevel == "0" )
        {
            m_logLevel = LM_INFO;
        }
        else if ( logLevel == "1" )
        {
            m_logLevel = LM_LOG;
        }
        else if ( logLevel == "2" )
        {
            m_logLevel = LM_WARNING;
        }
        else if ( logLevel == "3" )
        {
            m_logLevel = LM_ERROR;
        }

		LOGGER_SERVICE( m_serviceProvider )
			->setVerboseLevel( m_logLevel );

        if( Helper::s_hasOption( " -verbose ", m_commandLine ) == true )
        {
			LOGGER_SERVICE( m_serviceProvider )
				->setVerboseLevel( LM_MAX );

            LOGGER_INFO(m_serviceProvider)( "Verbose logging mode enabled" );
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeArchiveService_()
    {
        LOGGER_INFO(m_serviceProvider)( "Inititalizing Archive Service..." );

		{
			LOGGER_INFO(m_serviceProvider)( "initialize Zip..." );

			PluginInterface * pluginMengeZip;
			initPluginMengeZip( &pluginMengeZip );
			pluginMengeZip->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeZip );
		}

		{
			LOGGER_INFO(m_serviceProvider)( "initialize LZ4..." );

			PluginInterface * pluginMengeLZ4;
			initPluginMengeLZ4( &pluginMengeLZ4 );
			pluginMengeLZ4->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeLZ4 );
		}

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeRenderEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Initializing Render Service..." );
		
		uint32_t config_gl_version = 2;

		char config_SysGlesVersion[S3E_CONFIG_STRING_MAX] = {0};
		if( s3eConfigGetString( "S3E", "SysGlesVersion", config_SysGlesVersion ) == S3E_RESULT_SUCCESS )
		{
			if( strcmp( config_SysGlesVersion, "1" ) == 0 )
			{
				config_gl_version = 1;
			}
			else if( strcmp( config_SysGlesVersion, "2" ) == 0 )
			{
				config_gl_version = 2;
			}
			else
			{
				LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initializeRenderEngine_ not support OpenGL ES version '%s'"
					, config_SysGlesVersion
					);

				return false;
			}
		}

		RenderSystemInterface * renderSystem = nullptr;

		for( uint32_t i = config_gl_version; i != 0; --i )
		{
			if( i == 2 )
			{
				if( SERVICE_CREATE( RenderSystem, &renderSystem ) == false )
				{
					return false;
				}

				if( SERVICE_REGISTRY( m_serviceProvider, renderSystem ) == false )
				{
					continue;
				}

				if( renderSystem->initialize() == false )
				{
					LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initializeRenderEngine_ Failed to initialize Render System version %d"
						, i
						);

					SERVICE_UNREGISTRY( m_serviceProvider, renderSystem );
					SERVICE_DESTROY( RenderSystem, renderSystem );
					renderSystem = nullptr;
				}

				break;
			}
			else if( i == 1 )
			{
				if( SERVICE_CREATE( RenderSystemES1, &renderSystem ) == false )
				{
					continue;
				}

				if( SERVICE_REGISTRY( m_serviceProvider, renderSystem ) == false )
				{
					continue;
				}

				if( renderSystem->initialize() == false )
				{
					LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initializeRenderEngine_ Failed to initialize Render System version %d"
						, i
						);

					SERVICE_UNREGISTRY( m_serviceProvider, renderSystem );
					SERVICE_DESTROY( RenderSystemES1, renderSystem );
					renderSystem = nullptr;
				}

				break;
			}
		}

		if( renderSystem == nullptr )
		{
			return false;
		}

		m_renderSystem = renderSystem;

        RenderServiceInterface * renderService;
        if( SERVICE_CREATE( RenderService, &renderService ) == false )
        {
            return false;
        }

        if( SERVICE_REGISTRY( m_serviceProvider, renderService ) == false )
        {
            return false;
        }

        m_renderService = renderService;

        if( m_renderService->initialize() == false )
        {
            LOGGER_ERROR(m_serviceProvider)("MarmaladeApplication::initializeRenderEngine_ Failed to initialize Render Engine"
                );

            return false;
        }

		RenderTextureServiceInterface * renderTextureManager;
		if( SERVICE_CREATE( RenderTextureManager, &renderTextureManager) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY( m_serviceProvider, renderTextureManager ) == false )
		{
			return false;
		}

		m_renderTextureManager = renderTextureManager;

		if( m_renderTextureManager->initialize() == false )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeRenderEngine_ Failed to initialize Render Texture Service"
				);

			return false;
		}

		RenderMaterialServiceInterface * renderMaterialManager;
		if( SERVICE_CREATE( RenderMaterialManager, &renderMaterialManager ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY( m_serviceProvider, renderMaterialManager ) == false )
		{
			return false;
		}

		m_renderMaterialManager = renderMaterialManager;

		if( m_renderMaterialManager->initialize() == false )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeRenderEngine_ Failed to initialize Render Material Service"
				);

			return false;
		}

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::getMaxClientResolution( Resolution & _resolution ) const
    {
		int32 width = s3eSurfaceGetInt( S3E_SURFACE_WIDTH );
		int32 height = s3eSurfaceGetInt( S3E_SURFACE_HEIGHT );

		_resolution.setWidth( width );
		_resolution.setHeight( height );
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::_initialize()
    {
        setlocale( LC_CTYPE, "" );

        String scriptInit;
        Helper::s_getOption( " -s:", m_commandLine, &scriptInit );

		if( Helper::s_hasOption( " -dev ", m_commandLine ) == true )
		{
			m_developmentMode = true;
		}

        ServiceProviderInterface * serviceProvider;
        if( SERVICE_CREATE( ServiceProvider, &serviceProvider ) == false )
        {
            return false;
        }

        m_serviceProvider = serviceProvider;

        if( m_serviceProvider->registryService( "PlatformService", this ) == false )
        {
            return false;
        }

        if( this->initializeStringizeService_() == false )
        {
            return false;
        }

		int32 deviceClassID = s3eDeviceGetInt( S3E_DEVICE_CLASS );
		
		switch( deviceClassID )
		{
		case  S3E_DEVICE_CLASS_WINDOWS_GENERIC:
			{
				m_platformName = STRINGIZE_STRING_LOCAL(m_serviceProvider, "WIN");
			}break;
		case S3E_DEVICE_CLASS_OSX_DESKTOP:
			{
				m_platformName = STRINGIZE_STRING_LOCAL(m_serviceProvider, "OSX");
			}break;
		case S3E_DEVICE_CLASS_IPHONE:
			{
				m_platformName = STRINGIZE_STRING_LOCAL(m_serviceProvider, "IOS");
			}break;
		case S3E_DEVICE_CLASS_ANDROID_GENERIC:
			{
				m_platformName = STRINGIZE_STRING_LOCAL(m_serviceProvider, "ANDROID");
			}break;
		default:
			break;
		};
				
        if( this->initializeLogEngine_() == false )
        {
            return false;
        }

        if( this->initializeMarmaladeLayerService_() == false )
        {
            return false;
        }

        if( this->initializeNotificationService_() == false )
        {
            return false;
        }

        if( this->initializeUnicodeEngine_() == false )
        {
            return false;
        }

        if( this->initializeFileEngine_() == false )
        {
            return false;
        }

		if( this->initializeConfigEngine_() == false )
		{
			return false;
		}

		if( this->initializeArchiveService_() == false )
		{
			return false;
		}

        if( this->initializeThreadEngine_() == false )
        {
            return false;
        }

        if( this->initializeParticleEngine2_() == false )
        {
            return false;
        }

        //	if( this->initializePhysicEngine2D_() == false )
        //	{
        //		return false;
        //	}

        if( this->initializeRenderEngine_() == false )
        {
            return false;
        }
                
        if( this->initializeSoundEngine_() == false )
        {
            if( this->initializeSilentSoundEngine_() == false )
            {
                return false;
            }
        }

		s3eDeviceYield(0);
        
        if( this->initializeScriptEngine_() == false )
        {
            return false;
        }

		s3eDeviceYield(0);

		if( this->initializeModuleEngine_() == false )
		{
			return false;
		}

		if( this->initializeDataManager_() == false )
		{
			return false;
		}

		if( this->initializeMemoryManager_() == false )
		{
			return false;
		}

        if( this->initializeCodecEngine_() == false )
        {
            return false;
        }

        if( this->initializeInputEngine_() == false )
        {
            return false;
        }

        if( this->initializeApplicationService_() == false )
        {
            return false;
        }

		if( this->initializePrefetcherService_() == false )
		{
			return false;
		}

        if( this->initializePluginService_() == false )
        {
            return false;
        }

		LOGGER_WARNING(m_serviceProvider)("Device info:"
			);

		const char * deviceID = s3eDeviceGetString( S3E_DEVICE_ID );

		LOGGER_WARNING(m_serviceProvider)("ID: %s"
			, deviceID
			);

		const char * deviceOS = s3eDeviceGetString( S3E_DEVICE_OS );

		LOGGER_WARNING(m_serviceProvider)("OS: %s"
			, deviceOS
			);

		const char * deviceOSVersion= s3eDeviceGetString( S3E_DEVICE_OS_VERSION );

		LOGGER_WARNING(m_serviceProvider)("OS Version: %s"
			, deviceOSVersion
			);

		const char * deviceClass = s3eDeviceGetString( S3E_DEVICE_CLASS );

		LOGGER_WARNING(m_serviceProvider)("Class: %s"
			, deviceClass
			);

		const char * deviceArchitecture = s3eDeviceGetString( S3E_DEVICE_ARCHITECTURE );

		LOGGER_WARNING(m_serviceProvider)("Architecture: %s"
			, deviceArchitecture
			);

		const char * deviceName = s3eDeviceGetString( S3E_DEVICE_NAME );

		LOGGER_WARNING(m_serviceProvider)("Name: %s"
			, deviceName
			);		
        
		m_timer = new MarmaladeTimer;
		m_timer->initialize();

		m_marmaladeInput = new MarmaladeInput;
		m_marmaladeInput->setServiceProvider( m_serviceProvider );
		
		if( m_marmaladeInput->initialize() == false )
		{
			return false;
		}
		
        {
            LOGGER_INFO(m_serviceProvider)( "initialize Image Codec..." );

			Menge::PluginInterface * pluginMengeImageCodec;
            initPluginMengeImageCodec( &pluginMengeImageCodec );
            pluginMengeImageCodec->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeImageCodec );
        }

        {
            LOGGER_INFO(m_serviceProvider)( "initialize Sound Codec..." );

			Menge::PluginInterface * pluginMengeSoundCodec;
            initPluginMengeSoundCodec( &pluginMengeSoundCodec );
            pluginMengeSoundCodec->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeSoundCodec );
        }

		{
			LOGGER_INFO(m_serviceProvider)( "initialize Ogg Vorbis Codec..." );

			Menge::PluginInterface * pluginMengeOggVorbis;
			initPluginMengeOggVorbis( &pluginMengeOggVorbis );
			pluginMengeOggVorbis->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeOggVorbis );
		}

		{
			LOGGER_INFO(m_serviceProvider)( "initialize Video Codec..." );

			Menge::PluginInterface * pluginMengeVideoCodec;
			initPluginMengeVideoCodec( &pluginMengeVideoCodec );
			pluginMengeVideoCodec->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeVideoCodec );
		}

		{
			LOGGER_INFO(m_serviceProvider)( "initialize Amplifier..." );

			Menge::PluginInterface * pluginMengeAmplifier;
			initPluginMengeAmplifier( &pluginMengeAmplifier );
			pluginMengeAmplifier->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeAmplifier );
		}

		{
			LOGGER_INFO(m_serviceProvider)( "initialize Path Finder..." );

			Menge::PluginInterface * pluginPluginPathFinder;
			initPluginPathFinder( &pluginPluginPathFinder );
			pluginPluginPathFinder->initialize( m_serviceProvider );
			m_plugins.push_back( pluginPluginPathFinder );
		}

		{
			LOGGER_INFO( m_serviceProvider )("initialize Spine...");

			PluginInterface * pluginMengeSpine;
			initPluginMengeSpine( &pluginMengeSpine );
			pluginMengeSpine->initialize( m_serviceProvider );
			m_plugins.push_back( pluginMengeSpine );
		}

		TVectorString modules;
		CONFIG_VALUES(m_serviceProvider, "Modules", "Name", modules);

		for( TVectorString::const_iterator
			it = modules.begin(),
			it_end = modules.end();
		it != it_end;
		++it )
		{
			const String & moduleName = *it;

			if( MODULE_SERVICE(m_serviceProvider)
				->runModule( Helper::stringizeString(m_serviceProvider, moduleName) ) == false )
			{
				LOGGER_ERROR(m_serviceProvider)("Application Failed to run module %s"
					, moduleName.c_str()
					);

				return false;
			}
		}

		String languagePack;
		Helper::s_getOption( " -lang:", m_commandLine, &languagePack );

		if( languagePack.empty() == true )
		{
			languagePack = CONFIG_VALUE(m_serviceProvider, "Locale", "Default", "en");
		}

        LOGGER_WARNING(m_serviceProvider)("Locale %s"
            , languagePack.c_str()
            );

		String personalityModule = CONFIG_VALUE(m_serviceProvider, "Game", "PersonalityModule", "Personality" );

		FilePath resourceIniPath;
		if( this->getApplicationPath_( "Resource", "Path", resourceIniPath ) == false )
		{
			return false;
		}

		Resolution defaultWindowResolution;
		this->getDesktopResolution( defaultWindowResolution );
		//Resolution defaultWindowResolution = CONFIG_VALUE(m_serviceProvider, "Window", "Size", Resolution(1024, 768));
		uint32_t defaultWindowBits = CONFIG_VALUE(m_serviceProvider, "Window", "Bits", 32U);
		bool defaultWindowFullscreen = CONFIG_VALUE(m_serviceProvider, "Window", "Fullscreen", true);
		bool defaultWindowVSync = CONFIG_VALUE(m_serviceProvider, "Window", "VSync", true);

		m_application->setDefaultWindowDescription( defaultWindowResolution, defaultWindowBits, defaultWindowFullscreen, defaultWindowVSync );

		s3eDeviceYield(0);
		
        if( m_application->loadResourcePacks( ConstString::none(), resourceIniPath ) == false )
        {
            LOGGER_CRITICAL(m_serviceProvider)( "Application invalid load resource pak %s"
				, resourceIniPath.c_str()
                );

            return false;
        }

		s3eDeviceYield(0);

        LOGGER_INFO(m_serviceProvider)( "Application Initialize... %s"
            , m_platformName.c_str()
            );

		FilePath accountPath = STRINGIZE_STRING_LOCAL( m_serviceProvider, "accounts.ini" );

        LOGGER_INFO(m_serviceProvider)( "Initializing Game data... %s"
			, accountPath.c_str()
			);
				
        if( m_application->initializeGame( Helper::stringizeString(m_serviceProvider, personalityModule), Helper::stringizeString(m_serviceProvider, languagePack), accountPath, scriptInit ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "Application invalid initialize game"
                );

            return false;
        }

        LOGGER_INFO(m_serviceProvider)( "Creating Render Window..." );

        if( m_application->createRenderWindow( nullptr ) == false )
        {
            return false;
        }

#	ifndef _DEBUG		
		s3ePointerSetInt( S3E_POINTER_HIDE_CURSOR, 1 );
#	endif

		m_timer->reset();

		m_application->turnSound( true );

		this->initializeMarmaladePauseCallback_();

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	static int32 s3eCallback_Application_S3E_SURFACE_SCREENSIZE( void * _systemData, void * _userData )
	{
		s3eSurfaceOrientation * orientation = static_cast<s3eSurfaceOrientation *>(_systemData);

		ApplicationInterface * app = static_cast<ApplicationInterface *>(_userData);
		
		Resolution resolution;
		resolution.setWidth( orientation->m_Width );
		resolution.setHeight( orientation->m_Height );

		app->changeWindowResolution( resolution );

		return 0;
	}
    //////////////////////////////////////////////////////////////////////////
    static int32 s3eCallback_Input_S3E_SURFACE_SCREENSIZE( void * _systemData, void * _userData )
    {
        s3eSurfaceOrientation * orientation = static_cast<s3eSurfaceOrientation *>(_systemData);
		
		(void)orientation;
        
        MarmaladeInput * input = static_cast<MarmaladeInput *>(_userData);
        
        input->updateSurfaceResolution();
        
        return 0;
    }
	//////////////////////////////////////////////////////////////////////////
	static int32 s3eCallback_UnPause( void * _systemData, void * _userData )
	{
		(void)_systemData;

		MarmaladeApplication * application = static_cast<MarmaladeApplication *>(_userData);

		application->setActivate( true );

		return 0;
	}
	//////////////////////////////////////////////////////////////////////////
	static int32 s3eCallback_Pause( void * _systemData, void * _userData )
	{
		(void)_systemData;

		MarmaladeApplication * application = static_cast<MarmaladeApplication *>(_userData);

		application->setActivate( false );

		return 0;
	}
	//////////////////////////////////////////////////////////////////////////
	void MarmaladeApplication::setActivate( bool _value )
	{
		m_active = _value;
	}
	//////////////////////////////////////////////////////////////////////////
	void MarmaladeApplication::initializeMarmaladePauseCallback_()
	{
		m_active = true;

		if( s3eDeviceRegister( S3E_DEVICE_UNPAUSE, &s3eCallback_UnPause, m_application ) != S3E_RESULT_SUCCESS )
		{
			const char * err_str = s3eDeviceGetErrorString();
			s3eDeviceError err = s3eDeviceGetError();

			LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initialize S3E_DEVICE_UNPAUSE %s [%d]"
				, err_str
				, err
				);
		}

		if( s3eDeviceRegister( S3E_DEVICE_PAUSE, &s3eCallback_Pause, m_application ) != S3E_RESULT_SUCCESS )
		{
			const char * err_str = s3eDeviceGetErrorString();
			s3eDeviceError err = s3eDeviceGetError();

			LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initialize S3E_DEVICE_PAUSE %s [%d]"
				, err_str
				, err
				);
		}

		if( s3eDeviceRegister( S3E_DEVICE_FOREGROUND, &s3eCallback_UnPause, m_application ) != S3E_RESULT_SUCCESS )
		{
			const char * err_str = s3eDeviceGetErrorString();
			s3eDeviceError err = s3eDeviceGetError();

			LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initialize S3E_DEVICE_FOREGROUND %s [%d]"
				, err_str
				, err
				);
		}

		if( s3eDeviceRegister( S3E_DEVICE_BACKGROUND, &s3eCallback_Pause, m_application ) != S3E_RESULT_SUCCESS )
		{
			const char * err_str = s3eDeviceGetErrorString();
			s3eDeviceError err = s3eDeviceGetError();

			LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initialize S3E_DEVICE_BACKGROUND %s [%d]"
				, err_str
				, err
				);
		}
	}
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::loop()
    {
        if( m_application == nullptr )
        {
            return;
        }

		if( s3eSurfaceRegister( S3E_SURFACE_SCREENSIZE, &s3eCallback_Application_S3E_SURFACE_SCREENSIZE, m_application ) != S3E_RESULT_SUCCESS )
		{
			const char * err_str = s3eSurfaceGetErrorString();
			s3eSurfaceError err = s3eSurfaceGetError();

			LOGGER_ERROR(m_serviceProvider)("MarmaladeApplication::initialize S3E_SURFACE_SCREENSIZE %s [%d]"
				, err_str
				, err
				);
		}

		if( s3eSurfaceRegister( S3E_SURFACE_SCREENSIZE, &s3eCallback_Input_S3E_SURFACE_SCREENSIZE, m_marmaladeInput ) != S3E_RESULT_SUCCESS )
		{
			const char * err_str = s3eSurfaceGetErrorString();
			s3eSurfaceError err = s3eSurfaceGetError();

			LOGGER_ERROR(m_serviceProvider)("MarmaladeApplication::initialize S3E_SURFACE_SCREENSIZE %s [%d]"
				, err_str
				, err
				);
		}

        while( true )
        {
            s3eBool quit = s3eDeviceCheckQuitRequest();

            if( quit == S3E_TRUE )
            {
                break;
            }

			float frameTime = m_timer->getDeltaTime();

			if( m_active == false )
			{
				s3eDeviceYield( 100 );

				continue;
			}
            
            if( m_application->isFocus() == true )
            {
                s3eDeviceBacklightOn();
            }

            m_marmaladeInput->update();
            
            bool updating = m_application->beginUpdate();

            if( updating == true )
            {                
                m_application->tick( frameTime );
            }
            else
            {
                s3eDeviceYield( 20 );
            }

			if( m_application->isFocus() == true )
			{
				if( m_application->render() == true )
				{
					m_application->flush();
				}
			}

			m_application->endUpdate();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::openUrlInDefaultBrowser( const WString & _url )
    {
        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::finalize()
    {
		if( m_timer != nullptr )
		{
			delete m_timer;
			m_timer = nullptr;
		}

		if( m_marmaladeInput != nullptr )
		{
			m_marmaladeInput->finalize();
			delete m_marmaladeInput;
			m_marmaladeInput = nullptr;
		}

		for( TVectorPlugins::iterator
			it = m_plugins.begin(),
			it_end = m_plugins.end();
		it != it_end;
		++it )
		{
			PluginInterface * plugin = *it;

			plugin->finalize();
		}

		if( m_application != nullptr )
		{
			m_application->finalize();
		}

		if( m_dataService != nullptr )
		{
			m_dataService->finalize();
			SERVICE_DESTROY( DataService, m_dataService );
		}

		if( m_moduleService != nullptr )
		{
			SERVICE_DESTROY( ModuleService, m_moduleService );
			m_moduleService = nullptr;
		}

		if( m_inputService != nullptr )
		{
			m_inputService->finalize();

			SERVICE_DESTROY( InputService, m_inputService );
			m_inputService = nullptr;
		}

		SERVICE_DESTROY( UnicodeService, m_unicodeService );        
		m_unicodeService = nullptr;

		SERVICE_DESTROY( UnicodeSystem, m_unicodeSystem );
		m_unicodeSystem = nullptr;

		if( m_fileService != nullptr )
		{            
			SERVICE_DESTROY( FileService, m_fileService );
			m_fileService = nullptr;
		}

		if( m_codecService != nullptr)
		{
			m_codecService->finalize();

			SERVICE_DESTROY( CodecService, m_codecService );
			m_codecService = nullptr;
		}

		if( m_particleService2 != nullptr )
		{
			SERVICE_DESTROY( ParticleService2, m_particleService2 );
			m_particleService2 = nullptr;
		}

		if( m_soundService != nullptr )
		{
			m_soundService->finalize();

			SERVICE_DESTROY( SoundService, m_soundService );
			m_soundService = nullptr;
		}

		if( m_soundSystem != nullptr )
		{
			m_soundSystem->finalize();

			SERVICE_DESTROY( SoundSystem, m_soundSystem );        
			m_soundSystem = nullptr;
		}

		SERVICE_DESTROY( Application, m_application );
		m_application = nullptr;

		if( m_scriptService != nullptr )
		{
			m_scriptService->finalize();
		}

		if( m_renderService != nullptr )
		{
			m_renderService->finalize();
		}

		if( m_renderMaterialManager != nullptr )
		{
			m_renderMaterialManager->finalize();
		}

		if( m_renderTextureManager != nullptr )
		{
			m_renderTextureManager->finalize();
		}

		if( m_renderSystem != nullptr )
		{
			m_renderSystem->finalize();
		}

		if( m_renderService != nullptr )
		{
			SERVICE_DESTROY( RenderService, m_renderService );
			m_renderService = nullptr;
		}

		if( m_renderMaterialManager != nullptr )
		{
			SERVICE_DESTROY( RenderMaterialManager, m_renderMaterialManager );
			m_renderMaterialManager = nullptr;
		}

		if( m_renderTextureManager != nullptr )
		{
			SERVICE_DESTROY( RenderTextureManager, m_renderTextureManager );
			m_renderTextureManager = nullptr;
		}        

		if( m_renderSystem != nullptr )
		{
			SERVICE_DESTROY( RenderSystem, m_renderSystem );
			m_renderSystem = nullptr;
		}

		if( m_prefetcherService != nullptr )
		{
			m_prefetcherService->finalize();

			SERVICE_DESTROY( PrefetcherService, m_prefetcherService );
			m_prefetcherService = nullptr;
		}

		if( m_threadService != nullptr )
		{
			m_threadService->finalize();

			SERVICE_DESTROY( ThreadService, m_threadService );
			m_threadService = nullptr;
		}        

		if( m_threadSystem != nullptr )
		{
			m_threadSystem->finalize();

			SERVICE_DESTROY( ThreadSystem, m_threadSystem );
			m_threadSystem = nullptr;
		}

		if( m_scriptService != nullptr )
		{
			SERVICE_DESTROY( ScriptService, m_scriptService );
			m_scriptService = nullptr;
		}

		if( m_configService != nullptr )
		{
			SERVICE_DESTROY( ConfigService, m_configService );
			m_configService = nullptr;
		}

		if( m_stringizeService != nullptr )
		{
			SERVICE_DESTROY( StringizeService, m_stringizeService );
			m_stringizeService = nullptr;
		}

		if( m_fileLog != nullptr )
		{
			if( m_logService != nullptr )
			{
				m_logService->unregisterLogger( m_fileLog );
			}

			delete m_fileLog;
			m_fileLog = nullptr;
		}

		if( m_archiveService != nullptr )
		{
			SERVICE_DESTROY( ArchiveService, m_archiveService );
			m_archiveService = nullptr;
		}

		if( m_memoryService != nullptr )
		{
			m_memoryService->finalize();
			SERVICE_DESTROY( MemoryService, m_memoryService );
			m_memoryService = nullptr;
		}

		for( TVectorPlugins::iterator
			it = m_plugins.begin(),
			it_end = m_plugins.end();
		it != it_end;
		++it )
		{
			PluginInterface * plugin = *it;

			plugin->destroy();
		}	

		m_plugins.clear();


		if( m_loggerConsole != nullptr )
		{
			if( m_logService != nullptr )
			{
				m_logService->unregisterLogger( m_loggerConsole );
			}

			delete m_loggerConsole;
			m_loggerConsole = nullptr;
		}

		if( m_notificationService != nullptr )
		{
			SERVICE_DESTROY( NotificationService, m_notificationService );
			m_notificationService = nullptr;
		}

		SERVICE_DESTROY( LogService, m_logService );
		m_logService = nullptr;

		SERVICE_DESTROY( ServiceProvider, m_serviceProvider );
		m_serviceProvider = nullptr;
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::stop()
    {
        m_running = false;
        s3eDeviceRequestQuit();
    }
	//////////////////////////////////////////////////////////////////////////
	const ConstString & MarmaladeApplication::getPlatformName() const
	{
		return m_platformName;
	}
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::getDesktopResolution( Resolution & _resolution ) const
    {
        int32 width = s3eSurfaceGetInt( S3E_SURFACE_WIDTH );
        int32 height = s3eSurfaceGetInt( S3E_SURFACE_HEIGHT );

        _resolution.setWidth( width );
        _resolution.setHeight( height );
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::minimizeWindow()
    {

    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::setHandleMouse( bool _handle )
    {

    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::setCursorPosition( const mt::vec2f & _pos )
    {

    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::showKeyboard()
    {
		if( m_marmaladeInput != nullptr )
		{
			m_marmaladeInput->showKeyboard( true );
		}
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::hideKeyboard()
    {
		if( m_marmaladeInput != nullptr )
		{
			m_marmaladeInput->showKeyboard( false );
		}
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::notifyWindowModeChanged( const Resolution & _resolution, bool _fullscreen )
    {
		
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::notifyVsyncChanged( bool _vsync )
    {

    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::notifyCursorModeChanged( bool _mode )
    {
		if( _mode == true )
		{
			s3ePointerSetInt( S3E_POINTER_HIDE_CURSOR, 0 );
		}
		else
		{
			s3ePointerSetInt( S3E_POINTER_HIDE_CURSOR, 1 );
		}		
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::notifyCursorClipping( const Viewport & _viewport )
    {

    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::notifyCursorUnClipping()
    {

    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::notifyCursorIconSetup( const ConstString & _name, const FilePath & _path, const Blobject & _buffer )
    {
		return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::isDevelopmentMode() const
    {
        return m_developmentMode;
    }
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::isRoamingMode() const
	{
		return false;
	}
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::onEvent( const ConstString & _event, const TMapParams & _params )
    {

    }
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::createDirectoryUserPicture( const WString & _path, const WString & _file, const void * _data, size_t _size )
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::createDirectoryUserMusic( const WString & _path, const WString & _file, const void * _data, size_t _size )
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	uint32_t MarmaladeApplication::getProcessHandleCount() const
	{
		return 0U;
	}
}
