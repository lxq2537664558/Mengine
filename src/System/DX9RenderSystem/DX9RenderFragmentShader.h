#pragma once

#include "Interface/RenderSystemInterface.h"
#include "Interface/MemoryInterface.h"

#include "Core/ServantBase.h"

#include <d3d9.h>

namespace Mengine
{
    class DX9RenderFragmentShader
        : public ServantBase<RenderFragmentShaderInterface>
    {
    public:
		DX9RenderFragmentShader();
		~DX9RenderFragmentShader() override;

	public:
		const ConstString & getName() const override;

	public:
		bool initialize( const ConstString & _name, const MemoryInterfacePtr & _memory );

	public:
		bool compile( IDirect3DDevice9 * _pD3DDevice );

	public:
		bool enable( IDirect3DDevice9 * _pD3DDevice );

	protected:
		ConstString m_name;

		MemoryInterfacePtr m_memory;

		IDirect3DPixelShader9 * m_shader;
    };
	//////////////////////////////////////////////////////////////////////////
	typedef stdex::intrusive_ptr<DX9RenderFragmentShader> DX9RenderFragmentShaderPtr;
}