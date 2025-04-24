#include <filesystem>

#include <QObject>
#include <QDir>
#include <QStandardPaths>

#include "ctx.h"
#include "lib/utils.h"
//#include "lib/vmfpp/vmfpp.h"

#include <chrono>
#include "lib/zlib_compressor.h"
#include "lib/file_packer.h"
#include "lib/godot.h"

using namespace std::chrono;

Ctx::Ctx() {
  g_instance = this;
  Utils::init();

  assetPackManager = new AssetPackManager(this);
  config()->set(ConfigKeys::CacheDir, globals::cacheDirectory);

  createConfigDirectory(QStringList({
    globals::configDirectory,
    globals::cacheDirectory,
    globals::configDirectoryAutoMasker,
    globals::configDirectoryAssets,
    globals::configDirectoryAssetsTextures,
    globals::configDirectoryAssetsU2net,
    globals::configDirectoryAssetsModels}));
  unpackAppArtifacts();

  if(!SQL::initialize(globals::pathDatabase.absoluteFilePath()))
    throw QString("Cannot open db at %1").arg(globals::pathDatabase.absoluteFilePath()).toStdString();

  webServer = new WebServer(this);

  QString err;
  assetPackManager->load(globals::configDirectoryAssetsTextures, err);
  assetPackManager->scan();

  connect(assetPackManager, &AssetPackManager::scanProgress, [=](const int pct) {
    // qDebug() << "Scan progress" << pct;
      if (pct == 100) {
        godot::initGodotResourceTemplates();
      }
  });
}

void Ctx::unpackAppArtifacts() {
  // Python auto-masker
  auto dest_auto_masker = globals::configDirectory;
  auto files_auto_masker = {
    ":/tools/masker/masker.sh",
    ":/tools/masker/masker.py"
  };

  for(const auto &fp: files_auto_masker) {
    QFile f(fp);
    QFileInfo fileInfo(f);

    auto to_path = QString("%1/%2").arg(globals::configDirectoryAutoMasker, fileInfo.fileName());
    qDebug() << "writing" << to_path;
    f.copy(to_path);
    f.setPermissions(QFile::ExeUser);
    f.close();
  }
}

void Ctx::createConfigDirectory(const QStringList &lst) {
  for(const auto &d: lst) {
    if(!std::filesystem::exists(d.toStdString())) {
      qDebug() << QString("Creating directory: %1").arg(d);
      if(!QDir().mkpath(d))
        throw std::runtime_error("Could not create directory " + d.toStdString());
    }
  }
}

//void Ctx::aria() {
//  //aria2 = new Aria2("ws://127.0.0.1:1337/jsonrpc");
//  //  auto z = config()->get(ConfigKeys::TextureMetadata).toByteArray();
//  //  auto doc = QJsonDocument::fromJson(z);
//  //  auto arr = doc.array();
//  //  qDebug() << arr.size();
//}

//void Ctx::automask(){
//  imageAutoMasker = new ImageAutoMasker(this);
//  //  imageAutoMasker->detect(
//  //    "/path/to/src/tools/test.jpg");
//  //    "/path/to/src/tools/out.png");
//}

void Ctx::onApplicationLog(const QString &msg) {
}

Ctx::~Ctx() {}

Ctx* g_instance = nullptr;