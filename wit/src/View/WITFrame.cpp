#include "WITFrame.h"

// - Qt
#include <QtGui>

// - TEMP
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h> 
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <iostream>

WITFrame::WITFrame(QWidget *parent)
	: QWidget(parent)
{
	this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	this->renderWindow = new vtkQtGraphicsViewRenderWindow(this);
	this->renderWindow->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	// Create the renderer
	this->renderer = vtkRenderer::New();
	this->renderWindow->AddRenderer(this->renderer);

	// - TEMP
	// create sphere geometry 
	vtkSphereSource *sphere = vtkSphereSource::New(); 
	sphere->SetRadius(1.0); 
	sphere->SetThetaResolution(18); 
	sphere->SetPhiResolution(18);

	// map to graphics library 
	vtkPolyDataMapper *map = vtkPolyDataMapper::New(); 
	map->SetInput(sphere->GetOutput());

	// actor coordinates geometry, properties, transformation 
	vtkActor *aSphere = vtkActor::New(); 
	aSphere->SetMapper(map); 
	aSphere->GetProperty()->SetColor(0,0,1); // sphere color blue
	
	this->renderer->AddActor(aSphere);
	this->renderer->SetBackground(1,1,1);

	this->renderWindow->show();

	this->createContextMenu();
}

void WITFrame::resizeEvent(QResizeEvent *e)
{
	this->renderWindow->resize(this->width(),this->height());
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

    viewTop->setCheckable(true);
    viewBottom->setCheckable(true);
    viewRight->setCheckable(true);
    viewLeft->setCheckable(true);
    viewCustom->setCheckable(true);

    viewGroup->addAction(viewBottom);
    viewGroup->addAction(viewLeft);
    viewGroup->addAction(viewRight);
    viewGroup->addAction(viewCustom);
    viewGroup->addAction(viewTop);

    contextMenu->addAction(viewTop);
    contextMenu->addAction(viewBottom);
    contextMenu->addAction(viewLeft);
    contextMenu->addAction(viewRight);
    contextMenu->addAction(viewCustom);
}

void WITFrame::contextMenuEvent(QContextMenuEvent *ev)
{
    mnu_Context->exec(ev->globalPos());
}