#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QResizeEvent>
#include <QToolButton>
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
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include "common_widget_styles.h"


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
    TimeIndicatorBar(QWidget* parent, int *frameRate, int *frameWidth, int *frameCount, int width, int fullHeight, int *currentFrame);
    TimeIndicator* timeIndicator_;
    QWidget* LBound;
    QWidget* RBound;
    
    void paintEvent(QPaintEvent* event);
    int getRBound();
    int getLBound();

private:
    int* frameWidth_; 
    int* frameCount_; 
    int* currentFrame_;
    int* frameRate_;

    int fullHeight_;
    int fullWidth_;
    int tickInterval_ = 10;
    int offset_;
    int tickLayerHeight_ = 23;
    int boundHandleThickness = 8;
    int boundLayerHeight_ = 10;
    bool barClicked_ = false;
    bool LBoundClicked_ = false;
    bool RBoundClicked_ = false;

    int RBoundFrame_;
    int LBoundFrame_;
    
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
private slots:
    void onTickBarClick(QPoint pos);
    void onTickBarUnClick();
    void onLBoundClick(QPoint pos);
    void onLBoundUnClick();
    void onRBoundClick(QPoint pos);
    void onRBoundUnClick();

signals:
    void tickBarClickedSignal(QPoint position);    
    void tickBarUnClickedSignal();

    void RBoundClickedSignal(QPoint position);
    void RBoundUnClickedSignal();

    void LBoundClickedSignal(QPoint position);
    void LBoundUnClickedSignal();


};

class Timeline : public QWidget{
    Q_OBJECT
public:
    Timeline(QWidget *parent, int *frameRate);

    TimeIndicatorBar* timeIndicatorBar;
    int currentFrame_ = 0;
    bool playing_ = false;

    void step();
    void setTheme(QString theme = "Dark");
    // void clickTimeIndicatorBar(QEvent* event);
    
private:
    int *frameRate_; // initialized on creating the instance 
    int frameWidth_; // initialized when reading the zoomSlider value
    int frameCount_ = 240;
    int layerHeight_ = 50;
    QString theme_;
    QScrollArea* scroller;
    QToolButton* playButton;

    void resizeEvent(QResizeEvent *event) override;
    void playButtonClickEvent();
    
private slots:
    void zoomSliderChanged(int value);

signals:
    void playSignal(bool playing);
};