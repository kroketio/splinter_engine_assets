#include <QGuiApplication>
#include <QFileSystemWatcher>
#include <QDebug>

class CSSTheme : public QObject {
Q_OBJECT

public:
  CSSTheme(QApplication *app, const QString &path_css, QObject *parent = nullptr) : 
        m_app(app),
        m_path_css(path_css),
        QObject(parent) {

    connect(&m_watcher, &QFileSystemWatcher::fileChanged,
    this, &CSSTheme::onFileChanged);
  }

  void enable_watcher() {
    m_watcher.addPath(m_path_css);
    qDebug() << "css theme watching file:" << m_path_css;
  }

  QString readCSS() {
    QFile theme_file(m_path_css);
    theme_file.open(QFile::ReadOnly);

    if(theme_file.isOpen()) {
      auto body = QString(theme_file.readAll());

      // CSS variables
      QMap<QString, QString> css_vars;
      for(const auto &line: body.split('\n')) {
        if(!line.startsWith("color_")) {
            continue;
        }

        QStringList spl = line.split('=');
        QString key = QString("$%1").arg(spl.at(0));
        QString val = spl.at(1);
        //key = key.remove(0, 6);

        // qDebug() << "key" << key;
        // qDebug() << "val" << val;
        // css_vars[key] = val;
        body = body.replace(key, val);
      }

      theme_file.close();
      return body;
    }

    return {};
  }

private slots:
  void onFileChanged(const QString &path) {
    qDebug() << "reloading app CSS:";
    auto css_body = readCSS();
    m_app->setStyleSheet(css_body);
    qDebug() << "reloaded app CSS:";
  }

private:
  QString m_path_css;
  QFileSystemWatcher m_watcher;
  QApplication *m_app = nullptr;
};