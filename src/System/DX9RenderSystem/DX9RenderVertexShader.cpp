#   include "DX9RenderVertexShader.h"

#	include "DX9ErrorHelper.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	DX9RenderVertexShader::DX9RenderVertexShader()
		: m_shader( nullptr )
	{
	}
	//////////////////////////////////////////////////////////////////////////
	DX9RenderVertexShader::~DX9RenderVertexShader()
	{
		if( m_shader != nullptr )
		{
			m_shader->Release();
			m_shader = nullptr;
		}		
	}
	//////////////////////////////////////////////////////////////////////////
	const ConstString & DX9RenderVertexShader::getName() const
	{
		return m_name;
	}
	//////////////////////////////////////////////////////////////////////////
	bool DX9RenderVertexShader::initialize( const ConstString & _name, const MemoryInterfacePtr & _memory )
	{
		m_name = _name;

		m_memory = _memory;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool DX9RenderVertexShader::compile( IDirect3DDevice9 * _pD3DDevice )
	{
		const DWORD * dx_source = m_memory->getMemory();

		IF_DXCALL( _pD3DDevice, CreateVertexShader, (dx_source, &m_shader) )
		{
			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool DX9RenderVertexShader::enable( IDirect3DDevice9 * _pD3DDevice )
	{
		IF_DXCALL( _pD3DDevice, SetVertexShader, (m_shader) )
		{
			return false;
		}

		return true;
	}
}