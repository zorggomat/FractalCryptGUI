#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QRunnable>
#include <QEventLoop>
#include <QThreadPool>
#include <QCloseEvent>

namespace Ui { class ProgressDialog; }

class ProgressDialog : public QDialog
{
    Q_OBJECT

    bool finished;
public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();
    virtual void closeEvent(QCloseEvent *event);

public slots:
    void start();
    void setMaximum(int);
    void setValue(int);
    void setText(QString);
    void quit();

private:
    Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
