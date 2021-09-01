#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createFileAction = new QAction("Create", this);
    openFileAction = new QAction("Open", this);
    resizeAction = new QAction("Resize", this);
    menuBar()->addActions({createFileAction, openFileAction, resizeAction});

    confirmBox = new QMessageBox(QMessageBox::Warning,
                                 "Warning",
                                 "Are you sure you want to overwrite part of the container?",
                                 QMessageBox::Yes | QMessageBox::No,
                                 this);

    connect(createFileAction, &QAction::triggered, this, &MainWindow::createButtonClicked);
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openButtonClicked);
    connect(resizeAction, &QAction::triggered, this, &MainWindow::resizeButtonClicked);

    connect(ui->getFilesButton, &QPushButton::clicked, this, &MainWindow::getFilesButtonClicked);
    connect(ui->createLayerButton, &QPushButton::clicked, this, &MainWindow::createLayerButtonClicked);
    connect(ui->removeLayerButton, &QPushButton::clicked, this, &MainWindow::removeLayerButtonClicked);

    connect(ui->passwordsTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::passwordsTextChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::show(QString path)
{
    containerPath = path;
    ui->currentPathLabel->setText("Current container: " + path);
    QMainWindow::show();
}

void MainWindow::createButtonClicked()
{
    createContainerWindow.exec();
    if(createContainerWindow.containerSelected)
        show(createContainerWindow.containerPath);
}

void MainWindow::openButtonClicked()
{
    QString path = QFileDialog::getOpenFileName();
    if(path.isEmpty() == false)
        show(path);
}

void MainWindow::resizeButtonClicked()
{
    resizeContainerWindow.exec(containerPath, getPasswords());
}

void MainWindow::getFilesButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory();
    if(path.isEmpty()) return;

    FractalCryptCore::StatusCode r = FractalCryptCore::readLayer(containerPath, path, getPasswords());

    if(r == FractalCryptCore::OK)
        QMessageBox::about(nullptr, "Result", FractalCryptCore::getCodeDescription(r));
    else QMessageBox::critical(nullptr, "Error", FractalCryptCore::getCodeDescription(r));
}

void MainWindow::createLayerButtonClicked()
{
    archiveWindow.exec(containerPath, getPasswords());
}

void MainWindow::removeLayerButtonClicked()
{
    if(confirmBox->exec() != QMessageBox::Yes)
        return;

    FractalCryptCore::StatusCode r = FractalCryptCore::removeLayer(containerPath, getPasswords());

    if(r == FractalCryptCore::OK)
        QMessageBox::about(nullptr, "Result", FractalCryptCore::getCodeDescription(r));
    else QMessageBox::critical(nullptr, "Error", FractalCryptCore::getCodeDescription(r));
}

QStringList MainWindow::getPasswords()
{
    return ui->passwordsTextEdit->toPlainText().simplified().split(QChar(' '), QString::SkipEmptyParts);
}

void MainWindow::passwordsTextChanged()
{
    int passwordsCount = getPasswords().length();
    switch (passwordsCount)
    {
        case 0:
        ui->getFilesButton->setText("Get files");
        ui->getFilesButton->setEnabled(false);
        ui->createLayerButton->setText("Create 1-st layer");
        ui->removeLayerButton->setText("Remove all layers");
        break;

        case 1:
        ui->getFilesButton->setText("Get 1-st layer files");
        ui->getFilesButton->setEnabled(true);
        ui->createLayerButton->setText("Create 2-nd layer");
        ui->removeLayerButton->setText("Remove 2-nd layer");
        break;

        case 2:
        ui->getFilesButton->setText("Get 2-nd layer files");
        ui->getFilesButton->setEnabled(true);
        ui->createLayerButton->setText("Create 3-rd layer");
        ui->removeLayerButton->setText("Remove 3-rd layer");
        break;

        case 3:
        ui->getFilesButton->setText("Get 3-rd layer files");
        ui->getFilesButton->setEnabled(true);
        ui->createLayerButton->setText("Create 4-th layer");
        ui->removeLayerButton->setText("Remove 4-th layer");
        break;

        default:
        ui->getFilesButton->setText("Get " + QString::number(passwordsCount) + "-th layer files");
        ui->getFilesButton->setEnabled(true);
        ui->createLayerButton->setText("Create " + QString::number(passwordsCount+1) + "-th layer");
        ui->removeLayerButton->setText("Remove " + QString::number(passwordsCount+1) + "-th layer");
        break;
    }
}
