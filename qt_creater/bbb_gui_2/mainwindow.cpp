#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QThread>

MainWindow* MainWindow::instance = nullptr;

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
     ui->light_left->setVisible(on);  //


}
