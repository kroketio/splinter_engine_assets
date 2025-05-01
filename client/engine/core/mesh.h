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

  class JustAQuad : public Mesh {
    Q_OBJECT
  public:
    static JustAQuad* from(const side_struct& side
    ) {
      auto quad = new JustAQuad;

      QVector3D edge1 = side.vertices_plus[1] - side.vertices_plus[0];
      QVector3D edge2 = side.vertices_plus[2] - side.vertices_plus[0];
      // QVector3D normal = QVector3D::normal(edge1, edge2);
      QVector3D normal{0.0f,  1.0f,  0.0f};
      QVector3D tangent{0.0f,  0.0f,  1.0f};
      QVector3D bitangent{0.0f,  0.0f,  1.0f};
      //QVector3D tangent = (side.vertices_plus[1] - side.vertices_plus[0]).normalized(); // X direction
      //QVector3D bitangent = QVector3D::crossProduct(normal, tangent).normalized(); // Y direction

      std::vector<QVector2D> coords = {
        QVector2D(0.0f,  0.0f),
        QVector2D(1.0f,  0.0f),
        QVector2D(1.0f,  1.0f),
        QVector2D(0.0f,  1.0f)
      };

      int i = 0;
      for (const auto &vert : side.vertices_plus) {
        int e = 1;
        quad->m_vertices.append(Vertex{
          vert,
          normal, // normal
          tangent, // tangent
          bitangent, // bitangent
          coords[i]
        });
        i += 1;
      }

      quad->m_indices << 0 << 1 << 2 << 0 << 2 << 3;

      return quad;

      // if (vertices_plus.size() > 0) {
      //   int weogpwe = 1;
      //
      //   auto mesh = new engine::Mesh;
      //   // Compute normal from the plane (using right-hand rule)
      //   QVector3D edge1 = vertices_plus[1] - vertices_plus[0];
      //   QVector3D edge2 = vertices_plus[2] - vertices_plus[0];
      //   QVector3D normal = QVector3D::normal(edge1, edge2);
      //
      //   // Compute tangent and bitangent (arbitrary basis, planar)
      //   QVector3D tangent = (vertices_plus[1] - vertices_plus[0]).normalized(); // X direction
      //   QVector3D bitangent = QVector3D::crossProduct(normal, tangent).normalized(); // Y direction
      //
      //
      // }

      // QVector3D v0(704, -256, 0);
      // QVector3D v1(768, -256, 0);
      // QVector3D v2(768, -832, 0);
      // QVector3D v3(704, -832, 0);
    }

    explicit JustAQuad(QObject *parent = 0) {
      // m_vertices = {{
      //                   QVector3D(-0.5f, 0.0f, 0.5f), // position
      //                   QVector3D(0.0f, 1.0f, 0.0f), // normal
      //                   QVector3D(1.0f, 0.0f, 0.0f), // tangent
      //                   QVector3D(0.0f, 0.0f, 1.0f), // bitangent
      //                   QVector2D(0.0f, 0.0f) // texCoords
      //               },
      //               {QVector3D(0.5f, 0.0f, 0.5f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f),
      //                QVector3D(0.0f, 0.0f, 1.0f), QVector2D(1.0f, 0.0f)},
      //               {QVector3D(0.5f, 0.0f, -0.5f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f),
      //                QVector3D(0.0f, 0.0f, 1.0f), QVector2D(1.0f, 1.0f)},
      //               {QVector3D(-0.5f, 0.0f, -0.5f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f),
      //                QVector3D(0.0f, 0.0f, 1.0f), QVector2D(0.0f, 1.0f)}};
      //
      // QVector3D v0(704, -256, 0);
      // QVector3D v1(768, -256, 0);
      // QVector3D v2(768, -832, 0);
      // QVector3D v3(704, -832, 0);
      //
      //
      // m_vertices.append(Vertex{
      //     v0,//QVector3D(-0.5f, 0.0f,  0.5f), // position
      //     QVector3D(0.0f,  1.0f,  0.0f), // normal
      //     QVector3D(1.0f,  0.0f,  0.0f), // tangent
      //     QVector3D(0.0f,  0.0f,  1.0f), // bitangent
      //     QVector2D(0.0f,  0.0f)         // texCoords
      // });
      //
      // m_vertices.append(Vertex{
      //     v1,//QVector3D( 0.5f, 0.0f,  0.5f),
      //     QVector3D(0.0f,  1.0f,  0.0f),
      //     QVector3D(1.0f,  0.0f,  0.0f),
      //     QVector3D(0.0f,  0.0f,  1.0f),
      //     QVector2D(1.0f,  0.0f)
      // });
      //
      // m_vertices.append(Vertex{
      //     v2,//QVector3D( 0.5f, 0.0f, -0.5f),
      //     QVector3D(0.0f,  1.0f,  0.0f),
      //     QVector3D(1.0f,  0.0f,  0.0f),
      //     QVector3D(0.0f,  0.0f,  1.0f),
      //     QVector2D(1.0f,  1.0f)
      // });
      //
      // m_vertices.append(Vertex{
      //     v3,//QVector3D(-0.5f, 0.0f, -0.5f),
      //     QVector3D(0.0f,  1.0f,  0.0f),
      //     QVector3D(1.0f,  0.0f,  0.0f),
      //     QVector3D(0.0f,  0.0f,  1.0f),
      //     QVector2D(0.0f,  1.0f)
      // });

      // m_indices << 0 << 1 << 2 << 0 << 2 << 3;
    }

    void dumpObjectInfo(int level = 0) {};
    void dumpObjectTree(int level = 0) {};

    bool isGizmo() const { return true; };
    bool isLight() const { return false; };
    bool isMesh() const { return true; };
    bool isModel() const { return false; };
  };
} // namespace engine
