#include "engine/core/scene.h"

namespace engine {
  Scene::Scene(): QObject(0), m_camera(0) {  //  m_gizmo(0)
    setObjectName("Untitled Scene");
    //    m_gizmo = new TransformGizmo(this);
    m_camera = new Camera(this);
    m_gridlineNameCounter = 1;
    m_ambientLightNameCounter = 1;
    m_directionalLightNameCounter = 1;
    m_pointLightNameCounter = 1;
    m_spotLightNameCounter = 1;
  }

  // Add & remove members

  Scene::Scene(const Scene & scene): QObject(0) {
    setObjectName(scene.objectName());

    //    m_gizmo = new TransformGizmo(this);
    m_camera = new Camera(*scene.m_camera);
    m_gridlineNameCounter = scene.m_gridlineNameCounter;
    m_ambientLightNameCounter = scene.m_ambientLightNameCounter;
    m_directionalLightNameCounter = scene.m_directionalLightNameCounter;
    m_pointLightNameCounter = scene.m_pointLightNameCounter;
    m_spotLightNameCounter = scene.m_spotLightNameCounter;

    for (int i = 0; i < scene.m_gridlines.size(); i++)
      addGridline(new Gridline(*scene.m_gridlines[i]));
    for (int i = 0; i < scene.m_ambientLights.size(); i++)
      addAmbientLight(new AmbientLight(*scene.m_ambientLights[i]));
    for (int i = 0; i < scene.m_directionalLights.size(); i++)
      addDirectionalLight(new DirectionalLight(*scene.m_directionalLights[i]));
    for (int i = 0; i < scene.m_pointLights.size(); i++)
      addPointLight(new PointLight(*scene.m_pointLights[i]));
    for (int i = 0; i < scene.m_spotLights.size(); i++)
      addSpotLight(new SpotLight(*scene.m_spotLights[i]));
    //    for (int i = 0; i < scene.m_models.size(); i++)
    //        addModel(new Model(*scene.m_models[i]));
  }

  bool Scene::loadVMF(const std::filesystem::path& path) {
    path_vmf = path;
    qDebug() << "vmf open" << path.string();

    vmf = vmfpp::VMF::openFile(path.string());
    if (!vmf.has_value()) {
      qWarning() << "vmf open failed; faulty vmf?";
      return false;
    }

    vmfpp::Node* node_ptr = reinterpret_cast<vmfpp::Node*>(&*vmf);
    for (const auto &node: node_ptr->getChildren()) {
      if (node.first != "world") continue;
      if (node.second.size() <= 0) continue;

      auto solids = node.second[0].getChild("solid");
      for (const auto& solid: solids) {
        auto bla_solid = solid_struct{};
        bla_solid.id = stoi(solid.getValue("id")[0]);
        auto sides = solid.getChild("side");

        for (const auto& side: sides) {
          auto bla_side = side_struct{};

          auto vaxis = side.getValue("uaxis")[0];
          auto uaxis = side.getValue("vaxis")[0];
          bla_side.rotation = stoi(side.getValue("rotation")[0]);
          bla_side.id = stoi(side.getValue("id")[0]);
          bla_side.material = side.getValue("material")[0];
          // auto plane = side.getValue("plane")[0];

          auto side_childs = side.getChildren();
          for (const auto& side_child: side_childs) {
            if (side_child.first == "vertices_plus") {
              auto vert = side_child.second[0].getValue("v");

              bla_side.vertices_plus = [vert] {
                std::array<QVector3D, 4> arr;
                for (size_t i = 0; i < 4; ++i) {
                  float x = 0.0f, y = 0.0f, z = 0.0f;
                  const char* cstr = vert[i].c_str();
                  char* end;
                  x = std::strtof(cstr, &end);
                  cstr = end;
                  y = std::strtof(cstr, &end);
                  cstr = end;
                  z = std::strtof(cstr, &end);
                  arr[i] = QVector3D(x, y, z);
                  qDebug() << arr[i];
                }
                return arr;
              }();

              // break;
            }

            // break;
          }

          bla_solid.sides.emplace_back(bla_side);


          auto mesh = engine::JustAQuad::from(bla_side);

          // engine::Material* material = new engine::Material;
          // aiColor4D color; float value; aiString aiStr;
          // material->setObjectName("Untitled");
          // material->setDiffuse(100.0f);
          // material->setColor(QVector3D(100.0f, 100.0f, 100.0f) / material->diffuse());
          // material->setDiffuse(100.0f);
          // mesh->setMaterial(material);

          QSharedPointer<Material> mat;
          if (CACHE_MATERIALS.contains("wow")) {
            mat = CACHE_MATERIALS["wow"];
          } else {
            mat = QSharedPointer<Material>(new Material(QVector3D(1.0f, 1.0f, 1.0f), 0, 0.6, 0, this));
            const auto tex = QSharedPointer<engine::Texture>(new Texture(Texture::Diffuse));
            mat->setObjectName("Untitled");
            QImage img("/home/dsc/texturefun/blenderkit/blenderkit_1k/RedBrick21670_1K_Color.png");
            tex->setImage(img);
            mat->setDiffuseTexture(tex);
            CACHE_MATERIALS["wow"] = mat;
          }

          mesh->setMaterial(mat);
          this->meshAdded(mesh);
        }
      }
    }

    return true;
  }

  Scene::~Scene() {

    qDebug() << "Scene" << this->objectName() << "is destroyed";
  }

  bool Scene::addMesh(Mesh* mesh) {
    m_meshes.push_back(mesh);
    mesh->setParent(this);
    mesh->setObjectName("randommesh");
    emit meshAdded(mesh);
  }

  bool Scene::setCamera(Camera * camera) {
    if (m_camera == camera) return false;

    if (m_camera) {
      Camera* tmp = m_camera;
      m_camera = 0;
      delete tmp;
    }

    if (camera) {
      m_camera = camera;
      m_camera->setParent(this);

      qDebug() << "Camera" << camera->objectName() << "is assigned to scene" << this->objectName();
    }

    cameraChanged(m_camera);
    return true;
  }

  bool Scene::addGridline(Gridline * gridline) {
    if (!gridline || m_gridlines.contains(gridline))
      return false;

    m_gridlines.push_back(gridline);
    gridline->setParent(this);
    gridline->setObjectName("Gridline" + QString::number(m_gridlineNameCounter++));
    gridlineAdded(gridline);


    qDebug() << "Gridline" << gridline->objectName() << "is added to scene" << this->objectName();

    return true;
  }

  bool Scene::addLight(AbstractLight * l) {
    if (SpotLight* light = qobject_cast<SpotLight*>(l))
      return addSpotLight(light);
    else if (AmbientLight* light = qobject_cast<AmbientLight*>(l))
      return addAmbientLight(light);
    else if (DirectionalLight* light = qobject_cast<DirectionalLight*>(l))
      return addDirectionalLight(light);
    else if (PointLight* light = qobject_cast<PointLight*>(l))
      return addPointLight(light);
    return false;
  }

  bool Scene::addAmbientLight(AmbientLight * light) {
    if (!light || m_ambientLights.contains(light))
      return false;
    if (m_ambientLights.size() >= 8) {

      qDebug() << "The amount of ambient lights has reached the upper limit of 8.";
      return false;
    }

    m_ambientLights.push_back(light);
    light->setParent(this);
    light->setObjectName("AmbientLight" + QString::number(m_ambientLightNameCounter++));
    lightAdded(light);


    qDebug() << "Ambient light" << light->objectName() << "is added to scene" << this->objectName();

    return true;
  }

  bool Scene::addDirectionalLight(DirectionalLight * light) {
    if (!light || m_directionalLights.contains(light))
      return false;
    if (m_directionalLights.size() >= 8) {

      qDebug() << "The amount of directional lights has reached the upper limit of 8.";
      return false;
    }

    m_directionalLights.push_back(light);
    light->setParent(this);
    light->setObjectName("DirectionalLight" + QString::number(m_directionalLightNameCounter++));
    lightAdded(light);


    qDebug() << "Directional light" << light->objectName() << "is added to scene" << this->objectName();

    return true;
  }

  bool Scene::addPointLight(PointLight * light) {
    if (!light || m_pointLights.contains(light))
      return false;
    if (m_pointLights.size() >= 8) {

      qDebug() << "The amount of point lights has reached the upper limit of 8.";
      return false;
    }

    m_pointLights.push_back(light);
    light->setParent(this);
    light->setObjectName("PointLight" + QString::number(m_pointLightNameCounter++));
    lightAdded(light);


    qDebug() << "Point light" << light->objectName() << "is added to scene" << this->objectName();

    return true;
  }

  bool Scene::addSpotLight(SpotLight * light) {
    if (!light || m_spotLights.contains(light))
      return false;
    if (m_spotLights.size() >= 8) {

      qDebug() << "The amount of spotlights has reached the upper limit of 8.";
      return false;
    }

    m_spotLights.push_back(light);
    light->setParent(this);
    light->setObjectName("SpotLight" + QString::number(m_spotLightNameCounter++));
    lightAdded(light);


    qDebug() << "Spot light" << light->objectName() << "is added to scene" << this->objectName();

    return true;
  }

  //bool Scene::addModel(Model * model) {
  //    if (!model || m_models.contains(model))
  //        return false;
  //
  //    m_models.push_back(model);
  //    model->setParent(this);
  //    modelAdded(model);
  //
  //
  //        qDebug() << "Model" << model->objectName() << "is added to scene" << this->objectName();
  //
  //    return true;
  //}

  bool Scene::removeGridline(QObject * gridline) {
    for (int i = 0; i < m_gridlines.size(); i++)
      if (m_gridlines[i] == gridline) {
        m_gridlines.erase(m_gridlines.begin() + i);
        gridlineRemoved(gridline);

        qDebug() << "Gridline" << gridline->objectName() << "is removed from scene" << this->objectName();
        return true;
      }
    return false;
  }

  bool Scene::removeLight(QObject * light) {
    for (int i = 0; i < m_ambientLights.size(); i++)
      if (m_ambientLights[i] == light) {
        m_ambientLights.erase(m_ambientLights.begin() + i);
        lightRemoved(light);

        qDebug() << "Ambient light" << light->objectName() << "is removed from scene" << this->objectName();
        return true;
      }
    for (int i = 0; i < m_directionalLights.size(); i++)
      if (m_directionalLights[i] == light) {
        m_directionalLights.erase(m_directionalLights.begin() + i);
        lightRemoved(light);

        qDebug() << "Directional light" << light->objectName() << "is removed from scene" << this->objectName();
        return true;
      }
    for (int i = 0; i < m_pointLights.size(); i++)
      if (m_pointLights[i] == light) {
        m_pointLights.erase(m_pointLights.begin() + i);
        lightRemoved(light);

        qDebug() << "Point light" << light->objectName() << "is removed from scene" << this->objectName();
        return true;
      }
    for (int i = 0; i < m_spotLights.size(); i++)
      if (m_spotLights[i] == light) {
        m_spotLights.erase(m_spotLights.begin() + i);
        lightRemoved(light);

        qDebug() << "Spot light" << light->objectName() << "is removed from scene" << this->objectName();
        return true;
      }
    return false;
  }

  bool Scene::removeModel(QObject * model, bool recursive) {
    return false;
    //    for (int i = 0; i < m_models.size(); i++)
    //        if (m_models[i] == model) {
    //            m_models.erase(m_models.begin() + i);
    //            modelRemoved(model);
    //
    //                qDebug() << "Model" << model->objectName() << "is removed from scene" << this->objectName();
    //            return true;
    //        }
    //    if (!recursive) return false;
    //    for (int i = 0; i < m_models.size(); i++)
    //        if (m_models[i]->removeChildModel(model, recursive))
    //            return true;
    //    return false;
  }

  // Dump info

  void Scene::dumpObjectInfo(int l) {
    qDebug().nospace() << tab(l) << "Scene: " << objectName();
    qDebug("%s%lld gridline(s), %lld ambient light(s), %lld directional light(s), %lld point light(s), %lld spotlights(s), %d model(s)",
           tab(l),
           m_gridlines.size(),
           m_ambientLights.size(),
           m_directionalLights.size(),
           m_pointLights.size(),
           m_spotLights.size(),
           0);
    //m_models.size());
  }

  void Scene::dumpObjectTree(int l) {
    dumpObjectInfo(l);
    //    m_gizmo->dumpObjectTree(l + 1);
    m_camera->dumpObjectTree(l + 1);
    for (int i = 0; i < m_gridlines.size(); i++)
      m_gridlines[i]->dumpObjectTree(l + 1);
    for (int i = 0; i < m_ambientLights.size(); i++)
      m_ambientLights[i]->dumpObjectTree(l + 1);
    for (int i = 0; i < m_directionalLights.size(); i++)
      m_directionalLights[i]->dumpObjectTree(l + 1);
    for (int i = 0; i < m_pointLights.size(); i++)
      m_pointLights[i]->dumpObjectTree(l + 1);
    for (int i = 0; i < m_spotLights.size(); i++)
      m_spotLights[i]->dumpObjectTree(l + 1);
    //    for (int i = 0; i < m_models.size(); i++)
    //        m_models[i]->dumpObjectTree(l + 1);
  }

  // Get properties

  //TransformGizmo * Scene::transformGizmo() const {
  //    return m_gizmo;
  //}

  Camera * Scene::camera() const {
    return m_camera;
  }

  const QVector<Gridline*>& Scene::gridlines() const {
    return m_gridlines;
  }

  const QVector<AmbientLight*>& Scene::ambientLights() const {
    return m_ambientLights;
  }

  const QVector<DirectionalLight*>& Scene::directionalLights() const {
    return m_directionalLights;
  }

  const QVector<PointLight*>& Scene::pointLights() const {
    return m_pointLights;
  }

  const QVector<SpotLight*>& Scene::spotLights() const {
    return m_spotLights;
  }

  //const QVector<Model*>& Scene::models() const {
  //    return m_models;
  //}

  void Scene::childEvent(QChildEvent * e) {
    if (e->added()) {
      if (Camera* camera = qobject_cast<Camera*>(e->child()))
        setCamera(camera);
      else if (Gridline* gridline = qobject_cast<Gridline*>(e->child()))
        addGridline(gridline);
      else if (AbstractLight* light = qobject_cast<AbstractLight*>(e->child()))
        addLight(light);
      //        else if (Model* model = qobject_cast<Model*>(e->child()))
      //            addModel(model);
    } else if (e->removed()) {
      if (m_camera == e->child()) {
        m_camera = 0;

        qDebug() << "Warning: Camera is removed from scene" << this->objectName();
        cameraChanged(0);
        return;
      }
      if (removeGridline(e->child())) return;
      if (removeLight(e->child())) return;
      if (removeModel(e->child(), false)) return;
    }
  }
}