#pragma once
#include <QObject>
#include <QDebug>
#include <QFileInfo>

namespace globals {
  extern QString configRoot;
  extern QString homeDir;
  extern QString configDirectory;
  extern QString configDirectoryAutoMasker;
  extern QString configDirectoryAssets;
  extern QString configDirectoryAssetsTextures;
  extern QString configDirectoryAssetsModels;
  extern QString configDirectoryAssetsU2net;
  extern QString cacheDirectory;
  extern QFileInfo pathDatabase;
}

enum TextureLicense {
  cc0 = 1,
  royalty_free
};

enum TextureSize {
  null = 0,
  x256,
  x512,
  x768,
  x1024,
  x2048,
  x4096
};

enum TextureImageType {
  unknown = 0,
  diffuse,
  roughness,
  ao,
  arm,
  displacement,
  normal,
  metalness,
  opacity,
  emission,
  scattering,  // subsurface scattering
  specular,
  textureImageTypeCount
};

enum TextureImageExt {
  PNG,
  JPG
};

struct TextureTag {
  explicit TextureTag(QString tag) {
    name = tag.toLower();
  }

  QString name = "";
  unsigned int score = 0;
};

struct PackedFile {
  QString fileName;
  QByteArray data;
};

struct RawImageInfo
{
  unsigned short channels;
  unsigned short height;
  unsigned short width;
  bool success;
};

QString cleanTextureTag(QString tag);
extern QMap<QString, QSharedPointer<TextureTag>> CACHE_TEXTURE_TAGS;

extern const QMap<QString, TextureSize> str2TextureSize;
extern const QMap<TextureSize, QString> textureSize2Str;

extern const QMap<QString, TextureImageType> str2TextureImageType;
extern const QMap<TextureImageType, QString> textureImageType2Str;

struct TextureImageInfo {
  explicit TextureImageInfo(const QString &name) {
    auto spl = name.split("_");
    if (spl.size() < 3) {
      this->errorString = "split('_') < 3";
      qWarning() << "skipping" << name << this->errorString;
      success = false;
      return;
    }

    std::ranges::reverse(spl);
    const QString &ttype_str = spl[0];
    const QString &tsize_str = spl[1];
    QStringList name_spl = spl.mid(2);
    std::ranges::reverse(name_spl);
    this->name = name_spl.join('_');

    if(str2TextureSize.contains(tsize_str))
      this->size = str2TextureSize[tsize_str];
    else
      this->size = TextureSize::null;

    if(str2TextureImageType.contains(ttype_str))
      this->type = str2TextureImageType[ttype_str];
    else {
      this->errorString = "unknown ttype " + ttype_str;
      qDebug() << "skipping" << name << this->errorString;
      success = false;
      return;
    }

    if (spl.length() == 4)
      variant = spl.at(3);
    if (spl.length() == 5) {
      variant = QString("%1_%2").arg(spl.at(3), spl.at(4));
    }
  }

  QString name = "";
  TextureSize size;
  TextureImageType type;
  QString variant = "";
  bool success = true;
  QString errorString = "";
};

Q_DECLARE_METATYPE(TextureImageType)