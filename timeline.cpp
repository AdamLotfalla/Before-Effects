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

TimeIndicator::TimeIndicator(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(width, height);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
}

void TimeIndicator::Elongate(int newHeight)
{
    height = newHeight;
    this->setFixedHeight(height);
}

void TimeIndicator::MoveCenter(int x, int y)
{
    this->move(x - width/2, y);
    update();
}

void TimeIndicator::paintEvent(QPaintEvent *event)
{
    int centerX = width/2;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content
    painter.setRenderHint(QPainter::Antialiasing, true);

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
    painter.drawLine(centerX,8, centerX, height);
}




// ----------------------- INDICATOR BAR -----------------------
TimeIndicatorBar::TimeIndicatorBar(QWidget* parent, int FRATE, int FWIDTH, int FCOUNT, int WIDTH, int HEIGHT){
    frameRate = FRATE;
    frameWidth = FWIDTH;
    frameCount = FCOUNT;
    height = HEIGHT;
    width = WIDTH;
    this->resize(WIDTH, HEIGHT);

    timeIndicator = new TimeIndicator(parent);
    timeIndicator->show();
    timeIndicator->MoveCenter(500, 20);
    // timeIndicator->setAttribute(Qt::WA_AlwaysStackOnTop);


    // QObject::connect(this, &TimeIndicatorBar::leftButtonClicked, &TimeIndicatorBar::onClick);
    update();
}

void TimeIndicatorBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;

    switch (int((float)frameWidth/5))
    {
    case 1:
        tickInterval = 20;
        break;
    case 2 ... 3:
        tickInterval = 10;
        break;
    case 4 ... 7:
        tickInterval = 5;
        break;
    default:
        tickInterval = 1;
        break;
    };

    pen.setWidth(0);
    painter.setPen(pen);

    for(int i = 0; i<= frameCount; i++){
        QRect rect(i*frameWidth, 0, frameWidth, height);
        
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

    for(int i = 0; i <= frameCount; i++){
        if(i % tickInterval == 0){
            painter.drawLine(i*frameWidth, height * 7/8, i*frameWidth, height);
            painter.drawText(QPoint(i*frameWidth + 1, height * 11/16), QString::number(i));
        }
    }

    pen.setColor("#444444");
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0, height, width, height);
}

void TimeIndicatorBar::onClick(QMouseEvent *event)
{
    setMouseTracking(true);
    int x = event->pos().x();

    timeIndicator->MoveCenter(x/frameWidth * frameWidth, 20);
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
    QString buttonStyle = 
        "QToolButton {"
        "    background-color: #2E2E2E;"  // Match toolbar exactly
        "    border: none;"
        "    padding: 4px;"
        "    margin: 0px;"
        "}"
        "QToolButton:pressed {"
        "    background-color: #4E4E4E;"  // Much lighter for visibility
        "    border-radius: 3px;"
        "}"
        "QToolButton:hover:!pressed {"
        "    background-color: #3A3A3A;"  // Subtle hover
        "    border-radius: 3px;"
        "}";
    
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


    QString sliderStyle = 
        "QSlider {"
        "    background: transparent;"  // Transparent background
        "}"
        "QSlider::groove:horizontal {"
        "    border: none;"
        "    height: 4px;"
        "    background: #404040;"  // Dark track
        "    border-radius: 2px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: white;"    // White progress
        "    border-radius: 2px;"
        "}"
        "QSlider::add-page:horizontal {"
        "    background: #404040;"  // Dark unfilled portion
        "    border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: white;"
        "    border: 2px solid #2E2E2E;"  // Border matches toolbar
        "    width: 14px;"
        "    height: 14px;"
        "    margin: -6px 0;"
        "    border-radius: 7px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "    border: 2px solid #3E3E3E;"
        "}"
        "QSlider::handle:horizontal:pressed {"
        "    border: 2px solid #4E4E4E;"
        "}";
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

    timeIndicatorBar = new TimeIndicatorBar(scroller, frameRate, frameWidth, frameCount, 5 * frameCount + 235, 23);
    setIndicatorBarFrameWidth(zoomSlider->value() * 5);

    scroller->setWidget(timeIndicatorBar);
}

void Timeline::zoomSliderChanged(int value)
{
    if(frameWidth != value){
        frameWidth = value * 5;
        setIndicatorBarFrameWidth(value * 5);
        this->repaint();
        this->timeIndicatorBar->repaint();
    }
}

void Timeline::setIndicatorBarFrameWidth(int newFrameWidth)
{
    timeIndicatorBar->frameWidth = newFrameWidth;
    timeIndicatorBar->resize(qMax(timeIndicatorBar->frameCount * timeIndicatorBar->frameWidth + 235, this->width() - 4), timeIndicatorBar->height);
}





