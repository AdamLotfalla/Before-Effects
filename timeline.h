#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QResizeEvent>

class TimeIndicator : public QWidget{
public:
    TimeIndicator(QWidget* parent);

    void MoveCenter(int x, int y = 20);

private:
    int IndicatorWidth_ = 12;
    void paintEvent(QPaintEvent* event);
};


class TimeIndicatorBar : public QWidget{   
public:
    TimeIndicatorBar(QWidget* parent, int *frameRate, int *frameWidth, int *frameCount, int width, int barHeight, int fullHeight, int *currentFrame);
    TimeIndicator* timeIndicator_;
    
    void paintEvent(QPaintEvent* event);
    void setFrameWidth(int value);
    int getFrameWidth();
    int getFrameCount();

private:
    int* frameWidth_; 
    int* frameCount_; 
    int* currentFrame_;
    int* frameRate_;

    int barHeight_;
    int fullHeight_;
    int fullWidth_;
    int tickInterval_ = 10;
    // int singleBarHeight_ = 50;

    void onClick(QMouseEvent* event);
    void onUnClick(QMouseEvent* event);
    void resizeEvent(QResizeEvent *event) override;
};



class Timeline : public QWidget{
public:
    Timeline(QWidget *parent = nullptr);

    TimeIndicatorBar* timeIndicatorBar;
    int currentFrame_;

    // void clickTimeIndicatorBar(QEvent* event);
    void zoomSliderChanged(int value);

private:
    int frameRate_ = 24;
    int frameWidth_ = 10;
    int frameCount_ = 240;
    int singleBarHeight_ = 50;
    QScrollArea* scroller;

    void resizeEvent(QResizeEvent *event) override;
};