#pragma once

#include <iostream>

#include <QMainWindow>
#include <QCompleter>
#include <QCheckBox>
#include <QPushButton>
#include <QClipboard>
#include <QStringListModel>
#include <QTimer>
#include <QQuickWidget>
#include <QQuickView>
#include <QQmlContext>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QMutex>
#include <QFileDialog>

#include "ctx.h"
#include "engine/gl/gl_window.h"
#include "lib/utils.h"

#include "assimp/mesh.h"

namespace Ui {
  class MainWindow;
}

class AppContext;
class MainWindow : public QMainWindow
{
Q_OBJECT
Q_PROPERTY(int example_property MEMBER m_example_property NOTIFY example_propertyChanged);

public:
  explicit MainWindow(AppContext *ctx, QWidget *parent = nullptr);
  void test();
  engine::Mesh* loadMesh(const aiMesh * aiMeshPtr);
  ~MainWindow() override;

  qreal screenDpiRef;
  QRect screenGeo;
  QRect screenRect;
  qreal screenDpi;
  qreal screenDpiPhysical;
  qreal screenRatio;

signals:
  void example_propertyChanged();

public slots:
  void onWindowTitle(const QString &title);
  void onExample(int status);

protected:
  void closeEvent(QCloseEvent *event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

private:
  Ui::MainWindow *ui;
  AppContext *m_ctx = nullptr;
  QQuickWidget *m_quickWidget = nullptr;

  unsigned int m_example_property = 320;
  void createQml();
  void destroyQml();
private:
  float bla = 0.0;
  engine::Mesh *m_mesh = nullptr;
  engine::OpenGLWindow* m_glWindow;
  engine::Scene* m_scene;
};
