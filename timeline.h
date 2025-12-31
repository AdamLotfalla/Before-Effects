#pragma once
#include <QWidget>

class TimeIndicatorBar : public QWidget{

public:
    TimeIndicatorBar(QWidget* parent, int frameRate, int frameWidth, int frameCount, int height);
    void paintEvent(QPaintEvent* event);
protected:
    int frameRate;  
    int frameWidth; 
    int frameCount; 
    int tickInterval = 10;
    int height;

    int singleBarHeight = 50;
};

class Timeline : public QWidget{
private:
    int frameRate = 24;
    int frameWidth = 10; // how much is each step in the time line
    int frameCount = 240;

    int singleBarHeight = 50;

    // QWidget* timeIndicatorBar;


public:
    Timeline(QWidget *parent = nullptr);

    void clickTimeIndicatorBar();

};