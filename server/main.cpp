#include <QCoreApplication>
#include <QResource>
#include <QDirIterator>
#include <QBuffer>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "png.h"

#include <unistd.h>
#include <sys/types.h>
#include "lib/logger.h"
#include "ctx.h"

#include <QCoreApplication>

#include "VTFLib.h"
#include "VTFFormat.h"

#include "lib/vtf/common/image.hpp"
#include "lib/vtf/common/enums.hpp"
#include "lib/vtf/common/util.hpp"
#include "lib/vtf/common/vtftools.hpp"

using namespace VTFLib;

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
