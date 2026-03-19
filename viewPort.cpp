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
    actualNodes_ = nodes;
    edges_ = edges;
    inPathEditingMode_ = pathEditing;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    calculateBoundaries();
    position_ = QPointF(0.5 * (minX_ + maxX_), 0.5 * (minY_ + maxY_));
}

path::path(QPointF initialPoint, QGraphicsItem *parent, bool *pathEditing) : QGraphicsItem(parent)
{
    inPathEditingMode_ = pathEditing;
    addPoint(initialPoint);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);

    calculateBoundaries();
    position_ = QPointF(0.5 * (minX_ + maxX_), 0.5 * (minY_ + maxY_));
}

QRectF path::boundingRect() const
{
    return shape().boundingRect().adjusted(
        -strokeWidth_, -strokeWidth_,
         strokeWidth_,  strokeWidth_);
}

void path::addPoint(QPointF point)
{
    node* newNode = new node(point);
    actualNodes_.push_back(newNode);

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
    return actualNodes_[index]->position_;
}

void path::showSnapMargin(bool state)
{
    firstPointHighlighted_ = state;
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

int path::getLastNodeIndex()
{
    return actualNodes_.size() - 1;
}

int path::getNodeCount()
{
    return actualNodes_.size();
}

void path::changeNodeMode(char newMode, int index)
{
    actualNodes_[index]->mode = newMode;
}

void path::moveBezierHandle(QPointF newPosition, int index, int handleIndex)
{
    node* currentNode = actualNodes_[index];

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

    calculateBoundaries();
}

void path::toggleRotationMode()
{
    inRotationMode_ = !inRotationMode_;
    update();
}

bool path::inRotationMode()
{
    return inRotationMode_;
}

void path::setDrawingMode(bool state)
{
    inPathDrawingMode_ = state;
}

QPointF path::mapToItemRotation(const QPointF &point) const
{
    if (qFuzzyCompare(rotation_, 0.0f)) return point;
    
    QPointF center = QPointF(
        (minX_ + maxX_) * 0.5,
        (minY_ + maxY_) * 0.5
    );
    
    // Translate to origin
    QPointF translated = point - center;
    
    // Apply inverse rotation
    qreal radians = rotation_ * M_PI / 180.0;
    qreal cosA = std::cos(radians);
    qreal sinA = std::sin(radians);
    
    QPointF rotated(
        translated.x() * cosA - translated.y() * sinA,
        translated.x() * sinA + translated.y() * cosA
    );
    
    // Translate back
    return rotated + center;
}

void path::rotate(float angle)
{
    rotation_ += angle;
    update();
    calculateBoundaries();
}

QPointF path::mapToItemRotation(const QPointF& point, const bool reverse) const {
    if (qFuzzyCompare(rotation_, 0.0f)) return point;
    
    QPointF center = QPointF(
        (minX_ + maxX_) * 0.5,
        (minY_ + maxY_) * 0.5
    );
    
    // Translate to origin
    QPointF translated = point - center;
    
    // Apply inverse rotation
    qreal radians = pow(-1,reverse) * rotation_ * M_PI / 180.0;
    qreal cosA = std::cos(radians);
    qreal sinA = std::sin(radians);
    
    QPointF rotated(
        translated.x() * cosA - translated.y() * sinA,
        translated.x() * sinA + translated.y() * cosA
    );
    
    // Translate back
    return rotated + center;
}

QPointF path::mapToItemRotation(qreal x, qreal y)
{
    return mapToItemRotation(QPointF(x,y));
}

void path::rescale(qreal xCenterD, qreal yCenterD)
{
    prepareGeometryChange();

    qreal xGlobalPos = xCenterD + position_.x();
    qreal yGlobalPos = yCenterD + position_.y();

    qreal xOriginalMax = (maxX_ - position_.x()) / scaleX_ + position_.x();
    qreal yOriginalMax = (maxY_ - position_.y()) / scaleY_ + position_.y();
    qreal xOriginalMin = (minX_ - position_.x()) / scaleX_ + position_.x();
    qreal yOriginalMin = (minY_ - position_.y()) / scaleY_ + position_.y();
    
    qreal xm = std::min((xGlobalPos - xOriginalMax), (xGlobalPos - xOriginalMin));
    qreal ym = std::min((yGlobalPos - yOriginalMax), (yGlobalPos - yOriginalMin));

    qreal xOriginalCoord = xCenterD - xm;
    qreal yOriginalCoord = yCenterD - ym;


    if(yOriginalCoord != 0 && yCenterD != 0){
        scaleY_ = yCenterD / yOriginalCoord;
    }
    if(xOriginalCoord != 0 && xCenterD != 0){
        scaleX_ = xCenterD / xOriginalCoord;
    }

        
    calculateBoundaries();
    update();
}

void path::addHighlightedNode(int index)
{
    actualNodes_[index]->setHighlighted(true);
    highlightedNodes_.push_back(index);
}

void path::removeHighlightedNode(int index)
{
    actualNodes_[index]->setHighlighted(false);
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
    return actualNodes_[index]->isHighlighted();
}

int path::accessHighlightedVector(int index)
{
    return highlightedNodes_[index];
}

void path::clearHighlightedNodes()
{
    for(auto i : highlightedNodes_){
        actualNodes_[i]->setHighlighted(false);
    }

    QVector<int> emptyVector;
    highlightedNodes_ = emptyVector;
}

void path::movePath(QPointF offset)
{
    prepareGeometryChange(); // Notify Qt BEFORE changing data
    
    for(int i = 0; i < actualNodes_.size(); i++){
        actualNodes_[i]->position_ += offset;
        bezierHandle* H1 = actualNodes_[i]->H1;
        bezierHandle* H2 = actualNodes_[i]->H2;
        if(H1 != nullptr)
           H1->position_ += offset; 
        if(H2 != nullptr)
            H2->position_ += offset;
    }

    position_ += offset;
    
    calculateBoundaries();
}

void path::moveNode(QPointF offset, int index)
{
    actualNodes_[index]->position_ += (offset);
    if(actualNodes_[index]->H1) actualNodes_[index]->H1->position_ += offset;
    if(actualNodes_[index]->H2) actualNodes_[index]->H2->position_ += offset;

    calculateBoundaries();
}

void path::setSnapping(bool state)
{
    firstPointSnapping_ = state;
}

QPainterPath path::shape() const
{
    if (actualNodes_.isEmpty())
        return QPainterPath();

    QPointF globalPivotPoint = position_ + pivotPoint_;

    float theta = rotation_ * M_PI / 180.0;
    qreal cosT = std::cos(theta);
    qreal sinT = std::sin(theta);

    auto transformPoint = [&](QPointF p)
    {
        // scale
        p.setX(p.x() * scaleX_ + globalPivotPoint.x() * (1 - scaleX_));
        p.setY(p.y() * scaleY_ + globalPivotPoint.y() * (1 - scaleY_));

        // rotate
        qreal x = p.x();
        qreal y = p.y();

        return QPointF(
            x*cosT - y*sinT + globalPivotPoint.x()*(1-cosT) + globalPivotPoint.y()*sinT,
            x*sinT + y*cosT + globalPivotPoint.y()*(1-cosT) - globalPivotPoint.x()*sinT
        );
    };

    QPainterPath p;

    p.moveTo(transformPoint(actualNodes_[0]->position_));

    for (int i = 0; i < actualNodes_.size(); i++)
    {
        for (int j : edges_[i])
        {
            QPointF P0 = transformPoint(actualNodes_[i]->position_);
            QPointF P3 = transformPoint(actualNodes_[j]->position_);

            char startMode = actualNodes_[i]->mode;
            char endMode = actualNodes_[j]->mode;

            if ((endMode == 'S' || endMode == 'M') && startMode == 'L')
            {
                QPointF P1 = transformPoint(actualNodes_[j]->H1->position_);
                p.quadTo(P1, P3);
            }
            else if ((endMode == 'S' || endMode == 'M') && (startMode == 'S' || startMode == 'M'))
            {
                QPointF P1 = transformPoint(actualNodes_[i]->H2->position_);
                QPointF P2 = transformPoint(actualNodes_[j]->H1->position_);
                p.cubicTo(P1, P2, P3);
            }
            else if (endMode == 'L' && (startMode == 'S' || startMode == 'M'))
            {
                QPointF P1 = transformPoint(actualNodes_[i]->H2->position_);
                p.quadTo(P1, P3);
            }
            else
            {
                p.lineTo(P3);
            }
        }
    }

    return p;
}

void path::calculateBoundaries()
{

    prepareGeometryChange();

    minX_ = std::numeric_limits<qreal>::max();
    minY_ = std::numeric_limits<qreal>::max();
    maxX_ = std::numeric_limits<qreal>::lowest();
    maxY_ = std::numeric_limits<qreal>::lowest();

    auto recalculateBoundariesForPoint = [&](QPointF point){        
        if (point.x() < minX_) minX_ = point.x();
        if (point.y() < minY_) minY_ = point.y();
        if (point.x() > maxX_) maxX_ = point.x();
        if (point.y() > maxY_) maxY_ = point.y();
    };

    auto scalePoint = [&](QPointF point){
        return QPointF(
            point.x() * scaleX_ + (position_.x() + pivotPoint_.x()) * (1-scaleX_),
            point.y() * scaleY_ + (position_.y() + pivotPoint_.y()) * (1-scaleY_)
        );
    };

    for (int i = 0; i < actualNodes_.size(); i++){
        // int nextNodeIndex = (i+1) % getNodeCount();
        for(int nextNodeIndex : edges_[i]){

            qreal x = actualNodes_[i]->position_.x();
            qreal y = actualNodes_[i]->position_.y();



            QPointF P0 = scalePoint(actualNodes_[i]->position_);
            QPointF P1,P2;
            QPointF P3 = scalePoint(actualNodes_[nextNodeIndex]->position_);
            
            qreal tx,ty;
            qreal stationaryPx, stationaryPy;
            
            char startMode = actualNodes_[i]->mode;
            char endMode = actualNodes_[nextNodeIndex]->mode;

            
            if((endMode == 'S' || endMode == 'M') && (startMode == 'S' || startMode == 'M')){
                P1 = scalePoint(actualNodes_[i]->H2->position_);
                P2 = scalePoint(actualNodes_[nextNodeIndex]->H1->position_);

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
            //quadratic bezier curve: B(t) = (1-t)^2 P0 + 2(1-t)t P1 + t^2 P2
            //stationary point when t = (p0 - p1)/(p0 - 2p1 + p2)
            else if(endMode == 'L' && (startMode == 'S' || startMode == 'M')){
                P1 = scalePoint(actualNodes_[i]->H2->position_);
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
                P1 = scalePoint(actualNodes_[nextNodeIndex]->H1->position_);
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

        recalculateBoundariesForPoint(scalePoint(actualNodes_[i]->position_));
    }

}

void path::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // return if the shape has no nodes
    if(actualNodes_.isEmpty()) return;

    for (node* n : drawnNodes_)
        delete n;

    drawnNodes_.clear();

    QPointF globalPivotPoint = position_ + pivotPoint_;

    // calculateBoundaries();

    QPainterPath cross;
    cross.moveTo(globalPivotPoint + QPointF(-3,-3));
    cross.lineTo(globalPivotPoint + QPointF( 3, 3));
    cross.moveTo(globalPivotPoint + QPointF(-3, 3));
    cross.lineTo(globalPivotPoint + QPointF( 3,-3));

    painter->setBrush(QColor(Qt::transparent));
    painter->setPen(QPen(QColor("#2ea15e"), 2));
    painter->drawPath(cross);

    //apply scale & rotation
    drawnNodes_.clear();
    for(int i = 0; i<actualNodes_.size(); i++){

        //scale
        QPointF scaledPoint;
        qreal x = actualNodes_[i]->position_.x();
        qreal y = actualNodes_[i]->position_.y();

        scaledPoint.setX(x*scaleX_ + globalPivotPoint.x() * (1-scaleX_));
        scaledPoint.setY(y*scaleY_ + globalPivotPoint.y() * (1-scaleY_));

        //rotation
        QPointF rotatedPoint;
        x = scaledPoint.x();
        y = scaledPoint.y();

        float theta_rad = rotation_ * M_PI / 180.0;

        rotatedPoint.setX(x*cos(theta_rad) - y*sin(theta_rad) + globalPivotPoint.x()*(1-cos(theta_rad)) + globalPivotPoint.y()*sin(theta_rad));
        rotatedPoint.setY(x*sin(theta_rad) + y*cos(theta_rad) + globalPivotPoint.y()*(1-cos(theta_rad)) - globalPivotPoint.x()*sin(theta_rad));

        node* transformedNode = new node(rotatedPoint);
        if(actualNodes_[i]->H1){
            QPointF H1_pos = actualNodes_[i]->H1->position_;
            x = H1_pos.x();
            y = H1_pos.y();
            
            //scale
            H1_pos.setX(x*scaleX_ + globalPivotPoint.x() * (1-scaleX_));
            H1_pos.setY(y*scaleY_ + globalPivotPoint.y() * (1-scaleY_));
            
            x = H1_pos.x();
            y = H1_pos.y();

            //rotation
            H1_pos.setX(x*cos(theta_rad) - y*sin(theta_rad) + globalPivotPoint.x()*(1-cos(theta_rad)) + globalPivotPoint.y()*sin(theta_rad));
            H1_pos.setY(x*sin(theta_rad) + y*cos(theta_rad) + globalPivotPoint.y()*(1-cos(theta_rad)) - globalPivotPoint.x()*sin(theta_rad));

            transformedNode->H1 = new bezierHandle(H1_pos);
        }

        if(actualNodes_[i]->H2){
            QPointF H2_pos = actualNodes_[i]->H2->position_;
            x = H2_pos.x();
            y = H2_pos.y();
            
            //scale
            H2_pos.setX(x*scaleX_ + globalPivotPoint.x() * (1-scaleX_));
            H2_pos.setY(y*scaleY_ + globalPivotPoint.y() * (1-scaleY_));
            
            x = H2_pos.x();
            y = H2_pos.y();

            //rotation
            H2_pos.setX(x*cos(theta_rad) - y*sin(theta_rad) + globalPivotPoint.x()*(1-cos(theta_rad)) + globalPivotPoint.y()*sin(theta_rad));
            H2_pos.setY(x*sin(theta_rad) + y*cos(theta_rad) + globalPivotPoint.y()*(1-cos(theta_rad)) - globalPivotPoint.x()*sin(theta_rad));

            transformedNode->H2 = new bezierHandle(H2_pos);
        }
        
        transformedNode->mode = actualNodes_[i]->mode;

        drawnNodes_.push_back(transformedNode);
    }


    // Draw Edges
    QPainterPath path;

    path.moveTo(drawnNodes_[0]->position_);
    for (int i = 0; i < drawnNodes_.size(); i++) {
        for(auto j : edges_[i]){
            char initialMode = drawnNodes_[i]->mode;
            char FinalMode = drawnNodes_[j]->mode;

            if((FinalMode == 'S' || FinalMode == 'M') && initialMode == 'L'){
                path.quadTo(drawnNodes_[j]->H1->position_, drawnNodes_[j]->position_);
            }
            else if((FinalMode == 'S' || FinalMode == 'M') && (initialMode == 'S' || initialMode == 'M')){
                path.cubicTo(drawnNodes_[i]->H2->position_, drawnNodes_[j]->H1->position_, drawnNodes_[j]->position_);
            }
            else if(FinalMode == 'L' && (initialMode == 'S' || initialMode == 'M')){
                path.quadTo(drawnNodes_[i]->H2->position_, drawnNodes_[j]->position_);
            }
            else if(FinalMode == 'L' && initialMode == 'L'){
                path.lineTo(drawnNodes_[j]->position_);
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
        painter->setBrush(Qt::NoBrush);
        
        QPainterPath previewPath;
        if((drawnNodes_[getLastNodeIndex()]->mode == 'S' || drawnNodes_[getLastNodeIndex()]->mode == 'M') && (drawnNodes_[0]->mode == 'S' || drawnNodes_[0]->mode == 'M') && firstPointSnapping_){
            previewPath.moveTo(drawnNodes_[getLastNodeIndex()]->position_);
            previewPath.cubicTo(drawnNodes_[getLastNodeIndex()]->H2->position_, drawnNodes_[0]->H1->position_, previewPoint_);
            
            painter->drawPath(previewPath);
        }
        else if((drawnNodes_[getLastNodeIndex()]->mode == 'S' || drawnNodes_[getLastNodeIndex()]->mode == 'M')){
            previewPath.moveTo(drawnNodes_[getLastNodeIndex()]->position_);
            previewPath.quadTo(drawnNodes_[getLastNodeIndex()]->H2->position_, previewPoint_);

            painter->drawPath(previewPath);
        }
        else if(drawnNodes_[getLastNodeIndex()]->mode == 'L' && (drawnNodes_[0]->mode == 'S' || drawnNodes_[0]->mode == 'M') && firstPointSnapping_){
            previewPath.moveTo(drawnNodes_[getLastNodeIndex()]->position_);
            previewPath.quadTo(drawnNodes_[0]->H1->position_, previewPoint_);

            painter->drawPath(previewPath);
        }
        else{
            painter->drawLine(drawnNodes_.back()->position_, previewPoint_);
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
        
        // Draw selection rectangle more concisely
        qreal padding = strokeWidth_ / 2.0 + selectionGrowth_;
        QPointF rectPoints[4] = {
            mapToItemRotation(minX_ - padding, minY_ - padding), // TL
            mapToItemRotation(maxX_ + padding, minY_ - padding), // TR
            mapToItemRotation(maxX_ + padding, maxY_ + padding), // BR
            mapToItemRotation(minX_ - padding, maxY_ + padding)  // BL
        };
        for(int i = 0; i < 4; i++)
            painter->drawLine(rectPoints[i], rectPoints[(i+1)%4]);
        
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        // Calculate common values
        QPointF center(0.5 * (maxX_ + minX_), 0.5 * (maxY_ + minY_));
        qreal handleHalf = handleD_ / 2.0;
        qreal handleOffset = handleHalf + handleGrowth_;
        
        // Define handle positions in object space
        QPointF cornerOffsets[4] = {
            {minX_ - handleOffset, minY_ - handleOffset}, // UL
            {maxX_ + handleOffset, minY_ - handleOffset}, // UR
            {maxX_ + handleOffset, maxY_ + handleOffset}, // DR
            {minX_ - handleOffset, maxY_ + handleOffset}  // DL
        };
        
        QPointF edgeOffsets[4] = {
            {center.x(), minY_ - handleOffset}, // Up
            {maxX_ + handleOffset, center.y()}, // Right
            {center.x(), maxY_ + handleOffset}, // Down
            {minX_ - handleOffset, center.y()}  // Left
        };
        
        // Transform all positions and create handle rectangles
        QPointF corners[4], edges[4];
        QRectF* handlePtrs[8] = {&ULHandle, &URHandle, &DRHandle, &DLHandle, 
                                &UHandle, &RHandle, &DHandle, &LHandle};
        QRectF* rotHandlePtrs[4] = {&ULRotationHandle, &URRotationHandle, 
                                    &DRRotationHandle, &DLRotationHandle};

        for(int i = 0; i < 4; i++) {
            corners[i] = mapToItemRotation(cornerOffsets[i]);
            edges[i] = mapToItemRotation(edgeOffsets[i]);
            
            // Corner handles
            *handlePtrs[i] = QRectF(corners[i].x() - handleHalf, 
                                    corners[i].y() - handleHalf, 
                                    handleD_, handleD_);
            
            // Rotation handles (use same positions as corners)
            *rotHandlePtrs[i] = QRectF(corners[i].x() - handleHalf, 
                                    corners[i].y() - handleHalf, 
                                    handleD_, handleD_);
        }

        for(int i = 0; i < 4; i++) {
            // Edge handles
            *handlePtrs[i+4] = QRectF(edges[i].x() - handleHalf, 
                                    edges[i].y() - handleHalf, 
                                    handleD_, handleD_);
        }

        // Load SVG renderers (consider making these static class members)
        static QSvgRenderer  PDiagonalArrow(QString(":/Handles/icons/PDiagonalArrows.svg"));
        static QSvgRenderer  NDiagonalArrow(QString(":/Handles/icons/NDiagonalArrows.svg"));
        static QSvgRenderer         UDArrow(QString(":/Handles/icons/UDArrows.svg"));
        static QSvgRenderer         LRArrow(QString(":/Handles/icons/LRArrows.svg"));
        static QSvgRenderer URRotationArrow(QString(":/Handles/icons/URcornerArrow.svg"));
        static QSvgRenderer ULRotationArrow(QString(":/Handles/icons/ULcornerArrow.svg"));
        static QSvgRenderer DRRotationArrow(QString(":/Handles/icons/DRcornerArrow.svg"));
        static QSvgRenderer DLRotationArrow(QString(":/Handles/icons/DLcornerArrow.svg"));

        painter->save();
        
        if(inRotationMode_) {
            // Rotation handles - all use same rotation
            QSvgRenderer* rotSvgs[4] = {&ULRotationArrow, &URRotationArrow, 
                                        &DRRotationArrow, &DLRotationArrow};
            for(int i = 0; i < 4; i++) {
                painter->save();
                painter->translate(corners[i]);
                painter->rotate(rotation_);
                rotSvgs[i]->render(painter, QRectF(-handleHalf, -handleHalf, handleD_, handleD_));
                painter->restore();
            }
        }
        else {
            // Scale corner handles
            QSvgRenderer* cornerSvgs[4] = {&NDiagonalArrow, &PDiagonalArrow, 
                                        &NDiagonalArrow, &PDiagonalArrow};
            for(int i = 0; i < 4; i++) {
                painter->save();
                painter->translate(corners[i]);
                painter->rotate(rotation_);
                cornerSvgs[i]->render(painter, QRectF(-handleHalf, -handleHalf, handleD_, handleD_));
                painter->restore();
            }
            
            // Edge handles
            QSvgRenderer* edgeSvgs[4] = {&UDArrow, &LRArrow, &UDArrow, &LRArrow};
            for(int i = 0; i < 4; i++) {
                painter->save();
                painter->translate(edges[i]);
                painter->rotate(rotation_);
                edgeSvgs[i]->render(painter, QRectF(-handleHalf, -handleHalf, handleD_, handleD_));
                painter->restore();
            }
        }
        
        painter->restore();
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
        for(int i = 0; i<drawnNodes_.size(); i++){
            
            // draw bezier handles for current node if they exist
            if((drawnNodes_[i]->mode == 'S' || drawnNodes_[i]->mode == 'M') && drawnNodes_[i]->H1 != nullptr && drawnNodes_[i]->H2 != nullptr){
                painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
                painter->setBrush(Qt::NoBrush);

                painter->drawLine(drawnNodes_[i]->position_, drawnNodes_[i]->H1->position_);
                painter->drawLine(drawnNodes_[i]->position_, drawnNodes_[i]->H2->position_);

                painter->setBrush(QBrush(Qt::white));
                painter->setPen(QPen(Qt::black, 1));
                painter->drawEllipse(drawnNodes_[i]->H1->position_, handleD_/2, handleD_/2);
                painter->drawEllipse(drawnNodes_[i]->H2->position_, handleD_/2, handleD_/2);
            }

            if(drawnNodes_[i]->isHighlighted())
                painter->setBrush(QBrush(QColor("#2A7FFF")));
            else
                painter->setBrush(handleBrush_);

            switch (drawnNodes_[i]->mode) // L: linear (rhombus), M: smooth (circle),  S: symmetric (square)
            {
            case 'L':
                Rhombus.translate(drawnNodes_[i]->position_ - QPoint(5,5));
                painter->drawPath(Rhombus);
                
                // painter->drawText(originalNodes_[i]->position_, QString("(%1,%2)").arg(originalNodes_[i]->position_.x()).arg(originalNodes_[i]->position_.y())); //debug point positions

                Rhombus.translate(-1 * drawnNodes_[i]->position_ + QPoint(5,5)); //reset the rohumbus to the original position to be moved in the next iteration (next node)
                break;
            case 'M':
                painter->drawEllipse(
                    drawnNodes_[i]->position_ - QPointF(handleD_/2, handleD_/2),
                    handleD_, 
                    handleD_);
                break;
            case 'S':
                painter->drawRect(
                    drawnNodes_[i]->position_.x() - handleD_/2,
                    drawnNodes_[i]->position_.y() - handleD_/2,
                    handleD_,
                    handleD_);
                break;
            }
        }
    }

    // Snapping rectangle to the first point
    if(firstPointHighlighted_){
        painter->setCompositionMode(QPainter::CompositionMode_Difference);
        painter->setPen(QPen(QColor("#BEBEBE"), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);

        QPointF p = actualNodes_[0]->position_;
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
    }

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
            QPointF firstPoint = currentPath->getPoint(0);
                    
            bool snap =
                std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;
            
            currentPath->setSnapping(snap);

            if (snap) {
                currentPath->addEdge(lastPointIndex, 0);
                currentPath->clearPreviewPoint();
                currentPath->setDrawingMode(false);
                currentPath->showSnapMargin(false);
            
                startedNewPath_ = false;

                currentPath->calculateBoundaries();
                currentPath->position_ = {(currentPath->minX_ + currentPath->maxX_)/2.0, (currentPath->minY_ + currentPath->maxY_)/2.0};
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
                if(selectedPath_ != clickedPath)
                    clickedPath->recentlySelected_ = true;

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

        if(selectedPath_ != nullptr){
            scaling_ =  !selectedPath_->inRotationMode() && event->button() == Qt::LeftButton &&
                        (selectedPath_->URHandle.contains(canvasLocalPos) ||
                         selectedPath_->ULHandle.contains(canvasLocalPos) ||
                         selectedPath_->DRHandle.contains(canvasLocalPos) ||
                         selectedPath_->DLHandle.contains(canvasLocalPos) ||
                          selectedPath_->UHandle.contains(canvasLocalPos) ||
                          selectedPath_->DHandle.contains(canvasLocalPos) ||
                          selectedPath_->RHandle.contains(canvasLocalPos) ||
                          selectedPath_->LHandle.contains(canvasLocalPos)
                        );
            rotating_ = selectedPath_->inRotationMode() && event->button() == Qt::LeftButton &&
                        (selectedPath_->URRotationHandle.contains(canvasLocalPos) ||
                         selectedPath_->ULRotationHandle.contains(canvasLocalPos) ||
                         selectedPath_->DRRotationHandle.contains(canvasLocalPos) ||
                         selectedPath_->DLRotationHandle.contains(canvasLocalPos)
                        );

            if(scaling_){
                holdStartPosition_ = canvasLocalPos;
                holding_ = true;

                if (selectedPath_->ULHandle.contains(canvasLocalPos)) activeScaleHandle_ = TopLeft;
                else if (selectedPath_->URHandle.contains(canvasLocalPos)) activeScaleHandle_ = TopRight;
                else if (selectedPath_->DLHandle.contains(canvasLocalPos)) activeScaleHandle_ = BottomLeft;
                else if (selectedPath_->DRHandle.contains(canvasLocalPos)) activeScaleHandle_ = BottomRight;
                else if (selectedPath_->UHandle.contains(canvasLocalPos)) activeScaleHandle_ = Top;
                else if (selectedPath_->DHandle.contains(canvasLocalPos)) activeScaleHandle_ = Bottom;
                else if (selectedPath_->LHandle.contains(canvasLocalPos)) activeScaleHandle_ = Left;
                else if (selectedPath_->RHandle.contains(canvasLocalPos)) activeScaleHandle_ = Right;

                if(activeScaleHandle_ == Left || activeScaleHandle_ == TopLeft || activeScaleHandle_ == BottomLeft){
                    scalingError_.setX(selectedPath_->minX_ - holdStartPosition_.x());
                }
                if(activeScaleHandle_ == Right || activeScaleHandle_ == TopRight || activeScaleHandle_ == BottomRight){
                    scalingError_.setX(holdStartPosition_.x() - selectedPath_->maxX_);
                }

                if(activeScaleHandle_ == Top || activeScaleHandle_ == TopLeft || activeScaleHandle_ == TopRight){
                    scalingError_.setY(selectedPath_->minY_ - holdStartPosition_.y());
                }
                if(activeScaleHandle_ == Bottom || activeScaleHandle_ ==  BottomLeft || activeScaleHandle_ == BottomRight){
                    scalingError_.setY(holdStartPosition_.y() - selectedPath_->maxY_);
                }
    
                selectedPath_->update();
                return;
            }
            else if(rotating_){
                holding_ = true;
                holdStartPosition_ = canvasLocalPos;

                return;
            }
        }

        if(clickedPath){
            if(selectedPath_ != clickedPath)
                clickedPath->recentlySelected_ = true;

            setSelectedPath(clickedPath, true);
            holding_ = true;
            holdStartPosition_ = canvasLocalPos;
            return;
        }

        setSelectedPath(nullptr, false);
        
    }
}

void viewPort::mouseMoveEvent(QMouseEvent *event)
{    

    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);
    
    if(panning_ && holding_){
        QPointF delta = scenePos - panStartScenePos_;
        canvas_->setPos(panStartCanvasPos_ + delta);

        holdStartPosition_ = canvasLocalPos; // needs to be removed

        canvas_->update();
        return;
    }

    if(!selectedPath_) return;
    
    if(bezierToolActivated_){
        QPointF firstPoint = selectedPath_->getPoint(0);
        snap_ =  std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;

        selectedPath_->setSnapping(snap_);
    
        QPointF target = snap_ ? firstPoint : canvasLocalPos;
        selectedPath_->showSnapMargin(snap_);
    
        if (startedNewPath_ && holding_) {
            selectedPath_->clearPreviewPoint();
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
    else if(nodeToolActivated_){
        if(selectedPath_ != nullptr && selectedPath_->nodesHighlighted() > 0 && holding_){
            for(int i = 0; i < selectedPath_->nodesHighlighted(); i++){
                selectedPath_->moveNode(canvasLocalPos - holdStartPosition_, selectedPath_->accessHighlightedVector(i));
            }
            holdStartPosition_ = canvasLocalPos; // needs to be removed

            selectedPath_->calculateBoundaries();
            selectedPath_->update();
        }
    }
    else if(selectionToolActivated_){
        if(holding_ && !scaling_ && !rotating_){
            selectedPath_->movePath(-1 * offset_);
            offset_ = canvasLocalPos - holdStartPosition_;
            selectedPath_->movePath(offset_);
        }
        else if(holding_ && rotating_){
        
            QPointF rotationCenter = QPointF(0.5 * (selectedPath_->maxX_ + selectedPath_->minX_), 0.5 * (selectedPath_->maxY_ + selectedPath_->minY_));
            QPointF startVector = holdStartPosition_ - rotationCenter;
            QPointF endVector =  canvasLocalPos - rotationCenter;
            qreal startAngle = std::atan2(startVector.x(), startVector.y());
            qreal endAngle = std::atan2(endVector.x(), endVector.y());
            qreal angleDifference = (endAngle - startAngle) * 180.0 / M_PI;
            selectedPath_->rotate(-1 * angleDifference);
            selectedPath_->update();

            holdStartPosition_ = canvasLocalPos; // needs to be removed
        }
        else if(holding_ && scaling_){
            QPointF delta = canvasLocalPos - selectedPath_->position_;

            if(shifting_ && (activeScaleHandle_ == TopRight || activeScaleHandle_ == BottomRight || activeScaleHandle_ == TopLeft || activeScaleHandle_ == BottomLeft)){
                delta.setX(std::max(abs(delta.x()), abs(delta.y())));
                delta.setY(std::max(abs(delta.x()), abs(delta.y())));
            }

            if(activeScaleHandle_ == Left || activeScaleHandle_ == TopLeft || activeScaleHandle_ == BottomLeft){
                delta.setX(delta.x() * -1);
            }
            if(activeScaleHandle_ == Top || activeScaleHandle_ == TopLeft || activeScaleHandle_ == TopRight){
                delta.setY(delta.y() * -1);
            }

            delta -= scalingError_;

            if(activeScaleHandle_ == Right || activeScaleHandle_ == Left){
                delta.setY(0);
            }
            if(activeScaleHandle_ == Top || activeScaleHandle_ == Bottom){
                delta.setX(0);
            }


            selectedPath_->rescale(delta.x(), delta.y());
            // selectedPath_->update();
        }
    }
    
    selectedPath_->update();
}

void viewPort::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        setCursor(Qt::ArrowCursor);
        panning_ = false;
    }

    
    QPointF scenePos = mapToScene(event->pos());
    QPointF canvasLocalPos = canvas_->mapFromScene(scenePos);

    if(rotating_){
        rotating_ = false;
        if(selectedPath_ != nullptr)
            selectedPath_->update();
    }
    else if(scaling_){
        scaling_ = false;
        if(selectedPath_ != nullptr){
            selectedPath_->calculateBoundaries();
            selectedPath_->update();
        }
    }
    else if(QLineF(holdStartPosition_, canvasLocalPos).length() < 2 && selectedPath_){
        if(!selectedPath_->recentlySelected_)
            selectedPath_->toggleRotationMode();
        else
            selectedPath_->recentlySelected_ = false;
    }
    
    offset_ = QPointF(0,0);    
    holding_ = false;
    if(selectedPath_){
        selectedPath_->update();
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
