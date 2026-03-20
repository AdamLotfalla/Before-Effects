#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWidget>
#include <QKeyEvent>
#include <QSvgRenderer>

#define UMask 0b10000000
#define RMask 0b01000000
#define DMask 0b00100000
#define LMask 0b00010000
#define ULMask 0b10010000
#define URMask 0b11000000
#define DRMask 0b01100000
#define DLMask 0b00110000

class bezierHandle{ 
    public:
    QPointF position_;
    bezierHandle (QPointF position);
};

class node{
    public:
    bool isHighlighted();
    node (QPointF position);
    QPointF position_;
    char mode = 'L'; // L: linear (rhombus), M: smooth (circle),  S: symmetric (square)

    bezierHandle* H1;
    bezierHandle* H2;
    
    private:
    void setHighlighted(bool state = true);
    bool highlighted_ = false;

    friend class path;
    // void paint (QPaintEvent* event);
};

class path : public QGraphicsItem{
    public:
    enum { Type = UserType + 1 };              //Unique ID for path
    int type() const override { return Type; } //Override standard type

    path(QVector<node*>& nodes, QVector<QVector<int>>& edges, QGraphicsItem* parent, bool *pathEditing);
    path(QPointF initialPoint, QGraphicsItem* parent, bool *pathEditing);
    void calculateBoundaries();


    QPointF getPoint(int index);
    int getLastNodeIndex();
    int getNodeCount();
    
    void setPreviewPoint(QPointF point);
    void clearPreviewPoint();

    void showSnapMargin(bool state);
    void setSnapping(bool state);
    void changeNodeMode(char newMode, int index);
    
    
    void setDrawingMode(bool state);
    
    void addHighlightedNode(int index);
    void removeHighlightedNode(int index);
    void clearHighlightedNodes();
    int nodesHighlighted(); //return size; 0 in case of empty
    bool isHighlighted(int index);
    int accessHighlightedVector(int index);
    
    void rotate(float angle);
    void rescale(qreal xOffset, qreal yOffset, QPointF error, bool restrictedX = 0, bool restrictedY = 0);
    void rescale(QPointF offset, QPointF error);
    void movePath(QPointF offset);
    void moveNode(QPointF offset, int index);
    void moveBezierHandle(QPointF newPosition, int index, int handleIndex);
    void addPoint(QPointF point);
    void addEdge(int start, int end);

    QPointF mapToItemRotation(const QPointF& point) const;
    QPointF mapToItemRotation(const QPointF& point, const bool reverse) const;
    QPointF mapToItemRotation(qreal x, qreal y);
    
    
    //attributes
    QPointF position_;
    float rotation_ = 0;
    qreal scaleX_ = 1, scaleY_ = 1;
    QPointF pivotPoint_ = QPointF(0,0);
    int strokeWidth_ = 2.0; 
    QColor strokeColor_ = Qt::blue;
    QColor fillColor_ = Qt::red;

    
    QRectF ULHandle, DLHandle, URHandle, DRHandle; //corner scale
    QRectF UHandle, DHandle, RHandle, LHandle;     //edge scale
    QRectF URRotationHandle, ULRotationHandle, DLRotationHandle, DRRotationHandle; //corner rotate
    qreal minX_, minY_, maxX_, maxY_;
    
    bool recentlySelected_ = false;
    void toggleRotationMode();
    bool inRotationMode();


    private:

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, 
        QWidget* widget = nullptr) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
        
        
    //using name convention: point = position; node = object that has more than position
    QVector<node*> actualNodes_;
    std::vector<node*> drawnNodes_;
    QVector<int> highlightedNodes_;
    QVector<QVector<int>> edges_;
    QPointF previewPoint_;

    //visuals 
    int handleD_ = 10;
    int selectionGrowth_ = 0;
    int handleGrowth_ = 10;
    uint8_t handleStates_ = 0;
    QPen handlePen_ = QPen(QColor("#000000"));
    QBrush handleBrush_ = QBrush(QColor("#FFFFFF"));

    //booleans
    bool firstPointSnapping_ = false;
    bool *inPathEditingMode_;
    bool inPathDrawingMode_;
    bool inRotationMode_ = false;
    bool firstPointHighlighted_ = false;
    bool hasDrawingPreview_ = false;
};

class viewPort : public QGraphicsView{

    Q_OBJECT

    public:
    viewPort(QWidget* parent);

    void enableSelectionTool(bool state);
    void enableNodeTool(bool state);
    void enableBezierTool(bool state);

    void setSelectedPath(path* newSelectedPath = nullptr, bool state = true);
    void setPathEditingMode(bool state);

    
    private:
    QGraphicsScene* scene_;
    int snapMargin_ = 10;
    int nodeSelectMargin_ = 10;

    enum ScaleHandle {
        None, 
        Left, 
        Right, 
        Top, 
        Bottom, 
        TopLeft, 
        TopRight, 
        BottomLeft, 
        BottomRight 
    };

    ScaleHandle activeScaleHandle_ = None;
    
    bool selectionToolActivated_ = false;
    bool bezierToolActivated_ = false;
    bool nodeToolActivated_ = false;
    
    bool startedNewPath_ = false;
    
    bool holding_ = false;
    bool shifting_ = false;
    bool panning_ = false;
    bool scaling_ = false;
    bool rotating_ = false;
    
    QPointF holdStartPosition_;
    QPointF offset_ = QPointF(0,0);
    // QPointF rotationStartPosition_;
    float originalRotation_;
    bool snap_ = false;
    bool inPathEditingMode_ = false;

    QPointF panStartScenePos_;
    QPointF panStartCanvasPos_;


    path* selectedPath_ = nullptr;

    QPointF scalingError_;
    

    QGraphicsItemGroup* canvas_;
    
    QVector<path*> objects_;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};