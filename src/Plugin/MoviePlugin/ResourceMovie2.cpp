#	include "ResourceMovie2.h"

#	include "Interface/ResourceInterface.h"
#	include "Interface/StringizeInterface.h"

#	include "Menge/ResourceImageDefault.h"
#	include "Menge/ResourceVideo.h"
#	include "Menge/ResourceSound.h"
#	include "Menge/ResourceParticle.h"

#	include "Metacode/Metacode.h"

#	include "Logger/Logger.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	static size_t Mengine_read_stream( ae_voidptr_t _data, ae_voidptr_t _buff, size_t _carriage, size_t _size )
	{
        (void)_carriage;

		InputStreamInterface * stream = (InputStreamInterface *)_data;

		size_t bytes = stream->read( _buff, _size );

		return bytes;
	}
	//////////////////////////////////////////////////////////////////////////
	static void Mengine_copy_stream( ae_voidptr_t _data, ae_constvoidptr_t _src, ae_voidptr_t _dst, size_t _size )
	{
		(void)_data;

		memcpy( _dst, _src, _size );
	}
    //////////////////////////////////////////////////////////////////////////
    static ae_voidptr_t Mengine_resource_provider( const aeMovieResource * _resource, ae_voidptr_t _data )
    {
        ResourceMovie2 * resourceMovie2 = (ResourceMovie2 *)_data;

        aeMovieResourceTypeEnum resource_type = _resource->type;

        switch( resource_type )
        {
        case AE_MOVIE_RESOURCE_IMAGE:
            {
                const aeMovieResourceImage * resource_image = (const aeMovieResourceImage *)_resource;

                ResourceReference * data_resource = resourceMovie2->createResourceImage_( resource_image->path, resource_image->trim_width, resource_image->trim_height );

                return data_resource;
            }break;
        case AE_MOVIE_RESOURCE_VIDEO:
            {
                const aeMovieResourceVideo * resource_video = (const aeMovieResourceVideo *)_resource;

                ResourceReference * data_resource = resourceMovie2->createResourceVideo_( resource_video );

                return data_resource;
            }break;
        case AE_MOVIE_RESOURCE_SOUND:
            {
                const aeMovieResourceSound * resource_sound = (const aeMovieResourceSound *)_resource;

                ResourceReference * data_resource = resourceMovie2->createResourceSound_( resource_sound );

                return data_resource;
            }break;
        case AE_MOVIE_RESOURCE_PARTICLE:
            {
                const aeMovieResourceParticle * resource_particle = (const aeMovieResourceParticle *)_resource;

                ResourceReference * data_resource = resourceMovie2->createResourceParticle_( resource_particle );

                return data_resource;
            }break;
        }

        return AE_NULL;
    }
    //////////////////////////////////////////////////////////////////////////
    static void Mengine_resource_deleter( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud )
    {
        (void)_type;
        (void)_data;
        (void)_ud;
    }
	//////////////////////////////////////////////////////////////////////////
	ResourceMovie2::ResourceMovie2()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	ResourceMovie2::~ResourceMovie2()
	{
	}	
    //////////////////////////////////////////////////////////////////////////
    bool ResourceMovie2::hasComposition( const ConstString & _name ) const
    {
        ae_bool_t exist = ae_has_movie_composition_data( m_movieData, _name.c_str() );

        return exist;
    }
    //////////////////////////////////////////////////////////////////////////
    float ResourceMovie2::getCompositionDuration( const ConstString & _name ) const
    {
        const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( m_movieData, _name.c_str() );

        if( compositionData == nullptr )
        {
            return 0.f;
        }

        float duration = ae_get_movie_composition_data_duration( compositionData );

        return duration;
    }
    //////////////////////////////////////////////////////////////////////////
    float ResourceMovie2::getCompositionFrameDuration( const ConstString & _name ) const
    {
        const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( m_movieData, _name.c_str() );

        if( compositionData == nullptr )
        {
            return 0.f;
        }

        float duration = ae_get_movie_composition_data_frame_duration( compositionData );

        return duration;
    }
	//////////////////////////////////////////////////////////////////////////
	const aeMovieData * ResourceMovie2::getMovieData() const
	{
		return m_movieData;
	}
	//////////////////////////////////////////////////////////////////////////
	const aeMovieCompositionData * ResourceMovie2::getCompositionData( const ConstString & _name ) const
	{
		if( this->isCompile() == false )
		{
			return nullptr;
		}

		const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( m_movieData, _name.c_str() );

		if( compositionData == nullptr )
		{
			return nullptr;
		}
				
		return compositionData;
	}
	//////////////////////////////////////////////////////////////////////////
	void ResourceMovie2::setMovieInstance( const aeMovieInstance * _instance )
	{
		m_instance = _instance;
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceMovie2::_loader( const Metabuf::Metadata * _meta )
	{
        const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2 * metadata
            = static_cast<const Metacode::Meta_Data::Meta_DataBlock::Meta_ResourceMovie2 *>(_meta);
               
        m_filePath = metadata->get_File_Path();

        return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceMovie2::_compile()
	{
		if( ResourceReference::_compile() == false )
		{
			return false;
		}

		if( m_filePath.empty() == true )
		{
			LOGGER_ERROR("ResourceMovie::_compile: '%s' group '%s' don`t have Key Frames Pack Path"
				, this->getName().c_str()
				, this->getGroup().c_str()
				);

			return false;
		}

		const ConstString & category = this->getCategory();

		InputStreamInterfacePtr stream = FILE_SERVICE()
			->openInputFile( category, m_filePath, false );

		if( stream == nullptr )
		{
			LOGGER_ERROR("ResourceMovie2::_compile: '%s' group '%s' can`t open file '%s'"
				, this->getName().c_str()
				, this->getGroup().c_str()
				, m_filePath.c_str()
				);

			return false;
		}

		aeMovieData * movieData = ae_create_movie_data( m_instance, &Mengine_resource_provider, &Mengine_resource_deleter, this );

		aeMovieStream * movie_stream = ae_create_movie_stream( m_instance, &Mengine_read_stream, &Mengine_copy_stream, stream.get() );

        ae_uint32_t data_version;
        ae_result_t result = ae_load_movie_data( movieData, movie_stream, &data_version );

		if( result != AE_RESULT_SUCCESSFUL )
		{
			LOGGER_ERROR("ResourceMovie2::_compile: '%s' group '%s' invalid load data from file '%s' result '%s'\ncurrent version '%d'\nload version '%d'"
				, this->getName().c_str()
				, this->getGroup().c_str()
				, m_filePath.c_str()
                , ae_get_result_string_info( result )
                , AE_MOVIE_SDK_VERSION
                , data_version
				);

			return 0;
		}

		ae_delete_movie_stream( movie_stream );

		m_movieData = movieData;
		stream = nullptr;
	
		for( TVectorResources::const_iterator
			it = m_resources.begin(),
			it_end = m_resources.end();
		it != it_end;
		++it )
		{
			const ResourceReferencePtr & resource = *it;

			if( resource->compile() == false )
			{
				return false;
			}
		}
		
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void ResourceMovie2::_release()
	{
        for( TVectorResources::const_iterator
            it = m_resources.begin(),
            it_end = m_resources.end();
            it != it_end;
            ++it )
        {
            const ResourceReferencePtr & resource = *it;

            resource->release();
        }

		ae_delete_movie_data( m_movieData );

		ResourceReference::_release();
	}
    //////////////////////////////////////////////////////////////////////////
    bool ResourceMovie2::_isValid() const
    {
        if( m_filePath.empty() == true )
        {
            LOGGER_ERROR("ResourceMovie::_isValid: '%s' group '%s' don`t have Key Frames Pack Path"
                , this->getName().c_str()
                , this->getGroup().c_str()
                );

            return false;
        }

        const ConstString & category = this->getCategory();

        InputStreamInterfacePtr stream = FILE_SERVICE()
            ->openInputFile( category, m_filePath, false );

        if( stream == nullptr )
        {
            LOGGER_ERROR("ResourceMovie2::_isValid: '%s' group '%s' can`t open file '%s'"
                , this->getName().c_str()
                , this->getGroup().c_str()
                , m_filePath.c_str()
                );

            return false;
        }

        aeMovieStream * movie_stream = ae_create_movie_stream( m_instance, &Mengine_read_stream, &Mengine_copy_stream, stream.get() );

        ae_uint32_t check_version;
        ae_result_t check_result = ae_check_movie_data( movie_stream, &check_version );

        if( check_result != AE_RESULT_SUCCESSFUL )
        {
            const ae_char_t * result_string_info = ae_get_result_string_info( check_result );

            LOGGER_ERROR("ResourceMovie2::_isValid: '%s' group '%s' file '%s' check movie data invalid '%s'"
                , this->getName().c_str()
                , this->getGroup().c_str()
                , m_filePath.c_str()
                , result_string_info
                );

            return false;
        }

        ae_delete_movie_stream( movie_stream );

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	ResourceReference * ResourceMovie2::createResourceImage_( const ae_string_t _path, float _width, float _height )
	{
		ResourceImageDefaultPtr image = RESOURCE_SERVICE()
			->generateResourceT<ResourceImageDefaultPtr>( STRINGIZE_STRING_LOCAL( "ResourceImageDefault" ) );

		const ConstString & category = this->getCategory();

		image->setCategory( category );

		PathString full_path;

		ConstString folder = Helper::getPathFolder( m_filePath );

		full_path += folder.c_str();
		full_path += _path;

		FilePath c_path = Helper::stringizeFilePath( full_path );

		mt::uv4f uv_image;
		mt::uv4f uv_alpha;

		mt::vec2f size( _width, _height );

		image->setup( c_path, ConstString::none(), uv_image, uv_alpha, size );

		m_resources.push_back( image );

		return image.get();
	}
	//////////////////////////////////////////////////////////////////////////
	ResourceReference * ResourceMovie2::createResourceVideo_( const aeMovieResourceVideo * _resource )
	{
		ResourceVideoPtr video = RESOURCE_SERVICE()
			->generateResourceT<ResourceVideoPtr>( STRINGIZE_STRING_LOCAL( "ResourceVideo" ) );

		const ConstString & category = this->getCategory();

		video->setCategory( category );

		PathString full_path;

		ConstString folder = Helper::getPathFolder( m_filePath );

		full_path += folder.c_str();
		full_path += _resource->path;

		FilePath fullPath = Helper::stringizeFilePath( full_path );

		video->setFilePath( fullPath );

		video->setFrameRate( _resource->frameRate );
		video->setDuration( _resource->duration );

		if( _resource->alpha == AE_TRUE )
		{
			video->setAlpha( true );
			video->setCodecType( STRINGIZE_STRING_LOCAL( "ogvaVideo" ) );
		}
		else
		{
			video->setAlpha( false );
			video->setCodecType( STRINGIZE_STRING_LOCAL( "ogvVideo" ) );
		}

		m_resources.push_back( video );

		return video.get();
	}
	//////////////////////////////////////////////////////////////////////////
	ResourceReference * ResourceMovie2::createResourceSound_( const aeMovieResourceSound * _resource )
	{
		ResourceSoundPtr sound = RESOURCE_SERVICE()
			->generateResourceT<ResourceSoundPtr>( STRINGIZE_STRING_LOCAL( "ResourceSound" ) );

		const ConstString & category = this->getCategory();

		sound->setCategory( category );

		PathString full_path;

		ConstString folder = Helper::getPathFolder( m_filePath );

		full_path += folder.c_str();
		full_path += _resource->path;

		FilePath c_path = Helper::stringizeFilePath( full_path );
		
		sound->setFilePath( c_path );
		
		sound->setCodecType( STRINGIZE_STRING_LOCAL( "oggSound" ) );

		m_resources.push_back( sound );

		return sound.get();
	}
    //////////////////////////////////////////////////////////////////////////
    ResourceReference * ResourceMovie2::createResourceParticle_( const aeMovieResourceParticle * _resource )
    {
        ResourceParticlePtr particle = RESOURCE_SERVICE()
            ->generateResourceT<ResourceParticlePtr>( STRINGIZE_STRING_LOCAL( "ResourceParticle" ) );

        const ConstString & category = this->getCategory();

        particle->setCategory( category );

        PathString full_path;

        ConstString folder = Helper::getPathFolder( m_filePath );

        full_path += folder.c_str();
        full_path += _resource->path;

        FilePath c_path = Helper::stringizeFilePath( full_path );

        particle->setFilePath( c_path );

        for( ae_uint32_t index = 0; index != _resource->image_count; ++index )
        {
            const aeMovieResourceImage * movieResourceImage = _resource->images[index];

            ResourceImage * resourceImage = static_cast<ResourceImage *>(movieResourceImage->data);
            
            particle->addResourceImage( resourceImage );
        }

        m_resources.push_back( particle );

        return particle.get();
    }
}