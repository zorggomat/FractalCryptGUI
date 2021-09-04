#include "aes.h"

AES::AES()
{
}

void AES::setMode(Mode mode)
{
    this->mode = mode;
}

void AES::setIODevice(QIODevice *iodevice)
{
    device = iodevice;
}

void AES::setRange(qint64 pos, qint64 end)
{
    this->pos = pos;
    this->end = end;
}

void AES::setPassword(QString password)
{
    if(password.isEmpty())
    {
        QRandomGenerator *random = QRandomGenerator::global();
        key.resize(32);
        random->generate(key.begin(), key.end());
    }
    else key = password.toUtf8();
}

void AES::run()
{
    if(end - pos >= 16 * 1024 * 1024) emit started();
    mode == Encrypt ? encryptFilePart(device, pos, end, &key) :
                      decryptFilePart(device, pos, end, &key) ;
    emit finished();
}

bool AES::encryptFilePart(QIODevice *file, qint64 pos, qint64 end, const QByteArray *key)
{
    QByteArray hashedKey = QCryptographicHash::hash(*key, QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(*key, QCryptographicHash::Md5);

    file->seek(pos);

    qint64 bufferSize = 4096;
    qint64 size = end - pos;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(parts);

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize];

    int len = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) return false;
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)hashedKey.data(), (unsigned char*)iv.data())) return false;

    for(int i = 0; i < parts; ++i)
    {
        file->read((char*)buffer, bufferSize);
        if(1 != EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) return false;
        file->seek(pos + i * bufferSize);
        file->write((char*)outBuffer, bufferSize);
        emit updateValue(i);
    }
    file->read((char*)buffer, additional);
    if(1 != EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, additional)) return false;
    file->seek(pos + parts * bufferSize);
    file->write((char*)outBuffer, additional);

    EVP_CIPHER_CTX_free(ctx);
    emit finished();
    return true;
}

bool AES::decryptFilePart(QIODevice *file, qint64 pos, qint64 end, const QByteArray *key)
{
    QByteArray hashedKey = QCryptographicHash::hash(*key, QCryptographicHash::Sha256);
    QByteArray iv = QCryptographicHash::hash(*key, QCryptographicHash::Md5);

    file->seek(pos);

    qint64 size = end - pos;
    qint64 bufferSize = 4096;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(parts);

    unsigned char buffer[bufferSize+16];
    unsigned char outBuffer[bufferSize+16];

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) return false;
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)hashedKey.data(), (unsigned char*)iv.data())) return false;

    int len = 0;

    file->read((char*)buffer, 16);
    if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, 16)) return false;
    for(int i = 0; i < parts; ++i)
    {
        file->seek(pos + 16 + i * bufferSize);
        file->read((char*)buffer, bufferSize);
        if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) return false;
        file->seek(pos + i * bufferSize);
        file->write((char*)outBuffer, bufferSize);
        emit updateValue(i);
    }
    file->seek(pos + 16 + parts * bufferSize);
    file->read((char*)buffer, additional);
    if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, additional)) return false;
    file->seek(pos + parts * bufferSize);
    file->write((char*)outBuffer, len);

    EVP_CIPHER_CTX_free(ctx);
    emit finished();
    return true;
}
