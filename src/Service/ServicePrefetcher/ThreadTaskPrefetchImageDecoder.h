#	pragma once

#	include "Interface/RenderSystemInterface.h"
#	include "Interface/StreamInterface.h"
#	include "Interface/FileSystemInterface.h"

#	include "Codec/ImageDecoderMemory.h"

#	include "Kernel/ThreadTask.h"

#	include "Core/MemoryCacheInput.h"

namespace Menge
{
	class ThreadTaskPrefetchImageDecoder
		: public ThreadTask
	{
	public:
		ThreadTaskPrefetchImageDecoder();
	
	public:
		void setServiceProvider( ServiceProviderInterface * _serviceProvider );

	public:
		void initialize( const ConstString& _pakName, const FilePath & _fileName, const ConstString & _codec );
		
	public:
		const ImageDecoderInterfacePtr & getDecoder() const;

	protected:
		bool _onRun() override;

	protected:
		bool _onMain() override;

	protected:
		void _onComplete( bool _successful ) override;

	protected:
		ServiceProviderInterface * m_serviceProvider;

		ConstString m_pakName;
		FilePath m_fileName; 
		ConstString m_codec;
				
	protected:
		FileGroupInterfacePtr m_group;
		InputStreamInterfacePtr m_stream;
		ImageDecoderInterfacePtr m_imageDecoder;

		MemoryCacheInputPtr m_memoryCache;

	protected:
		ImageDecoderInterfacePtr m_imageDecoderMemory;
	};

	typedef stdex::intrusive_ptr<ThreadTaskPrefetchImageDecoder> ThreadTaskPrefetchImageDecoderPtr;
}