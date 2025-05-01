#include <QQmlEngine>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "engine/gl/gl_window.h"

MainWindow::MainWindow(AppContext *ctx, QWidget *parent) :
    QMainWindow(parent),
    m_ctx(ctx),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  this->screenDpiRef = 128;
  this->screenGeo = QApplication::primaryScreen()->availableGeometry();
  this->screenRect = QGuiApplication::primaryScreen()->geometry();
  this->screenDpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
  this->screenDpiPhysical = QGuiApplication::primaryScreen()->physicalDotsPerInch();
  this->screenRatio = this->screenDpiPhysical / this->screenDpiRef;

  m_glWindow = new engine::OpenGLWindow;
  m_glWindow->setRenderer(new engine::OpenGLRenderer);
  ui->qml->layout()->addWidget(QWidget::createWindowContainer(m_glWindow));

  // this->createQml();
  this->show();
  this->test();

  // example get config value:
  auto test = config()->get(ConfigKeys::Test).toString();
  qDebug() << "config value: " << test;

  // example set config value
  // config()->set(ConfigKeys::Test, "test2");

  // example QWidget button handler
  connect(ui->pushButton, &QPushButton::clicked, [=] {
    qDebug() << "pushButton clicked";
  });
}

#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void MainWindow::test() {
  QString filePath = "/home/dsc/CLionProjects/godot/texture_browser/client/engine/resources/shapes/Plane.obj";

  Assimp::Importer importer;
  unsigned int flags =
    aiProcess_Triangulate |
    aiProcess_CalcTangentSpace |
    aiProcess_GenSmoothNormals |
    aiProcess_JoinIdenticalVertices |
    aiProcess_OptimizeGraph |
    aiProcess_GenUVCoords;

  // aiProcess_GenUVCoords
  /** <hr>This step converts non-UV mappings (such as spherical or
   *  cylindrical mapping) to proper texture coordinate channels.
   *
   * Most applications will support UV mapping only, so you will
   * probably want to specify this step in every case. Note that Assimp is not
   * always able to match the original mapping implementation of the
   * 3D app which produced a model perfectly. It's always better to let the
   * modelling app compute the UV channels - 3ds max, Maya, Blender,
   * LightWave, and Modo do this for example.
   *
   * @note If this step is not requested, you'll need to process the
   * <tt>#AI_MATKEY_MAPPING</tt> material property in order to display all assets
   * properly.
   */

  auto m_aiScenePtr = importer.ReadFile(filePath.toStdString(), flags);
  // QFile file(filePath);
  // QByteArray bytes = file.readAll();
  // m_aiScenePtr = importer.ReadFileFromMemory(bytes.constData(), bytes.length(), flags);

  if (!m_aiScenePtr || !m_aiScenePtr->mRootNode || m_aiScenePtr->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
    qWarning() << importer.GetErrorString();
    return;
  }

  const auto aiNodePtr = m_aiScenePtr->mRootNode;
  auto bla = aiNodePtr->mName.C_Str();
  auto aiMesh = m_aiScenePtr->mMeshes[aiNodePtr->mMeshes[0]];

  m_scene = new engine::Scene;
  m_scene->addGridline(new engine::Gridline);
  m_scene->addDirectionalLight(new engine::DirectionalLight(QVector3D(1, 1, 1), QVector3D(-2, -4, -3)));
  m_glWindow->setScene(new engine::OpenGLScene(m_scene));

  const auto start = std::chrono::high_resolution_clock::now();
  // auto* mesh = new engine::JustAQuad(this);
  // auto* mesh2 = engine::Mesh::loadMesh(aiMesh);
  int wegiwoeogwe = 1;

  // engine::Material* material = new engine::Material;
  // aiColor4D color; float value; aiString aiStr;
  // material->setObjectName("Untitled");
  // auto aiMaterialPtr = m_aiScenePtr->mMaterials[aiMesh->mMaterialIndex];
  // if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_NAME, aiStr))
  //   material->setObjectName(aiStr.length ? aiStr.C_Str() : "Untitled");
  // if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_AMBIENT, color))
  //   material->setAmbient((color.r + color.g + color.b) / 3.0f);
  // if (AI_SUCCESS == aiMaterialPtr->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
  //   material->setDiffuse((color.r + color.g + color.b) / 3.0f);
  //   material->setColor(QVector3D(color.r, color.g, color.b) / material->diffuse());
  // }
  // mesh->setMaterial(material);

  // m_scene->meshAdded(mesh);

  const auto end = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "function took " << duration.count() << " ms\n";

  // auto x = QString("/media/dsc/0376C0A40D1AE4C9/splinter_test_removeme/splinter.vmf");
  // m_scene->loadVMF("/media/dsc/0376C0A40D1AE4C9/splinter_test_removeme/splinter.vmf");
  m_scene->loadVMF("/home/dsc/CLionProjects/godot/texture_browser/cube_test.vmf");

  // QVector3D v0(704, -256, 0);
  // QVector3D v1(768, -256, 0);
  // QVector3D v2(768, -832, 0);
  // QVector3D v3(704, -832, 0);

  // // Compute normal from the plane (using right-hand rule)
  // QVector3D edge1 = v1 - v0;
  // QVector3D edge2 = v2 - v0;
  // QVector3D normal = QVector3D::normal(edge1, edge2);
  //
  // // Compute tangent and bitangent (arbitrary basis, planar)
  // QVector3D tangent = (v1 - v0).normalized(); // X direction
  // QVector3D bitangent = QVector3D::crossProduct(normal, tangent).normalized(); // Y direction
  //

  // // Optional: simple planar UVs (scale or transform as needed)
  // QVector2D uv0(0, 0);
  // QVector2D uv1(1, 0);
  // QVector2D uv2(1, 1);
  // QVector2D uv3(0, 1);

  // std::vector<engine::Vertex> vertices(4);
  // vertices[0] = {v0, normal, tangent, bitangent, uv0};
  // vertices[1] = {v1, normal, tangent, bitangent, uv1};
  // vertices[2] = {v2, normal, tangent, bitangent, uv2};
  // vertices[3] = {v3, normal, tangent, bitangent, uv3};

  // QString str = "\"plane\" \"(704 -256 0) (768 -256 0) (768 -832 0)\"";
  // int startd = str.indexOf('(');
  // QVector<QVector3D> points;
  //
  // while (startd != -1) {
  //   int end = str.indexOf(')', startd);
  //   QStringList nums = str.mid(startd + 1, end - startd - 1).split(' ', Qt::SkipEmptyParts);
  //   points.append(QVector3D(nums[0].toFloat(), nums[1].toFloat(), nums[2].toFloat()));
  //   startd = str.indexOf('(', end);
  // }

  int igwieogw = 1;

  // m_aiScenePtr->mMaterials[aiMeshPtr->mMaterialIndex]
  // mesh->setMaterial(loadMaterial(m_aiScenePtr->mMaterials[aiMeshPtr->mMaterialIndex]));
}

void MainWindow::closeEvent(QCloseEvent *event) {
  //event->ignore();
  QApplication::quit();
}

void MainWindow::createQml() {
  if(m_quickWidget != nullptr) return;
  m_quickWidget = new QQuickWidget(this);

  auto *qctx = m_quickWidget->rootContext();
  qctx->setContextProperty("cfg", config());
  qctx->setContextProperty("ctx", m_ctx);

  qctx->setContextProperty("mainwindow", this);

  m_quickWidget->setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

  ui->qml->layout()->addWidget(m_quickWidget);
}

void MainWindow::onExample(int status) {
  qDebug() << "example() called from QML";
}

void MainWindow::onWindowTitle(const QString &title) {
  this->setWindowTitle(title);
}

void MainWindow::destroyQml() {
  if(m_quickWidget == nullptr) return;
  m_quickWidget->disconnect();
  m_quickWidget->deleteLater();
  m_quickWidget = nullptr;
}

MainWindow::~MainWindow() {
  delete ui;
}

