#include <QCoreApplication>

#include "aria2.h"
#include "lib/config.h"

Aria2ItemProgress::Aria2ItemProgress(
    double total_length,
    double completed_length,
    double speed_download,
    double speed_upload,
    unsigned int seeders) :
    seeders(seeders),
    m_total_length(total_length),
    m_completed_length(completed_length),
    m_speed_download(speed_download),
    m_speed_upload(speed_upload) {
}

Aria2Item::Aria2Item(QByteArray &torrent, unsigned int uid, WebsocketClient *wsClient, QObject *parent) :
    m_wsClient(wsClient),
    torrent(torrent),
    uid(uid),
    m_statusTimer(new QTimer(this)),
    QObject(parent) {

  torrentInfo.parse(torrent);
  name = torrentInfo.name();

  connect(m_statusTimer, &QTimer::timeout, this, &Aria2Item::fetchStatus);
  connect(wsClient, &WebsocketClient::WSMessage, this, &Aria2Item::onWSmessage);

  m_statusTimer->setInterval(2 * 1000);
  m_statusTimer->start();
}

void Aria2Item::start() {
  this->unpause();
}


void Aria2Item::unpause() {
  if(gid.isEmpty())
    return;

  QJsonArray rtn;
  rtn << gid;

  auto method_uid = this->methodUID("aria2.unpause");
  Aria2::send(m_wsClient, method_uid, "aria2.unpause", rtn);
}

void Aria2Item::fetchStatus() {
  // runs every X seconds
  if(gid.isEmpty())
    return;

  QJsonArray rtn;
  rtn << gid;

  auto method_uid = this->methodUID("aria2.tellStatus");
  Aria2::send(m_wsClient, method_uid, "aria2.tellStatus", rtn);
}

void Aria2Item::onWSmessage(const QJsonObject &blob) {
  if(!blob.contains("id")) return;

  auto _uid = blob["id"].toString();
  if(_uid == methodUID("aria2.addTorrent")) {
    gid = blob["result"].toString();
    emit added();
  } else if(_uid == methodUID("aria2.tellStatus")) {
    auto result = blob["result"].toObject();

    auto completed_length = result["completedLength"].toString().toDouble();
    auto totalLength = result["totalLength"].toString().toDouble();
    auto downloadSpeed = result["downloadSpeed"].toString().toDouble();
    auto uploadSpeed = result["uploadSpeed"].toString().toDouble();
    auto seeders = result["numSeeders"].toString().toInt();

    progress = Aria2ItemProgress(totalLength, completed_length, downloadSpeed, uploadSpeed, seeders);
    qDebug() << progress.pctHumanStr();
    qDebug() << progress.downloadSpeedStr();
    emit progressChanged(progress);
  }

  // {"id":1,"jsonrpc":"2.0","result":"2e4ce4e14df97f86"}
  //  if(!blob.contains("result")) return;
  //  auto res = blob["result"].toString();
}

Aria2Item::~Aria2Item() = default;


Aria2::Aria2(const QString &url, QObject *parent)
    : QObject(parent),
    m_wsClient(new WebsocketClient(url, this)),
    m_proc(new QProcess(this)),
    m_someTimer(new QTimer(this)),
    m_url(url)
{
  connect(m_wsClient, &WebsocketClient::connectionEstablished, this, &Aria2::onWSConnected);
  connect(m_wsClient, &WebsocketClient::WSMessage, this, &Aria2::onWSmessage);
  connect(this, &Aria2::downloadStart, this, &Aria2::onDownloadStart);

  m_wsClient->start();
  m_someTimer.setInterval(30 * 1000);
}

// # tellStatus:
// # bittorrent":{"announceList":[["https:\/\/torrent.ubuntu.com\/announce"],["https:\/\/ipv6.torrent.ubuntu.com\/announce"]],"comment":"Ubuntu CD releases.ubuntu.com","creationDate":1714046391,"info":{"name":"ubuntu-24.04-desktop-amd64.iso"},"mode":"single"},"completedLength":"2621440","connections":"1","dir":"\/tmp\/x","downloadSpeed":"27272","files":[{"completedLength":"2621440","index":"1","length":"6114656256","path":"\/tmp\/x\/ubuntu-24.04-desktop-amd64.iso","selected":"true","uris":[]}],"gid":"584cb0276e642452","infoHash":"2aa4f5a7e209e54b32803d43670971c4c8caaa05","numPieces":"23326","numSeeders":"0","pieceLength":"262144","seeder":"false","status":"active","totalLength":"6114656256","uploadLength":"0","uploadSpeed":"0"}}

Aria2Item* Aria2::addTorrent(const QByteArray &torrent, const QDir &output) {
  auto meta = MetaInfo();
  meta.parse(torrent);
  // QSharedPointer<Aria2Item> item
//  items << item;
//  auto method_uid = item->methodUID("aria2.addTorrent");
//
//  QJsonArray p;
//  p << item->torrent_b64();
//  p << QJsonArray();
//
//  QJsonObject obj;
//  obj["dir"] = output.absolutePath();
//  obj["pause"] = "true";
//  p << obj;
//
//  Aria2::send(m_wsClient, method_uid, "aria2.addTorrent", p);
  return {};
}

void Aria2::onDownloadStart(const QString &gid) {
  for(const auto &item: items) {
    if(item->gid == gid) {
      item->changeStatus(Aria2ItemStatus::active);
      break;
    }
  }
}

void Aria2::onWSmessage(const QJsonObject &blob) {
  QString m;
  QJsonArray p;
  if(blob.contains("method") && blob.contains("params")) {
    m = blob["method"].toString();
    p = blob["params"].toArray();
    QJsonObject obj;

    if(m == "aria2.onDownloadStart") {
      obj = p.at(0).toObject();
      if(obj.contains("gid")) {
        emit onDownloadStart(obj["gid"].toString());
      } else {
        qWarning() << "aria2.onDownloadStart, gid missing";
      }
    }
    return;
  }

  m = m_rpcMethod;
  if(m == "aria2.addTorrent") {

  }

  // {"id":1,"jsonrpc":"2.0","result":{"downloadSpeed":"0","numActive":"0","numStopped":"0","numStoppedTotal":"0","numWaiting":"0","uploadSpeed":"0"}}
}

void Aria2::onWSConnected() {
  // some default items
  auto torrent = Utils::fileOpen(":/assets/torrents/pack1.torrent");
//  auto x = QSharedPointer<Aria2Item>(new Aria2Item(torrent, 1, m_wsClient, this));
//  this->addTorrent(x, QDir{"/tmp/x"});
}

//# https://aria2.github.io/manual/en/html/aria2c.html
//#
//# aria2.getPeers(gid)
//#
//# aria2.changeGlobalOption / aria2.getGlobalOption
//# - max-overall-download-limit
//# - max-overall-upload-limit
//#
//# aria2.addUri()   # magnet
//# aria2.addTorrent()
//# aria2.pauseAll()
//# aria2.forcePauseAll()
//
//# d = {"jsonrpc": "2.0","id":1, "method": "aria2.getGlobalStat", "params":[]}
//# blob = json.dumps(d)

void Aria2::getGlobalStat() {
  QJsonArray rtn;
  Aria2::send(m_wsClient, "x", "aria2.getGlobalStat", rtn);
}

void Aria2::send(WebsocketClient *wsClient, const QString &uid, const QString &method, const QJsonArray &params) {
  QJsonObject obj;
  obj["jsonrpc"] = "2.0";
  obj["id"] = uid;
  obj["method"] = method;

  QJsonDocument doc;
  obj["params"] = params;
  doc.setObject(obj);
  auto data = doc.toJson(QJsonDocument::Indented);
  qDebug() << "WebSocket send" << data;

  // fprintf(stderr, "%s\n", data.toStdString().c_str());
  wsClient->sendMsg(data);
}

void Aria2::start() {
  if(status != Aria2Status::idle) return;
  m_proc->start();

  auto state = m_proc->state();
  if (state == QProcess::ProcessState::Running || state == QProcess::ProcessState::Starting) {
      emit log("Can't start Tor, already running or starting");
      return;
  }

  // @TODO: cycle up
  if (Utils::portOpen("127.0.0.", 1337)) {
      emit log(QString("Unable to start Aria2 on %1:%2. Port already in use.").arg("127.0.0.1", 1337));
      return;
  }

  qDebug() << QString("Start process: %1").arg(this->m_pathProc);
  QStringList arguments;

  arguments << "--disable-ipv6";
  arguments << "--enable-rpc=true";
  arguments << "--rpc-listen-all=false";
  arguments << "--rpc-listen-port=1337";
  arguments << "--rpc-secure=false";
  arguments << "--no-conf=true";

  qDebug() << QString("%1 %2").arg(this->m_pathProc, arguments.join(" "));

  m_proc->start(this->m_pathProc, arguments);
}

void Aria2::stop() {
  if(status != Aria2Status::idle) return;
  m_proc->terminate();
}

Aria2::~Aria2() = default;

