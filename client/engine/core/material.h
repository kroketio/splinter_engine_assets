#pragma once

#include <QObject>

#include "texture.h"

namespace engine {
  class Material;
  extern QMap<QString, QSharedPointer<Material>> CACHE_MATERIALS;

  class Material: public QObject {
  Q_OBJECT

  public:
    Material(QObject * parent = 0);
    Material(QVector3D color, float ambient, float diffuse, float specular, QObject * parent = 0);
    Material(const Material& material);
    ~Material();

    void dumpObjectInfo(int level = 0);
    void dumpObjectTree(int level = 0);

    QVector3D color() const;
    float ambient();
    float diffuse();
    float specular();
    float shininess();
    QSharedPointer<engine::Texture> diffuseTexture();
    QSharedPointer<engine::Texture> specularTexture();
    QSharedPointer<engine::Texture> bumpTexture();

  public slots:
    void setColor(QVector3D color);
    void setAmbient(float ambient);
    void setDiffuse(float diffuse);
    void setSpecular(float specular);
    void setShininess(float shininess);
    void setDiffuseTexture(QSharedPointer<engine::Texture> diffuseTexture);
    void setSpecularTexture(QSharedPointer<engine::Texture> specularTexture);
    void setBumpTexture(QSharedPointer<engine::Texture> bumpTexture);

  signals:
    void colorChanged(QVector3D color);
    void ambientChanged(float ambient);
    void diffuseChanged(float diffuse);
    void specularChanged(float specular);
    void shininessChanged(float shininess);
    void diffuseTextureChanged(QSharedPointer<engine::Texture> diffuseTexture);
    void specularTextureChanged(QSharedPointer<engine::Texture> specularTexture);
    void bumpTextureChanged(QSharedPointer<engine::Texture> bumpTexture);

  protected:
    QVector3D m_color;
    float m_ambient, m_diffuse, m_specular, m_shininess;
    float m_shininess_default = 264.0f;
    QSharedPointer<engine::Texture> m_diffuseTexture, m_specularTexture, m_bumpTexture;
  };
}