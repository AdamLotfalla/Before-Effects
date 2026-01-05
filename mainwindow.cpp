#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString theme = "Dark";
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    
    QWidget* LPanel = new QWidget(this);
    
    QWidget* toolBar = new QWidget(this);
    toolBar->setAutoFillBackground(true);
    toolBar->setPalette(QPalette(QColor("#2E2E2E")));
    toolBar->setFixedHeight(30);

    QHBoxLayout* toolBarLayout = new QHBoxLayout(toolBar); 
    toolBar->setLayout(toolBarLayout);

    bezierPen = new QToolButton(toolBar);
    bezierPen->setStyleSheet(toolBarButtonStyle);
    bezierPen->setIcon(QIcon(QString(":/icons/%1/bezierPen_%1.svg").arg(theme)));
    bezierPen->setIconSize(QSize(20,20));
    bezierPen->setFixedSize(QSize(28,28));
    bezierPen->setCheckable(true);
    bezierPen->setChecked(false);

    connect(bezierPen, &QToolButton::pressed,
            this, &MainWindow::bezierTool);

    toolBarLayout->setContentsMargins(0,0,0,0);
    toolBarLayout->setSpacing(5);
    toolBarLayout->addWidget(bezierPen, 0, Qt::AlignVCenter);
    toolBarLayout->addStretch();
    

    ViewPort_ = new viewPort(this);
    
    Timeline* TimelinePanel = new Timeline(this, &frameRate_);
    TimelinePanel->setAutoFillBackground(true);
    // TimelinePanel->setPalette(QPalette(QColor(100,100,200)));

    
    QSplitter* timelineSplitter = new QSplitter(Qt::Vertical, centralWidget);
    timelineSplitter->addWidget(ViewPort_);
    timelineSplitter->addWidget(TimelinePanel);

    QVBoxLayout* verticalLayout = new QVBoxLayout(centralWidget);
    verticalLayout->addWidget(toolBar);
    verticalLayout->addWidget(timelineSplitter);
    verticalLayout->setContentsMargins(0,0,0,0);
    verticalLayout->setSpacing(0);

    QWidget* RPanel = new QWidget(this);
    RPanel->setAutoFillBackground(true);
    RPanel->setPalette(QPalette(QColor("#333333")));
    RPanel->setLayout(verticalLayout);

    QSplitter* sideSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    sideSplitter->addWidget(LPanel);
    sideSplitter->addWidget(RPanel);


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

void MainWindow::bezierTool()
{
    bezierPen->setChecked(enableBezier_);    
    enableBezier_ = !enableBezier_;
    ViewPort_->enableBezier(enableBezier_);
}

void MainWindow::startTimer(bool playing){
    if(playing){
        timer_->start();
    }
    else{
        timer_->stop();
    }
}