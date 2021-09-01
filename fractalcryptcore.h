#ifndef FRACTALCRYPTCORE_H
#define FRACTALCRYPTCORE_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QDir>
#include <QMap>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include "quazipfunctions.h"

namespace FractalCryptCore
{
    static constexpr char signature[8] = "fractal";

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

    void createNoize(QIODevice &iodevice, qint64 bytes);
    QByteArray generateRandomPassword();
    const QString& getCodeDescription(StatusCode statusCode);

    StatusCode writeLayer(QString containerPath, QStringList files, QStringList directories, QStringList passwords, QString newPassword);
    StatusCode readLayer(QString containerPath, QString filePath, QStringList passwords);
    StatusCode removeLayer(QString containerPath, QStringList passwords);

    StatusCode encryptFile(QIODevice &file, QStringList passwords, const QVector<qint64> &offsets);
    StatusCode decryptFile(QIODevice &file, QStringList passwords, QVector<qint64> &offsets);
    StatusCode resizeFile(QString path, QStringList passwords, qint64 newSize);

    bool encryptFilePart(QIODevice &file, qint64 pos, qint64 end, const QByteArray &key);
    bool decryptFilePart(QIODevice &file, qint64 pos, qint64 end, const QByteArray &key);

    void writeHeader(QIODevice &file, qint64 offset, quint64 size);
    StatusCode readHeader(QIODevice &file, qint64 offset, qint64 &size);
};

#endif // FRACTALCRYPTCORE_H
