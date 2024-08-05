#include "aes.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include "argon2.h"
#include <QDebug>

constexpr int IV_SIZE = 16;
constexpr int SALT_SIZE = 16;
constexpr int KEY_SIZE = 32;

AES::AES() 
    : ctx(EVP_CIPHER_CTX_new()), 
      success(false), 
      key(KEY_SIZE, 0) 
{
    if (!ctx) {
        qCritical() << "Failed to create EVP_CIPHER_CTX";
    }
}

AES::~AES() {
    if (ctx) {
        EVP_CIPHER_CTX_free(ctx);
    }
}

void AES::setMode(Mode mode) {
    this->mode = mode;
}

void AES::setIODevice(QIODevice *iodevice) {
    device = iodevice;
}

void AES::setRange(qint64 pos, qint64 end) {
    this->pos = pos;
    this->end = end;
}

void AES::setPassword(const QString &password) {
    if (!password.isEmpty()) {
        QByteArray secret = password.toUtf8();

        // Hash the password using SHA-512
        unsigned char hash[SHA512_DIGEST_LENGTH];
        SHA512(reinterpret_cast<const unsigned char*>(secret.constData()), secret.size(), hash);

        // Use the first 16 bytes of the hash as the salt
        salt = QByteArray(reinterpret_cast<const char*>(hash), SALT_SIZE);

        // Use the first 16 bytes of the hash as the IV
        iv = QByteArray(reinterpret_cast<const char*>(hash), SALT_SIZE);

        argon2id_hash_raw(4, 131072, 1, secret.data(), secret.size(), 
                          reinterpret_cast<unsigned char*>(salt.data()), SALT_SIZE,
                          reinterpret_cast<unsigned char*>(key.data()), KEY_SIZE);
    } else {
        RAND_bytes(reinterpret_cast<unsigned char*>(key.data()), KEY_SIZE);
        RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), IV_SIZE);
    }
}

void AES::run() {
    if (!ctx) {
        qCritical() << "Cipher context is not initialized.";
        return;
    }

    if (end - pos >= 16 * 1024 * 1024) {
        emit started();
    }
    
    success = processFile(device, pos, end, key, mode);

    emit finished();
}

bool AES::processFile(QIODevice *file, qint64 pos, qint64 end, const QByteArray &key, Mode mode) {
    if (!ctx) {
        qCritical() << "Cipher context is not initialized.";
        return false;
    }

    file->seek(pos);

    if (!EVP_CipherInit_ex(ctx, EVP_aes_256_xts(), nullptr, 
                          reinterpret_cast<const unsigned char*>(key.constData()), 
                          reinterpret_cast<const unsigned char*>(iv.constData()), 
                          (mode == Encrypt))) 
    {
        qCritical() << "EVP_CipherInit_ex failed.";
        return false;
    }

    constexpr int BUFFER_SIZE = 4096;
    
    QByteArray buffer(BUFFER_SIZE, Qt::Uninitialized);
    QByteArray outBuffer(BUFFER_SIZE + EVP_CIPHER_block_size(EVP_aes_256_xts()), Qt::Uninitialized);
    int len = 0;

    qint64 size = end - pos;
    qint64 parts = size / BUFFER_SIZE;
    qint64 additional = size % BUFFER_SIZE;
    emit setMaximumValue(parts);

    for (qint64 i = 0; i < parts; ++i) {
        if (file->read(buffer.data(), BUFFER_SIZE) != BUFFER_SIZE) {
            return false;
        }

        if (!EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char*>(outBuffer.data()), &len, 
                              reinterpret_cast<unsigned char*>(buffer.data()), BUFFER_SIZE)) 
        {
            qCritical() << "EVP_CipherUpdate failed.";
            return false;
        }

        file->seek(pos + i * BUFFER_SIZE);

        if (file->write(outBuffer.data(), len) != len) {
            return false;
        }

        emit updateValue(i);
    }

    if (additional > 0) {
        if (file->read(buffer.data(), additional) != additional) {
            return false;
        }

        if (!EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char*>(outBuffer.data()), &len, 
                              reinterpret_cast<unsigned char*>(buffer.data()), additional)) 
        {
            qCritical() << "EVP_CipherUpdate failed.";
            return false;
        }

        file->seek(pos + parts * BUFFER_SIZE);

        if (file->write(outBuffer.data(), len) != len) {
            return false;
        }
    }

    if (!EVP_CipherFinal_ex(ctx, reinterpret_cast<unsigned char*>(outBuffer.data()), &len)) {
        qCritical() << "EVP_CipherFinal_ex failed.";
        return false;
    }

    if (file->write(outBuffer.data(), len) != len) {
        return false;
    }
    
    return true;
}

bool AES::isSuccess() const {
    return success;
}
