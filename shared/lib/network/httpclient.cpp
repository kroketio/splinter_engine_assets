#include <QNetworkReply>
#include <QJsonParseError>

#include "httpclient.h"

HttpClient::HttpClient(QObject* parent)
  : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &HttpClient::requestTimeout);
}

void HttpClient::getJSON(const QString& url) {
  busy = true;
  qDebug() << "fetching " << url;

  QNetworkRequest request(url);
  QNetworkReply* reply = m_networkManager->get(request);

  m_timer->setInterval(5000);
  m_timer->start();

  connect(reply, &QNetworkReply::finished, this, &HttpClient::handleJSONReply);
}

void HttpClient::requestTimeout() { // is this needed?
  if(m_reply != nullptr) {
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
  }

  busy = false;
  emit requestFailed("timeout error");
}

void HttpClient::handleJSONReply() {
  m_timer->stop();
  busy = false;

  auto *reply = qobject_cast<QNetworkReply*>(sender());
  if (reply && reply->error() == QNetworkReply::NoError) {
    QByteArray responseData = reply->readAll();
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData, &error);
    if (error.error == QJsonParseError::NoError) {
      emit requestComplete(jsonDocument);
    } else {
      emit requestFailed(error.errorString());
    }
  } else {
    // Handle error
    emit requestFailed(reply->errorString());
  }

  reply->deleteLater();
}


