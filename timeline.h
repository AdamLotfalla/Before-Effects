#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QResizeEvent>

class TimeIndicator : public QWidget{
    Q_OBJECT
public:
    TimeIndicator(QWidget* parent);

    void MoveCenter(int x, int y = 20);

private:
    int IndicatorWidth_ = 12;
    void paintEvent(QPaintEvent* event);
};


class TimeIndicatorBar : public QWidget{   
    Q_OBJECT
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
    bool clicked_ = false;
    
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
private slots:
    void onClick(QPoint pos);
    void onUnClick();

signals:
    void clicked(QPoint position);    
    void unClicked();
};



class Timeline : public QWidget{
    Q_OBJECT
public:
    Timeline(QWidget *parent = nullptr);

    TimeIndicatorBar* timeIndicatorBar;
    int currentFrame_;

    // void clickTimeIndicatorBar(QEvent* event);
    
private:
    int frameRate_ = 24;
    int frameWidth_ = 10;
    int frameCount_ = 240;
    int singleBarHeight_ = 50;
    QScrollArea* scroller;
    void resizeEvent(QResizeEvent *event) override;
    
private slots:
    void zoomSliderChanged(int value);
};