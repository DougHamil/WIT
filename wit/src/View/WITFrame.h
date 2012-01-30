/*
  File:     WITFrame.h
  Summary:  The view frame of the application.  Multiple WITFrames can be created for multiple views.
            The WITFrame connects to the WITFrameController for access to the model.
*/

#ifndef WITFRAME_H
#define WITFRAME_H

// - Qt
#include <QtGui>
// - VTK
#include "VTK/vtkQtGraphicsViewRenderWindow.h"

class WITFrame : public QWidget
{

public:
	WITFrame(QWidget *parent = 0);
protected:
	void resizeEvent(QResizeEvent *event);
	void contextMenuEvent(QContextMenuEvent*);
private:
	vtkQtGraphicsViewRenderWindow *renderWindow;
	vtkRenderer *renderer;
	QMenu *mnu_Context;

	void createContextMenu();
};

#endif