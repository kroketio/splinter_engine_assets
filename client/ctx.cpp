#include <filesystem>

#include <QObject>
#include <QDir>
#include <QStandardPaths>

#include "ctx.h"
#include "lib/utils.h"

using namespace std::chrono;


AppContext::AppContext() {
  configRoot = QDir::homePath();
  homeDir = QDir::homePath();
}

AppContext::~AppContext() {}
