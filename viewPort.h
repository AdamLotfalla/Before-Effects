#pragma once
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
using namespace std;

class path : public QGraphicsItem{
    // Q_OBJECT
    
    public:
    path(QVector<QPointF>& points, QGraphicsItem* parent);
    path(QPointF initialPoint, QGraphicsItem* parent);
    // void select();
    QRectF boundingRect() const override;
    void addPoint(QPointF point);

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
    QVector<QPointF> points_;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;
    
};

class viewPort : public QGraphicsView{

    Q_OBJECT

    public:
    viewPort(QWidget* parent);

    private:
    QGraphicsScene* scene_;
};