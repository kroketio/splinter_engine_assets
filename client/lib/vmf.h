#pragma once

#include <QObject>
#include <QDebug>

#include "lib/vmfpp/vmfpp.h"

class QVMF : public QObject {
Q_OBJECT

public:
  explicit QVMF(QString dir, QObject *parent = nullptr);

  QString path_vmf;
  std::optional<vmfpp::VMF> vmf;

  bool open(const std::filesystem::path& path);
  bool save(const std::filesystem::path& path) const;
};
