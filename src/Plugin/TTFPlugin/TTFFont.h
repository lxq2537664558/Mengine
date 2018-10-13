#pragma once

#include "TTFServiceInterface.h"

#include "Interface/MemoryInterface.h"
#include "Interface/TextInterface.h"

#include "Config/String.h"

#include "Kernel/Factorable.h"

#include "Kernel/Servant.h"

#include "Kernel/FontBase.h"

#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "math/uv4.h"

#include "Config/Map.h"

#include "fe/fe.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
#	define MENGINE_TTF_FONT_GLYPH_HASH_SIZE 37
    //////////////////////////////////////////////////////////////////////////
    class TTFFont
        : public FontBase
    {
    public:
        TTFFont();
        ~TTFFont() override;

    public:
        void setFTLibrary( FT_Library _library );

    public:
        bool initialize( const FileGroupInterfacePtr & _category, const IniUtil::IniStore & _ini ) override;

    protected:
        bool _compile() override;
        void _release() override;

    public:
        bool hasGlyph( GlyphCode _code ) const override;
        bool getGlyph( uint32_t _layout, GlyphCode _code, GlyphCode _next, Glyph * _glyph ) const override;

    protected:
        uint32_t getLayoutCount() const override;

    protected:
        float getFontAscent() const override;
        float getFontDescent() const override;
        float getFontHeight() const override;
        float getFontSpacing() const override;

    protected:
        bool getFontPremultiply() const override;

    protected:
        bool _validateGlyphes( const U32String & _codes ) const override;
        bool _prepareGlyph( GlyphCode _code ) override;

    protected:
        MemoryInterfacePtr m_memory;

        FT_Library m_ftlibrary;
        FT_Face m_face;

        FileGroupInterfacePtr m_category;
        FilePath m_ttfPath;

        float m_ttfAscender;
        float m_ttfDescender;
        uint32_t m_ttfHeight;
        float m_ttfSpacing;

        uint32_t m_ttfLayoutCount;

        FilePath m_ttfFEPath;
        ConstString m_ttfFEName;
        fe_bundle * m_ttfFEBundle;
        fe_effect * m_ttfFEEffect;
        const fe_node * m_ttfEffectNodes[FE_MAX_PINS];

        struct TTFGlyphQuad
        {
            mt::vec2f offset;
            mt::vec2f size;

            mt::uv4f uv;
            RenderTextureInterfacePtr texture;
        };

        struct TTFGlyph
        {
            uint32_t ch;

            float advance;

            TTFGlyphQuad quads[4];
        };

        typedef Vector<TTFGlyph> VectorTTFGlyphs;
        VectorTTFGlyphs m_glyphsHash[MENGINE_TTF_FONT_GLYPH_HASH_SIZE];

        uint32_t m_ttfFESample;

        class PFindGlyph;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<TTFFont> TTFFontPtr;
    //////////////////////////////////////////////////////////////////////////
}