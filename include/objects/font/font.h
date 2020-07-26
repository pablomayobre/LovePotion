#pragma once

#include "common/exception.h"
#include "common/mmath.h"
#include "common/stringmap.h"
#include "common/vector.h"

#if defined (_3DS)
    #define FONT_DEFAULT_SIZE 22.5f;
#elif defined (__SWITCH__)
    #define FONT_DEFAULT_SIZE 14.0f;
#endif

namespace love
{
    class Font : public Object
    {
        public:
            typedef std::vector<uint32_t> Codepoints;

            struct ColoredString
            {
                std::string string;
                Color color;
            };

            enum AlignMode
            {
                ALIGN_LEFT,
                ALIGN_CENTER,
                ALIGN_RIGHT,
                ALIGN_JUSTIFY,
                ALIGN_MAX_ENUM
            };

            struct IndexedColor
            {
                Color color;
                int index;
            };

            struct ColoredCodepoints
            {
                std::vector<uint32_t> points;
                std::vector<IndexedColor> colors;
            };

            struct TextInfo
            {
                int width;
                int height;
            };

            struct GlyphMetrics
            {
                int width;
                int height;

                int advance;

                int bearingX;
                int bearingY;

                int page;
            };

            struct TextInfoCommand
            {
                std::string str;
                Color color;

                float x;
                float y;
            };

            #if defined (_3DS)
                enum SystemFontType
                {
                    TYPE_STANDARD  = CFG_REGION_USA,
                    TYPE_CHINESE   = CFG_REGION_CHN,
                    TYPE_TAIWANESE = CFG_REGION_TWN,
                    TYPE_KOREAN    = CFG_REGION_KOR,
                    TYPE_MAX_ENUM
                };
            #elif defined (__SWITCH__)
                enum SystemFontType
                {
                    TYPE_STANDARD               = PlSharedFontType_Standard,
                    TYPE_CHINESE_SIMPLIFIED     = PlSharedFontType_ChineseSimplified,
                    TYPE_CHINESE_TRADITIONAL    = PlSharedFontType_ChineseTraditional,
                    TYPE_CHINESE_SIMPLIFIED_EXT = PlSharedFontType_ExtChineseSimplified,
                    TYPE_KOREAN                 = PlSharedFontType_KO,
                    TYPE_NINTENDO_EXTENDED      = PlSharedFontType_NintendoExt,
                    TYPE_MAX_ENUM
                };
            #endif

            static love::Type type;

            static constexpr float DEFAULT_SIZE = FONT_DEFAULT_SIZE;

            Font(SystemFontType type, float size = DEFAULT_SIZE);
            Font(Data * data, float size = DEFAULT_SIZE);
            Font(float size = DEFAULT_SIZE);
            ~Font();

            void Print(const std::vector<ColoredString> & text, const DrawArgs & args, float * limit, const Color & color, AlignMode align);

            void Print(const std::vector<ColoredString> & text, const Color & blend);
            void PrintF(const std::vector<Font::ColoredString> & text, float wrap, AlignMode align, const Color & blend);

            void ClearBuffer();


            float GetWidth(const char * text);
            float GetHeight();

            float _GetGlyphWidth(u16 glyph);

            float GetSize();

            static bool GetConstant(const char * in, AlignMode & out);
            static bool GetConstant(AlignMode in, const char *& out);
            static std::vector<std::string> GetConstants(AlignMode);

            static bool GetConstant(const char * in, SystemFontType & out);
            static bool GetConstant(SystemFontType in, const char *& out);
            static std::vector<std::string> GetConstants(SystemFontType);

            static void GetCodepointsFromString(const std::string & text, Codepoints & codepoints);
            static void GetCodepointsFromString(const std::vector<ColoredString> & strings, ColoredCodepoints & codepoints);

            std::vector<TextInfoCommand> GenerateVertices(const ColoredCodepoints & codepoints, const DrawArgs & args, const Color & blend, float extraSpacing = 0.0f, Vector2 offset = {}, TextInfo * info = nullptr);

        private:
            FontHandle font;
            TextBuffer buffer;
            float size;
            TextHandle text;

            float lineHeight;
            TextureHandle texture;

            void PrintVertices(const std::string & line, const Vector2 & offset, const DrawArgs & args, const Color & color);

            std::pair<float, float> GenerateVertices(const std::string & line, const std::pair<float, float> & offset, const DrawArgs & args, const Color & blend, const Color & color);

            float GetScale() { return this->size / 30.0f; }

            static StringMap<AlignMode, ALIGN_MAX_ENUM>::Entry alignModeEntries[];
            static StringMap<AlignMode, ALIGN_MAX_ENUM> alignModes;

            static StringMap<SystemFontType, SystemFontType::TYPE_MAX_ENUM>::Entry sharedFontEntries[];
            static StringMap<SystemFontType, SystemFontType::TYPE_MAX_ENUM> sharedFonts;
    };
}
