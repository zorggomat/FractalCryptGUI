#include "archivewindow.h"
#include "ui_archivewindow.h"

ArchiveWindow::ArchiveWindow(QWidget *parent): QDialog(parent), ui(new Ui::ArchiveWindow)
{
    ui->setupUi(this);

    connect(ui->addFileButton, &QPushButton::clicked, this, &ArchiveWindow::addFileButton);
    connect(ui->addDirectoryButton, &QPushButton::clicked, this, &ArchiveWindow::addDirectoryButton);
    connect(ui->removeButton, &QPushButton::clicked, this, &ArchiveWindow::removeButton);
    connect(ui->clearButton, &QPushButton::clicked, this, &ArchiveWindow::clearButton);
    connect(ui->createLayerButton, &QPushButton::clicked, this, &ArchiveWindow::createLayerButton);
    connect(ui->visibleCheckBox, &QCheckBox::stateChanged, this, &ArchiveWindow::visibleCheckBoxStateChanged);

    ui->tableWidget->setColumnWidth(0, 404);
    ui->tableWidget->setColumnWidth(1, 80);
    ui->tableWidget->setColumnWidth(2, 80);

    confirmBox = new QMessageBox(QMessageBox::Warning,
                                 "Warning",
                                 "Are you sure you want to create an empty layer?",
                                 QMessageBox::Yes | QMessageBox::No,
                                 this);
}

ArchiveWindow::~ArchiveWindow()
{
    delete ui;
}

void ArchiveWindow::exec(QString containerPath, QStringList passwords)
{
    ui->passwordLineEdit->clear();
    this->containerPath = containerPath;
    this->passwords = passwords;
    QDialog::exec();
}

void ArchiveWindow::addFileButton()
{
    QStringList paths = QFileDialog::getOpenFileNames();
    if(paths.empty()) return;

    int rowNumber = ui->tableWidget->rowCount();
    for(QString path : paths)
    {
        bool skip = false;
        for(int i = 0; i < rowNumber; ++i)
            if(path == ui->tableWidget->item(i, 0)->text())
            {
                QMessageBox::warning(nullptr, "Warning", "File " + path + " is already in the list");
                skip = true;
                break;
            }
        if (skip) continue;

        QFile file(path);
        ui->tableWidget->insertRow(rowNumber);
        ui->tableWidget->setItem(rowNumber, 0, new QTableWidgetItem(path));
        ui->tableWidget->setItem(rowNumber, 1, new QTableWidgetItem("File"));
        ui->tableWidget->setItem(rowNumber, 2, new QTableWidgetItem(getFormattedSize(file.size())));
        for(int i = 0; i < 3; ++i)
            ui->tableWidget->item(rowNumber, i)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    }
}

void ArchiveWindow::addDirectoryButton()
{
    QString path = QFileDialog::getExistingDirectory();
    if(path.length() == 0) return;

    int rowNumber = ui->tableWidget->rowCount();
    for(int i = 0; i < rowNumber; ++i)
        if(path == ui->tableWidget->item(i, 0)->text())
        {
            QMessageBox::warning(nullptr, "Warning", "Directory " + path + " is already in the list");
            return;
        }

    ui->tableWidget->insertRow(rowNumber);
    ui->tableWidget->setItem(rowNumber, 0, new QTableWidgetItem(path));
    ui->tableWidget->setItem(rowNumber, 1, new QTableWidgetItem("Directory"));

    QThreadPool *pool = QThreadPool::globalInstance();
    DirectorySizeCalculator *sizeCalculator = new DirectorySizeCalculator(path);
    connect(sizeCalculator, &DirectorySizeCalculator::calculated, this, [this](QString directory, qint64 size){
        for(int i = 0; i < ui->tableWidget->rowCount(); ++i)
            if(directory == ui->tableWidget->item(i, 0)->text())
            {
                ui->tableWidget->setItem(i, 2, new QTableWidgetItem(getFormattedSize(size)));
                break;
            }
        delete QObject::sender();
    });
    pool->start(sizeCalculator);
}

void ArchiveWindow::removeButton()
{
    auto selectedItems = ui->tableWidget->selectedItems();
    for(auto item : selectedItems)
        ui->tableWidget->removeRow(ui->tableWidget->row(item));
}

void ArchiveWindow::clearButton()
{
    ui->tableWidget->setRowCount(0);
}

void ArchiveWindow::createLayerButton()
{
    QString newPassword =  ui->passwordLineEdit->text();

    if(newPassword.isEmpty())
    {
        QMessageBox::warning(nullptr, "Error", "Password can't be empty");
        return;
    }

    if(ui->tableWidget->rowCount() == 0)
        if(confirmBox->exec() != QMessageBox::Yes)
            return;

    QStringList files;
    QStringList directories;
    for(int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        //(ui->tableWidget->item(i, 1)->text() == "Directory" ? directories : files).push_back((ui->tableWidget->item(i, 0))->text());
        QString path = (ui->tableWidget->item(i, 0))->text();
        if (ui->tableWidget->item(i, 1)->text() == "Directory")
            directories.push_back(path);
        else files.push_back(path);
    }

    FractalCryptCore::StatusCode r = FractalCryptCore::Instance().writeLayer(containerPath, files, directories, passwords, newPassword);
    if(r == FractalCryptCore::OK)
    {
        QMessageBox::about(nullptr, "Result", FractalCryptCore::getCodeDescription(r));
        close();
        ui->tableWidget->setRowCount(0);
    }
    else QMessageBox::critical(nullptr, "Error", FractalCryptCore::getCodeDescription(r));
}

void ArchiveWindow::visibleCheckBoxStateChanged(int newState)
{
    ui->passwordLineEdit->setEchoMode((newState == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password));
}

QString ArchiveWindow::getFormattedSize(qint64 size)
{
    static const QStringList units = {"Bytes", "KB", "MB", "GB", "TB"};
    int i;
    double outputSize = size;
    for(i = 0; i < units.size() - 1; i++)
    {
        if(outputSize < 1024) break;
        outputSize = outputSize / 1024;
    }
    return QString("%0 %1").arg(outputSize, 0, 'f', (i ? 2 : 0)).arg(units[i]);
}
