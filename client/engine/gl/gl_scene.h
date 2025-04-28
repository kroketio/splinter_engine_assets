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
    void renderModels(bool pickingPass = false);

    void commitCameraInfo();
    void commitLightInfo();

  protected:
    void childEvent(QChildEvent *event) override;

  private:
    Scene* m_host;
    QVector<OpenGLMesh*> m_gizmoMeshes, m_gridlineMeshes, m_lightMeshes, m_normalMeshes;
    static OpenGLUniformBufferObject *m_cameraInfo, *m_lightInfo;

    private slots:
        // void gizmoAdded(AbstractGizmo* gizmo);
        void gridlineAdded(Gridline* gridline);
    void lightAdded(AbstractLight* light);
    // void modelAdded(Model* model);
    void meshAdded(Mesh* mesh);
    void hostDestroyed(QObject* host);
  };
}