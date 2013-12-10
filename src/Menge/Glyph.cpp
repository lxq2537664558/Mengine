#	include "ResourceGlyph.h"
#	include "Kernel/ResourceImplement.h"

#	include <cstdio>

#	include "Logger/Logger.h"
#	include "Core/String.h"

#	include "Interface/UnicodeInterface.h"


namespace Menge
{
    //////////////////////////////////////////////////////////////////////////
    Glyph::Glyph()
        : m_uv(0.f, 0.f, 0.f, 0.f)
        , m_offset(0.f, 0.f)
        , m_ratio(0.f)
        , m_size(0.f, 0.f)
    {
    }
	//////////////////////////////////////////////////////////////////////////
	Glyph::Glyph( const mt::vec4f & _uv, const mt::vec2f & _offset, float _ratio, const mt::vec2f & _size )
		: m_uv(_uv)
		, m_offset(_offset)
		, m_ratio(_ratio)
		, m_size(_size)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	void Glyph::addKerning( GlyphChar _char, float _kerning )
	{		
		m_kernings.insert( _char, _kerning );
	}
	//////////////////////////////////////////////////////////////////////////
	float Glyph::getKerning( GlyphChar _char ) const
	{
		const float * kerning;
		if( m_kernings.has( _char, &kerning ) == false )
		{
			return 0.f;
		}

		return *kerning;
	}
}