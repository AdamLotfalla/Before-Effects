#include "timeline.h"
#include <QLayout>
#include <QPainter>
#include <QPolygon>
#include <QPoint>
#include <QPainterPath>
#include <QSlider>
#include <QIcon>
#include <QPushButton>
#include <QSvgRenderer>
#include <QDebug>
#include <QFile>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include "common_widget_styles.h"

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
TimeIndicatorBar::TimeIndicatorBar(QWidget* parent, int *frameRate, int *frameWidth, int *frameCount, int width, int barHeight, int fullHeight, int *currentFrame){
    //TEMPORARY
    currentFrame_ = currentFrame;
    
    frameRate_ = frameRate;
    frameWidth_ = frameWidth;
    frameCount_ = frameCount;

    barHeight_ = barHeight;
    fullHeight_ = fullHeight;
    fullWidth_ = width;

    resize(fullWidth_, fullHeight_);
    setMouseTracking(true);

    timeIndicator_ = new TimeIndicator(this);
    timeIndicator_->show();
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    // timeIndicator->setAttribute(Qt::WA_AlwaysStackOnTop);
    
    
    connect(this, &TimeIndicatorBar::clicked,
            this, &TimeIndicatorBar::onClick);
    connect(this, &TimeIndicatorBar::unClicked,
            this, TimeIndicatorBar::onUnClick);
    
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

    QRect fullRect(0, 0, this->width(), barHeight_);
    painter.fillRect(fullRect, QColor("#1E1E1E"));

    QRect leftMargin(0, barHeight_, offset_, height() - barHeight_);
    QRect rightMargin(offset_ + *frameCount_ * *frameWidth_, barHeight_, width() - offset_ - (*frameCount_ * *frameWidth_), height() - barHeight_); 
        //added this long expression for width in case the total margin is not even, so division will be short by 1
    painter.fillRect(leftMargin, QColor("#242424"));
    painter.fillRect(rightMargin, QColor("#242424"));

    
    for(int i = 0; i< *frameCount_; i++){
        QRect rect(offset_ + i* *frameWidth_, 0, *frameWidth_, barHeight_);
        
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
            painter.drawLine(offset_ + i* *frameWidth_, barHeight_ * 7/8, offset_ + i* *frameWidth_, barHeight_ - 3); // -3 to compensate for pen width
            painter.drawText(QPoint(offset_ + i* *frameWidth_ + 1, barHeight_ * 11/16), QString::number(i));
        }
    }

    pen.setColor("#444444");
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0, barHeight_, this->width(), barHeight_);

    timeIndicator_->MoveCenter(offset_ + *currentFrame_ * *frameWidth_);
}

void TimeIndicatorBar::onClick(QPoint pos)
{
    clicked_ = true;
    repaint();
    int frame = round((pos.x() - offset_) / (float)*frameWidth_);
    *currentFrame_ = frame;
    timeIndicator_->MoveCenter(frame * (*frameWidth_));
}

void TimeIndicatorBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        emit clicked(event->pos());
    } 
    QWidget::mousePressEvent(event);
}

void TimeIndicatorBar::mouseMoveEvent(QMouseEvent *event)
{
    if(clicked_){
        int frame = round((event->pos().x() - offset_) / (float)*frameWidth_);
        frame = qBound(0, frame, *frameCount_);
        *currentFrame_ = frame;

        // The paint event does the movement for me
    }
    update();
}

void TimeIndicatorBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        emit unClicked();
    }
}

void TimeIndicatorBar::onUnClick()
{
    clicked_ = false;
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
    theme = "Dark";
    this->setPalette(QPalette(QColor("#333333")));

    QWidget* toolbar = new QWidget(this);
    toolbar->setFixedHeight(25);
    toolbar->setAutoFillBackground(true);
    toolbar->setPalette(QPalette(QColor("#2E2E2E")));

    QToolButton* zoomInButton = new QToolButton(toolbar);
    QToolButton* zoomOutButton = new QToolButton(toolbar);

    zoomInButton->setFixedSize(21,21);
    zoomOutButton->setFixedSize(21,21);

    zoomInButton->setIcon(QIcon(QString(":/icons/%1/zoomIn_%1.svg").arg(theme)));
    zoomOutButton->setIcon(QIcon(QString(":/icons/%1/zoomOut_%1.svg").arg(theme)));
    zoomInButton->setIconSize(QSize(-1, 15));
    zoomOutButton->setIconSize(QSize(-1,15));
    zoomInButton->setToolTip("Zoom In Timeline");
    zoomOutButton->setToolTip("Zoom Out Timeline");
    zoomInButton->setStyleSheet(buttonStyle);
    zoomOutButton->setStyleSheet(buttonStyle);
        

    playButton = new QToolButton(toolbar);
    playButton->setFixedSize(21,21);
    playButton->setIcon(QIcon(QString(":/icons/%1/play_%1.svg").arg(theme)));
    playButton->setIconSize(QSize(-1,15));
    playButton->setToolTip("Play");
    playButton->setStyleSheet(buttonStyle);

    QToolButton* goToStartButton = new QToolButton(toolbar);
    goToStartButton->setFixedSize(21,21);
    goToStartButton->setIcon(QIcon(QString(":/icons/%1/goToStart_%1.svg").arg(theme)));
    goToStartButton->setIconSize(QSize(-1,15));
    goToStartButton->setToolTip("Go To Playback Start");
    goToStartButton->setStyleSheet(buttonStyle);

    QToolButton* goToEndButton = new QToolButton(toolbar);
    goToEndButton->setFixedSize(21,21);
    goToEndButton->setIcon(QIcon(QString(":/icons/%1/goToEnd_%1.svg").arg(theme)));
    goToEndButton->setIconSize(QSize(-1,15));
    goToEndButton->setToolTip("Go To Playback End");
    goToEndButton->setStyleSheet(buttonStyle);

    QToolButton* nextFrameButton = new QToolButton(toolbar);
    nextFrameButton->setFixedSize(21,21);
    nextFrameButton->setIcon(QIcon(QString(":/icons/%1/nextFrame_%1.svg").arg(theme)));
    nextFrameButton->setIconSize(QSize(-1,15));
    nextFrameButton->setToolTip("Go To Next Frame");
    nextFrameButton->setStyleSheet(buttonStyle);


    QToolButton* previousFrameButton = new QToolButton(toolbar);
    previousFrameButton->setFixedSize(21,21);
    previousFrameButton->setIcon(QIcon(QString(":/icons/%1/previousFrame_%1.svg").arg(theme)));
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
        currentFrame_ = 0;
        timeIndicatorBar->update();
    });

    QObject::connect(goToEndButton, &QToolButton::pressed, [this](){
        currentFrame_ = frameCount_;
        timeIndicatorBar->update();
    });

    QObject::connect(nextFrameButton, &QToolButton::pressed, [this](){
        if(currentFrame_ < frameCount_)
            currentFrame_ ++;
        timeIndicatorBar->update();
    });

    QObject::connect(previousFrameButton, &QToolButton::pressed, [this](){
        if(currentFrame_ > 0)
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


    scroller = new QScrollArea(this);

    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0,0,0,0);
    verticalLayout->setSpacing(0);
    verticalLayout->addWidget(toolbar);
    verticalLayout->addWidget(scroller);

    frameWidth_ = 5 * zoomSlider->value();
    timeIndicatorBar = new TimeIndicatorBar(scroller, frameRate_, &frameWidth_, &frameCount_, frameWidth_ * frameCount_ + 235, 23, scroller->viewport()->height(), &currentFrame_);
    timeIndicatorBar->show();

    scroller->setWidget(timeIndicatorBar);
    // scroller->setWidgetResizable(true);
}

void Timeline::step()
{
    if(currentFrame_ < frameCount_) 
        currentFrame_ ++;
    else
        currentFrame_ = 0;
    update();
}

void Timeline::zoomSliderChanged(int value)
{
    int newFrameWidth = value * 5;
    if (frameWidth_ == newFrameWidth)
        return;

    // --- 1️⃣ compute OLD content position ---
    int oldOffset =
        (timeIndicatorBar->width() - frameCount_ * frameWidth_) / 2;

    int oldContentX =
        oldOffset + currentFrame_ * frameWidth_;

    int oldScroll =
        scroller->horizontalScrollBar()->value();

    int indicatorViewportX =
        oldContentX - oldScroll;

    // --- 2️⃣ apply zoom ---
    frameWidth_ = newFrameWidth;

    timeIndicatorBar->resize(
        qMax(frameCount_ * frameWidth_ + 235, width() - 4),
        scroller->viewport()->height()
    );

    // --- 3️⃣ compute NEW content position ---
    int newOffset =
        (timeIndicatorBar->width() - frameCount_ * frameWidth_) / 2;

    int newContentX =
        newOffset + currentFrame_ * frameWidth_;

    // --- 4️⃣ restore viewport position ---
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
            timeIndicatorBar->width(),
            scroller->viewport()->height()
        );
    }
}

void Timeline::playButtonClickEvent()
{
    playing_ = !playing_;

    if(playing_){
        playButton->setIcon(QIcon(QString(":/icons/%1/pause_%1.svg").arg(theme)));
    }
    else{
        playButton->setIcon(QIcon(QString(":/icons/%1/play_%1.svg").arg(theme)));
    }

    emit playSignal(playing_);
}
