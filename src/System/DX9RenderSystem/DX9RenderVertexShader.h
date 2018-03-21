#pragma once

#include "Interface/RenderSystemInterface.h"
#include "Interface/MemoryInterface.h"

#include "Core/ServantBase.h"

#include <d3d9.h>

namespace Mengine
{
	class DX9RenderVertexShader
		: public ServantBase<RenderVertexShaderInterface>
	{
	public:
		DX9RenderVertexShader();
		~DX9RenderVertexShader() override;

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

		IDirect3DVertexShader9 * m_shader;
	};
	//////////////////////////////////////////////////////////////////////////
	typedef stdex::intrusive_ptr<DX9RenderVertexShader> DX9RenderVertexShaderPtr;
}