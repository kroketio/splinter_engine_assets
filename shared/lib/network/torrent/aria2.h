#pragma once

#include <ostream>
#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QPointer>
#include <QProcess>

#include "lib/network/torrent/metainfo.h"
#include "lib/network/wsclient.h"
#include "lib/config.h"
#include "lib/utils.h"

enum Aria2Status {
    idle = 0,
    running
};

enum Aria2ItemStatus {
    disabled = 0,
    waiting,
    active,
    finished
};

struct Aria2ItemProgress {
  explicit Aria2ItemProgress(
      double total_length,
      double completed_length,
      double speed_download,
      double speed_upload,
      unsigned int seeders);
  ~Aria2ItemProgress() {
    qDebug() << "RIP Aria2ItemProgress";
  };

  unsigned int seeders;

  [[nodiscard]] QString pctHumanStr() {
    return QString("%1/%2 (%3)").arg(
      completedSizeStr(),
      totalSizeStr(),
      pctStr()
    );
  }

  [[nodiscard]] double pct() const {
    return (m_completed_length / m_total_length) * 100;
  }

  [[nodiscard]] QString pctStr() {
    return QString::number(pct(), 'G', 3) + "%";
  }

  [[nodiscard]] QString downloadSpeedStr() const {
    return QString("%1/s").arg(Utils::humanFileSize(m_speed_download));
  }

  [[nodiscard]] QString uploadSpeedStr() const {
    return QString("%1/s").arg(Utils::humanFileSize(m_speed_upload));
  }

  [[nodiscard]] QString fileSizeStr() const {
    return Utils::humanFileSize(m_total_length);
  }

  [[nodiscard]] QString totalSizeStr() const {
    return Utils::humanFileSize(m_total_length);
  }

  [[nodiscard]] QString completedSizeStr() const {
    return Utils::humanFileSize(m_completed_length);
  }


private:
  // bytes
  double m_total_length;
  double m_completed_length;
  double m_speed_download;
  double m_speed_upload;
};

class Aria2Item : public QObject {
Q_OBJECT

public:
  explicit Aria2Item(QByteArray &torrent, unsigned int uid, WebsocketClient *wsClient, QObject *parent = nullptr);
  ~Aria2Item() override;

  Aria2ItemProgress progress = Aria2ItemProgress(0, 0, 0, 0, 0);
  Aria2ItemStatus status = Aria2ItemStatus::disabled;
  MetaInfo torrentInfo;
  QString name;

  const QByteArray torrent;
  [[nodiscard]] QString torrent_b64() const {
    return QString::fromUtf8(torrent.toBase64());
  }

  void start();
  void unpause();
  void changeStatus(Aria2ItemStatus status) {
    this->status = status;
  }

  unsigned int uid;
  [[nodiscard]] QString methodUID(const QString &method) const {
    return QString("%1.%2").arg(method, QString::number(uid));
  }

  QString gid;

signals:
  void added();
  void statusChanged(Aria2ItemStatus status);
  void progressChanged(Aria2ItemProgress progress);

private slots:
  void fetchStatus();
  void onWSmessage(const QJsonObject &blob);

private:
  WebsocketClient *m_wsClient;
  QTimer *m_statusTimer;
};

class Aria2 : public QObject {
Q_OBJECT

public:
  explicit Aria2(const QString &url, QObject *parent = nullptr);
  Aria2Status status = Aria2Status::idle;
  QList<QSharedPointer<Aria2Item>> items;

  ~Aria2() override;

  void start();
  void stop();

  void getGlobalStat();
  Aria2Item* addTorrent(const QByteArray &torrent, const QDir &output);
  static void send(WebsocketClient *wsClient, const QString &uid, const QString &method, const QJsonArray &data);

private slots:
  void onWSmessage(const QJsonObject &blob);
  void onWSConnected();
  void onDownloadStart(const QString &gid);

signals:
  void downloadStart(QString gid);
  void log(QString msg);

private:
  QString m_url;
  QTimer m_someTimer;
  QString m_pathProc;

  QString m_rpcMethod;
  WebsocketClient *m_wsClient;
  QProcess *m_proc;
};

