#pragma once

#include <zlib.h>
#include <QByteArray>

#define GZIP_WINDOWS_BIT 15 + 16
#define GZIP_CHUNK_SIZE 32 * 1024

class QCompressor
{
public:
  static bool gzipCompress(QByteArray input, QByteArray &output, int level = -1);
  static bool gzipDecompress(QByteArray input, QByteArray &output);
};
