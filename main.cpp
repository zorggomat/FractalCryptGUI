#include <QApplication>
#include <QDateTime>

#include "core/fractalcryptcore.h"
#include "core/quazipfunctions.h"

#include "widgets/startwindow.h"
#include "widgets/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    StartWindow startWindow;
    startWindow.show();
    return a.exec();
}
