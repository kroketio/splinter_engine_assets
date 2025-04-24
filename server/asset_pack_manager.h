#pragma once

#include <QObject>
#include <QJsonDocument>
#include <QPixmap>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <QJsonObject>

#include "models/texture_image.h"
#include "models/texture.h"
#include "models/asset_pack.h"
#include "lib/utils.h"
#include "lib/config.h"

class Ctx;
class AssetPackManager : public QObject {
Q_OBJECT

public:
  explicit AssetPackManager(Ctx* ctx, QObject *parent = nullptr);
  void load(const QString& scan_dir, QString& err);
  void scan();
  bool scanning() const { return m_scanning; }

  QSharedPointer<AssetPack> assetPackLookup(AssetPackType atype, const QString &name);
  QSharedPointer<AssetPack> assetPackLookup(AssetPack *pack);

  // flat lists / data (from `packs`)
  QList<QSharedPointer<Texture>> textures_flat;
  QList<QSharedPointer<TextureTag>> texturetags_flat;

  QMap<AssetPackType, QList<QSharedPointer<AssetPack>>> packs;   // type, List[pack]

  // flat list, by type
  QList<QSharedPointer<Texture>> list(AssetPackType atype) {
    QList<QSharedPointer<Texture>> rtn;
    return rtn;
  }

signals:
  void scanStarted();
  void scanFinished();
  void scanProgress(int progress);
  void scanLogMessage(const QString &message, bool ok = true);

private slots:
  void onAssetPackProgressUpdated(int pct);
  void onAssetPackFinishedScan(AssetPack* pack);
  void onGenerateFlatAPI();

private:
  Ctx* m_ctx;
  QJsonObject m_infoJSON;
  QJsonObject m_collectionCache;  // key = path_collection, val = dict[md5:obj]

  bool m_scanning = false;
  QList<QSharedPointer<AssetPack>> m_activeScans;
  int m_activeScansProgressPrevious = 0;
};
