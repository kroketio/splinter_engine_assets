#include "models/asset_pack.h"
#include "models/texture.h"

QMap<QString, QSharedPointer<TextureTag>> CACHE_TEXTURE_TAGS = {};
QMap<QString, QPixmap> pixmapCache = {};

TextureImage::TextureImage(
    const TextureImageInfo &texinfo,
    const TextureImageExt ext, QObject *parent) : ext(ext), QObject(parent) {
  this->name = texinfo.name;
  this->size = texinfo.size;
  this->type = texinfo.type;
  this->variant = texinfo.variant;

  // name_technical
  if(!texinfo.variant.isEmpty()) {
    name_technical = QString("%1_%2").arg(
      textureImageType2Str[type],
      this->variant);
  } else {
    name_technical = textureImageType2Str[type];
  }
}

void TextureImage::setTextureImageType(const TextureImageType _type) {
  this->type = _type;
}

void TextureImage::setPath(const QFileInfo &path){
  this->path = path;
  this->basedir = path.absoluteDir().path() + "/";
}

TextureImage* TextureImage::fromPath(const QFileInfo &path) {
  const auto name = path.baseName();
  const auto texinfo = TextureImageInfo(name);
  const TextureImageExt ext = path.suffix() == "png" ? TextureImageExt::PNG : TextureImageExt::JPG;
  auto *cls = new TextureImage(texinfo, ext);
  cls->setPath(path);
  return cls;
}

void TextureImage::inspect_channels_and_dimensions() {
  RawImageInfo info;
  const auto ext = path.suffix();

  if(ext == "png") {
    info = Utils::pngInfo(path);
  } else if(ext == "jpg") {
    info = Utils::jpgInfo(path);
  } else {
    qWarning() << "could not determine filetype for image inspection" << path;
    return;
  }

  if(info.success) {
    channels = info.channels;
    dimensions = {info.width, info.height};
  } else {
    qWarning() << "png/jpg inspection failed for" << path.absoluteFilePath();
  }

  isAlpha = channels == 4;
}

void TextureImage::inspect_checksum() {
  QFile file(path.absoluteFilePath());
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "could not open" << path.absoluteFilePath() << "for checksum generation";
    return;
  }
  auto part = file.read(120000);
  checksum = QString(QCryptographicHash::hash(part, QCryptographicHash::Md5).toHex());
  file.close();
}

void TextureImage::ensure_thumbnail(bool force, QString &err) {
  if (path.absoluteFilePath().isEmpty()) {
    err = "cannot create thumbnail without source path";
    qWarning() << err;
    return;
  }

  const QFileInfo path= this->path_thumbnail();
  const QString path_out = path.absoluteFilePath();
  if(path_out.isEmpty()) {
    err = "could not generate path_out";
    qWarning() << err;
    return;
  }

  if(Utils::fileExists(path_out) && !force) {
    return;
  }

  auto width_new = 256;
  auto height_new = 256;

  qDebug() << "writing:" << path_out;
  QImage img = QImage(path.absoluteFilePath());
  img = img.scaled(height_new, width_new, Qt::KeepAspectRatio);

  bool res;
  if(isAlpha)
    res = img.save(path_out, "png", 80);
  else
    res = img.save(path_out, "jpg", 80);

  if(!res) {
    err = QString("could not save image %1").arg(path_out);
    qWarning() << err;
  } else if(!Utils::fileExists(path_out)) {
    err = QString("failed to write %1").arg(path_out);
    qWarning() << err;
  }
}

void TextureImage::metadata_generate() {
  inspect_channels_and_dimensions();
  inspect_checksum();
}

QFileInfo TextureImage::path_thumbnail() {
  if(channels == 0) {
    qWarning() << "cannot generate thumbnail path without inspecting channels for " << name;
    return {};
  }

  if (checksum.isEmpty()) {
    inspect_checksum();
    if (checksum.isEmpty()) {
      qWarning() << name << "cannot generate thumbnail path without checksum";
      return {};
    }
  }

  const QString ext = isAlpha ? "png" : "jpg";
  auto x = QFileInfo(globals::cacheDirectory + QDir::separator() + checksum + "." + ext);
  // qDebug() << x.absoluteFilePath();
  return x;
}

QJsonObject TextureImage::to_json() {
  QJsonObject obj;
  if(!checksum.isEmpty())
    obj["checksum"] = checksum;
  if(channels > 0)
    obj["channels"] = channels;
  if(size != TextureSize::null)
    obj["size"] = size;
  return obj;
}

Texture::Texture(const QString &name, QObject *parent) : 
  name(name),
  name_lower(name.toLower()),
  QObject(parent) {}

QFileInfo Texture::path_thumbnail() {
  for(const auto tsize: {
      TextureSize::x1024,
      TextureSize::x2048,
      TextureSize::x4096,
      TextureSize::x768,
      TextureSize::x512,
      TextureSize::x256}) {
    if(diffuse.contains(tsize)) {
      const QSharedPointer<TextureImage> tex = diffuse[tsize];
      if (const QFileInfo path = tex->path_thumbnail(); path.exists())
        return path;
    }
  }

  qWarning() << "no thumbnail found for" << this->name;
  return {};
}

// QPixmap Texture::thumbnail() {
//   auto cache_dir = config()->get(ConfigKeys::CacheDir).toString();

//   if(pixmapCache.contains(this->name))
//     return pixmapCache[this->name];

//   for(const auto tsize: {
//       TextureSize::x1024,
//       TextureSize::x2048,
//       TextureSize::x4096,
//       TextureSize::x768,
//       TextureSize::x512,
//       TextureSize::x256}) {
//     if(diffuse.contains(tsize)) {
//       auto tex = diffuse[tsize];
//       auto cached_file = tex->path_thumbnail();
//       if(!cache_dir.isEmpty() && Utils::fileExists(cached_file)) {
//         auto pixmap = QPixmap(cached_file);
//         pixmapCache[this->name] = pixmap;
//         return pixmap;
//       }
//     }
//   }
//   qWarning() << "no thumbnail found for" << this->name;
//   return {};
// }

QJsonObject Texture::toJSON() {
  QJsonObject obj;
  obj["name"] = name;
  obj["num_pixels"] = QString::number(num_pixels);

  for (const auto &tex: {
    QJsonTextureImage("diffuse", diffuse),
    QJsonTextureImage("roughness", roughness),
    QJsonTextureImage("ao", ao),
    QJsonTextureImage("arm", arm),
    QJsonTextureImage("displacement", displacement),
    QJsonTextureImage("normal", normal),
    QJsonTextureImage("opacity", opacity),
    QJsonTextureImage("metalness", metalness),
    QJsonTextureImage("emission", emission),
    QJsonTextureImage("specular", specular),
    QJsonTextureImage("scattering", scattering),
  }) {
    auto obj_ = QJsonObject();
    for (const auto& key: tex.textures.keys()) {
      auto key_str = textureSize2Str[key];
      obj_[key_str] = tex.textures[key]->to_json();
    }
    obj[tex.name] = obj_;
  }

  //qWarning() << QJsonDocument(obj).toJson(QJsonDocument::Indented);
  return obj;
}

