#include "timeline.h"

Layer::Layer(QWidget* parent, path *p, int height) : QWidget(parent)
{
    height_ = height;
    relatedPath_ = p;
    this->setFixedHeight(height_);

    setAutoFillBackground(false);
}

void Layer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content

    if(isSelected_)
        painter.setBrush(QColor("#55504a"));
    else
        painter.setBrush(QColor("#444444"));
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(0,0, this->width(), height_, 2, 2);
    painter.setPen(QPen("#FFFFFF"));
    painter.drawText(20,0, this->width(), height_, Qt::AlignVCenter, relatedPath_->name_);
}

TimeIndicator::TimeIndicator(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
}

void TimeIndicator::MoveCenter(int x, int y)
{
    this->move(x - IndicatorWidth_/2, y);
    update();
}

void TimeIndicator::paintEvent(QPaintEvent *event)
{
    int centerX = IndicatorWidth_/2;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content

    QPen pen(QColor("#7BC7B0"), 2, Qt::SolidLine);
    QBrush brush(QColor("#7BC7B0"));

    painter.setPen(pen);
    painter.setBrush(brush);


    QPainterPath path;
    path.moveTo(centerX -5,0);
    path.lineTo(centerX -5,3);
    path.lineTo(centerX +0,8);
    path.lineTo(centerX +5,3);
    path.lineTo(centerX +5,0);
    path.closeSubpath();

    painter.drawPath(path);

    painter.drawLine(centerX, 8, centerX, this->height());
}

// ----------------------- INDICATOR BAR -----------------------
TimeIndicatorBar::TimeIndicatorBar(QWidget* parent, int *frameRate, int *frameWidth, int *frameCount, int width, int fullHeight, int *currentFrame){
    //TEMPORARY
    currentFrame_ = currentFrame;
    
    frameRate_ = frameRate;
    frameWidth_ = frameWidth;
    frameCount_ = frameCount;

    fullHeight_ = fullHeight;
    fullWidth_ = width;

    RBoundFrame_ = *frameCount_;
    LBoundFrame_ = 0;

    resize(fullWidth_, fullHeight_);
    setMouseTracking(true);

    timeIndicator_ = new TimeIndicator(this);
    timeIndicator_->show();
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    // timeIndicator->setAttribute(Qt::WA_AlwaysStackOnTop);
    
    LBound = new QWidget(this);
    RBound = new QWidget(this);

    LBound->setFixedSize(boundHandleThickness, tickLayerHeight_);
    RBound->setFixedSize(boundHandleThickness, tickLayerHeight_);

    LBound->setAttribute(Qt::WA_TranslucentBackground);
    LBound->setStyleSheet("background: transparent;");

    RBound->setAttribute(Qt::WA_TranslucentBackground);
    RBound->setStyleSheet("background: transparent;");

    offset_ = (this->width() - *frameCount_ * *frameWidth_) / 2;


    
    connect(this, &TimeIndicatorBar::tickBarClickedSignal,
            this, &TimeIndicatorBar::onTickBarClick);
    connect(this, &TimeIndicatorBar::tickBarUnClickedSignal,
            this, &TimeIndicatorBar::onTickBarUnClick);
    connect(this, &TimeIndicatorBar::LBoundClickedSignal,
            this, &TimeIndicatorBar::onLBoundClick);
    connect(this, &TimeIndicatorBar::LBoundUnClickedSignal,
            this, &TimeIndicatorBar::onLBoundUnClick);
    connect(this, &TimeIndicatorBar::RBoundClickedSignal,
            this, &TimeIndicatorBar::onRBoundClick);
    connect(this, &TimeIndicatorBar::RBoundUnClickedSignal,
            this, &TimeIndicatorBar::onRBoundUnClick);
    
    
    update();
}

void TimeIndicatorBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;
    // fullHeight = this->height();
    fullWidth_ = *frameCount_ * *frameWidth_;
    offset_ = (width() - *frameCount_ * *frameWidth_) / 2;

    switch (int((float)*frameWidth_/5))
    {
    case 1:
        tickInterval_ = 20;
        break;
    case 2 ... 3:
        tickInterval_ = 10;
        break;
    case 4 ... 7:
        tickInterval_ = 5;
        break;
    default:
        tickInterval_ = 1;
        break;
    };

    pen.setWidth(0);
    painter.setPen(pen);

    QRect fullRect(0, 0, this->width(), tickLayerHeight_);
    painter.fillRect(fullRect, QColor("#1E1E1E"));

    QRect leftMargin(0, tickLayerHeight_, offset_, height() - tickLayerHeight_);
    QRect rightMargin(offset_ + *frameCount_ * *frameWidth_, tickLayerHeight_, width() - offset_ - (*frameCount_ * *frameWidth_), height() - tickLayerHeight_); 
        //added this long expression for width in case the total margin is not even, so division will be short by 1
    painter.fillRect(leftMargin, QColor("#202020"));
    painter.fillRect(rightMargin, QColor("#202020"));

    QRect LOutsideBound(offset_ , tickLayerHeight_ + boundLayerHeight_, LBoundFrame_ * *frameWidth_, height());
    QRect ROutsideBound(offset_ + RBoundFrame_ * *frameWidth_, tickLayerHeight_ + boundLayerHeight_, (*frameCount_ - RBoundFrame_) * *frameWidth_, height());

    painter.fillRect(LOutsideBound, QColor("#252525"));
    painter.fillRect(ROutsideBound, QColor("#252525"));
    
    for(int i = 0; i< *frameCount_; i++){
        QRect rect(offset_ + i* *frameWidth_, 0, *frameWidth_, tickLayerHeight_);
        
        if(i % 2){
            painter.fillRect(rect, QColor("#2D2D2D"));
        }
        else{
            painter.fillRect(rect, QColor("#292929"));
        }
    }
    pen.setWidth(3);
    pen.setColor("#999999");
    painter.setPen(pen);

    for(int i = 0; i <= *frameCount_; i++){
        if(i % tickInterval_ == 0){
            painter.drawLine(offset_ + i* *frameWidth_, tickLayerHeight_ * 7/8, offset_ + i* *frameWidth_, tickLayerHeight_ - 3); // -3 to compensate for pen width
            painter.drawText(QPoint(offset_ + i* *frameWidth_ + 1, tickLayerHeight_ * 11/16), QString::number(i));
        }
    }

    pen.setColor("#444444");
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0, tickLayerHeight_, this->width(), tickLayerHeight_);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content

    QRect boundLayerRect(
        offset_ + *frameWidth_ * LBoundFrame_,
        tickLayerHeight_,
        *frameWidth_ * (RBoundFrame_ - LBoundFrame_),
        boundLayerHeight_
    );

    painter.fillRect(boundLayerRect, QColor("#444444"));

    pen.setColor(QColor("#52786D"));
    QBrush brush(QColor("#52786D"));
    painter.setPen(pen);
    painter.setBrush(brush);


    LBound->move(offset_ + LBoundFrame_ * *frameWidth_ - boundHandleThickness, tickLayerHeight_);
    RBound->move(offset_ + RBoundFrame_ * *frameWidth_, tickLayerHeight_);
    painter.drawRoundedRect(RBound->x(), RBound->y(), boundHandleThickness, boundLayerHeight_, 1, 1);
    painter.drawRoundedRect(LBound->x(), LBound->y(), boundHandleThickness, boundLayerHeight_, 1, 1);
    
    timeIndicator_->MoveCenter(offset_ + *currentFrame_ * *frameWidth_);
}

int TimeIndicatorBar::getRBound()
{
    return RBoundFrame_;
}

int TimeIndicatorBar::getLBound()
{
    return LBoundFrame_;
}

void TimeIndicatorBar::addLayer(path* p, QWidget* parent)
{
    Layer* addedLayer = new Layer(parent, p, layerHeight_);
    layers_.push_back(addedLayer);
}

Layer *TimeIndicatorBar::getActiveLayer()
{
    return activeLayer_;
}

Layer* TimeIndicatorBar::accessLayer(int index)
{
    if(index < layers_.size())
        return layers_[index];
    else
        return nullptr;
}

void TimeIndicatorBar::setActiveLayer(Layer *l)
{
    if(activeLayer_ != nullptr){
        activeLayer_->isSelected_ = false;
    }
    activeLayer_ = l;

    if(activeLayer_ != nullptr){
        activeLayer_->isSelected_ = true;
    }
}

int TimeIndicatorBar::getLayerCount()
{
    return layers_.size();
}

int TimeIndicatorBar::getTopBarHeight()
{
    return tickLayerHeight_ + boundLayerHeight_;
}

void TimeIndicatorBar::onTickBarClick(QPoint pos)
{
    barClicked_ = true;
    repaint();
    int frame = round((pos.x() - offset_) / (float)*frameWidth_);

    if(frame > RBoundFrame_){
        *currentFrame_ = RBoundFrame_;
    }
    else if(frame < LBoundFrame_){
        *currentFrame_ = LBoundFrame_;
    }
    else{
        *currentFrame_ = frame;
    }
        
    timeIndicator_->MoveCenter(*currentFrame_ * (*frameWidth_));
}

void TimeIndicatorBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        int x = event->pos().x();
        int y = event->pos().y();

        int minLBound = offset_ + LBoundFrame_ * *frameWidth_ - boundHandleThickness;
        int maxLBound = offset_ + LBoundFrame_ * *frameWidth_;

        int minRBound = offset_ + RBoundFrame_ * *frameWidth_;
        int maxRBound = offset_ + RBoundFrame_ * *frameWidth_ + boundHandleThickness;

        if(y < tickLayerHeight_){
            emit tickBarClickedSignal(event->pos());
        }
        else if(y < tickLayerHeight_ + boundLayerHeight_){
            if(minLBound < x && x < maxLBound){
                emit LBoundClickedSignal(event->pos());
            }
            else if(minRBound < x && x < maxRBound){
                emit RBoundClickedSignal(event->pos());
            }
        }
            
    } 
    QWidget::mousePressEvent(event);
}

void TimeIndicatorBar::mouseMoveEvent(QMouseEvent *event)
{
    int frame = round((event->pos().x() - offset_) / (float)*frameWidth_);
    frame = qBound(0, frame, *frameCount_);
    if(barClicked_){
        *currentFrame_ = frame;
        // The paint event does the movement for me
    }
    else if(LBoundClicked_){
        if(frame >= RBoundFrame_){
            LBoundFrame_ = RBoundFrame_ - 1;
        }
        else{
            LBoundFrame_ = frame;
        }
    }
    else if(RBoundClicked_){
        if (frame <= LBoundFrame_){
            RBoundFrame_ = LBoundFrame_ + 1;
        }
        else{
            RBoundFrame_ = frame;
        }
    }
    update();
}

void TimeIndicatorBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        emit tickBarUnClickedSignal();
        emit LBoundUnClickedSignal();
        emit RBoundUnClickedSignal();
    }
}

void TimeIndicatorBar::onTickBarUnClick()
{
    barClicked_ = false;
}

void TimeIndicatorBar::onLBoundClick(QPoint pos)
{
    LBoundClicked_ = true;
}

void TimeIndicatorBar::onLBoundUnClick()
{
    LBoundClicked_ = false;
}

void TimeIndicatorBar::onRBoundClick(QPoint pos)
{
    RBoundClicked_ = true;
}

void TimeIndicatorBar::onRBoundUnClick()
{
    RBoundClicked_ = false;
}

void TimeIndicatorBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    timeIndicator_->resize(
        timeIndicator_->width(),
        height()
    );
}

// ----------------------- TIMELINE -----------------------
Timeline::Timeline (QWidget *parent, int *frameRate) : QWidget(parent){

    frameRate_ = frameRate;
    theme_= "Dark";
    this->setPalette(QPalette(QColor("#333333")));

    QWidget* toolbar = new QWidget(this);
    toolbar->setFixedHeight(25);
    toolbar->setAutoFillBackground(true);
    toolbar->setPalette(QPalette(QColor("#2E2E2E")));

    QToolButton* zoomInButton = new QToolButton(toolbar);
    QToolButton* zoomOutButton = new QToolButton(toolbar);

    zoomInButton->setFixedSize(21,21);
    zoomOutButton->setFixedSize(21,21);

    zoomInButton->setIcon(QIcon(QString(":/%1/icons/zoomIn_%1.svg").arg(theme_)));
    zoomOutButton->setIcon(QIcon(QString(":/%1/icons/zoomOut_%1.svg").arg(theme_)));
    zoomInButton->setIconSize(QSize(-1, 15));
    zoomOutButton->setIconSize(QSize(-1,15));
    zoomInButton->setToolTip("Zoom In Timeline");
    zoomOutButton->setToolTip("Zoom Out Timeline");
    zoomInButton->setStyleSheet(buttonStyle);
    zoomOutButton->setStyleSheet(buttonStyle);
        

    playButton = new QToolButton(toolbar);
    playButton->setFixedSize(21,21);
    playButton->setIcon(QIcon(QString(":/%1/icons/play_%1.svg").arg(theme_)));
    playButton->setIconSize(QSize(-1,15));
    playButton->setToolTip("Play");
    playButton->setStyleSheet(buttonStyle);

    QToolButton* goToStartButton = new QToolButton(toolbar);
    goToStartButton->setFixedSize(21,21);
    goToStartButton->setIcon(QIcon(QString(":/%1/icons/goToStart_%1.svg").arg(theme_)));
    goToStartButton->setIconSize(QSize(-1,15));
    goToStartButton->setToolTip("Go To Playback Start");
    goToStartButton->setStyleSheet(buttonStyle);

    QToolButton* goToEndButton = new QToolButton(toolbar);
    goToEndButton->setFixedSize(21,21);
    goToEndButton->setIcon(QIcon(QString(":/%1/icons/goToEnd_%1.svg").arg(theme_)));
    goToEndButton->setIconSize(QSize(-1,15));
    goToEndButton->setToolTip("Go To Playback End");
    goToEndButton->setStyleSheet(buttonStyle);

    QToolButton* nextFrameButton = new QToolButton(toolbar);
    nextFrameButton->setFixedSize(21,21);
    nextFrameButton->setIcon(QIcon(QString(":/%1/icons/nextFrame_%1.svg").arg(theme_)));
    nextFrameButton->setIconSize(QSize(-1,15));
    nextFrameButton->setToolTip("Go To Next Frame");
    nextFrameButton->setStyleSheet(buttonStyle);


    QToolButton* previousFrameButton = new QToolButton(toolbar);
    previousFrameButton->setFixedSize(21,21);
    previousFrameButton->setIcon(QIcon(QString(":/%1/icons/previousFrame_%1.svg").arg(theme_)));
    previousFrameButton->setIconSize(QSize(-1,15));
    previousFrameButton->setToolTip("Go To Previous Frame");
    previousFrameButton->setStyleSheet(buttonStyle);
    
    QSlider* zoomSlider = new QSlider(toolbar);
    zoomSlider->setRange(1, 20); //zoom amount from 1x to 20x
    zoomSlider->setSingleStep(1);
    zoomSlider->setFixedWidth(100);
    zoomSlider->setOrientation(Qt::Horizontal);
    zoomSlider->setStyleSheet(sliderStyle);

    QObject::connect(zoomSlider, &QSlider::valueChanged, this, &Timeline::zoomSliderChanged);
    
    QObject::connect(zoomInButton, &QToolButton::pressed, [zoomSlider](){
        if (zoomSlider->value() < zoomSlider->maximum()) {
            zoomSlider->setValue(zoomSlider->value() + 1);
        }
    });

    QObject::connect(zoomOutButton, &QToolButton::pressed, [zoomSlider](){
        if (zoomSlider->value() > zoomSlider->minimum()) {
            zoomSlider->setValue(zoomSlider->value() - 1);
        }
    });

    QObject::connect(playButton, &QToolButton::pressed,
                     this, &Timeline::playButtonClickEvent);

    QObject::connect(goToStartButton, &QToolButton::pressed, [this](){
        currentFrame_ = timeIndicatorBar->getLBound();
        timeIndicatorBar->update();
    });

    QObject::connect(goToEndButton, &QToolButton::pressed, [this](){
        currentFrame_ = timeIndicatorBar->getRBound();
        timeIndicatorBar->update();
    });

    QObject::connect(nextFrameButton, &QToolButton::pressed, [this](){
        if(currentFrame_ < timeIndicatorBar->getRBound())
            currentFrame_ ++;
        timeIndicatorBar->update();
    });

    QObject::connect(previousFrameButton, &QToolButton::pressed, [this](){
        if(currentFrame_ > timeIndicatorBar->getLBound())
            currentFrame_ --;
        timeIndicatorBar->update();
    });

    QHBoxLayout* toolBarLayout = new QHBoxLayout(toolbar);
    toolBarLayout->setContentsMargins(0,0,0,0);
    toolBarLayout->setSpacing(5);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(goToStartButton);
    toolBarLayout->addWidget(previousFrameButton);
    toolBarLayout->addWidget(playButton);
    toolBarLayout->addWidget(nextFrameButton);
    toolBarLayout->addWidget(goToEndButton);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(zoomOutButton);
    toolBarLayout->addWidget(zoomSlider, 0, Qt::AlignVCenter);
    toolBarLayout->addWidget(zoomInButton);

    timelineSplitterLayout_ = new QHBoxLayout();
    layerPanel_ = new QWidget(this);
    layerPanel_->setMinimumWidth(175);
    layersLayout_ = new QVBoxLayout(layerPanel_);
    layerPanel_->setLayout(layersLayout_);

    scroller = new QScrollArea(this);

    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0,0,0,0);
    verticalLayout->setSpacing(0);
    verticalLayout->addWidget(toolbar);
    verticalLayout->addLayout(timelineSplitterLayout_);

    timelineSplitterLayout_->addWidget(layerPanel_);
    timelineSplitterLayout_->addWidget(scroller);

    frameWidth_ = 5 * zoomSlider->value();
    timeIndicatorBar = new TimeIndicatorBar(scroller, frameRate_, &frameWidth_, &frameCount_, frameWidth_ * frameCount_ + 235, scroller->viewport()->height(), &currentFrame_);
    timeIndicatorBar->show();

    layersLayout_->setContentsMargins(0,0,0,0);
    layersLayout_->setSpacing(1);
    layersLayout_->addSpacing(timeIndicatorBar->getTopBarHeight()); //causes the program not to launch !?
    layersLayout_->addStretch();

    scroller->setWidget(timeIndicatorBar);
    // scroller->setWidgetResizable(true);
}

void Timeline::step()
{   
    if(currentFrame_ < timeIndicatorBar->getLBound() || currentFrame_ == timeIndicatorBar->getRBound())
        currentFrame_ = timeIndicatorBar->getLBound();
    else if(currentFrame_ < timeIndicatorBar->getRBound()) 
        currentFrame_ ++;
    else
        currentFrame_ = timeIndicatorBar->getRBound();
    update();
}

void Timeline::setTheme(QString theme)
{
    theme_ = theme;
}

void Timeline::addLayer(path* p)
{
    if(p != nullptr){
        timeIndicatorBar->addLayer(p, layerPanel_);
        Layer* newLayer = timeIndicatorBar->accessLayer(timeIndicatorBar->getLayerCount() - 1);
        layersLayout_->insertWidget(1, newLayer);
        newLayer->show();
    }
}

void Timeline::updateLayers()
{
    if (layersLayout_) {
       QLayoutItem *item;
        while ((item = layersLayout_->takeAt(0)) != nullptr) {
            if (QWidget *widget = item->widget()) {
                widget->hide();
            }
            delete item;
        }
        // delete layersLayout_;
    }
    else{
        layersLayout_ = new QVBoxLayout(layerPanel_);
        layersLayout_->setContentsMargins(0,0,0,0);
        layersLayout_->setSpacing(0);
    }

    layersLayout_->addSpacing(timeIndicatorBar->getTopBarHeight());
    for(int i = timeIndicatorBar->getLayerCount() - 1; i >= 0; i--){
        Layer* temporaryLayer = timeIndicatorBar->accessLayer(i);
        if(temporaryLayer){
            if(temporaryLayer == timeIndicatorBar->getActiveLayer()){
                temporaryLayer->isSelected_ = true;
            }
            layersLayout_->addWidget(temporaryLayer);
        }
    }

    
    layersLayout_->addStretch();
    for(int i = timeIndicatorBar->getLayerCount() - 1; i >= 0; i--){
        timeIndicatorBar->accessLayer(i)->show();
    }
    layerPanel_->update();
}

void Timeline::zoomSliderChanged(int value)
{
    int newFrameWidth = value * 5;
    if (frameWidth_ == newFrameWidth)
        return;

    // --- compute OLD content position ---
    int oldOffset =
        (timeIndicatorBar->width() - frameCount_ * frameWidth_) / 2;

    int oldContentX =
        oldOffset + currentFrame_ * frameWidth_;

    int oldScroll =
        scroller->horizontalScrollBar()->value();

    int indicatorViewportX =
        oldContentX - oldScroll;

    // --- apply zoom ---
    frameWidth_ = newFrameWidth;

    timeIndicatorBar->resize(
        qMax(frameCount_ * frameWidth_ + 235, width() - 4),
        scroller->viewport()->height()
    );

    // --- compute NEW content position ---
    int newOffset =
        (timeIndicatorBar->width() - frameCount_ * frameWidth_) / 2;

    int newContentX =
        newOffset + currentFrame_ * frameWidth_;

    // --- restore viewport position ---
    int newScroll =
        newContentX - indicatorViewportX;

    scroller->horizontalScrollBar()->setValue(newScroll);

    update();
    timeIndicatorBar->update();
}

void Timeline::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (timeIndicatorBar)
    {
        timeIndicatorBar->resize(
            qMax(frameCount_ * frameWidth_ + 235, width() - 4),
            scroller->viewport()->height()
        );
    }
}

void Timeline::playButtonClickEvent()
{
    playing_ = !playing_;

    if(playing_){
        playButton->setIcon(QIcon(QString(":/%1/icons/pause_%1.svg").arg(theme_)));
    }
    else{
        playButton->setIcon(QIcon(QString(":/%1/icons/play_%1.svg").arg(theme_)));
    }

    emit playSignal(playing_);
}

void Timeline::updateSelectedLayer(path *p)
{
    if(p == nullptr){
        timeIndicatorBar->setActiveLayer(nullptr);
        updateLayers();
        return;
    }

    for(int i = timeIndicatorBar->getLayerCount() - 1; i >= 0; i--){
        if(timeIndicatorBar->accessLayer(i)->relatedPath_ == p){
            timeIndicatorBar->setActiveLayer(timeIndicatorBar->accessLayer(i));
            break;
        }
    }

    updateLayers();
}
