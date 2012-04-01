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

WITFrame::WITFrame()
{
	this->renderer = vtkRenderer::New();
	this->createContextMenu();
	this->renderer->SetBackground(0.74, 0.74, 0.66);
	this->setCamera(vtkCamera::New());
}

vtkCamera *WITFrame::getCamera()
{
	return this->renderer->GetActiveCamera();
}
void WITFrame::setCamera(vtkCamera *newCam)
{
	this->renderer->SetActiveCamera(newCam);
	newCam->Delete();
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

void WITFrame::createContextMenu()
{
	mnu_Context = new QMenu();
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

void WITFrame::showContextMenu(QContextMenuEvent *ev)
{
    mnu_Context->exec(ev->globalPos());
}