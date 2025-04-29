#pragma once

//#include "transform_gizmo.h"
#include "camera.h"
#include "gridline.h"
#include "ambient_light.h"
#include "directional_light.h"
#include "point_light.h"
#include "spot_light.h"
//#include <Model.h>

#include "lib/vmf.h"

namespace engine {
  class SceneLoader;
  class SceneSaver;

  class Scene: public QObject {
    Q_OBJECT

public:
    Scene();
    Scene(const Scene& scene);
    ~Scene();

    bool loadVMF(const std::filesystem::path& path);

    bool setCamera(Camera* camera);
    bool addGridline(Gridline* gridline);
    bool addLight(AbstractLight* light);
    bool addAmbientLight(AmbientLight* light);
    bool addDirectionalLight(DirectionalLight* light);
    bool addPointLight(PointLight* light);
    bool addSpotLight(SpotLight* light);
    bool addMesh(Mesh* mesh);
    // bool addModel(Model* model);

    bool removeGridline(QObject* gridline);
    bool removeLight(QObject* light);
    bool removeModel(QObject* model, bool recursive);

    void dumpObjectInfo(int level = 0);
    void dumpObjectTree(int level = 0);

    // TransformGizmo* transformGizmo() const;
    Camera* camera() const;
    const QVector<Gridline*>& gridlines() const;
    const QVector<AmbientLight*>& ambientLights() const;
    const QVector<DirectionalLight*>& directionalLights() const;
    const QVector<PointLight*>& pointLights() const;
    const QVector<SpotLight*>& spotLights() const;
    // const QVector<Model*>& models() const;

  signals:
    void cameraChanged(Camera* camera);
    void gridlineAdded(Gridline* gridline);
    void gridlineRemoved(QObject* object);
    void lightAdded(AbstractLight* light);
    void lightRemoved(QObject* object);
    // void modelAdded(Model* model);
    void meshAdded(Mesh* mesh);
    void modelRemoved(QObject* object);

  protected:
    void childEvent(QChildEvent *event) override;

  private:
    // TransformGizmo * m_gizmo;
    Camera * m_camera;
    QVector<Gridline*> m_gridlines;
    QVector<AmbientLight*> m_ambientLights;
    QVector<DirectionalLight*> m_directionalLights;
    QVector<PointLight*> m_pointLights;
    QVector<SpotLight*> m_spotLights;
    // QVector<Model*> m_models;
    QVector<Mesh*> m_meshes;

    std::filesystem::path path_vmf;
    std::optional<vmfpp::VMF> vmf;

    int m_gridlineNameCounter;
    int m_ambientLightNameCounter;
    int m_directionalLightNameCounter;
    int m_pointLightNameCounter;
    int m_spotLightNameCounter;

    friend SceneLoader;
    friend SceneSaver;
  };
}