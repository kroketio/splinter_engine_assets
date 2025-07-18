#include "gl_mesh.h"
#include "gl_material.h"
#include "gl_uniform_buffer_object.h"

namespace engine {

  OpenGLMesh::OpenGLMesh(Mesh * mesh, QObject* parent): QObject(0) {
    m_host = mesh;
    m_sizeFixed = false;
    m_pickingID = 0;
    m_vao = 0;
    m_vbo = 0;
    m_ebo = 0;
    if (m_host->material()) {
      auto mat = m_host->material();
      auto mat_name = mat->objectName();
      if (GLMaterialCache.contains(mat_name))
        m_openGLMaterial = GLMaterialCache[mat_name];
      else {
        m_openGLMaterial = new OpenGLMaterial(mat);
        GLMaterialCache[mat_name] = m_openGLMaterial;
      }
    }
    else
      m_openGLMaterial = 0;

    // connect(m_host, SIGNAL(materialChanged(Material*)), this, SLOT(materialChanged(Material*)));
    connect(m_host, &Mesh::materialChanged, this, &OpenGLMesh::materialChanged);

    connect(m_host, SIGNAL(geometryChanged(QVector<Vertex>, QVector<uint32_t>)), this, SLOT(geometryChanged(QVector<Vertex>, QVector<uint32_t>)));
    connect(m_host, SIGNAL(destroyed(QObject*)), this, SLOT(hostDestroyed(QObject*)));

    setParent(parent);
  }

  OpenGLMesh::~OpenGLMesh() {
    this->destroy();
  }

  Mesh * OpenGLMesh::host() const {
    return m_host;
  }

  void OpenGLMesh::create() {
    this->destroy();

    m_vao = new QOpenGLVertexArrayObject;
    m_vao->create();
    m_vao->bind();
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vbo->create();
    m_vbo->bind();
    if (m_host->vertices().size())
      m_vbo->allocate(&m_host->vertices()[0], int(sizeof(Vertex) * m_host->vertices().size()));
    m_ebo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_ebo->create();
    m_ebo->bind();
    if (m_host->indices().size())
      m_ebo->allocate(&m_host->indices()[0], int(sizeof(uint32_t) * m_host->indices().size()));

    glFuncs = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
    glFuncs->glEnableVertexAttribArray(0);
    glFuncs->glEnableVertexAttribArray(1);
    glFuncs->glEnableVertexAttribArray(2);
    glFuncs->glEnableVertexAttribArray(3);
    glFuncs->glEnableVertexAttribArray(4);
    glFuncs->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glFuncs->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
    glFuncs->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tangent));
    glFuncs->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, bitangent));
    glFuncs->glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords));

    m_vao->release();
  }

  void OpenGLMesh::updateModelInfo() {
    QMatrix4x4 modelMat = m_host->globalModelMatrix();
    memcpy(shaderModelInfo.modelMat, modelMat.constData(), 64);
    memcpy(shaderModelInfo.normalMat, QMatrix4x4(modelMat.normalMatrix()).constData(), 64);
    shaderModelInfo.sizeFixed = this->m_sizeFixed;
    shaderModelInfo.selected = m_host->selected();
    shaderModelInfo.highlighted = 0;
    shaderModelInfo.pickingID = this->m_pickingID;

    if (m_modelInfo == nullptr) {
      m_modelInfo = new OpenGLUniformBufferObject;
      m_modelInfo->create();
      m_modelInfo->bind();
      m_modelInfo->allocate(MODEL_INFO_BINDING_POINT, nullptr, sizeof(ShaderModelInfo));
      m_modelInfo->release();
    }

    m_modelInfo->bind();
    m_modelInfo->write(0, &shaderModelInfo, sizeof(ShaderModelInfo));
    m_modelInfo->release();
  }

  void OpenGLMesh::updateMaterialInfo() {
    shaderModelInfo.highlighted = m_host->highlighted();

    if (m_modelInfo) {
      m_modelInfo->bind();
      const int offset = offsetof(ShaderModelInfo, highlighted);
      m_modelInfo->write(offset, &shaderModelInfo.highlighted, sizeof(shaderModelInfo.highlighted));
      m_modelInfo->release();
    }
  }

  void OpenGLMesh::commit() {
    updateModelInfo();      // Static data like transforms, selection, etc.
    updateMaterialInfo();   // Dynamic per-draw-call material state
    // QMatrix4x4 modelMat = m_host->globalModelMatrix();
    // memcpy(shaderModelInfo.modelMat, modelMat.constData(), 64);
    // memcpy(shaderModelInfo.normalMat, QMatrix4x4(modelMat.normalMatrix()).constData(), 64);
    // shaderModelInfo.sizeFixed = this->m_sizeFixed;
    // shaderModelInfo.selected = m_host->selected();
    // shaderModelInfo.highlighted = m_host->highlighted();
    // shaderModelInfo.pickingID = this->m_pickingID;
    //
    // if (m_host->highlighted())
    //   int weogw = 1;
    //
    // if (m_modelInfo == 0) {
    //   m_modelInfo = new OpenGLUniformBufferObject;
    //   m_modelInfo->create();
    //   m_modelInfo->bind();
    //   m_modelInfo->allocate(MODEL_INFO_BINDING_POINT, NULL, sizeof(ShaderModelInfo));
    //   m_modelInfo->release();
    // }
    // m_modelInfo->bind();
    // m_modelInfo->write(0, &shaderModelInfo, sizeof(ShaderModelInfo));
    // m_modelInfo->release();
  }

  void OpenGLMesh::render(bool updateHighlight) {
    if (!m_host->is_visible)
      return;

    if (m_vao == 0 || m_vbo == 0 || m_ebo == 0) {
      create(); // Initialize buffers if needed
    }

    if (!m_host->is_committed) {
      commit();
      m_host->is_committed = true;
      return;
    }

    if (updateHighlight) {
      updateMaterialInfo();
    }

    m_vao->bind();

    GLenum mode = GL_TRIANGLES;
    switch (m_host->meshType()) {
      case Mesh::Line:  mode = GL_LINES; break;
      case Mesh::Point: mode = GL_POINTS; break;
      default:          break;
    }

    glFuncs->glDrawElements(mode, static_cast<GLsizei>(m_host->indices().size()), GL_UNSIGNED_INT, nullptr);

    m_vao->release();
  }

  // void OpenGLMesh::render(bool pickingPass) {
  //   if (!m_host->is_visible) return;
  //
  //   if (m_vao == 0 || m_vbo == 0 || m_ebo == 0)
  //     create();
  //
  //   // Avoid calling commit() during rendering
  //   if (!m_host->is_committed) {
  //     commit();
  //     m_host->is_committed = true;
  //     return;  // Skip this frame
  //   }
  //
  //   static bool lastWireframe = false;
  //   bool wireframe = !pickingPass && m_host->wireFrameMode();
  //
  //   // Only update polygon mode if it has changed
  //   if (wireframe != lastWireframe) {
  //     glFuncs->glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
  //     lastWireframe = wireframe;
  //   }
  //
  //   // Only bind material if it's valid and needed
  //   static OpenGLMaterial* lastBoundMaterial = nullptr;
  //   if (!wireframe && m_openGLMaterial && m_openGLMaterial != lastBoundMaterial) {
  //     m_openGLMaterial->bind();
  //     lastBoundMaterial = m_openGLMaterial;
  //   }
  //
  //   m_vao->bind();
  //
  //   GLenum mode = GL_TRIANGLES;
  //   switch (m_host->meshType()) {
  //     case Mesh::Line:    mode = GL_LINES; break;
  //     case Mesh::Point:   mode = GL_POINTS; break;
  //     default:            break; // default to GL_TRIANGLES
  //   }
  //
  //   glFuncs->glDrawElements(mode, static_cast<GLsizei>(m_host->indices().size()), GL_UNSIGNED_INT, nullptr);
  //
  //   m_vao->release();
  //
  //   // Release material only if not wireframe and something was bound
  //   if (!wireframe && lastBoundMaterial) {
  //     lastBoundMaterial->release();
  //     lastBoundMaterial = nullptr;
  //   }
  // }

  void OpenGLMesh::destroy() {
    if (m_vao) delete m_vao;
    if (m_vbo) delete m_vbo;
    if (m_ebo) delete m_ebo;
    m_vao = 0;
    m_vbo = 0;
    m_ebo = 0;
  }

  void OpenGLMesh::setSizeFixed(bool sizeFixed) {
    m_sizeFixed = sizeFixed;
  }

  void OpenGLMesh::setPickingID(uint id) {
    m_pickingID = id;
  }

  void OpenGLMesh::childEvent(QChildEvent * e) {
    if (e->removed()) {
      if (e->child() == m_openGLMaterial)
        m_openGLMaterial = 0;
    }
  }

  void OpenGLMesh::materialChanged(const QSharedPointer<Material> &material) {
    if (material == nullptr || material.isNull())
      m_openGLMaterial = 0;
    else
      m_openGLMaterial = new OpenGLMaterial(material);
  }

  void OpenGLMesh::geometryChanged(const QVector<Vertex>&, const QVector<uint32_t>&) {
    this->create();
  }

  void OpenGLMesh::hostDestroyed(QObject *) {
    // Commit suicide
    delete this;
  }
}