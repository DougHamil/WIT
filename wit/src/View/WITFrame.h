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
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>

enum CameraView
{
	CUSTOM = 0,
	TOP = 1,
	BOTTOM = 2,
	LEFT = 3,
	RIGHT = 4,
	FRONT = 5,
	BACK = 6,
};

class WITFrame : public QObject
{
	Q_OBJECT
public:
	WITFrame();
	vtkCamera *getCamera();
	void setCamera(vtkCamera *newCam);
	void addActor(vtkActor* actor);
	void removeActor(vtkActor* actor);
	vtkRenderer* getRenderer(){return renderer;}
	QSize sizeHint() const{return QApplication::desktop()->screenGeometry().size();}
	void showContextMenu(QContextMenuEvent*);
public slots:
	void setCameraView(int view);
protected slots:

signals:
	void onSetCameraView(CameraView view);
	void onToggleDuplicate();

private:
	vtkRenderer *renderer;
	CameraView cameraView;
	QMenu *mnu_Context;
	QTime timer;
	void createContextMenu();
};

#endif