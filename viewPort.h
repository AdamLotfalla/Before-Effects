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
#include "common_widget_styles.h"

#define UMask 0b10000000
#define RMask 0b01000000
#define DMask 0b00100000
#define LMask 0b00010000
#define ULMask 0b10010000
#define URMask 0b11000000
#define DRMask 0b01100000
#define DLMask 0b00110000

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
    char mode = 'L'; // L: linear (rhombus), M: smooth (circle),  S: symmetric (square)

    bezierHandle* H1;
    bezierHandle* H2;
    
    private:
    void setHighlighted(bool state = true);
    bool highlighted_ = false;

    friend class path;
    // void paint (QPaintEvent* event);
};

class path : public QGraphicsItem, public AttributePanel{

    
    public:
    enum { Type = UserType + 1 };              //Unique ID for path
    int type() const override { return Type; } //Override standard type

    path(QVector<node*>& nodes, QVector<QVector<int>>& edges, QGraphicsItem* parent, bool *pathEditing);
    path(QPointF initialPoint, QGraphicsItem* parent, bool *pathEditing);
    void calculateBoundaries();

    QWidget* createAttributeWidget(QWidget* parent) override;


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
    void setRotation(float angle);
    void rescale(qreal xOffset, qreal yOffset, QPointF error, 
                 qreal originalHalfExtentX, qreal originalHalfExtentY, 
                 bool restrictedX = 0, bool restrictedY = 0, 
                 bool flipX = 0, bool flipY = 0);
    void setScale(qreal newScaleX, qreal newScaleY);
    void movePath(QPointF offset);
    void setPosition(QPointF newPos);
    void setPosition(qreal x, qreal y);
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
    const int handleD_ = 10;
    const int selectionGrowth_ = 0;
    const int handleGrowth_ = 10;
    const uint8_t handleStates_ = 0;
    const QPen handlePen_ = QPen(QColor("#000000"));
    const QBrush handleBrush_ = QBrush(QColor("#FFFFFF"));

    //booleans
    bool firstPointSnapping_ = false;
    bool *inPathEditingMode_;
    bool inPathDrawingMode_;
    bool inRotationMode_ = false;
    bool firstPointHighlighted_ = false;
    bool hasDrawingPreview_ = false;

    //signal
    std::function<void(QPointF)> onPositionChanged;
    std::function<void(qreal, qreal)> onScaleChanged;
    std::function<void(qreal)> onRotationChanged;
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
    const int snapMargin_ = 10;
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
    bool shifting_ = false;
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
    

    QGraphicsItemGroup* canvas_;
    
    QVector<path*> objects_;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    signals:
    void objectSelected(AttributePanel* obj);
};

class customSpinBox: public QWidget{

        Q_OBJECT
        
        private:
        int height_ = 22;
        int symbolWidth_ = 20;
        char symbol_;
        bool keyFramed = false;
        double value_ = 0.0;
        unsigned int precision_ = 0;
        
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

            painter.setBrush("#474747");
            painter.drawRect(this->width() - symbolWidth_, 0, symbolWidth_, height_);

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
            
            if(keyFramed){
                painter.setBrush("#7BC7B0"); 
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

            
            painter.setPen(QPen("#7BC7B0"));
            painter.drawText(0,0, symbolWidth_, height_, Qt::AlignHCenter | Qt::AlignVCenter, QString(symbol_));

            // value_ = (textEdit_->text()).toDouble();
            textEdit_->setGeometry(symbolWidth_, 0, this->width() - 2.0 * symbolWidth_, height_);

            
            // painter.setPen(QPen("#FFFFFF"));
            // painter.drawText(symbolWidth_,0, this->width() - 2.0 * symbolWidth_, height_, Qt::AlignVCenter, QString::number(value_, 'f', precision_));

        }

        void mouseReleaseEvent(QMouseEvent* event){
            if(event->button() == Qt::LeftButton){
                emit onMouseClick(event->position());
            }
        }
    
        public:

        void setKeyframe(bool state = true){
            keyFramed = state;
        }

        double getValue(){
            value_ = (textEdit_->text()).toDouble();
            return value_;
        }

        void setValue(qreal value){
            value_ = value;
            textEdit_->setText(QString::number(value_, 'f', precision_));
            emit valueChanged(value);
            update();
        }

        void setPrecision(unsigned int value){
            precision_ = value;

            delete textEdit_->validator();
            textEdit_->setValidator(new QDoubleValidator(-5000.0, 5000.0, precision_, this));

            textEdit_->update();
        }
        
        customSpinBox(QWidget* parent, char symbol): QWidget(parent){
            symbol_ = symbol;
            this->setFixedHeight(height_);
            this->setMinimumWidth(50);
            this->setAutoFillBackground(false);
            this->connect(this, &onMouseClick, this, &onClick);

            
            textEdit_ = new QLineEdit(QString::number(value_, 'f', precision_), this);
            textEdit_->setAlignment(Qt::AlignVCenter);
            textEdit_->setAutoFillBackground(false);
            textEdit_->setValidator(new QDoubleValidator(-5000.0, 5000.0, precision_, this));
            textEdit_->setGeometry(symbolWidth_, 0, this->width() - 2.0 * symbolWidth_, height_);
            textEdit_->setStyleSheet(spinBoxStyle);

            connect(textEdit_, &QLineEdit::textChanged, this, &customSpinBox::onTextChanged);
            connect(textEdit_, &QLineEdit::editingFinished, this, &customSpinBox::onEditingFinished);
            textEdit_->update();
        }

        private slots:

        void onClick(QPointF position){
            if(rhombusBox.contains(position.x(), position.y())){
                keyFramed = !keyFramed;
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

        void onEditingFinished() {
            textEdit_->setText(QString::number(value_, 'f', precision_));
            textEdit_->clearFocus();
        }

        signals:
        void onMouseClick(QPointF position);
        void valueChanged(qreal newValue);
    };