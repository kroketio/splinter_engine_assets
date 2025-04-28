#pragma once

#include <QObject>

#include "engine/core/texture.h"

namespace engine {
  class OpenGLTexture: public QObject {
    Q_OBJECT

public:
    OpenGLTexture(engine::Texture* texture);
    ~OpenGLTexture();

    void create();
    bool bind();
    void release();

  private:
    engine::Texture* m_host;
    QOpenGLTexture *m_openGLTexture;

    private slots:
        void imageChanged(const QImage& image);
    void hostDestroyed(QObject* host);
  };
}