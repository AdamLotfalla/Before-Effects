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
    Q_OBJECT
public:
    Layer(QWidget* parent, path* p, int* frameWidth);
    ~Layer(){
        relatedPath_ = nullptr;
    };

    enum DrawMode {
        hierarchy,
        keyframe
    };

    DrawMode drawMode_ = hierarchy; 

    void refresh();
    bool isValid() const {return relatedPath_ != nullptr;}
    void setDrawMode(DrawMode newMode);
    void setExpanded(bool state);
    path* relatedPath_ = nullptr;
    QColor color_ = QColor("#F16E7A"); //light coral; temporary

private:
    int* frameWidth_;
    int layerHeight_ = 22;
    int keyframeLayerHeight_ = 20;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event);

signals:
    void expandedChanged(bool expanded);
    void makeSelected();
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
    TickBar(QWidget* parent, int *frameRate, int *frameWidth, int *frameCount, int width, int *currentFrame);
    QWidget* LBound;
    QWidget* RBound;
    
    void paintEvent(QPaintEvent* event);
    int getRBound();
    int getLBound();

    // void addLayer(path* p, QWidget* parent);
    // void removeLayer(path* p);
    // Layer* accessLayer(int index);
    // Layer* getActiveLayer();
    // int getLayerCount();
    // void setActiveLayer(Layer* l);

    int getTopBarHeight();
    
    int getFrameWidth();
    int getOffset();
    int getXLayerStart();
    int getLayerWidth();


private:
    int* frameWidth_; 
    int* frameCount_; 
    int* currentFrame_;
    int* frameRate_;

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

    TimeCursor* cursor_;
    int* currentFrame_ = new int(0);
    bool playing_ = false;

    QVector<QPair<Layer*, Layer*>> layers_;
    Layer* activeLayer_ = nullptr; //active layer will always point to the hierarchy layer

    void step();
    void setTheme(QString theme = "Dark");
    void addLayer(path* p);
    void removeLayer(path* p);
    void setActiveLayer(Layer *l); //active layer will always point to the hierarchy layer
    
// private:
    int *frameRate_; // initialized on creating the instance 
    int frameWidth_; // initialized when reading the zoomSlider value
    int frameCount_ = 240;
    QVBoxLayout* toolBarVLayout_;
    QString theme_;
    TickBar* tickBar_;

    QWidget* hierarchyPanel_;
    QVBoxLayout* hierarchyVLayout_;
    QScrollArea* hierarchyScroller_;
    QWidget* hierarchyLayerPanel_;
    QVBoxLayout* hierarchyLayerLayout_;

    QWidget* keyframePanel_;
    QVBoxLayout* keyframeVLayout_;
    QScrollArea* keyframeHScroller_;

    QHash<path*, QPair<Layer*, Layer*>> layerLookup_;

    QToolButton* playButton;

    void resizeEvent(QResizeEvent *event) override;
    void playButtonClickEvent();
    
public slots:
    void setSelectedLayer(path* p);
    void refreshLayer(path* p);

private slots:
    void zoomSliderChanged(int value);

signals:
    void playSignal(bool playing);
    void frameChanged();
    void optimize(bool state);
    void setSelectedPath(path* newSelectedPath);
};

class TimeEditor : public Timeline{
public:
    TimeEditor(QWidget* parent, int* frameRate);

private:


    QScrollArea* keyframeVScroller_;
    QHBoxLayout* keyframeMarginLayout_;
    QWidget* keyframeScrollerPanel_;
    QWidget* keyframeLeftMargin_;
    QWidget* keyframeRightMargin_;
    QWidget* keyframeLayerPanel_;
    QVBoxLayout* keyframeLayerLayout_;
};

class GraphEditor : public Timeline{

};