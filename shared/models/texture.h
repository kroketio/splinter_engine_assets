#pragma once
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <chrono>
#include <utility>
#include <functional>

#include "../../server/external/VTFLib/thirdparty/include/Compressonator.h"
#include "../lib/globals.h"
#include "lib/config.h"
#include "lib/utils.h"

class AssetPack;

extern QMap<QString, QSharedPointer<TextureTag>> CACHE_TEXTURE_TAGS;
extern QMap<QString, QPixmap> pixmapCache;

class TextureImage : public QObject {
Q_OBJECT

public:
  explicit TextureImage(const TextureImageInfo &texinfo, TextureImageExt ext, QObject *parent = nullptr);
  static TextureImage* fromPath(const QFileInfo &path);

  QString name;
  QString name_original;
  QString name_technical;
  TextureImageExt ext;
  QString variant;

  TextureSize size = TextureSize::null;
  TextureImageType type = TextureImageType::unknown;
  [[nodiscard]] QString size_str() {
    if(textureSize2Str.contains(size))
      return textureSize2Str[size];
    return "not sure";
  }
  [[nodiscard]] QString type_str() {
    if(textureImageType2Str.contains(type))
      return textureImageType2Str[type];
    return "not sure";
  }

  bool isAlpha = false;

  QSize dimensions;
  QFileInfo path;
  AssetPack* pack = nullptr;
  QFileInfo path_thumbnail();
  QFileInfo path_vmt() const;
  QFileInfo path_vtf() const;
  QString get_ext() const {
    if(ext == TextureImageExt::PNG) return "png";
    else if(ext == TextureImageExt::JPG) return "jpg";
    else return "unknown";
  }

  QString basedir;

  QString checksum;
  unsigned short channels = 0;
  void metadata_generate();

  void ensure_thumbnail(bool force, QString &err);
  void setPack(AssetPack* p_pack);
  void setPath(const QFileInfo &path);
  void setTextureImageType(TextureImageType _type);

  [[nodiscard]] QString path_TextureImage(TextureImageType ttype) const {
    QString fn;
    if(ttype == TextureImageType::diffuse)
      fn = file_color();
    else if(ttype == TextureImageType::normal)
      fn = file_normal();
    else if(ttype == TextureImageType::displacement)
      fn = file_displacement();
    else if(ttype == TextureImageType::roughness)
      fn = file_roughness();
    else if(ttype == TextureImageType::ao)
      fn = file_ambient_occlussion();
    else if(ttype == TextureImageType::arm)
      fn = file_arm();
    else if(ttype == TextureImageType::metalness)
      fn = file_metalness();
    else if(ttype == TextureImageType::opacity)
      fn = file_opacity();
    else if(ttype == TextureImageType::emission)
      fn = file_emission();
    else if(ttype == TextureImageType::specular)
      fn = file_specular();
    else if(ttype == TextureImageType::scattering)
      fn = file_scattering();

    auto path = QString("%1/1k/%2").arg(basedir, fn);
    if(Utils::fileExists(path))
      return path;
    qWarning() << path << "not found";
    return {};
  }

  [[nodiscard]] QString file_color() const {
    return QString("%1_1K-Color." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_normal() const {
    return QString("%1_1K-NormalGL." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_displacement() const {
    return QString("%1_1K-Displacement." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_metalness() const {
    return QString("%1_1K-Metalness." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_opacity() const {
    return QString("%1_1K-Opacity." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_emission() const {
    return QString("%1_1K-Emission." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_specular() const {
    return QString("%1_1K-Specular." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_scattering() const {
    return QString("%1_1K-Scattering." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_roughness() const {
    return QString("%1_1K-Roughness." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_ambient_occlussion() const {
    return QString("%1_1K-AmbientOcclusion." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_arm() const {
    return QString("%1_1K-ARM." + get_ext()).arg(name);
  }
  [[nodiscard]] QString file_cache_thumbnail() const {
    return QString("%1-%2.%3").arg(name, checksum, this->isAlpha ? "png" : "jpg");
  }

  QJsonObject to_json();
  void inspect_channels_and_dimensions();
  void inspect_checksum();
};

struct QJsonTextureImage {
  QJsonTextureImage(QString name, QMap<TextureSize, QSharedPointer<TextureImage>> textures) : name(std::move(name)), textures(std::move(textures)) {};

  QString name;
  QMap<TextureSize, QSharedPointer<TextureImage>> textures;
};


class Texture : public QObject {
Q_OBJECT

public:
  explicit Texture(const QString &name, QObject *parent = nullptr);

  QString name;
  QString name_lower;
  unsigned int num_pixels;

  QMap<TextureSize, QSharedPointer<TextureImage>> diffuse;
  QMap<TextureSize, QSharedPointer<TextureImage>> roughness;
  QMap<TextureSize, QSharedPointer<TextureImage>> ao;
  QMap<TextureSize, QSharedPointer<TextureImage>> arm;
  QMap<TextureSize, QSharedPointer<TextureImage>> displacement;
  QMap<TextureSize, QSharedPointer<TextureImage>> normal;
  QMap<TextureSize, QSharedPointer<TextureImage>> metalness;
  QMap<TextureSize, QSharedPointer<TextureImage>> opacity;
  QMap<TextureSize, QSharedPointer<TextureImage>> emission;
  QMap<TextureSize, QSharedPointer<TextureImage>> specular;
  QMap<TextureSize, QSharedPointer<TextureImage>> scattering;

  QMap<TextureSize, QMap<QString, QSharedPointer<TextureImage>>> variants;
  QList<QSharedPointer<TextureImage>> textures;

  AssetPack* asset_pack() const { return m_asset_pack; }

  QMap<QString, QSharedPointer<TextureTag>> tags() {
    return m_tags;
  }

  QVariantList tags_as_variant() {
    return m_tags_as_variants;
  }

  void set_author(const QString &author) {
    m_author = author;
  }

  QString get_author() {
    return m_author;
  }

  void set_license(const QString &license) {
    m_license = license;
  }

  QString get_license() {
    return m_license;
  }

  void set_asset_pack(AssetPack* pack) {
    m_asset_pack = pack;
  }

  void append_tag(QSharedPointer<TextureTag> &tag) {
    if(!m_tags.contains(tag->name)) {
      m_tags[tag->name] = tag;
      m_tags_as_variants << tag->name;
    }
  }

  QFileInfo path_thumbnail();
  //QPixmap thumbnail();
  QJsonObject toJSON();

  QSharedPointer<TextureImage> get_image(TextureImageType type ,TextureSize size) {
    if(type == TextureImageType::diffuse) {
      if(diffuse.contains(size))
        return diffuse[size];
    } else if(type == TextureImageType::displacement) {
      if(displacement.contains(size))
        return displacement[size];
    } else if(type == TextureImageType::roughness) {
      if(roughness.contains(size))
        return roughness[size];
    } else if(type == TextureImageType::normal) {
      if(normal.contains(size))
        return normal[size];
    } else if(type == TextureImageType::ao) {
      if(ao.contains(size))
        return ao[size];
    } else if(type == TextureImageType::arm) {
      if(arm.contains(size))
        return arm[size];
    } else if(type == TextureImageType::metalness) {
      if(metalness.contains(size))
        return metalness[size];
    } else if(type == TextureImageType::opacity) {
      if(opacity.contains(size))
        return opacity[size];
    } else if(type == TextureImageType::emission) {
      if(emission.contains(size))
        return emission[size];
    } else if(type == TextureImageType::specular) {
      if(specular.contains(size))
        return specular[size];
    } else if(type == TextureImageType::scattering) {
      if(scattering.contains(size))
        return scattering[size];
    } else {
      qWarning() << "unknown weird type" << type;
    }

    return nullptr;
  }

  // @fuzzy: fallback to other resolutions when `size` is not available
  QSharedPointer<TextureImage> get_diffuse(TextureSize size, bool fuzzy = false) {
    if(diffuse.contains(size))
      return diffuse[size];

    if(!fuzzy)
      return nullptr;

    for(const auto tsize: {
      TextureSize::x1024,
      TextureSize::x4096,
      TextureSize::x2048,
      TextureSize::x768,
      TextureSize::x512,
      TextureSize::x256}) {
        if(diffuse.contains(size))
          return diffuse[size];
    }

    return nullptr;
  }

  void setTexture(const QSharedPointer<TextureImage> &tex) {
    // @TODO: support variants
    if(tex->type == TextureImageType::diffuse) {
      this->setDiffuse(tex->size, tex);
    } else if(tex->type == TextureImageType::displacement) {
      this->setDisplacement(tex->size, tex);
    } else if(tex->type == TextureImageType::roughness) {
      this->setRoughness(tex->size, tex);
    } else if(tex->type == TextureImageType::normal) {
      this->setNormal(tex->size, tex);
    } else if(tex->type == TextureImageType::ao) {
      this->setAO(tex->size, tex);
    } else if(tex->type == TextureImageType::arm) {
      this->setARM(tex->size, tex);
    } else if(tex->type == TextureImageType::metalness) {
      this->setMetalness(tex->size, tex);
    } else if(tex->type == TextureImageType::opacity) {
      this->setOpacity(tex->size, tex);
    } else if(tex->type == TextureImageType::emission) {
      this->setEmission(tex->size, tex);
    } else if(tex->type == TextureImageType::specular) {
      this->setSpecular(tex->size, tex);
    } else if(tex->type == TextureImageType::scattering) {
      this->setScattering(tex->size, tex);
    } else {
      qWarning() << "uhm weird type" << tex->type;
    }
  }

  void addVariant(TextureSize tsize, const QString &variant, const QSharedPointer<TextureImage> &tex) {
    variants[tsize][variant] = tex;
    textures << tex;
  }

  void setDiffuse(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    diffuse[tsize] = tex;
    textures << tex;
  }

  void setRoughness(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    roughness[tsize] = tex;
    textures << tex;
  }

  void setAO(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    ao[tsize] = tex;
    textures << tex;
  }

  void setARM(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    arm[tsize] = tex;
    textures << tex;
  }

  void setDisplacement(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    displacement[tsize] = tex;
    textures << tex;
  }

  void setNormal(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    normal[tsize] = tex;
    textures << tex;
  }

  void setMetalness(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    metalness[tsize] = tex;
    textures << tex;
  }

  void setOpacity(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    opacity[tsize] = tex;
    textures << tex;
  }

  void setEmission(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    emission[tsize] = tex;
    textures << tex;
  }

  void setSpecular(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    specular[tsize] = tex;
    textures << tex;
  }

  void setScattering(TextureSize tsize, const QSharedPointer<TextureImage> &tex) {
    scattering[tsize] = tex;
    textures << tex;
  }

  // detect the largest texture size
  void detect_largest_size() {
    if(diffuse.empty()) {
      qWarning() << "no diffuse textures to measure size on";
      return;
    }

    auto max_size = 0;
    for(const auto &diff: diffuse.values()) {
      auto _size = diff->dimensions.height() * diff->dimensions.width();
      if(_size > max_size)
        max_size = _size;
    }
    num_pixels = max_size;
  }

  QList<TextureSize> available_sizes() const {
    return this->diffuse.keys();
  }

private:
  QString m_license;
  QString m_author;
  AssetPack* m_asset_pack;

  QMap<QString, QSharedPointer<TextureTag>> m_tags;
  QVariantList                              m_tags_as_variants;
};
