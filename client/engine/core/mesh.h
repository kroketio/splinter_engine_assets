#pragma once

#include "abstract_entity.h"
#include "engine/core/material.h"
#include "vertex.h"

#include "assimp/mesh.h"

namespace engine {
  class ModelLoader;

  class Mesh : public AbstractEntity {
    Q_OBJECT

  public:
    enum MeshType { Triangle = 0, Line = 1, Point = 2 };

    Mesh(QObject *parent = 0);
    Mesh(MeshType meshType, QObject *parent = 0);
    Mesh(const Mesh &mesh);
    static Mesh *loadMesh(const aiMesh *aiMeshPtr);
    ~Mesh();

    bool is_visible = false;
    bool is_committed = false;

    void setBoundingSphere();

    void dumpObjectInfo(int level = 0) override;
    void dumpObjectTree(int level = 0) override;

    bool isGizmo() const override;
    bool isLight() const override;
    bool isMesh() const override;
    bool isModel() const override;

    QVector3D centerOfMass() const;
    float mass() const;

    MeshType meshType() const;
    const QVector<Vertex> &vertices() const;
    const QVector<uint32_t> &indices() const;
    QSharedPointer<Material> material() const;

    unsigned short m_indices_size;
    QPair<QVector3D, float> cached_bounding_shere;

    static Mesh *merge(const Mesh *mesh1, const Mesh *mesh2);

  public slots:
    void setMeshType(MeshType meshType);
    void setGeometry(const QVector<Vertex> &vertices, const QVector<uint32_t> &indices);
    bool setMaterial(const QSharedPointer<Material>& newMaterial);
    void reverseNormals();
    void reverseTangents();
    void reverseBitangents();

  signals:
    void meshTypeChanged(int meshType);
    void geometryChanged(const QVector<Vertex> &vertices, const QVector<uint32_t> &indices);
    void materialChanged(const QSharedPointer<Material> &material);

  // protected:
  //   void childEvent(QChildEvent *event) override;

  protected:
    MeshType m_meshType;
    QVector<Vertex> m_vertices;
    QVector<uint32_t> m_indices;
    QSharedPointer<Material> m_material;

    friend ModelLoader;
  };

  QDataStream &operator>>(QDataStream &in, Mesh::MeshType &meshType);

  struct side_struct {
    int id = -1;
    int rotation = 0;
    std::array<float, 5> uaxis = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::array<float, 5> vaxis = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::string material;
    std::array<QVector3D, 4> vertices_plus;
  };

  struct solid_struct {
    int id = -1;
    std::vector<side_struct> sides;
  };
} // namespace engine
