#	include "ZipPlugin.h"

#   include "Interface/StringizeInterface.h"

#	include "Interface/FileSystemInterface.h"

#	include "FileGroupZip.h"
#	include "ArchivatorZip.h"

#   include "Factory/FactoryDefault.h"

//////////////////////////////////////////////////////////////////////////
PLUGIN_FACTORY( MengeZip, Menge::ZipPlugin )
//////////////////////////////////////////////////////////////////////////
namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	ZipPlugin::ZipPlugin()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	bool ZipPlugin::_initialize()
	{
		FILE_SERVICE()
			->registerFileGroupFactory( STRINGIZE_STRING_LOCAL( "zip"), new FactoryDefault<FileGroupZip>() );
		
		ARCHIVE_SERVICE()
			->registerArchivator( STRINGIZE_STRING_LOCAL( "zip" ), new FactorableUnique<ArchivatorZip>() );

        return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void ZipPlugin::_finalize()
	{
		FILE_SERVICE()
			->unregisterFileGroupFactory( STRINGIZE_STRING_LOCAL( "zip") );

		ARCHIVE_SERVICE()
			->unregisterArchivator( STRINGIZE_STRING_LOCAL( "zip") );
	}
}