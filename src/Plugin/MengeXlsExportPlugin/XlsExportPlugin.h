#	pragma once

#	include "Interface/PluginInterface.h"
#	include "Interface/CodecInterface.h"

namespace Menge
{
	class DecoderFactoryInterface;

	class XlsExportPlugin
		: public PluginInterface
	{
	public:
		XlsExportPlugin();

	protected:
		bool initialize( ServiceProviderInterface * _serviceProvider ) override;
		void finalize() override;

	protected:
		void destroy() override;

	protected:
		bool proccess_( const char * _projectName );

    protected:
        ServiceProviderInterface * m_serviceProvider;
	};
}