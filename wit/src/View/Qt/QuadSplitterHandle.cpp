#include "quadsplitterhandle.h"
#include <QPainter>
#include <QLinearGradient>
#include <QPaintEvent>
#include <iostream>
#include <QPoint>

QuadSplitterHandle::QuadSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
    :QSplitterHandle(orientation,parent)
{
    siblingHandle = NULL;
    this->setMouseTracking(true);
}

void QuadSplitterHandle::mouseMoveEvent(QMouseEvent *event)
{
    QSplitterHandle::mouseMoveEvent(event);
    if(!siblingHandle)
        return;

    if(abs(siblingHandle->pos().x() - event->pos().x()) < this->height())
    {
        this->setCursor(Qt::SizeAllCursor);
    }
    else
    {
        this->setCursor(Qt::SizeVerCursor);
    }
}

void QuadSplitterHandle::mousePressEvent(QMouseEvent *event)
{
    // Call parent event
    QSplitterHandle::mousePressEvent(event);

    // Determine position of mouse press
    this->OnMousePress(event->x(),event->y());
}

/*
void QuadSplitterHandle::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QLinearGradient gradient;
    if (orientation() == Qt::Horizontal) {
        gradient.setStart(rect().left(), rect().height()/2);
        gradient.setFinalStop(rect().right(), rect().height()/2);
    } else {
        gradient.setStart(rect().width()/2, rect().top());
        gradient.setFinalStop(rect().width()/2, rect().bottom());
    }
    painter.fillRect(event->rect(), QBrush(gradient));
}
*/
