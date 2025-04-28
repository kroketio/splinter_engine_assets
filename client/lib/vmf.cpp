#include "vmf.h"

QVMF::QVMF(QString path_vmf, QObject *parent) : path_vmf(path_vmf), QObject(parent) {

}

bool QVMF::save(const std::filesystem::path& path) const {
  if (std::optional<vmfpp::VMF> root_node = vmfpp::VMF::openFile(path_vmf.toStdString()); root_node.has_value()) {
    return root_node->save(path.string());
  }

  return false;
}

void QVMF::parse() const {
  qDebug() << "vmf open" << path_vmf;

}
