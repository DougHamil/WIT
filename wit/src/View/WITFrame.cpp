#include "WITFrame.h"
#include "../WITApplication.h"

// - Qt
#include <QtGui>
#include <QGLContext>

// - TEMP
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h> 
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <iostream>

WITFrame::WITFrame(QWidget *parent,const QGLWidget *shared)
	: QVTKWidget2(parent, shared)
{
	//this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	this->setSizePolicy(QSizePolicy::Policy::Maximum,QSizePolicy::Policy::Maximum);
	this->setLayout(new QBoxLayout(QBoxLayout::LeftToRight));
	this->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
	this->renderer = 0;
	this->SetUseTDx(false);	
	this->createContextMenu();
	this->pathwayActor = 0;

	this->GetRenderWindow()->SetAAFrames(false);
	this->GetRenderWindow()->SetLineSmoothing(false);
	this->GetRenderWindow()->SetPolygonSmoothing(false);
	
}

WITFrame *WITFrame::CreateDuplicate(WITFrame &base)
{
	WITFrame *nF = new WITFrame();


	return nF;
}

vtkCamera *WITFrame::getCamera()
{
	return this->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
}
void WITFrame::setCamera(vtkCamera *newCam)
{
	this->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->SetActiveCamera(newCam);
}

void WITFrame::MakeCurrent()
{
	QVTKWidget2::MakeCurrent();
}

void WITFrame::initializeGL()
{
	QTime t;
	t.start();
	QVTKWidget2::initializeGL();
	qDebug("GL Init time: %d ms",t.elapsed());
}

void WITFrame::paintGL()
{
	QTime t;
	t.start();
	QVTKWidget2::paintGL();
	qDebug("GL Paint time: %d ms",t.elapsed());
}

void WITFrame::resizeGL(int w, int h)
{
	QTime t;
	t.start();
	QVTKWidget2::resizeGL(w,h);
	//qDebug("GL Resize time: %d ms", t.elapsed());
}

vtkRenderWindow *WITFrame::getRenderWindow()
{
	return (vtkRenderWindow*)this->GetRenderWindow();
}


void WITFrame::Frame()
{
	qDebug("%p Start to Frame time: %d ms",this, timer.elapsed());
	QTime t;
	t.start();
	QVTKWidget2::Frame();
	qDebug("%p Frame time: %d ms", this, t.elapsed());
}
void WITFrame::Start()
{
	QTime t;
	t.start();
	QVTKWidget2::Start();
	qDebug("%p Start time: %d ms", this, t.elapsed());
	timer.restart();
}
void WITFrame::End()
{
	QTime t;
	t.start();
	QVTKWidget2::End();
	qDebug("%p End time: %d ms", this, t.elapsed());
}
void WITFrame::setRenderer(vtkRenderer *renderer)
{
	this->renderer = renderer;
}

void WITFrame::addActor(vtkActor* actor)
{
	if(!this->renderer)
		return;

	qDebug("Adding actor %p to frame %p.", actor, this);
	// Only add if the actors isn't already added to the renderer
	if(!this->renderer->GetActors()->IsItemPresent(actor))
		this->renderer->AddActor(actor);

	//actor->SetVisibility(false);
}

void WITFrame::removeActor(vtkActor* actor)
{
	if(!this->renderer)
		return;

	if(this->renderer->GetActors()->IsItemPresent(actor))
		this->renderer->RemoveActor(actor);
}

void WITFrame::mousePressEvent(QMouseEvent *event)
{
	//qDebug("----------------------------------\n%p -- Mouse Press\n-----------------------------------",this);

	if(event->button() == Qt::MouseButton::MidButton)
	{
		emit this->onToggleDuplicate();
	}
	else 
		QVTKWidget2::mousePressEvent(event);
}

void WITFrame::resizeEvent(QResizeEvent *e)
{
	//this->vtkWidget->resize(this->width(),this->height());
	QVTKWidget2::resizeEvent(e);
}

void WITFrame::createContextMenu()
{
	mnu_Context = new QMenu(this);
	QMenu *contextMenu = mnu_Context;
    QActionGroup *viewGroup = new QActionGroup(contextMenu);
	
    QAction *viewBottom = new QAction("Bottom", viewGroup);
    QAction *viewLeft = new QAction("Left",viewGroup);
    QAction *viewRight = new QAction("Right",viewGroup);
    QAction *viewCustom = new QAction("Custom", viewGroup);
    QAction *viewTop = new QAction("Top",viewGroup);
	QAction *viewFront = new QAction("Front", viewGroup);
	QAction *viewBack = new QAction("Back", viewGroup);

	QSignalMapper *map = new QSignalMapper();
	map->setMapping(viewBottom,(int)CameraView::BOTTOM);
	map->setMapping(viewTop,(int)CameraView::TOP);
	map->setMapping(viewRight, (int)CameraView::RIGHT);
	map->setMapping(viewLeft, (int)CameraView::LEFT);
	map->setMapping(viewCustom, (int)CameraView::CUSTOM);
	map->setMapping(viewFront, (int)CameraView::FRONT);
	map->setMapping(viewBack, (int)CameraView::BACK);

	connect(viewBottom, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewTop, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewLeft, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewRight, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewCustom, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewFront, SIGNAL(triggered()), map, SLOT(map()));
	connect(viewBack, SIGNAL(triggered()), map, SLOT(map()));

	connect(map, SIGNAL(mapped(int)), this, SLOT(setCameraView(int)));

    viewTop->setCheckable(true);
    viewBottom->setCheckable(true);
    viewRight->setCheckable(true);
    viewLeft->setCheckable(true);
    viewCustom->setCheckable(true);
	viewFront->setCheckable(true);
	viewBack->setCheckable(true);

    viewGroup->addAction(viewBottom);
    viewGroup->addAction(viewLeft);
    viewGroup->addAction(viewRight);
    viewGroup->addAction(viewCustom);
	viewGroup->addAction(viewFront);
	viewGroup->addAction(viewBack);
    viewGroup->addAction(viewTop);

    contextMenu->addAction(viewTop);
    contextMenu->addAction(viewBottom);
    contextMenu->addAction(viewLeft);
    contextMenu->addAction(viewRight);
	contextMenu->addAction(viewFront);
	contextMenu->addAction(viewBack);
    contextMenu->addAction(viewCustom);
}

void WITFrame::setCameraView(int view)
{
	emit this->onSetCameraView((CameraView)view);
}

void WITFrame::contextMenuEvent(QContextMenuEvent *ev)
{
	qDebug("CONTEXT MENU EVENT!");
    mnu_Context->exec(ev->globalPos());
}