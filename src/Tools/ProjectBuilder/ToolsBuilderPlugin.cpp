#	include "Interface/ServiceInterface.h"

#	include "XmlToBinDecoder.h"
#	include "AlphaSpreadingPlugin.h"
#	include "XmlToAekConverter.h"

#	include "Interface/StringizeInterface.h"
#	include "Interface/ArchiveInterface.h"
#	include "Interface/LogSystemInterface.h"
#	include "Interface/CodecInterface.h"
#	include "Interface/DataInterface.h"
#	include "Interface/CacheInterface.h"
#   include "Interface/ConverterInterface.h"
#   include "Interface/FileSystemInterface.h"
#   include "Interface/PluginInterface.h"
#   include "Interface/WindowsLayerInterface.h"
#   include "Interface/UnicodeInterface.h"
#   include "Interface/ImageCodecInterface.h"
#	include "Interface/LoaderInterface.h"
#	include "Interface/ThreadSystemInterface.h"
#	include "Interface/ParticleSystemInterface.h"
#	include "Interface/ConfigInterface.h"

#   include "WindowsLayer/VistaWindowsLayer.h"

#	include "Codec/ConverterFactory.h"

#   include "Logger\Logger.h"

#	include <pybind/pybind.hpp>

SERVICE_EXTERN(ServiceProvider, Menge::ServiceProviderInterface);

SERVICE_EXTERN(ArchiveService, Menge::ArchiveServiceInterface);
 
SERVICE_EXTERN(UnicodeSystem, Menge::UnicodeSystemInterface);
SERVICE_EXTERN(UnicodeService, Menge::UnicodeServiceInterface);

SERVICE_EXTERN(StringizeService, Menge::StringizeServiceInterface);
SERVICE_EXTERN(LogService, Menge::LogServiceInterface);
SERVICE_EXTERN(ConfigService, Menge::ConfigServiceInterface);
SERVICE_EXTERN(CodecService, Menge::CodecServiceInterface);
SERVICE_EXTERN(DataService, Menge::DataServiceInterface);
SERVICE_EXTERN(CacheService, Menge::CacheServiceInterface);
SERVICE_EXTERN(ConverterService, Menge::ConverterServiceInterface);
SERVICE_EXTERN(PluginService, Menge::PluginServiceInterface);

SERVICE_EXTERN(FileService, Menge::FileServiceInterface);

SERVICE_EXTERN(LoaderService, Menge::LoaderServiceInterface);

SERVICE_EXTERN(ThreadSystem, Menge::ThreadSystemInterface);
SERVICE_EXTERN(ThreadService, Menge::ThreadServiceInterface);

//////////////////////////////////////////////////////////////////////////
extern "C" // only required if using g++
{
	extern bool initPluginMengeWin32FileGroup( Menge::PluginInterface ** _plugin );
	extern bool initPluginMengeImageCodec( Menge::PluginInterface ** _plugin );	
	extern bool initPluginMengeZip( Menge::PluginInterface ** _plugin );
	extern bool initPluginMengeLZ4( Menge::PluginInterface ** _plugin );
}

PyObject * PyToolException;

namespace Menge
{
	ServiceProviderInterface * serviceProvider = NULL;

	static bool initialize()
	{
		if( SERVICE_CREATE( ServiceProvider, &serviceProvider ) == false )
		{
			return false;
		}

		UnicodeSystemInterface * unicodeSystem;
		if( SERVICE_CREATE( UnicodeSystem, &unicodeSystem ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, unicodeSystem) == false )
		{
			return false;
		}

		UnicodeServiceInterface * unicodeService;
		if( SERVICE_CREATE( UnicodeService, &unicodeService ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, unicodeService) == false )
		{
			return false;
		}

		StringizeServiceInterface * stringizeService;
		if( SERVICE_CREATE( StringizeService, &stringizeService ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, stringizeService) == false )
		{
			return false;
		}

		ArchiveServiceInterface * archiveService;
		if( SERVICE_CREATE( ArchiveService, &archiveService ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, archiveService) == false )
		{
			return false;
		}

		LogServiceInterface * logService;
		if( SERVICE_CREATE( LogService, &logService ) == false )
		{
			return false;
		}

		class MyLoggerInterface
			: public LoggerInterface
		{
		public:
			void setVerboseLevel( EMessageLevel _level ) override 
			{
				(void)_level;
			};

			void setVerboseFlag( size_t _flag ) override 
			{
				(void)_flag;
			};

		public:
			bool validMessage( EMessageLevel _level, size_t _flag ) const override 
			{ 
				(void)_level;
				(void)_flag;

				return true; 
			};

		public:
			void log( EMessageLevel _level, size_t _flag, const char * _data, size_t _count ) override
			{
				(void)_level;
				(void)_flag;
				(void)_count;

				printf("%s"
					, _data
					);
			}

			void flush() override 
			{
			}
		};

		logService->setVerboseLevel( LM_WARNING );
		logService->registerLogger( new MyLoggerInterface );

		if( SERVICE_REGISTRY(serviceProvider, logService) == false )
		{
			return false;
		}

		LOGGER_WARNING(serviceProvider)("Inititalizing Config Manager..." );

		ConfigServiceInterface * configService;

		if( SERVICE_CREATE( ConfigService, &configService ) == false )
		{
			return false;
		}

		SERVICE_REGISTRY( serviceProvider, configService );

		ConverterServiceInterface * converterService;
		if( SERVICE_CREATE( ConverterService, &converterService ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY( serviceProvider, converterService ) == false )
		{
			return false;
		}

		CodecServiceInterface * codecService;
		if( SERVICE_CREATE( CodecService, &codecService ) == false )
		{
			return false;
		}
		
		if( SERVICE_REGISTRY(serviceProvider, codecService) == false )
		{
			return false;
		}

		DataServiceInterface * dataService;
		if( SERVICE_CREATE( DataService, &dataService ) == false )
		{
			return false;
		}

		dataService->initialize();

		if( SERVICE_REGISTRY(serviceProvider, dataService) == false )
		{
			return false;
		}

		ThreadSystemInterface * threadSystem;
		if( SERVICE_CREATE( ThreadSystem, &threadSystem ) == false )
		{
			LOGGER_ERROR(serviceProvider)("WinApplication::initializeThreadEngine_ failed to create ThreadSystem"
				);

			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, threadSystem) == false )
		{
			return false;
		}

		if( threadSystem->initialize() == false )
		{
			LOGGER_ERROR(serviceProvider)("WinApplication::initializeThreadEngine_ failed to initialize ThreadSystem"
				);

			return false;
		}

		ThreadServiceInterface * threadService;
		if( SERVICE_CREATE( ThreadService, &threadService ) == false )
		{
			LOGGER_ERROR(serviceProvider)("WinApplication::initializeThreadEngine_ failed to create ThreadService"
				);

			return false;               
		}

		if( SERVICE_REGISTRY( serviceProvider, threadService ) == false )
		{
			return false;
		}

		if( threadService->initialize( 16 ) == false )
		{
			return false;
		}
				
		CacheServiceInterface * cacheService;
		if( SERVICE_CREATE( CacheService, &cacheService ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, cacheService) == false )
		{
			return false;
		}

		if( cacheService->initialize() == false )
		{
			return false;
		}

		PluginServiceInterface * pluginService;
		if( SERVICE_CREATE( PluginService, &pluginService ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, pluginService) == false )
		{
			return false;
		}

		WindowsLayerInterface * windowsLayer = new VistaWindowsLayer();

		if( SERVICE_REGISTRY(serviceProvider, windowsLayer) == false )
		{
			return false;
		}

		FileServiceInterface * fileService;
		if( SERVICE_CREATE( FileService, &fileService ) == false )
		{
			return false;
		}

		if( SERVICE_REGISTRY(serviceProvider, fileService) == false )
		{
			return false;
		}

		if( fileService->initialize() == false )
		{
			return false;
		}

		PluginInterface * plugin_win32_file_group;
		initPluginMengeWin32FileGroup( &plugin_win32_file_group );
		
		if( plugin_win32_file_group == nullptr )
		{
			return false;
		}

		plugin_win32_file_group->initialize( serviceProvider );	
		
		PluginInterface * plugin_zip;
		initPluginMengeZip( &plugin_zip );
		plugin_zip->initialize( serviceProvider );

		PluginInterface * plugin_lz4;
		initPluginMengeLZ4( &plugin_lz4 );
		plugin_lz4->initialize( serviceProvider );
		
		PluginInterface * plugin_image_codec;
		initPluginMengeImageCodec( &plugin_image_codec );

		if( plugin_image_codec == nullptr )
		{
			return false;
		}

		plugin_image_codec->initialize( serviceProvider );
		
		LoaderServiceInterface * loaderService;
		if( SERVICE_CREATE( LoaderService, &loaderService ) == false )
		{
			return false;
		}

		SERVICE_REGISTRY( serviceProvider, loaderService );

		PLUGIN_SERVICE(serviceProvider)
			->loadPlugin( L"MengeDevelopmentConverterPlugin.dll" );

		PLUGIN_SERVICE(serviceProvider)
			->loadPlugin( L"MengeXmlCodecPlugin.dll" );

		PLUGIN_SERVICE(serviceProvider)
			->loadPlugin( L"AstralaxParticleSystem.dll" );

		if( FILE_SERVICE(serviceProvider)
			->mountFileGroup( ConstString::none(), ConstString::none(), Helper::stringizeString(serviceProvider, "dir") ) == false )
		{
			return false;
		}

		ConstString dev = Helper::stringizeString(serviceProvider, "dev");

		if( FILE_SERVICE(serviceProvider)
			->mountFileGroup( dev, ConstString::none(), Helper::stringizeString(serviceProvider, "dir") ) == false )
		{
			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	static bool s_convert( const WString & _fromPath, const WString & _toPath, const WString & _convertType, const WString & _params )
	{		
		String utf8_fromPath;
		Helper::unicodeToUtf8( serviceProvider, _fromPath, utf8_fromPath );

		String utf8_toPath;
		Helper::unicodeToUtf8( serviceProvider, _toPath, utf8_toPath );

		String utf8_convertType;
		Helper::unicodeToUtf8( serviceProvider, _convertType, utf8_convertType );

		String utf8_params;
		Helper::unicodeToUtf8( serviceProvider, _params, utf8_params );		

		ConverterOptions options;

		options.pakName = ConstString::none();
		options.inputFileName = Helper::stringizeString(serviceProvider, utf8_fromPath);
		options.outputFileName = Helper::stringizeString(serviceProvider, utf8_toPath);
		options.params = utf8_params;

		ConverterInterfacePtr converter = CONVERTER_SERVICE(serviceProvider)
			->createConverter( Helper::stringizeString(serviceProvider, utf8_convertType) );

		if( converter == nullptr )
		{
			LOGGER_ERROR(serviceProvider)("convertPVRToHTF can't create convert '%s'\nfrom: %s\nto: %s\n"
				, utf8_convertType.c_str()
				, options.inputFileName.c_str()
				, options.outputFileName.c_str()
				);

			return false;
		}

		converter->setOptions( &options );

		if( converter->convert() == false )
		{
			LOGGER_ERROR(serviceProvider)( "convertPVRToHTF can't convert '%s'\nfrom: %s\nto: %s\n"
				, utf8_convertType.c_str()
				, options.inputFileName.c_str()
				, options.outputFileName.c_str()
				);

			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	PyObject * convert( PyObject * self, PyObject * args )
	{
		(void)self;

		const wchar_t * fromPath;
		const wchar_t * toPath;
		const wchar_t * convertType;
		const wchar_t * params;

		if( !PyArg_ParseTuple(args, "uuuu", &fromPath, &toPath, &convertType, &params ) )
		{
			LOGGER_ERROR(serviceProvider)("convert: error parse args"
				);

			Py_RETURN_FALSE;
		}

		if( s_convert( fromPath, toPath, convertType, params ) == false )
		{
			LOGGER_ERROR(serviceProvider)("convert: error process %ls to %ls convert %ls"
				, fromPath
				, toPath
				, convertType
				);

			Py_RETURN_FALSE;
		}

		Py_RETURN_TRUE;
	}
	//////////////////////////////////////////////////////////////////////////
	bool s_magicParticlesAtlasFiles( const wchar_t * _path, PyObject * _atlasFiles )
	{
		String utf8_path;
		if( Helper::unicodeToUtf8( serviceProvider, _path, utf8_path ) == false )
		{
			return false;
		}

		ConstString c_path = Helper::stringizeString(serviceProvider, utf8_path);

		InputStreamInterfacePtr stream = FILE_SERVICE(serviceProvider)
			->openInputFile( ConstString::none(), c_path, false );

		if( stream == nullptr )
		{
			LOGGER_ERROR(serviceProvider)("magicParticlesAtlasFiles: Image file '%s' was not found"
				, c_path.c_str() 
				);

			return false;
		}

		ParticleEmitterContainerInterfacePtr container = PARTICLE_SYSTEM(serviceProvider)
			->createParticleEmitterContainer();

		if( container == nullptr )
		{
			LOGGER_ERROR(serviceProvider)("magicParticlesAtlasFiles: invalid create container '%s'"
				, c_path.c_str() 
				);

			return false;
		}

		if( PARTICLE_SYSTEM(serviceProvider)
			->loadParticleEmitterContainerFromMemory( container, stream ) == false )
		{
			LOGGER_ERROR(serviceProvider)("magicParticlesAtlasFiles: invalid load container '%s'"
				, c_path.c_str() 
				);

			return false;
		}

		const TVectorParticleEmitterAtlas &  atlas = container->getAtlas();

		for( TVectorParticleEmitterAtlas::const_iterator
			it = atlas.begin(),
			it_end = atlas.end();
		it != it_end;
		++it )
		{
			const ParticleEmitterAtlas & atlas = *it;
			
			PyObject * py_fileName = PyUnicode_FromString( atlas.filename.c_str() );

			PyList_Append( _atlasFiles, py_fileName );
			Py_DECREF( py_fileName );
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	PyObject * magicParticlesAtlasFiles( PyObject * self, PyObject * args )
	{
		(void)self;

		const wchar_t * path;

		if( !PyArg_ParseTuple(args, "u", &path) )
		{
			LOGGER_ERROR(serviceProvider)("magicParticlesAtlasFiles: error parse args"
				);

			Py_RETURN_FALSE;
		}

		PyObject * atlasFiles = PyList_New( 0 );

		if( s_magicParticlesAtlasFiles( path, atlasFiles ) == false )
		{
			LOGGER_ERROR(serviceProvider)("magicParticlesAtlasFiles: error process %ls"
				, path
				);

			Py_RETURN_FALSE;
		}

		return atlasFiles;
	}
	
}
/////////////////////////////////////////////////////////////////////////////////////
static PyMethodDef ModuleMethods[] =
{
	{"spreadingPngAlpha", Menge::spreadingPngAlpha, METH_VARARGS, "spreading png alpha"},
	{"writeBin", Menge::writeBin, METH_VARARGS, "write binary"},
	{"writeAek", Menge::writeAek, METH_VARARGS, "write aek"},	
	{"convert", Menge::convert, METH_VARARGS, "convert"},
	{"magicParticlesAtlasFiles", Menge::magicParticlesAtlasFiles, METH_VARARGS, "magicParticlesAtlasFiles"},
	{NULL, NULL, 0, NULL}
};
/////////////////////////////////////////////////////////////////////////////////////
struct PyModuleDef module_def = {
	PyModuleDef_HEAD_INIT,
	"ToolsBuilderPlugin",
	NULL,
	-1, 
	ModuleMethods,
	NULL, NULL, NULL, NULL
};
//////////////////////////////////////////////////////////////////////////
static void s_error( const wchar_t * _msg )
{
	LOGGER_ERROR(Menge::serviceProvider)("%ls"
		, _msg
		);
}
///////////////////////////////////////////////////////////////////////////////////
PyMODINIT_FUNC PyInit_ToolsBuilderPlugin(void)
{
	try
	{
		if( Menge::initialize() == false )
		{
			printf("PyInit_ToolsBuilderPlugin Menge::initialize failed\n"
				);

			return NULL;
		}
	}
	catch( const std::exception & se )
	{
		printf("PyInit_ToolsBuilderPlugin exception %s"
			, se.what()
			);

		return NULL;
	}
		
	PyObject * m =  PyModule_Create( &module_def );

	if( m == NULL )
	{
		return NULL;
	}

	PyToolException = PyErr_NewException("PyInit_ToolsBuilderPlugin.error", NULL, NULL);

	Py_INCREF( PyToolException );

	PyModule_AddObject( m, "error", PyToolException );
		
	pybind::initialize( false, false );

	PyObject * module_builtins = pybind::get_builtins();

	pybind::def_function( "Error", &s_error, module_builtins );
	
	return m;
}