#ifndef AES_H
#define AES_H

#include <QObject>
#include <QRunnable>
#include <QIODevice>
#include <QByteArray>
#include <openssl/evp.h>

enum Mode
{
    Encrypt,
    Decrypt
};

class AES : public QObject, public QRunnable
{
    Q_OBJECT

    Mode mode;
    QIODevice *device;
    qint64 pos, end;
    QByteArray key, salt, iv;
    EVP_CIPHER_CTX *ctx;
    bool success;

    bool processFile(QIODevice *file, qint64 pos, qint64 end, const QByteArray &key, Mode mode);

public:
    AES();
    ~AES();
    void run() override;
    void setMode(Mode mode);
    void setIODevice(QIODevice *iodevice);
    void setRange(qint64 pos, qint64 end);
    void setPassword(const QString &password);
    bool isSuccess() const;

signals:
    void started();
    void setMaximumValue(int);
    void updateValue(int);
    void finished();
};

#endif // AES_H
