#ifndef QUADSPLIT_H
#define QUADSPLIT_H
#include <QSplitter>
#include "QuadSplitter.h"

class QuadSplit : public QSplitter
{
    Q_OBJECT

    QuadSplitter parentSplit;
    QuadSplitter childSplit1;
    QuadSplitter childSplit2;

    QWidget *widget1;
    QWidget *widget2;
    QWidget *widget3;
    QWidget *widget4;

private slots:
    void parentSync()
    {
        if(middlePressed)
        {
            QPoint mousePoint = parentSplit.mapFromGlobal(QCursor::pos());
            QList<int> newSizes;
            newSizes.append(mousePoint.x());
            newSizes.append(parentSplit.size().width() - mousePoint.x());
            childSplit1.setSizes(newSizes);
            childSplit2.setSizes(newSizes);
        }
    }

    void sync1()
    {
        childSplit2.setSizes(childSplit1.sizes());
    }

    void sync2()
    {
        childSplit1.setSizes(childSplit2.sizes());
    }

    void OnParentHandlePress(int x, int y)
    {
        int barX = childSplit1.handle(1)->pos().x();
        int threshold = childSplit1.handleWidth()+2;
        if(x < barX +threshold && x > barX - threshold)
        {
            middlePressed = true;
        }
        else
        {
            middlePressed = false;
        }
    }


public:
    QuadSplit(QWidget *w1, QWidget *w2, QWidget *w3, QWidget *w4, QWidget *parent = 0);
    void show() {parentSplit.show();}
protected:

private:
    bool middlePressed;
};
#endif // QUADSPLIT_H
