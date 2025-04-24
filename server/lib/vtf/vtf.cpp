#include <unordered_map>
#include <functional>
#include <iostream>
#include <algorithm>

#include "vtf.h"

#include "lib/nameof.hpp"
#include "lib/vtf/common/enums.hpp"
#include "lib/vtf/common/image.hpp"
#include "lib/vtf/common/util.hpp"
#include "lib/vtf/common/vtftools.hpp"

using namespace VTFLib;

namespace vtf {

void VtfConvertThread::run() {
	m_worker = new VtfConvertWorker(m_textures, m_output_dir, this);
	connect(m_worker, &VtfConvertWorker::logMessage, this, &VtfConvertThread::logMessage);
	connect(m_worker, &VtfConvertWorker::finished, this, &QThread::quit);	
	connect(m_worker, &VtfConvertWorker::finished, m_worker, &VtfConvertWorker::deleteLater);
	connect(m_worker, &VtfConvertWorker::iterationFinished, this, &VtfConvertThread::iterationFinished);
	m_worker->process();
}

void VtfConvertThread::setTextures(QList<QSharedPointer<Texture>> textures) {
	m_textures = textures;
}

void VtfConvertThread::setOutputDir(QString output_dir) {
	m_output_dir = output_dir;
}

VtfConvertWorker::VtfConvertWorker(QList<QSharedPointer<Texture>> textures, QString output_dir, QObject* parent) : 
	m_textures(textures), 
	m_output_dir(output_dir), 
	QObject(parent) {}

void VtfConvertWorker::process() {
	for(QSharedPointer<Texture> &tex: m_textures) {
		auto path_relative = tex->assetPack()->dir();
		QSharedPointer<TextureImage> diffuse = tex->get_diffuse(TextureSize::x1024, true);
		if(!diffuse || !diffuse->path.exists()) {
			emit logMessage("skipping, no defuse: " + tex->name + " ");
			continue;
		}

		auto filename = diffuse->path.completeBaseName();
		auto filename_out_vtf =  QString("%1.vtf").arg(filename);
		auto filename_out_vmt =  QString("%1.vmt").arg(filename);

		QDir dir(m_output_dir + QDir::separator() + path_relative);
		QString path_in_str = diffuse->path.absoluteFilePath();
		QString path_out_vtf_str = dir.filePath(filename_out_vtf);
		QString path_out_vmt_str = dir.filePath(filename_out_vmt);
		std::filesystem::path path_in(path_in_str.toStdString());
		std::filesystem::path path_out_vtf(path_out_vtf_str.toStdString());
		std::filesystem::path path_out_vmt(path_out_vmt_str.toStdString());

		if(!std::filesystem::exists(path_out_vtf_str.toStdString())) {
			emit logMessage("writing: " + filename + ".vtf");
			this->process_file(path_relative, path_in, path_out_vtf, path_out_vmt);
		} else {
			emit logMessage("already exists: " + filename + ".vtf");
		}

		emit iterationFinished();
	}

    emit finished();
}

bool VtfConvertWorker::set_properties(VTFLib::CVTFFile* vtfFile) {
	int compressionLevel = 0;  // Cannot use DEFLATE when version is <= 7.5

	//if (m_opts->has(opts::version) || compressionLevel > 0 || vtfFile->GetFormat() == IMAGE_FORMAT_BC7) {
	// HARDCODED TO VERSION 7.6
	auto verStr = "7.2";

	int majorVer = 7;
	int minorVer = 2;
	// if (!get_version_from_str(verStr, majorVer, minorVer)) {
	// 	qWarning() << "Invalid version" << verStr << "! Valid versions: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6";
	// 	return false;
	// }

	minorVer = minorVer;
	vtfFile->SetVersion(majorVer, minorVer);

	// Set the DEFLATE compression level
	if (!vtfFile->SetAuxCompressionLevel(compressionLevel) && compressionLevel != 0) {
		qWarning() << "Could not set compression level to" << compressionLevel;
		return false;
	}

	// These should be defaulted to off
	// we're not going to set them explicitly to the value of the opts because we may have gotten them from another vtf

	bool normal = false;
	if (normal)
		vtfFile->SetFlag(TEXTUREFLAGS_NORMAL, true);

	// if (m_opts->get<bool>(opts::clamps))

	// if (m_opts->get<bool>(opts::trilinear))
	// 	vtfFile->SetFlag(TEXTUREFLAGS_TRILINEAR, true);

	// if (m_opts->get<bool>(opts::pointsample))
	// 	vtfFile->SetFlag(TEXTUREFLAGS_POINTSAMPLE, true);

	// if (m_opts->get<bool>(opts::srgb))
	// 	vtfFile->SetFlag(TEXTUREFLAGS_SRGB, true);

	// Mip count gets set earlier by user input
	if (vtfFile->GetMipmapCount() == 1)
		vtfFile->SetFlag(TEXTUREFLAGS_NOMIP, true);

	// Same deal for the below issues- only override default if specified
	// @TODO: possibly start at 0
	// if (m_opts->has(opts::startframe))
	// 	vtfFile->SetStartFrame(0);

	vtfFile->ComputeReflectivity();

	// @TODO: possibly set to 0
	// if (m_opts->has(opts::bumpscale))
	// 	vtfFile->SetBumpmapScale(m_opts->get<float>(opts::bumpscale));

	return true;
}

bool VtfConvertWorker::add_image_data_raw(
	VTFLib::CVTFFile* file, const void* data, VTFImageFormat format, VTFImageFormat dataFormat, int w, int h,
	bool create) {
	vlByte* dest = nullptr;

	// Convert to requested format, if necessary
	if (format != IMAGE_FORMAT_NONE && format != dataFormat) {

		auto sizeRequired = CVTFFile::ComputeImageSize(w, h, 1, 1, format);
		dest = (vlByte*)malloc(sizeRequired);

		if (!CVTFFile::Convert((vlByte*)data, dest, w, h, dataFormat, format)) {
			qWarning() << "Could not convert from" << NAMEOF_ENUM(dataFormat) << "to" << NAMEOF_ENUM(format) << ":" << util::get_last_vtflib_error();
			free(dest);
			return false;
		}
	}
	else {
		format = dataFormat;
	}

	// Create the file if we're told to do so
	// This is done here because we don't actually know w/h until now
	if (create) {
		if (!file->Init(w, h, 1, 1, 1, format, vlTrue, m_mips <= 0 ? CVTFFile::ComputeMipmapCount(w, h, 1) : m_mips)) {
			qWarning() << "Could not create VTF:" << util::get_last_vtflib_error();
			free(dest);
			return false;
		}
	}
	file->SetData(1, 1, 1, 0, dest ? dest : (vlByte*)data);

	return true;
}

bool VtfConvertWorker::add_image_data(
		const std::filesystem::path& imageSrc, 
		VTFLib::CVTFFile* file, 
		VTFImageFormat format, 
		bool create) {
	// Load the image
	auto image = imglib::Image::load(imageSrc);
	if (!image)
		return false;

	// If width and height are specified, resize in place
	if (m_height != -1 && m_width != -1) {
		if (!image->resize(m_width, m_height))
			return false;
	}

	// Add the raw image data
	return add_image_data_raw(
		file, image->data(), format, image->vtf_format(), image->width(), image->height(), create);
}

bool VtfConvertWorker::process_file(
  QString assetPackPath,
  const std::filesystem::path& srcFile,
  const std::filesystem::path& outputVtf,
  const std::filesystem::path& outputVmt) {

	const std::string formatStr = "DXT1";
	const auto srgb = false;
	const auto thumbnail = true;
	const auto verStr = "";
	const auto isNormal = false;

	auto nomips = false;
	m_mips = 10;

	m_width = 512;
	m_height = 512;

	if (!std::filesystem::exists(srcFile)) {
		std::cerr << "Could not open " << srcFile << ": file does not exist\n";
		return false;
	}

	if(srcFile.filename().extension() != ".png") {
		std::cerr << "Can only convert PNG files\n";
		return false;
	}

	bool isvtf = false;

	// If an out file name is not provided, we need to build our own
	// if (outputVtf.empty()) {
	// 	outFile = srcFile.parent_path() / srcFile.filename().replace_extension(".vtf");
	// }
	// else {
	// 	outFile = srcFile.parent_path() / outputVtf;
	// }

	auto format = ImageFormatFromUserString(formatStr.c_str());
	auto vtfFile = std::make_unique<CVTFFile>();

	// We will choose the best format to operate on here. This simplifies later code and lets us avoid extraneous
	// conversions
	auto formatInfo = CVTFFile::GetImageFormatInfo(format);
	imglib::ChannelType procChanType = imglib::UInt8;
	const auto procFormat = IMAGE_FORMAT_DXT1;
	// @TODO: fix mipmaps
	// const auto procFormat = [formatInfo, &procChanType]() -> VTFImageFormat
	// {
	// 	auto maxBpp = std::max(
	// 		std::max(formatInfo.uiRedBitsPerPixel, formatInfo.uiGreenBitsPerPixel),
	// 		std::max(formatInfo.uiBlueBitsPerPixel, formatInfo.uiAlphaBitsPerPixel));
	// 	if (maxBpp > 16) {
	// 		procChanType = imglib::Float;
	// 		return IMAGE_FORMAT_RGBA32323232F;
	// 	}
	// 	else if (maxBpp > 8) {
	// 		procChanType = imglib::UInt16;
	// 		return IMAGE_FORMAT_RGBA16161616F;
	// 	}
	// 	else {
	// 		procChanType = imglib::UInt8;
	// 		return IMAGE_FORMAT_RGBA8888;
	// 	}
	// }();

	// If we're processing a VTF, let's add that VTF image data
	size_t initialSize = 0;

  	// Add standard image data
	if (!add_image_data(srcFile, vtfFile.get(), procFormat, true)) {
		std::cerr << "Could not add image data from file " << srcFile << "\n";
		return false;
	}

	// Set the properties based on user input
	if (!set_properties(vtfFile.get())) {
		std::cerr << "Could not set properties on VTF\n";
		return false;
	}

	// Generate thumbnail
	if (thumbnail && !vtfFile->GenerateThumbnail(srgb)) {
		// std::cerr << fmt::format("Could not generate thumbnail: {}\n", util::get_last_vtflib_error());
		return false;
	}

	// Generate mips
	if (!vtfFile->GenerateMipmaps(MIPMAP_FILTER_CATROM, srgb)) {
		std::cerr << "Could not generate mipmaps!\n";
		return false;
	}

	// Convert to desired image format
	if (vtfFile->GetFormat() != format && !vtfFile->ConvertInPlace(format)) {
		// std::cerr << fmt::format("Could not convert image data to {}: {}\n", formatStr, util::get_last_vtflib_error());
		return false;
	}

	// Save to disk finally
	if (!vtfFile->Save(outputVtf.string().c_str())) {
		qDebug() << "Could not save file" << outputVtf.string() << ":" << util::get_last_vtflib_error();
		return false;
	}

	// write a basic VMT
	QString filename = QString::fromStdString(outputVtf.filename());
	filename = filename.replace(".vtf", "");
	QString vmt = "\"LightmappedGeneric\"\n" \
		"{\n" \
		"\"$baseTexture\" \"%1/%2\"\n" \
		"\"$surfaceprop\" \"brick\"\n" \
		"}\n";
	vmt = vmt.replace("%1", assetPackPath);
	vmt = vmt.replace("%2", filename);

    QFile file(outputVmt);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << vmt;
        file.close();
    }
	file.close();

	return true;
}

VtfConvertJob::VtfConvertJob(
	QList<QSharedPointer<Texture>> textures,
	QString output_dir,
	QObject *parent) : 
		m_textures(textures),
		m_output_dir(output_dir),
		QObject(parent) {

	m_num_textures = m_textures.size();
	qDebug() << "VtfConvertJob" << m_num_textures << "textures";

	// create necessary directories
	QStringList dirs_created = {};
	for(const auto &tex: textures) {
		auto path_relative = tex->assetPack()->dir();
		QDir dir(m_output_dir + QDir::separator() + path_relative);
		if (!dirs_created.contains(dir.absolutePath()) && !QDir(dir).exists()) {
			dir.mkpath(".");
			dirs_created << dir.absolutePath();
		}
	}

	// 6 threads
	unsigned int threads = 6;
	int baseSize = m_num_textures / threads;       // Base size for each segment
	int remainder = m_num_textures % threads;      // Remaining items to distribute

	int segmentSize[threads];
	for (int i = 0; i < threads; ++i) {
		segmentSize[i] = baseSize + (i < remainder ? 1 : 0);
	}

	QList<QSharedPointer<Texture>> a;
	QList<QSharedPointer<Texture>> b;
	QList<QSharedPointer<Texture>> c;
	QList<QSharedPointer<Texture>> d;
	QList<QSharedPointer<Texture>> e;
	QList<QSharedPointer<Texture>> f;

	// distribute
	int index = 0;
	for (int i = 0; i < threads; ++i) {
		for (int j = 0; j < segmentSize[i]; ++j) {
			if (i == 0) a.append(m_textures.at(index));
			else if (i == 1) b.append(m_textures.at(index));
			else if (i == 2) c.append(m_textures.at(index));
			else if (i == 3) d.append(m_textures.at(index));
			else if (i == 4) e.append(m_textures.at(index));
			else f.append(m_textures.at(index));
			++index;
		}
	}

	// create threads & start
	for(const QList<QSharedPointer<Texture>> &collection: {a, b, c, d, e, f}) {
		if(!collection.isEmpty()) {
			auto thread = new VtfConvertThread();
			connect(thread, &VtfConvertThread::logMessage, this, &VtfConvertJob::logMessage);
			connect(thread, &VtfConvertThread::finished, this, &VtfConvertJob::onThreadFinished);
			connect(thread, &VtfConvertThread::iterationFinished, this, &VtfConvertJob::onIterationFinished);
			connect(thread, &VtfConvertThread::finished, [this, thread]{
				thread->deleteLater();
			});

			thread->setTextures(collection);
			thread->setOutputDir(m_output_dir);
			m_threads << thread;
		}
	}

	qDebug() << "threads" << m_threads.size();
    for (VtfConvertThread* thread : m_threads) {
        if (thread) {
			qDebug() << "starting thread";
            thread->start();
        }
    }
}

void VtfConvertJob::onIterationFinished() {
	m_iterations += 1;

	double _progress = 100 * (double(m_iterations) / double(m_num_textures));
	emit progress((int)_progress);
}

void VtfConvertJob::onThreadFinished() {
	// qDebug() << "thread finished";
	m_threads_finished += 1;
	if(m_threads_finished == m_threads.size()) {
		qDebug() << "all threads done";
		emit finished();
	}
}

}
