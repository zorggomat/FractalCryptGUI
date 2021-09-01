#include <QApplication>
#include <QDateTime>

#include "startwindow.h"
#include "mainwindow.h"
#include "fractalcryptcore.h"
#include "quazipfunctions.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StartWindow startWindow;
    startWindow.show();
    return a.exec();
}
