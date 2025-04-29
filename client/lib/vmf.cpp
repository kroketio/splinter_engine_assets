#include "vmf.h"

QVMF::QVMF(QString path_vmf, QObject *parent) : path_vmf(path_vmf), QObject(parent) {

}

bool QVMF::save(const std::filesystem::path& path) const {
  if (std::optional<vmfpp::VMF> root_node = vmfpp::VMF::openFile(path_vmf.toStdString()); root_node.has_value()) {
    return root_node->save(path.string());
  }
  qWarning() << "invalid root node";
  return false;
}

bool QVMF::open(const std::filesystem::path& path) {
  qDebug() << "vmf open" << path_vmf;
  bool xa = vmf.has_value();

  vmf = vmfpp::VMF::openFile(path.string());
  if (vmf.has_value()) {
    return true;
  }

  return false;
}
