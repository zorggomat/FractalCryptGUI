#ifndef AES_H
#define AES_H

#include <QObject>
#include <QRunnable>
#include <QIODevice>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

enum Mode
{
    Encrypt,
    Decrypt
};

class AES : public QObject, public QRunnable
{
    Q_OBJECT

public:
    AES();
    ~AES();
    void run() override;
    void setMode(Mode mode);
    void setIODevice(QIODevice *iodevice);
    void setRange(qint64 pos, qint64 end);
    void setPassword(QString password);
    bool isSuccess() const;

signals:
    void started();
    void setMaximumValue(int);
    void updateValue(int);
    void finished();

private:
    Mode mode;
    QIODevice *device;
    qint64 pos, end;
    QByteArray key;
    EVP_CIPHER_CTX *ctx;
    bool success;
    bool encryptFilePart(QIODevice *file, qint64 pos, qint64 end, EVP_CIPHER_CTX *ctx);
    bool decryptFilePart(QIODevice *file, qint64 pos, qint64 end, EVP_CIPHER_CTX *ctx);
};

#endif // AES_H
