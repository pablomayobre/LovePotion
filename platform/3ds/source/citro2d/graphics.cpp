#include "common/runtime.h"
#include "citro2d/graphics.h"

#include "modules/window/window.h"

using namespace love;

#define WINDOW_MODULE() (Module::GetInstance<Window>(M_WINDOW))
#define TRANSPARENCY C2D_Color32(0, 0, 0, 1)

void love::citro2d::Graphics::Clear(std::optional<Colorf> color, std::optional<int> stencil, std::optional<double> depth)
{
    WINDOW_MODULE()->Clear(&color.value());
}

void love::citro2d::Graphics::Present()
{
    WINDOW_MODULE()->Present();
}

void love::citro2d::Graphics::SetColor(Colorf color)
{
    this->states.back().foreground = color;
}

/* Primitives */

inline const auto normalizeAngle = [](float angle)
{
    angle = fmodf(angle, M_TAU);
    if (angle < 0)
        angle += M_TAU;

    return angle;
};

inline std::vector<Vector2> GenerateOutline(const Vector2 * points, size_t count, float lineWidth)
{
    std::vector<Vector2> innerPoints(count);

    for (size_t startPoint = 0; startPoint < count; startPoint++)
    {
        const auto & middle = points[startPoint];
        const auto & after  = points[(startPoint + 1) % count];
        const auto & before = points[startPoint == 0 ? count - 1 : startPoint - 1];

        const float theta = normalizeAngle(atan2f(middle.y - after.y, middle.x - after.x));
        const float phi   = normalizeAngle(atan2f(middle.y - before.y, middle.x - before.x));

        const float angleWithinPolygon = normalizeAngle(phi - theta);
        const float angleOfRightTriangle = (M_PI - angleWithinPolygon) / 2;

        const float lengthOfKite = lineWidth * (1 / cosf(angleOfRightTriangle));

        const float offsetX = cosf(theta + (M_PI_2 - angleOfRightTriangle)) * lengthOfKite;
        const float offsetY = sinf(theta + (M_PI_2 - angleOfRightTriangle)) * lengthOfKite;

        innerPoints[startPoint] = Vector2(middle.x - offsetX, middle.y - offsetY);
    }

    return innerPoints;
}

void love::citro2d::Graphics::Polyfill(const Vector2 * points, size_t count, u32 color, float depth)
{
    for (size_t currentPoint = 2; currentPoint < count; currentPoint++)
    {
        C2D_DrawTriangle(points[0].x, points[0].y, color, points[currentPoint - 1].x, points[currentPoint - 1].y,
                         color, points[currentPoint].x, points[currentPoint].y, color, depth);
    }
}

void love::citro2d::Graphics::Polygon(DrawMode mode, const Vector2 * points, size_t count)
{
    Colorf color = this->GetColor();
    u32 foreground = C2D_Color32f(color.r, color.g, color.b, color.a);

    if (mode == DRAW_LINE)
        this->Polyline(points, count);
    else
        this->Polyfill(points, count, foreground, Graphics::CURRENT_DEPTH);
}

void love::citro2d::Graphics::Polyline(const Vector2 * points, size_t count)
{
    // Generate the outline and draw it
    std::vector<Vector2> outline = GenerateOutline(points, count, this->states.back().lineWidth);
    this->Polyfill(outline.data(), outline.size(), TRANSPARENCY, Graphics::CURRENT_DEPTH + Graphics::MIN_DEPTH);

    // Draw our filled polygon
    this->Polygon(DRAW_FILL, points, count);

    Graphics::CURRENT_DEPTH += Graphics::MIN_DEPTH;
}

void love::citro2d::Graphics::Rectangle(DrawMode mode, float x, float y, float width, float height)
{
    Vector2 points[4] =
    {
        {x, y},
        {x + width, y},
        {x + width, y + height},
        {x, y + height}
    };

    this->Polygon(mode, points, 4);
}

void love::citro2d::Graphics::Ellipse(DrawMode mode, float x, float y, float a, float b)
{
    Colorf color = this->GetColor();
    u32 foreground = C2D_Color32f(color.r, color.g, color.b, color.a);

    float radiusx = a / 2;
    float radiusy = b / 2;

    if (mode == DRAW_FILL)
        C2D_DrawEllipseSolid(x, y, Graphics::CURRENT_DEPTH, radiusx, radiusy, foreground);
    else
    {
        C2D_DrawEllipseSolid(x, y, Graphics::CURRENT_DEPTH + Graphics::MIN_DEPTH,
                             radiusx - this->states.back().lineWidth, radiusy - this->states.back().lineWidth,
                             TRANSPARENCY);

        C2D_DrawEllipseSolid(x, y, Graphics::CURRENT_DEPTH, radiusx, radiusy, foreground);

        Graphics::CURRENT_DEPTH += Graphics::MIN_DEPTH;
    }
}

void love::citro2d::Graphics::Circle(DrawMode mode, float x, float y, float radius)
{
    Colorf color = this->GetColor();
    u32 foreground = C2D_Color32f(color.r, color.g, color.b, color.a);

    if (mode == DRAW_FILL)
        C2D_DrawCircleSolid(x, y, Graphics::CURRENT_DEPTH, radius, foreground);
    else
    {
        C2D_DrawCircleSolid(x, y, Graphics::CURRENT_DEPTH + Graphics::MIN_DEPTH,
                            radius - this->states.back().lineWidth, TRANSPARENCY);

        C2D_DrawCircleSolid(x, y, Graphics::CURRENT_DEPTH, radius, foreground);

        Graphics::CURRENT_DEPTH += Graphics::MIN_DEPTH;
    }
}

void love::citro2d::Graphics::Arc(DrawMode mode, ArcMode arcmode, float x, float y, float radius, float angle1, float angle2)
{
    const auto calc45Triangle = [](float x, float y, float radius, float angle) -> std::array<Vector2, 3>
    {
        return
        {
            Vector2(x, y),
            Vector2(x + radius * cosf(angle), y + radius * sinf(angle)),
            Vector2(x + radius * sqrtf(2) * cosf(angle + 90), y + radius * sqrtf(2) * sinf(angle + 90))
        };
    };

    angle1 = normalizeAngle(angle1);
    angle2 = normalizeAngle(angle2);

    // Only go around counterclockwise rather than having a conditional
    if (angle2 > angle1)
        angle2 -= M_TAU;

    while (angle2 + M_PI_4 < angle1)
    {
        const auto & pts = calc45Triangle(x, y, radius, angle2);
        C2D_DrawTriangle(pts[0].x, pts[0].y, TRANSPARENCY, pts[1].x, pts[1].y, TRANSPARENCY,
                         pts[2].x, pts[2].y, TRANSPARENCY, Graphics::CURRENT_DEPTH + Graphics::MIN_DEPTH);
        angle2 += M_PI_4;
    }

    const std::array<Vector2, 3> finalTriangle =
    {
        Vector2(x, y),
        Vector2(x + radius * cosf(angle2), y + radius * sinf(angle2)),
        Vector2(x + radius * sqrtf(2) * cosf(angle1), y + radius * sqrtf(2) * sinf(angle1))
    };

    C2D_DrawTriangle(finalTriangle[0].x, finalTriangle[0].y, TRANSPARENCY, finalTriangle[1].x, finalTriangle[1].y, TRANSPARENCY,
                     finalTriangle[2].x, finalTriangle[2].y, TRANSPARENCY, Graphics::CURRENT_DEPTH + Graphics::MIN_DEPTH);

    this->Circle(mode, x, y, radius);

    Graphics::CURRENT_DEPTH += Graphics::MIN_DEPTH;
}

void love::citro2d::Graphics::Line(float startx, float starty, float endx, float endy)
{
    Colorf color = this->GetColor();
    u32 foreground = C2D_Color32f(color.r, color.g, color.b, color.a);

    C2D_DrawLine(startx, starty, foreground, endx, endy, foreground, this->states.back().lineWidth, Graphics::CURRENT_DEPTH);
}

void love::citro2d::Graphics::SetLineWidth(float width)
{
    this->states.back().lineWidth = width;
}

void love::citro2d::Graphics::SetDefaultFilter(const Texture::Filter & filter)
{
    Texture::defaultFilter = filter;
}

/* End Primitives */

void love::citro2d::Graphics::SetScissor(const Rect & scissor)
{
    DisplayState & state = this->states.back();

    if (state.scissor)
        C2D_Flush();

    int screenWidth = 0;
    this->GetDimensions(&screenWidth, nullptr);

    c2d.SetScissor(GPU_SCISSOR_NORMAL, scissor, screenWidth, false);

    state.scissor = true;
    state.scissorRect = scissor;
}

void love::citro2d::Graphics::SetScissor()
{
    if (states.back().scissor)
        C2D_Flush();

    int screenWidth = 0;
    this->GetDimensions(&screenWidth, nullptr);

    c2d.SetScissor(GPU_SCISSOR_DISABLE, {0, 0, screenWidth, 240}, screenWidth, false);

    states.back().scissor = false;
}

Graphics::RendererInfo love::citro2d::Graphics::GetRendererInfo() const
{
    RendererInfo info {};

    info.name    = RENDERER_NAME;
    info.device  = RENDERER_DEVICE;
    info.vendor  = RENDERER_VENDOR;
    info.version = RENDERER_VERSION;

    return info;
}