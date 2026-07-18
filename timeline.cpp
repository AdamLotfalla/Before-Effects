#include "timeline.h"

Layer::Layer(QWidget* parent, path *p, int* frameWidth) : QWidget(parent)
{
    relatedPath_ = p;
    frameWidth_ = frameWidth;
    this->setMinimumHeight(layerHeight_);
    setMouseTracking(true);

    setAutoFillBackground(false);
}

void Layer::refresh()
{
    update();
    updateGeometry();
}

void Layer::setDrawMode(DrawMode newMode)
{
    drawMode_ = newMode;
}

void Layer::setExpanded(bool state)
{
    if (relatedPath_->layerIsExpanded_ == state) return;
    relatedPath_->layerIsExpanded_ = state;
    update();
    updateGeometry();
}

void Layer::setOffset(int value)
{
    Layer::offset_ = value;
}

void Layer::setLBoundFrame(int frame)
{
    LboundFrame_ = frame;
}

void Layer::setRBoundFrame(int frame)
{
    RBoundFrame_ = frame;
}

void Layer::mousePressEvent(QMouseEvent *event)
{
    holding_ = true;
    prevDragDist_ = {0,0};
    holdStartPos_ = event->pos();
    if(drawMode_ == DrawMode::hierarchy){
        if(event->pos().x() <= 30){
            relatedPath_->layerIsExpanded_ = !relatedPath_->layerIsExpanded_;
            update();
            updateGeometry();
            emit expandedChanged();
        }
        if(event->pos().x() >= width() - 30){
            relatedPath_->visible_ = !relatedPath_->visible_;
            relatedPath_->update();
            update();
            emit visibilityChanged();
        }
        if(event->pos().x() > 30) emit makeSelected();
    }
    if(drawMode_ == DrawMode::keyframe){
        emit makeSelected();
        if(event->pos().x() >= offset_ + startFrame_ * *frameWidth_ - 5 && event->pos().x() <= offset_ + startFrame_ * *frameWidth_ +5 && event->pos().y() <= layerHeight_){
            draggingLboundary_ = true;
            QGuiApplication::setOverrideCursor(Qt::SizeHorCursor);
        }
        else if(event->pos().x() >= offset_ + endFrame_ * *frameWidth_ - 5 && event->pos().x() <= offset_ + endFrame_ * *frameWidth_ +5 && event->pos().y() <= layerHeight_){
            draggingRboundary_ = true;
            QGuiApplication::setOverrideCursor(Qt::SizeHorCursor);
        }
        else if(event->pos().y() <= layerHeight_){
            QGuiApplication::setOverrideCursor(Qt::ClosedHandCursor);
        }
    } 
}

void Layer::mouseReleaseEvent(QMouseEvent *event)
{
    holding_ = false;
    draggingLboundary_ = false;
    draggingRboundary_ = false;
    QGuiApplication::restoreOverrideCursor();

    if(dragFrameOffset_ == 0) return;
    
    // commit shifts to the maps
    std::array<std::map<int, qreal>*, 16> frameMaps = {
        &relatedPath_->xPositionFrames, &relatedPath_->yPositionFrames,
        &relatedPath_->xScaleFrames,    &relatedPath_->yScaleFrames,
        &relatedPath_->rotationFrames,  
        &relatedPath_->xPivotFrames,    &relatedPath_->yPivotFrames,
        &relatedPath_->strokeWidthFrames,
        &relatedPath_->fillRFrames, &relatedPath_->fillGFrames, &relatedPath_->fillBFrames, &relatedPath_->fillAFrames,
        &relatedPath_->strokeRFrames, &relatedPath_->strokeGFrames, &relatedPath_->strokeBFrames, &relatedPath_->strokeAFrames
    };

    for(auto map : frameMaps){
        std::map<int,qreal> shifted;

        for (auto const& [frame,value] : *map){
            shifted[frame + dragFrameOffset_] = value;
        }

        *map = std::move(shifted);
    }

    dragFrameOffset_ = 0;

}

void Layer::mouseMoveEvent(QMouseEvent *event)
{
    if(holding_){
        if(drawMode_ == DrawMode::keyframe){
            if(!draggingLboundary_ && !draggingRboundary_){
                dragFrameOffset_ = std::floor((event->pos() - holdStartPos_).x() / *frameWidth_);
                shiftKeyframeLayer((event->pos() - holdStartPos_).x());
                emit LayerDragged(dragFrameOffset_);
                if(startFrame_ * *frameWidth_ < -offset_) emit boundariesCrossed(- startFrame_ * *frameWidth_);
            }
            else if(draggingLboundary_){
                startFrame_ = std::floor((event->pos().x() - offset_) / *frameWidth_);
                update();
            }
            else if(draggingRboundary_){
                endFrame_ = std::floor((event->pos().x() - offset_) / *frameWidth_);
                update();
            }
        }
    }
    else{
        if(drawMode_ == DrawMode::keyframe){
            if(event->pos().x() >= offset_ + startFrame_ * *frameWidth_ - 5 && event->pos().x() <= offset_ + startFrame_ * *frameWidth_ +5 && event->pos().y() <= layerHeight_){
                QGuiApplication::setOverrideCursor(Qt::SizeHorCursor);
                hoveringLboundary_ = true;
            }
            else if(event->pos().x() >= offset_ + endFrame_ * *frameWidth_ - 5 && event->pos().x() <= offset_ + endFrame_ * *frameWidth_ +5 && event->pos().y() <= layerHeight_){
                QGuiApplication::setOverrideCursor(Qt::SizeHorCursor);
                hoveringRboundary_ = true;
            }
            else if(event->pos().x() >= offset_ + startFrame_ * *frameWidth_ && event->pos().x() <= offset_ + endFrame_ * *frameWidth_ && event->pos().y() <= layerHeight_){
                QGuiApplication::setOverrideCursor(Qt::OpenHandCursor);
                hoveringRboundary_ = false;
                hoveringLboundary_ = false;
            }
            else{
                QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
                hoveringRboundary_ = false;
                hoveringLboundary_ = false;
            }
            update();
        }
    }
}

void Layer::leaveEvent(QEvent *event)
{
    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
    hoveringLboundary_ = false;
    hoveringRboundary_ = false;
    update();
}

void Layer::shiftKeyframeLayer(qreal dist)
{
    int frameShifts = std::floor(
        (dist - prevDragDist_.x())
        / *frameWidth_
    );
    startFrame_ += frameShifts;
    endFrame_ += frameShifts;

    update();

    prevDragDist_.setX(
        prevDragDist_.x() + frameShifts * *frameWidth_
    );
}

void Layer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content

            
    std::array<bool, 16> hasFrames = {
        !relatedPath_->xPositionFrames.empty(),
        !relatedPath_->yPositionFrames.empty(),
        !relatedPath_->xScaleFrames.empty(),
        !relatedPath_->yScaleFrames.empty(),
        !relatedPath_->rotationFrames.empty(),
        !relatedPath_->xPivotFrames.empty(),
        !relatedPath_->yPivotFrames.empty(),
        !relatedPath_->strokeWidthFrames.empty(),
        !relatedPath_->fillRFrames.empty(),
        !relatedPath_->fillGFrames.empty(),
        !relatedPath_->fillBFrames.empty(),
        !relatedPath_->fillAFrames.empty(),
        !relatedPath_->strokeRFrames.empty(),
        !relatedPath_->strokeGFrames.empty(),
        !relatedPath_->strokeBFrames.empty(),
        !relatedPath_->strokeAFrames.empty()
    };
        

    if(drawMode_ == DrawMode::hierarchy){
        if(relatedPath_->isSelected())
            painter.setBrush(QColor("#585547"));
        else
            painter.setBrush(QColor("#444444"));
        painter.setPen(Qt::NoPen);

        QFont font = painter.font();
        font.setPointSize(8.5);
        painter.setFont(font);
        
        QPainterPath closedExpantionArrow;
        closedExpantionArrow.moveTo(12, (layerHeight_ - 8) / 2.0); //13.5
        closedExpantionArrow.lineTo(17,layerHeight_ / 2.0); //17.5
        closedExpantionArrow.lineTo(12, layerHeight_ - (layerHeight_ - 8) / 2.0); //21.5
        
        QPainterPath openExpantionArrow;
        openExpantionArrow.moveTo(10, (layerHeight_ - 5) / 2.0);
        openExpantionArrow.lineTo(14, 5 + (layerHeight_ - 5) / 2.0);
        openExpantionArrow.lineTo(18, (layerHeight_ - 5) / 2.0);
        
        
        painter.drawRoundedRect(0,0, this->width(), layerHeight_, 2, 2);
        painter.setPen(QPen("#FFFFFF"));
        painter.drawText(30,0, this->width() - 60, layerHeight_ - 1, Qt::AlignVCenter, relatedPath_->name_);
        
        QSvgRenderer* visible = new QSvgRenderer(QString(":/LayerUtils/icons/visible.svg"));
        QSvgRenderer* invisible = new QSvgRenderer(QString(":/LayerUtils/icons/invisible.svg"));

        if(relatedPath_->visible_)
            visible->render(&painter, QRect(width() - 25, (layerHeight_ - 20) / 2.0, 20, 20));
        else
            invisible->render(&painter, QRect(width() - 25, (layerHeight_ - 20) / 2.0, 20, 20));
        
        
        if(relatedPath_ && relatedPath_->layerIsExpanded_){
            int counter = 0;         
            int textBegin = 35, childBracketBegin = 14, childBracketEnd = 23; 
            
            for(int i = 0; i < hasFrames.size(); i++){
                if(!hasFrames[i]) continue;
                
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor("#373737"));
                painter.drawRect(0, counter * keyframeLayerHeight_ + layerHeight_, this->width(), keyframeLayerHeight_);
                
                painter.setPen(QPen("#FFFFFF"));
                QString keyframeText;
                
                switch (i)
                {
                case 0: keyframeText = "x-position";   break;
                case 1: keyframeText = "y-position";   break;
                case 2: keyframeText = "x-scale";      break;
                case 3: keyframeText = "y-scale";      break;
                case 4: keyframeText = "rotation";     break;
                case 5: keyframeText = "x-pivot";      break;
                case 6: keyframeText = "y-pivot";      break;
                case 7: keyframeText = "stroke width"; break;
                case 8: keyframeText = "fill R";   break;
                case 9: keyframeText = "fill G";   break;
                case 10: keyframeText = "fill B";   break;
                case 11: keyframeText = "fill A";   break;
                case 12: keyframeText = "stroke R"; break;
                case 13: keyframeText = "stroke G"; break;
                case 14: keyframeText = "stroke B"; break;
                case 15: keyframeText = "stroke A"; break;
                }
    
                painter.drawText(textBegin, counter * keyframeLayerHeight_ + layerHeight_ - 1, this->width() - textBegin, keyframeLayerHeight_, Qt::AlignVCenter, keyframeText);
   
                int VShfit = 2;
                painter.setPen(QPen(QBrush("#FFFFFF"), 1, Qt::DotLine));
                painter.drawLine(childBracketBegin, counter * keyframeLayerHeight_ + layerHeight_ + VShfit, childBracketBegin, (counter + 0.5) * keyframeLayerHeight_ + layerHeight_ + VShfit);
                painter.drawLine(childBracketBegin, (counter + 0.5) * keyframeLayerHeight_ + layerHeight_ + VShfit, childBracketEnd, (counter + 0.5) * keyframeLayerHeight_ + layerHeight_ + VShfit);
                counter++;
            }

            if(counter != 0){
                painter.setPen(QPen("#FFFFFF"));
                painter.drawPath(openExpantionArrow);
            }
    
            setFixedHeight(counter * keyframeLayerHeight_ + layerHeight_);
        }
        else if(relatedPath_ && !relatedPath_->layerIsExpanded_){
            
            int counter = 0;
            for(int i = 0; i < hasFrames.size(); i++){
                if(hasFrames[i]) counter ++;
            }
            if(counter != 0)    painter.drawPath(closedExpantionArrow);

            setFixedHeight(layerHeight_);
        }
    }
    else if(drawMode_ == DrawMode::keyframe){
        
        painter.translate(QPoint(Layer::offset_, 0));

        QPainterPath Rhombus;
        Rhombus.moveTo(5,0);
        Rhombus.lineTo(10,5);
        Rhombus.lineTo(5,10);
        Rhombus.lineTo(0,5);
        Rhombus.closeSubpath();
        
        painter.setPen(Qt::NoPen);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content    
        painter.setBrush(QBrush("#373737"));
        painter.drawRoundedRect(startFrame_ * *frameWidth_, 0, (endFrame_ - startFrame_) * *frameWidth_, 4000, 2, 2); // for layers
        painter.setBrush(QBrush(color_));
        painter.drawRoundedRect(startFrame_ * *frameWidth_, 0, (endFrame_ - startFrame_) * *frameWidth_, layerHeight_, 2, 2);
        
        
        //drawing hatches & outline if selected
        if(relatedPath_->isSelected()){
            painter.setPen(QPen(QColor("#ffffff"), 15));
            painter.setOpacity(0.1);
            
            QPainterPath Rect;
            Rect.addRoundedRect(startFrame_ * *frameWidth_, 0, (endFrame_ - startFrame_) * *frameWidth_, layerHeight_, 2, 2);
            painter.setClipPath(Rect);
            float lineSpacing = 30.0;
            int NLines = width() / lineSpacing + 1;
            while(NLines--){
                painter.drawLine(NLines * lineSpacing + startFrame_ * *frameWidth_ + 20, -10, NLines * lineSpacing + startFrame_ * *frameWidth_, keyframeLayerHeight_ + 10);
            }
            
            painter.setOpacity(0.8);
            painter.setPen(QPen(QColor("#f0f0f0"), 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(Rect);
            painter.setClipping(false);
        }
        
        

        // draw keyframes
        painter.setOpacity(1);
        painter.setPen(Qt::NoPen);
        
        std::array<const std::map<int, qreal>*, 16> frameMaps = {
            &relatedPath_->xPositionFrames, &relatedPath_->yPositionFrames,
            &relatedPath_->xScaleFrames,    &relatedPath_->yScaleFrames,
            &relatedPath_->rotationFrames,  
            &relatedPath_->xPivotFrames,    &relatedPath_->yPivotFrames,
            &relatedPath_->strokeWidthFrames,
            &relatedPath_->fillRFrames, &relatedPath_->fillGFrames, &relatedPath_->fillBFrames, &relatedPath_->fillAFrames,
            &relatedPath_->strokeRFrames, &relatedPath_->strokeGFrames, &relatedPath_->strokeBFrames, &relatedPath_->strokeAFrames
        };


        int counter = 0;
        for(int i = 0; i < frameMaps.size(); i++){ // for every frame map
            for(auto& j : *frameMaps[i]){ // for every keyframe in each frame map
                
                painter.setBrush(QBrush("#a4a588"));
                painter.setCompositionMode(QPainter::CompositionMode_Multiply);
                painter.drawEllipse(QPointF((j.first + dragFrameOffset_) * *frameWidth_, 0.5 * layerHeight_), 3, 3); 
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                
                if(relatedPath_->layerIsExpanded_){
                    painter.setBrush(QBrush("#7BC7B0"));
                    Rhombus.translate(QPointF((j.first + dragFrameOffset_) * *frameWidth_, (counter + 0.5) * keyframeLayerHeight_ + layerHeight_) - QPointF(5,5));
                    painter.drawPath(Rhombus);    
                    Rhombus.translate(-1 * (QPointF((j.first + dragFrameOffset_) * *frameWidth_, (counter + 0.5) * keyframeLayerHeight_ + layerHeight_) - QPointF(5,5)));
                }
            }
            if(hasFrames[i]) counter++;
        }
    
        if(relatedPath_->layerIsExpanded_){
            setFixedHeight(layerHeight_ + counter * keyframeLayerHeight_);
        }
        else{
            setFixedHeight(layerHeight_);
        }
        adjustSize();

        painter.setOpacity(0.5);
        painter.setBrush(QBrush("#161616"));
        painter.drawRect(startFrame_ * *frameWidth_, 0, std::max(LboundFrame_ - startFrame_, 0) * *frameWidth_, height());
        painter.drawRect(RBoundFrame_ * *frameWidth_, 0, std::max(endFrame_ - RBoundFrame_, 0) * *frameWidth_, height());
        if(!relatedPath_->visible_){
            painter.drawRect(startFrame_ * *frameWidth_, 0, (endFrame_ - startFrame_) * *frameWidth_, height());
        }
        
        painter.setOpacity(0.8);
        painter.setPen(QPen(QColor("#f0f0f0"), 1));
        painter.setBrush(QBrush("#f0f0f0"));
        if(hoveringLboundary_){
            painter.drawRoundedRect(startFrame_ * *frameWidth_ - 3, 0, 3, layerHeight_, 1, 1);
        }
        if(hoveringRboundary_){
            painter.drawRoundedRect(endFrame_ * *frameWidth_ - 3, 0, 3, layerHeight_, 1, 1);
        }
        painter.setOpacity(1);
        
        painter.translate(QPoint(- Layer::offset_, 0));
    }

}



TimeCursor::TimeCursor(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    setFixedWidth(IndicatorWidth_);
}

void TimeCursor::MoveCenter(int x, int y)
{
    this->move(x - IndicatorWidth_/2, y);
    update();
}

void TimeCursor::paintEvent(QPaintEvent *event)
{
    int centerX = IndicatorWidth_/2;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content

    QPen pen(QColor("#7BC7B0"), 2, Qt::SolidLine);
    QBrush brush(QColor("#7BC7B0"));

    painter.setPen(pen);
    painter.setBrush(brush);


    QPainterPath path;
    path.moveTo(centerX -5,0);
    path.lineTo(centerX -5,3);
    path.lineTo(centerX +0,8);
    path.lineTo(centerX +5,3);
    path.lineTo(centerX +5,0);
    path.closeSubpath();

    painter.drawPath(path);

    painter.drawLine(centerX, 8, centerX, this->height());
}



TickBar::TickBar(QWidget* parent, int *frameRate, int *frameWidth, int *frameCount, int width, int *currentFrame){
    //TEMPORARY
    currentFrame_ = currentFrame;
    
    frameRate_ = frameRate;
    frameWidth_ = frameWidth;
    frameCount_ = frameCount;

    fullWidth_ = width;

    RBoundFrame_ = *frameCount_;
    LBoundFrame_ = 0;

    setFixedSize(fullWidth_, getTopBarHeight());
    setMouseTracking(true);


    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background: transparent;");
    // timeIndicator->setAttribute(Qt::WA_AlwaysStackOnTop);
    
    LBound = new QWidget(this);
    RBound = new QWidget(this);

    LBound->setFixedSize(boundHandleThickness, tickLayerHeight_);
    RBound->setFixedSize(boundHandleThickness, tickLayerHeight_);

    LBound->setAttribute(Qt::WA_TranslucentBackground);
    LBound->setStyleSheet("background: transparent;");

    RBound->setAttribute(Qt::WA_TranslucentBackground);
    RBound->setStyleSheet("background: transparent;");




    connect(this, &TickBar::tickBarClickedSignal,
            this, &TickBar::onTickBarClick);
    connect(this, &TickBar::tickBarUnClickedSignal,
            this, &TickBar::onTickBarUnClick);
    connect(this, &TickBar::LBoundClickedSignal,
            this, &TickBar::onLBoundClick);
    connect(this, &TickBar::LBoundUnClickedSignal,
            this, &TickBar::onLBoundUnClick);
    connect(this, &TickBar::RBoundClickedSignal,
            this, &TickBar::onRBoundClick);
    connect(this, &TickBar::RBoundUnClickedSignal,
            this, &TickBar::onRBoundUnClick);

    
    update();
}

void TickBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen;
    // fullHeight = this->height();
    fullWidth_ = *frameCount_ * *frameWidth_;

    switch (int((float)*frameWidth_/5))
    {
    case 1:
        tickInterval_ = 20;
        break;
    case 2 ... 3:
        tickInterval_ = 10;
        break;
    case 4 ... 7:
        tickInterval_ = 5;
        break;
    default:
        tickInterval_ = 1;
        break;
    };

    pen.setWidth(0);
    painter.setPen(pen);

    QRect fullRect(0, 0, this->width(), tickLayerHeight_);
    painter.fillRect(fullRect, QColor("#1E1E1E"));

    QRect leftMargin(0, tickLayerHeight_, offset_, height() - tickLayerHeight_);
    QRect rightMargin(offset_ + *frameCount_ * *frameWidth_, tickLayerHeight_, width() - offset_ - (*frameCount_ * *frameWidth_), height() - tickLayerHeight_); 
        //added this long expression for width in case the total margin is not even, so division will be short by 1
    painter.fillRect(leftMargin, QColor("#202020"));
    painter.fillRect(rightMargin, QColor("#202020"));

    QRect LOutsideBound(offset_ , tickLayerHeight_ + boundLayerHeight_, LBoundFrame_ * *frameWidth_, height());
    QRect ROutsideBound(offset_ + RBoundFrame_ * *frameWidth_, tickLayerHeight_ + boundLayerHeight_, (*frameCount_ - RBoundFrame_) * *frameWidth_, height());

    painter.fillRect(LOutsideBound, QColor("#252525"));
    painter.fillRect(ROutsideBound, QColor("#252525"));
    
    for(int i = 0; i< *frameCount_; i++){
        QRect rect(offset_ + i* *frameWidth_, 0, *frameWidth_, tickLayerHeight_);
        
        if(i % 2){
            painter.fillRect(rect, QColor("#2D2D2D"));
        }
        else{
            painter.fillRect(rect, QColor("#292929"));
        }
    }
    pen.setWidth(3);
    pen.setColor("#999999");
    painter.setPen(pen);

    for(int i = 0; i <= *frameCount_; i++){
        if(i % tickInterval_ == 0){
            painter.drawLine(offset_ + i* *frameWidth_, tickLayerHeight_ * 7/8, offset_ + i* *frameWidth_, tickLayerHeight_ - 3); // -3 to compensate for pen width
            painter.drawText(QPoint(offset_ + i* *frameWidth_ + 1, tickLayerHeight_ * 11/16), QString::number(i));
        }
    }

    pen.setColor("#444444");
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0, tickLayerHeight_, this->width(), tickLayerHeight_);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver); // Draw over existing content

    QRect boundLayerRect(
        offset_ + *frameWidth_ * LBoundFrame_,
        tickLayerHeight_,
        *frameWidth_ * (RBoundFrame_ - LBoundFrame_),
        boundLayerHeight_
    );

    painter.fillRect(boundLayerRect, QColor("#444444"));

    pen.setColor(QColor("#52786D"));
    QBrush brush(QColor("#52786D"));
    painter.setPen(pen);
    painter.setBrush(brush);


    LBound->move(offset_ + LBoundFrame_ * *frameWidth_ - boundHandleThickness, tickLayerHeight_);
    RBound->move(offset_ + RBoundFrame_ * *frameWidth_, tickLayerHeight_);
    painter.drawRoundedRect(RBound->x(), RBound->y(), boundHandleThickness, boundLayerHeight_, 1, 1);
    painter.drawRoundedRect(LBound->x(), LBound->y(), boundHandleThickness, boundLayerHeight_, 1, 1);
    
    





}

int TickBar::getRBound()
{
    return RBoundFrame_;
}

int TickBar::getLBound()
{
    return LBoundFrame_;
}

int TickBar::getTopBarHeight()
{
    return tickLayerHeight_ + boundLayerHeight_;
}

int TickBar::getFrameWidth()
{
    return *frameWidth_;
}

int TickBar::getOffset()
{
    return offset_;
}

void TickBar::setOffset(qreal value)
{
    offset_ = value;
}

int TickBar::getXLayerStart()
{
    return (LBound->x() + boundHandleThickness);
}

int TickBar::getInterBoundDist()
{
    return (getRBound() - getLBound()) * *frameWidth_;
}

void TickBar::onTickBarClick(QPoint pos)
{
    barClicked_ = true;
    repaint();
    int frame = round((pos.x() - offset_) / (float)*frameWidth_);

    if(frame > RBoundFrame_){
        *currentFrame_ = RBoundFrame_;
    }
    else if(frame < LBoundFrame_){
        *currentFrame_ = LBoundFrame_;
    }
    else{
        *currentFrame_ = frame;
    }
        
    
    emit frameChanged();
}

void TickBar::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        int x = event->pos().x();
        int y = event->pos().y();

        int minLBound = offset_ + LBoundFrame_ * *frameWidth_ - boundHandleThickness;
        int maxLBound = offset_ + LBoundFrame_ * *frameWidth_;

        int minRBound = offset_ + RBoundFrame_ * *frameWidth_;
        int maxRBound = offset_ + RBoundFrame_ * *frameWidth_ + boundHandleThickness;

        if(y < tickLayerHeight_ + boundLayerHeight_){
            if(minLBound < x && x < maxLBound){
                emit LBoundClickedSignal(event->pos());
            }
            else if(minRBound < x && x < maxRBound){
                emit RBoundClickedSignal(event->pos());
            }
            else{
                emit tickBarClickedSignal(event->pos());
            }
        }            
    } 
    QWidget::mousePressEvent(event);
}

void TickBar::mouseMoveEvent(QMouseEvent *event)
{
    int frame = round((event->pos().x() - offset_) / (float)*frameWidth_);
    frame = qBound(0, frame, *frameCount_);
    if(barClicked_){
        *currentFrame_ = frame;
        emit frameChanged();
        // The paint event does the movement for me
    }
    else if(LBoundClicked_){
        if(frame >= RBoundFrame_){
            LBoundFrame_ = RBoundFrame_ - 1;
        }
        else{
            LBoundFrame_ = frame;
        }
        emit LBoundChanged(LBoundFrame_);
    }
    else if(RBoundClicked_){
        if (frame <= LBoundFrame_){
            RBoundFrame_ = LBoundFrame_ + 1;
        }
        else{
            RBoundFrame_ = frame;
        }
        emit RBoundChanged(RBoundFrame_);
    }
    
    update();
}

void TickBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        emit tickBarUnClickedSignal();
        emit LBoundUnClickedSignal();
        emit RBoundUnClickedSignal();
    }
}

void TickBar::onTickBarUnClick()
{
    barClicked_ = false;
}

void TickBar::onLBoundClick(QPoint pos)
{
    LBoundClicked_ = true;
}

void TickBar::onLBoundUnClick()
{
    LBoundClicked_ = false;
}

void TickBar::onRBoundClick(QPoint pos)
{
    RBoundClicked_ = true;
}

void TickBar::onRBoundUnClick()
{
    RBoundClicked_ = false;
}



Timeline::Timeline (QWidget *parent, int *frameRate) : QWidget(parent){

    frameRate_ = frameRate;
    theme_= "Dark";
    this->setPalette(QPalette(QColor("#292929")));

    QWidget* toolbar = new QWidget(this);
    toolbar->setFixedHeight(25);
    toolbar->setAutoFillBackground(true);
    toolbar->setPalette(QPalette(QColor("#2E2E2E")));

    QToolButton* zoomInButton = new QToolButton(toolbar);
    QToolButton* zoomOutButton = new QToolButton(toolbar);

    zoomInButton->setFixedSize(21,21);
    zoomOutButton->setFixedSize(21,21);

    zoomInButton->setIcon(QIcon(QString(":/%1/icons/zoomIn_%1.svg").arg(theme_)));
    zoomOutButton->setIcon(QIcon(QString(":/%1/icons/zoomOut_%1.svg").arg(theme_)));
    zoomInButton->setIconSize(QSize(-1, 15));
    zoomOutButton->setIconSize(QSize(-1,15));
    zoomInButton->setToolTip("Zoom In Timeline");
    zoomOutButton->setToolTip("Zoom Out Timeline");
    zoomInButton->setStyleSheet(buttonStyle);
    zoomOutButton->setStyleSheet(buttonStyle);
        

    playButton = new QToolButton(toolbar);
    playButton->setFixedSize(21,21);
    playButton->setIcon(QIcon(QString(":/%1/icons/play_%1.svg").arg(theme_)));
    playButton->setIconSize(QSize(-1,15));
    playButton->setToolTip("Play");
    playButton->setStyleSheet(buttonStyle);

    QToolButton* goToStartButton = new QToolButton(toolbar);
    goToStartButton->setFixedSize(21,21);
    goToStartButton->setIcon(QIcon(QString(":/%1/icons/goToStart_%1.svg").arg(theme_)));
    goToStartButton->setIconSize(QSize(-1,15));
    goToStartButton->setToolTip("Go To Playback Start");
    goToStartButton->setStyleSheet(buttonStyle);

    QToolButton* goToEndButton = new QToolButton(toolbar);
    goToEndButton->setFixedSize(21,21);
    goToEndButton->setIcon(QIcon(QString(":/%1/icons/goToEnd_%1.svg").arg(theme_)));
    goToEndButton->setIconSize(QSize(-1,15));
    goToEndButton->setToolTip("Go To Playback End");
    goToEndButton->setStyleSheet(buttonStyle);

    QToolButton* nextFrameButton = new QToolButton(toolbar);
    nextFrameButton->setFixedSize(21,21);
    nextFrameButton->setIcon(QIcon(QString(":/%1/icons/nextFrame_%1.svg").arg(theme_)));
    nextFrameButton->setIconSize(QSize(-1,15));
    nextFrameButton->setToolTip("Go To Next Frame");
    nextFrameButton->setStyleSheet(buttonStyle);


    QToolButton* previousFrameButton = new QToolButton(toolbar);
    previousFrameButton->setFixedSize(21,21);
    previousFrameButton->setIcon(QIcon(QString(":/%1/icons/previousFrame_%1.svg").arg(theme_)));
    previousFrameButton->setIconSize(QSize(-1,15));
    previousFrameButton->setToolTip("Go To Previous Frame");
    previousFrameButton->setStyleSheet(buttonStyle);
    
    QSlider* zoomSlider = new QSlider(toolbar);
    zoomSlider->setRange(1, 20); //zoom amount from 1x to 20x
    zoomSlider->setSingleStep(1);
    zoomSlider->setFixedWidth(100);
    zoomSlider->setOrientation(Qt::Horizontal);
    zoomSlider->setStyleSheet(sliderStyle);

    QObject::connect(zoomSlider, &QSlider::valueChanged, this, &Timeline::zoomSliderChanged);
    
    QObject::connect(zoomInButton, &QToolButton::pressed, [zoomSlider](){
        if (zoomSlider->value() < zoomSlider->maximum()) {
            zoomSlider->setValue(zoomSlider->value() + 1);
        }
    });

    QObject::connect(zoomOutButton, &QToolButton::pressed, [zoomSlider](){
        if (zoomSlider->value() > zoomSlider->minimum()) {
            zoomSlider->setValue(zoomSlider->value() - 1);
        }
    });

    QObject::connect(playButton, &QToolButton::pressed,
                     this, &Timeline::playButtonClickEvent);

    QObject::connect(goToStartButton, &QToolButton::pressed, [this](){
        *currentFrame_ = tickBar_->getLBound();
        tickBar_->update();
        cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());
        emit frameChanged(*currentFrame_);
    });

    QObject::connect(goToEndButton, &QToolButton::pressed, [this](){
        *currentFrame_ = tickBar_->getRBound();
        tickBar_->update();
        cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());
        emit frameChanged(*currentFrame_);
    });

    QObject::connect(nextFrameButton, &QToolButton::pressed, [this](){
        if(*currentFrame_ < tickBar_->getRBound())
            (*currentFrame_) ++;
        tickBar_->update();
        cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());
        emit frameChanged(*currentFrame_);
    });

    QObject::connect(previousFrameButton, &QToolButton::pressed, [this](){
        if(*currentFrame_ > tickBar_->getLBound())
            (*currentFrame_) --;
        tickBar_->update();
        cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());
        emit frameChanged(*currentFrame_);
    });

    QHBoxLayout* toolBarHLayout = new QHBoxLayout(toolbar);
    toolBarHLayout->setContentsMargins(0,0,0,0);
    toolBarHLayout->setSpacing(5);
    toolBarHLayout->addStretch();
    toolBarHLayout->addWidget(goToStartButton);
    toolBarHLayout->addWidget(previousFrameButton);
    toolBarHLayout->addWidget(playButton);
    toolBarHLayout->addWidget(nextFrameButton);
    toolBarHLayout->addWidget(goToEndButton);
    toolBarHLayout->addStretch();
    toolBarHLayout->addWidget(zoomOutButton);
    toolBarHLayout->addWidget(zoomSlider, 0, Qt::AlignVCenter);
    toolBarHLayout->addWidget(zoomInButton);

    frameWidth_ = 5 * zoomSlider->value();

    /*
    -----------------------------------------------------------------------------------------
    | hierarchyPanel_                       |   keyframeHScroller_                          |
    |   |_ hierarchyVLayout_                |       |_ keyframePanel_                       |
    |       |_ spacer                       |           |_ keyframeVLayout_                 |
    |       |_ hierarchyScroller_           |               |_ tickBar_                     |
    |           |_ hierarchyLayerPanel_     |               |_ keyframeVScroller_           |
    |               |_ hierarchyLayerLayout |                   |_ keyframeLayerPanel_      |
    |                   |_ layer(s)         |                       |_ keyframeLayerLayout_ |
    -----------------------------------------------------------------------------------------
    */

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(0);

    
    toolBarVLayout_ = new QVBoxLayout;
    toolBarVLayout_->addWidget(toolbar);
    toolBarVLayout_->addWidget(splitter);
    toolBarVLayout_->setContentsMargins(0,0,0,0);
    toolBarVLayout_->setSpacing(1);
    
    hierarchyPanel_ = new QWidget(this);
    hierarchyPanel_->setMinimumWidth(200);
    keyframeHScroller_ = new QScrollArea;
    keyframeHScroller_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    keyframeHScroller_->setWidgetResizable(true);
    
    hierarchyScroller_ = new QScrollArea(hierarchyPanel_);
    hierarchyScroller_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    hierarchyScroller_->setWidgetResizable(true);
    
    keyframePanel_ = new QWidget;
    tickBar_ = new TickBar(keyframePanel_, frameRate_, &frameWidth_, &frameCount_, frameWidth_ * frameCount_ + 235, currentFrame_);
    tickBar_->show();
    keyframeHScroller_->setWidget(keyframePanel_);
    
    hierarchyVLayout_ = new QVBoxLayout;
    hierarchyVLayout_->addWidget(hierarchyScroller_, 1);
    hierarchyVLayout_->setContentsMargins(0,0,0,0);
    hierarchyVLayout_->setSpacing(1);
    
    hierarchyPanel_->setLayout(hierarchyVLayout_);

    splitter->addWidget(hierarchyPanel_);
    splitter->addWidget(keyframeHScroller_);
    
    hierarchyLayerPanel_ = new QWidget;
    hierarchyLayerLayout_ = new QVBoxLayout;
    hierarchyLayerLayout_->setContentsMargins(2,1,2,0);
    hierarchyLayerLayout_->setSpacing(1);
    hierarchyLayerLayout_->addSpacing(tickBar_->getTopBarHeight());
    hierarchyLayerLayout_->addStretch();
    hierarchyLayerPanel_->setLayout(hierarchyLayerLayout_);
    hierarchyLayerPanel_->setAutoFillBackground(false);
    hierarchyScroller_->setWidget(hierarchyLayerPanel_);

    keyframeVScroller_ = new QScrollArea;

    keyframeVLayout_ = new QVBoxLayout;
    keyframeVLayout_->addWidget(tickBar_);
    keyframeVLayout_->addWidget(keyframeVScroller_, 1);
    keyframeVLayout_->setContentsMargins(0,0,0,0);
    keyframeVLayout_->setSpacing(1);

    keyframePanel_->setLayout(keyframeVLayout_);

    keyframeLayerPanel_ = new MarginPanel(keyframePanel_);
    keyframeLayerPanel_->setLMarginWidth(tickBar_->getLBound() * frameWidth_);
    keyframeLayerPanel_->setRMarginWidth(tickBar_->width() - tickBar_->getRBound() * frameWidth_ - 2 * tickBar_->getOffset());
    keyframeLayerPanel_->setOffset(tickBar_->getOffset());
    keyframeLayerPanel_->setInterBoundDist(tickBar_->getInterBoundDist());

    keyframeLayerPanel_->connect(tickBar_, &TickBar::LBoundChanged, [&](){
        keyframeLayerPanel_->setLMarginWidth(tickBar_->getLBound() * frameWidth_);
        keyframeLayerPanel_->setInterBoundDist(tickBar_->getInterBoundDist());
    });
    keyframeLayerPanel_->connect(tickBar_, &TickBar::RBoundChanged, [&](){
        keyframeLayerPanel_->setRMarginWidth(tickBar_->width() - tickBar_->getRBound() * frameWidth_ - 2 * tickBar_->getOffset());   
        keyframeLayerPanel_->setInterBoundDist(tickBar_->getInterBoundDist());
    });

    keyframeVScroller_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    keyframeVScroller_->setWidget(keyframeLayerPanel_);


    keyframeLayerLayout_ = new QVBoxLayout;
    keyframeLayerLayout_->setContentsMargins(0,0,0,0);
    keyframeLayerLayout_->setSpacing(1);
    keyframeLayerLayout_->addStretch();
    keyframeLayerPanel_->setLayout(keyframeLayerLayout_);

    keyframeVScroller_->setWidgetResizable(true);
    keyframeLayerPanel_->adjustSize();

    keyframeHScroller_->setFrameShape(QFrame::NoFrame);
    hierarchyScroller_->setFrameShape(QFrame::NoFrame);
    keyframeVScroller_->setFrameShape(QFrame::NoFrame);
    
    // QWidget* testLayer = new QWidget; 
    // testLayer->setFixedHeight(layerHeight_); 
    // testLayer->setStyleSheet("background:green;");
    // keyframeLayerLayout_->addWidget(testLayer);
    // keyframeLayerLayout_->addStretch();


    connect(hierarchyScroller_->verticalScrollBar(), &QScrollBar::valueChanged,
            keyframeVScroller_->verticalScrollBar(), &QScrollBar::setValue);
    connect(keyframeVScroller_->verticalScrollBar(), &QScrollBar::valueChanged,
            hierarchyScroller_->verticalScrollBar(), &QScrollBar::setValue);

    
    setLayout(toolBarVLayout_);
    
    cursor_ = new TimeCursor(keyframePanel_);
    cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());    
    
    connect(tickBar_, &TickBar::frameChanged, this, [this](){
        cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());
        emit frameChanged(*currentFrame_);
    });

    Layer::setOffset(tickBar_->getOffset());
}

void Timeline::step()
{   
    if(*currentFrame_ < tickBar_->getLBound() || *currentFrame_ == tickBar_->getRBound())
        *currentFrame_ = tickBar_->getLBound();
    else if(*currentFrame_ < tickBar_->getRBound()) 
        (*currentFrame_) ++;
    else
        *currentFrame_ = tickBar_->getRBound();

    cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());
    emit frameChanged(*currentFrame_);
    update();
}

void Timeline::setTheme(QString theme)
{
    theme_ = theme;
}

void Timeline::addLayer(path* p)
{
    setActiveLayer(nullptr);
    Layer* hierarchyLayer = new Layer(hierarchyLayerPanel_, p, &frameWidth_);
    Layer* keyframeLayer = new Layer(keyframeLayerPanel_, p, &frameWidth_);
    keyframeLayer->setDrawMode(Layer::DrawMode::keyframe); // the other is hierarchy by default

    connect(hierarchyLayer, &Layer::expandedChanged, keyframeLayer, &Layer::refresh);
    connect(hierarchyLayer, &Layer::visibilityChanged, keyframeLayer, &Layer::refresh);
    connect(hierarchyLayer, &Layer::makeSelected, [p, this](){emit setSelectedPath(p);});
    connect(keyframeLayer, &Layer::makeSelected, hierarchyLayer, &Layer::makeSelected);
    connect(keyframeLayer, &Layer::boundariesCrossed, [&](int crossDist){
        tickBar_->setOffset(crossDist + 2);
        tickBar_->setFixedWidth(frameCount_ * frameWidth_ + 2*tickBar_->getOffset());
        keyframeLayer->setOffset(tickBar_->getOffset());
        keyframeLayerPanel_->setOffset(tickBar_->getOffset());
        tickBar_->update();
        cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * frameWidth_);
    });
    connect(keyframeLayer, &Layer::LayerDragged, hierarchyLayer, [&](int frameOffset){
        emit frameChanged(*currentFrame_ - frameOffset); //pseudo frame change to live update path based on layer drag
    }); 
    connect(tickBar_, &TickBar::LBoundChanged, keyframeLayer, &Layer::setLBoundFrame);
    connect(tickBar_, &TickBar::RBoundChanged, keyframeLayer, &Layer::setRBoundFrame);

    hierarchyLayerLayout_->insertWidget(1, hierarchyLayer);
    keyframeLayerLayout_->insertWidget(0, keyframeLayer);

    hierarchyLayer->show();
    keyframeLayer->show();

    layers_.push_back({hierarchyLayer, keyframeLayer});
    layerLookup_.insert(p, {hierarchyLayer, keyframeLayer});
    setActiveLayer(hierarchyLayer);
}

void Timeline::removeLayer(path *p)
{
    auto it = layerLookup_.find(p);
    if(it == layerLookup_.end()) return;

    auto [hLayer, kLayer] = it.value();
    if(activeLayer_ == hLayer) setActiveLayer(nullptr);

    layerLookup_.erase(it);
    layers_.removeIf([hLayer](const QPair<Layer*,Layer*>& pr){ return pr.first == hLayer; });

    delete hLayer;
    delete kLayer;
}

void Timeline::setActiveLayer(Layer *l)
{
    if(activeLayer_ != nullptr){
        activeLayer_->relatedPath_->setSelected(false);
        activeLayer_->update();

        auto it = layerLookup_.find(activeLayer_->relatedPath_);
        if(it != layerLookup_.end()){
            it.value().second->update();
        }
    }
    
    activeLayer_ = l;
    
    if(activeLayer_ != nullptr){
        activeLayer_->relatedPath_->setSelected(true);
        activeLayer_->update();

        auto it = layerLookup_.find(activeLayer_->relatedPath_);
        if(it != layerLookup_.end()){
            it.value().second->update();
        }
    }
}

void Timeline::zoomSliderChanged(int value)
{
    int newFrameWidth = value * 5;
    if (frameWidth_ == newFrameWidth)
        return;

    // --- compute OLD content position ---
    int oldOffset =
        (tickBar_->width() - frameCount_ * frameWidth_) / 2;

    int oldContentX =
        oldOffset + *currentFrame_ * frameWidth_;

    int oldScroll =
        keyframeHScroller_->horizontalScrollBar()->value();

    int indicatorViewportX =
        oldContentX - oldScroll;

    // --- apply zoom ---
    frameWidth_ = newFrameWidth;

    tickBar_->setFixedWidth(
        qMax(frameCount_ * frameWidth_ + tickBar_->getOffset(), width() - 4)
    );
    keyframeLayerPanel_ = new MarginPanel(keyframePanel_);
    keyframeLayerPanel_->setLMarginWidth(tickBar_->getLBound() * frameWidth_);
    keyframeLayerPanel_->setRMarginWidth(tickBar_->width() - tickBar_->getRBound() * frameWidth_ - 2 * tickBar_->getOffset());   
    keyframeLayerPanel_->setOffset(tickBar_->getOffset());
    keyframeLayerPanel_->setInterBoundDist(tickBar_->getInterBoundDist());

    // --- compute NEW content position ---
    int newOffset =
        (tickBar_->width() - frameCount_ * frameWidth_) / 2;

    int newContentX =
        newOffset + *currentFrame_ * frameWidth_;

    // --- restore viewport position ---
    int newScroll =
        newContentX - indicatorViewportX;

    keyframeHScroller_->horizontalScrollBar()->setValue(newScroll);

    cursor_->MoveCenter(tickBar_->getOffset() + *currentFrame_ * tickBar_->getFrameWidth());

    update();
    tickBar_->update();
}

void Timeline::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (tickBar_)
    {
        tickBar_->setFixedWidth(
            qMax(frameCount_ * frameWidth_ + tickBar_->getOffset(), width() - 4)
        );
        keyframeLayerPanel_->setLMarginWidth(tickBar_->getLBound() * frameWidth_);
        keyframeLayerPanel_->setRMarginWidth(tickBar_->width() - tickBar_->getRBound() * frameWidth_ - 2 * tickBar_->getOffset());   
        keyframeLayerPanel_->setOffset(tickBar_->getOffset());
        keyframeLayerPanel_->setInterBoundDist(tickBar_->getInterBoundDist());
    }

    if(cursor_){
        cursor_->setFixedHeight(height());
    }
}

void Timeline::playButtonClickEvent()
{
    playing_ = !playing_;

    if(playing_){
        playButton->setIcon(QIcon(QString(":/%1/icons/pause_%1.svg").arg(theme_)));
    }
    else{
        playButton->setIcon(QIcon(QString(":/%1/icons/play_%1.svg").arg(theme_)));
    }

    emit playSignal(playing_);
}

void Timeline::refreshLayer(path* p)
{
    auto it = layerLookup_.find(p);
    if(it == layerLookup_.end()) return;
    it.value().first->update();    // now used to repaint the name text since text is extracted every layer paint event
    it.value().second->update();
}

void Timeline::setSelectedLayer(path *p)
{
    if(p == nullptr){ setActiveLayer(nullptr); return; }
    auto it = layerLookup_.find(p);
    if(it != layerLookup_.end())
        setActiveLayer(it.value().first);
}