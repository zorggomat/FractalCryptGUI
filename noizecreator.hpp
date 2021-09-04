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
        char buffer[4096];
        int parts = bytes / 4096;
        int additional = bytes % 4096;
        emit setMaximumValue(parts);
        for(int i = 0; i < parts; ++i)
        {
            for(int j = 0; j < 4096; ++j)
                buffer[j] = (char)random.generate();
            device->write(buffer, 4096);
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
