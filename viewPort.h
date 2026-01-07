#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWidget>

class node{
    private:
    
    public:
    char mode = 'L'; // L: linear, B: Bezier, S: symmetric
    QPointF position_;
    node (QPointF position);
    // void paint (QPaintEvent* event);
};

class path : public QGraphicsItem{
    // Q_OBJECT
    
    public:
    path(QVector<node*>& nodes, QVector<QVector<int>>& edges, QGraphicsItem* parent, bool *pathEditing);
    path(QPointF initialPoint, QGraphicsItem* parent, bool *pathEditing);
    // void select();
    QRectF boundingRect() const override;
    void addPoint(QPointF point);
    void addEdge(int start, int end);
    void modifyLastPoint(QPointF point);
    void setHighlightFirstPoint(bool state);
    QPointF getFirstPoint();
    void setPreviewPoint(QPointF point);
    void clearPreviewPoint();
    bool hasPreviewPoint() const;
    int getLastPointIndex();

    private:
    qreal minX_ = std::numeric_limits<qreal>::max();
    qreal minY_ = std::numeric_limits<qreal>::max();
    qreal maxX_ = std::numeric_limits<qreal>::min();
    qreal maxY_ = std::numeric_limits<qreal>::min();
    
    QPen handlePen_ = QPen(QColor("#000000"));
    QBrush handleBrush_ = QBrush(QColor("#FFFFFF"));

    int handleD_ = 8; //diameter
    int selectionGrowth_ = 0;
    int strokeWidth_ = 2.0; //temporary
    QColor strokeColor_ = Qt::blue; //temporary
    QColor fillColor_ = Qt::red; //temporary
    QVector<node*> nodes_;
    QVector<QVector<int>> edges_; //edgest connect indices


    bool *inPathEditingMode_;
    bool firstPointHighlighted_ = false;
    bool hasPreview_ = false;
    QPointF previewPoint_;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;
    
};

class viewPort : public QGraphicsView{

    Q_OBJECT

    public:
    viewPort(QWidget* parent);
    void enableBezier(bool state);
    
    private:
    QGraphicsScene* scene_;
    int snapMargin_ = 10;
    bool bezierActivated_ = false;
    bool startedNewPath_ = false;
    bool holding_ = false;
    bool snap = false;
    

    QGraphicsItemGroup* canvas_;
    
    QVector<path*> objects_;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};