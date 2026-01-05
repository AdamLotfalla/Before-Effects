#include "viewPort.h"


path::path(QVector<QPointF>& points, QGraphicsItem* parent, bool *pathEditing) : QGraphicsItem(parent)
{
    points_ = points;
    inPathEditingMode_ = pathEditing;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

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

path::path(QPointF initialPoint, QGraphicsItem *parent, bool *pathEditing) : QGraphicsItem(parent)
{
    inPathEditingMode_ = pathEditing;
    addPoint(initialPoint);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    minX_ = initialPoint.x();
    minY_ = initialPoint.y();
    maxX_ = initialPoint.x();
    maxY_ = initialPoint.y();
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


void path::modifyLastPoint(QPointF point)
{
    points_[points_.size() - 1] = point;

    if (point.x() < minX_) minX_ = point.x();
    if (point.y() < minY_) minY_ = point.y();
    if (point.x() > maxX_) maxX_ = point.x();
    if (point.y() > maxY_) maxY_ = point.y();

    update();
}

void path::setHighlightFirstPoint(bool state)
{
    firstPointHighlighted_ = state;
}

QPointF path::getFirstPoint()
{
    return points_[0];
}

void path::setPreviewPoint(QPointF point) {
    previewPoint_ = point;
    hasPreview_ = true;
    update();
}

void path::clearPreviewPoint() {
    hasPreview_ = false;
    update();
}

bool path::hasPreviewPoint() const {
    return hasPreview_;
}

void path::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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
    
    if (points_[0] == points_.back()) {
        path.closeSubpath();
    }
    
    // Fill
    if (fillColor_ != Qt::transparent) {
        painter->fillPath(path, QBrush(fillColor_));
    }
    
    // Stroke
    painter->setPen(QPen(strokeColor_, strokeWidth_));
    painter->drawPath(path);
    

    if (hasPreview_) {
        painter->setPen(QPen(Qt::gray, 1, Qt::DotLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(points_.back(), previewPoint_);

        painter->setBrush(QBrush(Qt::lightGray));
        painter->setPen(QPen(Qt::darkGray, 1));
        painter->drawEllipse(previewPoint_, 3, 3);
    }

    // Draw selection highlight if selected
    if (isSelected() && !*inPathEditingMode_) {
        setFlag(ItemIsMovable, true);
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
    else if(isSelected() && *inPathEditingMode_){
        setFlag(ItemIsMovable, false);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(handlePen_);
        painter->setBrush(handleBrush_);
        for(int i = 0; i<points_.size(); i++){
            painter->drawEllipse(points_[i].x() - handleD_ / 2.0, points_[i].y() - handleD_ / 2.0, handleD_, handleD_);
        }
    }

    if(firstPointHighlighted_){
        painter->setCompositionMode(QPainter::CompositionMode_Difference);
        painter->setPen(QPen(QColor("#BEBEBE"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);

        QPointF p = points_.first();
        painter->drawRect(QRectF(p - QPointF(6,6), QSizeF(12,12)));
    }
}



viewPort::viewPort(QWidget* parent): QGraphicsView(parent)
{    
    scene_ = new QGraphicsScene(0,0, 800, 600, this);
    this->setScene(scene_);
    scene_->setBackgroundBrush(QBrush(QColor("#1E1E1E")));

    setMouseTracking(true);
    
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate); // <-- IMPORTANT
    setCacheMode(QGraphicsView::CacheNone);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);  // We'll handle dragging manually
    
    horizontalScrollBar()->setEnabled(false);
    verticalScrollBar()->setEnabled(false);


        // Setup rendering
    
    // Create 16:9 canvas rectangle
    const int canvasWidth = 16 * 50;
    const int canvasHeight = 9 * 50; //(16:9 aspect ratio)
    
    canvas_ = new QGraphicsItemGroup();
    
    QGraphicsRectItem* canvasRect = new QGraphicsRectItem(0, 0, canvasWidth, canvasHeight);
    canvasRect->setBrush(QBrush(QColor(240, 240, 240)));
    canvasRect->setPen(QPen(QColor(100, 100, 100)));
    canvasRect->setFlag(QGraphicsItem::ItemIsMovable, false);  // Allow moving canvas
    canvasRect->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    canvasRect->setFlag(QGraphicsItem::ItemIsSelectable, false);    
    
    canvas_->addToGroup(canvasRect);
    canvas_->setHandlesChildEvents(false); // Let children handle their own events
    canvas_->setFlag(QGraphicsItem::ItemIsSelectable, false); // Group itself not selectable
    canvas_->setFlag(QGraphicsItem::ItemIsMovable, true); // But movable

    
    scene_->addItem(canvas_);    
    canvas_->setPos((scene_->width() - canvasWidth) /2, (scene_->height() - canvasHeight)/2);
}

void viewPort::enableBezier(bool state)
{
    bezierActivated_ = state;
    startedNewPath_ = false;

    if(!objects_.empty()){
        objects_.back()->clearPreviewPoint();
    }
}

void viewPort::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);
    
    if (bezierActivated_ && event->button() == Qt::LeftButton)
    {
        holding_ = true;

        if(!startedNewPath_){
            if(!objects_.empty()){
                objects_.back()->setSelected(false);
            }
            startedNewPath_ = true;
            path* newPath = new path(canvasLocalPos, canvas_, &bezierActivated_);
            newPath->addPoint(canvasLocalPos);
            newPath->setSelected(true);
            newPath->update();
            
            objects_.push_back(newPath);
        }
        else{
            path* currentPath = objects_.back();
            QPointF pointToAdd = canvasLocalPos;
            QPointF firstPoint = currentPath->getFirstPoint();
                    
            bool snap =
                std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;
        
            if (snap) {
                currentPath->addPoint(firstPoint);
                currentPath->clearPreviewPoint();
                currentPath->setHighlightFirstPoint(false);
            
                startedNewPath_ = false;
                currentPath->update();
                return;
            }
            else{
                currentPath->addPoint(pointToAdd);
                currentPath->update();
            }        
        }

        scene_->update();
        return;
    }
    else if(event->button() == Qt::LeftButton && !objects_.empty()){ //click outside of any shape deselects the last shape
        objects_.back()->setSelected(false);
    }
    
    QGraphicsView::mousePressEvent(event);
}

void viewPort::mouseMoveEvent(QMouseEvent *event)
{
    if (!bezierActivated_ || objects_.empty()) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);
    path* current = objects_.back();

    QPointF firstPoint = current->getFirstPoint();

    snap =  std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
            std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;

    QPointF target = snap ? firstPoint : canvasLocalPos;
    current->setHighlightFirstPoint(snap);

    if (startedNewPath_ && holding_) {
        current->clearPreviewPoint();
        current->modifyLastPoint(target);
    } else if(startedNewPath_) {
        current->setPreviewPoint(target);
    }
    else{
        if(!objects_.empty() && holding_){
            objects_.back()->setSelected(false);
        }
        QGraphicsView::mouseMoveEvent(event);
    }
}

void viewPort::mouseReleaseEvent(QMouseEvent *event)
{
    holding_ = false;
}
