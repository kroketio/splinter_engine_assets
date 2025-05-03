#pragma once

#include <QObject>

#include "engine/core/mesh.h"

namespace engine{
  class JustAQuad : public Mesh {
    Q_OBJECT
  public:
    static JustAQuad* from(const side_struct& side) {
      auto* quad = new JustAQuad();
      quad->is_visible = true;

      // Extract u and v axis vectors
      QVector3D uvec(side.uaxis[0], side.uaxis[1], side.uaxis[2]);
      QVector3D vvec(side.vaxis[0], side.vaxis[1], side.vaxis[2]);
      float u_offset = side.uaxis[3];
      float v_offset = side.vaxis[3];
      float u_scale = side.uaxis[4];
      float v_scale = side.vaxis[4];

      if (u_scale < 0.25)
        u_scale = u_scale / 0.25;
      if (v_scale < 0.25)
        v_scale = v_scale / 0.25;

      // u_scale = 0.05;

      // Add horizontal offset of 24 units in U direction
      float horizontal_uv_offset = 0.0f;

      // Compute normal (assuming 4 vertices are co-planar and in correct winding)
      QVector3D normal = QVector3D::crossProduct(
          side.vertices_plus[1] - side.vertices_plus[0],
          side.vertices_plus[2] - side.vertices_plus[0]
      ).normalized();

      QVector3D tangent = uvec.normalized();
      QVector3D bitangent = vvec.normalized();

      for (int i = 0; i < 4; ++i) {
        const QVector3D& pos = side.vertices_plus[i];

        // Raw UVs (no normalization), with 24-unit U offset
        float u = (QVector3D::dotProduct(pos, uvec) + u_offset + horizontal_uv_offset) * u_scale;
        float v = (QVector3D::dotProduct(pos, vvec) + v_offset) * v_scale;

        // Rotate UVs 90° counter-clockwise: (u, v) → (1 - v, u)
        float rotated_u = -v;  // (1 - v) is only valid in normalized space; just rotate raw instead
        float rotated_v = u;

        quad->m_vertices.append(Vertex(pos, normal, tangent, bitangent, QVector2D(rotated_u, rotated_v)));
      }

      // Triangle indices for quad: 0-1-2 and 0-2-3
      quad->m_indices.append(0);
      quad->m_indices.append(1);
      quad->m_indices.append(2);
      quad->m_indices.append(0);
      quad->m_indices.append(2);
      quad->m_indices.append(3);

      quad->m_indices_size = quad->m_indices.size();
      quad->setBoundingSphere();

      return quad;
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
}

// // QVector3D edge1 = side.vertices_plus[1] - side.vertices_plus[0];
// // QVector3D edge2 = side.vertices_plus[2] - side.vertices_plus[0];
// // QVector3D normal = QVector3D::normal(edge1, edge2);
// //QVector3D normal{0.0f,  1.0f,  0.0f};
// //QVector3D normal{0.0f,  1.0f,  0.0f};
//
// QVector3D edge1 = side.vertices_plus[1] - side.vertices_plus[0];
// QVector3D edge2 = side.vertices_plus[2] - side.vertices_plus[0];
// QVector3D normal = QVector3D::crossProduct(edge1, edge2).normalized();
//
// QVector3D tangent{side.uaxis[0], side.uaxis[1], side.uaxis[2]};
// QVector3D bitangent{side.uaxis[0], side.uaxis[1], side.uaxis[2]};
// //QVector3D tangent = (side.vertices_plus[1] - side.vertices_plus[0]).normalized(); // X direction
// //QVector3D bitangent = QVector3D::crossProduct(normal, tangent).normalized(); // Y direction
//
// std::vector<QVector2D> coords = {
//   QVector2D(0.0f,  0.0f),
//   QVector2D(1.0f,  0.0f),
//   QVector2D(1.0f,  1.0f),
//   QVector2D(0.0f,  1.0f)
// };
// float uaxis_scale = 0.25f;
// float vaxis_scale = 0.25f;
//
// int i = 0;
// for (const auto &vert : side.vertices_plus) {
//   float u = QVector3D::dotProduct(vert, QVector3D(side.uaxis[0],side.uaxis[1],side.uaxis[2])) + side.uaxis[3];
//   float v = QVector3D::dotProduct(vert, QVector3D(side.vaxis[0],side.vaxis[1],side.vaxis[2])) + side.vaxis[3];
//   QVector2D uv(u, v);
//
//   quad->m_vertices.append(Vertex{
//     vert,
//     normal, // normal
//     tangent, // tangent
//     bitangent, // bitangent
//     uv
//   });
//   i += 1;
// }