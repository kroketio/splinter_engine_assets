#include "image_auto_masker.h"

ImageAutoMasker::ImageAutoMasker(QObject *parent) : m_proc(new QProcess(this)), QObject(parent) {
  m_application_path = "/path/to/tools/masker/dist/masker";
  connect(m_proc, &QProcess::finished, this, &ImageAutoMasker::onFinished);
  int i = 1;
}

void ImageAutoMasker::convert(const QFileInfo &path) {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QTemporaryFile tempFile;
  if(!tempFile.open()) {
    qWarning() << "Failed to create temporary file.";
    return;
  }

  // tempFile()
  m_file_output = Utils::tempFile("png");

  QString tempOut = tempFile.fileName();
  auto args = QStringList({
    "/path/to/test.jpg",
    m_file_output.absoluteFilePath()
  });

  m_proc->setProcessEnvironment(env);
  m_proc->setProcessChannelMode(QProcess::MergedChannels);
  m_proc->start(m_application_path, args);
}

void ImageAutoMasker::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
  auto res = m_proc->readAll();
  QString err;

  if(exitCode == 0) {
    if(Utils::fileExists(m_file_output.absoluteFilePath())) {
      emit result(m_file_output);
    } else {
      err = QString("file did not exist: %1").arg(m_file_output.absoluteFilePath());
      emit error(err);
    }
  } else {
    err = QString("non-0 exit code: %1").arg(res);
  }

  m_proc->close();
}

bool ImageAutoMasker::detect(QString path_input) {
  // QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  // env.insert("PYTHONUNBUFFERED", "1");
  // m_file_output = QDir::tempPath() = "/tmp_imageautomasker.png";

  // auto args = QStringList() << "-c" << QString("source %1 && python3 %2 %3 %4").arg(
  //     "/path/to/venv/bin/activate",
  //     "/path/to/autoMasker/masker.py",
  //     path_input, m_file_output);

  // m_proc->setProcessEnvironment(env);
  // m_proc->setProcessChannelMode(QProcess::MergedChannels);
  // m_proc->start(this->cmd_interpreter, args);

  // int i = 0;
  return false;
}
