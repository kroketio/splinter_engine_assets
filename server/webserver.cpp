#include "webserver.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QtConcurrent>

#include <QMimeDatabase>
#include <algorithm>
#include <qguiapplication_platform.h>
#include <random>

#include "ctx.h"
#include "lib/file_packer.h"
#include "lib/godot.h"

using namespace std::chrono;

WebServer::WebServer(QObject *parent) : m_server(new QHttpServer(this)), QObject(parent) {
  m_server->route("/", [] {
    return "Hello";
  });

  m_server->route("/api/1/asset_pack/vmtvtf_pack", [](const QHttpServerRequest &request) {
    QFuture<QHttpServerResponse> future = QtConcurrent::run([&request] {
#if defined(Q_OS_LINUX)
      const QUrlQuery query(request.url());

      const QString asset_pack = query.queryItemValue("asset_pack", QUrl::FullyDecoded).toLower();

      const Ctx *ctx = Ctx::instance();
      QList<QSharedPointer<AssetPack>> packs = ctx->assetPackManager->packs[AssetPackType::assetPackTexture];
      QSharedPointer<AssetPack> pack;

      for (const auto &_pack: packs) {
        if (_pack->name() == asset_pack)
          pack = _pack;
      }

      QString err_msg;
      if (pack.isNull()) {
        err_msg = QString("asset_pack '%1' not found").arg(asset_pack);
        qWarning() << err_msg;
        return QHttpServerResponse(err_msg, QHttpServerResponder::StatusCode::NotFound);
      }

      QStringList vmts;
      QStringList vtfs;

      for (auto &key: pack->textures.keys()) {
        const QSharedPointer<Texture> texture = pack->textures.value(key);
        const auto diffuse = texture->get_diffuse(TextureSize::x1024, true);
        if (diffuse.isNull())
          continue;

        auto _vmt = diffuse->path_vmt();
        auto _vtf = diffuse->path_vtf();
        if (_vmt.exists())
          vmts << _vmt.absoluteFilePath();
        if (_vtf.exists())
          vtfs << _vtf.absoluteFilePath();
      }

      if (vmts.empty() || vtfs.empty()) {
        err_msg = QString("could not find any vmt's or vtf's");
        qWarning() << err_msg;
        return QHttpServerResponse(err_msg, QHttpServerResponder::StatusCode::NotFound);
      }

      QStringList merged = vmts + vtfs;

      //==================

      QByteArray out;
      for (const QString &str : merged)
          out += (str + "\n").toUtf8();

      QTemporaryFile tempZipFile;
      tempZipFile.setAutoRemove(false);
      if (!tempZipFile.open()) {
        err_msg = QString("Failed to create temporary zip file for asset pack: %1").arg(asset_pack);
        qWarning() << err_msg;
        return QHttpServerResponse(err_msg, QHttpServerResponder::StatusCode::NotFound);
      }
      QString outputZipPath = tempZipFile.fileName() + ".zip";
      tempZipFile.close();

      QProcess process;
      QStringList arguments;
      arguments << "-j";  // flat .zip
      arguments << outputZipPath << "-@";

      process.start("zip", arguments);
      if (!process.waitForStarted()) {
        err_msg = QString("Failed to start zip process for asset pack: %1").arg(asset_pack);
        qWarning() << err_msg;
        return QHttpServerResponse(err_msg, QHttpServerResponder::StatusCode::NotFound);
      }

      process.write(out);
      process.closeWriteChannel();
      process.waitForFinished(-1);
      qDebug() << "Zip stderr:" << process.readAllStandardError();

      auto response = QHttpServerResponse::fromFile(outputZipPath);
      QHttpHeaders headers;
      headers.insert(0, "Content-Disposition", QString("attachment; filename=\"%1\"").arg(pack->name() + "_vmtvtf_pack.zip"));
      response.setHeaders(std::move(headers));
      return response;
#else
        return QHttpServerResponse(QString("only available on Linux"), QHttpServerResponder::StatusCode::NotFound);
#endif
    });
    return future;
  });

  m_server->route("/api/1/textures/godot_pack", [](const QHttpServerRequest &request) {
    QFuture<QHttpServerResponse> future = QtConcurrent::run([&request] {
      const QUrlQuery query(request.url());

      QString image_size = query.queryItemValue("image_size").toLower();
      if (image_size.isEmpty())
        return QHttpServerResponse("image_size empty, provide e.g: 1024", QHttpServerResponder::StatusCode::NotFound);

      if (!str2TextureSize.contains(image_size))
        return QHttpServerResponse("wrong image_size", QHttpServerResponder::StatusCode::NotFound);
      auto image_size_enum = str2TextureSize[image_size];

      const QString name = query.queryItemValue("name", QUrl::FullyDecoded);
      qDebug() << "packing" << name;
      const QString asset_pack = query.queryItemValue("asset_pack", QUrl::FullyDecoded).toLower();

      const Ctx *ctx = Ctx::instance();
      QList<QSharedPointer<AssetPack>> packs = ctx->assetPackManager->packs[AssetPackType::assetPackTexture];
      QSharedPointer<AssetPack> pack;

      for (const auto &_pack: packs) {
        if (_pack->name() == asset_pack)
          pack = _pack;
      }

      QString err_msg;

      if (pack.isNull()) {
        err_msg = QString("asset_pack '%1' not found").arg(asset_pack);
        qWarning() << err_msg;
        return QHttpServerResponse(err_msg, QHttpServerResponder::StatusCode::NotFound);
      }

      if (!pack->textures_lower.contains(name.toLower())) {
        err_msg = QString("texture by name not found: %s").arg(name.toLower());
        qWarning() << err_msg;
        return QHttpServerResponse(err_msg, QHttpServerResponder::StatusCode::NotFound);
      }

      const auto tex = pack->textures_lower[name.toLower()];

      QStringList paths;
      for (int i = 0; i < TextureImageType::textureImageTypeCount; ++i) {
        TextureImageType type = static_cast<TextureImageType>(i);
        if (type == TextureImageType::unknown)
          continue;

        if (auto img = tex->get_image(type, image_size_enum); !img.isNull())
          paths << img->path.absoluteFilePath();
      }

      if (paths.isEmpty()) {
        err_msg = QString("no suitable paths found");
        qWarning() << err_msg;
        return QHttpServerResponse(err_msg, QHttpServerResponder::StatusCode::NotFound);
      }

      const auto tres = godot::generateMaterialTres(tex, image_size_enum);
      const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
      const QString baseName = tex->name_lower + ".tres";
      const QString filePath = tempDir + "/" + baseName;
      qDebug() << "writing" << filePath;

      QFile file(filePath);
      if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(&file);
        out << tres;
        file.close();
      }

      qDebug() << "packing";
      paths << filePath;
      const QByteArray data = packFiles(paths);
      QFile::remove(filePath);

      QHttpHeaders headers;
      headers.insert(0, "Content-Disposition", QString("attachment; filename=\"%1\"").arg(tex->name + ".bin"));

      qDebug() << "sending len" << data.length();
      QHttpServerResponse response("application/octet-stream", data);
      response.setHeaders(std::move(headers));
      return response;
    });
    return future;
  });

  m_server->route("/api/1/textures/image", [](const QHttpServerRequest &request) {
    QFuture<QHttpServerResponse> future = QtConcurrent::run([&request] {
      const QUrlQuery query(request.url());

      QString image_size = query.queryItemValue("image_size").toLower();
      if (image_size.isEmpty())
        return QHttpServerResponse("image_size empty, provide e.g: 1024", QHttpServerResponder::StatusCode::NotFound);

      if (!str2TextureSize.contains(image_size))
        return QHttpServerResponse("wrong image_size", QHttpServerResponder::StatusCode::NotFound);
      auto image_size_enum = str2TextureSize[image_size];

      QString image_type = query.queryItemValue("image_type").toLower();
      if (image_type.isEmpty())
        image_type = "diffuse";
      image_type = image_type[0].toUpper() + image_type.mid(1);
      if (!str2TextureImageType.contains(image_type))
        return QHttpServerResponse("wrong image_type", QHttpServerResponder::StatusCode::NotFound);
      auto image_type_enum = str2TextureImageType[image_type];

      const QString name = query.queryItemValue("name", QUrl::FullyDecoded);
      const QString asset_pack = query.queryItemValue("asset_pack", QUrl::FullyDecoded).toLower();

      const Ctx *ctx = Ctx::instance();
      QList<QSharedPointer<AssetPack>> packs = ctx->assetPackManager->packs[AssetPackType::assetPackTexture];
      QSharedPointer<AssetPack> pack;

      for (const auto &_pack: packs) {
        if (_pack->name() == asset_pack)
          pack = _pack;
      }

      if (pack.isNull())
        return QHttpServerResponse("asset_pack not found", QHttpServerResponder::StatusCode::NotFound);

      if (!pack->textures.contains(name))
        return QHttpServerResponse("texture by name not found", QHttpServerResponder::StatusCode::NotFound);

      auto tex = pack->textures[name];

      if (tex->name == name && tex->asset_pack()->name() == asset_pack) {
        auto res = tex->get_image(image_type_enum, image_size_enum);
        if (res == nullptr)
          return QHttpServerResponse("image not found", QHttpServerResponder::StatusCode::NotFound);

        auto imagePath = res->path.absoluteFilePath();
        QFile imageFile(imagePath);
        if (!imageFile.open(QIODevice::ReadOnly))
          return QHttpServerResponse("image not found", QHttpServerResponder::StatusCode::NotFound);

        const QByteArray imageData = imageFile.readAll();
        imageFile.close();

        for (const auto &b: tex->textures) {
          qDebug() << b->path.absoluteFilePath();
        }

        return QHttpServerResponse(QString("image/jpg").toUtf8(), imageData);
      }

      return QHttpServerResponse("image not found", QHttpServerResponder::StatusCode::NotFound);
    });
    return future;
  });

  m_server->route("/api/1/textures/thumbnail", [](const QHttpServerRequest &request) {
    QFuture<QHttpServerResponse> future = QtConcurrent::run([&request] {
      const QUrlQuery query(request.url());
      const QString name = query.queryItemValue("file", QUrl::FullyDecoded).toLower();
      if (name.isEmpty() || name.contains("..") || !name.endsWith(".jpg"))
        return QHttpServerResponse("Image not found", QHttpServerResponder::StatusCode::NotFound);

      const QString imagePath = globals::cacheDirectory + "/" + name;
      QFile imageFile(imagePath);
      if (!imageFile.open(QIODevice::ReadOnly))
        return QHttpServerResponse("Image not found", QHttpServerResponder::StatusCode::NotFound);

      const QByteArray imageData = imageFile.readAll();
      imageFile.close();

      return QHttpServerResponse(QString("image/jpg").toUtf8(), imageData);
    });
    return future;
  });

  m_server->route("/api/1/textures", [](const QHttpServerRequest &request) {
    const QUrlQuery query(request.url());

    const QString keyword = query.queryItemValue("q").toLower();
    const bool random_sort = query.queryItemValue("random_sort").toLower() == "true";
    QString type = query.queryItemValue("type");
    QString resolution = query.queryItemValue("resolution");
    QString format = query.queryItemValue("format");
    QString license = query.queryItemValue("license");
    QString author = query.queryItemValue("author");
    QString sort = query.queryItemValue("sort", QUrl::FullyDecoded);
    QString order = query.queryItemValue("order", QUrl::FullyDecoded);
    int page = query.queryItemValue("page").toInt();
    int limit = query.queryItemValue("limit").toInt();

    if (page <= 0)
      page = 1;
    if (limit <= 0)
      limit = 10;

    const Ctx *ctx = Ctx::instance();
    QList<QSharedPointer<Texture>> textures = ctx->assetPackManager->textures_flat;
    if (random_sort) {
      std::random_device rd;
      std::mt19937 g(rd());
      std::ranges::shuffle(textures, g);
    }

    qDebug() << "size: " << textures.size();
    QList<QSharedPointer<Texture>> results;

    int results_count = 0;
    for (const auto &tex: textures) {
      if (!keyword.isEmpty()) {
        if (!tex->name_lower.contains(keyword))
          continue;
      }

      results_count += 1;
      results << tex;

      if (results_count >= limit)
        break;
    }

    QJsonArray rtn;
    for (const QSharedPointer<Texture> &result: results) {
      QJsonObject texture;
      texture["name"] = result->name;
      texture["thumbnail"] = result->path_thumbnail().fileName();
      texture["thumbnail_url"] = "/api/1/textures/thumbnail?file=" + result->path_thumbnail().fileName();

      // sizes
      QJsonArray sizes_arr;
      QJsonArray sizes_str_arr;
      for (const auto size: result->available_sizes()) {
        sizes_arr << size;
        sizes_str_arr << textureSize2Str[size];
      }
      texture["sizes"] = sizes_arr;
      texture["sizes_str"] = sizes_str_arr;

      //
      // texture["type"] = type.isEmpty() ? "diffuse" : type;
      // texture["resolution"] = resolution.isEmpty() ? "4k" : resolution;
      // texture["format"] = format.isEmpty() ? "jpg" : format;
      texture["asset_pack"] = result->asset_pack()->name();
      texture["license"] = result->get_license();
      texture["author"] = result->get_author();
      // texture["author"] = author.isEmpty() ? "John Doe" : author;
      // texture["url"] = "/textures/" + texture["id"].toString();
      rtn.append(texture);
    }

    QJsonObject response;
    response["page"] = page;
    response["limit"] = limit;
    response["total"] = 200; // fake total count
    response["results"] = rtn;

    return QHttpServerResponse(response);
  });

  m_tcp_server = new QTcpServer(this);
  m_tcp_port = 19200;

  if (!m_tcp_server->listen(QHostAddress::Any, m_tcp_port) || !m_server->bind(m_tcp_server)) {
    qWarning() << QCoreApplication::translate("QHttpServerExample", "Server failed to listen on a port.");
  }

  int e = 1;
}
