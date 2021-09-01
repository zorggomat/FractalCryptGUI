#include "createcontainerwindow.h"
#include "ui_createcontainerwindow.h"

CreateContainerWindow::CreateContainerWindow(QWidget *parent): QDialog(parent), ui(new Ui::CreateContainerWindow)
{
    ui->setupUi(this);
    connect(ui->createButton, &QPushButton::clicked, this, &CreateContainerWindow::createButtonClicked);
    connect(ui->selectPathButton, &QPushButton::clicked, this, &CreateContainerWindow::selectPathButtonClicked);

    ui->sizeLineEdit->setValidator(new QIntValidator);
}

CreateContainerWindow::~CreateContainerWindow()
{
    delete ui;
}

int CreateContainerWindow::exec()
{
    containerSelected = false;
    return QDialog::exec();
}

void CreateContainerWindow::createButtonClicked()
{
    quint64 bytes = ui->sizeLineEdit->text().toULongLong();
    if(bytes == 0) return;
    bytes *= qPow(1024, ui->comboBox->currentIndex() + 1);

    QString path = ui->pathLineEdit->text();
    QFile file(path);
    if(!file.open(QIODevice::Truncate | QIODevice::ReadWrite))
    {
        QMessageBox::critical(this, "Error", "Unable to open the file");
        return;
    }
    FractalCryptCore::createNoize(file, bytes);
    QByteArray key = FractalCryptCore::generateRandomPassword();
    if(!FractalCryptCore::encryptFilePart(file, 0, bytes, key))
    {
        QMessageBox::critical(this, "Error", "Encryption error");
        return;
    }

    ui->pathLineEdit->clear();
    ui->sizeLineEdit->clear();
    containerSelected = true;
    containerPath = path;
    close();
}

void CreateContainerWindow::selectPathButtonClicked()
{
    QString path = QFileDialog::getSaveFileName();
    if(path.length() != 0)
        ui->pathLineEdit->setText(path);
}
