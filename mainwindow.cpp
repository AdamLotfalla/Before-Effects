#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    preCreateAttributeWidgets();

    QString theme = "Dark";
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    
    AttributePanelWidget* LPanel = new AttributePanelWidget(this);
    
    QWidget* toolBar = new QWidget(this);
    toolBar->setAutoFillBackground(true);
    toolBar->setPalette(QPalette(QColor("#2E2E2E")));
    toolBar->setFixedHeight(30);

    QHBoxLayout* toolBarLayout = new QHBoxLayout(toolBar); 
    toolBar->setLayout(toolBarLayout);

    bezierPen_ = new QToolButton(toolBar);
    bezierPen_->setStyleSheet(toolBarButtonStyle);
    bezierPen_->setIcon(QIcon(QString(":/%1/icons/bezierPen_%1.svg").arg(theme)));
    bezierPen_->setIconSize(QSize(20,20));
    bezierPen_->setFixedSize(QSize(28,28));
    bezierPen_->setCheckable(true);
    bezierPen_->setChecked(false);
    
    nodeTool_ = new QToolButton(toolBar);
    nodeTool_->setStyleSheet(toolBarButtonStyle);
    nodeTool_->setIcon(QIcon(QString(":/%1/icons/nodeTool_%1.svg").arg(theme)));
    nodeTool_->setIconSize(QSize(20,20));
    nodeTool_->setFixedSize(QSize(28,28));
    nodeTool_->setCheckable(true);
    nodeTool_->setChecked(false);
    
    selectionTool_ = new QToolButton(toolBar);
    selectionTool_->setStyleSheet(toolBarButtonStyle);
    selectionTool_->setIcon(QIcon(QString(":/%1/icons/selectionTool_%1.svg").arg(theme)));
    selectionTool_->setIconSize(QSize(20,20));
    selectionTool_->setFixedSize(QSize(28,28));
    selectionTool_->setCheckable(true);
    selectionTool_->setChecked(false);
    
    connect(selectionTool_, &QToolButton::toggled,
            this, &MainWindow::selectionTool);

    connect(nodeTool_, &QToolButton::toggled,
            this, &MainWindow::nodeTool);

    connect(bezierPen_, &QToolButton::toggled,
            this, &MainWindow::bezierTool);
    
    toolBarLayout->setContentsMargins(0,0,0,0);
    toolBarLayout->setSpacing(5);
    toolBarLayout->addWidget(selectionTool_, 0, Qt::AlignVCenter); 
    toolBarLayout->addWidget(nodeTool_, 0, Qt::AlignVCenter); 
    toolBarLayout->addWidget(bezierPen_, 0, Qt::AlignVCenter);
    toolBarLayout->addStretch();
    

    
    TimelinePanel_ = new Timeline(this, &frameRate_);
    TimelinePanel_->setAutoFillBackground(true);
    // TimelinePanel_->setPalette(QPalette(QColor(100,100,200)));

    viewPort_ = new viewPort(this, TimelinePanel_->currentFrame_);

    
    QSplitter* timelineSplitter = new QSplitter(Qt::Vertical, centralWidget);
    timelineSplitter->addWidget(viewPort_);
    timelineSplitter->addWidget(TimelinePanel_);

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
    TimelinePanel_->setMinimumHeight(200);

    timelineSplitter->setHandleWidth(1);
    sideSplitter->setHandleWidth(1); 

    
    timelineSplitter->setStyleSheet("QSplitter::handle { background-color: gray; }");
    sideSplitter->setStyleSheet("QSplitter::handle { background-color: gray; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(sideSplitter);
    mainLayout->setContentsMargins(0, 0, 0, 0); 


    QObject::connect(TimelinePanel_, &Timeline::playSignal,
                     this, &MainWindow::startTimer);
    
    timer_ = new QTimer(this); 
    timer_->setInterval(1000/frameRate_);
    QObject::connect(timer_, &QTimer::timeout, TimelinePanel_, &Timeline::step);
    QObject::connect(viewPort_, &viewPort::attributePanelUpdateNeeded, LPanel, &AttributePanelWidget::showObject);
    QObject::connect(viewPort_, &viewPort::pathCreated, TimelinePanel_, &Timeline::addLayer);
    QObject::connect(viewPort_, &viewPort::pathDeleted, TimelinePanel_, &Timeline::removeLayer);
    QObject::connect(viewPort_, &viewPort::layerInfoUpdated, TimelinePanel_, &Timeline::updateLayers);
    QObject::connect(viewPort_, &viewPort::layerSelected, TimelinePanel_, &Timeline::updateSelectedLayer);
    QObject::connect(TimelinePanel_, &Timeline::frameChanged, viewPort_, &viewPort::onFrameChanged);
    QObject::connect(TimelinePanel_, &Timeline::optimize, viewPort_, &viewPort::optimize);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete templateAttributeWidget_;
    delete tempPath;
}

void MainWindow::bezierTool(bool checked)
{
    bezierPen_->setChecked(checked);    
    enableBezier_ = checked;
    viewPort_->enableBezierTool(checked);
    
    if(enableNodeTool_ && checked)
        nodeTool(!checked);
    if(enableSelectionTool_ && checked)
        selectionTool(!checked);
    
    
    viewPort_->setPathEditingMode(checked);
}

void MainWindow::preCreateAttributeWidgets()
{
    path* tempPath = new path(QPointF(0, 0), nullptr, nullptr, nullptr);
    templateAttributeWidget_ = tempPath->createAttributeWidget(nullptr);
    templateAttributeWidget_->hide();
}

void MainWindow::selectionTool(bool checked)
{
    selectionTool_->setChecked(checked);
    enableSelectionTool_ = checked;
    viewPort_->enableSelectionTool(checked);

    if(enableBezier_ && checked)
        bezierTool(!checked);
    if(enableNodeTool_ && checked)
        nodeTool(!checked);

    viewPort_->setPathEditingMode(false);
}

void MainWindow::nodeTool(bool checked)
{
    nodeTool_->setChecked(checked);
    enableNodeTool_ = checked;
    viewPort_->enableNodeTool(checked);
    
    if(enableBezier_ && checked)
        bezierTool(!checked);
    if(enableSelectionTool_ && checked)
        selectionTool(!checked);
    
    viewPort_->setPathEditingMode(checked);
}

void MainWindow::startTimer(bool playing){
    if(playing){
        timer_->start();
    }
    else{
        timer_->stop();
    }
}