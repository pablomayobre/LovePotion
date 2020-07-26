#include "common/runtime.h"
#include "modules/graphics/graphics.h"
#include "common/backend/primitives.h"
#include "modules/window/window.h"

using namespace love;

#define WINDOW_MODULE() (Module::GetInstance<Window>(Module::M_WINDOW))

Graphics::Graphics()
{
    this->states.reserve(10);
    this->states.push_back(DisplayState());

    this->transformStack.reserve(16);
    this->transformStack.push_back(TransformState());

    auto window = Module::GetInstance<Window>(M_WINDOW);

    if (window != nullptr)
        window->SetGraphics(this);
}

Graphics::~Graphics()
{
    this->states.clear();
    this->defaultFont.Set(nullptr);
}

void Graphics::SetDepth(float depth)
{
    #if defined (__SWITCH__)
        throw love::Exception("setDepth not supported on this console");
    #endif

    this->stereoDepth = std::clamp(depth, -4.0f, 4.0f);
}

Color Graphics::GetColor()
{
    return this->states.back().foreground;
}

void Graphics::SetColor(const Color & color)
{
    this->states.back().foreground = color;
}

Color Graphics::GetBackgroundColor()
{
    return this->states.back().background;
}

void Graphics::SetBackgroundColor(const Color & color)
{
    this->states.back().background = color;
}

float Graphics::GetLineWidth()
{
    return this->states.back().lineWidth;
}

void Graphics::SetLineWidth(float width)
{
    this->states.back().lineWidth = width;
}

Rect Graphics::GetScissor()
{
    return this->states.back().scissorRect;
}

void Graphics::SetScissor(int x, int y, int width, int height)
{
    this->states.back().scissorRect = {x, y, width, height};
}

void Graphics::SetDefaultFilter(const Texture::Filter & filter)
{
    Texture::defaultFilter = filter;
    this->states.back().defaultFilter = filter;
}

const Texture::Filter & Graphics::GetDefaultFilter() const
{
    return Texture::defaultFilter;
}

void Graphics::GetDimensions(int * width, int * height)
{
    auto window_sizes = WINDOW_MODULE()->GetFullscreenModes();
    int currentDisplay = WINDOW_MODULE()->GetDisplay();

    std::pair<int, int> size = window_sizes[currentDisplay];

    if (width)
        *width = size.first;

    if (height)
        *height = size.second;
}

void Graphics::Origin()
{
    auto & transform = this->transformStack.back();

    transform.offsetX = 0.0f;
    transform.offsetY = 0.0f;

    transform.rotation = 0.0f;

    transform.scalarX = 1.0f;
    transform.scalarY = 1.0f;
}

void Graphics::Push()
{
    if (this->transformStack.size() == MAX_USER_STACK_DEPTH)
        throw Exception("Maximum stack depth reached (more pushes than pops?)");

    this->transformStack.push_back(transformStack.back());
}

void Graphics::Translate(float offsetX, float offsetY)
{
    auto & transform = this->transformStack.back();

    transform.offsetX = offsetX;
    transform.offsetY = offsetY;
}

void Graphics::Rotate(float rotation)
{
    this->transformStack.back().rotation = rotation;
}

void Graphics::Scale(float scalarX, float scalarY)
{
    auto & transform = this->transformStack.back();

    transform.scalarX = scalarX;
    transform.scalarY = scalarY;
}

void Graphics::Pop()
{
    if (this->transformStack.size() < 1)
        throw Exception("Minimum stack depth reached (more pops than pushes?)");

    this->transformStack.pop_back();
}

Graphics::RendererInfo Graphics::GetRendererInfo()
{
    return this->rendererInfo;
}

/* Objects */

Image * Graphics::NewImage(Data * data)
{
    return new Image(data);
}

Font * Graphics::NewFont(float size)
{
    return new Font(size);
}

Font * Graphics::NewFont(Font::SystemFontType type, float size)
{
    return new Font(type, size);
}

Font * Graphics::NewFont(Data * data, float size)
{
    return new Font(data, size);
}

Quad * Graphics::NewQuad(Quad::Viewport viewport, double sw, double sh)
{
    return new Quad(viewport, sw, sh);
}

void Graphics::Draw(Drawable * drawable, const DrawArgs & args)
{
    drawable->Draw(args, this->states.back().Foreground());
}

void Graphics::Draw(Texture * texture, Quad * quad, const DrawArgs & args)
{
    texture->Draw(args, quad, this->states.back().Foreground());
}

void Graphics::Print(const std::vector<Font::ColoredString> & strings, const DrawArgs & args)
{
    this->CheckSetDefaultFont();

    if (this->states.back().font.Get() != nullptr)
        this->Print(strings, this->states.back().font.Get(), args);
}

void Graphics::Print(const std::vector<Font::ColoredString> & strings, Font * font, const DrawArgs & args)
{
    font->Print(strings, args, nullptr, this->states.back().Foreground(), Font::ALIGN_LEFT);
}

void Graphics::PrintF(const std::vector<Font::ColoredString> & strings, const DrawArgs & args, float wrap, Font::AlignMode align)
{
    this->CheckSetDefaultFont();

    if (this->states.back().font.Get() != nullptr)
        this->PrintF(strings, this->states.back().font.Get(), args, wrap, align);
}

void Graphics::PrintF(const std::vector<Font::ColoredString> & strings, Font * font, const DrawArgs & args, float wrap, Font::AlignMode align)
{
    font->Print(strings, args, &wrap, this->states.back().Foreground(), align);
}

/* End Objects */

void Graphics::Rectangle(const std::string & mode, float x, float y, float width, float height, float rx, float ry)
{
    Primitives::Rectangle(mode, x, y, width, height, rx, ry, this->GetLineWidth(), this->states.back().Foreground());
}

void Graphics::Polygon(const std::string & mode, std::vector<Graphics::Point> points)
{
    Primitives::Polygon(mode, points, this->GetLineWidth(), this->states.back().Foreground());
}

void Graphics::Arc(const std::string & mode, float x, float y, float radius, float startAngle, float endAngle)
{
    Primitives::Arc(mode, x, y, radius, startAngle, endAngle, this->states.back().Foreground());
}

void Graphics::Ellipse(const std::string & mode, float x, float y, float radiusX, float radiusY)
{
    Primitives::Ellipse(mode, x, y, radiusX, radiusY, this->states.back().Foreground());
}

void Graphics::Line(float startx, float starty, float endx, float endy)
{
    Primitives::Line(startx, starty, endx, endy, this->GetLineWidth(), this->states.back().Foreground());
}

void Graphics::Circle(float x, float y, float radius)
{
    Primitives::Circle("fill", x, y, radius, this->GetLineWidth(), this->states.back().Foreground());
}

void Graphics::Reset()
{
    DisplayState blankState;
    this->RestoreState(blankState);
    this->Origin();
}

/* Private */

void Graphics::RestoreState(const DisplayState & state)
{
    this->SetColor(state.foreground);
    this->SetBackgroundColor(state.background);
    this->SetFont(state.font);
}

/* Font */

void Graphics::CheckSetDefaultFont()
{
    // We don't create or set the default Font if an existing font is in use.
    if (this->states.back().font.Get() != nullptr)
        return;

    // Create a new default font if we don't have one yet.
    if (!this->defaultFont.Get())
        this->defaultFont.Set(this->NewFont(), Acquire::NORETAIN);

    this->states.back().font.Set(this->defaultFont.Get());
}

void Graphics::SetFont(Font * font)
{
    DisplayState & state = this->states.back();
    state.font.Set(font);
}

Font * Graphics::GetFont()
{
    this->CheckSetDefaultFont();
    return this->states.back().font.Get();
}

/* End Font */

void Graphics::Transform(float * x, float * y)
{
    if (x == nullptr || y == nullptr)
        return;

    DrawArgs args;

    args.offsetX = 0.0f;
    args.offsetY = 0.0f;

    args.x = *x;
    args.y = *y;

    args.r = 0.0f;

    args.scalarX = 1.0f;
    args.scalarY = 1.0f;

    this->Transform(&args);

    *x = args.x;
    *y = args.y;
}

void Graphics::TransformScale(float * x, float * y)
{
    if (this->transformStack.empty())
        return;

    auto & transform = this->transformStack.back();

    if (x != nullptr)
        *x *= transform.scalarX;

    if (y != nullptr)
        *y *= transform.scalarY;
}

void Graphics::Transform(DrawArgs * args)
{
    if (this->transformStack.empty())
        return;

    auto & transform = this->transformStack.back();

    float ox = args->x;
    float oy = args->y;

    #if defined (_3DS)
        float slider = osGet3DSliderState();
        int displayNum = WINDOW_MODULE()->GetDisplay();

        if (gfxIs3D() && displayNum < 2)
        {
            if (displayNum == 0)
                args->x -= (slider * this->stereoDepth);
            else if (displayNum == 1)
                args->x += (slider * this->stereoDepth);
        }
    #endif

    /* Translate */
    args->x += transform.offsetX;
    args->y += transform.offsetY;

    /* Rotate */
    if (transform.rotation != 0)
    {
        args->x -= transform.offsetX;
        args->y -= transform.offsetY;

        float nx = ox * cos(transform.rotation) - oy * sin(transform.rotation);
        float ny = ox * sin(transform.rotation) + oy * cos(transform.rotation);

        args->x = nx += transform.offsetX;
        args->y = ny += transform.offsetY;

        args->r += transform.rotation;
    }

    /* Scale */
    args->x *= transform.scalarX;
    args->y *= transform.scalarY;

    args->scalarX *= transform.scalarX;
    args->scalarY *= transform.scalarY;
}
