#pragma once

#include <QObject>

#include "engine/core/scene.h"
#include "gl_mesh.h"
#include "gl_uniform_buffer_object.h"

namespace engine {
  class OpenGLScene: public QObject {
    Q_OBJECT

public:
    OpenGLScene(Scene* scene);
    Scene* host() const;

    OpenGLMesh* pick(uint32_t pickingID);

    void renderAxis();
    void renderGridlines();
    void renderLights();
    bool isMeshInFrustum(const Mesh *mesh, const QVector<QVector4D> &planes, const QMatrix4x4 &modelMatrix);
    void renderModels(bool pickingPass = false);
    void updateFrustumPlanes(const QMatrix4x4 &vp);

    void commitCameraInfo();
    void commitLightInfo();

    QVector<QVector4D> m_frustumPlanes;

  protected:
    void childEvent(QChildEvent *event) override;

  private:
    Scene* m_host;
    QVector<OpenGLMesh*> m_gizmoMeshes, m_gridlineMeshes, m_lightMeshes, m_normalMeshes;
    static OpenGLUniformBufferObject *m_cameraInfo, *m_lightInfo;

  public slots:
    void meshAdded(Mesh* mesh);

  private slots:
    // void gizmoAdded(AbstractGizmo* gizmo);
    void gridlineAdded(Gridline* gridline);
    void lightAdded(AbstractLight* light);
    // void modelAdded(Model* model);
    void hostDestroyed(QObject* host);
  };
}