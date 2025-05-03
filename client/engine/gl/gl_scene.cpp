#include "gl_scene.h"

namespace engine {
  struct ShaderAxisInfo { // struct size: 64
    //                         // base align  // aligned offset
    float hostModelMat[16];    // 64          // 0
  } shaderAxisInfo;

  struct ShaderCameraInfo { // struct size: 144
    //                         // base align  // aligned offset
    float projMat[16];         // 64          // 0
    float viewMat[16];         // 64          // 64
    QVector4D cameraPos;       // 16          // 128
  } shaderCameraInfo;

  struct ShaderAmbientLight { // struct size: 16
    //                         // base align  // aligned offset
    QVector4D color;           // 16          // 0
  };

  struct ShaderDirectionalLight { // struct size: 32
    //                         // base align  // aligned offset
    QVector4D color;           // 16          // 0
    QVector4D direction;       // 16          // 16
  };

  struct ShaderPointLight { // struct size: 48
    //                           // base align  // aligned offset
    QVector4D color;             // 16          // 0
    QVector4D pos;               // 16          // 16
    QVector4D attenuation;       // 16          // 32
  };

  struct ShaderSpotLight { // struct size: 80
    //                           // base align  // aligned offset
    QVector4D color;             // 16          // 0
    QVector4D pos;               // 16          // 16
    QVector4D direction;         // 16          // 32
    QVector4D attenuation;       // 16          // 48
    QVector4D cutOff;            // 16          // 64
  };

  struct ShaderlightInfo { // struct size: 1424
    //                                          // base align  // aligned offset
    int ambientLightNum;                        // 4           // 0
    int directionalLightNum;                    // 4           // 4
    int pointLightNum;                          // 4           // 8
    int spotLightNum;                           // 4           // 12
    ShaderAmbientLight ambientLight[8];         // 16          // 16
    ShaderDirectionalLight directionalLight[8]; // 32          // 144
    ShaderPointLight pointLight[8];             // 48          // 400
    ShaderSpotLight spotLight[8];               // 80          // 784
  };

  static ShaderlightInfo shaderlightInfo;

  OpenGLUniformBufferObject *OpenGLScene::m_cameraInfo = 0;
  OpenGLUniformBufferObject *OpenGLScene::m_lightInfo = 0;

  OpenGLScene::OpenGLScene(Scene * scene) {
    m_host = scene;

    // this->gizmoAdded(m_host->transformGizmo());
    for (int i = 0; i < m_host->gridlines().size(); i++)
      this->gridlineAdded(m_host->gridlines()[i]);
    for (int i = 0; i < m_host->pointLights().size(); i++)
      this->lightAdded(m_host->pointLights()[i]);
    for (int i = 0; i < m_host->spotLights().size(); i++)
      this->lightAdded(m_host->spotLights()[i]);
    for (int i = 0; i < m_host->ambientLights().size(); i++)
      this->lightAdded(m_host->ambientLights()[i]);
    // for (int i = 0; i < m_host->models().size(); i++)
    //     this->modelAdded(m_host->models()[i]);

    connect(m_host, SIGNAL(gridlineAdded(Gridline*)), this, SLOT(gridlineAdded(Gridline*)));
    connect(m_host, SIGNAL(lightAdded(AbstractLight*)), this, SLOT(lightAdded(AbstractLight*)));
    // connect(m_host, SIGNAL(modelAdded(Model*)), this, SLOT(modelAdded(Model*)));
    connect(m_host, SIGNAL(meshAdded(Mesh*)), this, SLOT(meshAdded(Mesh*)));
    connect(m_host, SIGNAL(destroyed(QObject*)), this, SLOT(hostDestroyed(QObject*)));
  }

  Scene * OpenGLScene::host() const {
    return m_host;
  }

  OpenGLMesh * OpenGLScene::pick(uint32_t pickingID) {
    if (pickingID >= 1000 && pickingID - 1000 < (uint32_t) m_normalMeshes.size())
      return m_normalMeshes[pickingID - 1000];
    else if (pickingID >= 100 && pickingID - 100 < (uint32_t) m_lightMeshes.size())
      return m_lightMeshes[pickingID - 100];
    else if (pickingID >= 90 && pickingID - 90 < (uint32_t) m_gizmoMeshes.size())
      return m_gizmoMeshes[pickingID - 90];
    return 0;
  }

  void OpenGLScene::renderAxis() {
    // if (m_host->transformGizmo()->alwaysOnTop())
    //     glClear(GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < m_gizmoMeshes.size(); i++)
      m_gizmoMeshes[i]->render();
  }

  void OpenGLScene::renderGridlines() {
    for (int i = 0; i < m_gridlineMeshes.size(); i++)
      m_gridlineMeshes[i]->render();
  }

  void OpenGLScene::renderLights() {
    for (int i = 0; i < m_lightMeshes.size(); i++) {
      m_lightMeshes[i]->setPickingID(100 + i);
      m_lightMeshes[i]->render();
    }
  }

  // for (auto* mesh : m_sceneMeshes) {
  //   QMatrix4x4 modelMatrix = mesh->globalModelMatrix();
  //
  //   if (!isMeshInFrustum(mesh, m_frustumPlanes, modelMatrix))
  //     continue; // Skip this mesh — it's outside the frustum
  //
  //   mesh->render(); // Or your actual drawing function
  // }

  bool OpenGLScene::isMeshInFrustum(const Mesh* mesh, const QVector<QVector4D>& planes, const QMatrix4x4& modelMatrix) {
    auto [center, radius] = mesh->cached_bounding_shere;

    const QVector3D worldCenter = modelMatrix * center;
    for (const auto& plane : planes) {
      float distance = QVector3D::dotProduct(plane.toVector3D(), worldCenter) + plane.w();
      if (distance < -radius)
        return false;
    }

    for (const auto& plane : planes) {
      float distance = QVector3D::dotProduct(plane.toVector3D(), worldCenter) + plane.w();

      qDebug() << "not culled:"
               << " Center:" << worldCenter
               << " Radius:" << radius
               << " Distance:" << distance;
    }

    return true;
  }

  void OpenGLScene::renderModels(bool pickingPass) {
    // auto start = std::chrono::high_resolution_clock::now();
    std::unordered_map<OpenGLMaterial*, std::vector<OpenGLMesh*>> meshBatches;

    for (auto* mesh : m_normalMeshes) {
      if (!mesh->host()->is_visible)
        continue;

      OpenGLMaterial* mat = mesh->m_openGLMaterial;
      meshBatches[mat].push_back(mesh);
    }

    OpenGLMaterial* lastBoundMaterial = nullptr;
    int i = 0;
    for (const auto& [material, meshList] : meshBatches) {
      // bind material once for the batch
      if (!pickingPass && material && material != lastBoundMaterial) {
        material->bind();
        lastBoundMaterial = material;
      }

      // render each mesh in this batch
      for (auto* mesh : meshList) {
        mesh->setPickingID(1000 + i);
        mesh->render(pickingPass);
        i += 1;
      }

      // Release material if used
      if (!pickingPass && material) {
        material->release();
      }
    }

    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> elapsed = end - start;
    // std::cout << "Function took " << elapsed.count() << " ms" << std::endl;
  }

  void OpenGLScene::updateFrustumPlanes(const QMatrix4x4& vp) {
    m_frustumPlanes.resize(6);

    // Right
    m_frustumPlanes[0] = QVector4D(
        vp(0,3) - vp(0,0),
        vp(1,3) - vp(1,0),
        vp(2,3) - vp(2,0),
        vp(3,3) - vp(3,0)
    );

    // Left
    m_frustumPlanes[1] = QVector4D(
        vp(0,3) + vp(0,0),
        vp(1,3) + vp(1,0),
        vp(2,3) + vp(2,0),
        vp(3,3) + vp(3,0)
    );

    // Bottom
    m_frustumPlanes[2] = QVector4D(
        vp(0,3) + vp(0,1),
        vp(1,3) + vp(1,1),
        vp(2,3) + vp(2,1),
        vp(3,3) + vp(3,1)
    );

    // Top
    m_frustumPlanes[3] = QVector4D(
        vp(0,3) - vp(0,1),
        vp(1,3) - vp(1,1),
        vp(2,3) - vp(2,1),
        vp(3,3) - vp(3,1)
    );

    // Far
    m_frustumPlanes[4] = QVector4D(
        vp(0,3) - vp(0,2),
        vp(1,3) - vp(1,2),
        vp(2,3) - vp(2,2),
        vp(3,3) - vp(3,2)
    );

    // Near
    m_frustumPlanes[5] = QVector4D(
        vp(0,3) + vp(0,2),
        vp(1,3) + vp(1,2),
        vp(2,3) + vp(2,2),
        vp(3,3) + vp(3,2)
    );

    // Normalize
    for (auto &plane : m_frustumPlanes) {
      // float len = QVector3D(plane.x(), plane.y(), plane.z()).length();
      // plane /= len;
      QVector3D normal = QVector3D(plane.x(), plane.y(), plane.z());
      float length = normal.length();
      if (length != 0.0f) plane /= length;
    }
  }

  void OpenGLScene::commitCameraInfo() {
    auto cam = m_host->camera();
    if (!cam) return;
    auto proj_matrix = cam->projectionMatrix();
    auto view_matrix = cam->viewMatrix();

    // QMatrix4x4 vp = proj_matrix * view_matrix;
    // QVector<QVector4D> frustumPlanes;
    // //updateFrustumPlanes(vp);

    memcpy(shaderCameraInfo.projMat, proj_matrix.constData(), 64);
    memcpy(shaderCameraInfo.viewMat, view_matrix.constData(), 64);
    shaderCameraInfo.cameraPos = cam->position().toVector4D();

    if (m_cameraInfo == 0) {
      m_cameraInfo = new OpenGLUniformBufferObject;
      m_cameraInfo->create();
      m_cameraInfo->bind();
      m_cameraInfo->allocate(CAMERA_INFO_BINDING_POINT, NULL, sizeof(ShaderCameraInfo));
      m_cameraInfo->release();
    }

    m_cameraInfo->bind();
    m_cameraInfo->write(0, &shaderCameraInfo, sizeof(ShaderCameraInfo));
    m_cameraInfo->release();
  }

  void OpenGLScene::commitLightInfo() {
    int ambientLightNum = 0, directionalLightNum = 0, pointLightNum = 0, spotLightNum = 0;
    for (int i = 0; i < m_host->ambientLights().size(); i++)
      if (m_host->ambientLights()[i]->enabled()) {
        shaderlightInfo.ambientLight[ambientLightNum].color = m_host->ambientLights()[i]->color().toVector4D() * m_host->ambientLights()[i]->intensity();
        ambientLightNum++;
      }
    for (int i = 0; i < m_host->directionalLights().size(); i++)
      if (m_host->directionalLights()[i]->enabled()) {
        shaderlightInfo.directionalLight[directionalLightNum].color = m_host->directionalLights()[i]->color().toVector4D() * m_host->directionalLights()[i]->intensity();
        shaderlightInfo.directionalLight[directionalLightNum].direction = m_host->directionalLights()[i]->direction().toVector4D();
        directionalLightNum++;
      }
    for (int i = 0; i < m_host->pointLights().size(); i++)
      if (m_host->pointLights()[i]->enabled()) {
        shaderlightInfo.pointLight[pointLightNum].color = m_host->pointLights()[i]->color().toVector4D() * m_host->pointLights()[i]->intensity();
        shaderlightInfo.pointLight[pointLightNum].pos = m_host->pointLights()[i]->position().toVector4D();
        shaderlightInfo.pointLight[pointLightNum].attenuation[0] = m_host->pointLights()[i]->enableAttenuation();
        shaderlightInfo.pointLight[pointLightNum].attenuation[1] = m_host->pointLights()[i]->attenuationQuadratic();
        shaderlightInfo.pointLight[pointLightNum].attenuation[2] = m_host->pointLights()[i]->attenuationLinear();
        shaderlightInfo.pointLight[pointLightNum].attenuation[3] = m_host->pointLights()[i]->attenuationConstant();
        pointLightNum++;
      }
    for (int i = 0; i < m_host->spotLights().size(); i++)
      if (m_host->spotLights()[i]->enabled()) {
        shaderlightInfo.spotLight[spotLightNum].color = m_host->spotLights()[i]->color().toVector4D() * m_host->spotLights()[i]->intensity();
        shaderlightInfo.spotLight[spotLightNum].pos = m_host->spotLights()[i]->position().toVector4D();
        shaderlightInfo.spotLight[spotLightNum].direction = m_host->spotLights()[i]->direction().toVector4D();
        shaderlightInfo.spotLight[spotLightNum].attenuation[0] = m_host->spotLights()[i]->enableAttenuation();
        shaderlightInfo.spotLight[spotLightNum].attenuation[1] = m_host->spotLights()[i]->attenuationQuadratic();
        shaderlightInfo.spotLight[spotLightNum].attenuation[2] = m_host->spotLights()[i]->attenuationLinear();
        shaderlightInfo.spotLight[spotLightNum].attenuation[3] = m_host->spotLights()[i]->attenuationConstant();
        shaderlightInfo.spotLight[spotLightNum].cutOff[0] = (float) cos(rad(m_host->spotLights()[i]->innerCutOff()));
        shaderlightInfo.spotLight[spotLightNum].cutOff[1] = (float) cos(rad(m_host->spotLights()[i]->outerCutOff()));
        spotLightNum++;
      }

    shaderlightInfo.ambientLightNum = ambientLightNum;
    shaderlightInfo.directionalLightNum = directionalLightNum;
    shaderlightInfo.pointLightNum = pointLightNum;
    shaderlightInfo.spotLightNum = spotLightNum;

    if (m_lightInfo == 0) {
      m_lightInfo = new OpenGLUniformBufferObject;
      m_lightInfo->create();
      m_lightInfo->bind();
      m_lightInfo->allocate(LIGHT_INFO_BINDING_POINT, NULL, sizeof(ShaderlightInfo));
      m_lightInfo->release();
    }
    m_lightInfo->bind();
    m_lightInfo->write(0, &shaderlightInfo, sizeof(ShaderlightInfo));
    m_lightInfo->release();
  }

  void OpenGLScene::childEvent(QChildEvent * e) {
    if (e->removed()) {
      for (int i = 0; i < m_gridlineMeshes.size(); i++)
        if (m_gridlineMeshes[i] == e->child())
          m_gridlineMeshes.removeAt(i);
      for (int i = 0; i < m_lightMeshes.size(); i++)
        if (m_lightMeshes[i] == e->child())
          m_lightMeshes.removeAt(i);
      for (int i = 0; i < m_normalMeshes.size(); i++)
        if (m_normalMeshes[i] == e->child())
          m_normalMeshes.removeAt(i);
    }
  }

  // void OpenGLScene::gizmoAdded(AbstractGizmo* gizmo) {
  //     for (int i = 0; i < gizmo->markers().size(); i++) {
  //         m_gizmoMeshes.push_back(new OpenGLMesh(gizmo->markers()[i], this));
  //         m_gizmoMeshes.back()->setSizeFixed(true);
  //         m_gizmoMeshes.back()->setPickingID(90 + i);
  //     }
  // }

  void OpenGLScene::gridlineAdded(Gridline * gridline) {
    // @TODO: readd
    // m_gridlineMeshes.push_back(new OpenGLMesh(gridline->marker(), this));
  }

  void OpenGLScene::lightAdded(AbstractLight * light) {
    // TODO: readd
    int e = 1;
    // if (light->marker())
    //     m_lightMeshes.push_back(new OpenGLMesh(light->marker(), this));
  }

  // void OpenGLScene::modelAdded(Model * model) {
  //     connect(model, SIGNAL(childMeshAdded(Mesh*)), this, SLOT(meshAdded(Mesh*)));
  //     for (int i = 0; i < model->childMeshes().size(); i++)
  //         meshAdded(model->childMeshes()[i]);
  //     for (int i = 0; i < model->childModels().size(); i++)
  //         modelAdded(model->childModels()[i]);
  // }

  void OpenGLScene::meshAdded(Mesh* mesh) {
    // const auto mat = new Material(QVector3D(1.0f, 1.0f, 1.0f), 0, 0.6, 0, this);
    // const auto tex = QSharedPointer<engine::Texture>(new Texture(Texture::Diffuse));
    // QImage img("/home/dsc/texturefun/blenderkit/blenderkit_1k/RedBrick21670_1K_Color.png");
    // tex->setImage(img);
    // mat->setDiffuseTexture(tex);
    // mesh->setMaterial(mat);

    // int index = m_normalMeshes.size();
    // int maxColumns = 10, cellSize = 1;
    // int x = (index % maxColumns) * cellSize;
    // int y = (index / maxColumns) * cellSize;

    auto m = new OpenGLMesh(mesh, this);
    mesh->setPosition(QVector3D(0, 0, 0));
    m_normalMeshes.push_back(m);
  }

  void OpenGLScene::hostDestroyed(QObject *) {
    delete this;
  }
}