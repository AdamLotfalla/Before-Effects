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
#include <QRubberBand>
#include "viewPort.h"
#include "common_widget_styles.h"

class MarginPanel : public QWidget{
private:
    int LMarginWidth_;
    int RMarginWidth_;
    int offset_;
    int interBoundDist_;
    
    void paintEvent(QPaintEvent* event){
        QPainter painter(this);
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush("#1e1e1e"));
        painter.drawRect(0,0, offset_, height());
        painter.drawRect(offset_ + LMarginWidth_ + interBoundDist_ + RMarginWidth_, 0, width() - offset_ - interBoundDist_ - RMarginWidth_, height());
        painter.setBrush(QBrush("#252525"));
        painter.drawRect(offset_ + LMarginWidth_, 0, interBoundDist_, height());
        painter.setBrush(QBrush("#212121"));
        painter.drawRect(offset_,0, LMarginWidth_ , height());
        painter.drawRect(offset_ + LMarginWidth_ + interBoundDist_, 0, RMarginWidth_, height());
    }
public:
    MarginPanel(QWidget* parent) : QWidget(parent){setAutoFillBackground(false);};
    void setLMarginWidth(int value){
        LMarginWidth_ = value;
        update();
    }
    void setRMarginWidth(int value){
        RMarginWidth_ = value;
        update();
    }
    void setOffset(int value){
        offset_ = value;
        update();
    }
    void setInterBoundDist(int value){
        interBoundDist_ = value;
        update();
    }
};

class Layer : public QWidget{
    Q_OBJECT
public:
    Layer(QWidget* parent, path* p, int* frameWidth);
    ~Layer(){
        relatedPath_ = nullptr;
    };
    void selectKeyframesInRect(QRect localRect, bool additive);
    void commitSelectedKeyframeShift(int offset);
    enum DrawMode {
        hierarchy,
        keyframe
    };

    DrawMode drawMode_ = hierarchy; 

    void refresh();
    bool isValid() const {return relatedPath_ != nullptr;}
    void setDrawMode(DrawMode newMode);
    void setExpanded(bool state);
    static void setOffset(int value);
    path* relatedPath_ = nullptr;
    QColor color_ = QColor("#F16E7A"); //light coral; temporary
    void setLBoundFrame(int frame);
    void setRBoundFrame(int frame);
    
private:
    inline static int offset_;
    int* frameWidth_;
    int layerHeight_ = 22;
    int keyframeLayerHeight_ = 20;
    // int startFrame_ = 0, endFrame_ = 240;
    int LboundFrame_, RBoundFrame_; // int LboundFrame_ = startFrame_, RBoundFrame_ = endFrame_; 
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent *event) override;
    void shiftKeyframeLayer(qreal Xdist);
    bool rectSelecting_ = false;
    QRubberBand* rubberBand_ = nullptr;

    bool holding_ = false;
    bool draggingLboundary_ = false;
    bool draggingRboundary_ = false;
    bool hoveringLboundary_ = false;
    bool hoveringRboundary_ = false;
    bool draggingLayer_ = false;
    QPointF holdStartPos_;
    QPointF prevDragDist_;
    int dragFrameOffset_ = 0; 
    //I will use this so the keyframe positions only change visually while dragging the layer, but the changes will be commited and it will be zeroed on unclick 
    QMap<std::map<int,qreal>*, QSet<int>> selectedKeyframes; // each map will contain many selected keyframes

signals:
    void expandedChanged();
    void visibilityChanged();
    void makeSelected();
    void LayerDragged(int frameOffset);
    void keyframeMoved();
    void boundariesCrossed(qreal crossDist);
    void rectSelectionFinished(QRect panelRect, bool shifting);
    void keyframeDragging(int offset);
    void keyframeDragFinished();
public slots:
    void applyExternalDragOffset(int offset);
    void commitExternalDrag();
};

class TimeCursor : public QWidget{
    Q_OBJECT
public:
    TimeCursor(QWidget* parent);

    void MoveCenter(int x, int y = 20);

private:
    int IndicatorWidth_ = 10;
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
    void setOffset(qreal value);
    int getXLayerStart();
    int getInterBoundDist();


private:
    int* frameWidth_; 
    int* frameCount_; 
    int* currentFrame_;
    int* frameRate_;

    int fullWidth_;
    int tickInterval_ = 10;
    int offset_ = 118;
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
    void RBoundChanged(int frame);

    void LBoundClickedSignal(QPoint position);
    void LBoundUnClickedSignal();
    void LBoundChanged(int frame);

    void LayerClickedSignal(int layer);
    void frameChanged();
};

class Timeline : public QWidget{
    Q_OBJECT
public:
    Timeline(QWidget *parent, int *frameRate);

    TickBar* tickBar_;
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
    
private:
    int *frameRate_; // initialized on creating the instance 
    int frameWidth_; // initialized when reading the zoomSlider value
    int frameCount_ = 240;
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
    MarginPanel* keyframeLayerPanel_ ;
    QVBoxLayout* keyframeLayerLayout_;

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
    void frameChanged(int frame);
    void optimize(bool state);
    void setSelectedPath(path* newSelectedPath);
};