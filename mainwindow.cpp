#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBoxLayout>
#include <QWidget>
#include <QPalette>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    QPushButton* button = new QPushButton("Click Me!", this);
    button->show();    
    
    QWidget* LPanel = new QWidget(this);
    LPanel->setAutoFillBackground(true);
    LPanel->setPalette(QPalette(QColor(100,200,100)));
    
    QWidget* RPanel = new QWidget(this);
    RPanel->setAutoFillBackground(true);
    RPanel->setPalette(QPalette(QColor(200,100,100)));
    
    updatelabel = new QLabel("Text", this);
    updatelabel->move(200,200);
    
    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    splitter->addWidget(LPanel);
    splitter->addWidget(RPanel);
    
    QList<int> sizes;
    sizes << 200 << 600;  // Left: 200px, Right: 600px
    splitter->setSizes(sizes);
    
    LPanel->setMinimumWidth(50); 
    RPanel->setMinimumWidth(100);  
    
    splitter->setHandleWidth(1); 
    splitter->setStyleSheet("QSplitter::handle { background-color: gray; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(splitter);
    mainLayout->setContentsMargins(0, 0, 0, 0); 
    
    
    QObject::connect(button, &QPushButton::clicked, this, &MainWindow::changeText);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeText(){
    updatelabel->setText("Clicked :<");
}