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
#include <QToolButton>
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

    for(int i = 0; i< *frameCount_; i++){
        QRect rect(i* *frameWidth_, 0, *frameWidth_, barHeight_);
        
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
            painter.drawLine(i* *frameWidth_, barHeight_ * 7/8, i* *frameWidth_, barHeight_ - 3); // -3 to compensate for pen width
            painter.drawText(QPoint(i* *frameWidth_ + 1, barHeight_ * 11/16), QString::number(i));
        }
    }

    pen.setColor("#444444");
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0, barHeight_, this->width(), barHeight_);

    timeIndicator_->MoveCenter(*currentFrame_ * *frameWidth_);
}

void TimeIndicatorBar::onClick(QPoint pos)
{
    clicked_ = true;
    int frame = round(pos.x() / (float)*frameWidth_);
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
        int frame = round(event->pos().x() / (float)*frameWidth_);
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
Timeline::Timeline (QWidget *parent) : QWidget(parent){

    QString theme = "Dark";
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

    // For QToolButton - better for icon-only buttons
    
    zoomInButton->setStyleSheet(buttonStyle);
    zoomOutButton->setStyleSheet(buttonStyle);
        


    
    QSlider* zoomSlider = new QSlider(toolbar);
    zoomSlider->setRange(1, 20); //zoom amount from 1x to 20x
    zoomSlider->setSingleStep(1);
    zoomSlider->setFixedWidth(100);
    zoomSlider->setOrientation(Qt::Horizontal);
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



    zoomSlider->setStyleSheet(sliderStyle);


    QHBoxLayout* toolBarLayout = new QHBoxLayout(toolbar);
    toolBarLayout->setContentsMargins(0,0,0,0);
    toolBarLayout->setSpacing(5);
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
    currentFrame_ = 19;
    timeIndicatorBar = new TimeIndicatorBar(scroller, &frameRate_, &frameWidth_, &frameCount_, frameWidth_ * frameCount_ + 235, 23, scroller->viewport()->height(), &currentFrame_);
    timeIndicatorBar->show();

    scroller->setWidget(timeIndicatorBar);
    // scroller->setWidgetResizable(true);
}

void Timeline::zoomSliderChanged(int value)
{
    if(frameWidth_ != value * 5){
        frameWidth_ = value * 5;

        timeIndicatorBar->resize(qMax(frameCount_ * frameWidth_ + 235, this->width() - 4), scroller->viewport()->height());

        repaint();
        timeIndicatorBar->repaint();
    }
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
