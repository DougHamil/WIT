#ifndef QUADSPLITTER_H
#define QUADSPLITTER_H

#include <QSplitter>

/**
* Allows a 4-way split view of QWidgets.  The viewing area of each widget
* can be controlled using two splitter handles.
*/
class QuadSplitter : public QSplitter
{

public:
    QuadSplitter():QSplitter(){}
    QuadSplitter(Qt::Orientation orientation, QWidget *parent = 0);

signals:
    void OnHandlePress(int x, int y);
public slots:
protected:
    QSplitterHandle *createHandle();

};

#endif // QUADSPLITTER_H
