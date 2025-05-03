#pragma once

#include <QObject>

#include "engine/core/common.h"

namespace engine {
#define CAMERA_INFO_BINDING_POINT 0
#define MODEL_INFO_BINDING_POINT 1
#define MATERIAL_INFO_BINDING_POINT 2
#define LIGHT_INFO_BINDING_POINT 3

  class OpenGLUniformBufferObject {

  public:
    OpenGLUniformBufferObject();
    ~OpenGLUniformBufferObject();

    GLuint bufferId() const;

    bool create();
    void allocate(int indx, const void *data, int count);
    void destroy();

    void bind();
    void write(int offset, const void* data, int count);
    void release();

    static void bindUniformBlock(QOpenGLShaderProgram * shader);
    static QOpenGLFunctions_3_3_Core* glFuncs;
  private:
    GLuint m_id;
  };
}