#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPushButton* button = new QPushButton("Click Me!", this);
    button->show();
    
    updatelabel = new QLabel("Text", this);
    updatelabel->move(200,200);
    QObject::connect(button, &QPushButton::clicked, this, &MainWindow::changeText);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeText(){
    updatelabel->setText("Clicked :<");
}