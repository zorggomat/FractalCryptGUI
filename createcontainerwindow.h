#ifndef CREATECONTAINERWINDOW_H
#define CREATECONTAINERWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QtMath>

#include "fractalcryptcore.h"

namespace Ui { class CreateContainerWindow; }

class CreateContainerWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CreateContainerWindow(QWidget *parent = nullptr);
    ~CreateContainerWindow();
    int exec();
    bool containerSelected;
    QString containerPath;

private:
    Ui::CreateContainerWindow *ui;

private slots:
    void createButtonClicked();
    void selectPathButtonClicked();
};

#endif // CREATECONTAINERWINDOW_H
