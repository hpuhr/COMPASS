#include "primitive.h"

Primitive::Primitive(const QColor& color)
  : color(color)
{
}

Primitive::~Primitive()
{
}

const QColor& Primitive::getColor() const
{
  return color;
}

Sprite::Sprite(const QColor& color, Sprite::Style style, float size)
  : Primitive(color)
  , style(style)
  , size(size)
{
}

float Sprite::getSize() const
{
  return size;
}

Sprite::Style Sprite::getStyle() const
{
  return style;
}

Primitive::Type Sprite::getType() const
{
  return Polygon::Type::SPRITE;
}

Line::Line(const QColor& color, Line::Style style, float lineWidth)
  : Primitive(color)
  , style(style)
  , lineWidth(lineWidth)
{
}

float Line::getLineWidth() const
{
  return lineWidth;
}

Primitive::Type Line::getType() const
{
  return Polygon::Type::LINE;
}

Line::Style Line::getStyle() const
{
  return style;
}

Polygon::FillStyle Polygon::getFillStyle() const
{
  return fillStyle;
}

Polygon::LineStyle Polygon::getLineStyle() const
{
  return lineStyle;
}

Primitive::Type Polygon::getType() const
{
  return Polygon::Type::POLYGON;
}
