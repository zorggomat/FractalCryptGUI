#ifndef FRACTALCRYPTCORE_H
#define FRACTALCRYPTCORE_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QCryptographicHash>
#include <QDir>
#include <QMap>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "quazipfunctions.h"
#include "progressdialog.h"
#include "aes.h"
#include "noizecreator.hpp"

class FractalCryptCore
{
    QEventLoop *loop;
    ProgressDialog *progressDialog;
    NoizeCreator *noizeCreator;
    AES *aes;

    FractalCryptCore();
    ~FractalCryptCore();
    FractalCryptCore(const FractalCryptCore&);
    FractalCryptCore& operator=(const FractalCryptCore&);

public:

    enum StatusCode
    {
        OK,
        ContainerUnavailable,
        SignatureInvalid,
        HeaderSizeInvalid,
        NotEnoughSpace,
        ZipError,
        EncryptionError,
        NewSizeTooSmall
    };

    static constexpr char signature[8] = "fractal";

    static FractalCryptCore& Instance();

    void createNoize(QIODevice *iodevice, qint64 bytes);

    static const QString& getCodeDescription(StatusCode statusCode);

    StatusCode writeLayer(QString containerPath, QStringList files, QStringList directories, QStringList passwords, QString newPassword);
    StatusCode readLayer(QString containerPath, QString filePath, QStringList passwords);
    StatusCode removeLayer(QString containerPath, QStringList passwords);

    StatusCode encryptFile(QIODevice *file, QStringList passwords, const QVector<qint64> &offsets);
    StatusCode decryptFile(QIODevice *file, QStringList passwords, QVector<qint64> &offsets);
    StatusCode resizeFile(QString path, QStringList passwords, qint64 newSize);

    static void writeHeader(QIODevice *file, qint64 offset, quint64 size);
    static StatusCode readHeader(QIODevice *file, qint64 offset, qint64 &size);
};

#endif // FRACTALCRYPTCORE_H
