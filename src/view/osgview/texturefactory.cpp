#include "texturefactory.h"

#include <QtGui/QImage>
#include <osg/Image>
#include <osg/Texture2D>

#include <iostream>

CachedImage::~CachedImage()
{
  delete qtImage;
  qtImage = nullptr;
}

TextureFactory::TextureFactory()
{
}

TextureFactory::~TextureFactory()
{
}

osg::Texture2D* TextureFactory::getTextureForStyle(Sprite::Style style)
{
  auto iter = textureCache.find(style);
  if (iter != std::end(textureCache))
  {
    return iter->second.get()->texture.get();
  }
  std::string texturePath;
  switch (style)
  {
  case Sprite::Style::TRIANGLE:
    texturePath = "./data/textures/triangle.png";
    break;
  case Sprite::Style::CIRCLE:
    texturePath = "./data/textures/circle.png";
    break;
  case Sprite::Style::SQUARE:
    texturePath = "./data/textures/square.png";
    break;
  case Sprite::Style::CROSS:
    texturePath = "./data/textures/cross.png";
    break;
  default:
    texturePath = "./unsupported_sprite_style";
    break;
  }

  QImage image(texturePath.c_str());
  if (image.isNull())
  {
    std::cerr << "Error: could not load image from resource path: "
              << texturePath << std::endl;
    return nullptr;
  }

  std::unique_ptr<CachedImage> cImage(new CachedImage);

  cImage->qtImage =
      new QImage(image.convertToFormat(QImage::Format::Format_RGBA8888));

  cImage->image = new osg::Image;

  cImage->image->setImage(cImage->qtImage->width(), cImage->qtImage->height(),
                          1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
                          (unsigned char*)cImage->qtImage->constBits(),
                          osg::Image::AllocationMode::NO_DELETE);

  osg::Texture2D* texture = new osg::Texture2D;
  texture->setImage(cImage->image);
  cImage->texture = texture;

  textureCache.insert(std::make_pair(style, std::move(cImage)));
  return texture;
}
