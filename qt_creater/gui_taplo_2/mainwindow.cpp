#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    instance = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setLightLeftVisible(bool on)
{
    ui->label_3->setVisible(on);  //
    ui->label_4->setVisible(on);

}
