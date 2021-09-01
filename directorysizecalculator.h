#ifndef DIRECTORYSIZECALCULATOR_H
#define DIRECTORYSIZECALCULATOR_H

#include <QObject>
#include <QRunnable>
#include <QThread>
#include <QDir>
#include <QStack>

class DirectorySizeCalculator : public QObject, public QRunnable
{
    Q_OBJECT

public:
    QString directoryPath;
    DirectorySizeCalculator(QString path);
    void run();

signals:
    void calculated(QString directory, qint64 size);
};

#endif // DIRECTORYSIZECALCULATOR_H
