#include <QCoreApplication>

#include "wsclient.h"
#include "lib/utils.h"
#include "lib/config.h"

WebsocketClient::WebsocketClient(const QString &url, QObject *parent)
    : QObject(parent),
    webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this)),
    m_url(url)
{
  connect(webSocket, &QWebSocket::stateChanged, this, &WebsocketClient::onStateChanged);
  connect(webSocket, &QWebSocket::connected, this, &WebsocketClient::onConnected);
  connect(webSocket, &QWebSocket::disconnected, this, &WebsocketClient::onDisconnected);

  // >= Qt6.5
  // connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), this, &WebsocketClient::onError);

  connect(webSocket, &QWebSocket::binaryMessageReceived, this, &WebsocketClient::onbinaryMessageReceived);
  connect(webSocket, &QWebSocket::textMessageReceived, this, &WebsocketClient::onMessageReceived);

  // Keep websocket connection alive
  connect(&m_pingTimer, &QTimer::timeout, [this]{
      if (webSocket->state() == QAbstractSocket::ConnectedState) {
          webSocket->ping();
      }
  });
  m_pingTimer.setInterval(30 * 1000);
  m_pingTimer.start();

  connect(&m_connectionTimeout, &QTimer::timeout, this, &WebsocketClient::onConnectionTimeout);
}

void WebsocketClient::sendMsg(const QByteArray &data) {
  qDebug() << ">" << data;
  if (webSocket->state() == QAbstractSocket::ConnectedState) {
    webSocket->sendBinaryMessage(data);
  }
}

void WebsocketClient::start() {
  if (m_stopped) {
    return;
  }

  // connect & reconnect on errors/close
  auto state = webSocket->state();
  if (state != QAbstractSocket::ConnectedState && state != QAbstractSocket::ConnectingState) {
    qDebug() << "WebSocket connect:" << m_url.url();
    webSocket->open(m_url);
  }
}

void WebsocketClient::restart() {
  m_stopped = false;
  this->start();
}

void WebsocketClient::stop() {
  m_stopped = true;
  webSocket->close();
  m_connectionTimeout.stop();
}

void WebsocketClient::onConnected() {
  qDebug() << "WebSocket connected";
  emit connectionEstablished();
}

void WebsocketClient::onDisconnected() {
  qDebug() << "WebSocket disconnected";
  QTimer::singleShot(1000, [this]{this->start();});
}

void WebsocketClient::onStateChanged(QAbstractSocket::SocketState state) {
  if (state == QAbstractSocket::ConnectingState) {
    m_connectionTimeout.start(m_timeout*1000);
  }
  else if (state == QAbstractSocket::ConnectedState) {
    m_connectionTimeout.stop();
  }
}

void WebsocketClient::onError(QAbstractSocket::SocketError error) {
  qCritical() << "WebSocket error: " << error;
  auto state = webSocket->state();
  if (state == QAbstractSocket::ConnectedState || state == QAbstractSocket::ConnectingState) {
    webSocket->abort();
  }
}

void WebsocketClient::onConnectionTimeout() {
  qWarning() << "Websocket connection timeout";
  m_timeout = std::min(m_timeout + 5, 60);
  m_connectionTimeout.setInterval(m_timeout*1000);
  this->onDisconnected();
}

void WebsocketClient::onMessageReceived(const QString &message) {
  this->onbinaryMessageReceived(message.toUtf8());
}

void WebsocketClient::onbinaryMessageReceived(const QByteArray &message) {
  qDebug() << "WebSocket received:" << message;

  if (!Utils::validateJSON(message)) {
    qCritical() << "Could not interpret WebSocket message as JSON";
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(message);
  QJsonObject object = doc.object();

  emit WSMessage(object);
}

WebsocketClient::~WebsocketClient() = default;