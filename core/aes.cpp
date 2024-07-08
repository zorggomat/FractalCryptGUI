#include "aes.h"
#include <QDebug>
#include <openssl/rand.h>
#include <argon2.h>
#include <QRandomGenerator>
#include <QIODevice>

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

void AES::setPassword(const QString& password)
{
    if (password.isEmpty())
    {
        QRandomGenerator *random = QRandomGenerator::global();
        key.resize(32);
        for (int i = 0; i < key.size(); ++i)
            key[i] = random->generate();
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

        QByteArray secret = password.toUtf8();

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
    if (!RAND_bytes(iv, sizeof(iv))) {
        qDebug() << "Failed to generate IV.";
        return false;
    }

    // Store the IV at the start of the file
    file->seek(pos);
    if (file->write(reinterpret_cast<char*>(iv), sizeof(iv)) != sizeof(iv)) {
        qDebug() << "Failed to write IV to file.";
        return false;
    }

    const qint64 bufferSize = 4096;
    qint64 size = end - pos - sizeof(iv);
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(static_cast<int>(parts));

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_xts(), NULL, reinterpret_cast<const unsigned char*>(key.constData()), iv)) {
        qDebug() << "EVP_EncryptInit_ex failed.";
        return false;
    }

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize + EVP_CIPHER_block_size(EVP_aes_256_xts())];
    int len = 0;

    for (qint64 i = 0; i < parts; ++i)
    {
        if (file->read(reinterpret_cast<char*>(buffer), bufferSize) != bufferSize) {
            qDebug() << "Failed to read from file.";
            return false;
        }

        if (!EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) {
            qDebug() << "EVP_EncryptUpdate failed.";
            return false;
        }

        file->seek(pos + sizeof(iv) + i * bufferSize);
        if (file->write(reinterpret_cast<char*>(outBuffer), len) != len) {
            qDebug() << "Failed to write to file.";
            return false;
        }

        emit updateValue(static_cast<int>(i));
    }

    if (additional > 0) {
        if (file->read(reinterpret_cast<char*>(buffer), additional) != additional) {
            qDebug() << "Failed to read from file.";
            return false;
        }

        if (!EVP_EncryptUpdate(ctx, outBuffer, &len, buffer, additional)) {
            qDebug() << "EVP_EncryptUpdate failed.";
            return false;
        }

        file->seek(pos + sizeof(iv) + parts * bufferSize);
        if (file->write(reinterpret_cast<char*>(outBuffer), len) != len) {
            qDebug() << "Failed to write to file.";
            return false;
        }
    }

    if (!EVP_EncryptFinal_ex(ctx, outBuffer, &len)) {
        qDebug() << "EVP_EncryptFinal_ex failed.";
        return false;
    }
    if (file->write(reinterpret_cast<char*>(outBuffer), len) != len) {
        qDebug() << "Failed to write final block to file.";
        return false;
    }

    return true;
}

bool AES::decryptFilePart(QIODevice *file, qint64 pos, qint64 end, EVP_CIPHER_CTX *ctx)
{
    unsigned char iv[16];

    // Read the IV from the start of the file
    file->seek(pos);
    if (file->read(reinterpret_cast<char*>(iv), sizeof(iv)) != sizeof(iv)) {
        qDebug() << "Failed to read IV from file.";
        return false;
    }

    const qint64 bufferSize = 4096;
    qint64 size = end - pos - sizeof(iv);
    qint64 parts = size / bufferSize;
    qint64 additional = size % bufferSize;
    emit setMaximumValue(static_cast<int>(parts));

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_xts(), NULL, reinterpret_cast<const unsigned char*>(key.constData()), iv)) {
        qDebug() << "EVP_DecryptInit_ex failed.";
        return false;
    }

    unsigned char buffer[bufferSize];
    unsigned char outBuffer[bufferSize + EVP_CIPHER_block_size(EVP_aes_256_xts())];
    int len = 0;
    int outLen = 0;

    for (qint64 i = 0; i < parts; ++i)
    {
        if (file->read(reinterpret_cast<char*>(buffer), bufferSize) != bufferSize) {
            qDebug() << "Failed to read from file.";
            return false;
        }

        if (!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, bufferSize)) {
            qDebug() << "EVP_DecryptUpdate failed.";
            return false;
        }

        file->seek(pos + sizeof(iv) + i * bufferSize);
        if (file->write(reinterpret_cast<char*>(outBuffer), len) != len) {
            qDebug() << "Failed to write to file.";
            return false;
        }

        emit updateValue(static_cast<int>(i));
    }

    if (additional > 0) {
        if (file->read(reinterpret_cast<char*>(buffer), additional) != additional) {
            qDebug() << "Failed to read from file.";
            return false;
        }

        if (!EVP_DecryptUpdate(ctx, outBuffer, &len, buffer, additional)) {
            qDebug() << "EVP_DecryptUpdate failed.";
            return false;
        }

        file->seek(pos + sizeof(iv) + parts * bufferSize);
        if (file->write(reinterpret_cast<char*>(outBuffer), len) != len) {
            qDebug() << "Failed to write to file.";
            return false;
        }
    }

    if (!EVP_DecryptFinal_ex(ctx, outBuffer, &outLen)) {
        qDebug() << "EVP_DecryptFinal_ex failed.";
        return false;
    }
    if (file->write(reinterpret_cast<char*>(outBuffer), outLen) != outLen) {
        qDebug() << "Failed to write final block to file.";
        return false;
    }

    return true;
}
