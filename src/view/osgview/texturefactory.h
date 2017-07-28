#ifndef TEXTUREFACTORY_H
#define TEXTUREFACTORY_H


#include <map>
#include <memory>
#include <osg/Texture2D>
#include <QtGui/QImage>

#include "primitive.h"

/// Holds cached image data.
/// Needed because the three data structures share data.
/// Deletes the held resources when destroyed.
class CachedImage
{
public:
  ~CachedImage();

  QImage* qtImage{nullptr};
  osg::ref_ptr<osg::Image> image{nullptr};
  osg::ref_ptr<osg::Texture2D> texture{nullptr};
};

/// Loads textures and caches them. Textures are owned by this class, and are
/// not deleted until it is deleted.
class TextureFactory
{
public:
  TextureFactory();
  virtual ~TextureFactory();

  osg::Texture2D* getTextureForStyle(Sprite::Style style);

private:
  std::map<Sprite::Style, std::unique_ptr<CachedImage>> textureCache;
};

#endif // TEXTUREFACTORY_H
