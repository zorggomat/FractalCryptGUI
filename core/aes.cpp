#include "aes.h"

AES::AES()
{
    ctx = EVP_CIPHER_CTX_new();
}

AES::~AES()
{
    EVP_CIPHER_CTX_free(ctx);
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
    success = (mode == Encrypt ? encryptFilePart(device, pos, end, &key, ctx) :
                                 decryptFilePart(device, pos, end, &key, ctx));
    emit finished();
}

bool AES::encryptFilePart(QIODevice *file, qint64 pos, qint64 end, const QByteArray *password, EVP_CIPHER_CTX *ctx)
{
    unsigned char key[512];
    unsigned char iv[16];
    PKCS5_PBKDF2_HMAC_SHA1(password->data(), password->size(), nullptr, 0, 10000, 512, key);
    PKCS5_PBKDF2_HMAC_SHA1(password->data(), password->size(), nullptr, 0, 25000, 16, iv);

    file->seek(pos);

    const qint64 bufferSize = 4096;
    qint64 size = end - pos;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(parts);

    if(!ctx) return false;
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_xts(), NULL, key, NULL)) return false;

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize];
    int len = 0;

    for(int i = 0; i < parts; ++i)
    {
        file->read((char*)buffer, bufferSize);
        if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, iv)) return false;
        if(1 != EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) return false;
        file->seek(pos + i * bufferSize);
        file->write((char*)outBuffer, bufferSize);
        emit updateValue(i);
    }
    file->read((char*)buffer, additional);
    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, iv)) return false;
    if(1 != EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, additional)) return false;
    file->seek(pos + parts * bufferSize);
    file->write((char*)outBuffer, additional);

    emit finished();
    return true;
}

bool AES::decryptFilePart(QIODevice *file, qint64 pos, qint64 end, const QByteArray *password, EVP_CIPHER_CTX *ctx)
{
    unsigned char key[512];
    unsigned char iv[16];
    PKCS5_PBKDF2_HMAC_SHA1(password->data(), password->size(), nullptr, 0, 10000, 512, key);
    PKCS5_PBKDF2_HMAC_SHA1(password->data(), password->size(), nullptr, 0, 25000, 16, iv);

    file->seek(pos);

    const qint64 bufferSize = 4096;
    qint64 size = end - pos;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(parts);

    if(!ctx) return false;
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_xts(), NULL, key, NULL)) return false;

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize];
    int len = 0;

    for(int i = 0; i < parts; ++i)
    {
        file->read((char*)buffer, bufferSize);
        if(!EVP_DecryptInit_ex(ctx, NULL, NULL, NULL, iv)) return false;
        if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) return false;
        file->seek(pos + i * bufferSize);
        file->write((char*)outBuffer, bufferSize);
        emit updateValue(i);
    }
    file->seek(pos + parts * bufferSize);
    file->read((char*)buffer, additional);
    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, NULL, iv)) return false;
    if(!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, additional)) return false;
    file->seek(pos + parts * bufferSize);
    file->write((char*)outBuffer, len);

    emit finished();
    return true;
}

bool AES::isSuccess()
{
    return success;
}
