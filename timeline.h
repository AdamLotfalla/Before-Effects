#pragma once
#include <QWidget>
#include <QScrollArea>

class TimeIndicatorBar : public QWidget{
public:
    TimeIndicatorBar(QWidget* parent, int frameRate, int frameWidth, int frameCount, int width, int height);
    void paintEvent(QPaintEvent* event);
    int frameWidth; 
    int frameCount; 
    int height, width;
protected:
    int frameRate;  
    int tickInterval = 10;
    
    QWidget* parent;
    
    int singleBarHeight = 50;
};




class Timeline : public QWidget{
public:
    Timeline(QWidget *parent = nullptr);

    TimeIndicatorBar* timeIndicatorBar;

    void clickTimeIndicatorBar(QEvent* event);
    void zoomSliderChanged(int value);
    void setIndicatorBarFrameWidth(int value);

protected:
    int frameRate = 24;
    int frameWidth = 10; // how much is each step in the time line
    int frameCount = 240;

    int singleBarHeight = 50;

    QScrollArea* scroller;
};