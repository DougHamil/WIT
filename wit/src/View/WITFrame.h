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
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <QVTKWidget2.h>
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

class WITFrame : public QVTKWidget2
{
	Q_OBJECT
public:
	WITFrame(QWidget *parent = 0, const QGLWidget *shared = 0);
	static WITFrame *CreateDuplicate(WITFrame &base);
	
	vtkCamera *getCamera();
	void setCamera(vtkCamera *newCam);
	vtkRenderWindow *getRenderWindow();
	void addActor(vtkActor* actor);
	void removeActor(vtkActor* actor);
	void setRenderer(vtkRenderer *renderer);
	vtkRenderer* getRenderer(){return renderer;}
	vtkActor *getPathwayActor(){return pathwayActor;}
	void setPathwayActor(vtkActor *a){pathwayActor = a;}
	QSize sizeHint() const{return QApplication::desktop()->screenGeometry().size();}
public slots:
	void setCameraView(int view);
	void Frame();
	void Start();
	void End();
protected slots:
	void MakeCurrent();
signals:
	void onSetCameraView(CameraView view);
	void onToggleDuplicate();
protected:
	void resizeEvent(QResizeEvent *event);
	void contextMenuEvent(QContextMenuEvent*);
	void mousePressEvent(QMouseEvent *event);
	void paintGL();
	void initializeGL();
	void resizeGL(int w, int h);
private:
	vtkActor *pathwayActor;
	vtkRenderer *renderer;
	CameraView cameraView;
	QMenu *mnu_Context;
	QVTKWidget2 *vtkWidget;
	QTime timer;
	void createContextMenu();
};

#endif