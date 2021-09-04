#ifndef ARCHIVEWINDOW_H
#define ARCHIVEWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QThreadPool>

#include "core/fractalcryptcore.h"
#include "core/directorysizecalculator.h"

namespace Ui { class ArchiveWindow; }

class ArchiveWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ArchiveWindow(QWidget *parent = nullptr);
    ~ArchiveWindow();
    void exec(QString containerPath, QStringList passwords);

private:
    Ui::ArchiveWindow *ui;
    qint64 getDirectorySize(QString path);
    QString getFormattedSize(qint64 size);
    QString containerPath;
    QStringList passwords;
    QMessageBox *confirmBox;

private slots:
    void addFileButton();
    void addDirectoryButton();
    void removeButton();
    void clearButton();
    void createLayerButton();
    void visibleCheckBoxStateChanged(int newState);
};

#endif // ARCHIVEWINDOW_H
