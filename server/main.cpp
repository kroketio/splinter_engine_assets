#include <QCoreApplication>
#include <QResource>
#include <QDirIterator>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "png.h"

#include <unistd.h>
#include <sys/types.h>
#include "lib/logger.h"
#include "ctx.h"

#include <QCoreApplication>

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(assets);

  qDebug() << "qt" << qVersion();
  QCoreApplication::setApplicationName("asset_server");
  QCoreApplication::setOrganizationDomain("kroket.io");
  QCoreApplication::setOrganizationName("Kroket Ltd.");
  QCoreApplication app(argc, argv);

  qInstallMessageHandler(customMessageHandler);
  auto *ctx = new Ctx();
  logger_ctx = ctx;

  // list qrc:// files
  QDirIterator it(":", QDirIterator::Subdirectories);
  while (it.hasNext()) { qDebug() << it.next(); }

  ctx->isDebug = false;

  return QCoreApplication::exec();
}
