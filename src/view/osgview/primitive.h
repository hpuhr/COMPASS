#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <QtGui/QColor>


class Primitive
{
public:
  Primitive(const QColor& color);
  virtual ~Primitive();

  enum class Type
  {
    SPRITE,
    LINE,
    POLYGON
  };

  virtual Primitive::Type getType() const = 0;
  const QColor& getColor() const;

private:
  QColor color{Qt::GlobalColor::white};
};


class Sprite : public Primitive
{
public:
  enum class Style
  {
    CROSS,
    CIRCLE,
    SQUARE,
    TRIANGLE
  };

  Sprite(const QColor& color, Sprite::Style style, float size);

  virtual Primitive::Type getType() const override;

  float getSize() const;
  Sprite::Style getStyle() const;

private:
  Sprite::Style style{Sprite::Style::CIRCLE};
  float size{10};
};

class Line : public Primitive
{
public:
  enum class Style
  {
    CONTINUOUS,
    DASHED,
    DOTTED
  };

  Line(const QColor& color, Line::Style style, float lineWidth);

  virtual Primitive::Type getType() const override;

  virtual float getLineWidth() const;

  Line::Style getStyle() const;

private:
  Line::Style style{Line::Style::CONTINUOUS};
  float lineWidth{1};
};

///////////////////////////////////////////////////////////////////////////

class Polygon : public Primitive
{
public:
  enum class LineStyle
  {
    NONE,
    CONTINUOUS,
    DASHED,
    DOTTED
  };

  enum class FillStyle
  {
    NONE,
    FILLED
  };

  virtual Primitive::Type getType() const override;

  virtual Polygon::FillStyle getFillStyle() const;
  virtual Polygon::LineStyle getLineStyle() const;

private:
  Polygon::FillStyle fillStyle{Polygon::FillStyle::FILLED};
  Polygon::LineStyle lineStyle{Polygon::LineStyle::NONE};
};

#endif // PRIMITIVE_H
