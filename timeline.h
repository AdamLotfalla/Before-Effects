#pragma once
#include <QWidget>
#include <QScrollArea>

class TimeIndicator : public QWidget{
private:
    void paintEvent(QPaintEvent* event);
    int height = 500, width = 12;
public:
    TimeIndicator(QWidget* parent);
    void Elongate(int newHeight);
    void MoveCenter(int x, int y);
};


class TimeIndicatorBar : public QWidget{   
public:
    TimeIndicatorBar(QWidget* parent, int FRATE, int FWIDTH, int FCOUNT, int WIDTH, int HEIGHT);
    void paintEvent(QPaintEvent* event);
    int frameWidth; 
    int frameCount; 
    int height, width;
    TimeIndicator* timeIndicator;
protected:
    int frameRate;  
    int tickInterval = 10;
    int singleBarHeight = 50;
    QWidget* parent;
    void onClick(QMouseEvent* event);
    void onUnClick(QMouseEvent* event);
};



class Timeline : public QWidget{
public:
    Timeline(QWidget *parent = nullptr);

    TimeIndicatorBar* timeIndicatorBar;
    TimeIndicator* timeIndicator;

    void clickTimeIndicatorBar(QEvent* event);
    void zoomSliderChanged(int value);
    void setIndicatorBarFrameWidth(int value);

    int currentFrame = 0;

protected:
    int frameRate = 24;
    int frameWidth = 10; // how much is each step in the time line
    int frameCount = 240;

    int singleBarHeight = 50;

    QScrollArea* scroller;
};