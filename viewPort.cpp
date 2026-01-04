#include "viewPort.h"


path::path(QVector<QPointF>& points, QGraphicsItem* parent) : QGraphicsItem(parent)
{
    points_ = points;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    if(!points_.isEmpty()){
        minX_ = points_[0].x();
        minY_ = points_[0].y();
        maxX_ = points_[0].x();
        maxY_ = points_[0].y();
    }

    for (const QPointF& point : points_) {
        if (point.x() < minX_) minX_ = point.x();
        if (point.y() < minY_) minY_ = point.y();
        if (point.x() > maxX_) maxX_ = point.x();
        if (point.y() > maxY_) maxY_ = point.y();
    }
}

path::path(QPointF initialPoint, QGraphicsItem *parent)
{
    addPoint(initialPoint);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

QRectF path::boundingRect() const 
{
    if (points_.isEmpty()) return QRectF();

    qreal padding = strokeWidth_ / 2.0;
    return QRectF(minX_ - padding, minY_ - padding,
                  maxX_ - minX_ + strokeWidth_, maxY_ - minY_ + strokeWidth_);
}

void path::addPoint(QPointF point)
{
    points_.push_back(point);

    if(point.x() < minX_) minX_ = point.x();
    if(point.y() < minY_) minY_ = point.y();
    if(point.x() > maxX_) maxX_ = point.x();
    if(point.y() > maxY_) maxY_ = point.y();

    update();
}

void path::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                 QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    
    // Create and draw the path
    QPainterPath path;
    path.moveTo(points_[0]);
    
    for (int i = 1; i < points_.size(); ++i) {
        path.lineTo(points_[i]);
    }
    
    if (points_[0] == points_[points_.size() - 1]) {
        path.closeSubpath();
    }
    
    // Fill
    if (fillColor_ != Qt::transparent) {
        painter->fillPath(path, QBrush(fillColor_));
    }
    
    // Stroke
    painter->setPen(QPen(strokeColor_, strokeWidth_));
    painter->drawPath(path);
    

    // Draw selection highlight if selected
    if (isSelected()) {
        painter->setCompositionMode(QPainter::CompositionMode_Difference);
        painter->setPen(QPen(QColor("#BEBEBE"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(
            -selectionGrowth_,
            -selectionGrowth_,
            selectionGrowth_,
            selectionGrowth_
        ));
        
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(handlePen_);
        painter->setBrush(handleBrush_);
    
        painter->drawEllipse(minX_ - handleD_/2.0 - selectionGrowth_, minY_ - handleD_/2.0 - selectionGrowth_, handleD_, handleD_);
        painter->drawEllipse(minX_ - handleD_/2.0 - selectionGrowth_, maxY_ - handleD_/2.0 + selectionGrowth_, handleD_, handleD_);
        painter->drawEllipse(maxX_ - handleD_/2.0 + selectionGrowth_, minY_ - handleD_/2.0 - selectionGrowth_, handleD_, handleD_);
        painter->drawEllipse(maxX_ - handleD_/2.0 + selectionGrowth_, maxY_ - handleD_/2.0 + selectionGrowth_, handleD_, handleD_);
    }
    


}



viewPort::viewPort(QWidget* parent): QGraphicsView(parent)
{    
    scene_ = new QGraphicsScene(0,0, 800, 600, this);
    this->setScene(scene_);
    scene_->setBackgroundBrush(QBrush(QColor("#1E1E1E")));

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate); // <-- IMPORTANT
    setCacheMode(QGraphicsView::CacheNone);

        // Setup rendering
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);  // We'll handle dragging manually
    
    // Create 16:9 canvas rectangle
    const int canvasWidth = 16 * 50;
    const int canvasHeight = 9 * 50; //(16:9 aspect ratio)
    
    QGraphicsItemGroup* canvas = new QGraphicsItemGroup();
    
    QGraphicsRectItem* canvasRect = new QGraphicsRectItem(0, 0, canvasWidth, canvasHeight);
    canvasRect->setBrush(QBrush(QColor(240, 240, 240)));
    canvasRect->setPen(QPen(QColor(100, 100, 100)));
    canvasRect->setFlag(QGraphicsItem::ItemIsMovable, false);  // Allow moving canvas
    canvasRect->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    canvasRect->setFlag(QGraphicsItem::ItemIsSelectable, false);    
    
    canvas->addToGroup(canvasRect);
    canvas->setHandlesChildEvents(false); // Let children handle their own events
    canvas->setFlag(QGraphicsItem::ItemIsSelectable, false); // Group itself not selectable
    canvas->setFlag(QGraphicsItem::ItemIsMovable, true); // But movable

    
    scene_->addItem(canvas);    

    canvas->setPos((scene_->width() - canvasWidth) /2, (scene_->height() - canvasHeight)/2);

    QVector<QPointF> squarePoints = {
        QPointF(150, 150),
        QPointF(150, 50),
        QPointF(50, 50),
        QPointF(50, 150),
        QPointF(150, 150)
    };

    path* square = new path(squarePoints, canvas);
}