#include "viewPort.h"

node::node(QPointF position)
{
    position_ = position;
}

bool node::isHighlighted()
{
    return highlighted_;
}

void node::setHighlighted(bool state)
{
    highlighted_ = state;
}

path::path(QVector<node*>& nodes, QVector<QVector<int>>& edges, QGraphicsItem* parent, bool *pathEditing) : QGraphicsItem(parent)
{
    nodes_ = nodes;
    edges_ = edges;
    inPathEditingMode_ = pathEditing;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    calculateBoundaries();
}

path::path(QPointF initialPoint, QGraphicsItem *parent, bool *pathEditing) : QGraphicsItem(parent)
{
    inPathEditingMode_ = pathEditing;
    addPoint(initialPoint);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    calculateBoundaries();
}

QRectF path::boundingRect() const 
{
    if (nodes_.isEmpty()) return QRectF();

    qreal padding = strokeWidth_ / 2.0;
    return QRectF(minX_ - padding, minY_ - padding,
                  maxX_ - minX_ + strokeWidth_, maxY_ - minY_ + strokeWidth_);
}

void path::addPoint(QPointF point)
{
    node* newNode = new node(point);
    nodes_.push_back(newNode);

    QVector<int> emptyVector = {};
    edges_.push_back(emptyVector);

    calculateBoundaries();

    update();
}

void path::addEdge(int start, int end)
{
    edges_[start].push_back(end);
}

void path::modifyLastPoint(QPointF point)
{
    nodes_[nodes_.size() - 1]->position_ = point;
    calculateBoundaries();

    update();
}

void path::showSnapMargin(bool state)
{
    firstPointHighlighted_ = state;
}

QPointF path::getFirstPoint()
{
    return nodes_[0]->position_;
}

void path::setPreviewPoint(QPointF point) {
    previewPoint_ = point;
    hasDrawingPreview_ = true;
    update();
}

void path::clearPreviewPoint() {
    hasDrawingPreview_ = false;
    update();
}

bool path::hasPreviewPoint() const {
    return hasDrawingPreview_;
}

int path::getLastPointIndex()
{
    return nodes_.size() - 1;
}

int path::getNodeCount()
{
    return nodes_.size();
}

void path::addHighlightedNodeIndex(int index)
{
    nodes_[index]->setHighlighted(true);
    highlightedNodes_.push_back(index);
}

void path::removeHighlightedNodeIndex(int index)
{
    nodes_[index]->setHighlighted(false);
    if(highlightedNodes_.indexOf(index) != -1){
        highlightedNodes_.remove(highlightedNodes_.indexOf(index));
    }
}

bool path::nodesHighlightedExist()
{
    return !highlightedNodes_.isEmpty();
}

void path::clearHighlightedNodes()
{
    for(auto i : highlightedNodes_){
        nodes_[i]->setHighlighted(false);
    }

    QVector<int> emptyVector;
    highlightedNodes_ = emptyVector;
}

void path::movePath(QPointF offset)
{
    prepareGeometryChange(); // Notify Qt BEFORE changing data
    
    for(int i = 0; i < nodes_.size(); i++){
        nodes_[i]->position_ += offset;
    }
    
    calculateBoundaries();
}

void path::calculateBoundaries()
{
    minX_ = std::numeric_limits<qreal>::max();
    minY_ = std::numeric_limits<qreal>::max();
    maxX_ = std::numeric_limits<qreal>::lowest();
    maxY_ = std::numeric_limits<qreal>::lowest();

    for (int i = 0; i < nodes_.size(); i++){
        if (nodes_[i]->position_.x() < minX_) minX_ = nodes_[i]->position_.x();
        if (nodes_[i]->position_.y() < minY_) minY_ = nodes_[i]->position_.y();
        if (nodes_[i]->position_.x() > maxX_) maxX_ = nodes_[i]->position_.x();
        if (nodes_[i]->position_.y() > maxY_) maxY_ = nodes_[i]->position_.y();
    }
}

void path::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    
    if(nodes_.isEmpty()){
        return;
    }
    // Create and draw the path
    QPainterPath path;
    path.moveTo(nodes_[0]->position_);
    
    for (int i = 0; i < nodes_.size(); i++) {
        for(auto j : edges_[i]){
            path.lineTo(nodes_[j]->position_);
        }
    }
    
    if (nodes_[0]->position_ == nodes_.back()->position_) {
        path.closeSubpath();
    }
    
    // Fill
    if (fillColor_ != Qt::transparent) {
        painter->fillPath(path, QBrush(fillColor_));
    }
    
    // Stroke
    painter->setPen(QPen(strokeColor_, strokeWidth_));
    painter->drawPath(path);
    
    // Preview for next point to draw when using bezier pen
    if (hasDrawingPreview_) {
        painter->setPen(QPen(Qt::gray, 1, Qt::DotLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(nodes_.back()->position_, previewPoint_);

        painter->setBrush(QBrush(Qt::lightGray));
        painter->setPen(QPen(Qt::darkGray, 1));
        painter->drawEllipse(previewPoint_, 3, 3);
    }

    // Draw selection highlight if selected
    if (isSelected() && inPathEditingMode_ != nullptr && !*inPathEditingMode_) {
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
    else if(isSelected() && inPathEditingMode_ != nullptr && *inPathEditingMode_){

        QPainterPath Rhombus;
        Rhombus.moveTo(5,0);
        Rhombus.lineTo(9,5);
        Rhombus.lineTo(5,9);
        Rhombus.lineTo(0,5);
        Rhombus.closeSubpath();



        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(handlePen_);

        for(int i = 0; i<nodes_.size(); i++){
            if(nodes_[i]->isHighlighted())
                painter->setBrush(QBrush(QColor("#2A7FFF")));
            else
                painter->setBrush(handleBrush_);
            switch (nodes_[i]->mode)
            {
            case 'L':
                Rhombus.translate(nodes_[i]->position_-QPoint(5,5));
                painter->drawPath(Rhombus);
                Rhombus.translate(-nodes_[i]->position_+QPoint(5,5));
                break;
            case 'S':
                painter->drawEllipse(nodes_[i]->position_.x() - handleD_ / 2.0, nodes_[i]->position_.y() - handleD_ / 2.0, handleD_, handleD_);
                break;
            default:
                break;
            }
        }
    }

    // Snapping rectangle to the first point
    if(firstPointHighlighted_){
        painter->setCompositionMode(QPainter::CompositionMode_Difference);
        painter->setPen(QPen(QColor("#BEBEBE"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);

        QPointF p = nodes_.first()->position_;
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
    // canvas_->setFlag(QGraphicsItem::ItemIsMovable, true); // But movable

    
    scene_->addItem(canvas_);    
    canvas_->setPos((scene_->width() - canvasWidth) /2, (scene_->height() - canvasHeight)/2);
}

void viewPort::enableSelectionTool(bool state)
{
    selectionToolActivated_ = state;
    if(selectedPath_ != nullptr)
        selectedPath_->update();
}

void viewPort::enableNodeTool(bool state)
{
    nodeToolActivated_ = state;
    
    if(selectedPath_ != nullptr)
        selectedPath_->update();
}

void viewPort::enableBezierTool(bool state)
{
    bezierToolActivated_ = state;
    startedNewPath_ = false;
    
    if(!objects_.empty()){
        objects_.back()->clearPreviewPoint();
    }
}

void viewPort::setSelectedPath(path *newSelectedPath, bool state)
{
    //If we are deselecting the current path
    if (!state) {
        if (selectedPath_ != nullptr) {
            selectedPath_->setSelected(false);
            selectedPath_->update();
            selectedPath_ = nullptr;
        }
        return;
    }

    //If we are selecting a new path
    if (newSelectedPath != nullptr) {
        if (selectedPath_ != nullptr && selectedPath_ != newSelectedPath) {
            selectedPath_->setSelected(false);
            selectedPath_->update();
        }
        selectedPath_ = newSelectedPath;
        selectedPath_->setSelected(true);
        selectedPath_->update();
    }
}

void viewPort::setPathEditingMode(bool state)
{
    inPathEditingMode_ = state;
}

void viewPort::mousePressEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);


    if(event->button() == Qt::LeftButton){
        holdStartPosition_= canvasLocalPos;
        holding_ = true;
    } //set holding information

    if (bezierToolActivated_ && event->button() == Qt::LeftButton)
    {
        path* currentPath;

        if(!startedNewPath_){
            if(!objects_.empty()){
                setSelectedPath(objects_.back(), false);
            }
            startedNewPath_ = true;
            currentPath = new path(canvasLocalPos, canvas_, &inPathEditingMode_);
            setSelectedPath(currentPath);
            
            objects_.push_back(currentPath);
        }
        else{
            currentPath = objects_.back();
            int lastPointIndex = currentPath->getLastPointIndex();
            QPointF pointToAdd = canvasLocalPos;
            QPointF firstPoint = currentPath->getFirstPoint();
                    
            bool snap =
                std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;
        
            if (snap) {
                currentPath->addEdge(lastPointIndex, 0);
                currentPath->clearPreviewPoint();
                currentPath->showSnapMargin(false);
            
                startedNewPath_ = false;
            }
            else{
                currentPath->addPoint(pointToAdd);
                currentPath->addEdge(lastPointIndex, lastPointIndex + 1);
            }        
        }

        currentPath->calculateBoundaries();
        currentPath->update();
        scene_->update();
        return;
    }
    else if(nodeToolActivated_ && event->button() == Qt::LeftButton){
        bool clickedOnNode = false;
        inPathEditingMode_ = true;

        if(selectedPath_ == nullptr){
            path* clickedPath = qgraphicsitem_cast<path*>(itemAt(event->pos()));
            
            if (clickedPath) {
                setSelectedPath(clickedPath, true);
                holdStartPosition_ = canvasLocalPos;
                holding_ = true;
            } 
            else{
                return;
            }

            // QGraphicsView::mousePressEvent(event);
        }
        
        QRectF searchRect(
                scenePos.x() - nodeSelectMargin_, 
                scenePos.y() - nodeSelectMargin_,
                nodeSelectMargin_ * 2, 
                nodeSelectMargin_ * 2
            );
        for (int i = 0; i < selectedPath_->getNodeCount(); i++) {
            QPointF nodePos = canvas_->mapToScene(selectedPath_->nodes_[i]->position_); //converted to scene coordincates
            if (searchRect.contains(nodePos)) {
                if(shifting_){
                    if(selectedPath_->nodes_[i]->isHighlighted())
                    selectedPath_->removeHighlightedNodeIndex(i);
                    else
                    selectedPath_->addHighlightedNodeIndex(i);
                }
                else{
                    if(selectedPath_->nodes_[i]->isHighlighted() && selectedPath_->highlightedNodes_.size() > 1)
                    return;
                    selectedPath_->clearHighlightedNodes();
                    selectedPath_->addHighlightedNodeIndex(i);
                }
                clickedOnNode = true;
            }
        }
        selectedPath_->update();

        if(!clickedOnNode){
            if(selectedPath_ != nullptr){
                QVector<int> emptyVector;
                selectedPath_->highlightedNodes_ = emptyVector;
                selectedPath_->update();
            }
            QGraphicsView::mousePressEvent(event);
            return;
        }
    }
    else if (selectionToolActivated_ && event->button() == Qt::LeftButton) {
        path* clickedPath = qgraphicsitem_cast<path*>(itemAt(event->pos()));
        
        if (clickedPath) {
            setSelectedPath(clickedPath, true);
            holdStartPosition_ = canvasLocalPos;
            holding_ = true;
        } else {
            setSelectedPath(nullptr, false);
            holding_ = false;
        }
        return; 
    }
    
    // QGraphicsView::mousePressEvent(event);
}

void viewPort::mouseMoveEvent(QMouseEvent *event)
{
    // if (!(bezierToolActivated_ || nodeToolActivated_ || selectionToolActivated_) || selectedPath_ == nullptr) {
    //     QGraphicsView::mouseMoveEvent(event);
    //     return;
    // }        

    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);
    
    if(bezierToolActivated_ && selectedPath_ != nullptr){
        QPointF firstPoint = selectedPath_->getFirstPoint();
        snap_ =  std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;
    
        QPointF target = snap_ ? firstPoint : canvasLocalPos;
        selectedPath_->showSnapMargin(snap_);
    
        if (startedNewPath_ && holding_) {
            selectedPath_->clearPreviewPoint();
            selectedPath_->modifyLastPoint(target);
        } else if(startedNewPath_) {
            selectedPath_->setPreviewPoint(target);
        }
        else{
            if(!objects_.empty() && holding_){
                setSelectedPath(objects_.back(), false);
            }
            QGraphicsView::mouseMoveEvent(event);
        }
    }
    else if(nodeToolActivated_ && selectedPath_ != nullptr){
        if(selectedPath_ != nullptr && selectedPath_->nodesHighlightedExist() && holding_){
            for(int i = 0; i < selectedPath_->highlightedNodes_.size(); i++){
                selectedPath_->nodes_[selectedPath_->highlightedNodes_[i]]->position_ += (canvasLocalPos - holdStartPosition_);
            }
            holdStartPosition_ = canvasLocalPos;

            selectedPath_->calculateBoundaries();
            selectedPath_->update();
        }
    }
    else if(selectionToolActivated_ && selectedPath_ != nullptr){
        if(selectedPath_ != nullptr && holding_){
            QPointF offset = canvasLocalPos - holdStartPosition_;
            selectedPath_->movePath(offset);
            holdStartPosition_ = canvasLocalPos;
        }
    }
    
    if(selectedPath_ != nullptr){
        // selectedPath_->calculateBoundaries();
        selectedPath_->update();
    }
}

void viewPort::mouseReleaseEvent(QMouseEvent *event)
{
    holding_ = false;
}

void viewPort::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift){
        shifting_ = true;
    }
}

void viewPort::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift){
        shifting_ = false;
    }
}
