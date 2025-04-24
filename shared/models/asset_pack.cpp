#include <QObject>

#include "asset_pack.h"
#include "lib/network/torrent/metainfo.h"

const QMap<QString, AssetPackType> str2AssetPackType = {
    {"texture", AssetPackType::assetPackTexture},
    {"model", AssetPackType::assetPackModel},
    {"music", AssetPackType::assetPackMusic},
    {"sound", AssetPackType::assetPackSound}};

void AssetPackThread::run() {
  m_worker = new AssetPackWorker(m_paths, m_pack, nullptr);
  connect(m_worker, &AssetPackWorker::logMessage, this,
          &AssetPackThread::logMessage);
  connect(m_worker, &AssetPackWorker::finished, this, &QThread::quit);
  connect(m_worker, &AssetPackWorker::finished, m_worker,
          &AssetPackWorker::deleteLater);
  connect(m_worker, &AssetPackWorker::progress, this,
          &AssetPackThread::progress);
  connect(m_worker, &AssetPackWorker::meta_sql_mutations, this,
          &AssetPackThread::meta_sql_mutations);
  m_worker->process();
}

AssetPackWorker::AssetPackWorker(
  const QList<QFileInfo> &paths,
  AssetPack* pack,
  QObject* parent) :
    m_paths(paths),
    m_pack(pack),
    QObject(parent) {
  qDebug() << "AssetPackWorker::AssetPackWorker";
  m_iterations = m_paths.size();
  m_meta_json = m_pack->meta_json();
  m_meta_sql = m_pack->sql_meta();
}

void AssetPackWorker::process() {
  for(int i = 0; i < m_iterations; i++) {
    this->processPath(m_paths.at(i));
    auto _progress = int(100 * double(i)/double(m_iterations));
    emit progress(_progress);
  }

  // update SQL
  if(m_meta_sql_mutations.length() > 0)
    emit meta_sql_mutations(m_meta_sql_mutations);
}

QSharedPointer<TextureImage> AssetPackWorker::processPath(const QFileInfo &path) {
  // 1. ensure Collection exists
  // 2. create new TextureImage
  // 3. generate cache

  QJsonObject meta_json_meta;
  if(m_meta_json.contains("meta"))
    meta_json_meta = m_meta_json["meta"].toObject();

  // ==== 1.
  auto basename = path.baseName();
  auto texinfo = TextureImageInfo(basename);
  if(!texinfo.success) {
    emit logMessage(QString("%1: %2").arg(basename, texinfo.errorString), false);
    return nullptr;
  }

  emit logMessage(basename);
  QSharedPointer<Texture> collection;

  m_pack->mutex.lock();
  if(!m_pack->textures.contains(texinfo.name)) {
    collection = QSharedPointer<Texture>(new Texture(texinfo.name, this));
    m_pack->textures[texinfo.name] = collection;
    m_pack->textures_lower[texinfo.name.toLower()] = collection;
    collection->set_asset_pack(m_pack);
    m_pack->mutex.unlock();

    // assign meta (author/license/tags)
    if(meta_json_meta.contains(texinfo.name)) {
      QJsonObject blob = meta_json_meta[texinfo.name].toObject();
      if(blob.contains("author"))
        collection->set_author(blob["author"].toString());
      if(blob.contains("license"))
        collection->set_license(blob["license"].toString());
      if(blob.contains("tags")) {
        for(const auto &tag_val: blob["tags"].toArray()) {
          const int tag_idx = tag_val.toInt();
          QSharedPointer<TextureTag> tag = m_pack->tags.at(tag_idx);
          collection->append_tag(tag);
          tag->score += 1;
        }
      }
    }
  } else {
    collection = m_pack->textures[texinfo.name];
    m_pack->mutex.unlock();
  }

  // ==== 2.
  const TextureImageExt ext = path.suffix() == "png" ? TextureImageExt::PNG : TextureImageExt::JPG;
  QSharedPointer<TextureImage> tex =
      QSharedPointer<TextureImage>(new TextureImage(texinfo, ext, this));
  tex->setPath(path);

  // for the diffuse (the rest can be on-demand)
  if(texinfo.type == TextureImageType::diffuse) {
    if(m_meta_sql.contains(tex->name)) {
      const QSharedPointer<TextureCacheDbItem> item = m_meta_sql[tex->name];
      tex->isAlpha = item->alpha;
      tex->checksum = item->checksum;
      tex->dimensions = {static_cast<int>(item->width), static_cast<int>(item->height)};
      tex->channels = item->channels;
    } else {
      tex->metadata_generate();
      const auto item = QSharedPointer<TextureCacheDbItem>(
        new TextureCacheDbItem(tex->checksum, tex->name, tex->isAlpha, tex->dimensions.width(), tex->dimensions.height(), tex->channels));
      m_meta_sql[tex->name] = item;  // @TODO: this is probably on the stack, for this thread
      m_meta_sql_mutations << item;
    }

    // generate thumbnail
    QString err;
    tex->ensure_thumbnail(false, err); // hasThumbnail plz
    if(!err.isEmpty()) {
        emit logMessage(QString("%1: %2").arg(basename, err), false);
    }
  }

  collection->setTexture(tex);
  return tex;
}

AssetPackScanJob::AssetPackScanJob(
	AssetPack* pack,
	QList<QFileInfo> paths,
	QObject *parent) :
		m_pack(pack),
        m_paths(paths),
		QObject(parent) {

  m_num_threads = Utils::getCpuCount();
  if (m_num_threads > 8)
    m_num_threads = 8;

  m_num_paths = m_paths.size();
}

void AssetPackScanJob::onProgressUpdated() {
  const auto whole = double(m_threads_progress.length() * 100);
  const auto part = double(
    std::accumulate(m_threads_progress.begin(),
    m_threads_progress.end(), 0));

  auto _progress = static_cast<int>(100 * static_cast<double>(part) / static_cast<double>(whole));
  if(_progress > m_last_progress) {
    emit progress(_progress);
    m_last_progress = _progress;
  }
}

void AssetPackScanJob::onThreadFinished() {
  // qDebug() << "thread finished";
  m_threads_finished += 1;
  if(m_threads_finished == m_threads.size()) {
    qDebug() << "all threads done";

    // update SQL
    if (!m_meta_sql_mutations.isEmpty())
      SQL::setTextureCache(m_pack->name(), m_pack->dir(), m_meta_sql_mutations);

    emit finished();
  }
}

void AssetPackScanJob::onMetaSqlMutations(QList<QSharedPointer<TextureCacheDbItem>> items) {
  for (const auto&item: items)
    m_meta_sql_mutations << item;
}

void AssetPackScanJob::start() {
  timeStart = Utils::timeStart();

  const int base_size = m_num_paths / m_num_threads;
  const int remainder = m_num_paths % m_num_threads;

  int segmentSize[m_num_threads];
  for (int i = 0; i < m_num_threads; ++i) {
    segmentSize[i] = base_size + (i < remainder ? 1 : 0);
  }

  QList<QFileInfo> a;
  QList<QFileInfo> b;
  QList<QFileInfo> c;
  QList<QFileInfo> d;
  QList<QFileInfo> e;
  QList<QFileInfo> f;

  // distribute
  int index = 0;
  for (int i = 0; i < m_num_threads; ++i) {
    for (int j = 0; j < segmentSize[i]; ++j) {
      if (i == 0) a.append(m_paths.at(index));
      else if (i == 1) b.append(m_paths.at(index));
      else if (i == 2) c.append(m_paths.at(index));
      else if (i == 3) d.append(m_paths.at(index));
      else if (i == 4) e.append(m_paths.at(index));
      else f.append(m_paths.at(index));
      ++index;
    }
  }

  unsigned int idx = 0;
  for(const auto &_paths: {a, b, c, d, e, f}) {
    if(!_paths.isEmpty()) {
      m_threads_progress << 0;

      auto thread = new AssetPackThread();
      connect(thread, &AssetPackThread::logMessage, this, &AssetPackScanJob::logMessage);
      connect(thread, &AssetPackThread::finished, this, &AssetPackScanJob::onThreadFinished);
      connect(thread, &AssetPackThread::meta_sql_mutations, this, &AssetPackScanJob::onMetaSqlMutations);
      connect(thread, &AssetPackThread::progress, [this, idx](int pct) {
        if (pct > m_threads_progress[idx]) {
          m_threads_progress[idx] = pct;
          this->onProgressUpdated();
        }
      });

      connect(thread, &AssetPackThread::finished, [this, thread]{
        thread->deleteLater();
      });

      thread->setPack(m_pack);
      thread->setPaths(_paths);
      m_threads << thread;
    }
    idx++;
  }

  for (AssetPackThread* thread : m_threads) {
    if (thread) {
      qDebug() << "starting thread";
      thread->start();
    }
  }
}

void AssetPackScanJob::stop() {

}

AssetPack::AssetPack(
  Ctx* ctx,
  const QString &name,
  const QString &dir,
  const QJsonObject &json,
  QObject *parent) :
    m_ctx(ctx),
    m_name(name),
    m_dir(dir),
    m_json(json),
    QObject(parent) {
  qDebug() << "AssetPack constructor";

  auto type = m_json["type"].toString();
  atype = str2AssetPackType[type];

  // const auto cache_dir = config()->get(ConfigKeys::CacheDir).toString();
  // const auto fn_cache_json = QString(QCryptographicHash::hash(QString(m_name).toUtf8(), QCryptographicHash::Md5).toHex());
}

QSharedPointer<AssetPack> AssetPack::fromJson(Ctx* ctx, const QFileInfo &path) {
  auto blob = AssetPack::read_pack_json(path);
  auto _name = blob["name"].toString();
  auto _path = blob["path"].toString();

  auto* assetPack = new AssetPack(ctx, _name, _path, blob);
  auto description = blob["description"].toString();
  assetPack->setDescription(description);
  assetPack->setPathJson(path);
  assetPack->getTextureCache();
  assetPack->parseTags();

  QDir subdir;
  auto asset_path = blob["path"].toString();
  subdir.setPath(QDir::cleanPath(path.absolutePath() + QDir::separator() + asset_path));
  if(!subdir.exists()) {
    qWarning() << "dir does not exist:" << subdir.absolutePath();
    return nullptr;
  }

  assetPack->addDataPath(subdir);
  return QSharedPointer<AssetPack>(assetPack);
}

void AssetPack::parseTags() {
  QJsonArray tags;
  if(!m_json.contains("tags"))
    return;

  unsigned int tag_count = 0;
  tags = m_json["tags"].toArray();
  for (const auto& tag : tags) {
    QString tag_str = tag.toString();
    QSharedPointer<TextureTag> _tag;
    if(CACHE_TEXTURE_TAGS.contains(tag_str))
      _tag = CACHE_TEXTURE_TAGS[tag_str];
    else {
      _tag = QSharedPointer<TextureTag>(new TextureTag(tag_str));
      CACHE_TEXTURE_TAGS[tag_str] = _tag;
    }

    this->addTag(_tag);
    tag_count += 1;
  }

  qDebug() << "created" << tag_count << "new TextureTag's";
}

void AssetPack::getTextureCache() {
    m_sql_meta = SQL::getTextureCache(m_name, m_dir);
}

void AssetPack::setDescription(const QString &description) {
  m_description = description;
}

void AssetPack::addDataPath(const QDir &dir) {
  dataPaths << dir;
}

void AssetPack::scan() {
  qDebug() << "AssetPack::scanstart" << m_name;

  QRegularExpression rx(".*.(png|jpg)$");
  QList<QFileInfo> paths;
  for(const QDir &path: dataPaths) {
    qDebug() << "scan_textures:" << path.absolutePath();
    for (const QFileInfo& _path: Utils::fileFind(rx, path.absolutePath(), 0, 2, 500000))
      paths.append(_path);
  }

  qDebug() << "scan_textures" << paths.size();
  m_job = new AssetPackScanJob(this, paths, this);

  connect(m_job, &AssetPackScanJob::logMessage, [this](QString msg, bool ok) {
    emit scanLogMessage(QString("[%1] %2").arg(m_name, msg), ok);
  });

  connect(m_job, &AssetPackScanJob::progress, [this](int pct) {
    scanProgress = pct;
    emit scanProgressUpdated(pct);
  });

  connect(m_job, &AssetPackScanJob::finished, [this]() {
      Utils::timeEnd("AssetPack::scan_textures()", m_job->timeStart);
      emit finishedScan(this);
  });

  m_job->start(); // free?
}

QJsonObject AssetPack::read_pack_json(const QFileInfo &path) {
  if(!Utils::fileExists(path.canonicalFilePath())) {
    qWarning() << path.canonicalFilePath() << "does not exist";
  }

  qDebug() << "reading" << path.canonicalFilePath();
  QJsonDocument json = QJsonDocument::fromJson(Utils::fileOpen(path.canonicalFilePath()));
  auto blob = json.object();

  if(!AssetPack::validate_pack_json_schema(blob)) {
    qWarning() << "validate_pack_json_schema failed";
  }

  return blob;
}

bool AssetPack::validate_pack_json_schema(const QJsonObject &blob) {
  auto name = blob["name"].toString();
  auto type = blob["type"].toString();
  auto description = blob["description"].toString();
  auto asset_path = blob["path"].toString();
  QString error;

  if(name.isEmpty() || name.length() > 32 || name.length() < 2) {
    error = "key 'name' invalid or too short/long";
    qWarning() << error;
    return false;
  }

  if(type.isEmpty()) {
    error = "key 'type' not defined";
    qWarning() << error;
    return false;
  } else {
    if(!str2AssetPackType.contains(type)) {
      qWarning() << "unknown asset pack type:" << type;
      return false;
    }
  }

  if(asset_path.isEmpty()) {
    error = "key 'path' not defined";
    qWarning() << error;
    return false;
  }

  return true;
}

void AssetPack::download() {
  // m_torrent = m_ctx->aria2->addTorrent(m_torrent->torrent_b64(), {m_ctx->configDirectoryAssetsTextures});
  // connect(m_torrent, &Aria2Item::added, this, &AssetPack::onTorrentAdded);
  // connect(m_torrent, &Aria2Item::progressChanged, this, &AssetPack::onTorrentProgress);
  // connect(m_torrent, &Aria2Item::statusChanged, this, &AssetPack::onTorrentStatusChanged);
}

void AssetPack::onTorrentAdded() {
  m_torrent->start();
}

void AssetPack::onTorrentStatusChanged(Aria2ItemStatus status) {
  int i = 1;
}

void AssetPack::onTorrentProgress(Aria2ItemProgress progress) {
  int i = 1;
}

Shop::Shop(QObject *parent) : 
    QObject(parent) {
  int eerw = 1;
  this->load();
}

void Shop::load() {
  // auto data = Utils::fileOpen(":/assetpacks.json");
  // auto doc = QJsonDocument::fromJson(data);
  // if(!doc.isArray()) {
  //  qWarning() << "fuck";
  //  return;
  // }

  // auto arr = doc.toArray();
  // for(const auto &obj: arr.values()) {
  //   auto blob = obj.toObject();
  //   auto name = blob["name"].toString();
  //   auto torrent = blob["torrent"].toByteArray(); // @TODO: hmm 
  //   auto atype_str = blob["type"].toString();
  //   //new QSharedPointer<AssetPack> z;
  // }
}
