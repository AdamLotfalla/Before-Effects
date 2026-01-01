#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QResizeEvent>

class TimeIndicator : public QWidget{
private:
    void paintEvent(QPaintEvent* event);
    int IndicatorWidth = 12;
public:
    TimeIndicator(QWidget* parent);
    // void Elongate(int newHeight);
    void MoveCenter(int x, int y);
};


class TimeIndicatorBar : public QWidget{   
public:
    TimeIndicatorBar(QWidget* parent, int FRATE, int FWIDTH, int FCOUNT, int WIDTH, int BARHEIGHT, int FULLHEIGHT, int &CURRENTF);
    void paintEvent(QPaintEvent* event);
    int frameWidth; 
    int frameCount; 
    int barHeight, fullHeight, fullWidth;
    int currentFrame;
    TimeIndicator* timeIndicator;
protected:
    int frameRate;  
    int tickInterval = 10;
    int singleBarHeight = 50;
    QWidget* parent;
    void onClick(QMouseEvent* event);
    void onUnClick(QMouseEvent* event);
    void resizeEvent(QResizeEvent *event) override;
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

    void resizeEvent(QResizeEvent *event) override;

    QScrollArea* scroller;
};