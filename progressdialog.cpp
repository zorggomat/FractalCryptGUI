#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::start()
{
    ui->progressBar->setValue(0);
    finished = false;
    QDialog::exec();
}

void ProgressDialog::setMaximum(int value)
{
    ui->progressBar->setMaximum(value);
}

void ProgressDialog::setValue(int value)
{
    ui->progressBar->setValue(value);
}

void ProgressDialog::setText(QString text)
{
    ui->label->setText(text);
}

void ProgressDialog::quit()
{
    finished = true;
    close();
}

void ProgressDialog::closeEvent(QCloseEvent *event)
{
    finished ? event->accept() : event->ignore();
}
