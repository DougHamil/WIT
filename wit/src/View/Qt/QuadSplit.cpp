#include "QuadSplit.h"
#include <QMouseEvent>
#include "QuadSplitterHandle.h"

QuadSplit::QuadSplit(QWidget *w1, QWidget *w2, QWidget *w3, QWidget *w4, QWidget *parent)
    : QSplitter(parent),
      widget1(w1),
      widget2(w2),
      widget3(w3),
      widget4(w4)
{
    middlePressed = false;

    parentSplit.setOrientation(Qt::Vertical);
    parentSplit.setParent(this,Qt::Widget);
    childSplit1.addWidget(w1);
    childSplit1.addWidget(w2);
    childSplit2.addWidget(w3);
    childSplit2.addWidget(w4);

    parentSplit.addWidget(&childSplit1);
    parentSplit.addWidget(&childSplit2);

    parentSplit.setChildrenCollapsible(false);
    childSplit1.setChildrenCollapsible(false);
    childSplit2.setChildrenCollapsible(false);

    ((QuadSplitterHandle*)parentSplit.handle(1))->setSiblingHandle(childSplit1.handle(1));

    connect (&childSplit1,
             SIGNAL(splitterMoved(int,int)),
             this,
             SLOT(sync1()));

    connect (&childSplit2,
             SIGNAL(splitterMoved(int,int)),
             this,
             SLOT(sync2()));

    connect (&parentSplit,
             SIGNAL(splitterMoved(int,int)),
             this,
             SLOT(parentSync()));

    connect (parentSplit.handle(1),
             SIGNAL(OnMousePress(int,int)),
             this,
             SLOT(OnParentHandlePress(int,int)));
}


