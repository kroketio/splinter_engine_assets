#include <QApplication>
#include <QResource>
#include <QtCore>
#include <QSslSocket>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include <unistd.h>
#include <sys/types.h>
#include "mainwindow.h"
#include "lib/logger.h"
#include "ctx.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(engine);

  qDebug() << "qt" << qVersion();
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setApplicationName("hello");
  QApplication::setOrganizationDomain("kroket.io");
  QApplication::setOrganizationName("Kroket Ltd.");
  QApplication app(argc, argv);
  //qInstallMessageHandler(customMessageHandler);

  qDebug() << "SSL version: " << QSslSocket::sslLibraryVersionString();
  qDebug() << "SSL build: " << QSslSocket::sslLibraryBuildVersionString();

  // QDirIterator it(":", QDirIterator::Subdirectories);
  // while (it.hasNext()) {
  //   auto zz = QString(it.next());
  //   if(!zz.startsWith(":/qt-project.org") && !zz.startsWith(":/HelloModAA")) {
  //     qDebug() << zz;
  //   }
  // }

  auto *ctx = new AppContext();
  ctx->isDebug = false;
  auto *mainWindow = new MainWindow(ctx);

  return QApplication::exec();
}
