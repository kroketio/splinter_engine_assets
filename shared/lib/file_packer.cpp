#include "file_packer.h"

QList<PackedFile> unpackFiles(const QByteArray& buffer) {
  QList<PackedFile> result;

  // Split header and data
  int headerEnd = buffer.indexOf('\n');
  if (headerEnd == -1) {
    qWarning() << "No JSON header found!";
    return result;
  }

  QByteArray jsonHeader = buffer.left(headerEnd);
  QByteArray fileDataSection = buffer.mid(headerEnd + 1);

  QJsonDocument doc = QJsonDocument::fromJson(jsonHeader);
  if (!doc.isArray()) {
    qWarning() << "Invalid JSON format!";
    return result;
  }

  QJsonArray jsonArray = doc.array();
  for (const QJsonValue& value : jsonArray) {
    QJsonObject obj = value.toObject();
    QString fileName = obj["fileName"].toString();
    qint64 offset = obj["offset"].toInt();
    qint64 size = obj["fileSize"].toInt();

    QByteArray fileData = fileDataSection.mid(offset, size);

    result.append(PackedFile{fileName, fileData});
  }

  return result;
}

QByteArray packFiles(const QStringList& filepaths) {
  struct MetaEntry {
    QString fileName;
    qint64 fileSize;
    qint64 offset;
  };

  QList<MetaEntry> metadata;
  QByteArray finalBuffer;
  qint64 currentOffset = 0;

  QByteArray fileDataSection;

  for (const QString& path : filepaths) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
      qWarning() << "Failed to open" << path;
      continue;
    }

    QByteArray content = file.readAll();
    MetaEntry entry = {
      QFileInfo(file).fileName(),
      content.size(),
      currentOffset
  };

    metadata.append(entry);
    fileDataSection.append(content);
    currentOffset += content.size();
  }

  // Convert metadata to JSON
  QJsonArray jsonArray;
  for (const auto& entry : metadata) {
    QJsonObject obj;
    obj["fileName"] = entry.fileName;
    obj["fileSize"] = static_cast<qint64>(entry.fileSize);
    obj["offset"] = static_cast<qint64>(entry.offset);
    jsonArray.append(obj);
  }

  QJsonDocument doc(jsonArray);
  QByteArray jsonHeader = doc.toJson(QJsonDocument::Compact);
  jsonHeader.append('\n'); // Terminate JSON header line

  finalBuffer.append(jsonHeader);
  finalBuffer.append(fileDataSection);

  return finalBuffer;
}