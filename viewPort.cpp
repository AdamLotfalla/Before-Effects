#include "viewPort.h"
#include <QDebug>

QSvgRenderer path::PDiagonalArrow(QString(":/Handles/icons/PDiagonalArrows.svg"));
QSvgRenderer path::NDiagonalArrow(QString(":/Handles/icons/NDiagonalArrows.svg"));
QSvgRenderer path::UDArrow(QString(":/Handles/icons/UDArrows.svg"));
QSvgRenderer path::LRArrow(QString(":/Handles/icons/LRArrows.svg"));
QSvgRenderer path::URRotationArrow(QString(":/Handles/icons/URcornerArrow.svg"));
QSvgRenderer path::ULRotationArrow(QString(":/Handles/icons/ULcornerArrow.svg"));
QSvgRenderer path::DRRotationArrow(QString(":/Handles/icons/DRcornerArrow.svg"));
QSvgRenderer path::DLRotationArrow(QString(":/Handles/icons/DLcornerArrow.svg"));
QSvgRenderer path::PivotMark(QString(":/Handles/icons/PivotMark.svg"));


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
    qreal actualMinX = std::min(minX_, maxX_);
    qreal actualMaxX = std::max(minX_, maxX_);
    qreal actualMinY = std::min(minY_, maxY_);
    qreal actualMaxY = std::max(minY_, maxY_);

    qreal pad = strokeWidth_ / 2.0 + selectionGrowth_ + handleD_ + 4.0;

    // The four corners of the padded unrotated box
    QPointF corners[4] = {
        {actualMinX - pad, actualMinY - pad},
        {actualMaxX + pad, actualMinY - pad},
        {actualMaxX + pad, actualMaxY + pad},
        {actualMinX - pad, actualMaxY + pad}
    };

    // Rotate each corner around the pivot
    qreal minX =  std::numeric_limits<qreal>::max();
    qreal maxX =  std::numeric_limits<qreal>::lowest();
    qreal minY =  std::numeric_limits<qreal>::max();
    qreal maxY =  std::numeric_limits<qreal>::lowest();

    for (const QPointF& c : corners) {
        QPointF rotated = mapToItemRotation(c);
        minX = std::min(minX, rotated.x());
        maxX = std::max(maxX, rotated.x());
        minY = std::min(minY, rotated.y());
        maxY = std::max(maxY, rotated.y());
    }

    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

void path::addPoint(QPointF point)
{
    prepareGeometryChange();
    needTransformUpdate_ = true;
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

QPointF path::getActualPoint(int index)
{
    return actualNodes_[index]->position_;
}

QPointF path::getDrawnPoint(int index)
{
    updateTransformedNodes();
    return drawnNodes_[index]->position_;
}

void path::showSnapMargin(bool state)
{
    firstNodeHighlighted_ = state;
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

void path::changeNodeMode(handleMode newMode, int index)
{
    needTransformUpdate_ = true;
    actualNodes_[index]->mode = newMode;
    if(newMode == handleMode::smooth || newMode == handleMode::symmetric){
        if(!actualNodes_[index]->H1) actualNodes_[index]->H1 = new bezierHandle(getActualPoint(index) - QPointF(10,10));
        if(!actualNodes_[index]->H2) actualNodes_[index]->H2 = new bezierHandle(getActualPoint(index) + QPointF(10,10));
    }
}

void path::incrementNodeMode(int index)
{
    actualNodes_[index]->mode = static_cast<handleMode>((static_cast<int>(actualNodes_[index]->mode) + 1) % 3);
    updateTransformedNodes();
}

void path::moveBezierHandle(QPointF newPosition, int index, int handleIndex)
{
    needTransformUpdate_ = true;
    node* currentNode = actualNodes_[index];

    if (currentNode->H1 == nullptr && handleIndex == 1){
        currentNode->H1 = new bezierHandle(currentNode->position_ - (newPosition - currentNode->position_));
    }

    if(currentNode->H2 == nullptr && handleIndex == 2){
        currentNode->H2 = new bezierHandle(newPosition);
    }
    

    switch (handleIndex)
    {
    case 1:
        currentNode->H1->position_ = newPosition;
        if(currentNode->mode == handleMode::symmetric){
            currentNode->H2->position_ = currentNode->position_ - (newPosition - currentNode->position_);
        }
        break;
    case 2:
        currentNode->H2->position_ = newPosition;
        if(currentNode->mode == handleMode::symmetric){
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

    QPointF center = position_ + pivotPoint_;
    
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
    prepareGeometryChange();
    needTransformUpdate_ = true;
    rotation_ += angle;

    if(onRotationChanged)
        onRotationChanged(rotation_ + angle);

    calculateBoundaries();
    update();
}

void path::setRotation(float angle)
{
    prepareGeometryChange();
    rotation_ = angle;

    if(onRotationChanged)
        onRotationChanged(angle);

    calculateBoundaries();
    update();
}

QPointF path::mapToItemRotation(const QPointF& point, const bool reverse) const {
    if (qFuzzyCompare(rotation_, 0.0f)) return point;
    
    QPointF center = position_ + pivotPoint_;
    
    // Translate to origin
    QPointF translated = point - center;
    
    // Apply inverse rotation
    qreal radians = (reverse ? -rotation_ : rotation_) * M_PI / 180.0;
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

void path::rescale(qreal xCenterD, qreal yCenterD, QPointF error,
                   qreal originalHalfExtentX, qreal originalHalfExtentY,
                   bool restrictedX, bool restrictedY,
                   bool flipX, bool flipY)
{
    prepareGeometryChange();
    needTransformUpdate_ = true;

    qreal xRotatedCenterD = yCenterD * sin(rotation_ * M_PI / 180.0) + xCenterD * cos(rotation_ * M_PI / 180.0);
    qreal yRotatedCenterD = yCenterD * cos(rotation_ * M_PI / 180.0) - xCenterD * sin(rotation_ * M_PI / 180.0);

    if(flipX) xRotatedCenterD *= -1;
    if(flipY) yRotatedCenterD *= -1;

    // Use frozen press-time extents — never recomputed, numerically stable at zero
    qreal xOriginalMax = position_.x() + originalHalfExtentX;
    qreal xOriginalMin = position_.x() - originalHalfExtentX;
    qreal yOriginalMax = position_.y() + originalHalfExtentY;
    qreal yOriginalMin = position_.y() - originalHalfExtentY;

    qreal xm = std::min((xRotatedCenterD - (xOriginalMax - position_.x())),
                        (xRotatedCenterD - (xOriginalMin - position_.x())));
    qreal ym = std::min((yRotatedCenterD - (yOriginalMax - position_.y())),
                        (yRotatedCenterD - (yOriginalMin - position_.y())));

    qreal xUnscaledCenterD = xRotatedCenterD - xm;
    qreal yUnscaledCenterD = yRotatedCenterD - ym;

    if(std::abs(xRotatedCenterD) < std::abs(error.x())) error.setX(xRotatedCenterD);
    else if(xRotatedCenterD < error.x()) error.setX(error.x() * -1);

    if(std::abs(yRotatedCenterD) < std::abs(error.y())) error.setY(yRotatedCenterD);
    else if(yRotatedCenterD < error.y()) error.setY(error.y() * -1);

    if(yUnscaledCenterD != 0 && yRotatedCenterD != 0 && !restrictedY)
        scaleY_ = (yRotatedCenterD - error.y()) / yUnscaledCenterD;
    if(xUnscaledCenterD != 0 && xRotatedCenterD != 0 && !restrictedX)
        scaleX_ = (xRotatedCenterD - error.x()) / xUnscaledCenterD;

    if(onScaleChanged)
        onScaleChanged(scaleX_, scaleY_);

    calculateBoundaries();
    update();
}

void path::setScale(qreal newScaleX, qreal newScaleY)
{
    prepareGeometryChange();
    needTransformUpdate_ = true;
    scaleX_ = newScaleX;
    scaleY_ = newScaleY;

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

bool path::getHandleExistance(int index, short int handleIndex)
{
    if(handleIndex == 0){
        return actualNodes_[index]->H1 != nullptr;
    }
    else if(handleIndex == 1){
        return actualNodes_[index]->H2 != nullptr;
    }
    return false;
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
    needTransformUpdate_ = true;
    
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
    if(onPositionChanged)
        onPositionChanged(position_);
    
    calculateBoundaries();
}

void path::setPosition(QPointF newPos)
{
    movePath(newPos - position_);
    if(onPositionChanged)
        onPositionChanged(position_);
}

void path::setPosition(qreal x, qreal y)
{
    setPosition(QPointF(x,y));
}

void path::moveNode(QPointF offset, int index)
{
    prepareGeometryChange();
    needTransformUpdate_ = true;

    actualNodes_[index]->position_ += (offset);
    if(actualNodes_[index]->H1) actualNodes_[index]->H1->position_ += offset;
    if(actualNodes_[index]->H2) actualNodes_[index]->H2->position_ += offset;

    calculateBoundaries();
}

void path::setNodePosition(QPointF newPos, int index)
{
    prepareGeometryChange();
    needTransformUpdate_ = true;

    QPointF oldPos = actualNodes_[index]->position_;
    actualNodes_[index]->position_ = newPos;

    QPointF offset = newPos - oldPos;
    if(actualNodes_[index]->H1) actualNodes_[index]->H1->position_ += offset;
    if(actualNodes_[index]->H2) actualNodes_[index]->H2->position_ += offset;

    calculateBoundaries();
}

QPointF path::getActualHandlePosition(int index, short int HandleIndex)
{
    if(HandleIndex == 0){
        return actualNodes_[index]->H1->position_;
    }
    else if(HandleIndex == 1){
        return actualNodes_[index]->H2->position_;
    }

    return QPointF(0,0);
}

QPointF path::getDrawnHandlePosition(int index, short int HandleIndex)
{
    if(HandleIndex == 0){
        return drawnNodes_[index]->H1->position_;
    }
    else if(HandleIndex == 1){
        return drawnNodes_[index]->H2->position_;
    }

    return QPointF(0,0);
}

void path::setHandlePosition(QPointF newPosition, int index, short int HandleIndex)
{
    if(HandleIndex == 0){
        actualNodes_[index]->H1->position_ = newPosition;
        if(actualNodes_[index]->mode == handleMode::symmetric){
            actualNodes_[index]->H2->position_ = 2.0 * actualNodes_[index]->position_ - newPosition;
        }
    }
    else if(HandleIndex == 1){
        actualNodes_[index]->H2->position_ = newPosition;
        if(actualNodes_[index]->mode == handleMode::symmetric){
            actualNodes_[index]->H1->position_ = 2.0 * actualNodes_[index]->position_ - newPosition;
        }
    }
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

            handleMode startMode = actualNodes_[i]->mode;
            handleMode endMode = actualNodes_[j]->mode;

            if ((endMode == handleMode::symmetric || endMode == handleMode::smooth) && startMode == handleMode::linear)
            {
                QPointF P1 = transformPoint(actualNodes_[j]->H1->position_);
                p.quadTo(P1, P3);
            }
            else if ((endMode == handleMode::symmetric || endMode == handleMode::smooth) && (startMode == handleMode::symmetric || startMode == handleMode::smooth))
            {
                QPointF P1 = transformPoint(actualNodes_[i]->H2->position_);
                QPointF P2 = transformPoint(actualNodes_[j]->H1->position_);
                p.cubicTo(P1, P2, P3);
            }
            else if (endMode == handleMode::linear && (startMode == handleMode::symmetric || startMode == handleMode::smooth))
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

void path::updateTransformedNodes()
{
    if (!needTransformUpdate_) return;
    
    for (node* n : drawnNodes_)
        delete n;
    drawnNodes_.clear();

    
    QPointF globalPivotPoint = position_ + pivotPoint_;

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

    needTransformUpdate_ = false;
}

void path::calculateBoundaries()
{
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

        QPointF P0 = scalePoint(actualNodes_[i]->position_), P1;

        //quadratic
        //B(t) = (1-t)^2 P0 + 2(1-t)t P1 + t^2 P2
        //B'(0) = -2P0 + 2P1
        //B'(1) = -2P1 + 2P2
        //stationary point when t = (p0 - p1)/(p0 - 2p1 + p2)

        //cubic
        //B(t) = (1-t)^3 P0 + 3t(1-t)^2 P1 + 3(1-t)t^2 P2 + t^3 P3
        //B'(0) = -3P0 + 3P1
        //B'(1) = -3P2 + 3P3
        // B' = dB/dt
        // slope: dy/dx = dy/dt ÷ dx/dt
        // this is the same as the slope equation for the line connecting the point and its control point. depends only on the point (for smooth nodes)


        //correction for miter joints with thick stroke: Smooth nodes
        if(actualNodes_[i]->mode == handleMode::smooth){
            P1 = scalePoint(actualNodes_[i]->H2->position_);

            qreal initialSlopeAngle = atan2((P0 - scalePoint(actualNodes_[i]->H1->position_)).y(), (P0 - scalePoint(actualNodes_[i]->H1->position_)).x());
            qreal terminalSlopeAngle = atan2((P1 - P0).y(), (P1 - P0).x());

            if(sin(initialSlopeAngle * 0.5 + terminalSlopeAngle * 0.5) == 0) return; //this will happen if the two lines meet at 180°
            qreal d = std::min(strokeWidth_ * 0.5 / sin(initialSlopeAngle * 0.5 + terminalSlopeAngle * 0.5), 10.0); //10 is the default miter limit

            qreal theta = (abs(initialSlopeAngle) + abs(terminalSlopeAngle))/2.0 + std::min(initialSlopeAngle, terminalSlopeAngle);
            QPointF miterPoint = P0 + QPointF(d*cos(theta), d*sin(theta));

            recalculateBoundariesForPoint(miterPoint);
        }


        for(int nextNodeIndex : edges_[i]){

            qreal x = actualNodes_[i]->position_.x();
            qreal y = actualNodes_[i]->position_.y();


            QPointF P2, P3 = scalePoint(actualNodes_[nextNodeIndex]->position_);
            
            qreal tx,ty;
            qreal stationaryPx, stationaryPy;
            
            handleMode startMode = actualNodes_[i]->mode;
            handleMode endMode = actualNodes_[nextNodeIndex]->mode;


            if((endMode == handleMode::symmetric || endMode == handleMode::smooth) && (startMode == handleMode::symmetric || startMode == handleMode::smooth)){
                P1 = scalePoint(actualNodes_[i]->H2->position_);
                P2 = scalePoint(actualNodes_[nextNodeIndex]->H1->position_);


                
                //calculate t using the derivative
                QPointF a = -3.0 * P0 + 9.0 * P1 - 9.0 * P2 + 3.0 * P3;
                QPointF b = 6.0 * P0 - 12.0 * P1 + 6.0 * P2;
                QPointF c = -3.0 * P0 + 3.0 * P1;
                
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
            }
            else if(endMode == handleMode::linear && (startMode == handleMode::symmetric || startMode == handleMode::smooth)){
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

                //correction for miter joint for the end (linear) node 
                qreal initialSlopeAngle = atan2((P2-P1).y(), (P2-P1).x());
                for(int k : edges_[nextNodeIndex]){
                    qreal terminalSlopeAngle = atan2((P2 - scalePoint(getActualPoint(k))).y(), (P2 - scalePoint(getActualPoint(k))).x());

                    if(sin(initialSlopeAngle * 0.5 + terminalSlopeAngle * 0.5) == 0) return; //this will happen if the two lines meet at 180°
                    qreal d = std::min(strokeWidth_ * 0.5 / sin(initialSlopeAngle * 0.5 + terminalSlopeAngle * 0.5), 10.0); //10 is the default miter limit

                    qreal theta = (abs(initialSlopeAngle) + abs(terminalSlopeAngle))/2.0 + std::min(initialSlopeAngle, terminalSlopeAngle);
                    QPointF miterPoint = P0 + QPointF(d*cos(theta), d*sin(theta));

                    recalculateBoundariesForPoint(miterPoint);
                }
            }
            else if((endMode == handleMode::symmetric || endMode == handleMode::smooth) && startMode == handleMode::linear){
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


                //correction for miter joint for the start (linear) node 
                    //MISSING (I think it is unnecessary because I already implemented it once in the previous if())
            }
            else if(endMode == handleMode::linear && startMode == handleMode::linear){
                //correction for miter joint (linear -- linear)
                P1 = P3;
                qreal initialSlopeAngle = atan2((P1 - P0).y(), (P1-P0).x());
                for(auto k : edges_[nextNodeIndex]){
                    if(actualNodes_[k]->mode == handleMode::linear){
                        qreal terminalSlopeAngle = atan2((scalePoint(getActualPoint(k)) - P1).y(),(scalePoint(getActualPoint(k)) - P1).x());
                        if(sin(initialSlopeAngle * 0.5 + terminalSlopeAngle * 0.5) == 0) return; //this will happen if the two lines meet at 180°
                        qreal d = std::min(strokeWidth_ * 0.5 / sin(initialSlopeAngle * 0.5 + terminalSlopeAngle * 0.5), 10.0); //10 is the default miter limit

                        qreal theta = (abs(initialSlopeAngle) + abs(terminalSlopeAngle))/2.0 + std::min(initialSlopeAngle, terminalSlopeAngle);
                        QPointF miterPoint = P0 + QPointF(d*cos(theta), d*sin(theta));

                        recalculateBoundariesForPoint(miterPoint);
                    }
                }
            }
        }


        recalculateBoundariesForPoint(scalePoint(actualNodes_[i]->position_));

    }


    qreal temporary;
    if(scaleX_ < 0){ //switch variables
        temporary = minX_;
        minX_ = maxX_;
        maxX_ = temporary;
    }

    if(scaleY_ < 0){
        temporary = minY_;
        minY_ = maxY_;
        maxY_ = temporary;
    }
}

void path::makeDirty()
{
    needTransformUpdate_ = true;
}

QWidget *path::createAttributeWidget(QWidget *parent)
{
    if (cachedAttributeWidget_ && cachedAttributeWidget_->parent() == parent) {
        return cachedAttributeWidget_;
    }
    
    if (cachedAttributeWidget_) {
        delete cachedAttributeWidget_;
        cachedAttributeWidget_ = nullptr;
    }

    QWidget* background = new QWidget(parent);
    QVBoxLayout* VLayout = new QVBoxLayout(background);
    background->setLayout(VLayout);   
    
    QLabel* transformTitle = new QLabel("Transformation");
    QPalette palette = transformTitle->palette();
    palette.setColor(QPalette::WindowText, QColor("#888888"));
    transformTitle->setPalette(palette);
    VLayout->addWidget(transformTitle);

    
    QHBoxLayout* PosLayout = new QHBoxLayout();

    customSpinBox* xPositionBox = new customSpinBox(background, 'X');
    xPositionBox->setValue(position_.x());
    xPositionBox->update();
    
    customSpinBox* yPositionBox = new customSpinBox(background, 'Y');
    yPositionBox->setValue(position_.y());
    yPositionBox->update();
    
    QLabel* positionLabel = new QLabel(background);
    positionLabel->setText("Position");
    positionLabel->setMinimumWidth(20);

    PosLayout->addWidget(positionLabel);
    PosLayout->addWidget(xPositionBox);
    PosLayout->addWidget(yPositionBox);
    VLayout->addLayout(PosLayout);
    

    
    onPositionChanged = [xPositionBox, yPositionBox](QPointF pos) {
        xPositionBox->blockSignals(true);
        yPositionBox->blockSignals(true);
        xPositionBox->setValue(pos.x());
        yPositionBox->setValue(pos.y());
        xPositionBox->blockSignals(false);
        yPositionBox->blockSignals(false);
        
        xPositionBox->update();
        yPositionBox->update();
    };

    xPositionBox->connect(xPositionBox, &customSpinBox::valueChanged, [this](qreal value){
        setPosition(value, position_.y());
        update();
    });
    
    yPositionBox->connect(yPositionBox, &customSpinBox::valueChanged, [this](qreal value){
        setPosition(position_.x(), value);
        update();
    });


    QHBoxLayout* scaleLayout = new QHBoxLayout();

    customSpinBox* xScaleBox = new customSpinBox(background, 'X');
    xScaleBox->setValue(scaleX_);
    xScaleBox->setPrecision(2);
    xScaleBox->update();
    
    customSpinBox* yScaleBox = new customSpinBox(background, 'Y');
    yScaleBox->setValue(scaleY_);
    yScaleBox->setPrecision(2);
    yScaleBox->update();

    QLabel* scaleLabel = new QLabel(background);
    scaleLabel->setText("Scale");
    scaleLabel->setMinimumWidth(20);
    
    scaleLayout->addWidget(scaleLabel);
    scaleLayout->addWidget(xScaleBox);
    scaleLayout->addWidget(yScaleBox);

    VLayout->addLayout(scaleLayout);

    
    onScaleChanged = [xScaleBox, yScaleBox](qreal newScaleX, qreal newScaleY) {
        xScaleBox->blockSignals(true);
        yScaleBox->blockSignals(true);
        xScaleBox->setValue(newScaleX);
        yScaleBox->setValue(newScaleY);
        xScaleBox->blockSignals(false);
        yScaleBox->blockSignals(false);
        
        xScaleBox->update();
        yScaleBox->update();
    };

    xScaleBox->connect(xScaleBox, &customSpinBox::valueChanged, [this](qreal value){
        setScale(value, scaleY_);
        update();
    });
    
    yScaleBox->connect(yScaleBox, &customSpinBox::valueChanged, [this](qreal value){
        setScale(scaleX_, value);
        update();
    });


    QHBoxLayout* rotationLayout = new QHBoxLayout();
    rotationLayout->setSpacing(3);

    customSpinBox* rotationBox = new customSpinBox(background, 'Z');
    rotationBox->setValue(rotation_);
    rotationBox->setPrecision(2);
    rotationBox->update();

    QLabel* rotationLabel = new QLabel(background);
    rotationLabel->setText("rotation");
    rotationLabel->setMinimumWidth(20);

    rotationLayout->addWidget(rotationLabel, 1);
    rotationLayout->addWidget(rotationBox, 2);

    VLayout->addLayout(rotationLayout);

    onRotationChanged = [rotationBox](qreal newRotation){
        rotationBox->blockSignals(true);
        rotationBox->setValue(newRotation);
        rotationBox->blockSignals(false);

        rotationBox->update();
    };

    rotationBox->connect(rotationBox, &customSpinBox::valueChanged, [this](qreal value){
        setRotation(value);
        update();
    });


    QHBoxLayout* pivotLayout = new QHBoxLayout();

    customSpinBox* xPivotBox = new customSpinBox(background, 'X');
    xPivotBox->setValue(pivotPoint_.x());

    customSpinBox* yPivotBox = new customSpinBox(background, 'Y');
    yPivotBox->setValue(pivotPoint_.y());  // Fixed: was pivotPoint_.x()

    
    QLabel* pivotLabel = new QLabel(background);
    pivotLabel->setText("Pivot");
    pivotLabel->setMinimumWidth(20);
    
    pivotLayout->addWidget(pivotLabel);
    pivotLayout->addWidget(xPivotBox);
    pivotLayout->addWidget(yPivotBox);

    VLayout->addLayout(pivotLayout);
    
    xPivotBox->connect(xPivotBox, &customSpinBox::valueChanged, [this](qreal value){
        pivotPoint_.setX(value);
        update();
    });

    yPivotBox->connect(yPivotBox, &customSpinBox::valueChanged, [this](qreal value){
        pivotPoint_.setY(value);
        update();
    });


    VLayout->addSpacing(10);
    QLabel* ColorTitle = new QLabel("Color");
    ColorTitle->setPalette(palette);
    VLayout->addWidget(ColorTitle);

    // ── Fill ──────────────────────────────────────────────────────────────
    QCheckBox* fillEnable = new QCheckBox(background);

    QHBoxLayout* fillLayout = new QHBoxLayout();
    QLabel* fillLabel = new QLabel();
    fillLabel->setText("Fill");

    QVBoxLayout* fillColumnsLayout = new QVBoxLayout();
    QHBoxLayout* fillRow1Layout = new QHBoxLayout();
    QHBoxLayout* fillRow2Layout = new QHBoxLayout();

    VLayout->addLayout(fillLayout);
    fillLayout->addWidget(fillEnable, 0);
    fillLayout->addWidget(fillLabel, 1);
    fillLayout->addLayout(fillColumnsLayout, 4);
    fillColumnsLayout->addLayout(fillRow1Layout);
    fillColumnsLayout->addLayout(fillRow2Layout);

    ColorSelector* fillPreview = new ColorSelector(background);
    fillPreview->setColor(fillColor_);
    fillPreview->setDisplayMode(color_widgets::ColorPreview::DisplayMode::SplitAlpha);

    QString fillHexString = QString("%1%2%3%4")
        .arg(fillColor_.red(),   2, 16, QChar('0'))
        .arg(fillColor_.green(), 2, 16, QChar('0'))
        .arg(fillColor_.blue(),  2, 16, QChar('0'))
        .arg(fillColor_.alpha(), 2, 16, QChar('0'))
        .toUpper();

    customSpinBox* fillHexSpinBox   = new customSpinBox(background, '#');
    customSpinBox* fillAlphaSpinBox = new customSpinBox(background, 'A');
    customSpinBox* fillRSpinBox     = new customSpinBox(background, 'R');
    customSpinBox* fillGSpinBox     = new customSpinBox(background, 'G');
    customSpinBox* fillBSpinBox     = new customSpinBox(background, 'B');

    fillHexSpinBox->setStringInput(true);
    fillHexSpinBox->setValue(fillHexString);

    fillAlphaSpinBox->setMinimum(0); fillAlphaSpinBox->setMaximum(255);
    fillRSpinBox->setMinimum(0);     fillRSpinBox->setMaximum(255);
    fillGSpinBox->setMinimum(0);     fillGSpinBox->setMaximum(255);
    fillBSpinBox->setMinimum(0);     fillBSpinBox->setMaximum(255);

    fillAlphaSpinBox->setValue(fillColor_.alpha());
    fillRSpinBox->setValue(fillColor_.red());
    fillGSpinBox->setValue(fillColor_.green());
    fillBSpinBox->setValue(fillColor_.blue());

    fillRow1Layout->addWidget(fillPreview,   1);
    fillRow1Layout->addWidget(fillHexSpinBox,1);
    fillRow1Layout->addWidget(fillAlphaSpinBox, 1);
    fillRow2Layout->addWidget(fillRSpinBox);
    fillRow2Layout->addWidget(fillGSpinBox);
    fillRow2Layout->addWidget(fillBSpinBox);

    auto updateFillControls = [this, fillPreview, fillRSpinBox, fillGSpinBox, fillBSpinBox, fillAlphaSpinBox, fillHexSpinBox]() {
        fillRSpinBox->blockSignals(true);
        fillGSpinBox->blockSignals(true);
        fillBSpinBox->blockSignals(true);
        fillAlphaSpinBox->blockSignals(true);
        fillHexSpinBox->blockSignals(true);

        fillRSpinBox->setValue(fillColor_.red());
        fillGSpinBox->setValue(fillColor_.green());
        fillBSpinBox->setValue(fillColor_.blue());
        fillAlphaSpinBox->setValue(fillColor_.alpha());
        fillHexSpinBox->setValue(QString("%1%2%3%4")
            .arg(fillColor_.red(),   2, 16, QChar('0'))
            .arg(fillColor_.green(), 2, 16, QChar('0'))
            .arg(fillColor_.blue(),  2, 16, QChar('0'))
            .arg(fillColor_.alpha(), 2, 16, QChar('0'))
            .toUpper());
        fillPreview->setColor(fillColor_);

        fillRSpinBox->blockSignals(false);
        fillGSpinBox->blockSignals(false);
        fillBSpinBox->blockSignals(false);
        fillAlphaSpinBox->blockSignals(false);
        fillHexSpinBox->blockSignals(false);
    };

    fillHexSpinBox->connect(fillHexSpinBox, &customSpinBox::stringValueChanged, [this, updateFillControls](QString value){
        if(value[0] == '#') value.remove(0, 1);
        bool ok; int r, g, b, a = fillColor_.alpha();
        if(value.length() >= 6){
            r = value.mid(0,2).toInt(&ok,16); g = value.mid(2,2).toInt(&ok,16); b = value.mid(4,2).toInt(&ok,16);
            if(value.length() >= 8) a = value.mid(6,2).toInt(&ok,16);
            fillColor_ = QColor(r,g,b,a); updateFillControls(); update();
        }
    });
    fillRSpinBox->connect(fillRSpinBox,    &customSpinBox::valueChanged, [this, updateFillControls](qreal v){ fillColor_.setRed(v);   updateFillControls(); update(); });
    fillGSpinBox->connect(fillGSpinBox,    &customSpinBox::valueChanged, [this, updateFillControls](qreal v){ fillColor_.setGreen(v); updateFillControls(); update(); });
    fillBSpinBox->connect(fillBSpinBox,    &customSpinBox::valueChanged, [this, updateFillControls](qreal v){ fillColor_.setBlue(v);  updateFillControls(); update(); });
    fillAlphaSpinBox->connect(fillAlphaSpinBox,&customSpinBox::valueChanged, [this, updateFillControls](qreal v){ fillColor_.setAlpha(v); updateFillControls(); update(); });
    fillPreview->connect(fillPreview, &ColorSelector::colorSelected, [this, updateFillControls](QColor c){ fillColor_ = c; updateFillControls(); update(); });

    fillEnable->connect(fillEnable, &QCheckBox::toggled, [this, fillPreview, fillHexSpinBox, fillAlphaSpinBox, fillRSpinBox, fillGSpinBox, fillBSpinBox](bool state){
        fill_ = state;
        fillPreview->setDisabled(!state);
        fillHexSpinBox->setDisabled(!state);
        fillAlphaSpinBox->setDisabled(!state);
        fillRSpinBox->setDisabled(!state);
        fillGSpinBox->setDisabled(!state);
        fillBSpinBox->setDisabled(!state);
        update();
    });


    // ── Stroke ────────────────────────────────────────────────────────────
    QCheckBox* strokeEnable = new QCheckBox(background);
    strokeEnable->setChecked(stroke_);

    QHBoxLayout* strokeLayout = new QHBoxLayout();
    QLabel* strokeLabel = new QLabel();
    strokeLabel->setText("Stroke");

    QVBoxLayout* strokeColumnsLayout = new QVBoxLayout();
    QHBoxLayout* strokeRow1Layout = new QHBoxLayout();
    QHBoxLayout* strokeRow2Layout = new QHBoxLayout();

    VLayout->addSpacing(10);
    VLayout->addLayout(strokeLayout);
    strokeLayout->addWidget(strokeEnable, 0);
    strokeLayout->addWidget(strokeLabel, 1);
    strokeLayout->addLayout(strokeColumnsLayout, 4);
    strokeColumnsLayout->addLayout(strokeRow1Layout);
    strokeColumnsLayout->addLayout(strokeRow2Layout);

    ColorSelector* strokePreview = new ColorSelector(background);
    strokePreview->setColor(strokeColor_);
    strokePreview->setDisplayMode(color_widgets::ColorPreview::DisplayMode::SplitAlpha);

    customSpinBox* strokeHexSpinBox   = new customSpinBox(background, '#');
    customSpinBox* strokeAlphaSpinBox = new customSpinBox(background, 'A');
    customSpinBox* strokeRSpinBox     = new customSpinBox(background, 'R');
    customSpinBox* strokeGSpinBox     = new customSpinBox(background, 'G');
    customSpinBox* strokeBSpinBox     = new customSpinBox(background, 'B');

    strokeHexSpinBox->setStringInput(true);
    strokeHexSpinBox->setValue(QString("%1%2%3%4")
        .arg(strokeColor_.red(),   2, 16, QChar('0'))
        .arg(strokeColor_.green(), 2, 16, QChar('0'))
        .arg(strokeColor_.blue(),  2, 16, QChar('0'))
        .arg(strokeColor_.alpha(), 2, 16, QChar('0'))
        .toUpper());

    strokeAlphaSpinBox->setMinimum(0); strokeAlphaSpinBox->setMaximum(255);
    strokeRSpinBox->setMinimum(0);     strokeRSpinBox->setMaximum(255);
    strokeGSpinBox->setMinimum(0);     strokeGSpinBox->setMaximum(255);
    strokeBSpinBox->setMinimum(0);     strokeBSpinBox->setMaximum(255);

    strokeAlphaSpinBox->setValue(strokeColor_.alpha());
    strokeRSpinBox->setValue(strokeColor_.red());
    strokeGSpinBox->setValue(strokeColor_.green());
    strokeBSpinBox->setValue(strokeColor_.blue());

    strokeRow1Layout->addWidget(strokePreview,    1);
    strokeRow1Layout->addWidget(strokeHexSpinBox, 1);
    strokeRow1Layout->addWidget(strokeAlphaSpinBox, 1);
    strokeRow2Layout->addWidget(strokeRSpinBox);
    strokeRow2Layout->addWidget(strokeGSpinBox);
    strokeRow2Layout->addWidget(strokeBSpinBox);

    auto updateStrokeControls = [this, strokePreview, strokeRSpinBox, strokeGSpinBox, strokeBSpinBox, strokeAlphaSpinBox, strokeHexSpinBox]() {
        strokeRSpinBox->blockSignals(true);
        strokeGSpinBox->blockSignals(true);
        strokeBSpinBox->blockSignals(true);
        strokeAlphaSpinBox->blockSignals(true);
        strokeHexSpinBox->blockSignals(true);

        strokeRSpinBox->setValue(strokeColor_.red());
        strokeGSpinBox->setValue(strokeColor_.green());
        strokeBSpinBox->setValue(strokeColor_.blue());
        strokeAlphaSpinBox->setValue(strokeColor_.alpha());
        strokeHexSpinBox->setValue(QString("%1%2%3%4")
            .arg(strokeColor_.red(),   2, 16, QChar('0'))
            .arg(strokeColor_.green(), 2, 16, QChar('0'))
            .arg(strokeColor_.blue(),  2, 16, QChar('0'))
            .arg(strokeColor_.alpha(), 2, 16, QChar('0'))
            .toUpper());
        strokePreview->setColor(strokeColor_);

        strokeRSpinBox->blockSignals(false);
        strokeGSpinBox->blockSignals(false);
        strokeBSpinBox->blockSignals(false);
        strokeAlphaSpinBox->blockSignals(false);
        strokeHexSpinBox->blockSignals(false);
    };

    strokeHexSpinBox->connect(strokeHexSpinBox, &customSpinBox::stringValueChanged, [this, updateStrokeControls](QString value){
        if(value[0] == '#') value.remove(0, 1);
        bool ok; int r, g, b, a = strokeColor_.alpha();
        if(value.length() >= 6){
            r = value.mid(0,2).toInt(&ok,16); g = value.mid(2,2).toInt(&ok,16); b = value.mid(4,2).toInt(&ok,16);
            if(value.length() >= 8) a = value.mid(6,2).toInt(&ok,16);
            strokeColor_ = QColor(r,g,b,a); updateStrokeControls(); update();
        }
    });
    strokeRSpinBox->connect(strokeRSpinBox,    &customSpinBox::valueChanged, [this, updateStrokeControls](qreal v){ strokeColor_.setRed(v);   updateStrokeControls(); update(); });
    strokeGSpinBox->connect(strokeGSpinBox,    &customSpinBox::valueChanged, [this, updateStrokeControls](qreal v){ strokeColor_.setGreen(v); updateStrokeControls(); update(); });
    strokeBSpinBox->connect(strokeBSpinBox,    &customSpinBox::valueChanged, [this, updateStrokeControls](qreal v){ strokeColor_.setBlue(v);  updateStrokeControls(); update(); });
    strokeAlphaSpinBox->connect(strokeAlphaSpinBox,&customSpinBox::valueChanged, [this, updateStrokeControls](qreal v){ strokeColor_.setAlpha(v); updateStrokeControls(); update(); });
    strokePreview->connect(strokePreview, &ColorSelector::colorSelected, [this, updateStrokeControls](QColor c){ strokeColor_ = c; updateStrokeControls(); update(); });

    strokeEnable->connect(strokeEnable, &QCheckBox::toggled, [this, strokePreview, strokeHexSpinBox, strokeAlphaSpinBox, strokeRSpinBox, strokeGSpinBox, strokeBSpinBox](bool state){
        stroke_ = state;
        strokePreview->setDisabled(!state);
        strokeHexSpinBox->setDisabled(!state);
        strokeAlphaSpinBox->setDisabled(!state);
        strokeRSpinBox->setDisabled(!state);
        strokeGSpinBox->setDisabled(!state);
        strokeBSpinBox->setDisabled(!state);
        // if(!state){ strokeColor_ = Qt::transparent; update(); }

        update();
    });


    fillEnable->setChecked(fill_);
    if(!fill_){
        fillPreview->setDisabled(!fill_);
        fillHexSpinBox->setDisabled(!fill_);
        fillAlphaSpinBox->setDisabled(!fill_);
        fillRSpinBox->setDisabled(!fill_);
        fillGSpinBox->setDisabled(!fill_);
        fillBSpinBox->setDisabled(!fill_);
    }

    strokeEnable->setChecked(stroke_);
    if(!stroke_){
        strokePreview->setDisabled(!stroke_);
        strokeHexSpinBox->setDisabled(!stroke_);
        strokeAlphaSpinBox->setDisabled(!stroke_);
        strokeRSpinBox->setDisabled(!stroke_);
        strokeGSpinBox->setDisabled(!stroke_);
        strokeBSpinBox->setDisabled(!stroke_);
    }

    customSpinBox* strokeWidthSpinBox = new customSpinBox(background, 'W');
    QLabel* strokeWidthLabel = new QLabel(background);
    strokeWidthLabel->setText("Stroke width");
    strokeWidthLabel->setMinimumWidth(20);
    strokeWidthSpinBox->setValue(strokeWidth_);

    QHBoxLayout* strokeWidthLayout = new QHBoxLayout();

    VLayout->addSpacing(10);
    VLayout->addLayout(strokeWidthLayout);
    strokeWidthLayout->addWidget(strokeWidthLabel, 1);
    strokeWidthLayout->addWidget(strokeWidthSpinBox, 2);

    strokeWidthSpinBox->connect(strokeWidthSpinBox, &customSpinBox::valueChanged, [this](qreal value){
        strokeWidth_ = value;
        update();
    });

    QHBoxLayout* jointLayout = new QHBoxLayout();
    QLabel* JointLabel = new QLabel(background);
    JointLabel->setText("Joint type");


    VLayout->addLayout(jointLayout);


    QToolButton* miterJointButton = new QToolButton(background);
    miterJointButton->setStyleSheet(toolBarButtonStyle);
    miterJointButton->setIcon(QIcon(":/JointTypes/icons/MiterJoint.svg"));
    miterJointButton->setIconSize(QSize(25,25));
    miterJointButton->setFixedSize(QSize(28,28));
    miterJointButton->setCheckable(true);
    miterJointButton->setChecked(true);
    miterJointButton->setToolTip("Miter Joint");
    
    
    QToolButton* bevelJointButton = new QToolButton(background);
    bevelJointButton->setStyleSheet(toolBarButtonStyle);
    bevelJointButton->setIcon(QIcon(":/JointTypes/icons/BevelJoint.svg"));
    bevelJointButton->setIconSize(QSize(25,25));
    bevelJointButton->setFixedSize(QSize(28,28));
    bevelJointButton->setCheckable(true);
    bevelJointButton->setChecked(false);
    bevelJointButton->setToolTip("Bevel Joint");
    
    QToolButton* roundJointButton = new QToolButton(background);
    roundJointButton->setStyleSheet(toolBarButtonStyle);
    roundJointButton->setIcon(QIcon(":/JointTypes/icons/RoundJoint.svg"));
    roundJointButton->setIconSize(QSize(25,25));
    roundJointButton->setFixedSize(QSize(28,28));
    roundJointButton->setCheckable(true);
    roundJointButton->setChecked(false);
    roundJointButton->setToolTip("Round Joint");
    
    
    jointLayout->addWidget(JointLabel);
    jointLayout->addWidget(miterJointButton);
    jointLayout->addWidget(bevelJointButton);
    jointLayout->addWidget(roundJointButton);

    miterJointButton->connect(miterJointButton, &QToolButton::toggled, [this, bevelJointButton, roundJointButton](bool state){
        if(state && pathJointStyle != Qt::PenJoinStyle::MiterJoin){
            pathJointStyle = Qt::PenJoinStyle::MiterJoin;
            bevelJointButton->setChecked(false);
            roundJointButton->setChecked(false);
            update();
        }
    });
    bevelJointButton->connect(bevelJointButton, &QToolButton::toggled, [this, miterJointButton, roundJointButton](bool state){
        if(state && pathJointStyle != Qt::PenJoinStyle::BevelJoin){
            pathJointStyle = Qt::PenJoinStyle::BevelJoin;
            miterJointButton->setChecked(false);
            roundJointButton->setChecked(false);
            update();
        }
    });
    roundJointButton->connect(roundJointButton, &QToolButton::toggled, [this, bevelJointButton, miterJointButton](bool state){
        if(state && pathJointStyle != Qt::PenJoinStyle::RoundJoin){
            pathJointStyle = Qt::PenJoinStyle::RoundJoin;
            miterJointButton->setChecked(false);
            bevelJointButton->setChecked(false);
            update();
        }
    });


    cachedAttributeWidget_ = background;
    return background;
}

void path::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setClipping(false);

    // return if the shape has no nodes
    if(actualNodes_.isEmpty()) return;

    updateTransformedNodes();

    // Draw Edges
    QPainterPath path;

    path.moveTo(drawnNodes_[0]->position_);
    for (int i = 0; i < drawnNodes_.size(); i++) {
        for(auto j : edges_[i]){
            handleMode initialMode = drawnNodes_[i]->mode;
            handleMode FinalMode = drawnNodes_[j]->mode;

            if((FinalMode == handleMode::symmetric || FinalMode == handleMode::smooth) && initialMode == handleMode::linear){
                path.quadTo(drawnNodes_[j]->H1->position_, drawnNodes_[j]->position_);
            }
            else if((FinalMode == handleMode::symmetric || FinalMode == handleMode::smooth) && (initialMode == handleMode::symmetric || initialMode == handleMode::smooth)){
                path.cubicTo(drawnNodes_[i]->H2->position_, drawnNodes_[j]->H1->position_, drawnNodes_[j]->position_);
            }
            else if(FinalMode == handleMode::linear && (initialMode == handleMode::symmetric || initialMode == handleMode::smooth)){
                path.quadTo(drawnNodes_[i]->H2->position_, drawnNodes_[j]->position_);
            }
            else if(FinalMode == handleMode::linear && initialMode == handleMode::linear){
                path.lineTo(drawnNodes_[j]->position_);
            }
        }
    }



    
    // Fill
    if (!inPathDrawingMode_ && fill_) {
        painter->fillPath(path, QBrush(fillColor_));
    }
    else{
        painter->fillPath(path, QBrush(Qt::transparent));
    }
    
    // Stroke
    if(!inPathDrawingMode_ && stroke_){
        painter->setPen(QPen(strokeColor_, strokeWidth_, Qt::SolidLine, Qt::SquareCap, pathJointStyle));
    }
    else if(!inPathDrawingMode_ && !stroke_){
        painter->setPen(QPen(Qt::NoPen));
    }
    else if(inPathDrawingMode_){
        painter->setPen(QPen(QColor("#4C7FD1"), 1));
    }
    painter->drawPath(path);

    // Preview for next point to draw when using bezier pen
    if (hasDrawingPreview_) {
        painter->setPen(QPen(QColor("#B84343"), 1));
        painter->setBrush(Qt::NoBrush);
        
        QPainterPath previewPath;
        if((drawnNodes_[getLastNodeIndex()]->mode == handleMode::symmetric || drawnNodes_[getLastNodeIndex()]->mode == handleMode::smooth) && (drawnNodes_[0]->mode == handleMode::symmetric || drawnNodes_[0]->mode == handleMode::smooth) && firstPointSnapping_){
            previewPath.moveTo(drawnNodes_[getLastNodeIndex()]->position_);
            previewPath.cubicTo(drawnNodes_[getLastNodeIndex()]->H2->position_, drawnNodes_[0]->H1->position_, previewPoint_);
            
            painter->drawPath(previewPath);
        }
        else if((drawnNodes_[getLastNodeIndex()]->mode == handleMode::symmetric || drawnNodes_[getLastNodeIndex()]->mode == handleMode::smooth)){
            previewPath.moveTo(drawnNodes_[getLastNodeIndex()]->position_);
            previewPath.quadTo(drawnNodes_[getLastNodeIndex()]->H2->position_, previewPoint_);

            painter->drawPath(previewPath);
        }
        else if(drawnNodes_[getLastNodeIndex()]->mode == handleMode::linear && (drawnNodes_[0]->mode == handleMode::symmetric || drawnNodes_[0]->mode == handleMode::smooth) && firstPointSnapping_){
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

        // Calculate common values
        QPointF center(0.5 * (maxX_ + minX_), 0.5 * (maxY_ + minY_));
        qreal handleHalf = handleD_ / 2.0;
        qreal handleOffset = handleHalf + handleGrowth_;
        
        qreal actualMinX = std::min(minX_, maxX_);
        qreal actualMaxX = std::max(minX_, maxX_);
        qreal actualMinY = std::min(minY_, maxY_);
        qreal actualMaxY = std::max(minY_, maxY_);
        
        // Draw selection rectangle more concisely
        qreal padding = strokeWidth_ / 2.0 + selectionGrowth_;
        QPointF rectPoints[4] = {
            mapToItemRotation(actualMinX - padding, actualMinY - padding), // TL
            mapToItemRotation(actualMaxX + padding, actualMinY - padding), // TR
            mapToItemRotation(actualMaxX + padding, actualMaxY + padding), // BR
            mapToItemRotation(actualMinX - padding, actualMaxY + padding)  // BL
        };
        for(int i = 0; i < 4; i++)
            painter->drawLine(rectPoints[i], rectPoints[(i+1)%4]);
        
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);


        // Define handle positions in object space
        QPointF cornerOffsets[4] = {
            {actualMinX - handleOffset - strokeWidth_, actualMinY - handleOffset - strokeWidth_}, // UL
            {actualMaxX + handleOffset + strokeWidth_, actualMinY - handleOffset - strokeWidth_}, // UR
            {actualMaxX + handleOffset + strokeWidth_, actualMaxY + handleOffset + strokeWidth_}, // DR
            {actualMinX - handleOffset - strokeWidth_, actualMaxY + handleOffset + strokeWidth_}  // DL
        };
        
        QPointF edgeOffsets[4] = {
            {center.x(), actualMinY - handleOffset - strokeWidth_}, // Up
            {actualMaxX + handleOffset, center.y()}, // Right
            {center.x(), actualMaxY + handleOffset + strokeWidth_}, // Down
            {actualMinX - handleOffset - strokeWidth_, center.y()}  // Left
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
        
        painter->save();
        painter->translate(position_ + pivotPoint_);
        painter->rotate(rotation_);
        PivotMark.render(painter, QRectF(-handleHalf, -handleHalf, handleD_, handleD_));
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
            if((drawnNodes_[i]->mode == handleMode::symmetric || drawnNodes_[i]->mode == handleMode::smooth) && drawnNodes_[i]->H1 != nullptr && drawnNodes_[i]->H2 != nullptr){
                painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
                painter->setBrush(Qt::NoBrush);

                painter->drawLine(drawnNodes_[i]->position_, drawnNodes_[i]->H1->position_);
                painter->drawLine(drawnNodes_[i]->position_, drawnNodes_[i]->H2->position_);

                painter->setBrush(QBrush(Qt::white));
                painter->setPen(QPen(Qt::black, 1));
                painter->drawEllipse(drawnNodes_[i]->H1->position_, handleD_/2, handleD_/2);
                painter->drawEllipse(drawnNodes_[i]->H2->position_, handleD_/2, handleD_/2);
            }

            if(actualNodes_[i]->isHighlighted() && selectedHandle_ == -1)
                painter->setBrush(QBrush(QColor("#2A7FFF")));
            else
                painter->setBrush(handleBrush_);

            switch (drawnNodes_[i]->mode){
            case handleMode::linear:
                Rhombus.translate(drawnNodes_[i]->position_ - QPoint(5,5));
                painter->drawPath(Rhombus);
                
                // painter->drawText(originalNodes_[i]->position_, QString("(%1,%2)").arg(originalNodes_[i]->position_.x()).arg(originalNodes_[i]->position_.y())); //debug point positions

                Rhombus.translate(-1 * drawnNodes_[i]->position_ + QPoint(5,5)); //reset the rohumbus to the original position to be moved in the next iteration (next node)
                break;
            case handleMode::smooth:
                painter->drawEllipse(
                    drawnNodes_[i]->position_,
                    handleD_/2.0, 
                    handleD_/2.0);
                break;
            case handleMode::symmetric:
                painter->drawRect(
                    drawnNodes_[i]->position_.x() - handleD_/2.0,
                    drawnNodes_[i]->position_.y() - handleD_/2.0,
                    handleD_,
                    handleD_);
                break;
            }
        }
    }

    // Snapping rectangle to the first point
    if(firstNodeHighlighted_){
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

void viewPort::setSelectedPath(path* newSelectedPath, bool state, bool hideAttributePanel)
{
    if (!state) {
        if (selectedPath_ != nullptr) {
            selectedPath_->setSelected(false);
            selectedPath_->update();
            selectedPath_ = nullptr;
        }
        emit objectSelected(nullptr); // ← notify panel: nothing selected
        return;
    }

    if (newSelectedPath != nullptr) {
        if (selectedPath_ != nullptr && selectedPath_ != newSelectedPath) {
            selectedPath_->setSelected(false);
            selectedPath_->update();
        }

        if(selectedPath_ != newSelectedPath && !hideAttributePanel){
            emit objectSelected(newSelectedPath);
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
            startedNewPath_ = true;
            currentPath = new path(canvasLocalPos, canvas_, &inPathEditingMode_);
            currentPath->setDrawingMode(true);
            setSelectedPath(currentPath, true, true);
            
            objects_.push_back(currentPath);
            return; //early return when having just one node
        }
        else{
            currentPath = objects_.back();
            int lastPointIndex = currentPath->getLastNodeIndex();
            QPointF pointToAdd = canvasLocalPos;
            QPointF firstPoint = currentPath->getActualPoint(0);
                    
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
                emit objectSelected(currentPath);
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
            QPointF nodePos = canvas_->mapToScene(selectedPath_->getDrawnPoint(i)); //converted to scene coordincates, scale, then rotate to follow the object transformation

            if (searchRect.contains(nodePos)) {

                if(controlPressed_){
                    selectedPath_->incrementNodeMode(i);
                    selectedPath_->makeDirty();
                    update();
                }

                if(shiftPressed_){
                    if(selectedPath_->isHighlighted(i))
                        selectedPath_->removeHighlightedNode(i);
                    else{
                        selectedPath_->addHighlightedNode(i);
                        holdStartPosition_ = canvasLocalPos;
                        holding_ = true;
                    }  

                    update();
                }
                else{
                    if(selectedPath_->isHighlighted(i) && selectedPath_->nodesHighlighted() > 1)
                        return;
                    
                    if(selectedPath_->isHighlighted(i)){
                        selectedPath_->clearHighlightedNodes();
                    }
                    else{
                        selectedPath_->clearHighlightedNodes();
                        selectedPath_->addHighlightedNode(i);
                    }
                    holdStartPosition_ = canvasLocalPos;
                    holding_ = true;
                }
                clickedOnNode = true;
            }
            else if(selectedPath_->getHandleExistance(i, 0) && searchRect.contains(canvas_->mapToScene(selectedPath_->getDrawnHandlePosition(i, 0)))){
                selectedPath_->clearHighlightedNodes();
                selectedPath_->addHighlightedNode(i);
                selectedPath_->selectedHandle_ = 0;
                clickedOnNode = true;
                holdStartPosition_ = canvasLocalPos;
                holding_ = true;
                break;
            }
            else if(selectedPath_->getHandleExistance(i, 1) && searchRect.contains(canvas_->mapToScene(selectedPath_->getDrawnHandlePosition(i, 1)))){
                selectedPath_->clearHighlightedNodes();
                selectedPath_->addHighlightedNode(i);
                selectedPath_->selectedHandle_ = 1;
                clickedOnNode = true;
                holdStartPosition_ = canvasLocalPos;
                holding_ = true;
                break;
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

                
                qreal rotation_radians = selectedPath_->rotation_ * M_PI / 180.0;

                qreal actualMinX = std::min(selectedPath_->minX_, selectedPath_->maxX_);
                qreal actualMaxX = std::max(selectedPath_->minX_, selectedPath_->maxX_);
                qreal actualMinY = std::min(selectedPath_->minY_, selectedPath_->maxY_);
                qreal actualMaxY = std::max(selectedPath_->minY_, selectedPath_->maxY_);

                qreal absScaleX = std::abs(selectedPath_->scaleX_);
                qreal absScaleY = std::abs(selectedPath_->scaleY_);
                originalHalfExtentX_ = (actualMaxX - selectedPath_->position_.x()) / absScaleX;
                originalHalfExtentY_ = (actualMaxY - selectedPath_->position_.y()) / absScaleY;

                qreal tangentialD = (holdStartPosition_.y() - selectedPath_->position_.y()) * sin(rotation_radians) + (holdStartPosition_.x() - selectedPath_->position_.x()) * cos(rotation_radians); 
                qreal radialD     = (holdStartPosition_.y() - selectedPath_->position_.y()) * cos(rotation_radians) - (holdStartPosition_.x() - selectedPath_->position_.x()) * sin(rotation_radians);

                if(activeScaleHandle_ == Left || activeScaleHandle_ == TopLeft || activeScaleHandle_ == BottomLeft){
                    scalingError_.setX(- tangentialD - (selectedPath_->position_.x() - actualMinX));
                }
                if(activeScaleHandle_ == Right || activeScaleHandle_ == TopRight || activeScaleHandle_ == BottomRight){
                    scalingError_.setX(tangentialD - (actualMaxX - selectedPath_->position_.x()));
                }

                if(activeScaleHandle_ == Top || activeScaleHandle_ == TopLeft || activeScaleHandle_ == TopRight){
                    scalingError_.setY(- radialD - (selectedPath_->position_.y() - actualMinY));
                }
                if(activeScaleHandle_ == Bottom || activeScaleHandle_ ==  BottomLeft || activeScaleHandle_ == BottomRight){
                    scalingError_.setY(radialD - (actualMaxY - selectedPath_->position_.y()));
                }

                scaleDragNegX_ = (selectedPath_->scaleX_ < 0);
                scaleDragNegY_ = (selectedPath_->scaleY_ < 0);
    
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
        QPointF firstPoint = selectedPath_->getActualPoint(0);
        snap_ =  std::abs(canvasLocalPos.x() - firstPoint.x()) <= snapMargin_ &&
                std::abs(canvasLocalPos.y() - firstPoint.y()) <= snapMargin_;

        selectedPath_->setSnapping(snap_);
    
        QPointF target = snap_ ? firstPoint : canvasLocalPos;
        selectedPath_->showSnapMargin(snap_);
    
        if (startedNewPath_ && holding_) {
            selectedPath_->clearPreviewPoint();
            selectedPath_->changeNodeMode(handleMode::symmetric, selectedPath_->getLastNodeIndex()); // this causes a crash
            selectedPath_->moveBezierHandle(canvasLocalPos, selectedPath_->getLastNodeIndex(), 2); // or maybe this?!
        }
        else if(startedNewPath_ && !holding_) {
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
                QPointF newPos = canvasLocalPos;

                newPos = selectedPath_->mapToItemRotation(newPos, true);

                newPos.setX(newPos.x()/selectedPath_->scaleX_ - 1.0/selectedPath_->scaleX_ * (selectedPath_->position_.x() + selectedPath_->pivotPoint_.x()) * (1-selectedPath_->scaleX_));
                newPos.setY(newPos.y()/selectedPath_->scaleY_ - 1.0/selectedPath_->scaleY_ * (selectedPath_->position_.y() + selectedPath_->pivotPoint_.y()) * (1-selectedPath_->scaleY_));

                if(selectedPath_->selectedHandle_ == -1)
                    selectedPath_->setNodePosition(newPos, selectedPath_->accessHighlightedVector(i));
                else if(selectedPath_->selectedHandle_ == 0)
                    selectedPath_->setHandlePosition(newPos, selectedPath_->accessHighlightedVector(i), 0);
                else if(selectedPath_->selectedHandle_ == 1)
                    selectedPath_->setHandlePosition(newPos, selectedPath_->accessHighlightedVector(i), 1);
                
            }

            selectedPath_->makeDirty();
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
            //clockwise rotation is positive, measured from positive x
            QPointF rotationCenter = selectedPath_->position_ + selectedPath_->pivotPoint_;
            QPointF startVector = holdStartPosition_ - rotationCenter;
            QPointF endVector =  canvasLocalPos - rotationCenter;
            qreal startAngle = std::atan2(startVector.y(), startVector.x());
            qreal endAngle = std::atan2(endVector.y(), endVector.x());
            qreal angleDifference = (endAngle - startAngle) * 180.0 / M_PI;

            if (angleDifference > 180)  angleDifference -= 360;
            if (angleDifference < -180) angleDifference += 360;

            // selectedPath_->rotation_ += angleDifference;

            selectedPath_->rotate(angleDifference);
            selectedPath_->update();

            holdStartPosition_ = canvasLocalPos; // needs to be removed
        }
        else if(holding_ && scaling_){
            QPointF delta = canvasLocalPos - selectedPath_->position_;

            if(shiftPressed_ && (activeScaleHandle_ == TopRight || activeScaleHandle_ == BottomRight || activeScaleHandle_ == TopLeft || activeScaleHandle_ == BottomLeft)){
                delta.setX(std::max(abs(delta.x()), abs(delta.y())));
                delta.setY(std::max(abs(delta.x()), abs(delta.y())));
            }

            bool restrictX = (activeScaleHandle_ == Top || activeScaleHandle_ == Bottom);
            bool restrictY = (activeScaleHandle_ == Left || activeScaleHandle_ == Right);

            bool flipX = (activeScaleHandle_ == Left || activeScaleHandle_ == TopLeft || activeScaleHandle_ == BottomLeft);
            bool flipY = (activeScaleHandle_ == Top || activeScaleHandle_ == TopLeft || activeScaleHandle_ == TopRight);

            bool effectiveFlipX = flipX ^ scaleDragNegX_;
            bool effectiveFlipY = flipY ^ scaleDragNegY_;
            selectedPath_->rescale(delta.x(), delta.y(), scalingError_,
                       originalHalfExtentX_, originalHalfExtentY_,
                       restrictX, restrictY,
                       effectiveFlipX, effectiveFlipY);
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
        selectedPath_->selectedHandle_ = -1;
        selectedPath_->update();
    }
}

void viewPort::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift){
        shiftPressed_ = true;
    }
    else if(event->key() == Qt::Key_Delete){
        if(selectedPath_ != nullptr){
            objects_.removeOne(selectedPath_); // remove from the list first
            delete selectedPath_;
            selectedPath_ = nullptr;
            emit objectSelected(nullptr);
        }
        update();
    }
    else if(event->key() == Qt::Key_Control){
        controlPressed_ = true;
    }
}

void viewPort::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift){
        shiftPressed_ = false;
    }
    else if(event->key() == Qt::Key_Control){
        controlPressed_ = false;
    }
}
