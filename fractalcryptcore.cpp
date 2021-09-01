#include "fractalcryptcore.h"

using namespace FractalCryptCore;

void FractalCryptCore::createNoize(QIODevice &iodevice, qint64 bytes)
{
    QDataStream stream(&iodevice);
    QRandomGenerator random = QRandomGenerator::securelySeeded();
    for(qint64 i = 0; i < bytes; ++i)
        stream << (qint8)random.generate();
}

bool FractalCryptCore::encryptFilePart(QIODevice &file, qint64 pos, qint64 end, const QByteArray &key)
{
    QByteArray hashedKey = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(key, QCryptographicHash::Md5);

    file.seek(pos);

    qint64 bufferSize = 4096;
    qint64 size = end - pos;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize];

    int len = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) return false;
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)hashedKey.data(), (unsigned char*)iv.data())) return false;

    for(int i = 0; i < parts; ++i)
    {
        file.read((char*)buffer, bufferSize);
        if(1 != EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) return false;
        file.seek(pos + i * bufferSize);
        file.write((char*)outBuffer, bufferSize);
    }
    file.read((char*)buffer, additional);
    if(1 != EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, additional)) return false;
    file.seek(pos + parts * bufferSize);
    file.write((char*)outBuffer, additional);

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool FractalCryptCore::decryptFilePart(QIODevice &file, qint64 pos, qint64 end, const QByteArray &key)
{
    QByteArray hashedKey = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(key, QCryptographicHash::Md5);

    file.seek(pos);

    qint64 size = end - pos;
    qint64 bufferSize = 4096;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;

    unsigned char buffer[bufferSize+16];
    unsigned char outBuffer[bufferSize+16];

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) return false;
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)hashedKey.data(), (unsigned char*)iv.data())) return false;

    int len = 0;

    file.read((char*)buffer, 16);
    if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, 16)) return false;
    for(int i = 0; i < parts; ++i)
    {
        file.seek(pos + 16 + i * bufferSize);
        file.read((char*)buffer, bufferSize);
        if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) return false;
        file.seek(pos + i * bufferSize);
        file.write((char*)outBuffer, bufferSize);
    }
    file.seek(pos + 16 + parts * bufferSize);
    file.read((char*)buffer, additional);
    if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, additional)) return false;
    file.seek(pos + parts * bufferSize);
    file.write((char*)outBuffer, len);

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

QByteArray FractalCryptCore::generateRandomPassword()
{
    QRandomGenerator *random = QRandomGenerator::global();
    QByteArray result(32, Qt::Initialization::Uninitialized);
    random->generate(result.begin(), result.end());
    return result;
}

StatusCode FractalCryptCore::encryptFile(QIODevice &file, QStringList passwords, const QVector<qint64> &offsets)
{
    for(int i = passwords.size()-1; i >= 0; --i)
    {
        QByteArray key = passwords[i].toUtf8();
        if(!encryptFilePart(file, offsets[i], file.size(), key))
            return EncryptionError;
    }
    return OK;
}

StatusCode FractalCryptCore::decryptFile(QIODevice &file, QStringList passwords, QVector<qint64> &offsets)
{
    qint64 pos = 0;
    qint64 fileSize = file.size();
    offsets.push_back(0);

    for(int i = 0; i < passwords.size(); ++i)
    {
        //Decrypt layer
        QByteArray key = passwords.at(i).toUtf8();
        if(!decryptFilePart(file, pos, fileSize, key))
            return EncryptionError;

        //Read header
        qint64 layerSize;
        if(readHeader(file, pos, layerSize) != OK)
        {
            passwords.erase(passwords.begin()+i+1, passwords.end());
            encryptFile(file, passwords, offsets);
            return SignatureInvalid;
        }
        if(pos + layerSize + 16 > fileSize)
        {
            passwords.erase(passwords.begin()+i+1, passwords.end());
            encryptFile(file, passwords, offsets);
            return HeaderSizeInvalid;
        }

        //Next layer index
        if(layerSize % 16 != 0)
            layerSize += 16 - (layerSize % 16);
        pos += 16 + layerSize;
        offsets.push_back(pos);
    }

    return OK;
}

StatusCode FractalCryptCore::resizeFile(QString path, QStringList passwords, qint64 newSize)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    qint64 currentSize = file.size();

    if(newSize == currentSize) return OK;

    QVector<qint64> offsets;
    StatusCode r = decryptFile(file, passwords, offsets);
    if(r != OK) return r;

    if(offsets.back() > newSize)
    {
        offsets.pop_back();
        encryptFile(file, passwords, offsets);
        return NewSizeTooSmall;
    }
    if(newSize < currentSize)
    {
        file.resize(newSize);
    }
    else
    {
        file.seek(currentSize);
        createNoize(file, newSize - currentSize);
    }
    offsets.pop_back();
    return encryptFile(file, passwords, offsets);;
}

StatusCode FractalCryptCore::writeLayer(QString containerPath, QStringList files, QStringList directories, QStringList passwords, QString newPassword)
{
    //Open, decrypt and prepare container
    QFile container(containerPath);
    if(!container.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    qint64 containerSize = container.size();

    QVector<qint64> offsets;
    StatusCode r = decryptFile(container, passwords, offsets);
    if(r != OK) return r;
    container.close();

    //Write file
    qint64 size;
    if(!QuazipFunctions::writeZip(containerPath, offsets.back() + 16, files, directories, size))
    {
        offsets.pop_back();
        encryptFile(container, passwords, offsets);
        return ZipError;
    }
    if(container.size() > containerSize)
    {
        container.resize(containerSize);
        container.open(QIODevice::ReadWrite);
        //createNoize(container, container.size() - offsets.back());
        passwords.push_back(generateRandomPassword());
        encryptFile(container, passwords, offsets);
        return NotEnoughSpace;
    }

    //Write header
    container.open(QIODevice::ReadWrite);
    writeHeader(container, offsets.back(), size);

    //Encrypt and close container
    passwords.push_back(newPassword);
    return encryptFile(container, passwords, offsets);
}

StatusCode FractalCryptCore::readLayer(QString containerPath, QString filePath, QStringList passwords)
{
    //Open, decrypt and prepare container
    QFile container(containerPath);
    if(!container.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    QVector<qint64> offsets;
    StatusCode r = decryptFile(container, passwords, offsets);
    if (r != OK) return r;
    offsets.pop_back();

    //Get size
    qint64 layerSize;
    if(readHeader(container, offsets.back(), layerSize) != OK)
    {
        encryptFile(container, passwords, offsets);
        return SignatureInvalid;
    }
    if(offsets.back() + layerSize + 16 > container.size())
    {
        encryptFile(container, passwords, offsets);
        return HeaderSizeInvalid;
    }

    //Read archive and encrypt container
    bool success = QuazipFunctions::readZip(container, offsets.back()+16, offsets.back()+16+layerSize, filePath);
    encryptFile(container, passwords, offsets);
    return success ? OK : ZipError;
}

StatusCode FractalCryptCore::removeLayer(QString containerPath, QStringList passwords)
{
    //Open, decrypt and prepare container
    QFile container(containerPath);
    if(!container.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    QVector<qint64> offsets;
    StatusCode r = decryptFile(container, passwords, offsets);
    if(r != OK) return r;
    container.seek(offsets.back());

    //Write noize and encrypt
    createNoize(container, container.size() - offsets.back());
    passwords.push_back(generateRandomPassword());
    encryptFile(container, passwords, offsets);
    return OK;
}

const QString& FractalCryptCore::getCodeDescription(StatusCode statusCode)
{
    static QMap<StatusCode, QString> map({
                                             {OK,                   "Operation successfully completed"},
                                             {ContainerUnavailable, "Unable to open the container"},
                                             {SignatureInvalid,     "Wrong password"},
                                             {HeaderSizeInvalid,    "The container is damaged or created incorrectly"},
                                             {NotEnoughSpace,       "Not enough space in the container"},
                                             {ZipError,             "Error processing zip archive"},
                                             {EncryptionError,      "Encryption error"},
                                             {NewSizeTooSmall,      "The new size is smaller than the size of the data in the container"}
                                         });
    return map[statusCode];
}

void FractalCryptCore::writeHeader(QIODevice &file, qint64 offset, quint64 size)
{
    file.seek(offset);
    file.write(signature, 8);
    char sizeBytes[8];
    for(int i = 0; i < 8; ++i)
        sizeBytes[i] = (size >> 8 * (7 - i));
    file.write(sizeBytes, 8);
}

StatusCode FractalCryptCore::readHeader(QIODevice &file, qint64 offset, qint64 &size)
{
    file.seek(offset);
    quint8 headerSignature[8];
    quint8 headerSize[8];
    file.read((char*)headerSignature, 8);
    file.read((char*)headerSize, 8);
    if(strcmp((const char*)headerSignature, signature) != 0)
        return SignatureInvalid;
    size = 0;
    for(int j = 0; j < 8; ++j)
        size = (size << 8) + headerSize[j];
    return OK;
}
