#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWidget>
#include <QKeyEvent>
#include <QSvgRenderer>
#include <QLayout>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QtColorWidgets/color_2d_slider.hpp>
#include <QtColorWidgets/ColorPreview>
#include <QtColorWidgets/ColorSelector>
#include <QToolButton>
#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include "common_widget_styles.h"

enum class handleMode{
    smooth,   //circle(M)
    linear,   //rhombus(L)
    symmetric //square(S)
};

using namespace color_widgets;

class customSpinBox: public QWidget{

    Q_OBJECT
    
    private:
    int height_ = 22;
    int symbolWidth_ = 20;
    char symbol_;
    bool keyFramed = false;
    double value_ = 0.0;
    unsigned int precision_ = 0;
    bool stringInput = false;
    QString stringValue = "";
    
    qreal maximum_ = +5000.0;
    qreal minimum_ = -5000.0;
    
    QRect rhombusBox;
    QLineEdit* textEdit_;

    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        QPainterPath path;
        path.addRoundedRect(0, 0, this->width(), height_, 5, 5);

        painter.setClipPath(path);

        painter.setBrush(QBrush("#272727"));
        painter.drawRoundedRect(0,0, this->width(), height_, 5,5);



        QPainterPath Rhombus;
        Rhombus.moveTo(4,0);
        Rhombus.lineTo(8,4);
        Rhombus.lineTo(4,8);
        Rhombus.lineTo(0,4);
        Rhombus.closeSubpath();

        rhombusBox = {
        this->width() - symbolWidth_ + static_cast<int>((symbolWidth_ - 8) * 0.5), 
        static_cast<int>((height_ - 8) * 0.5),
        8,
        8
        };

        Rhombus.translate(rhombusBox.topLeft());
        
        if(!stringInput){
            painter.setBrush(QBrush("#474747"));
            painter.drawRect(this->width() - symbolWidth_, 0, symbolWidth_, height_);
            
            if(keyFramed){
                painter.setBrush(QBrush("#7BC7B0")); 
                painter.setPen(Qt::NoPen);
                painter.drawPath(Rhombus);
            }
            else{
                QPainterPathStroker RhombusStroke;
                RhombusStroke.setWidth(2.5);
                RhombusStroke.setJoinStyle(Qt::RoundJoin);
                
                QPainterPath strokePath = RhombusStroke.createStroke(Rhombus);
                QPainterPath insideStroke = strokePath.intersected(Rhombus);
                
                painter.fillPath(insideStroke, QColor("#cecece"));
            }
        }

        
        painter.setPen(QPen("#7BC7B0"));
        painter.drawText(0,0, symbolWidth_, height_, Qt::AlignHCenter | Qt::AlignVCenter, QString(symbol_));

        if(!isEnabled()){
            painter.setCompositionMode(QPainter::CompositionMode_Darken);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor("#1E1E1E")));
            painter.setOpacity(0.5);
            painter.drawRect(0,0, this->width(), this->height());

            textEdit_->setStyleSheet(disabledLineEditStyle);
        }
        else{
            textEdit_->setStyleSheet(enabledLineEditStyle);
        }
    }


    void resizeEvent(QResizeEvent*) override
    {
        textEdit_->setGeometry(symbolWidth_, 0,
                                width() - 2*symbolWidth_,
                                height_);
    }

    public:

    void setKeyframe(bool state = true){
        keyFramed = state;
        update();
    }

    double getValue(){
        value_ = (textEdit_->text()).toDouble();
        return value_;
    }

    void setValue(qreal value){
        value_ = value;
        textEdit_->setText(QString::number(value_, 'f', precision_));
        // emit valueChanged(value);
        update();
    }

    void setValue(QString value){
        stringValue = value;
        textEdit_->setText(value);
        update();
    }

    void setMaximum(qreal value){
        maximum_ = value;
        delete textEdit_->validator();
        textEdit_->setValidator(new QDoubleValidator(minimum_, maximum_, precision_, this));
    }

    void setMinimum(qreal value){
        minimum_ = value;
        delete textEdit_->validator();
        textEdit_->setValidator(new QDoubleValidator(minimum_, maximum_, precision_, this));
    }

    void setPrecision(unsigned int value){
        precision_ = value;

        delete textEdit_->validator();
        textEdit_->setValidator(new QDoubleValidator(minimum_, maximum_, precision_, this));

        textEdit_->setText(QString::number(value_, 'f', precision_)); // reformat
    }

    void setStringInput(bool state){
        if(state){
            stringInput = state;
            textEdit_->setValidator(new QRegularExpressionValidator(QRegularExpression("#??[0-9a-fA-F]{6}?[0-9a-fA-F]{,2}?")));
        }
    }
    
    customSpinBox(QWidget* parent, char symbol): QWidget(parent){
        symbol_ = symbol;
        this->setFixedHeight(height_);
        this->setMinimumWidth(50);
        this->setAutoFillBackground(false);
        this->connect(this, &customSpinBox::onMouseClick, this, &customSpinBox::onClick);

        
        textEdit_ = new QLineEdit(QString::number(value_, 'f', precision_), this);
        textEdit_->setAlignment(Qt::AlignVCenter);
        textEdit_->setAutoFillBackground(false);
        textEdit_->setValidator(new QDoubleValidator(minimum_, maximum_, precision_, this));
        textEdit_->setGeometry(symbolWidth_, 0, this->width() - 2.0 * symbolWidth_, height_);
        textEdit_->setStyleSheet(spinBoxStyle);


        // connect(textEdit_, &QLineEdit::textChanged, this, &customSpinBox::onTextChanged);
        connect(textEdit_, &QLineEdit::editingFinished, this, &customSpinBox::onEditingFinished);
        textEdit_->update();
    }

    void mousePressEvent(QMouseEvent* event) override {
        emit onMouseClick(event->pos());
    }

    void setDisabled(bool state){
        textEdit_->setDisabled(state);
        QWidget::setDisabled(state);
        update();
    }
    

    private slots:

    void onClick(QPointF position){
        if(rhombusBox.contains(position.x(), position.y())){
            keyFramed = !keyFramed;
            emit toggledKeyframe(keyFramed, value_);
            update();
        };
    };

    void onTextChanged(const QString& text) {
        bool ok;
        double newValue = text.toDouble(&ok);
        if (ok) {
            value_ = newValue;
            emit valueChanged(value_);
        }
    }

    void onEditingFinished()
    {
        if(stringInput){
            stringValue = textEdit_->text();
            emit stringValueChanged(stringValue);
            return;
        }

        bool ok;
        double newValue = textEdit_->text().toDouble(&ok);
    
        if (ok) {
            value_ = newValue;
            emit valueChanged(value_);
        }
    
        textEdit_->setText(QString::number(value_, 'f', precision_));
    }

    signals:
    void onMouseClick(QPointF position);
    void valueChanged(qreal newValue);
    void stringValueChanged(QString newValue);
    void toggledKeyframe(bool state, qreal value);
};

class AttributePanel{
    public:
    virtual ~AttributePanel() = default;
    virtual QWidget* createAttributeWidget(QWidget* parent) = 0;
};

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

    handleMode mode = handleMode::linear; 

    bezierHandle* H1;
    bezierHandle* H2;
    
    private:
    void setHighlighted(bool state = true);
    bool highlighted_ = false;

    friend class path;
    // void paint (QPaintEvent* event);
};

class path : public QObject, public QGraphicsItem, public AttributePanel{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    public:
    enum { Type = UserType + 1 };              //Unique ID for path
    int type() const override { return Type; } //Override standard type

    path(QVector<node*>& nodes, QVector<QVector<int>>& edges, QGraphicsItem* parent, bool *pathEditing, int* frame);
    path(QPointF initialPoint, QGraphicsItem* parent, bool *pathEditing, int* frame);
    ~path(){
        for(node* n : actualNodes_) {
            delete n->H1;
            delete n->H2;
            delete n;
        }
        actualNodes_.clear();
        
        // Remove from scene if still there
        if(scene()) {
            scene()->removeItem(this);
        }
        
        // Clean up cached widget
        if(cachedAttributeWidget_) {
            cachedAttributeWidget_->deleteLater();
        }
    };
    
    void calculateBoundaries();
    void makeDirty();

    QWidget* createAttributeWidget(QWidget* parent) override;


    QPointF getActualPoint(int index);
    QPointF getDrawnPoint(int index);
    int getLastNodeIndex();
    int getNodeCount();
    
    void setPreviewPoint(QPointF point);
    void clearPreviewPoint();

    void showSnapMargin(bool state);
    void setSnapping(bool state);
    void changeNodeMode(handleMode newMode, int index);
    void incrementNodeMode(int index);
    
    
    void setDrawingMode(bool state);
    
    void addHighlightedNode(int index);
    void removeHighlightedNode(int index);
    void clearHighlightedNodes();
    bool isHighlighted(int index);
    int accessHighlightedVector(int index);
    int nodesHighlighted(); //return size; 0 in case of empty
    short int selectedHandle_ = -1; // -1 for none, 0 for H1, 1 for H2 
    bool getHandleExistance(int index, short int handleIndex);
    
    void rotate(float angle);
    void setRotation(float angle);
    void rescale(qreal xOffset, qreal yOffset, QPointF error, 
                 qreal originalHalfExtentX, qreal originalHalfExtentY, 
                 bool restrictedX = 0, bool restrictedY = 0, 
                 bool flipX = 0, bool flipY = 0);
    void setScale(qreal newScaleX, qreal newScaleY);
    void movePath(QPointF offset);
    void setPosition(QPointF newPos);
    void setPosition(qreal x, qreal y);
    void setPivotPoint(qreal x, qreal y);
    void moveNode(QPointF offset, int index);
    void setNodePosition(QPointF newPos, int index);
    QPointF getActualHandlePosition(int index, short int HandleIndex /*0 for H1, 1 for H2*/);
    QPointF getDrawnHandlePosition(int index, short int HandleIndex /*0 for H1, 1 for H2*/);
    handleMode getNodeMode(int index);
    void setHandlePosition(QPointF newPosition, int index, short int HandleIndex /*0 for H1, 1 for H2*/);
    void moveBezierHandle(QPointF newPosition, int index, int handleIndex);
    void addPoint(QPointF point);
    void addEdge(int start, int end);
    QString getName();
    void setName(QString newName);

    QPointF mapToItemRotation(const QPointF& point) const;
    QPointF mapToItemRotation(const QPointF& point, const bool reverse) const;
    QPointF mapToItemRotation(qreal x, qreal y);
    
    
    //attributes
    QString name_ = "New Path";
    QPointF position_;
    float rotation_ = 0;
    qreal scaleX_ = 1, scaleY_ = 1;
    QPointF pivotPoint_ = QPointF(0,0);
    qreal strokeWidth_ = 3.0; 
    QColor strokeColor_ = QColor("#D1495B");
    QColor fillColor_ = QColor("#EDAE49");
    bool fill_ = true;
    bool stroke_ = true;
    Qt::PenJoinStyle pathJointStyle = Qt::MiterJoin;
    Qt::PenCapStyle pathCapStyle = Qt::SquareCap;

    //keyframing
    int* currentFrame_;
    std::map<int, qreal> xPositionFrames;
    std::map<int, qreal> yPositionFrames;
    std::map<int, qreal> xPivotFrames;
    std::map<int, qreal> yPivotFrames;
    std::map<int, qreal> xScaleFrames;
    std::map<int, qreal> yScaleFrames;
    std::map<int, qreal> rotationFrames;
    std::map<int, qreal> strokeWidthFrames;
    std::map<int, qreal> fillRFrames;
    std::map<int, qreal> fillGFrames;
    std::map<int, qreal> fillBFrames;
    std::map<int, qreal> fillAFrames;
    std::map<int, qreal> strokeRFrames;
    std::map<int, qreal> strokeGFrames;
    std::map<int, qreal> strokeBFrames;
    std::map<int, qreal> strokeAFrames;

    
    
    QRectF ULHandle, DLHandle, URHandle, DRHandle; //corner scale
    QRectF UHandle, DHandle, RHandle, LHandle;     //edge scale
    QRectF URRotationHandle, ULRotationHandle, DLRotationHandle, DRRotationHandle; //corner rotate
    qreal minX_, minY_, maxX_, maxY_;
    
    bool recentlySelected_ = false;
    void optimize(bool state);
    void toggleRotationMode();
    bool inRotationMode();
    void supressKeyframeWrite(bool state);

    bool layerIsExpanded_ = false; // don't like it being public
    
    private:
    
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    
    //optimization
    QWidget* cachedAttributeWidget_ = nullptr;
    bool needTransformUpdate_ = true;
    void updateTransformedNodes();
    
    //using name convention: point = position; node = object that has more than position
    QVector<node*> actualNodes_;
    std::vector<node*> drawnNodes_;
    QVector<int> highlightedNodes_;
    QVector<QVector<int>> edges_;
    QPointF previewPoint_;
    
    //svgs for handles
    static QSvgRenderer* PDiagonalArrow;
    static QSvgRenderer* NDiagonalArrow;
    static QSvgRenderer* UDArrow;
    static QSvgRenderer* LRArrow;
    static QSvgRenderer* URRotationArrow;
    static QSvgRenderer* ULRotationArrow;
    static QSvgRenderer* DRRotationArrow;
    static QSvgRenderer* DLRotationArrow;
    static QSvgRenderer* PivotMark;

    static void initSvgRenderers();
    
    //visuals 
    const qreal handleD_ = 10;
    const qreal selectionGrowth_ = 0;
    const qreal handleGrowth_ = 10;
    const uint8_t handleStates_ = 0;
    const QPen handlePen_ = QPen(QColor("#000000"));
    const QBrush handleBrush_ = QBrush(QColor("#FFFFFF"));
    
    //booleans
    bool firstPointSnapping_ = false;
    bool *inPathEditingMode_;
    bool inPathDrawingMode_;
    bool inRotationMode_ = false;
    bool firstNodeHighlighted_ = false;
    bool hasDrawingPreview_ = false;
    bool optimized_ = false;
    bool supressKeyframeWrite_ = false;


    //signal
    std::function<void(QPointF)> onPositionChanged;
    std::function<void(qreal, qreal)> onScaleChanged;
    std::function<void(qreal)> onRotationChanged;
    signals:
    void updateLayer();
    void updateSpinBoxes(bool xposF, bool yposF, bool xpivotF, bool ypivotF, 
                         bool rotationF, bool xscaleF, bool yscaleF, 
                         bool RfillF, bool GfillF, bool BfillF, bool AfillF,
                         bool RstrokeF, bool GstrokeF, bool BstrokeF, bool AstrokeF, 
                         bool strokeWF);
};

class viewPort : public QGraphicsView{

    Q_OBJECT

    public:
    viewPort(QWidget* parent, int* frame);

    void enableSelectionTool(bool state);
    void enableNodeTool(bool state);
    void enableBezierTool(bool state);
    void createTestPath();


    void setSelectedPath(path* newSelectedPath = nullptr, bool state = true, bool hideAttributePanel = false);
    void setPathEditingMode(bool state);

    void optimize(bool state);
    QRectF canvasSceneRect() const { return canvas_->sceneBoundingRect(); }

    
    private:
    QGraphicsScene* scene_;
    const int snapMargin_ = 20;
    const int nodeSelectMargin_ = 10;
    const float snappingAngle = 45.0;

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
    bool shiftPressed_ = false;
    bool controlPressed_ = false;
    bool panning_ = false;
    bool scaling_ = false;
    bool rotating_ = false;

    bool scaleDragNegX_ = false;
    bool scaleDragNegY_ = false;

    qreal originalHalfExtentX_ = 0;
    qreal originalHalfExtentY_ = 0;
    
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
    int* currentFrame_;

    QGraphicsItemGroup* canvas_;
    
    QVector<path*> paths_;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void onFrameChanged();
    void supressKeyframesSlot(bool state);

signals:
    void attributePanelUpdateNeeded(AttributePanel* obj);
    void pathCreated(path* p);
    void pathDeleted(path* p);
    void updateLayer(path* p);
    void selectLayer(path* p);
    void frameChanged();
    void optimizeSignal(bool state);
    void supressKeyframesSignal(bool state);
};
