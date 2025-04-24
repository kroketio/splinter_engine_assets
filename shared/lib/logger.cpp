#include <QObject>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QCommandLineParser>
#include <QStandardPaths>

#include "logger.h"

void *logger_ctx = nullptr;
QFile *logFile = nullptr;
QTextStream *logStream = nullptr;

void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QString logMessage;
    switch (type) {
    case QtDebugMsg:
        logMessage = "\033[90m[D]\033[0m ";
        break;
    case QtInfoMsg:
        logMessage = "[I] ";
        break;
    case QtWarningMsg:
        logMessage = "[W] ";
        break;
    case QtCriticalMsg:
        logMessage = "[C] ";
        break;
    default:
        logMessage = "[U] ";
        break;
    }

    auto fn = QString(context.file);
    auto fnspl = fn.split("/");

    int seconds = QDateTime::currentDateTime().time().second();
    logMessage += QString::number(seconds) + " ";
    logMessage += QString("\033[90m%4\t%1:%2\033[0m\n    %3").arg(fnspl.last()).arg(context.line).arg(msg).arg(context.function);

    // to console
    QTextStream stream(type != QtInfoMsg ? stderr : stdout);
    stream << logMessage << "\n";

    // to file
    if(logFile != nullptr && logFile->isOpen()) {
      if(logStream == nullptr)
        logStream = new QTextStream(logFile);
      *logStream << logMessage << '\n';
    }

    // to app
    // if(logger_ctx != nullptr)
    //   logger_ctx->applicationLog(logMessage);
}
