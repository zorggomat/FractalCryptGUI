#include "aes.h"
#include <QDebug>
#include <openssl/rand.h>
#include <argon2.h>
#include <QRandomGenerator>

AES::AES() : device(nullptr), pos(0), end(0), mode(Encrypt), success(false)
{
    ctx = EVP_CIPHER_CTX_new();
}

AES::~AES()
{
    if (ctx) {
        EVP_CIPHER_CTX_free(ctx);
        ctx = nullptr;
    }
}

void AES::run()
{
    if (end - pos >= 16 * 1024 * 1024)
        emit started();

    success = (mode == Encrypt ? encryptFilePart(device, pos, end, ctx) :
                                 decryptFilePart(device, pos, end, ctx));

    emit finished();
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
    if (password.isEmpty())
    {
        QRandomGenerator *random = QRandomGenerator::global();
        key.resize(32);
        random->generate(key.begin(), key.end());
    }
    else
    {
        // Using Argon2id to derive key from password
        const uint32_t t_cost = 4;       // 4 Iterations
        const uint32_t m_cost = 131072;  // Memory cost of 128MB
        const uint32_t parallelism = 1;  // Use 1 thread for hashing

        QByteArray pwdHash(32, 0);
        QByteArray salt(16, 0);
        RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()), salt.size()); // Generate a random salt

        QByteArray secret(password.toUtf8());

        int result = argon2id_hash_raw(t_cost, m_cost, parallelism,
                                       secret.data(), secret.size(),
                                       reinterpret_cast<unsigned char*>(salt.data()), salt.size(),
                                       reinterpret_cast<unsigned char*>(pwdHash.data()), pwdHash.size());
        if (result != ARGON2_OK)
        {
            qDebug() << "Argon2id hashing failed with error code: " << result;
            return;
        }

        key = pwdHash; // Use the derived hash as the encryption key
    }
}

bool AES::isSuccess() const
{
    return success;
}

bool AES::encryptFilePart(QIODevice *file, qint64 pos, qint64 end, EVP_CIPHER_CTX *ctx)
{
    unsigned char iv[16];
    RAND_bytes(iv, sizeof(iv));

    file->seek(pos);

    const qint64 bufferSize = 4096;
    qint64 size = end - pos;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(static_cast<int>(parts));

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_xts(), NULL, reinterpret_cast<const unsigned char*>(key.constData()), iv))
        return false;

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize];
    int len = 0;

    for (int i = 0; i < parts; ++i)
    {
        file->read(reinterpret_cast<char*>(buffer), bufferSize);
        if (!EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, bufferSize))
            return false;
        file->seek(pos + i * bufferSize);
        file->write(reinterpret_cast<char*>(outBuffer), len);
        emit updateValue(i);
    }
    file->seek(pos + parts * bufferSize);
    file->read(reinterpret_cast<char*>(buffer), additional);
    if (!EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, additional))
        return false;
    file->seek(pos + parts * bufferSize);
    file->write(reinterpret_cast<char*>(outBuffer), len);

    emit finished();
    return true;
}

bool AES::decryptFilePart(QIODevice *file, qint64 pos, qint64 end, EVP_CIPHER_CTX *ctx)
{
    unsigned char iv[16];
    RAND_bytes(iv, sizeof(iv));

    file->seek(pos);

    const qint64 bufferSize = 4096;
    qint64 size = end - pos;
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(static_cast<int>(parts));

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_xts(), NULL, reinterpret_cast<const unsigned char*>(key.constData()), iv))
        return false;

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize];
    int len = 0;

    for (int i = 0; i < parts; ++i)
    {
        file->read(reinterpret_cast<char*>(buffer), bufferSize);
        if (!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, bufferSize))
            return false;
        file->seek(pos + i * bufferSize);
        file->write(reinterpret_cast<char*>(outBuffer), len);
        emit updateValue(i);
    }
    file->seek(pos + parts * bufferSize);
    file->read(reinterpret_cast<char*>(buffer), additional);
    if (!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, additional))
        return false;
    file->seek(pos + parts * bufferSize);
    file->write(reinterpret_cast<char*>(outBuffer), len);

    emit finished();
    return true;
}
