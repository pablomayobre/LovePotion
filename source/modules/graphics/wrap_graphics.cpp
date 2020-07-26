#include "common/runtime.h"
#include "modules/graphics/wrap_graphics.h"

using namespace love;

#define instance() (Module::GetInstance<Graphics>(Module::M_GRAPHICS))
#define WINDOW_MODULE() (Module::GetInstance<Window>(Module::M_WINDOW))

int Wrap_Graphics::SetDepth(lua_State * L)
{
    float depth = luaL_checknumber(L, 1);

    Luax::CatchException(L, [&]() {
        instance()->SetDepth(depth);
    });

    return 0;
}

int Wrap_Graphics::Arc(lua_State * L)
{
    const char * mode = luaL_checkstring(L, 1);
    Graphics::DrawMode drawMode = Graphics::DRAW_FILL;

    if (!Graphics::GetConstant(mode, drawMode))
        return luaL_error(L, "Invalid draw mode %s", mode);

    float x = luaL_optnumber(L, 2, 0);
    float y = luaL_optnumber(L, 3, 0);

    float radius = luaL_checknumber(L, 4);

    float startAngle = luaL_checknumber(L, 5);
    float endAngle = luaL_checknumber(L, 6);

    instance()->Transform(&x, &y);
    instance()->TransformScale(&radius, nullptr);

    Luax::CatchException(L, [&]() {
        instance()->Arc(mode, x, y, radius, startAngle, endAngle);
    });

    return 0;
}

int Wrap_Graphics::Ellipse(lua_State * L)
{
    const char * mode = luaL_checkstring(L, 1);
    Graphics::DrawMode drawMode = Graphics::DRAW_FILL;

    if (!Graphics::GetConstant(mode, drawMode))
        return luaL_error(L, "Invalid draw mode %s", mode);

    float x = luaL_optnumber(L, 2, 0);
    float y = luaL_optnumber(L, 3, 0);

    float radiusX = luaL_checknumber(L, 4);
    float radiusY = luaL_checknumber(L, 5);

    instance()->Transform(&x, &y);
    instance()->TransformScale(&radiusX, &radiusY);

    Luax::CatchException(L, [&]() {
        instance()->Ellipse(mode, x, y, radiusX, radiusY);
    });

    return 0;
}

int Wrap_Graphics::Rectangle(lua_State * L)
{
    const char * mode = luaL_checkstring(L, 1);
    Graphics::DrawMode drawMode = Graphics::DRAW_FILL;

    if (!Graphics::GetConstant(mode, drawMode))
        return luaL_error(L, "Invalid draw mode %s", mode);

    float x = luaL_optnumber(L, 2, 0);
    float y = luaL_optnumber(L, 3, 0);

    float width = luaL_checknumber(L, 4);
    float height = luaL_checknumber(L, 5);

    float rx = luaL_optnumber(L, 6, 1);
    float ry = luaL_optnumber(L, 7, 1);

    instance()->Transform(&x, &y);
    instance()->TransformScale(&width, &height);

    Luax::CatchException(L, [&]() {
        instance()->Rectangle(mode, x, y, width, height, rx, ry);
    });

    return 0;
}

int Wrap_Graphics::Circle(lua_State * L)
{
    const char * mode = luaL_checkstring(L, 1);
    Graphics::DrawMode drawMode = Graphics::DRAW_FILL;

    if (!Graphics::GetConstant(mode, drawMode))
        return luaL_error(L, "Invalid draw mode %s", mode);

    float x = luaL_optnumber(L, 2, 0);
    float y = luaL_optnumber(L, 3, 0);

    float radius = luaL_optnumber(L, 4, 1);

    instance()->Transform(&x, &y);
    instance()->TransformScale(&radius, nullptr);

    Luax::CatchException(L, [&]() {
        instance()->Circle(x, y, radius);
    });

    return 0;
}

int Wrap_Graphics::Line(lua_State * L)
{
    int argc = lua_gettop(L);
    bool isTable = lua_istable(L, 1);
    int tableLength = 0;

    Graphics::Point start, end = Graphics::Point();

    if (isTable)
    {
        tableLength= lua_objlen(L, 1);

        if (tableLength == 0 || (tableLength > 0 && tableLength % 4) != 0)
            return luaL_error(L, "Need at least two verticies to draw a line");

        if ((tableLength % 4) == 0)
        {
            for (int outer = 0; outer < tableLength; outer += 4)
            {
                for (int inner = 1; inner <= 4; inner++)
                {
                    lua_rawgeti(L, 1, inner + outer);

                    start.x = luaL_checknumber(L, -4);
                    start.y = luaL_checknumber(L, -3);

                    end.x = luaL_checknumber(L, -2);
                    end.y = luaL_checknumber(L, -1);

                    lua_pop(L, 4);

                    instance()->Transform(&start.x, &start.y);
                    instance()->Transform(&end.x, &end.y);

                    Luax::CatchException(L, [&]() {
                        instance()->Line(start.x, start.y, end.x, end.y);
                    });
                }
            }
        }
    }
    else
    {
        if ((argc % 4) != 0)
            return luaL_error(L, "Need at least two verticies to draw a line");

        for (int index = 0; index < argc; index += 4)
        {
            start.x = luaL_checknumber(L, index + 1);
            start.y = luaL_checknumber(L, index + 2);

            end.x = luaL_checknumber(L, index + 3);
            end.y = luaL_checknumber(L, index + 4);

            instance()->Transform(&start.x, &start.y);
            instance()->Transform(&end.x, &end.y);

            Luax::CatchException(L, [&]() {
                instance()->Line(start.x, start.y, end.x, end.y);
            });
        }
    }

    return 0;
}

int Wrap_Graphics::Polygon(lua_State * L)
{
    int argc = lua_gettop(L) - 1;
    const char * mode = luaL_checkstring(L, 1);
    Graphics::DrawMode drawMode = Graphics::DRAW_FILL;

    if (!Graphics::GetConstant(mode, drawMode))
        return luaL_error(L, "Invalid draw mode %s", mode);

    bool isTable = false;
    if (argc == 1 && lua_istable(L, 2))
    {
        argc = (int)lua_objlen(L, 2);
        isTable = true;
    }

    if (argc % 2 != 0)
        return luaL_error(L, "Number of vertex components must be a multiple of two");
    else if (argc < 6)
        return luaL_error(L, "Need at least three vertices to draw a polygon");

    const int numverticies = argc / 2;

    std::vector<Graphics::Point> points(numverticies);

    if (isTable)
    {
        float x = 0;
        float y = 0;

        for (int i = 0; i < numverticies; ++i)
        {
            lua_rawgeti(L, 2, (i * 2) + 1);
            lua_rawgeti(L, 2, (i * 2) + 2);

            x = luaL_checkinteger(L, -2);
            y = luaL_checkinteger(L, -1);

            instance()->Transform(&x, &y);

            points[i] = {x, y};

            lua_pop(L, 2);
        }
    }
    else
    {
        float x = 0;
        float y = 0;

        for (int i = 0; i < numverticies; ++i)
        {
            x = luaL_checkinteger(L, (i * 2) + 2);
            y = luaL_checkinteger(L, (i * 2) + 3);

            instance()->Transform(&x, &y);

            points[i] = {x, y};

            lua_pop(L, 2);
        }
    }

    Luax::CatchException(L, [&]() {
        instance()->Polygon(mode, points);
    });

    return 0;
}

int Wrap_Graphics::SetLineWidth(lua_State * L)
{
    float width = luaL_checknumber(L, 1);

    instance()->SetLineWidth(width);

    return 0;
}

int Wrap_Graphics::GetLineWidth(lua_State * L)
{
    lua_pushnumber(L, instance()->GetLineWidth());

    return 1;
}

int Wrap_Graphics::Push(lua_State * L)
{
    Luax::CatchException(L, [&]() {
        instance()->Push();
    });

    return 0;
}

int Wrap_Graphics::Origin(lua_State  *L)
{
    instance()->Origin();

    return 0;
}

int Wrap_Graphics::Translate(lua_State * L)
{
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);

    instance()->Translate(x, y);

    return 0;
}

int Wrap_Graphics::Rotate(lua_State * L)
{
    float angle = luaL_checknumber(L, 1);

    instance()->Rotate(angle);

    return 0;
}

int Wrap_Graphics::Scale(lua_State * L)
{
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);

    instance()->Scale(x, y);

    return 0;
}

int Wrap_Graphics::Pop(lua_State * L)
{
    Luax::CatchException(L, [&]() {
        instance()->Pop();
    });

    return 0;
}

int Wrap_Graphics::Clear(lua_State * L)
{
    Color clearColor = { 0, 0, 0, 0 };

    clearColor.r = luaL_checknumber(L, 1);
    clearColor.g = luaL_checknumber(L, 2);
    clearColor.b = luaL_checknumber(L, 3);
    clearColor.a = luaL_optnumber(L, 4, 1.0f);

    instance()->CURRENT_DEPTH = 0.0f;

    clearColor.Adjust();
    WINDOW_MODULE()->Clear(&clearColor);

    return 0;
}

int Wrap_Graphics::GetWidth(lua_State * L)
{
    int width = 0;

    Luax::CatchException(L, [&]() {
        instance()->GetDimensions(&width, nullptr);
    });

    lua_pushnumber(L, width);

    return 1;
}

int Wrap_Graphics::GetHeight(lua_State * L)
{
    int height = 0;

    Luax::CatchException(L, [&]() {
        instance()->GetDimensions(nullptr, &height);
    });

    lua_pushnumber(L, height);

    return 1;
}

int Wrap_Graphics::GetDimensions(lua_State * L)
{
    int width = 0;
    int height = 0;

    Luax::CatchException(L, [&]() {
        instance()->GetDimensions(&width, &height);
    });

    lua_pushnumber(L, width);
    lua_pushnumber(L, height);

    return 2;
}

int Wrap_Graphics::Present(lua_State * L)
{
    WINDOW_MODULE()->Present();

    instance()->GetFont()->ClearBuffer();

    return 0;
}

int Wrap_Graphics::SetScissor(lua_State * L)
{
    bool enabled = lua_gettop(L) == 0;

    if (enabled)
    {
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);

        float width = luaL_checknumber(L, 3);
        float height = luaL_checknumber(L, 4);

        instance()->SetScissor(x, y, width, height);
    }

    Rect scissor = instance()->GetScissor();
    Primitives::Scissor(enabled, scissor.x, scissor.y, scissor.w, scissor.h);

    return 0;
}

int Wrap_Graphics::GetScissor(lua_State * L)
{
    Rect scissor = instance()->GetScissor();

    lua_pushnumber(L, scissor.x);
    lua_pushnumber(L, scissor.y);
    lua_pushnumber(L, scissor.w);
    lua_pushnumber(L, scissor.h);

    return 4;
}

int Wrap_Graphics::SetDefaultFilter(lua_State * L)
{
    Texture::Filter filter;

    const char * min = luaL_checkstring(L, 1);
    const char * mag = luaL_optstring(L, 2, min);

    if (!Texture::GetConstant(min, filter.min))
        return Luax::EnumError(L, "filter mode", Texture::GetConstants(filter.min), min);

    if (!Texture::GetConstant(mag, filter.mag))
        return Luax::EnumError(L, "filter mode", Texture::GetConstants(filter.mag), mag);

    instance()->SetDefaultFilter(filter);

    return 0;
}

int Wrap_Graphics::GetDefaultFilter(lua_State * L)
{
    const Texture::Filter & filter = instance()->GetDefaultFilter();

    const char * min;
    const char * mag;

    if (!Texture::GetConstant(filter.min, min))
        return luaL_error(L, "Unknown minification filter mode.");

    if (!Texture::GetConstant(filter.mag, mag))
        return luaL_error(L, "Unknown magnification filter mode.");

    lua_pushstring(L, min);
    lua_pushstring(L, mag);

    return 2;
}

int Wrap_Graphics::NewImage(lua_State * L)
{
    std::string path = luaL_checkstring(L, 1);
    StrongReference<Image> image;

    if (Wrap_Filesystem::CanGetData(L, 1))
    {
        Luax::CatchException(L, [&]() {
            Data * data = Wrap_Filesystem::GetData(L, 1);

            image.Set(instance()->NewImage(data), Acquire::NORETAIN);
        });
    }

    Luax::PushType(L, image);

    return 1;
}

int Wrap_Graphics::NewFont(lua_State * L)
{
    Font * font = nullptr;

    if (lua_isnumber(L, 1))
    {
        float size = luaL_checknumber(L, 1);

        Luax::CatchException(L, [&]() {
            font = instance()->NewFont(size);
        });
    }
    else
    {
        if (Wrap_Filesystem::CanGetData(L, 1))
        {
            const char * string = luaL_checkstring(L, 1);
            float size = luaL_optnumber(L, 2, Font::DEFAULT_SIZE);

            std::filesystem::path filepath(string);

            // Check for enum value
            if (filepath.extension() == "")
            {
                Font::SystemFontType type = Font::SystemFontType::TYPE_STANDARD;

                if (!Font::GetConstant(string, type))
                    return Luax::EnumError(L, "font type", Font::GetConstants(type), string);

                Luax::CatchException(L, [&]() {
                    font = instance()->NewFont(type, size);
                });
            }
            else //check for custom font
            {
                Luax::CatchException(L, [&]() {
                    font = instance()->NewFont(Wrap_Filesystem::GetData(L, 1), size);
                });
            }
        }
    }

    Luax::PushType(L, font);
    font->Release();

    return 1;
}

int Wrap_Graphics::NewQuad(lua_State * L)
{
    Quad::Viewport v;

    v.x = luaL_checknumber(L, 1);
    v.y = luaL_checknumber(L, 2);

    v.w = luaL_checknumber(L, 3);
    v.h = luaL_checknumber(L, 4);

    double sw = 0.0;
    double sh = 0.0;

    if (Luax::IsType(L, 5, Texture::type))
    {
        Texture * texture = Wrap_Texture::CheckTexture(L, 5);

        sw = texture->GetWidth();
        sh = texture->GetHeight();
    }
    else
    {
        sw = luaL_checknumber(L, 5);
        sh = luaL_checknumber(L, 6);
    }

    Quad * quad = instance()->NewQuad(v, sw, sh);

    Luax::PushType(L, quad);
    quad->Release();

    return 1;
}

int Wrap_Graphics::Draw(lua_State * L)
{
    Drawable * drawable = nullptr;
    Texture * texture = nullptr;
    Quad * quad = nullptr;

    int start = 2;

    if (Luax::IsType(L, 2, Quad::type))
    {
        texture = Wrap_Texture::CheckTexture(L, 1);
        quad = Luax::ToType<Quad>(L, 2);

        start = 3;
    }
    else if (lua_isnil(L, 2) && !lua_isnoneornil(L, 3))
        return Luax::TypeErrror(L, 2, "Quad");
    else
    {
        drawable = Luax::CheckType<Drawable>(L, 1);
        start = 2;
    }

    DrawArgs args;

    args.x = luaL_optnumber(L, start, 0);
    args.y = luaL_optnumber(L, start + 1, 0);

    args.r = luaL_optnumber(L, start + 2, 0);

    args.scalarX = luaL_optnumber(L, start + 3, 1);
    args.scalarY = luaL_optnumber(L, start + 4, 1);

    args.offsetX = luaL_optnumber(L, start + 5, 0);
    args.offsetY = luaL_optnumber(L, start + 6, 0);

    args.depth = instance()->CURRENT_DEPTH;

    instance()->Transform(&args);

    Luax::CatchException(L, [&]() {
        if (texture && quad)
            instance()->Draw(texture, quad, args);
        else
            instance()->Draw(drawable, args);
    });

    return 0;
}

int Wrap_Graphics::Print(lua_State * L)
{
    std::vector<Font::ColoredString> string;
    Wrap_Font::CheckColoredString(L, 1, string);

    DrawArgs args;

    args.x = luaL_optnumber(L, 2, 0);
    args.y = luaL_optnumber(L, 3, 0);

    args.depth = Graphics::CURRENT_DEPTH;

    instance()->Transform(&args);

    if (Luax::IsType(L, 2, Font::type))
    {
        Font * font = Wrap_Font::CheckFont(L, 2);

        Luax::CatchException(L, [&]() {
            instance()->Print(string, font, args);
        });
    }
    else
    {
        Luax::CatchException(L, [&]() {
            instance()->Print(string, args);
        });
    }

    return 0;
}

int Wrap_Graphics::PrintF(lua_State * L)
{
    std::vector<Font::ColoredString> string;
    Wrap_Font::CheckColoredString(L, 1, string);

    DrawArgs args;

    args.x = luaL_optnumber(L, 2, 0);
    args.y = luaL_optnumber(L, 3, 0);

    args.depth = Graphics::CURRENT_DEPTH;

    instance()->Transform(&args);

    int start = 2;

    Font * font = nullptr;
    if (Luax::IsType(L, 2, Font::type))
    {
        font = Wrap_Font::CheckFont(L, 2);
        start++;
    }

    float wrap = luaL_checknumber(L, start + 2);

    Font::AlignMode mode = Font::ALIGN_LEFT;
    const char * alignment = lua_isnoneornil(L, start + 3) ? nullptr : luaL_checkstring(L, start + 3);

    if (alignment != nullptr && !Font::GetConstant(alignment, mode))
        return Luax::EnumError(L, "alignment", Font::GetConstants(mode), alignment);

    Luax::CatchException(L, [&]() {
        instance()->PrintF(string, args, wrap, mode);
    });

    return 0;
}

int Wrap_Graphics::SetNewFont(lua_State * L)
{
    int ret = Wrap_Graphics::NewFont(L);
    Font * font = Luax::CheckType<Font>(L, -1);

    instance()->SetFont(font);

    return ret;
}

int Wrap_Graphics::SetFont(lua_State * L)
{
    Font * font = Luax::CheckType<Font>(L, 1);

    instance()->SetFont(font);

    return 0;
}

int Wrap_Graphics::GetFont(lua_State * L)
{
    Font * font = nullptr;
    Luax::CatchException(L, [&]() {
        font = instance()->GetFont();
    });

    Luax::PushType(L, font);

    return 1;
}

int Wrap_Graphics::Reset(lua_State * L)
{
    instance()->Reset();

    return 0;
}

int Wrap_Graphics::GetBackgroundColor(lua_State * L)
{
    Color background = instance()->GetBackgroundColor();

    lua_pushnumber(L, background.r);
    lua_pushnumber(L, background.g);
    lua_pushnumber(L, background.b);

    return 3;
}

int Wrap_Graphics::SetBackgroundColor(lua_State * L)
{
    Color background = {0.0f, 0.0f, 0.0f, 0.0f};

    if (lua_istable(L, 1))
    {
        for (int i = 1; i <= 4; i++)
            lua_rawgeti(L, 1, i);

        background.r = luaL_checknumber(L, -4);
        background.g = luaL_checknumber(L, -3);
        background.b = luaL_checknumber(L, -2);
        background.a = luaL_optnumber(L, -1, 1.0f);
    }
    else if (lua_isnumber(L, 1))
    {
        background.r = luaL_checknumber(L, 1);
        background.g = luaL_checknumber(L, 2);
        background.b = luaL_checknumber(L, 3);
        background.a = luaL_optnumber(L, 4, 1.0f);
    }

    instance()->SetBackgroundColor(background);

    return 0;
}

int Wrap_Graphics::GetColor(lua_State * L)
{
    Color foreground = instance()->GetColor();

    lua_pushnumber(L, foreground.r);
    lua_pushnumber(L, foreground.g);
    lua_pushnumber(L, foreground.b);
    lua_pushnumber(L, foreground.a);

    return 4;
}

int Wrap_Graphics::SetColor(lua_State * L)
{
    Color foreground = {0.0f, 0.0f, 0.0f, 0.0f};

    if (lua_istable(L, 1))
    {
        for (int i = 1; i <= 4; i++)
            lua_rawgeti(L, 1, i);

        foreground.r = luaL_checknumber(L, -4);
        foreground.g = luaL_checknumber(L, -3);
        foreground.b = luaL_checknumber(L, -2);
        foreground.a = luaL_optnumber(L, -1, 1.0f);
    }
    else if (lua_isnumber(L, 1))
    {
        foreground.r = luaL_checknumber(L, 1);
        foreground.g = luaL_checknumber(L, 2);
        foreground.b = luaL_checknumber(L, 3);
        foreground.a = luaL_optnumber(L, 4, 1.0f);
    }

    instance()->SetColor(foreground);

    return 0;
}

int Wrap_Graphics::Register(lua_State * L)
{
    luaL_Reg reg[] =
    {
        { "circle",             Circle             },
        { "clear",              Clear              },
        { "draw",               Draw               },
        { "getBackgroundColor", GetBackgroundColor },
        { "getColor",           GetColor           },
        { "getDefaultFilter",   GetDefaultFilter   },
        { "getDimensions",      GetDimensions      },
        { "getFont",            GetFont            },
        { "getHeight",          GetHeight          },
        { "getLineWidth",       GetLineWidth       },
        { "getScissor",         GetScissor         },
        { "getWidth",           GetWidth           },
        { "line",               Line               },
        { "newFont",            NewFont            },
        { "newImage",           NewImage           },
        { "newQuad",            NewQuad            },
        { "origin",             Origin             },
        { "polygon",            Polygon            },
        { "pop",                Pop                },
        { "present",            Present            },
        { "print",              Print              },
        { "printf",             PrintF             },
        { "push",               Push               },
        { "rectangle",          Rectangle          },
        { "reset",              Reset              },
        { "rotate",             Rotate             },
        { "scale",              Scale              },
        { "setBackgroundColor", SetBackgroundColor },
        { "setColor",           SetColor           },
        { "setDefaultFilter",   SetDefaultFilter   },
        { "setLineWidth",       SetLineWidth       },
        { "setNewFont",         SetNewFont         },
        { "setFont",            SetFont            },
        { "setScissor",         SetScissor         },
        { "translate",          Translate          },
        { "setDepth",           SetDepth           },
        { 0,                    0                  }
    };

    lua_CFunction types[] =
    {
        Wrap_Drawable::Register,
        Wrap_Texture::Register,
        Wrap_Font::Register,
        Wrap_Image::Register,
        Wrap_Quad::Register,
        0
    };

    Graphics * instance = instance();

    if (instance == nullptr)
        Luax::CatchException(L, [&]() { instance = new Graphics(); });
    else
        instance->Retain();

    WrappedModule module;

    module.instance = instance;
    module.name = "graphics";
    module.functions = reg;
    module.type = &Module::type;
    module.types = types;

    return Luax::RegisterModule(L, module);
}
