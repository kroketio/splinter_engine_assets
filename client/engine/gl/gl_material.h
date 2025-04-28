#pragma once

#include <QObject>

#include "engine/core/material.h"
#include "gl_uniform_buffer_object.h"
#include "engine/gl/gl_texture.h"

namespace engine {
  class OpenGLMaterial: public QObject {
    Q_OBJECT

public:
    OpenGLMaterial(Material* material, QObject* parent = 0);

    Material* host() const;

    void bind();
    void release();

  private:
    Material* m_host;
    OpenGLTexture* m_openGLDiffuseTexture, *m_openGLSpecularTexture, *m_openGLBumpTexture;
    static OpenGLUniformBufferObject *m_materialInfo;

    private slots:
        void diffuseTextureChanged(QSharedPointer<engine::Texture> diffuseTexture);
    void specularTextureChanged(QSharedPointer<engine::Texture> specularTexture);
    void bumpTextureChanged(QSharedPointer<engine::Texture> bumpTexture);
    void hostDestroyed(QObject* host);
  };
}