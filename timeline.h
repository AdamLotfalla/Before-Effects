#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QResizeEvent>
#include <QToolButton>

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
    Timeline(QWidget *parent, int *frameRate);

    TimeIndicatorBar* timeIndicatorBar;
    int currentFrame_ = 0;
    bool playing_ = false;

    void step();
    // void clickTimeIndicatorBar(QEvent* event);
    
private:
    int *frameRate_; // initialized on creating the instance 
    int frameWidth_; // initialized when reading the zoomSlider value
    int frameCount_ = 240;
    int layerHeight_ = 50;
    QString theme;
    QScrollArea* scroller;
    QToolButton* playButton;

    void resizeEvent(QResizeEvent *event) override;
    void playButtonClickEvent();
    
private slots:
    void zoomSliderChanged(int value);

signals:
    void playSignal(bool playing);
};