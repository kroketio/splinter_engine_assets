#include <QBuffer>
#include <QCoreApplication>
#include <QDirIterator>
#include <QObject>
#include <QResource>

#include <algorithm>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "vtf.h"

#include "lib/nameof.hpp"
#include "lib/vtf/common/enums.hpp"
#include "lib/vtf/common/image.hpp"
#include "lib/vtf/common/util.hpp"
#include "lib/vtf/common/vtftools.hpp"

#include "models/asset_pack.h"
#include "models/texture.h"

using namespace VTFLib;

namespace vtf {
  bool generate_vmt_vtf_files(void* tex) {
    auto *texture = static_cast<Texture *>(tex);

    const auto diffuse = texture->get_diffuse(TextureSize::x1024, true);
    QFileInfo path_input_image = diffuse->path;
    QFileInfo path_output_vtf = diffuse->path_vtf();
    QFileInfo path_output_vmt = diffuse->path_vmt();
    QString asset_pack_name = texture->asset_pack()->name();
    QString texture_name = diffuse->name;

    if (path_output_vmt.exists()) {
      return false;
    }

    if (!path_input_image.exists()) {
      qWarning() << "vtf convert: input path does not exist:" << path_input_image.absoluteFilePath();
    }

    auto diffuse_width = diffuse->dimensions.width();
    if (diffuse_width != 128 && diffuse_width != 256 && diffuse_width != 512 && diffuse_width != 1024 && diffuse_width != 2048 && diffuse_width != 4096) {
      return false;
    }

    auto path_output_vtf_str = path_output_vtf.absoluteFilePath().toStdString();
    auto path_output_vmt_str = path_output_vmt.absoluteFilePath().toStdString();

    qDebug() << "generating vtf/vmt for" << texture_name;

    QFile img_file(path_input_image.absoluteFilePath());
    if (!img_file.open(QIODevice::ReadOnly)) {
      qWarning() << "vtf convert: could not open:" << path_input_image.absoluteFilePath();
      return false;
    }

    QByteArray imageData = img_file.readAll();
    img_file.close();

    FILE *fp = fmemopen(imageData.data(), imageData.size(), "rb");
    if (!fp) {
      qWarning() << "vtf convert: could not fmemopen:" << path_input_image.absoluteFilePath();
      return false;
    }

    const auto image = imglib::Image::load(fp);
    if (!image) {
      fclose(fp);
      qWarning() << "vtf convert: could not stb open:" << path_input_image.absoluteFilePath();
      return false;
    }

    fclose(fp);

    if (image->m_height == -1 && image->m_width == -1) {
      qWarning() << "vtf convert: could not stb open, bad width or height:" << path_input_image.absoluteFilePath();
      return false;
    }

    bool height_2 = image->m_height > 0 && (image->m_height & (image->m_height - 1)) == 0;
    bool width_2 = image->m_width > 0 && (image->m_width & (image->m_width - 1)) == 0;
    if (!height_2 && !width_2) {
      qWarning() << "vtf convert: bad width or height, not in power of two:" << path_input_image.absoluteFilePath();
      return false;
    }

    constexpr int target_width = 512;
    int target_height = (image->m_height * target_width) / image->m_width;

    if (image->m_height != target_height && image->m_width != target_width) {
      if (!image->resize(target_width, target_height))
        return false;
    }

    auto format = diffuse->isAlpha ? ImageFormatFromUserString("DXT5") : ImageFormatFromUserString("DXT1");
    const auto vtfFile = std::make_unique<CVTFFile>();
    const int w = image->m_width;
    const int h = image->m_height;
    const auto srgb = true;

    vlByte *dest = nullptr;

    if (format != IMAGE_FORMAT_NONE && format != image->vtf_format()) {
      const auto sizeRequired = CVTFFile::ComputeImageSize(w, h, 1, 1, format);
      dest = static_cast<vlByte *>(malloc(sizeRequired));

      if (!CVTFFile::Convert((vlByte *) image->data(), dest, w, h, image->vtf_format(), format)) {
        qWarning() << "Could not convert from" << NAMEOF_ENUM(image->vtf_format()) << "to" << NAMEOF_ENUM(format) << ":"
                   << util::get_last_vtflib_error();
        free(dest);
        return false;
      }
    } else {
      format = image->vtf_format();
    }

    VTFLib::CVTFFile *file = vtfFile.get();
    auto mips_computed = CVTFFile::ComputeMipmapCount(w, h, 1);
    if (!file->Init(w, h, 1, 1, 1, format, vlTrue, mips_computed)) {
      qWarning() << "Could not create VTF:" << util::get_last_vtflib_error();
      free(dest);
      return false;
    }

    file->SetData(1, 1, 1, 0, dest ? dest : (vlByte *) image->data());

    constexpr int majorVer = 7;
    constexpr int minorVer = 2;
    constexpr int compressionLevel = 0;

    vtfFile->SetVersion(majorVer, minorVer);
    if (!vtfFile->SetAuxCompressionLevel(compressionLevel) && compressionLevel != 0) {
      qWarning() << "Could not set compression level to" << compressionLevel;
      return false;
    }

    vtfFile->SetFlag(TEXTUREFLAGS_SRGB, true);
    vtfFile->SetFlag(TEXTUREFLAGS_ANISOTROPIC, true);

    auto mipcount = vtfFile->GetMipmapCount();
    if (mipcount == 1)
      vtfFile->SetFlag(TEXTUREFLAGS_NOMIP, true);

    vtfFile->ComputeReflectivity();

    bool thumbnail = true;
    if (thumbnail && !vtfFile->GenerateThumbnail(srgb)) {
      qWarning() << "could not generate thumbnail" << texture_name;
    }

    if (!vtfFile->GenerateMipmaps(MIPMAP_FILTER_CUBIC, srgb)) {
      qWarning() << "Could not generate mipmaps!" << texture_name;
    }

    if (vtfFile->GetFormat() != format && !vtfFile->ConvertInPlace(format)) {
      qWarning() << "Could not convert image data to the desired format";
      return false;
    }

    // write
    if (!vtfFile->Save(path_output_vtf_str.c_str())) {
      qWarning() << "Could not save file" << path_output_vtf.absolutePath() << ":" << util::get_last_vtflib_error();
      return false;
    }

    QString filename = path_output_vtf.fileName();
    filename = filename.replace(".vtf", "");
    QString vmt = "\"LightmappedGeneric\"\n"
                  "{\n"
                  "\"$baseTexture\" \"%1/%2\"\n"
                  "\"$surfaceprop\" \"brick\"\n"
                  "\"$translucent\" %3\n"
                  "\"%keywords\" \"%4%5\"\n"
                  "}\n";
    vmt = vmt.replace("%1", asset_pack_name);
    vmt = vmt.replace("%2", diffuse->name_original);
    vmt = vmt.replace("%3", diffuse->isAlpha ? "1" : "0");
    vmt = vmt.replace("%4", asset_pack_name.replace("_1k", ""));

    if (auto tags = texture->tags(); !tags.empty()) {
      QStringList keys = tags.keys();
      auto joinedKeys = keys.join(",");
      vmt = vmt.replace("%5", "," + joinedKeys);
    }

    QFile file_vmt(path_output_vmt.absoluteFilePath());
    if (file_vmt.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file_vmt);
      out << vmt;
      file_vmt.close();
    }
    file_vmt.close();
    return true;
  }
}
