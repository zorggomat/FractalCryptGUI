#ifndef RESIZECONTAINERWINDOW_H
#define RESIZECONTAINERWINDOW_H

#include <QWidget>
#include <QFile>
#include <QtMath>
#include <QMessageBox>

#include "core/fractalcryptcore.h"

namespace Ui { class ResizeContainerWindow; }

class ResizeContainerWindow : public QDialog
{
    Q_OBJECT

    QString containerPath;
    QStringList passwords;
    QMessageBox *confirmBox;
public:
    explicit ResizeContainerWindow(QWidget *parent = nullptr);
    ~ResizeContainerWindow();
    int exec(QString containerPath, QStringList passwords);

private slots:
    void resizeButtonClicked();

private:
    Ui::ResizeContainerWindow *ui;
};

#endif // RESIZECONTAINERWINDOW_H
