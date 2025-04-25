#include "asset_pack_manager.h"
#include "ctx.h"

AssetPackManager::AssetPackManager(Ctx* ctx, QObject *parent) :
    m_ctx(ctx), QObject(parent) {
  qDebug() << "AssetPackManager constructor";
}

void AssetPackManager::load(const QString &scan_dir, QString &err) {
  qDebug() << "scanning" << scan_dir;
  if(!Utils::dirExists(scan_dir)) {
    err = QString("%1 does not exist").arg(scan_dir);
    qWarning() << err;
    return;
  }

  QRegularExpression rx(".*.json");
  QList<QFileInfo> paths = Utils::fileFind(rx, scan_dir, 0, 0, 5000);

  for(const auto &path_info: paths) {
    const QSharedPointer<AssetPack> ptr = AssetPack::fromJson(m_ctx, path_info);
    const AssetPack* pack = ptr.data();

    disconnect(pack, &AssetPack::finishedScan, this, &AssetPackManager::onAssetPackFinishedScan);
    disconnect(pack, &AssetPack::scanProgressUpdated, this, &AssetPackManager::onAssetPackProgressUpdated);
    disconnect(pack, &AssetPack::scanLogMessage, this, &AssetPackManager::scanLogMessage);
    connect(pack, &AssetPack::finishedScan, this, &AssetPackManager::onAssetPackFinishedScan);
    connect(pack, &AssetPack::scanProgressUpdated, this, &AssetPackManager::onAssetPackProgressUpdated);
    connect(pack, &AssetPack::scanLogMessage, this, &AssetPackManager::scanLogMessage);

    if(!packs.contains(pack->atype))
      packs[pack->atype] = QList<QSharedPointer<AssetPack>>();
    packs[pack->atype] << ptr;
  }
}

void AssetPackManager::scan() {
  for (const auto& atype: {AssetPackType::assetPackTexture}) {
    for (const QSharedPointer pack: packs[atype]) {
      pack->scan();

      if (m_activeScans.isEmpty())
        emit scanStarted();
      m_activeScans << pack;
    }
  }

  m_scanning = !m_activeScans.isEmpty();
}

void AssetPackManager::onAssetPackProgressUpdated(int pct) {
  const auto whole = m_activeScans.length() * 100;
  unsigned int part = 0;
  for(auto const &assetPack: m_activeScans)
    part += assetPack->scanProgress;

  auto _progress = static_cast<int>(100 * static_cast<double>(part) / static_cast<double>(whole));
  if (_progress > m_activeScansProgressPrevious) {
    emit scanProgress(_progress);
    m_activeScansProgressPrevious = _progress;
  }
}

void AssetPackManager::onAssetPackFinishedScan(AssetPack* pack) {
  if (auto ptr = assetPackLookup(pack); m_activeScans.contains(ptr))
    m_activeScans.removeAll(ptr);

  if (m_activeScans.isEmpty()) {
    this->onGenerateFlatAPI();
    qDebug() << "AssetPackManager scanFinished";
    m_scanning = false;
    emit scanProgress(100);
    emit scanFinished();
  }
}

void AssetPackManager::onGenerateFlatAPI() {
  qDebug() << "onGenerateFlatAPI()";

  // fills the *_flat variables
  textures_flat.clear();
  texturetags_flat.clear();

  // textures
  if(packs.contains(AssetPackType::assetPackTexture)) {
    auto _packs = packs[AssetPackType::assetPackTexture];
    QStringList tags_added;

    for(const auto &p: _packs) {
      for(const auto &t: p->textures.values()) {
        textures_flat << t;

        for (auto it = t->tags().cbegin(); it != t->tags().cend(); ++it) {
          QSharedPointer<TextureTag> tval = it.value();
          if(!tags_added.contains(tval->name)) {
            texturetags_flat << tval;
            tags_added << tval->name;
          }
        }
      }
    }
  }

  std::sort(texturetags_flat.begin(), texturetags_flat.end(), [](
      const QSharedPointer<TextureTag>& a,
      const QSharedPointer<TextureTag>& b)->bool{
    return a->score > b->score;
  });
}

QSharedPointer<AssetPack> AssetPackManager::assetPackLookup(AssetPack* pack) {
  if(!packs.contains(pack->atype))
    packs[pack->atype] = QList<QSharedPointer<AssetPack>>();

  for(auto &_pack: packs[pack->atype]) {
    if(_pack->name() == pack->name())
      return _pack;
  }

  return nullptr;
}

QSharedPointer<AssetPack> AssetPackManager::assetPackLookup(AssetPackType atype, const QString &name) {
  if(!packs.contains(atype))
    packs[atype] = QList<QSharedPointer<AssetPack>>();

  for(auto &pack: packs[atype]) {
    if(pack->name() == name)
      return pack;
  }

  return nullptr;
}
