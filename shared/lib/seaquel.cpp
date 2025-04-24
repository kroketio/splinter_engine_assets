#include "lib/seaquel.h"

// Define the static member
QSqlDatabase SQL::db = QSqlDatabase();

QSqlDatabase& SQL::getInstance() {
  // Ensure the database is initialized
  if(!db.isValid()) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << "Database instance created.";
  }

  return db;
}

bool SQL::initialize(const QString& path_db) {
  QSqlDatabase& db = getInstance();
  db.setDatabaseName(path_db);

  if(!db.open()) {
    qDebug() << "Error: Unable to open the database!" << db.lastError().text();
    return false;
  }

  SQL::create_schema();
  qDebug() << "Database opened successfully: " << path_db;
  return true;
}

QSqlQuery SQL::exec(const QString &sql) {
  const QSqlDatabase & db = SQL::getInstance();
  QSqlQuery q(db);
  const auto res = q.exec(sql);
  if(!res) {
    const auto err = q.lastError().text();
    if (!err.contains("already exists"))
      qCritical() << "SQL error: " << err;
  }
  return q;
}

QSqlQuery& SQL::exec(QSqlQuery &q) {
  if (auto res = q.exec(); !res) {
    if (auto err = q.lastError().text(); !err.contains("already exists"))
      qCritical() << "SQL error: " << err;
  }
  return q;
}

void SQL::create_schema() {
  QSqlDatabase& db = SQL::getInstance();
  auto table_cache_texture = "create table cache_texture "
    "(id integer primary key, "
    "fn varchar(64),"
    "dir varchar(64),"
    "asset_pack varchar(32),"
    "checksum varchar(32), "
    "alpha INTEGER NOT NULL CHECK (alpha IN (0, 1)),"
    "channels INTEGER,"
    "width INTEGER,"
    "height INTEGER,"
    "CONSTRAINT unique_fn_asset_pack UNIQUE (fn, dir))";

  auto idx_cache_texture = "CREATE INDEX idx_cache_texture_query ON cache_texture (asset_pack, dir, fn, checksum, alpha, width, height, channels);";
  for(const auto table_item: {table_cache_texture, idx_cache_texture})
    SQL::exec(table_item);
}

QMap<QString, QSharedPointer<TextureCacheDbItem>> SQL::getTextureCache(const QString &asset_pack_name, const QString &asset_pack_dir) {
  const QSqlDatabase& db = SQL::getInstance();
  QSqlQuery query(db);
  query.prepare("SELECT fn, checksum, alpha, width, height, channels FROM cache_texture WHERE asset_pack = :asset_pack AND dir = :dir;");
  query.bindValue(":asset_pack", asset_pack_name);
  query.bindValue(":dir", asset_pack_dir);

  QMap<QString, QSharedPointer<TextureCacheDbItem>> rtn;
  auto q = SQL::exec(query);

  int i = 0;
  while (q.next()) {
    QString fn = q.value(0).toString();
    QString checksum = q.value(1).toString();
    bool isAlpha = q.value(2).toBool();
    const unsigned int width = q.value(3).toInt();
    const unsigned int height = q.value(4).toInt();
    const unsigned int channels = q.value(5).toInt();

    const QSharedPointer<TextureCacheDbItem> item =
      QSharedPointer<TextureCacheDbItem>(new TextureCacheDbItem(checksum, fn, isAlpha, width, height, channels));

    rtn[fn] = item;
    i++;
  }

  qDebug() << "getTextureCache results" << i << "for asset pack" << asset_pack_name;
  return rtn;
}

void SQL::clearTextureCache(const QString &asset_pack_name, const QString &asset_pack_dir) {
  qDebug() << "clearTextureCache" << asset_pack_name;
  const QSqlDatabase & db = SQL::getInstance();
  QSqlQuery query(db);
  query.prepare("DELETE FROM cache_texture WHERE asset_pack = :asset_pack AND dir = :dir;");
  query.bindValue(":asset_pack", asset_pack_name);
  query.bindValue(":dir", asset_pack_dir);
  SQL::exec(query);
}

void SQL::setTextureCache(const QString& asset_pack_name, const QString& asset_pack_dir, const QList<QSharedPointer<TextureCacheDbItem>> &items) {
  qDebug() << "setTextureCache for" << asset_pack_name << "dir" << asset_pack_dir;
  QSqlDatabase& db = SQL::getInstance();
  QSqlQuery query(db);

  query.prepare("INSERT INTO cache_texture (asset_pack, dir, fn, checksum, alpha, width, height, channels) VALUES (?, ?, ?, ?, ?, ?, ?, ?) ON CONFLICT(fn, dir) DO UPDATE SET checksum = excluded.checksum, alpha = excluded.alpha, width = excluded.width, height = excluded.height, channels = excluded.channels;");

  QVariantList packs;
  QVariantList dirs;
  QVariantList fns;
  QVariantList checksum;
  QVariantList alpha;
  QVariantList width;
  QVariantList height;
  QVariantList channels;

  for(const auto& item: items) {
    packs << asset_pack_name;
    dirs << asset_pack_dir;
    fns << item->fn;
    checksum << item->checksum;
    alpha << item->alpha;
    width << item->width;
    height << item->height;
    channels << item->channels;
  }

  query.addBindValue(packs);
  query.addBindValue(dirs);
  query.addBindValue(fns);
  query.addBindValue(checksum);
  query.addBindValue(alpha);
  query.addBindValue(width);
  query.addBindValue(height);
  query.addBindValue(channels);

  if(!db.transaction()) {
    qWarning() << "Failed to begin transaction!";
    return;
  }

  if(!query.execBatch()) {
    qWarning() << "Batch insert failed:" << query.lastError();
    db.rollback();
  } else {
    db.commit();
    qWarning() << "Rows inserted successfully!";
  }
}
