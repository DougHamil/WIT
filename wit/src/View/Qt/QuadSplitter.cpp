#include "quadsplitter.h"
#include "quadsplitterhandle.h"

QuadSplitter::QuadSplitter(Qt::Orientation orientation, QWidget *parent) :
    QSplitter(orientation, parent)
{
}

QSplitterHandle *QuadSplitter::createHandle()
{
    return new QuadSplitterHandle(orientation(), this);
}
