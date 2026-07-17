#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ffmpegProcess = nullptr;
    tempDir = nullptr;
    progressDialog = nullptr;
    tempPath = nullptr; 


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

    QToolButton* exportButton = new QToolButton(toolBar);
    exportButton->setStyleSheet(toolBarButtonStyle);
    exportButton->setIcon(QIcon(QString(":/%1/icons/Export_%1.svg").arg(theme)));
    exportButton->setIconSize(QSize(15,15));
    exportButton->setFixedSize(QSize(28,28));
    exportButton->setToolTip("Export Animation to MP4");


    connect(exportButton, &QToolButton::clicked, this, &MainWindow::exportAnimation);
    
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
    toolBarLayout->addWidget(exportButton);
    toolBarLayout->addSpacing(8);
    

    
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
    QObject::connect(TimelinePanel_, &Timeline::playSignal,
                     viewPort_, &viewPort::supressKeyframesSlot);
    timer_ = new QTimer(this); 
    timer_->setInterval(1000/frameRate_);
    QObject::connect(timer_, &QTimer::timeout, TimelinePanel_, &Timeline::step);
    QObject::connect(viewPort_, &viewPort::attributePanelUpdateNeeded, LPanel, &AttributePanelWidget::showObject);
    QObject::connect(viewPort_, &viewPort::pathCreated, TimelinePanel_, &Timeline::addLayer);
    QObject::connect(viewPort_, &viewPort::pathDeleted, TimelinePanel_, &Timeline::removeLayer);
    // QObject::connect(viewPort_, &viewPort::updateLayer, TimelinePanel_, &Timeline::onLayersUpdate);
    QObject::connect(viewPort_, &viewPort::updateLayer, TimelinePanel_, &Timeline::refreshLayer);

    // select path from timeline and select layer from viewport
    QObject::connect(viewPort_, &viewPort::selectLayer, TimelinePanel_, &Timeline::setSelectedLayer);
    QObject::connect(TimelinePanel_, &Timeline::setSelectedPath, [&](path* p){viewPort_->setSelectedPath(p);});

    QObject::connect(TimelinePanel_, &Timeline::frameChanged, viewPort_, &viewPort::onFrameChanged);
    QObject::connect(TimelinePanel_, &Timeline::optimize, viewPort_, &viewPort::optimize);

    selectionTool(true);
    viewPort_->createTestPath();
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
    tempPath = new path(QPointF(0, 0), nullptr, nullptr, nullptr);
    templateAttributeWidget_ = tempPath->createAttributeWidget(nullptr);
    templateAttributeWidget_->hide();
}

void MainWindow::exportAnimation()
{
    viewPort_->setSelectedPath(nullptr);
    
    QString ffmpegPath = QStandardPaths::findExecutable("ffmpeg");
    if (ffmpegPath.isEmpty()) { 
        QMessageBox::critical(this, "Error", "FFmpeg not found."); 
        return; 
    }

    QString savePath = QFileDialog::getSaveFileName(this,
        "Export Animation", QDir::homePath(), "MP4 Video (*.mp4)");
    if (savePath.isEmpty()) return;

    if (!TimelinePanel_ || !TimelinePanel_->tickBar_ || !viewPort_) {
        QMessageBox::critical(this, "Error", "Timeline or Viewport not initialized.");
        return;
    }

    int startFrame  = TimelinePanel_->tickBar_->getLBound();
    int endFrame    = TimelinePanel_->tickBar_->getRBound();
    int totalFrames = (endFrame - startFrame) + 1;

    if (totalFrames <= 0) {
        QMessageBox::warning(this, "Export", "No frames to export.");
        return;
    }

    originalFrame = *TimelinePanel_->currentFrame_;

    if (tempDir) { delete tempDir; tempDir = nullptr; }
    tempDir = new QTemporaryDir();
    if (!tempDir->isValid()) {
        QMessageBox::critical(this, "Error", "Could not create temp dir.");
        return;
    }

    if (progressDialog) { delete progressDialog; progressDialog = nullptr; }
    progressDialog = new QProgressDialog("Rendering frames...", "Cancel", 0, totalFrames, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);

    const QSize  outputSize(1920, 1080);
    QRectF canvasRect = viewPort_->canvasSceneRect();
    if (canvasRect.isEmpty()) canvasRect = QRectF(0, 0, 1280, 720);

    bool cancelled = false;
    for (int i = 0; i < totalFrames; ++i) {
        if (progressDialog->wasCanceled()) { cancelled = true; break; }

        *TimelinePanel_->currentFrame_ = startFrame + i;
        viewPort_->onFrameChanged(*TimelinePanel_->currentFrame_);

        QImage image(outputSize, QImage::Format_RGB32);
        image.fill(QColor("#1E1E1E"));

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        viewPort_->scene()->render(&painter, image.rect(), canvasRect);
        painter.end();

        QString fileName = QString("%1/frame_%2.png")
                            .arg(tempDir->path())
                            .arg(i, 6, 10, QChar('0'));
        if (!image.save(fileName)) {
            QMessageBox::critical(this, "Error", QString("Failed to save frame %1.").arg(i));
            cancelled = true;
            break;
        }

        progressDialog->setValue(i + 1);
        QCoreApplication::processEvents();
    }    

    if (cancelled) {
        progressDialog->close();
        delete tempDir; tempDir = nullptr;
        // Restore frame
        *TimelinePanel_->currentFrame_ = originalFrame;
        viewPort_->onFrameChanged(*TimelinePanel_->currentFrame_);
        return;
    }

    // 8. Encode with FFmpeg
    progressDialog->setLabelText("Encoding with FFmpeg...");
    progressDialog->setRange(0, 0); // Indeterminate

    if (ffmpegProcess) { ffmpegProcess->deleteLater(); ffmpegProcess = nullptr; }
    ffmpegProcess = new QProcess(this);

    QStringList args;
    args << "-y"
         << "-framerate" << QString::number(frameRate_)
         << "-i"         << (tempDir->path() + "/frame_%06d.png")
         << "-c:v"       << "libx264"
         << "-pix_fmt"   << "yuv420p"
         << "-crf"       << "18"
         << savePath;

    connect(ffmpegProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onExportFinished);

    ffmpegProcess->start("ffmpeg", args);

    if (!ffmpegProcess->waitForStarted(3000)) {
        QMessageBox::critical(this, "Error", "Failed to start FFmpeg.");
        onExportFinished(-1, QProcess::CrashExit);
    }
}

void MainWindow::onExportFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (progressDialog) { progressDialog->close(); delete progressDialog; progressDialog = nullptr; }

    if (exitCode == 0) {
        QMessageBox::information(this, "Export Complete", "Video exported successfully!");
    } else {
        QString errorOutput;
        if (ffmpegProcess) errorOutput = ffmpegProcess->readAllStandardError();
        QMessageBox::warning(this, "Export Failed",
            "FFmpeg failed to encode the video.\n\nDetails:\n" + errorOutput);
    }

    if (tempDir) { delete tempDir; tempDir = nullptr; }

    *TimelinePanel_->currentFrame_ = originalFrame;
    viewPort_->onFrameChanged(*TimelinePanel_->currentFrame_);

    if (ffmpegProcess) { ffmpegProcess->deleteLater(); ffmpegProcess = nullptr; }
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