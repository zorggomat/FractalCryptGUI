#ifndef NOIZECREATOR_H
#define NOIZECREATOR_H

#include <QObject>
#include <QRunnable>
#include <QIODevice>
#include <QDataStream>
#include <QRandomGenerator>

class NoizeCreator : public QObject, public QRunnable
{
    Q_OBJECT

    QIODevice *device;
    qint64 bytes;
public:
    NoizeCreator() {}
    void setDevice(QIODevice *iodevice)
    {
        device = iodevice;
    }
    void setNumberOfBytes(qint64 numberOfBytes)
    {
        bytes = numberOfBytes;
    }
    void run()
    {
        if(bytes >= 16 * 1024 * 1024) emit started();
        QRandomGenerator random = QRandomGenerator::securelySeeded();
        const int bufferSize = 4096;
        char buffer[bufferSize];
        int parts = bytes / bufferSize;
        int additional = bytes % bufferSize;
        emit setMaximumValue(parts);
        for(int i = 0; i < parts; ++i)
        {
            random.fillRange((uint*)buffer, bufferSize / sizeof(uint));
            device->write(buffer, bufferSize);
            emit updateValue(i);
        }
        for(int j = 0; j < additional; ++j)
            buffer[j] = (char)random.generate();
        device->write(buffer, additional);
        emit finished();
    }

signals:
    void started();
    void setMaximumValue(int);
    void updateValue(int);
    void finished();
};

#endif // NOIZECREATOR_H
