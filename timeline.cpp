#include "timeline.h"
#include <QScrollArea>
#include <QLayout>
#include <QPainter>

TimeIndicatorBar::TimeIndicatorBar(QWidget* parent, int FRATE, int FWIDTH, int FCOUNT, int HEIGHT){
    QWidget* rect = new QWidget(parent);
    frameRate = FRATE;
    frameWidth = FWIDTH;
    frameCount = FCOUNT;
    height = HEIGHT;
}

Timeline::Timeline (QWidget *parent) : QWidget(parent){

    QScrollArea* scroller = new QScrollArea(this);
    scroller->setBaseSize(frameCount * frameWidth, this->height());

    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->addWidget(scroller);
    verticalLayout->setContentsMargins(1,1,1,1);

    TimeIndicatorBar* timeIndicatorBar = new TimeIndicatorBar(scroller, frameRate, frameWidth, frameCount, 23);
    timeIndicatorBar->setFixedSize(frameCount*frameWidth, 23);


    scroller->setWidget(timeIndicatorBar);

    
    
}

void TimeIndicatorBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;

    tickInterval = -1 * frameWidth + 20;

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

}
