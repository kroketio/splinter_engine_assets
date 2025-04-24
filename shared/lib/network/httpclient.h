#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonDocument>

class HttpClient : public QObject
{
Q_OBJECT

public:
  explicit HttpClient(QObject* parent = nullptr);

  bool busy = false;
  void getJSON(const QString& url);

signals:
  void requestComplete(const QJsonDocument& response);
  void requestFailed(const QString& errorString);

private slots:
  void requestTimeout();
  void handleJSONReply();

private:
  QNetworkAccessManager *m_networkManager;
  QNetworkReply *m_reply = nullptr;
  QTimer *m_timer;
};

