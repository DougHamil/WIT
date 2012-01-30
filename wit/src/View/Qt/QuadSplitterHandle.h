#ifndef QUADSPLITTERHANDLE_H
#define QUADSPLITTERHANDLE_H

#include <QSplitterHandle>
#include <QObject>

class QuadSplitterHandle : public QSplitterHandle
{
    Q_OBJECT
public:

    QuadSplitterHandle(Qt::Orientation orientation, QSplitter *parent);
    void setSiblingHandle(QSplitterHandle *handle){siblingHandle = handle;}

protected:

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *);
    QSplitterHandle *siblingHandle;

signals:
    void OnMousePress(int x, int y);
};

#endif // QUADSPLITTERHANDLE_H
