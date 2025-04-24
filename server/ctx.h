#pragma once

#include <QObject>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QTimer>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QMutex>
#include <QDir>

#include "lib/config.h"
#include "models/treemodel.h"

#if defined(Q_OS_LINUX) and defined(PLUGIN_AUTOMASK)
#include "lib/image_auto_masker.h"
#endif

#include "asset_pack_manager.h"
#include "lib/globals.h"
#include "lib/network/torrent/aria2.h"
#include "lib/seaquel.h"
#include "models/texture.h"
#include "models/texture_image.h"
#include "webserver.h"

class Ctx;
extern Ctx* g_instance;

class Ctx : public QObject {
Q_OBJECT

public:
  explicit Ctx();
  ~Ctx() override;

  bool isDebug;
  QString preloadModel;

  AssetPackManager *assetPackManager;
  WebServer *webServer;
  Aria2 *aria2;
  // ImageAutoMasker *imageAutoMasker;
  // TextureModel *textureModel;
  // TextureProxyModel *textureProxyModel;
  // TextureQMLProvider *textureQmlProvider;
  // TreeModel *treez;

  QString configRoot;
  QString homeDir;
  void readInfoJSON(QString path);

  static Ctx* instance() {
    return g_instance;
  }

signals:
  void applicationLog(const QString &msg);
  void windowTitle(QString title);

private slots:
  void onApplicationLog(const QString &msg);

private:
  QSqlDatabase m_db;

  void unpackAppArtifacts();
  static void createConfigDirectory(const QStringList &lst);
};
