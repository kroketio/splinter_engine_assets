#pragma once

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QPointer>
#include "lib/config.h"

class WebsocketClient : public QObject {
Q_OBJECT

public:
  explicit WebsocketClient(const QString &url, QObject *parent = nullptr);
  ~WebsocketClient() override;
  void start();
  void restart();
  void stop();
  void sendMsg(const QByteArray &data);

  QWebSocket *webSocket;

signals:
  void connectionEstablished();
  void WSMessage(QJsonObject message);

private slots:
  void onConnected();
  void onDisconnected();
  void onStateChanged(QAbstractSocket::SocketState state);
  void onbinaryMessageReceived(const QByteArray &message);
  void onMessageReceived(const QString &message);
  void onError(QAbstractSocket::SocketError error);
  void onConnectionTimeout();

private:
  QUrl m_url;
  QTimer m_pingTimer;
  QTimer m_connectionTimeout;
  int m_timeout = 20;
  bool m_stopped = false;
};

