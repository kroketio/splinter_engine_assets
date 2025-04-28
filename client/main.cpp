#include <fstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "lib/vmf.h"

#include "engine/gl/gl_window.h"

int main() {
  auto m_openGLWindow = new engine::OpenGLWindow;
  m_openGLWindow->setRenderer(new engine::OpenGLRenderer);

  auto vmf = new QVMF("test.vmf");
  vmf->parse();

  // load into engine

  bool res = vmf->save("/tmp/test.vmf");

  return 0;
}