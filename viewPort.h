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

    void calculateBoundaries(); // modify to calculate boundaries based on actualpoints and scale

    //using name convention: point = position; node = object that has more than position
    QVector<node*> actualNodes_;
    std::vector<node*> drawnNodes_;
    
    QVector<int> highlightedNodes_;
    QVector<QVector<int>> edges_; //edgest connect indices

    void addPoint(QPointF point);
    void addEdge(int start, int end);
    
    QRectF boundingRect() const override;

    
    void rotate(float angle);
    void rescale(qreal xOffset, qreal yOffset);
    QPointF getScale();
    
    void addHighlightedNode(int index);
    void removeHighlightedNode(int index);
    void clearHighlightedNodes();
    int nodesHighlighted(); //return size; 0 in case of empty
    bool isHighlighted(int index);
    int accessHighlightedVector(int index);
    
    QRectF ULHandle;
    QRectF DLHandle; 
    QRectF URHandle; 
    QRectF DRHandle; 
    QRectF UHandle;
    QRectF DHandle;
    QRectF RHandle;
    QRectF LHandle;
    QRectF URRotationHandle;
    QRectF ULRotationHandle;
    QRectF DLRotationHandle;
    QRectF DRRotationHandle;
    
    //attributes
    QPointF position_;
    float rotation_ = 0;
    qreal scaleX_ = 1, scaleY_ = 1;
    QPointF pivotPoint_ = QPointF(0,0);
    
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
        QWidget* widget = nullptr) override;
        
        
        int handleD_ = 10; //diameter
        int selectionGrowth_ = 0;
        int handleGrowth_ = 10;
        uint8_t handleStates_ = 0;

        bool firstPointSnapping_ = false;
        
        //------------TEMPORARY---------------- until implementing color and width selector
    int strokeWidth_ = 2.0; 
    QColor strokeColor_ = Qt::blue;
    QColor fillColor_ = Qt::red;
    QPen handlePen_ = QPen(QColor("#000000"));
    QBrush handleBrush_ = QBrush(QColor("#FFFFFF"));
    
    QPointF previewPoint_;
    
    
    bool *inPathEditingMode_;
    bool inPathDrawingMode_;
    bool inRotationMode_ = false;
    bool firstPointHighlighted_ = false;
    bool hasDrawingPreview_ = false;
    
    
    void setSnapping(bool state);
    
    
    
    
    
    
    
    
    
    
    
    
    //before overhaul
    QPointF getPoint(int index);
    QPointF getFirstPoint();
    void modifyLastPoint(QPointF point);
    int getLastNodeIndex();

    void setPreviewPoint(QPointF point);
    void clearPreviewPoint();
    bool hasPreviewPoint() const;
    
    void showSnapMargin(bool state);
    int getNodeCount();
    void changeNodeMode(char newMode, int index);

    void moveBezierHandle(QPointF newPosition, int index, int handleIndex);

    void toggleRotationMode();
    bool inRotationMode();
    void setDrawingMode(bool state);
    QPointF mapToItemRotation(const QPointF& point) const;
    QPointF mapToItemRotation(const QPointF& point, const bool reverse) const;

    uint8_t getHandleStates(); 
    //4 bits for each edge resize handle. for corner resize, 2 bits will be set.
    //4 bits for each corner rotate handle.  
    //counter clockwise, starting from upper edge. resize first

    void setHandleStates(uint8_t newStates);

    void movePath(QPointF offset);
    void moveNode(QPointF offset, int index);

    qreal minX_, minY_, maxX_, maxY_;
    
    private:
    
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