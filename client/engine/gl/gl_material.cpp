#include "gl_material.h"
#include "engine/gl/gl_texture.h"

namespace engine {
  QMap<QString, OpenGLMaterial*> GLMaterialCache = {};
  struct ShaderMaterialInfo {
    QVector4D color;      // 16          // 0
    float ambient;        // 4           // 16
    float diffuse;        // 4           // 20
    float specular;       // 4           // 24
    float shininess;      // 4           // 28
    int useDiffuseMap;    // 4           // 32
    int useSpecularMap;   // 4           // 36
    int useBumpMap;       // 4           // 40
    int padding;          // 4           // 44
  };

  static ShaderMaterialInfo shaderMaterialInfo;


  OpenGLUniformBufferObject *OpenGLMaterial::m_materialInfo = 0;

  OpenGLMaterial::OpenGLMaterial(const QSharedPointer<Material> &material, QObject* parent): QObject(0) {
    m_host = material;

    this->diffuseTextureChanged(m_host->diffuseTexture());
    this->specularTextureChanged(m_host->specularTexture());
    this->bumpTextureChanged(m_host->bumpTexture());

    connect(m_host.data(), &Material::diffuseTextureChanged, this, &OpenGLMaterial::diffuseTextureChanged);
    connect(m_host.data(), &Material::specularTextureChanged, this, &OpenGLMaterial::specularTextureChanged);
    connect(m_host.data(), &Material::bumpTextureChanged, this, &OpenGLMaterial::bumpTextureChanged);
    connect(m_host.data(), &Material::destroyed, this, &OpenGLMaterial::hostDestroyed);

    setObjectName(m_host->objectName());
    setParent(parent);
  }

  QSharedPointer<Material> OpenGLMaterial::host() const {
    return m_host;
  }

  void OpenGLMaterial::bind() {
    shaderMaterialInfo.useDiffuseMap = false;
    shaderMaterialInfo.useSpecularMap = false;
    shaderMaterialInfo.useBumpMap = false;

    if (m_openGLDiffuseTexture)
      shaderMaterialInfo.useDiffuseMap = m_openGLDiffuseTexture->bind();
    if (m_openGLSpecularTexture)
      shaderMaterialInfo.useSpecularMap = m_openGLSpecularTexture->bind();
    if (m_openGLBumpTexture)
      shaderMaterialInfo.useBumpMap = m_openGLBumpTexture->bind();

    shaderMaterialInfo.color = m_host->color().toVector4D();
    shaderMaterialInfo.ambient = m_host->ambient();
    shaderMaterialInfo.diffuse = m_host->diffuse();
    shaderMaterialInfo.specular = m_host->specular();
    shaderMaterialInfo.shininess = m_host->shininess();

    if (m_materialInfo == 0) {
      m_materialInfo = new OpenGLUniformBufferObject;
      m_materialInfo->create();
      m_materialInfo->bind();
      m_materialInfo->allocate(MATERIAL_INFO_BINDING_POINT, NULL, sizeof(ShaderMaterialInfo));
      m_materialInfo->release();
    }
    m_materialInfo->bind();
    m_materialInfo->write(0, &shaderMaterialInfo, sizeof(ShaderMaterialInfo));
    m_materialInfo->release();
    is_bound = true;
  }

  void OpenGLMaterial::release() {
    if (m_openGLDiffuseTexture) m_openGLDiffuseTexture->release();
    if (m_openGLSpecularTexture) m_openGLSpecularTexture->release();
    if (m_openGLBumpTexture) m_openGLBumpTexture->release();

    shaderMaterialInfo.color = QVector4D(0, 0, 0, 0);
    shaderMaterialInfo.useDiffuseMap = 0;
    shaderMaterialInfo.useSpecularMap = 0;
    shaderMaterialInfo.useBumpMap = 0;

    if (m_materialInfo == 0) {
      m_materialInfo = new OpenGLUniformBufferObject;
      m_materialInfo->create();
      m_materialInfo->bind();
      m_materialInfo->allocate(3, NULL, sizeof(ShaderMaterialInfo));
      m_materialInfo->release();
    }
    m_materialInfo->bind();
    m_materialInfo->write(0, &shaderMaterialInfo, sizeof(ShaderMaterialInfo));
    m_materialInfo->release();
  }

  void OpenGLMaterial::diffuseTextureChanged(QSharedPointer<engine::Texture> diffuseTexture) {
    if (diffuseTexture.isNull())
      m_openGLDiffuseTexture = 0;
    else if (diffuseTexture->property("OpenGLTexturePointer").isValid())
      m_openGLDiffuseTexture = diffuseTexture->property("OpenGLTexturePointer").value<OpenGLTexture*>();
    else
      m_openGLDiffuseTexture = new OpenGLTexture(diffuseTexture.data());
  }

  void OpenGLMaterial::specularTextureChanged(QSharedPointer<engine::Texture> specularTexture) {
    if (specularTexture.isNull())
      m_openGLSpecularTexture = 0;
    else if (specularTexture->property("OpenGLTexturePointer").isValid())
      m_openGLSpecularTexture = specularTexture->property("OpenGLTexturePointer").value<OpenGLTexture*>();
    else
      m_openGLSpecularTexture = new OpenGLTexture(specularTexture.data());
  }

  void OpenGLMaterial::bumpTextureChanged(QSharedPointer<engine::Texture> bumpTexture) {
    if (bumpTexture.isNull())
      m_openGLBumpTexture = 0;
    else if (bumpTexture->property("OpenGLTexturePointer").isValid())
      m_openGLBumpTexture = bumpTexture->property("OpenGLTexturePointer").value<OpenGLTexture*>();
    else
      m_openGLBumpTexture = new OpenGLTexture(bumpTexture.data());
  }

  void OpenGLMaterial::hostDestroyed(QObject *) {
    // Commit suicide
    delete this;
  }
}