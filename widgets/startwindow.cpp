#include "startwindow.h"
#include "ui_startwindow.h"

StartWindow::StartWindow(QWidget *parent): QWidget(parent), ui(new Ui::StartWindow)
{
    ui->setupUi(this);
    connect(ui->createButton, &QPushButton::clicked, this, &StartWindow::createButtonClicked);
    connect(ui->openButton, &QPushButton::clicked, this, &StartWindow::openButtonClicked);
}

StartWindow::~StartWindow()
{
    delete ui;
}

void StartWindow::createButtonClicked()
{
    mainWindow.createContainerWindow.exec();
    if(mainWindow.createContainerWindow.containerSelected)
    {
        close();
        mainWindow.show(mainWindow.createContainerWindow.containerPath);
    }
}

void StartWindow::openButtonClicked()
{
    QString path = QFileDialog::getOpenFileName();
    if(path.length() != 0)
    {
        close();
        mainWindow.show(path);
    }
}
