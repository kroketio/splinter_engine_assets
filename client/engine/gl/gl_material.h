#pragma once

#include <QObject>

#include "engine/core/material.h"
#include "engine/gl/gl_texture.h"
#include "gl_uniform_buffer_object.h"

namespace engine {
  class OpenGLMaterial;
  extern QMap<QString, OpenGLMaterial*> GLMaterialCache;
  class OpenGLMaterial : public QObject {
    Q_OBJECT

  public:
    OpenGLMaterial(const QSharedPointer<Material> &material, QObject *parent = 0);

    QSharedPointer<Material> host() const;

    void bind();
    void release();

    bool is_bound = false;

  private:
    QSharedPointer<Material> m_host;
    OpenGLTexture *m_openGLDiffuseTexture, *m_openGLSpecularTexture, *m_openGLBumpTexture;
    static OpenGLUniformBufferObject *m_materialInfo;

  private slots:
    void diffuseTextureChanged(QSharedPointer<engine::Texture> diffuseTexture);
    void specularTextureChanged(QSharedPointer<engine::Texture> specularTexture);
    void bumpTextureChanged(QSharedPointer<engine::Texture> bumpTexture);
    void hostDestroyed(QObject *host);
  };
} // namespace engine
