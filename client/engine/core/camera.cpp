#include "camera.h"

namespace engine {
  Camera::Camera(QObject* parent): QObject(0) {
    setObjectName("Camera");
    reset();

    // setPosition(QVector3D(5, 5, 5));
    // setPosition(QVector3D(704.0f, -256.0f, 0.0f));
    setPosition(m_position_spawn);
    setParent(parent);
  }

  Camera::Camera(QVector3D position, QVector3D direction, QObject* parent): QObject(0) {
    setObjectName("Camera");
    setMovingSpeed(0.2f * 35);
    setFieldOfView(40.0f);
    setNearPlane(0.1f);
    setFarPlane(m_far_plane);
    setPosition(position);
    setDirection(direction);
    setParent(parent);
  }

  Camera::Camera(const Camera & camera): QObject(0) {
    m_movingSpeed = camera.m_movingSpeed;
    m_fieldOfView = camera.m_fieldOfView;
    m_aspectRatio = camera.m_aspectRatio;
    m_nearPlane = camera.m_nearPlane;
    m_farPlane = camera.m_farPlane;
    m_position = camera.m_position;
    m_direction = camera.m_direction;
    m_up = camera.m_up;
    setObjectName(camera.objectName());
  }

  Camera::~Camera() {
    qDebug() << "Camera" << this->objectName() << "is destroyed";
  }

  void Camera::moveForward(float shift) {
    setPosition(m_position + m_direction * shift * m_movingSpeed);
  }

  void Camera::moveRight(float shift) {
    setPosition(m_position + QVector3D::crossProduct(m_direction, m_up) * shift * m_movingSpeed);
  }

  void Camera::moveUp(float shift) {
    setPosition(m_position + m_up * shift* m_movingSpeed);
  }

  void Camera::turnLeft(float angle) {
    QMatrix4x4 mat;
    mat.rotate(angle, m_hmm);
    setDirection(mat * m_direction);
  }

  void Camera::lookUp(float angle) {
    QMatrix4x4 mat;
    mat.rotate(angle, QVector3D::crossProduct(m_direction, m_up));
    setDirection(mat * m_direction);
  }

  void Camera::dumpObjectInfo(int l) {
    qDebug().nospace() << tab(l) << "Camera: " << objectName();
    qDebug().nospace() << tab(l + 1) << "Position:      " << m_position;
    qDebug().nospace() << tab(l + 1) << "Direction:     " << m_direction;
    qDebug().nospace() << tab(l + 1) << "Up:            " << m_up;
    qDebug().nospace() << tab(l + 1) << "Moving Speed:  " << m_movingSpeed;
    qDebug().nospace() << tab(l + 1) << "Field of View: " << m_fieldOfView;
    qDebug().nospace() << tab(l + 1) << "Aspect Ratio:  " << m_aspectRatio;
    qDebug().nospace() << tab(l + 1) << "Near Plane:    " << m_nearPlane;
    qDebug().nospace() << tab(l + 1) << "Far Plane:     " << m_farPlane;
  }

  void Camera::dumpObjectTree(int l) {
    dumpObjectInfo(l);
  }

  float Camera::movingSpeed() const {
    return m_movingSpeed;
  }

  float Camera::fieldOfView() const {
    return m_fieldOfView;
  }

  float Camera::aspectRatio() const {
    return m_aspectRatio;
  }

  float Camera::nearPlane() const {
    return m_nearPlane;
  }

  float Camera::farPlane() const {
    return m_farPlane;
  }

  QVector3D Camera::position() const {
    return m_position;
  }

  QVector3D Camera::direction() const {
    return m_direction;
  }

  // Get projection and view matrix

  QMatrix4x4 Camera::projectionMatrix() const {
    QMatrix4x4 mat;
    mat.perspective(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);
    return mat;
  }

  QMatrix4x4 Camera::viewMatrix() const {
    QMatrix4x4 mat;
    mat.lookAt(m_position, m_position + m_direction, m_up);
    return mat;
  }

  // Public slots

  void Camera::reset() {
    setMovingSpeed(0.2f * 35);
    setFieldOfView(45.0f);
    setAspectRatio(1.0f);
    setNearPlane(0.1f);
    setFarPlane(m_far_plane);
    setPosition(m_position_spawn);
    setDirection(QVector3D(-1, -1, -1));

    qDebug() << this->objectName() << "is reset";
  }

  void Camera::setMovingSpeed(float movingSpeed) {
    if (!isEqual(m_movingSpeed, movingSpeed)) {
      m_movingSpeed = movingSpeed;
      movingSpeedChanged(m_movingSpeed);
    }
  }

  void Camera::setFieldOfView(float fieldOfView) {
    if (!isEqual(m_fieldOfView, fieldOfView)) {
      m_fieldOfView = fieldOfView;
      fieldOfViewChanged(m_fieldOfView);
    }
  }

  void Camera::setAspectRatio(float aspectRatio) {
    if (!isEqual(m_aspectRatio, aspectRatio)) {
      m_aspectRatio = aspectRatio;
      aspectRatioChanged(m_aspectRatio);
    }
  }

  void Camera::setNearPlane(float nearPlane) {
    if (!isEqual(m_nearPlane, nearPlane)) {
      m_nearPlane = nearPlane;
      nearPlaneChanged(m_nearPlane);
    }
  }

  void Camera::setFarPlane(float farPlane) {
    if (!isEqual(m_farPlane, farPlane)) {
      m_farPlane = farPlane;
      farPlaneChanged(m_farPlane);
    }
  }

  void Camera::setPosition(QVector3D position) {
    if (!isEqual(m_position, position)) {
      m_position = position;
      positionChanged(m_position);
    }
  }

  void Camera::setDirection(QVector3D direction) {
    direction.normalize();
    if (!isEqual(m_direction, direction)) {
      m_direction = direction;
      setUpVector();
      directionChanged(m_direction);
    }
  }

  // Private functions

  void Camera::setUpVector() {
    QVector3D t = QVector3D::crossProduct(m_direction, m_hmm);
    m_up = QVector3D::crossProduct(t, m_direction);
  }

  // void Camera::setUpVector() {
  //   const QVector3D worldUp(0, 0, 1);  // Y-up system
  //   QVector3D right = QVector3D::crossProduct(m_direction.normalized(), worldUp).normalized();
  //   m_up = QVector3D::crossProduct(right, m_direction).normalized();
  // }
}