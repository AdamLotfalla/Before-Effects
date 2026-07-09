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
#include <QSplitter>
#include "viewPort.h"
#include "common_widget_styles.h"


class Layer : public QWidget{
public:
    Layer(QWidget* parent, path* p, int height);
    ~Layer(){
        relatedPath_ = nullptr;
    };

    bool isSelected_ = false;
    bool isExpanded_ = false;
    bool isValid() const {return relatedPath_ != nullptr;}
    path* relatedPath_ = nullptr;

private:
    int layerHeight_;
    int keyframeLayerHeight_ = 24;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event);
};

class TimeCursor : public QWidget{
    Q_OBJECT
public:
    TimeCursor(QWidget* parent);

    void MoveCenter(int x, int y = 20);

private:
    int IndicatorWidth_ = 12;
    void paintEvent(QPaintEvent* event);
};

class TickBar : public QWidget{   
    Q_OBJECT
public:
    TickBar(QWidget* parent, int *frameRate, int *frameWidth, int *frameCount, int width, int fullHeight, int *currentFrame);
    TimeCursor* timeIndicator_;
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

    // QVector<Layer*> layers_;
    // Layer* activeLayer_ = nullptr;
    // int layerHeight_ = 35;

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

    TickBar* tickBar;
    int* currentFrame_ = new int(0);
    bool playing_ = false;

    QVector<Layer*> layers_;
    Layer* activeLayer_ = nullptr;

    void step();
    void setTheme(QString theme = "Dark");
    void addLayer(path* p);
    void removeLayer(path* p);
    // void clickTimeIndicatorBar(QEvent* event);
    
private:
    int *frameRate_; // initialized on creating the instance 
    int frameWidth_; // initialized when reading the zoomSlider value
    int frameCount_ = 240;
    int layerHeight_ = 35; //there is alos a layerHeight_ member in Layer
    QVBoxLayout* toolBarVLayout_;
    QString theme_;

    QWidget* hierarchyPanel_;
    QVBoxLayout* hierarchyVLayout_;
    QScrollArea* hierarchyScroller_;
    QWidget* hierarchyLayerPanel_;
    QVBoxLayout* hierarchyLayerLayout_;

    QWidget* keyframePanel_;
    QVBoxLayout* keyframeVLayout_;
    QScrollArea* keyframeHScroller_;
    QScrollArea* keyframeVScroller_;
    QWidget* keyframeLayerPanel_;
    QVBoxLayout* keyframeLayerLayout_;



    QToolButton* playButton;

    void resizeEvent(QResizeEvent *event) override;
    void playButtonClickEvent();

    void updateKeyframeLayerPanelHeight();
    
public slots:
    void onLayersUpdate();
    void updateSelectedLayer(path* p);

private slots:
    void zoomSliderChanged(int value);

signals:
    void playSignal(bool playing);
    void frameChanged();
    void optimize(bool state);
};