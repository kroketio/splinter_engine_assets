#pragma once

#include <QByteArray>
#include <QClipboard>
#include <QStringListModel>
#include <QCryptographicHash>
#include <QTimer>
#include <QThread>
#include <QMutex>

#include "lib/debounce.h"
#include "lib/nameof.hpp"
#include "lib/network/torrent/aria2.h"
#include "lib/network/torrent/metainfo.h"
#include "lib/globals.h"
#include "lib/seaquel.h"
#include "lib/utils.h"
#include "models/texture.h"

class Ctx;

enum AssetPackType {
  assetPackTexture = 0,
  assetPackModel,
  assetPackMusic,
  assetPackSound
};

extern const QMap<QString, AssetPackType> str2AssetPackType;

enum AssetPackStatus {
  AssetPackStatusIdle = 0,
  AssetPackStatusInstalling,
  AssetPackStatusUnpacking,
  AssetPackStatusReady
};

// ================= thread stuff

class AssetPackWorker;
class AssetPackThread;
class AssetPack;

class AssetPackWorker : public QObject {
Q_OBJECT

  public:
    explicit AssetPackWorker(
      const QList<QFileInfo> &paths,
      AssetPack* pack,
      QObject* parent = nullptr);

  public slots:
    void process();

  signals:
    void finished();
    void logMessage(const QString &msg, bool ok = true);
    void progress(int pct);
    void meta_sql_mutations(QList<QSharedPointer<TextureCacheDbItem>> items);

  private:
    QSharedPointer<TextureImage> processPath(const QFileInfo &path);

    QJsonObject m_meta_json;
    QMap<QString, QSharedPointer<TextureCacheDbItem>> m_meta_sql;

    unsigned int m_iterations;
    QList<QFileInfo> m_paths;
    AssetPack* m_pack;

    QList<QSharedPointer<TextureCacheDbItem>> m_meta_sql_mutations;
};

class AssetPackThread : public QThread {
Q_OBJECT

  public:
    void setPaths(QList<QFileInfo> paths) { m_paths = paths; }
    void setPack(AssetPack* pack) { m_pack = pack; }

  protected:
    void run() override;

  signals:
    void logMessage(const QString &msg, bool ok = true);
    void progress(int pct);
    void meta_sql_mutations(QList<QSharedPointer<TextureCacheDbItem>> items);

  private:
    AssetPackWorker* m_worker = nullptr;
    QList<QFileInfo> m_paths;
    AssetPack* m_pack = nullptr;
};

class AssetPackScanJob : public QObject {
Q_OBJECT

  public:
    explicit AssetPackScanJob(AssetPack* pack, QList<QFileInfo> paths, QObject *parent = nullptr);
    void start();
    void stop();

    std::chrono::time_point<std::chrono::high_resolution_clock> timeStart;

  public slots:
    void onThreadFinished();
    void onMetaSqlMutations(QList<QSharedPointer<TextureCacheDbItem>> items);
    void onProgressUpdated();

  signals:
    void finished();
    void progress(int pct);
    void logMessage(QString msg, bool ok = true);

  private:
    QList<QFileInfo> m_paths;
    AssetPack* m_pack = nullptr;

    QList<AssetPackWorker*> m_workers;
    QList<AssetPackThread*> m_threads;
    QList<unsigned int> m_threads_progress;

    QList<QSharedPointer<TextureCacheDbItem>> m_meta_sql_mutations;

    unsigned int m_threads_finished = 0;
    unsigned int m_iterations = 0;
    double m_last_progress = 0.0;
    unsigned int m_num_threads;
    unsigned int m_num_paths = 0;
};

// =================

class AssetPack : public QObject {
Q_OBJECT

public:
  explicit AssetPack(
    Ctx* ctx,
    const QString &name,
    const QString &dir,
    const QJsonObject &json,
    QObject *parent = nullptr);
  ~AssetPack() override = default;
  static QSharedPointer<AssetPack> fromJson(Ctx *ctx, const QFileInfo &path);

  QString name() { return m_name; }
  QString dir() { return m_dir; }

  // tags
  QList<QSharedPointer<TextureTag>> tags;
  void addTag(const QSharedPointer<TextureTag> &tag) { tags << tag; }
  void parseTags();

  QString description() { return m_description; }
  void setDescription(const QString &description);

  AssetPackType atype = AssetPackType::assetPackTexture;
  AssetPackStatus status = AssetPackStatus::AssetPackStatusIdle;

  QJsonObject meta_json() const { return m_json; }
  QMap<QString, QSharedPointer<TextureCacheDbItem>> sql_meta() const { return m_sql_meta; }

  void remove();
  void download();
  void scan_textures();
  void scan();
  void getTextureCache();

  QList<QDir> dataPaths;
  QMap<QString, QSharedPointer<Texture>> textures;
  QMap<QString, QSharedPointer<Texture>> textures_lower;  // lowercase IDs

  QSharedPointer<TextureImage> addTextureImage(const QFileInfo &path);

  // json
  static bool validate_pack_json_schema(const QJsonObject &blob);
  static QJsonObject read_pack_json(const QFileInfo &path);
  void setPathJson(const QFileInfo &path) { m_json_path = path; }
  void addDataPath(const QDir &dir);
  QMutex mutex;
  int scanProgress = 0;

  // torrent
  QString torrent_path;
  void install_from_torrent(const QString &path);
  void install_from_torrent(const QByteArray &data);

signals:
  void scanProgressUpdated(int progress);
  void finishedScan(AssetPack* pack);
  void scanLogMessage(const QString &msg, bool ok = true);

private slots:
  void onTorrentAdded();
  void onTorrentStatusChanged(Aria2ItemStatus status);
  void onTorrentProgress(Aria2ItemProgress progress);

private:
  Ctx* m_ctx = nullptr;

  QJsonObject m_json;  // json pack description file
  QFileInfo m_json_path;

  QString m_name;
  QString m_dir;
  QString m_description;

  AssetPackScanJob *m_job = nullptr;

  // checksum, isAlpha, width, height
  QMap<QString, QSharedPointer<TextureCacheDbItem>> m_sql_meta;

  // torrent
  void loadTorrent();
  Aria2Item *m_torrent = nullptr;
  QByteArray m_torrent_data;
};

class Shop : public QObject {
Q_OBJECT

public:
  explicit Shop(QObject *parent = nullptr);
  QList<QSharedPointer<AssetPack>> assetpacks;

  void load();
};
