#include "quazipfunctions.h"

class QuazipFunctions::CountingFile : public QFile
{
    qint64 bytesWritten;
public:
    explicit CountingFile(QString path): QFile(path), bytesWritten(0) { }
    qint64 getBytesWritten()
    {
        return bytesWritten;
    }
protected:
    virtual qint64 writeData(const char *data, qint64 maxSize) override
    {
        qint64 result = QFile::writeData(data, maxSize);
        bytesWritten += result;
        return result;
    }
};

bool QuazipFunctions::compressSubDir(QuaZip* zip, QString dir, QString origDir, quint32 &fileCount)
{
    if (!zip) return false;

    QDir directory(dir);
    if (!directory.exists()) return false;

    QDir origDirectory(origDir);
    QuaZipFile dirZipFile(zip);
    QString path = origDirectory.dirName() + "/";
    if(dir != origDir)
        path += origDirectory.relativeFilePath(dir) + "/";
    QuaZipNewInfo info(path, dir);

    if (!dirZipFile.open(QIODevice::WriteOnly, info)) return false;
    dirZipFile.close();

    QFileInfoList directories = directory.entryInfoList(QDir::AllDirs|QDir::System|QDir::Hidden|QDir::NoDotAndDotDot);
    for (QFileInfo dir : directories)
        if(!compressSubDir(zip, dir.absoluteFilePath(), origDir, fileCount)) return false;

    QFileInfoList files = directory.entryInfoList(QDir::Files|QDir::System|QDir::Hidden);
    for (QFileInfo file : files)
    {
        if(!file.isFile()||file.absoluteFilePath() == zip->getZipName()) continue;
        QString filename = origDirectory.dirName() + "/" + origDirectory.relativeFilePath(file.absoluteFilePath());
        if (!JlCompress::compressFile(zip ,file.absoluteFilePath(), filename)) return false;
        fileCount++;
    }
    ++fileCount;
    return true;
}

bool QuazipFunctions::writeZip(QString containerPath, quint64 offset, const QStringList &files, const QStringList &directories, qint64 &bytesWritten)
{
    CountingFile container(containerPath);
    if(!container.open(QIODevice::ReadWrite)) return false;

    QuaZip zip(&container);
    if(!zip.open(QuaZip::Mode::mdCreate)) return false;
    container.seek(offset);

    for(QString file : files)
    {
        if(!JlCompress::compressFile(&zip, file, QFileInfo(file).fileName()))
            return false;
    }

    quint32 fileCount = files.size();
    for(QString dir : directories)
    {
        if(!compressSubDir(&zip, dir, dir, fileCount))
            return false;
    }

    zip.close();
    bytesWritten = container.getBytesWritten() - 12*fileCount;
    return zip.getZipError() == UNZ_OK;
}

bool QuazipFunctions::readZip(QIODevice &file, quint64 offset, quint64 size, QString destination)
{
    QuaZip zip(&file);
    zip.setAutoClose(false);
    if(!zip.open(QuaZip::Mode::mdUnzip, NULL, size, 1)) return false;
    file.seek(offset);

    QDir directory(destination);
    if (!zip.goToFirstFile()) return false;
    do {
        if (!JlCompress::extractFile(&zip, "", directory.absoluteFilePath(zip.getCurrentFileName())))
            return false;
    } while (zip.goToNextFile());
    zip.close();
    return zip.getZipError() == UNZ_OK;
}
