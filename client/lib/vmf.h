#pragma once

#include <QObject>
#include <QDebug>

#include "lib/vmfpp/vmfpp.h"

class QVMF : public QObject {
Q_OBJECT

public:
  explicit QVMF(QString dir, QObject *parent = nullptr);

  QString path_vmf;

  void parse() const;
  bool QVMF::save(const std::filesystem::path& path) const;
};
