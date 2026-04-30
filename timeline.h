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
#include "viewPort.h"
#include "common_widget_styles.h"


class Layer : public QWidget{
public:
    Layer(QWidget* parent, path* p, int height);
    ~Layer(){
        relatedPath_ = nullptr;
    };

    bool isSelected_ = false;
    bool isValid() const {return relatedPath_ != nullptr;}
    path* relatedPath_ = nullptr;

private:
    int height_;
    void paintEvent(QPaintEvent* event) override;
};

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
    void addLayer(path* p, QWidget* parent);
    void removeLayer(path* p);
    Layer* accessLayer(int index);
    Layer* getActiveLayer();
    int getLayerCount();
    int getTopBarHeight();

    void setActiveLayer(Layer* l);

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

    QVector<Layer*> layers_;
    Layer* activeLayer_ = nullptr;
    int layerHeight_ = 35;

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

    void LayerClickedSignal(int layer);
    void frameChanged();
};

class Timeline : public QWidget{
    Q_OBJECT
public:
    Timeline(QWidget *parent, int *frameRate);

    TimeIndicatorBar* timeIndicatorBar;
    int* currentFrame_ = new int(0);
    bool playing_ = false;

    void step();
    void setTheme(QString theme = "Dark");
    void addLayer(path* p);
    void removeLayer(path* p);
    // void clickTimeIndicatorBar(QEvent* event);
    
private:
    int *frameRate_; // initialized on creating the instance 
    int frameWidth_; // initialized when reading the zoomSlider value
    int frameCount_ = 240;
    int layerHeight_ = 50;
    QWidget* layerPanel_;
    QHBoxLayout* timelineSplitterLayout_;
    QVBoxLayout* layersLayout_;
    QString theme_;
    QScrollArea* scroller;
    QToolButton* playButton;

    void resizeEvent(QResizeEvent *event) override;
    void playButtonClickEvent();
    
public slots:
    void updateLayers();
    void updateSelectedLayer(path* p);

private slots:
    void zoomSliderChanged(int value);

signals:
    void playSignal(bool playing);
    void frameChanged();
    void optimize(bool state);
};