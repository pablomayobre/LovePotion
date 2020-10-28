#include "common/runtime.h"
#include "common/backend/primitives.h"

#include "deko3d/vertex.h"

using namespace love;
using namespace vertex;

void Primitives::Rectangle(const std::string & mode, float x, float y, float width, float height, float rx, float ry, float lineWidth, const Colorf & color)
{}

void Primitives::Arc(const std::string & mode, float x, float y, float radius, float startAngle, float endAngle, const Colorf & color)
{}

void Primitives::Ellipse(const std::string & mode, float x, float y, float radiusX, float radiusY, const Colorf & color)
{}

void Primitives::Circle(const std::string & mode, float x, float y, float radius, float lineWidth, const Colorf & color)
{}

void Primitives::Polygon(const std::string & mode, std::vector<Graphics::Point> points, float lineWidth, const Colorf & color)
{}

void Primitives::Line(float x1, float y1, float x2, float y2, float lineWidth, const Colorf & color)
{}

void Primitives::Scissor(bool enabled, int x, int y, int width, int height)
{}
