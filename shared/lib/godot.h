#pragma once

#include "models/texture.h"
#include "models/asset_pack.h"

// class Godot : public QObject {
// Q_OBJECT
//
// public:
//   explicit WebServer(QObject *parent = nullptr);
//   ~WebServer() override = default;
//
// private:
//   QHttpServer* m_server;
//   QTcpServer* m_tcp_server;
//   int m_tcp_port = 19200;
// };

// struct ee {
//   QSharedPointer<TextureImage> img;
//   QMap<QString, QString> options;
// };

namespace godot {
  extern QMap<QString, QMap<QString, QString>> resourceTemplates;
  extern QMap<QString, QString> nameToResourceTemplateLookup;

  void initGodotResourceTemplates();
  QString genID(int idx, const QString &filename);
  QString generateMaterialTres(QSharedPointer<Texture> tex, TextureSize tsize);

  QMap<QString, QString> parseResource(const QString& inp);
  QString camelToSnake(const QString &camel);
  QString removeNumbers(QString input);
}