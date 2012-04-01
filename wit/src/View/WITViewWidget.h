#ifndef WIT_VIEW_WIDGETH
#define WIT_VIEW_WIDGETH

#define WITFRAME_COUNT 4

#include <QtGui>

#include "WITFrame.h"

#include <QVTKWidget2.h>

class WITViewWidget : public QVTKWidget2 {

	Q_OBJECT
public:
	WITViewWidget(QWidget *parent = 0, const QGLWidget *shared = 0);
	WITFrame **getFrames(){return this->frames;}
	inline int getFrameCount(){return WITFRAME_COUNT;}
	WITFrame *getFrameAtPosition(int, int);
	int *screenToFrame(WITFrame *,int, int);
public slots:
	void onSetViewCount(int);
protected:
	void contextMenuEvent(QContextMenuEvent*);
private:
	
	WITFrame * frames[WITFRAME_COUNT];
	int activeFrameCount;
};

#endif