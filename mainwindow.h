#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

#include "createcontainerwindow.h"
#include "archivewindow.h"
#include "resizecontainerwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void show(QString path);
    CreateContainerWindow createContainerWindow;

private:
    Ui::MainWindow *ui;
    ArchiveWindow archiveWindow;
    ResizeContainerWindow resizeContainerWindow;
    QString containerPath;
    QAction *createFileAction;
    QAction *openFileAction;
    QAction *resizeAction;
    QMessageBox *confirmBox;
    QStringList getPasswords();

private slots:
    void createButtonClicked();
    void openButtonClicked();
    void resizeButtonClicked();
    void getFilesButtonClicked();
    void createLayerButtonClicked();
    void removeLayerButtonClicked();
    void passwordsTextChanged();
};
#endif // MAINWINDOW_H
