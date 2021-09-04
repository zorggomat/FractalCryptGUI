#ifndef QUAZIPFUNCTIONS_H
#define QUAZIPFUNCTIONS_H

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "quazip/quazipdir.h"
#include "quazip/JlCompress.h"

class QuazipFunctions
{
    class CountingFile;
    static bool compressSubDir(QuaZip* zip, QString dir, QString origDir, quint32 &fileCount);
public:
    static bool writeZip(QString containerPath, quint64 offset, const QStringList &files, const QStringList &directories, qint64 &bytesWritten);
    static bool readZip(QIODevice &file, quint64 offset, quint64 end, QString destination);
};

#endif // QUAZIPFUNCTIONS_H
