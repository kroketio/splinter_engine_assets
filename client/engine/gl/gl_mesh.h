#pragma once

#include <QObject>
#include <QOpenGLVersionFunctionsFactory>

#include "engine/core/mesh.h"
#include "engine/core/vertex.h"
#include "gl_material.h"

namespace engine {
  struct ShaderModelInfo {
    float modelMat[16];     // 64 bytes @ offset 0
    float normalMat[16];    // 64 bytes @ offset 64
    int sizeFixed;          // 4 bytes  @ offset 128
    int selected;           // 4 bytes  @ offset 132
    int highlighted;        // 4 bytes  @ offset 136
    unsigned int pickingID; // 4 bytes  @ offset 140
    int padding[4];         // 16 bytes @ offset 144 — brings total to 160
  };

  class OpenGLMesh: public QObject {
    Q_OBJECT

public:
    OpenGLMesh(Mesh* mesh, QObject* parent = 0);
    ~OpenGLMesh();

    Mesh* host() const;

    void create();
    void updateModelInfo();
    void updateMaterialInfo();
    void commit();
    void render(bool updateHighlight = false);
    void destroy();

    void setSizeFixed(bool sizeFixed);
    void setPickingID(uint id);

    OpenGLMaterial *m_openGLMaterial;

  protected:
    void childEvent(QChildEvent *event) override;

  private:
    Mesh* m_host;
    bool m_sizeFixed;
    uint m_pickingID;

    QOpenGLVertexArrayObject * m_vao;
    QOpenGLBuffer * m_vbo, *m_ebo;
    QOpenGLFunctions_3_3_Core * glFuncs;

    ShaderModelInfo shaderModelInfo;
    OpenGLUniformBufferObject *m_modelInfo = 0;

    private slots:
      void materialChanged(const QSharedPointer<Material> &material);
      void geometryChanged(const QVector<Vertex>& vertices, const QVector<uint32_t>& indices);
      void hostDestroyed(QObject* host);
  };
}