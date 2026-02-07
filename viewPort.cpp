#include "viewPort.h"

bezierHandle::bezierHandle(QPointF position)
{
    position_ = position;
}

node::node(QPointF position)
{
    position_ = position;
    H1 = nullptr;
    H2 = nullptr;
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
    originalNodes_ = nodes;
    edges_ = edges;
    inPathEditingMode_ = pathEditing;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    calculateBoundaries();
    originalWidth = maxX_ - minX_;
    originalHeight = maxY_ - minY_;
}

path::path(QPointF initialPoint, QGraphicsItem *parent, bool *pathEditing) : QGraphicsItem(parent)
{
    inPathEditingMode_ = pathEditing;
    addPoint(initialPoint);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    calculateBoundaries();
    originalWidth = maxX_ - minX_;
    originalHeight = maxY_ - minY_;
}

QRectF path::boundingRect() const 
{
    if (originalNodes_.isEmpty()) return QRectF();

    qreal padding = strokeWidth_ / 2.0;
    return QRectF(minX_ - padding, minY_ - padding,
                  maxX_ - minX_ + strokeWidth_, maxY_ - minY_ + strokeWidth_);
}

void path::addPoint(QPointF point)
{
    node* newNode = new node(point);
    originalNodes_.push_back(newNode);

    QVector<int> emptyVector = {};
    edges_.push_back(emptyVector);

    calculateBoundaries();

    update();
}

void path::addEdge(int start, int end)
{
    edges_[start].push_back(end);
}

QPointF path::getPoint(int index)
{
    return originalNodes_[index]->position_;
}

void path::modifyLastPoint(QPointF point)
{
    originalNodes_[originalNodes_.size() - 1]->position_ = point;
    calculateBoundaries();

    update();
}

void path::showSnapMargin(bool state)
{
    firstPointHighlighted_ = state;
}

QPointF path::getFirstPoint()
{
    return originalNodes_[0]->position_;
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

int path::getLastNodeIndex()
{
    return originalNodes_.size() - 1;
}

void path::applyCurrentTransform()
{
    // If there is no scale, do nothing
    if (qFuzzyCompare(scaleX, 1.0) && qFuzzyCompare(scaleY, 1.0)) return;

    prepareGeometryChange();

    // Bake the current scale into the original node positions
    for (node* n : originalNodes_) {
        n->position_ = QPointF(
            (n->position_.x() - scalePivotPoint_.x()) * scaleX + scalePivotPoint_.x(),
            (n->position_.y() - scalePivotPoint_.y()) * scaleY + scalePivotPoint_.y()
        );

        if(n->H1){
            n->H1->position_ = QPointF(
                (n->H1->position_.x() - scalePivotPoint_.x()) * scaleX + scalePivotPoint_.x(),
                (n->H1->position_.y() - scalePivotPoint_.y()) * scaleX + scalePivotPoint_.y()
            );
        }
        if(n->H2){
            n->H2->position_ = QPointF(
                (n->H2->position_.x() - scalePivotPoint_.x()) * scaleX + scalePivotPoint_.x(),
                (n->H2->position_.y() - scalePivotPoint_.y()) * scaleX + scalePivotPoint_.y()
            );
        }
    }

    // Reset scale to 1.0
    scaleX = 1.0;
    scaleY = 1.0;

    // Recalculate boundaries based on new permanent positions
    calculateBoundaries();
}

int path::getNodeCount()
{
    return originalNodes_.size();
}

void path::changeNodeMode(char newMode, int index)
{
    originalNodes_[index]->mode = newMode;
}

void path::moveBezierHandle(QPointF newPosition, int index, int handleIndex)
{
    node* currentNode = originalNodes_[index];

    if (currentNode->H1 == nullptr){
        currentNode->H1 = new bezierHandle(currentNode->position_ - (newPosition - currentNode->position_));
    }

    if(currentNode->H2 == nullptr){
        currentNode->H2 = new bezierHandle(newPosition);
    }
    

    switch (handleIndex)
    {
    case 1:
        currentNode->H1->position_ = newPosition;
        if(currentNode->mode == 'S'){
            currentNode->H2->position_ = currentNode->position_ - (newPosition - currentNode->position_);
        }
        break;
    case 2:
        currentNode->H2->position_ = newPosition;
        if(currentNode->mode == 'S'){
            currentNode->H1->position_ = currentNode->position_ - (newPosition - currentNode->position_);
        }
        break;
    }
}

void path::toggleRotationMode()
{
    inRotationMode_ = !inRotationMode_;
}

bool path::inRotationMode()
{
    return inRotationMode_;
}

void path::setDrawingMode(bool state)
{
    inPathDrawingMode_ = state;
}

void path::rotate(float angle)
{
    rotation += angle;
    update();
}

QPointF path::mapToItemRotation(const QPointF& point) const {
    if (qFuzzyCompare(rotation, 0.0f)) return point;
    
    QPointF center = QPointF(
        (minX_ + maxX_) * 0.5,
        (minY_ + maxY_) * 0.5
    );
    
    // Translate to origin
    QPointF translated = point - center;
    
    // Apply inverse rotation
    qreal radians = -rotation * M_PI / 180.0;
    qreal cosA = std::cos(radians);
    qreal sinA = std::sin(radians);
    
    QPointF rotated(
        translated.x() * cosA - translated.y() * sinA,
        translated.x() * sinA + translated.y() * cosA
    );
    
    // Translate back
    return rotated + center;
}

void path::rescale(qreal xOffset, qreal yOffset)
{
    scaleX += xOffset;
    scaleY += yOffset;
}

QPointF path::getScale()
{
    return QPointF(scaleX, scaleY);
}

void path::addHighlightedNode(int index)
{
    originalNodes_[index]->setHighlighted(true);
    highlightedNodes_.push_back(index);
}

void path::removeHighlightedNode(int index)
{
    originalNodes_[index]->setHighlighted(false);
    if(highlightedNodes_.indexOf(index) != -1){
        highlightedNodes_.remove(highlightedNodes_.indexOf(index));
    }
}

int path::nodesHighlighted()
{
    return highlightedNodes_.size();
}

bool path::isHighlighted(int index)
{
    return originalNodes_[index]->isHighlighted();
}

int path::accessHighlightedVector(int index)
{
    return highlightedNodes_[index];
}

uint8_t path::getHandleStates()
{
    return handleStates_;
}

void path::setHandleStates(uint8_t newStates)
{
    handleStates_ = newStates;
}

void path::clearHighlightedNodes()
{
    for(auto i : highlightedNodes_){
        originalNodes_[i]->setHighlighted(false);
    }

    QVector<int> emptyVector;
    highlightedNodes_ = emptyVector;
}

void path::movePath(QPointF offset)
{
    prepareGeometryChange(); // Notify Qt BEFORE changing data
    
    for(int i = 0; i < originalNodes_.size(); i++){
        originalNodes_[i]->position_ += offset;
        bezierHandle* H1 = originalNodes_[i]->H1;
        bezierHandle* H2 = originalNodes_[i]->H2;
        if(H1 != nullptr)
           H1->position_ += offset; 
        if(H2 != nullptr)
            H2->position_ += offset;
    }
    
    calculateBoundaries();
}

void path::moveNode(QPointF offset, int index)
{
    originalNodes_[index]->position_ += (offset);
    if(originalNodes_[index]->H1) originalNodes_[index]->H1->position_ += offset;
    if(originalNodes_[index]->H2) originalNodes_[index]->H2->position_ += offset;
}

void path::recalculateBoundariesForPoint(QPointF point)
{        
    if (point.x() < minX_) minX_ = point.x();
    if (point.y() < minY_) minY_ = point.y();
    if (point.x() > maxX_) maxX_ = point.x();
    if (point.y() > maxY_) maxY_ = point.y();
}

void path::calculateBoundaries()
{
    minX_ = std::numeric_limits<qreal>::max();
    minY_ = std::numeric_limits<qreal>::max();
    maxX_ = std::numeric_limits<qreal>::lowest();
    maxY_ = std::numeric_limits<qreal>::lowest();

    //quadratic bezier curve: B(t) = (1-t)^2 P0 + 2(1-t)t P1 + t^2 P2
    //stationary point when t = (p0 - p1)/(p0 - 2p1 + p2)

    for (int i = 0; i < originalNodes_.size(); i++){
        // int nextNodeIndex = (i+1) % getNodeCount();
        for(int nextNodeIndex : edges_[i]){
            QPointF P0 = originalNodes_[i]->position_;
            QPointF P1,P2;
            QPointF P3 = originalNodes_[nextNodeIndex]->position_;
            
            qreal tx,ty;
            qreal stationaryPx, stationaryPy;
            
            char startMode = originalNodes_[i]->mode;
            char endMode = originalNodes_[nextNodeIndex]->mode;

            
            if((endMode == 'S' || endMode == 'M') && (startMode == 'S' || startMode == 'M')){
                P1 = originalNodes_[i]->H2->position_;
                P2 = originalNodes_[nextNodeIndex]->H1->position_;

                //calculate t using the derivative
                
                QPointF a = -3.0 * P0 + 9.0 * P1 - 9.0 * P2 + 3.0 * P3;
                QPointF b = 6.0 * P0 - 12.0 * P1 + 6.0 * P2;
                QPointF c = -3.0 * P0 + 3.0 * P1;
                
                // if((a.x() > 1e-10 && a.y() > 1e-10)){

                    //first root (positive):
                    if(b.x() * b.x() - 4.0 * a.x() * c.x() < 0) 
                        tx = 0;
                    else
                        tx = (-1.0 * b.x() + sqrt(pow(b.x(),2) - 4.0 * a.x() * c.x())) / (2.0 * a.x());
                    if(b.y() * b.y() - 4.0 * a.y() * c.y() < 0)
                        ty = 0;
                    else
                        ty = (-1.0 * b.y() + sqrt(pow(b.y(),2) - 4.0 * a.y() * c.y())) / (2.0 * a.y());
                    if(tx < 0 || tx > 1) tx=0;
                    if(ty < 0 || ty > 1) ty=0;

                    stationaryPx = pow(1.0-tx,3) * P0.x() + 3.0 * pow(1.0-tx,2) * tx * P1.x() + 3.0 * (1.0-tx) * pow(tx,2) * P2.x() + pow(tx,3) * P3.x() ;
                    stationaryPy = pow(1.0-ty,3) * P0.y() + 3.0 * pow(1.0-ty,2) * ty * P1.y() + 3.0 * (1.0-ty) * pow(ty,2) * P2.y() + pow(ty,3) * P3.y() ;
                    recalculateBoundariesForPoint(QPointF(stationaryPx, stationaryPy));
                    
                    //second root (negative):
                    if(b.x() * b.x() - 4.0 * a.x() * c.x() < 0) 
                        tx = 0;
                    else
                        tx = (-1.0 * b.x() - sqrt(pow(b.x(),2) - 4.0 * a.x() * c.x())) / (2.0 * a.x());
                    if(b.y() * b.y() - 4.0 * a.y() * c.y() < 0)
                        ty = 0;
                    else
                        ty = (-1.0 * b.y() - sqrt(pow(b.y(),2) - 4.0 * a.y() * c.y())) / (2.0 * a.y());
                    if(tx < 0 || tx > 1) tx=0;
                    if(ty < 0 || ty > 1) ty=0;
                    
                    stationaryPx = pow(1.0-tx,3) * P0.x() + 3.0 * pow(1.0-tx,2) * tx * P1.x() + 3.0 * (1.0-tx) * pow(tx,2) * P2.x() + pow(tx,3) * P3.x() ;
                    stationaryPy = pow(1.0-ty,3) * P0.y() + 3.0 * pow(1.0-ty,2) * ty * P1.y() + 3.0 * (1.0-ty) * pow(ty,2) * P2.y() + pow(ty,3) * P3.y() ;
                    recalculateBoundariesForPoint(QPointF(stationaryPx, stationaryPy));
                // }
            }
            else if(endMode == 'L' && (startMode == 'S' || startMode == 'M')){
                P1 = originalNodes_[i]->H2->position_;
                P2 = P3;

                if(abs(P0.x() - 2.0 * P1.x() + P2.x()) < 1e-10) continue;
                
                tx = (P0.x() - P1.x()) / (P0.x() - 2.0 * P1.x() + P2.x());
                ty = (P0.y() - P1.y()) / (P0.y() - 2.0 * P1.y() + P2.y());

                if(tx < 0 || tx > 1) tx=0;
                if(ty < 0 || ty > 1) ty=0;

                stationaryPx = pow(1.0 - tx,2) * P0.x() + 2.0 * (1.0 - tx) * tx * P1.x() + pow(tx,2) * P2.x();
                stationaryPy = pow(1.0 - ty,2) * P0.y() + 2.0 * (1.0 - ty) * ty * P1.y() + pow(ty,2) * P2.y();

                recalculateBoundariesForPoint(QPointF(stationaryPx, stationaryPy));
            }
            else if((endMode == 'S' || endMode == 'M') && startMode == 'L'){
                P1 = originalNodes_[nextNodeIndex]->H1->position_;
                P2 = P3;

                if(abs(P0.x() - 2.0 * P1.x() + P2.x()) < 1e-10) continue;

                tx = (P0.x() - P1.x()) / (P0.x() - 2.0 * P1.x() + P2.x());
                ty = (P0.y() - P1.y()) / (P0.y() - 2.0 * P1.y() + P2.y());

                if(tx < 0 || tx > 1) tx=0;
                if(ty < 0 || ty > 1) ty=0;

                stationaryPx = pow(1.0 - tx,2) * P0.x() + 2.0 * (1.0 - tx) * tx * P1.x() + pow(tx,2) * P2.x();
                stationaryPy = pow(1.0 - ty,2) * P0.y() + 2.0 * (1.0 - ty) * ty * P1.y() + pow(ty,2) * P2.y();

                recalculateBoundariesForPoint(QPointF(stationaryPx, stationaryPy));
            }
        }

        recalculateBoundariesForPoint(originalNodes_[i]->position_);
    }

    originalMinX_ = minX_;
    originalMinY_ = minY_;
    originalMaxX_ = maxX_;
    originalMaxY_ = maxY_;

    originalHeight = maxY_ - minY_;
    originalWidth = maxX_ - minX_;
}

void path::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Save painter state
    painter->save();
    
    // Apply rotation transformation
    if (!qFuzzyCompare(rotation, (float)0.0)) {
        QPointF center = QPointF(
            (minX_ + maxX_) * 0.5,
            (minY_ + maxY_) * 0.5
        );
        
        painter->translate(center);
        painter->rotate(rotation);
        painter->translate(-center);
    }

    painter->setRenderHint(QPainter::Antialiasing);
    
    // return if the shape has no nodes
    if(originalNodes_.isEmpty()){
        return;
    }

    QPainterPath cross;
    cross.moveTo(scalePivotPoint_ + QPointF(-3,-3));
    cross.lineTo(scalePivotPoint_ + QPointF( 3, 3));
    cross.moveTo(scalePivotPoint_ + QPointF(-3, 3));
    cross.lineTo(scalePivotPoint_ + QPointF( 3,-3));

    
    painter->setBrush(QColor(Qt::transparent));
    painter->setPen(QPen(QColor("#2ea15e"), 2));
    painter->drawPath(cross);

    //calculate scaled nodes
    QPainterPath path;
    applyCurrentTransform();

    // Draw Edges
    path.moveTo(originalNodes_[0]->position_);
    for (int i = 0; i < originalNodes_.size(); i++) {
        for(auto j : edges_[i]){
            char initialMode = originalNodes_[i]->mode;
            char FinalMode = originalNodes_[j]->mode;

            if((FinalMode == 'S' || FinalMode == 'M') && initialMode == 'L'){
                path.quadTo(originalNodes_[j]->H1->position_, originalNodes_[j]->position_);
            }
            else if((FinalMode == 'S' || FinalMode == 'M') && (initialMode == 'S' || initialMode == 'M')){
                path.cubicTo(originalNodes_[i]->H2->position_, originalNodes_[j]->H1->position_, originalNodes_[j]->position_);
            }
            else if(FinalMode == 'L' && (initialMode == 'S' || initialMode == 'M')){
                path.quadTo(originalNodes_[i]->H2->position_, originalNodes_[j]->position_);
            }
            else if(FinalMode == 'L' && initialMode == 'L'){
                path.lineTo(originalNodes_[j]->position_);
            }
        }
    }
    
    // Fill
    if (!inPathDrawingMode_) {
        painter->fillPath(path, QBrush(fillColor_));
    }
    else{
        painter->fillPath(path, QBrush(Qt::transparent));
    }
    
    // Stroke
    if(!inPathDrawingMode_){
        painter->setPen(QPen(strokeColor_, strokeWidth_));
    }
    else{
        painter->setPen(QPen(QColor("#4C7FD1"), 1));
    }
    painter->drawPath(path);
    
    // Preview for next point to draw when using bezier pen
    if (hasDrawingPreview_) {
        painter->setPen(QPen(QColor("#B84343"), 1));
        // painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);

        if(originalNodes_[getLastNodeIndex()]->mode == 'S' || originalNodes_[getLastNodeIndex()]->mode == 'M'){
            QPainterPath previewPath;

            previewPath.moveTo(originalNodes_[getLastNodeIndex()]->position_);
            previewPath.quadTo(originalNodes_[getLastNodeIndex()]->H2->position_, previewPoint_);

            painter->drawPath(previewPath);
        }
        else{
            painter->drawLine(originalNodes_.back()->position_, previewPoint_);
        }

        painter->setBrush(QBrush(Qt::lightGray));
        painter->setPen(QPen(Qt::darkGray, 1));
        painter->drawEllipse(previewPoint_, 2, 2);
    }
    


    // Draw selection highlight and handles if selected
    if (isSelected() && inPathEditingMode_ != nullptr && !*inPathEditingMode_) {
        painter->setCompositionMode(QPainter::CompositionMode_Difference);
        painter->setPen(QPen(QColor("#BEBEBE"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        qreal padding = strokeWidth_ / 2.0;
        QRectF scaledBounds(minX_ - padding, minY_ - padding,
                           maxX_ - minX_ + strokeWidth_, maxY_ - minY_ + strokeWidth_);
        painter->drawRect(scaledBounds.adjusted(
            -selectionGrowth_,
            -selectionGrowth_,
            selectionGrowth_,
            selectionGrowth_
        ));
        
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        //remember that coordinates are those of the tope left corner
        ULHandle = QRectF(minX_ - handleD_/2.0 - handleGrowth_, minY_ - handleD_/2.0 - handleGrowth_, handleD_, handleD_);
        URHandle = QRectF(maxX_ - handleD_/2.0 + handleGrowth_, minY_ - handleD_/2.0 - handleGrowth_, handleD_, handleD_); 
        DRHandle = QRectF(maxX_ - handleD_/2.0 + handleGrowth_, maxY_ - handleD_/2.0 + handleGrowth_, handleD_, handleD_);
        DLHandle = QRectF(minX_ - handleD_/2.0 - handleGrowth_, maxY_ - handleD_/2.0 + handleGrowth_, handleD_, handleD_);
        UHandle  = QRectF(0.5 * (maxX_ + minX_) - handleD_/2.0, minY_ - handleD_ /2.0 - handleGrowth_, handleD_, handleD_);
        DHandle  = QRectF(0.5 * (maxX_ + minX_) - handleD_/2.0, maxY_ - handleD_ /2.0 + handleGrowth_, handleD_, handleD_);
        RHandle  = QRectF(maxX_ - handleD_/2.0 + handleGrowth_, 0.5 * (maxY_ + minY_) - handleD_/2.0, handleD_, handleD_);
        LHandle  = QRectF(minX_ - handleD_/2.0 - handleGrowth_, 0.5 * (maxY_ + minY_) - handleD_/2.0, handleD_, handleD_);

        ULRotationHandle = QRectF(minX_ - handleD_/2.0 - handleGrowth_, minY_ - handleD_/2.0 - handleGrowth_, handleD_, handleD_);
        URRotationHandle = QRectF(maxX_ - handleD_/2.0 + handleGrowth_, minY_ - handleD_/2.0 - handleGrowth_, handleD_, handleD_); 
        DRRotationHandle = QRectF(maxX_ - handleD_/2.0 + handleGrowth_, maxY_ - handleD_/2.0 + handleGrowth_, handleD_, handleD_);
        DLRotationHandle = QRectF(minX_ - handleD_/2.0 - handleGrowth_, maxY_ - handleD_/2.0 + handleGrowth_, handleD_, handleD_);
        
        QSvgRenderer PDiagonalArrow(QString(":/Handles/icons/PDiagonalArrows.svg"));
        QSvgRenderer NDiagonalArrow(QString(":/Handles/icons/NDiagonalArrows.svg"));
        QSvgRenderer UDArrow(QString(":/Handles/icons/UDArrows.svg"));
        QSvgRenderer LRArrow(QString(":/Handles/icons/LRArrows.svg"));
        QSvgRenderer URRotationArrow(QString(":/Handles/icons/URcornerArrow.svg"));
        QSvgRenderer ULRotationArrow(QString(":/Handles/icons/ULcornerArrow.svg"));
        QSvgRenderer DRRotationArrow(QString(":/Handles/icons/DRcornerArrow.svg"));
        QSvgRenderer DLRotationArrow(QString(":/Handles/icons/DLcornerArrow.svg"));
        

        
        if(inRotationMode_){
            URRotationArrow.render(painter, URRotationHandle);
            ULRotationArrow.render(painter, ULRotationHandle);
            DRRotationArrow.render(painter, DRRotationHandle);
            DLRotationArrow.render(painter, DLRotationHandle);
        }
        else{
            PDiagonalArrow.render(painter, URHandle);
            PDiagonalArrow.render(painter, DLHandle);
    
            NDiagonalArrow.render(painter, ULHandle);
            NDiagonalArrow.render(painter, DRHandle);
            UDArrow.render(painter, UHandle);
            UDArrow.render(painter, DHandle);
            LRArrow.render(painter, RHandle);
            LRArrow.render(painter, LHandle);
        }
            
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

        // draw nodes
        for(int i = 0; i<originalNodes_.size(); i++){
            
            // draw bezier handles for current node if they exist
            if((originalNodes_[i]->mode == 'S' || originalNodes_[i]->mode == 'M') && originalNodes_[i]->H1 != nullptr && originalNodes_[i]->H2 != nullptr){
                painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
                painter->setBrush(Qt::NoBrush);

                painter->drawLine(originalNodes_[i]->position_, originalNodes_[i]->H1->position_);
                painter->drawLine(originalNodes_[i]->position_, originalNodes_[i]->H2->position_);

                painter->setBrush(QBrush(Qt::white));
                painter->setPen(QPen(Qt::black, 1));
                painter->drawEllipse(originalNodes_[i]->H1->position_, handleD_/2, handleD_/2);
                painter->drawEllipse(originalNodes_[i]->H2->position_, handleD_/2, handleD_/2);
            }

            if(originalNodes_[i]->isHighlighted())
                painter->setBrush(QBrush(QColor("#2A7FFF")));
            else
                painter->setBrush(handleBrush_);

            switch (originalNodes_[i]->mode) // L: linear (rhombus), M: smooth (circle),  S: symmetric (square)
            {
            case 'L':
                Rhombus.translate(originalNodes_[i]->position_ - QPoint(5,5));
                painter->drawPath(Rhombus);
                
                // painter->drawText(originalNodes_[i]->position_, QString("(%1,%2)").arg(originalNodes_[i]->position_.x()).arg(originalNodes_[i]->position_.y())); //debug point positions

                Rhombus.translate(-1 * originalNodes_[i]->position_ + QPoint(5,5)); //reset the rohumbus to the original position to be moved in the next iteration (next node)
                break;
            case 'M':
                painter->drawEllipse(
                    originalNodes_[i]->position_ - QPointF(handleD_/2, handleD_/2),
                    handleD_, 
                    handleD_);
                break;
            case 'S':
                painter->drawRect(
                    originalNodes_[i]->position_.x() - handleD_/2,
                    originalNodes_[i]->position_.y() - handleD_/2,
                    handleD_,
                    handleD_);
                break;
            }
        }

    }
    else{
        painter->restore();
    }

    // Snapping rectangle to the first point
    if(firstPointHighlighted_){
        painter->setCompositionMode(QPainter::CompositionMode_Difference);
        painter->setPen(QPen(QColor("#BEBEBE"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);

        QPointF p = originalNodes_[0]->position_;
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
    canvas_->setFlag(QGraphicsItem::ItemIsSelectable, false);

    
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

    if(event->button() == Qt::MiddleButton){
        panning_ = true;
        holding_ = true;
        setCursor(Qt::ClosedHandCursor);
        holdStartPosition_ = canvasLocalPos;

        panStartScenePos_ = scenePos;
        panStartCanvasPos_ = canvas_->pos();
        return;
    } //panning

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
            currentPath->setDrawingMode(true);
            setSelectedPath(currentPath);
            
            objects_.push_back(currentPath);
        }
        else{
            currentPath = objects_.back();
            int lastPointIndex = currentPath->getLastNodeIndex();
            QPointF pointToAdd = canvasLocalPos;
            QPointF firstPoint = currentPath->getFirstPoint();
                    
            bool snap =
                std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;
        
            if (snap) {
                currentPath->addEdge(lastPointIndex, 0);
                currentPath->clearPreviewPoint();
                currentPath->setDrawingMode(false);
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
        }
        
        QRectF searchRect(
                scenePos.x() - nodeSelectMargin_, 
                scenePos.y() - nodeSelectMargin_,
                nodeSelectMargin_ * 2, 
                nodeSelectMargin_ * 2
            );
        for (int i = 0; i < selectedPath_->getNodeCount(); i++) {
            QPointF nodePos = canvas_->mapToScene(selectedPath_->getPoint(i)); //converted to scene coordincates
            if (searchRect.contains(nodePos)) {
                if(shifting_){
                    if(selectedPath_->isHighlighted(i))
                        selectedPath_->removeHighlightedNode(i);
                    else{
                        selectedPath_->addHighlightedNode(i);
                        holdStartPosition_ = canvasLocalPos;
                        holding_ = true;
                    }  
                }
                else{
                    if(selectedPath_->isHighlighted(i) && selectedPath_->nodesHighlighted() > 1)
                        return;
                    selectedPath_->clearHighlightedNodes();
                    selectedPath_->addHighlightedNode(i);
                    holdStartPosition_ = canvasLocalPos;
                    holding_ = true;
                }
                clickedOnNode = true;
            }
        }
        selectedPath_->update();

        if(!clickedOnNode){
            if(selectedPath_ != nullptr){
                selectedPath_->clearHighlightedNodes();
                selectedPath_->update();
            }
            QGraphicsView::mousePressEvent(event);
            return;
        }
    }
    else if (selectionToolActivated_ && event->button() == Qt::LeftButton) {
        path* clickedPath = qgraphicsitem_cast<path*>(itemAt(event->pos()));

        if(clickedPath){
            if(clickedPath != selectedPath_){
                setSelectedPath(clickedPath, true);
                holdStartPosition_ = canvasLocalPos;
                holding_ = true;
            }
                
            return;
        }

        if(selectedPath_ != nullptr){
            QPointF rotatedPos = selectedPath_->mapToItemRotation(canvasLocalPos);

            scaling_ =  !selectedPath_->inRotationMode() && event->button() == Qt::LeftButton &&
                        (selectedPath_->URHandle.contains(rotatedPos) ||
                         selectedPath_->ULHandle.contains(rotatedPos) ||
                         selectedPath_->DRHandle.contains(rotatedPos) ||
                         selectedPath_->DLHandle.contains(rotatedPos) ||
                          selectedPath_->UHandle.contains(rotatedPos) ||
                          selectedPath_->DHandle.contains(rotatedPos) ||
                          selectedPath_->RHandle.contains(rotatedPos) ||
                          selectedPath_->LHandle.contains(rotatedPos)
                        );
            rotating_ = selectedPath_->inRotationMode() && event->button() == Qt::LeftButton &&
                        (selectedPath_->URRotationHandle.contains(rotatedPos) ||
                         selectedPath_->ULRotationHandle.contains(rotatedPos) ||
                         selectedPath_->DRRotationHandle.contains(rotatedPos) ||
                         selectedPath_->DLRotationHandle.contains(rotatedPos)
                        );

            if(scaling_){
                holdStartPosition_ = rotatedPos;
                holding_ = true;
    
                // Apply existing transform before starting a new one.
                // This prevents the shape from jumping when changing the pivot.
                selectedPath_->applyCurrentTransform(); 
                
                // Note: applyCurrentTransform calls calculateBoundaries internaly, 
                // so originalMinX_ etc are fresh.
    
                selectedPath_->originalScaleX = selectedPath_->getScale().x();
                selectedPath_->originalScaleY = selectedPath_->getScale().y();
    
                uint8_t states;
    
                // Use originalMinX_/originalMaxY_ etc. for Pivot.
                // The math (node - pivot) * scale requires pivot to be in original node space.
                if(selectedPath_->URHandle.contains(canvasLocalPos)){
                    states = URMask;
                    selectedPath_->scalePivotPoint_ = QPointF(selectedPath_->originalMinX_, selectedPath_->originalMaxY_);
                }
                else if(selectedPath_->ULHandle.contains(canvasLocalPos)){
                    states = ULMask;
                    selectedPath_->scalePivotPoint_ = QPointF(selectedPath_->originalMaxX_, selectedPath_->originalMaxY_);
                }
                else if(selectedPath_->DLHandle.contains(canvasLocalPos)){
                    states = DLMask;
                    selectedPath_->scalePivotPoint_ = QPointF(selectedPath_->originalMaxX_, selectedPath_->originalMinY_);
                }
                else if(selectedPath_->DRHandle.contains(canvasLocalPos)){
                    states = DRMask;
                    selectedPath_->scalePivotPoint_ = QPointF(selectedPath_->originalMinX_, selectedPath_->originalMinY_);
                }
                else if(selectedPath_->UHandle.contains(canvasLocalPos)){
                    states = UMask;
                    selectedPath_->scalePivotPoint_ = QPointF((selectedPath_->maxX_ + selectedPath_->minX_) * 0.5, selectedPath_->maxY_);
                }
                else if(selectedPath_->DHandle.contains(canvasLocalPos)){
                    states = DMask;
                    selectedPath_->scalePivotPoint_ = QPointF((selectedPath_->maxX_ + selectedPath_->minX_) * 0.5, selectedPath_->minY_);
                }
                else if(selectedPath_->RHandle.contains(canvasLocalPos)){
                    states = RMask;
                    selectedPath_->scalePivotPoint_ = QPointF(selectedPath_->minX_, (selectedPath_->maxY_ + selectedPath_->minY_) * 0.5);
                }
                else if(selectedPath_->LHandle.contains(canvasLocalPos)){
                    states = LMask;
                    selectedPath_->scalePivotPoint_ = QPointF(selectedPath_->maxX_, (selectedPath_->maxY_ + selectedPath_->minY_) * 0.5);
                }
                
                selectedPath_->setHandleStates(states);
                selectedPath_->update();
                return;
            }
            else if(rotating_){
                holding_ = true;
                holdStartPosition_ = canvasLocalPos;
            }
            else if(!clickedPath){
                selectedPath_->setSelected(false);
                selectedPath_->update();
                selectedPath_ = nullptr;
            }
        }

        
    }
}

void viewPort::mouseMoveEvent(QMouseEvent *event)
{    

    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);
    
    if(panning_ && holding_){
        QPointF delta = scenePos - panStartScenePos_;
        canvas_->setPos(panStartCanvasPos_ + delta);

        holdStartPosition_ = canvasLocalPos;

        canvas_->update();
        return;
    }

    
    if(bezierToolActivated_ && selectedPath_ != nullptr){
        QPointF firstPoint = selectedPath_->getFirstPoint();
        snap_ =  std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;
    
        QPointF target = snap_ ? firstPoint : canvasLocalPos;
        selectedPath_->showSnapMargin(snap_);
    
        if (startedNewPath_ && holding_) {
            selectedPath_->clearPreviewPoint();
            // selectedPath_->modifyLastPoint(target); //this was added before bezier functionality
            selectedPath_->changeNodeMode('S', selectedPath_->getLastNodeIndex()); // this causes a crash
            selectedPath_->moveBezierHandle(canvasLocalPos, selectedPath_->getNodeCount() - 1, 2); // or maybe this?!
            selectedPath_->update();
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
        if(selectedPath_ != nullptr && selectedPath_->nodesHighlighted() > 0 && holding_){
            for(int i = 0; i < selectedPath_->nodesHighlighted(); i++){
                selectedPath_->moveNode(canvasLocalPos - holdStartPosition_, selectedPath_->accessHighlightedVector(i));
            }
            holdStartPosition_ = canvasLocalPos;

            selectedPath_->calculateBoundaries();
            selectedPath_->update();
        }
    }
    else if(selectionToolActivated_ && selectedPath_ != nullptr){
        if(holding_ && !scaling_){
            QPointF offset = canvasLocalPos - holdStartPosition_;
            selectedPath_->movePath(offset);
            holdStartPosition_ = canvasLocalPos;
        }
        else if(holding_ && rotating_){
            // QPointF rotatedPos = selectedPath_->mapToItemRotation(canvasLocalPos);
            
            QPointF rotationCenter = QPointF(0.5 * (selectedPath_->maxX_ + selectedPath_->minX_), 0.5 * (selectedPath_->maxY_ + selectedPath_->minY_));
            QPointF startVector = holdStartPosition_ - rotationCenter;
            QPointF endVector =  canvasLocalPos - rotationCenter;
            qreal startAngle = std::atan2(startVector.x(), startVector.y());
            qreal endAngle = std::atan2(endVector.x(), endVector.y());
            qreal angleDifference = (endAngle - startAngle) * 180.0 / M_PI;
            selectedPath_->rotate(-1 * angleDifference);
            selectedPath_->update();

            holdStartPosition_ = canvasLocalPos;
        }
        else if(holding_ && scaling_){
            uint8_t state = selectedPath_->getHandleStates();
            QPointF delta = canvasLocalPos - holdStartPosition_;

            // scale offsets
            qreal xOffset = delta.x() / selectedPath_->originalWidth;
            qreal yOffset = delta.y() / selectedPath_->originalHeight;
            
            selectedPath_->update();
            if(state == URMask){
                selectedPath_->rescale(xOffset, -1 * yOffset); //- delta.y() because y increases as you move down
            }
            else if(state == ULMask ){
                selectedPath_->rescale(-1 * xOffset, -1 * yOffset); // - delta.x() because x increases as you move right
            }
            else if(state == DRMask){
                selectedPath_->rescale(xOffset, yOffset);
            }
            else if(state == DLMask){
                selectedPath_->rescale(-1 * xOffset, yOffset);
            }
            else if(state &UMask){
                selectedPath_->rescale(0, -1 * yOffset);
            }
            else if(state &DMask){
                selectedPath_->rescale(0, yOffset);
            }
            else if(state & RMask){
                selectedPath_->rescale(xOffset, 0);
            }
            else if(state & LMask){
                selectedPath_->rescale(-1 * xOffset, 0);
            }

            holdStartPosition_ = canvasLocalPos;
            selectedPath_->update();
        }
    }
    
    if(selectedPath_ != nullptr){
        selectedPath_->update();
    }
}

void viewPort::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        setCursor(Qt::ArrowCursor);
        panning_ = false;
    }

    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);

    holding_ = false;
    
    // switch between rotation mode and scale mode if mouse clicked and declicked without moving
    path* clickedPath = qgraphicsitem_cast<path*>(itemAt(event->pos()));
    if (clickedPath && selectedPath_ != nullptr && clickedPath == selectedPath_  && holdStartPosition_ == canvasLocalPos){
        selectedPath_->toggleRotationMode();
        selectedPath_->update();
    }

    if(rotating_){
        rotating_ = false;
        if(selectedPath_ != nullptr)
            selectedPath_->update();
    }
    
        
    if(scaling_){
        scaling_ = false;
        if(selectedPath_ != nullptr){
            selectedPath_->calculateBoundaries();
            selectedPath_->update();
        }
    }

            
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
