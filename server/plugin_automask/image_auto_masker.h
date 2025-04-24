#pragma once

#include <QDir>
#include <QObject>
#include <QProcess>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QTemporaryFile>

#include "lib/utils.h"

class ImageAutoMasker : public QObject {
Q_OBJECT
Q_PROPERTY(int loading MEMBER m_loading NOTIFY loadingChanged);

public:
  explicit ImageAutoMasker(QObject *parent = nullptr);
  bool detect(QString path_input);
  void convert(const QFileInfo &path);
  QString path_vmf;

public slots:
  void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

signals:
  void loadingChanged(bool status);
  void result(const QFileInfo &img);
  void error(const QString &msg);

private:
  QString m_application_path;
  QFileInfo m_file_output;
  QString cmd_interpreter = "/bin/bash";

  bool m_loading = false;
  QProcess *m_proc;
};
