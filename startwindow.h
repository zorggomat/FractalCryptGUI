#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QWidget>
#include <QFileDialog>

#include "createcontainerwindow.h"
#include <mainwindow.h>

namespace Ui { class StartWindow; }

class StartWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StartWindow(QWidget *parent = nullptr);
    ~StartWindow();

private:
    Ui::StartWindow *ui;
    MainWindow mainWindow;

private slots:
    void createButtonClicked();
    void openButtonClicked();
};

#endif // STARTWINDOW_H
