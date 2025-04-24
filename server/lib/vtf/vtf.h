#pragma once

#include <QObject>
#include <QDebug>
#include <QThread>
#include <filesystem>
#include <QTimer>

#include "VTFLib.h"
#include "VTFFormat.h"

#include "models/asset_pack.h"
#include "models/texture_collection.h"

namespace VTFLib {
	class CVTFFile;
}

class VtfConvertWorker;
class VtfConvertThread;

namespace vtf {
  class VtfConvertWorker : public QObject {
    Q_OBJECT

    public:
      VtfConvertWorker(QList<QSharedPointer<Texture>> textures, QString output_dir, QObject* parent = 0);

    public slots:
        void process();

    signals:
      void finished();
      void iterationFinished();
      void logMessage(QString msg);

    private:
      bool set_properties(VTFLib::CVTFFile* file);

      // bool process_file(QList<Texture> *textures);
      bool process_file(QString assetPackPath, const std::filesystem::path& srcFile, 
        const std::filesystem::path& outputVtf, const std::filesystem::path& outputVmt);

      bool add_image_data(const std::filesystem::path& imageSrc, VTFLib::CVTFFile* file, VTFImageFormat format, bool create);

      bool add_image_data_raw(
        VTFLib::CVTFFile* file, const void* data, VTFImageFormat format, VTFImageFormat dataFormat, int w, int h,
        bool create);

      // bool add_vtf_image_data(VTFLib::CVTFFile* srcImage, VTFLib::CVTFFile* file, VTFImageFormat format);
      // VTFLib::CVTFFile*
      // init_from_file(const std::filesystem::path& src, VTFLib::CVTFFile* file, VTFImageFormat newFormat);
      QString m_output_dir;
      int m_mips = 10;
      int m_width = -1;
      int m_height = -1;
      QList<QSharedPointer<Texture>> m_textures;
  };

  class VtfConvertThread : public QThread {
    Q_OBJECT

    public:
      void setTextures(QList<QSharedPointer<Texture>> textures);
      void setOutputDir(QString output_dir);

    protected:
      void run() override;

    signals:
      void logMessage(QString msg);
      void iterationFinished();

    private:
      QString m_output_dir;
      VtfConvertWorker* m_worker = nullptr;
      QList<QSharedPointer<Texture>> m_textures;
  };

  class VtfConvertJob : public QObject {
    Q_OBJECT

    public:
      VtfConvertJob(QList<QSharedPointer<Texture>> textures, QString output_dir, QObject *parent = 0);

    public slots:
      void onThreadFinished();
      void onIterationFinished();

    signals:
      void finished();
      void progress(int pct);
      void logMessage(QString msg);

    private:
      QList<VtfConvertWorker*> m_workers;
      QList<QSharedPointer<Texture>> m_textures;
      QList<VtfConvertThread*> m_threads;
      QString m_output_dir;
      unsigned int m_num_textures = 0;
      unsigned int m_threads_finished = 0;
      unsigned int m_iterations = 0;
  };

}
