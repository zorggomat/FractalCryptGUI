#include "resizecontainerwindow.h"
#include "ui_resizecontainerwindow.h"

ResizeContainerWindow::ResizeContainerWindow(QWidget *parent) : QDialog(parent), ui(new Ui::ResizeContainerWindow)
{
    ui->setupUi(this);
    connect(ui->resizeButton, &QPushButton::clicked, this, &ResizeContainerWindow::resizeButtonClicked);
    confirmBox = new QMessageBox(QMessageBox::Warning,
                                 "Warning",
                                 "If you did not enter all passwords, some of your data will be lost.",
                                 QMessageBox::Ok | QMessageBox::Cancel,
                                 this);
}

ResizeContainerWindow::~ResizeContainerWindow()
{
    delete ui;
}

int ResizeContainerWindow::exec(QString containerPath, QStringList passwords)
{
    this->containerPath = containerPath;
    this->passwords = passwords;

    ui->warningLabel->setText("Warning! You have entered passwords for " + QString::number(passwords.size()) + " layers.");
    ui->sizeLineEdit->clear();

    qint64 size = QFile(containerPath).size();
    int i;
    for(i = 0; i < 3; ++i)
        if((size /= 1024) % 1024 != 0) break;
    ui->comboBox->setCurrentIndex(i);
    ui->currentSizeLabel->setText("Current size: " + QString::number(size) + " " + ui->comboBox->currentText());

    return QDialog::exec();
}

void ResizeContainerWindow::resizeButtonClicked()
{
    if(confirmBox->exec() != QMessageBox::Ok) return;

    quint64 newSize = ui->sizeLineEdit->text().toULongLong();
    if(newSize <= 0) return;
    newSize *= qPow(1024, ui->comboBox->currentIndex() + 1);

    FractalCryptCore::StatusCode r = FractalCryptCore::resizeFile(containerPath, passwords, newSize);

    if(r == FractalCryptCore::OK)
    {
        QMessageBox::about(nullptr, "Result", FractalCryptCore::getCodeDescription(r));
        close();
    }
    else QMessageBox::critical(nullptr, "Error", FractalCryptCore::getCodeDescription(r));
}
