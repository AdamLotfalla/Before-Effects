#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWidget>
#include <QKeyEvent>

class node{
    public:
    bool isHighlighted();
    node (QPointF position);
    QPointF position_;
    char mode = 'L'; // L: linear, B: Bezier, S: symmetric
    
    private:
    void setHighlighted(bool state = true);
    bool highlighted_ = false;

    friend class path;
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
    void showSnapMargin(bool state);
    QPointF getFirstPoint();
    void setPreviewPoint(QPointF point);
    void clearPreviewPoint();
    bool hasPreviewPoint() const;
    int getLastPointIndex();
    int getNodeCount();
    void addHighlightedNodeIndex(int index);
    void removeHighlightedNodeIndex(int index);
    bool nodesHighlightedExist();
    void clearHighlightedNodes();
    void calculateBoundaries();
    QVector<node*> nodes_;
    QVector<int> highlightedNodes_;

    QColor fillColor_ = Qt::red; //temporary

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
    QVector<QVector<int>> edges_; //edgest connect indices


    bool *inPathEditingMode_;
    bool firstPointHighlighted_ = false;
    bool hasDrawingPreview_ = false;
    QPointF previewPoint_;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;
    
};

class viewPort : public QGraphicsView{

    Q_OBJECT

    public:
    viewPort(QWidget* parent);
    void enableBezierTool(bool state);
    void enableNodeTool(bool state);
    void setSelectedPath(path* newSelectedPath = nullptr, bool state = true);
    void setPathEditingMode(bool state);

    QPointF holdStartPosition_;
    
    private:
    QGraphicsScene* scene_;
    int snapMargin_ = 10;
    int nodeSelectMargin_ = 10;
    bool bezierToolActivated_ = false;
    bool nodeToolActivated_ = false;
    bool startedNewPath_ = false;
    bool holding_ = false;
    bool snap_ = false;
    bool inPathEditingMode_ = false;
    bool shifting_ = false;

    path* selectedPath_ = nullptr;
    

    QGraphicsItemGroup* canvas_;
    
    QVector<path*> objects_;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};