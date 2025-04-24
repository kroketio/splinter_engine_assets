#pragma once
#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

#include "lib/utils.h"

struct TextureCacheDbItem {
  QString checksum;
  QString fn;
  bool alpha;
  unsigned int width;
  unsigned int height;
  unsigned int channels;

  TextureCacheDbItem(const QString& checksum_, const QString& fn_, bool alpha_, unsigned int width_, unsigned int height_, unsigned int channels_) : 
    checksum(checksum_), fn(fn_), alpha(alpha_), width(width_), height(height_), channels(channels_) {}
};

class SQL {
public:
  static QSqlDatabase& getInstance();
  static bool initialize(const QString& dbName);
  static void create_schema();
  static QSqlQuery exec(const QString &sql);
  static QSqlQuery& exec(QSqlQuery &q);

  // static void SQLUpdateTitle(const QString &url, const QString &title);
  // static void saveHistory();
  static void clearTextureCache(const QString &asset_pack_name, const QString &asset_pack_dir);
  static QMap<QString, QSharedPointer<TextureCacheDbItem>> getTextureCache(const QString &asset_pack_name, const QString &asset_pack_dir);
  static void setTextureCache(const QString &asset_pack_name, const QString &asset_pack_dir, const QList<QSharedPointer<TextureCacheDbItem>> &items);

private:
  SQL() = default;
  ~SQL() = default;
  SQL(const SQL&) = delete;
  SQL& operator=(const SQL&) = delete;

  static QSqlDatabase db;
};
