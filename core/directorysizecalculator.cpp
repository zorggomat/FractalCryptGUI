#include "directorysizecalculator.h"

DirectorySizeCalculator::DirectorySizeCalculator(QString path): directoryPath(path)
{
}

void DirectorySizeCalculator::run()
{
    qint64 size = 0;
    QStack<QDir> directories;
    directories.push(directoryPath);

    while(!directories.empty())
    {
        QDir directory(directories.back());
        directories.pop();
        //Files
        for(QString filePath : directory.entryList(QDir::Files|QDir::System|QDir::Hidden))
            size += QFileInfo(directory, filePath).size();
        //Directories
        for(QString childDirPath : directory.entryList(QDir::AllDirs|QDir::System|QDir::Hidden|QDir::NoDotAndDotDot))
            directories.push(directory.path() + "/" + childDirPath);
    }

    emit calculated(directoryPath, size);
}
