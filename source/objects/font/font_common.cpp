#include "common/runtime.h"
#include "objects/font/font.h"

using namespace love;

namespace
{
    struct Word
    {
        std::string_view text;
        float width;
        u32 line;
    };

    std::vector<Word> words;
}

void Font::GetCodepointsFromString(const std::string & text, Codepoints & codepoints)
{
    codepoints.reserve(text.size());

    auto currentChar = text.c_str();
    const auto end = currentChar + text.size();

    while (currentChar != end)
    {
        uint32_t codepoint;
        const auto bytes = decode_utf8(&codepoint, (uint8_t *)currentChar);

        if (bytes < 0)
            continue;

        codepoints.push_back(codepoint);

        currentChar += bytes;
    }
}

void Font::GetCodepointsFromString(const std::vector<ColoredString> & strings, ColoredCodepoints & codepoints)
{
    if (strings.empty())
        return;

    codepoints.points.reserve(strings[0].string.size());

    for (const ColoredString & str : strings)
    {
        // Don't add empty string colors
        if (str.string.size() == 0)
            continue;

        IndexedColor clr = {str.color, (int)codepoints.points.size()};
        codepoints.colors.push_back(clr);

        Font::GetCodepointsFromString(str.string, codepoints.points);
    }

    if (codepoints.colors.size() == 1)
    {
        IndexedColor clr = codepoints.colors[0];

        if (clr.index == 0 && clr.color == Color(1.0f, 1.0f, 1.0f, 1.0f))
            codepoints.colors.pop_back();
    }
}

void Font::Print(const std::vector<Font::ColoredString> & text, const DrawArgs & args, const Color & blend)
{
    ColoredCodepoints codepoints;
    Font::GetCodepointsFromString(text, codepoints);

    this->GenerateVertices(codepoints, blend);
}

void Font::PrintF(const std::vector<Font::ColoredString> & text, float wrap, Font::AlignMode align, Color & blend)
{
    ColoredCodepoints codepoints;
    Font::GetCodepointsFromString(text, codepoints);

    // this->GenerateVertices - Create an array/vector of things to draw
    // this->PrintV - Display text on screen after calling GenerateVertices
}

std::vector<TextInfoCommand> Font::GenerateVertices(const ColoredCodepoints & codepoints, const Color & blend, float extraSpacing, Vector2 offset, TextInfo * info)
{
    float dx = offset.x;
    float dy = offset.y;

    float heightOffset = 0.0f;
    int maxWidth = 0;

    std::vector<TextInfoCommand> commands;

    for (int i = 0; i < (int)codepoints.points.size(); i++)
    {
        uint32_t glyph = codepoints.points[i];
        uint8_t out[5];
        auto bytes = encode_utf8(out, glyph);

        if (bytes < 0)
            bytes = encode_utf8(out, 0xFFFD);

        if (glyph == '\n')
        {
            if (dx > maxWidth)
                maxWidth = (int)dx;

            // Wrap newlines, don't print them
            dy += std::floor(this->GetHeight() * 1.0f + 0.5f);
            dx = offset.x;

            // Add to vector

            continue;
        }

        if (glyph == '\r')
            continue;

        dx += this->_GetGlyphWidth(glyph);

        // commands.back().str.append((char *)out, bytes);
    }

    if (dx > maxWidth)
        maxWidth = (int)dx;

    if (!line.empty())
    {
        // Draw Text
    }
}

void Font::Print(const std::vector<Font::ColoredString> & strings, const DrawArgs & args, float * limit, const Color & blend, Font::AlignMode align)
{
    float currentSize = 0.0f;
    auto offset = std::make_pair(0.0f, 0.0f);
    std::string line;

    for (size_t i = 0; i < strings.size(); i++)
    {
        const std::string & str = strings[i].string;
        auto currentChar = str.c_str();
        const auto end = currentChar + str.size();
        const Color & clr = strings[i].color;

        Word word;

        while (currentChar != end)
        {
            uint32_t codepoint;
            const auto bytes = decode_utf8(&codepoint, (uint8_t *)currentChar);

            if (codepoint == '\n' || (limit != nullptr && currentSize >= *limit))
            {
                auto size = this->GenerateVertices(line, offset, args, blend, clr);

                offset.first = 0.0f;
                offset.second += size.second;
            }

            if (codepoint == '\n')
                line.clear();
            else
            {
                if (limit != nullptr && currentSize >= *limit)
                {
                    currentSize = 0.0f;
                    line.clear();
                }

                line.append(currentChar, bytes);
            }

            currentChar += bytes;
        }

        if (!line.empty())
        {
            auto size = this->GenerateVertices(line, offset, args, blend, clr);
            offset.first += size.first;

            line.clear();
        }
    }
}

bool Font::GetConstant(const char * in, AlignMode & out)
{
    return alignModes.Find(in, out);
}

bool Font::GetConstant(AlignMode in, const char *& out)
{
    return alignModes.Find(in, out);
}

std::vector<std::string> Font::GetConstants(AlignMode)
{
    return alignModes.GetNames();
}

bool Font::GetConstant(const char * in, Font::SystemFontType & out)
{
    return sharedFonts.Find(in, out);
}

bool Font::GetConstant(Font::SystemFontType in, const char *& out)
{
    return sharedFonts.Find(in, out);
}

std::vector<std::string> Font::GetConstants(Font::SystemFontType)
{
    return sharedFonts.GetNames();
}

StringMap<Font::AlignMode, Font::ALIGN_MAX_ENUM>::Entry Font::alignModeEntries[] =
{
    { "left",    ALIGN_LEFT    },
    { "right",   ALIGN_RIGHT   },
    { "center",  ALIGN_CENTER  },
    { "justify", ALIGN_JUSTIFY }
};

StringMap<Font::AlignMode, Font::ALIGN_MAX_ENUM> Font::alignModes(Font::alignModeEntries, sizeof(Font::alignModeEntries));
