#include "LoaderResourceMovie2.h"

#include "ResourceMovie2.h"

#include "Metacode/Metacode.h"

namespace Mengine
{   
    //////////////////////////////////////////////////////////////////////////
    LoaderResourceMovie2::LoaderResourceMovie2()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    LoaderResourceMovie2::~LoaderResourceMovie2()
    {
    }    
    //////////////////////////////////////////////////////////////////////////
    bool LoaderResourceMovie2::load( const LoadableInterfacePtr & _loadable, const Metabuf::Metadata * _meta )
    {
        ResourceMovie2 * resource = stdex::intrusive_get<ResourceMovie2 *>( _loadable );

        const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2 * metadata
            = static_cast<const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2 *>(_meta);

        const FilePath & filePath = metadata->get_File_Path();
        resource->setFilePath( filePath );

        const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2::VectorMeta_Composition & includes_composition = metadata->get_Includes_Composition();

        for( const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2::Meta_Composition & meta_composition : includes_composition )
        {
            const ConstString & compositionName = meta_composition.get_Name();

            ResourceMovie2CompositionDesc desc;

            desc.duration = meta_composition.get_Duration();
            desc.frameDuration = meta_composition.get_FrameDuration();            
            desc.has_bounds = meta_composition.get_Bounds( &desc.bounds );
            
            desc.master = meta_composition.getd_Master( true );

            const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2::Meta_Composition::VectorMeta_Layer & includes_layer = meta_composition.get_Includes_Layer();

            for( const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2::Meta_Composition::Meta_Layer & meta_layer : includes_layer )
            {
                uint32_t layerIndex = meta_layer.get_Index();
                const ConstString & layerName = meta_layer.get_Name();
                const ConstString & layerType = meta_layer.get_Type();
                const mt::mat4f & layerMatrix = meta_layer.get_Matrix();
                const ColourValue & layerColor = meta_layer.get_Color();

                ResourceMovie2CompositionLayer layer;
                layer.index = layerIndex;
                layer.name = layerName;
                layer.type = layerType;
                layer.matrix = layerMatrix;
                layer.color = layerColor;

                desc.layers.emplace_back( layer );
            }

            const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2::Meta_Composition::VectorMeta_SubComposition & includes_subcomposition = meta_composition.get_Includes_SubComposition();

            for( const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2::Meta_Composition::Meta_SubComposition & meta_subcomposition : includes_subcomposition )
            {
                uint32_t subcompositionIndex = meta_subcomposition.get_Index();
                const ConstString & subcompositionName = meta_subcomposition.get_Name();

                ResourceMovie2CompositionSubComposition subcomposition;
                subcomposition.index = subcompositionIndex;
                subcomposition.name = subcompositionName;

                desc.subcompositions.emplace_back( subcomposition );
            }

            resource->setCompositionDesc( compositionName, desc );
        }

        return true;
    }
}