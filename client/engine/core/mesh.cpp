#include <QBuffer>

#include "engine/core/mesh.h"
//#include <AbstractGizmo.h>
#include "abstract_light.h"

namespace engine {
  Mesh::Mesh(QObject * parent): AbstractEntity(0) {
    m_meshType = Triangle;
    m_material = 0;
    setObjectName("Untitled Mesh");
    setParent(parent);
  }

  Mesh::Mesh(MeshType _meshType, QObject * parent): AbstractEntity(0) {
    m_meshType = _meshType;
    m_material = 0;
    setObjectName("Untitled Mesh");
    setParent(parent);
  }

  Mesh::Mesh(const Mesh & mesh): AbstractEntity(mesh) {
    m_meshType = mesh.m_meshType;
    m_vertices = mesh.m_vertices;
    m_indices = mesh.m_indices;
    m_material = QSharedPointer<Material>(new Material(*mesh.m_material));
    setObjectName(mesh.objectName());
  }

  Mesh::~Mesh() {

    qDebug() << "Mesh" << this->objectName() << "is destroyed";
  }

  // Dump info

  void Mesh::dumpObjectInfo(int l) {
    qDebug().nospace() << tab(l) << "Mesh: " << objectName();
    qDebug().nospace() << tab(l + 1) << "Type: " <<
        (m_meshType == Triangle ? "Triangle" : (m_meshType == Line ? "Line" : "Point"));
    qDebug().nospace() << tab(l + 1) << "Visible: " << m_visible;
    qDebug().nospace() << tab(l + 1) << "Position: " << m_position;
    qDebug().nospace() << tab(l + 1) << "Rotation: " << m_rotation;
    qDebug().nospace() << tab(l + 1) << "Scaling:  " << m_scaling;
    qDebug("%s%lld vertices, %lld indices, %d material",
           tab(l + 1), m_vertices.size(), m_indices.size(), m_material != 0);

    QVector3D position;
    QVector3D normal;
    QVector3D tangent;
    QVector3D bitangent;
    QVector2D texCoords;

    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    buffer.open(QIODevice::WriteOnly);
    QTextStream out(&buffer);

    for (int i = 0; i != m_vertices.size(); i++) {
      qDebug() << QString("[%1] vertices").arg(QString::number(i));
      qDebug() << "bitangent" << m_vertices[i].bitangent;
      qDebug() << "position" << m_vertices[i].position;
      qDebug() << "normal" << m_vertices[i].normal;
      qDebug() << "tangent" << m_vertices[i].tangent;
      qDebug() << "texCoords" << m_vertices[i].texCoords;
    }

    qDebug() << "=======================";

    for (int i = 0; i != m_indices.size(); i++) {
      qDebug() << QString("[%1] m_indices").arg(QString::number(i));
      qDebug() << m_indices[i];
    }

    buffer.close();

  }

  void Mesh::dumpObjectTree(int l) {
    dumpObjectInfo(l);
    if (m_material)
      m_material->dumpObjectTree(l + 1);
  }

  bool Mesh::isGizmo() const {
    //    if (qobject_cast<AbstractGizmo*>(parent())) return true;
    return false;
  }

  bool Mesh::isLight() const {
    if (qobject_cast<AbstractLight*>(parent())) return true;
    return false;
  }

  bool Mesh::isMesh() const {
    if (isGizmo()) return false;
    if (isLight()) return false;
    return true;
  }

  bool Mesh::isModel() const {
    return false;
  }

  QVector3D Mesh::centerOfMass() const {
    QVector3D centerOfMass;
    float totalMass = 0;
    QMatrix4x4 modelMat = globalModelMatrix();
    for (int i = 0; i < m_indices.size();) {
      QVector3D centroid;
      float mass = 0;
      if (m_meshType == Point) {
        centroid = modelMat * m_vertices[m_indices[i + 0]].position;
        mass = 1.0f;
        i += 1;
      } else if (m_meshType == Line) {
        QVector3D p0 = modelMat * m_vertices[m_indices[i + 0]].position;
        QVector3D p1 = modelMat * m_vertices[m_indices[i + 1]].position;
        centroid = (p0 + p1) / 2;
        mass = p0.distanceToPoint(p1);
        i += 2;
      } else if (m_meshType == Triangle) {
        QVector3D p0 = modelMat * m_vertices[m_indices[i + 0]].position;
        QVector3D p1 = modelMat * m_vertices[m_indices[i + 1]].position;
        QVector3D p2 = modelMat * m_vertices[m_indices[i + 2]].position;
        centroid = (p0 + p1 + p2) / 3;
        mass = QVector3D::crossProduct(p1 - p0, p2 - p0).length() / 2;
        i += 3;
      }
      centerOfMass += centroid * mass;
      totalMass += mass;
    }
    return centerOfMass / totalMass;
  }

  float Mesh::mass() const {
    float totalMass = 0;
    QMatrix4x4 modelMat = globalModelMatrix();
    for (int i = 0; i < m_indices.size();) {
      if (m_meshType == Point) {
        totalMass += 1.0f;
        i += 1;
      } else if (m_meshType == Line) {
        QVector3D p0 = modelMat * m_vertices[m_indices[i + 0]].position;
        QVector3D p1 = modelMat * m_vertices[m_indices[i + 1]].position;
        totalMass += p0.distanceToPoint(p1);
        i += 2;
      } else if (m_meshType == Triangle) {
        QVector3D p0 = modelMat * m_vertices[m_indices[i + 0]].position;
        QVector3D p1 = modelMat * m_vertices[m_indices[i + 1]].position;
        QVector3D p2 = modelMat * m_vertices[m_indices[i + 2]].position;
        totalMass += QVector3D::crossProduct(p1 - p0, p2 - p0).length() / 2;
        i += 3;
      }
    }
    return totalMass;
  }

  Mesh::MeshType Mesh::meshType() const {
    return m_meshType;
  }

  const QVector<Vertex>& Mesh::vertices() const {
    return m_vertices;
  }

  const QVector<uint32_t>& Mesh::indices() const {
    return m_indices;
  }

  QSharedPointer<Material> Mesh::material() const{
    return m_material;
  }

  void Mesh::setBoundingSphere() {
    QVector3D center(0, 0, 0);
    for (const auto& v : m_vertices)
      center += v.position;
    center /= m_vertices.size();

    float maxRadiusSq = 0.0f;
    for (const auto& v : m_vertices)
      maxRadiusSq = std::max(maxRadiusSq, (v.position - center).lengthSquared());

    cached_bounding_shere = qMakePair(center, std::sqrt(maxRadiusSq));
  }

  Mesh * Mesh::merge(const Mesh * mesh1, const Mesh * mesh2) {
    if (mesh1 == 0 && mesh2 == 0)
      return 0;
    else if (mesh1 == 0 || mesh2 == 0) {
      if (mesh1 == 0) mesh1 = mesh2;
      Mesh* mergedMesh = new Mesh(mesh1->meshType());
      mergedMesh->setObjectName(mesh1->objectName());
      mergedMesh->setMaterial(QSharedPointer<Material>(new Material(*mesh1->material())));
      for (int i = 0; i < mesh1->m_vertices.size(); i++)
        mergedMesh->m_vertices.push_back(mesh1->globalModelMatrix() * mesh1->m_vertices[i]);
      mergedMesh->m_indices = mesh1->m_indices;
      return mergedMesh;
    }

    if (mesh1->meshType() != mesh2->meshType()) {

      qDebug() << "Failed to merge" << mesh1->objectName() << "and" << mesh2->objectName() << ": type not match";
      return 0;
    }


    qDebug() << "Merging" << mesh1->objectName() << "and" << mesh2->objectName();

    Mesh* mergedMesh = new Mesh(mesh1->meshType());
    mergedMesh->setObjectName(mesh1->objectName() + mesh2->objectName());
    mergedMesh->setMaterial(QSharedPointer<Material>(new Material));

    for (int i = 0; i < mesh1->m_vertices.size(); i++)
      mergedMesh->m_vertices.push_back(mesh1->globalModelMatrix() * mesh1->m_vertices[i]);

    for (int i = 0; i < mesh2->m_vertices.size(); i++)
      mergedMesh->m_vertices.push_back(mesh2->globalModelMatrix() * mesh2->m_vertices[i]);

    mergedMesh->m_indices = mesh1->m_indices;
    for (int i = 0; i < mesh2->m_indices.size(); i++)
      mergedMesh->m_indices.push_back(mesh2->m_indices[i] + mesh1->m_vertices.size());

    return mergedMesh;
  }

  void Mesh::setMeshType(MeshType meshType) {
    if (m_meshType != meshType) {
      m_meshType = meshType;

      qDebug() << "The type of mesh" << this->objectName() << "is set to"
           << (m_meshType == Triangle ? "Triangle" : (m_meshType == Line ? "Line" : "Point"));
      meshTypeChanged(m_meshType);
    }
  }

  void Mesh::setGeometry(const QVector<Vertex>& vertices, const QVector<uint32_t>& indices) {
    if (m_vertices != vertices || m_indices != indices) {
      m_vertices = vertices;
      m_indices = indices;
      geometryChanged(m_vertices, m_indices);
    }
  }

  bool Mesh::setMaterial(const QSharedPointer<Material>& material) {
    if (m_material == material) return false;

    if (material) {
      m_material = material;
      qDebug() << material->objectName();
      m_material->setParent(this);
      qDebug() << "Material" << material->objectName() << "is assigned to mesh" << objectName();
    }

    materialChanged(m_material);
    return true;
  }

  void Mesh::reverseNormals() {
    for (int i = 0; i < m_vertices.size(); i++)
      m_vertices[i].normal = -m_vertices[i].normal;

    qDebug() << "Normals of" << this->objectName() << "is reversed";
    geometryChanged(m_vertices, m_indices);
  }

  void Mesh::reverseTangents() {
    for (int i = 0; i < m_vertices.size(); i++)
      m_vertices[i].tangent = -m_vertices[i].tangent;

    qDebug() << "Tangents of" << this->objectName() << "is reversed";
    geometryChanged(m_vertices, m_indices);
  }

  void Mesh::reverseBitangents() {
    for (int i = 0; i < m_vertices.size(); i++)
      m_vertices[i].bitangent = -m_vertices[i].bitangent;

    qDebug() << "Bitangents of" << this->objectName() << "is reversed";
    geometryChanged(m_vertices, m_indices);
  }

  engine::Mesh* Mesh::loadMesh(const aiMesh* aiMeshPtr) {
    auto* mesh = new engine::Mesh;
    mesh->setObjectName(aiMeshPtr->mName.length ? aiMeshPtr->mName.C_Str() : "Untitled");

    for (uint32_t i = 0; i < aiMeshPtr->mNumVertices; i++) {
      engine::Vertex vertex;
      if (aiMeshPtr->HasPositions())
        vertex.position = QVector3D(aiMeshPtr->mVertices[i].x, aiMeshPtr->mVertices[i].y, aiMeshPtr->mVertices[i].z);
      if (aiMeshPtr->HasNormals())
        vertex.normal = QVector3D(aiMeshPtr->mNormals[i].x, aiMeshPtr->mNormals[i].y, aiMeshPtr->mNormals[i].z);
      if (aiMeshPtr->HasTangentsAndBitangents()) {
        // Use left-handed tangent space
        vertex.tangent = QVector3D(aiMeshPtr->mTangents[i].x, aiMeshPtr->mTangents[i].y, aiMeshPtr->mTangents[i].z);
        vertex.bitangent = QVector3D(aiMeshPtr->mBitangents[i].x, aiMeshPtr->mBitangents[i].y, aiMeshPtr->mBitangents[i].z);

        // Gram-Schmidt process, re-orthogonalize the TBN vectors
        vertex.tangent -= QVector3D::dotProduct(vertex.tangent, vertex.normal) * vertex.normal;
        vertex.tangent.normalize();

        // Deal with mirrored texture coordinates
        if (QVector3D::dotProduct(QVector3D::crossProduct(vertex.tangent, vertex.normal), vertex.bitangent) < 0.0f)
          vertex.tangent = -vertex.tangent;
      }
      if (aiMeshPtr->HasTextureCoords(0))
        vertex.texCoords = QVector2D(aiMeshPtr->mTextureCoords[0][i].x, aiMeshPtr->mTextureCoords[0][i].y);
      mesh->m_vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < aiMeshPtr->mNumFaces; i++)
      for (uint32_t j = 0; j < 3; j++)
        mesh->m_indices.push_back(aiMeshPtr->mFaces[i].mIndices[j]);

    QVector3D center = mesh->centerOfMass();

    for (int i = 0; i < mesh->m_vertices.size(); i++)
      mesh->m_vertices[i].position -= center;

    mesh->m_position = center;
    return mesh;
  }

  // Material * ModelLoader::loadMaterial(const aiMaterial * aiMaterialPtr) {
  //   Material* material = new Material;
  //   aiColor4D color; float value; aiString aiStr;
  //
  //   if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_NAME, aiStr))
  //     material->setObjectName(aiStr.length ? aiStr.C_Str() : "Untitled");
  //   if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_AMBIENT, color))
  //     material->setAmbient((color.r + color.g + color.b) / 3.0f);
  //   if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
  //     material->setDiffuse((color.r + color.g + color.b) / 3.0f);
  //     material->setColor(QVector3D(color.r, color.g, color.b) / material->diffuse());
  //   }
  //   if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_SPECULAR, color))
  //     material->setSpecular((color.r + color.g + color.b) / 3.0f);
  //   if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_SHININESS, value) && !qFuzzyIsNull(value))
  //     material->setShininess(value);
  //   if (AI_SUCCESS == aiMaterialPtr->GetTexture(aiTextureType_DIFFUSE, 0, &aiStr)) {
  //     QString filePath = m_dir.absolutePath() + '/' + QString(aiStr.C_Str()).replace('\\', '/');
  //     material->setDiffuseTexture(textureLoader.loadFromFile(Texture::Diffuse, filePath));
  //   }
  //   if (AI_SUCCESS == aiMaterialPtr->GetTexture(aiTextureType_SPECULAR, 0, &aiStr)) {
  //     QString filePath = m_dir.absolutePath() + '/' + QString(aiStr.C_Str()).replace('\\', '/');
  //     material->setSpecularTexture(textureLoader.loadFromFile(Texture::Specular, filePath));
  //   }
  //   if (AI_SUCCESS == aiMaterialPtr->GetTexture(aiTextureType_HEIGHT, 0, &aiStr)) {
  //     QString filePath = m_dir.absolutePath() + '/' + QString(aiStr.C_Str()).replace('\\', '/');
  //     material->setBumpTexture(textureLoader.loadFromFile(Texture::Bump, filePath));
  //   }
  //   return material;
  // }

  QDataStream & operator>>(QDataStream & in, engine::Mesh::MeshType & meshType) {
    qint32 t;
    in >> t;
    if (t == 0)
      meshType = Mesh::Triangle;
    else if (t == 1)
      meshType = Mesh::Line;
    else
      meshType = Mesh::Point;
    return in;
  }
}