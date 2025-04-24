#pragma once
#include <QObject>
#include <QCoreApplication>
#include <QTcpServer>
#include <QHttpServer>

class WebServer : public QObject {
  Q_OBJECT

public:
  explicit WebServer(QObject *parent = nullptr);
  ~WebServer() override = default;

private:
  QHttpServer* m_server;
  QTcpServer* m_tcp_server;
  int m_tcp_port = 19200;
};