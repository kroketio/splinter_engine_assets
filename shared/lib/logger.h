#pragma once

#include <QNetworkAccessManager>
#include <QCommandLineParser>
#include <QStorageInfo>
#include <QCoreApplication>
#include <algorithm>
#include <QFileInfo>
#include <iostream>
#include <QProcess>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <random>
#include <chrono>
#include <array>

using namespace std;

extern void *logger_ctx;
extern QFile *logFile;
extern QTextStream *logStream;

void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);