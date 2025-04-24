#include "lib/globals.h"

namespace globals {
  QString configRoot;
  QString homeDir;
  QString configDirectory;
  QString configDirectoryAutoMasker;
  QString configDirectoryAssets;
  QString configDirectoryAssetsTextures;
  QString configDirectoryAssetsModels;
  QString configDirectoryAssetsU2net;
  QString cacheDirectory;
  QFileInfo pathDatabase;

  QMap<QString, QSharedPointer<TextureTag>> CACHE_TEXTURE_TAGS = {};
}

QString cleanTextureTag(QString tag) {
    if(tag == "wooden")
      tag = "wood";
    else if(tag == "gray")
      tag = "grey";
    else if(tag == "mountain")
      tag = "rock";
    else if(tag == "sci" || tag == "fi")
      tag = "scifi";
    else if(tag == "stone")
      tag = "stones";
    else if(tag == "brick")
      tag = "bricks";
    else if(tag == "rock")
      tag = "rocks";
    else if(tag == "leaf")
      tag = "grass";
    else if(tag == "ground" || tag == "flooring")
      tag = "floor";
    else if(tag == "crack")
      tag = "cracks";
    return tag;
}

const QMap<TextureImageType, QString> textureImageType2Str = {
  {TextureImageType::diffuse, "Color"},
  {TextureImageType::normal, "NormalGL"},
  {TextureImageType::displacement, "Displacement"},
  {TextureImageType::roughness, "Roughness"},
  {TextureImageType::ao, "AmbientOcclusion"},
  {TextureImageType::arm, "ARM"},
  {TextureImageType::metalness, "Metalness"},
  {TextureImageType::opacity, "Opacity"},
  {TextureImageType::emission, "Emission"},
  {TextureImageType::scattering, "Scattering"},
};
const QMap<QString, TextureImageType> str2TextureImageType = {
  {"unknown", TextureImageType::unknown},
  {"Color", TextureImageType::diffuse},
  {"Diffuse", TextureImageType::diffuse},
  {"NormalGL", TextureImageType::normal},
  {"NormalDX", TextureImageType::normal},
  {"Displacement", TextureImageType::displacement},
  {"Roughness", TextureImageType::roughness},
  {"AmbientOcclusion", TextureImageType::ao},
  {"ARM", TextureImageType::arm},
  {"Metalness", TextureImageType::metalness},
  {"Specular", TextureImageType::specular},
  {"Opacity", TextureImageType::opacity},
  {"Emission", TextureImageType::emission},
  {"Scattering", TextureImageType::scattering},
};
const QMap<TextureSize, QString> textureSize2Str = {
  {TextureSize::null, "null"},
  {TextureSize::x256, "256"},
  {TextureSize::x512, "512"},
  {TextureSize::x768, "768"},
  {TextureSize::x1024, "1K"},
  {TextureSize::x2048, "2K"},
  {TextureSize::x4096, "4K"}
};
const QMap<QString, TextureSize> str2TextureSize = {
  {"null", TextureSize::null},
  {"256", TextureSize::x256},
  {"512", TextureSize::x512},
  {"768", TextureSize::x768},
  {"1K", TextureSize::x1024},
  {"2K", TextureSize::x2048},
  {"4K", TextureSize::x4096},
  {"1k", TextureSize::x1024},
  {"2k", TextureSize::x2048},
  {"4k", TextureSize::x4096},
};
