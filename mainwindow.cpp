#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBoxLayout>
#include <QWidget>
#include <QPalette>
#include <QSplitter>
#include "timeline.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    
    QWidget* LPanel = new QWidget(this);
    LPanel->setAutoFillBackground(true);
    LPanel->setPalette(QPalette(QColor(100,200,100)));
    
    QWidget* RPanel = new QWidget(this);
    RPanel->setAutoFillBackground(true);
    RPanel->setPalette(QPalette(QColor(200,100,100)));
    
    Timeline* TimelinePanel = new Timeline(this, &frameRate_);
    TimelinePanel->setAutoFillBackground(true);
    // TimelinePanel->setPalette(QPalette(QColor(100,100,200)));
    
    QSplitter* timelineSplitter = new QSplitter(Qt::Vertical, centralWidget);
    timelineSplitter->addWidget(RPanel);
    timelineSplitter->addWidget(TimelinePanel);

    QSplitter* sideSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    sideSplitter->addWidget(LPanel);
    sideSplitter->addWidget(timelineSplitter);


    sideSplitter->setSizes({200,600});
    timelineSplitter->setSizes({500,300});
    
    LPanel->setMinimumWidth(50); 
    RPanel->setMinimumWidth(100);
    TimelinePanel->setMinimumHeight(200);

    timelineSplitter->setHandleWidth(1);
    sideSplitter->setHandleWidth(1); 

    
    timelineSplitter->setStyleSheet("QSplitter::handle { background-color: gray; }");
    sideSplitter->setStyleSheet("QSplitter::handle { background-color: gray; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(sideSplitter);
    mainLayout->setContentsMargins(0, 0, 0, 0); 


    QObject::connect(TimelinePanel, &Timeline::playSignal,
                     this, &MainWindow::startTimer);
    
    timer_ = new QTimer(this); 
    timer_->setInterval(1000/frameRate_);
    QObject::connect(timer_, &QTimer::timeout, TimelinePanel, &Timeline::step);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startTimer(bool playing){
    if(playing){
        timer_->start();
    }
    else{
        timer_->stop();
    }
}