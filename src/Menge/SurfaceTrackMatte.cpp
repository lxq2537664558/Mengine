#	include "SurfaceTrackMatte.h"

#	include "Interface/StringizeInterface.h"

#	include "Kernel/ResourceImage.h"

#	include "Logger/Logger.h"

namespace	Menge
{
	//////////////////////////////////////////////////////////////////////////
	SurfaceTrackMatte::SurfaceTrackMatte()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	SurfaceTrackMatte::~SurfaceTrackMatte()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	void SurfaceTrackMatte::setResourceImage( const ResourceImagePtr & _resourceImage )
	{
		if( m_resourceImage == _resourceImage )
		{
			return;
		}

		m_resourceImage = _resourceImage;

		this->recompile();
	}
	//////////////////////////////////////////////////////////////////////////
	const ResourceImagePtr & SurfaceTrackMatte::getResourceImage() const
	{
		return m_resourceImage;
	}
	//////////////////////////////////////////////////////////////////////////
	void SurfaceTrackMatte::setResourceTrackMatteImage( const ResourceImagePtr & _resourceTrackMatteImage )
	{
		if( m_resourceTrackMatteImage == _resourceTrackMatteImage )
		{
			return;
		}

		m_resourceTrackMatteImage = _resourceTrackMatteImage;

		this->recompile();
	}
	//////////////////////////////////////////////////////////////////////////
	const ResourceImagePtr & SurfaceTrackMatte::getResourceTrackMatteImage() const
	{
		return m_resourceTrackMatteImage;
	}
    //////////////////////////////////////////////////////////////////////////
    const mt::vec2f & SurfaceTrackMatte::getMaxSize() const
    {
        const mt::vec2f & maxSize = m_resourceImage->getMaxSize();

        return maxSize;
    }
    //////////////////////////////////////////////////////////////////////////
    const mt::vec2f & SurfaceTrackMatte::getSize() const
    {
        const mt::vec2f & size = m_resourceImage->getSize();

        return size;
    }
    //////////////////////////////////////////////////////////////////////////
    const mt::vec2f & SurfaceTrackMatte::getOffset() const
    {
        const mt::vec2f & offset = m_resourceImage->getOffset();

        return offset;
    }
    //////////////////////////////////////////////////////////////////////////
    uint32_t SurfaceTrackMatte::getUVCount() const
    {
        return 2;
    }
    //////////////////////////////////////////////////////////////////////////
    const mt::uv4f & SurfaceTrackMatte::getUV( uint32_t _index ) const
    {
        if( _index == 0 )
        {
            return m_resourceImage->getUVImage();
        }
        if( _index == 1 )
        {
            return m_resourceTrackMatteImage->getUVImage();
        }

        return mt::uv4f::identity();
    }
    //////////////////////////////////////////////////////////////////////////
    const ColourValue & SurfaceTrackMatte::getColour() const
    {
        const ColourValue & color = m_resourceImage->getColor();

        return color;
    }
    //////////////////////////////////////////////////////////////////////////
    bool SurfaceTrackMatte::update( float _current, float _timing )
    {
        (void)_current;
        (void)_timing;

        return false;
    }
	//////////////////////////////////////////////////////////////////////////
	bool SurfaceTrackMatte::_compile()
	{
		if( m_resourceImage == nullptr )
		{
			LOGGER_ERROR( m_serviceProvider )("SurfaceTrackMatte::_compile: '%s' resource is null"
				, m_name.c_str()
				);

			return false;
		}

		if( m_resourceImage.compile() == false )
		{
			LOGGER_ERROR( m_serviceProvider )("SurfaceTrackMatte::_compile: '%s' resource '%s' is not compile"
				, m_name.c_str()
				, m_resourceImage->getName().c_str()
				);

			return false;
		}

		if( m_resourceTrackMatteImage == nullptr )
		{
			LOGGER_ERROR( m_serviceProvider )("SurfaceTrackMatte::_compile: '%s' resource is null"
				, m_name.c_str()
				);

			return false;
		}

		if( m_resourceTrackMatteImage.compile() == false )
		{
			LOGGER_ERROR( m_serviceProvider )("SurfaceTrackMatte::_compile: '%s' resource '%s' is not compile"
				, m_name.c_str()
				, m_resourceTrackMatteImage->getName().c_str()
				);

			return false;
		}

		this->invalidateMaterial();

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void SurfaceTrackMatte::_release()
	{
		m_resourceImage.release();
		m_resourceTrackMatteImage.release();
	}
	//////////////////////////////////////////////////////////////////////////
	RenderMaterialInterfacePtr SurfaceTrackMatte::_updateMaterial() const
	{
		RenderTextureInterfacePtr textures[2];

		textures[0] = m_resourceImage->getTexture();
		textures[1] = m_resourceTrackMatteImage->getTexture();

		RenderMaterialInterfacePtr material = RENDERMATERIAL_SERVICE( m_serviceProvider )
			->getMaterial( STRINGIZE_STRING_LOCAL( m_serviceProvider, "TrackMatte_Blend" ), PT_TRIANGLELIST, 2, textures );

		return material;
	}
	//////////////////////////////////////////////////////////////////////////
}