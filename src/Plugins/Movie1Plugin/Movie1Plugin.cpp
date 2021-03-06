#include "Movie1Plugin.h"

#include "Interface/PrototypeServiceInterface.h"
#include "Interface/ApplicationInterface.h"
#include "Interface/StringizeServiceInterface.h"
#include "Interface/ScriptServiceInterface.h"
#include "Interface/ResourceServiceInterface.h"
#include "Interface/ConfigServiceInterface.h"
#include "Interface/DataServiceInterface.h"
#include "Interface/CodecServiceInterface.h"
#include "Interface/LoaderServiceInterface.h"
#include "Interface/VocabularyServiceInterface.h"

#include "MovieScriptEmbedding.h"

#include "Plugins/ResourcePrefetcherPlugin/ResourcePrefetcherServiceInterface.h"
#include "Plugins/ResourceValidatePlugin/ResourceValidateServiceInterface.h"

#include "Environment/Python/PythonScriptWrapper.h"
#include "Environment/Python/PythonAnimatableEventReceiver.h"

#include "Engine/HotSpot.h"
#include "Engine/HotSpotShape.h"
#include "Engine/ResourceShape.h"

#include "Kernel/RenderScissor.h"
#include "Kernel/DefaultPrototypeGenerator.h"
#include "Kernel/NodePrototypeGenerator.h"
#include "Kernel/ResourcePrototypeGenerator.h"
#include "Kernel/NodeRenderHelper.h"
#include "Kernel/PolygonHelper.h"

#include "Kernel/Logger.h"

#include "Movie.h"
#include "MovieSlot.h"
#include "MovieSceneEffect.h"
#include "MovieEvent.h"
#include "MovieInternalObject.h"
#include "MovieMesh2D.h"

#include "ResourceMovie.h"
#include "ResourceInternalObject.h"

#include "LoaderResourceMovie.h"

#include "DataflowAEK.h"

#include "ResourceMovieValidator.h"

#include "pybind/pybind.hpp"

//////////////////////////////////////////////////////////////////////////
PLUGIN_FACTORY( Movie1, Mengine::Movie1Plugin )
//////////////////////////////////////////////////////////////////////////
namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    Movie1Plugin::Movie1Plugin()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    Movie1Plugin::~Movie1Plugin()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool Movie1Plugin::_availablePlugin() const
    {
        bool available = CONFIG_VALUE( "Engine", "Movie1PluginAvailable", true );

        return available;
    }    
    //////////////////////////////////////////////////////////////////////////
    bool Movie1Plugin::_initializePlugin()
    {
        this->addDependencyService( "PrefetcherService" );

        ADD_SCRIPT_EMBEDDING( MovieScriptEmbedding );

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "Movie" ), Helper::makeFactorableUnique<NodePrototypeGenerator<Movie, 128>>() ) == false )
        {
            return false;
        }

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieSlot" ), Helper::makeFactorableUnique<NodePrototypeGenerator<MovieSlot, 128>>() ) == false )
        {
            return false;
        }

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieSceneEffect" ), Helper::makeFactorableUnique<NodePrototypeGenerator<MovieSceneEffect, 128>>() ) == false )
        {
            return false;
        }

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieInternalObject" ), Helper::makeFactorableUnique<NodePrototypeGenerator<MovieInternalObject, 128>>() ) == false )
        {
            return false;
        }

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieEvent" ), Helper::makeFactorableUnique<NodePrototypeGenerator<MovieEvent, 128>>() ) == false )
        {
            return false;
        }

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieMesh2D" ), Helper::makeFactorableUnique<NodePrototypeGenerator<MovieMesh2D, 128>>() ) == false )
        {
            return false;
        }

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Resource" ), STRINGIZE_STRING_LOCAL( "ResourceMovie" ), Helper::makeFactorableUnique<ResourcePrototypeGenerator<ResourceMovie, 64>>() ) == false )
        {
            return false;
        }

        if( PROTOTYPE_SERVICE()
            ->addPrototype( STRINGIZE_STRING_LOCAL( "Resource" ), STRINGIZE_STRING_LOCAL( "ResourceInternalObject" ), Helper::makeFactorableUnique<ResourcePrototypeGenerator<ResourceInternalObject, 64> >() ) == false )
        {
            return false;
        }

        DataflowInterfacePtr dataflowAEK = Helper::makeFactorableUnique<DataflowAEK>();

        if( dataflowAEK->initialize() == false )
        {
            return false;
        }

        VOCABULARY_SET( DataflowInterface, STRINGIZE_STRING_LOCAL( "Dataflow" ), STRINGIZE_STRING_LOCAL( "aekMovie" ), dataflowAEK );

        CODEC_SERVICE()
            ->registerCodecExt( "aek", STRINGIZE_STRING_LOCAL( "aekMovie" ) );

        if( SERVICE_EXIST( ResourcePrefetcherServiceInterface ) == true )
        {
            ResourcePrefetcherInterfacePtr resourcePrefetcher = VOCABULARY_GET( STRINGIZE_STRING_LOCAL( "ResourcePrefetcher" ), STRINGIZE_STRING_LOCAL( "Dataflow" ) );

            RESOURCEPREFETCHER_SERVICE()
                ->addResourcePrefetcher( STRINGIZE_STRING_LOCAL( "ResourceMovie" ), resourcePrefetcher );
        }

        if( SERVICE_EXIST( ResourceValidateServiceInterface ) == true )
        {
            VOCABULARY_SET( ResourceValidatorInterface, STRINGIZE_STRING_LOCAL( "Validator" ), STRINGIZE_STRING_LOCAL( "ResourceMovie" ), Helper::makeFactorableUnique<ResourceMovieValidator>() );
        }

        if( SERVICE_EXIST( LoaderServiceInterface ) == true )
        {
            VOCABULARY_SET( LoaderInterface, STRINGIZE_STRING_LOCAL( "Loader" ), STRINGIZE_STRING_LOCAL( "ResourceMovie" ), Helper::makeFactorableUnique<LoaderResourceMovie>() );
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void Movie1Plugin::_finalizePlugin()
    {
        REMOVE_SCRIPT_EMBEDDING( MovieScriptEmbedding );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "Movie" ) );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieSlot" ) );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieSceneEffect" ) );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieInternalObject" ) );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieEvent" ) );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Node" ), STRINGIZE_STRING_LOCAL( "MovieMesh2D" ) );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Resource" ), STRINGIZE_STRING_LOCAL( "ResourceMovie" ) );

        PROTOTYPE_SERVICE()
            ->removePrototype( STRINGIZE_STRING_LOCAL( "Resource" ), STRINGIZE_STRING_LOCAL( "ResourceInternalObject" ) );

        if( SERVICE_EXIST( ResourcePrefetcherServiceInterface ) == true )
        {
            RESOURCEPREFETCHER_SERVICE()
                ->removeResourcePrefetcher( STRINGIZE_STRING_LOCAL( "ResourceMovie" ) );
        }

        if( SERVICE_EXIST( ResourceValidateServiceInterface ) == true )
        {
            VOCABULARY_REMOVE( STRINGIZE_STRING_LOCAL( "Validator" ), STRINGIZE_STRING_LOCAL( "ResourceMovie" ) );
        }

        DataflowInterfacePtr dataflow = VOCABULARY_REMOVE( STRINGIZE_STRING_LOCAL( "Dataflow" ), STRINGIZE_STRING_LOCAL( "aekMovie" ) );
        dataflow->finalize();

        if( SERVICE_EXIST( LoaderServiceInterface ) == true )
        {
            VOCABULARY_REMOVE( STRINGIZE_STRING_LOCAL( "Loader" ), STRINGIZE_STRING_LOCAL( "ResourceMovie" ) );
        }
    }
}
