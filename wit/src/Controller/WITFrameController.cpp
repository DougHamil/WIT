#include "WITFrameController.h"
#include "../WITApplication.h"


// - VTK
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>

WITFrameController::WITFrameController(WITFrame *fr)
{
	this->frame = fr;

	this->prepareFrame(this->frame, false);

	this->dupFrame = new WITFrame(0,fr);

	this->prepareFrame(this->dupFrame, true);

	this->dupFrame->setCamera(this->frame->getCamera());

	connect(WITApplication::getInstance().getPathwayController(), SIGNAL(pathwayVizLoaded(WITPathwayViz*)), this, SLOT(onPathwayVizLoaded(WITPathwayViz*)));
	connect(WITApplication::getInstance().getPathwayController(), SIGNAL(pathwayVizUpdated()), this, SLOT(onPathwayVizUpdated()));
	connect(WITApplication::getInstance().getSubjectDataController(), SIGNAL(onVolumeLoaded(VolumeInfo&)), this, SLOT(onVolumeLoaded(VolumeInfo&)));

	// Make sure when an undo event occurs, we re-render
	connect(WITApplication::getInstance().getUndoSystem(),SIGNAL(onUndo()), this, SLOT(onRender()));
	connect(WITApplication::getInstance().getUndoSystem(),SIGNAL(onRedo()), this, SLOT(onRender()));
}

void WITFrameController::prepareFrame(WITFrame *fr, bool isDup)
{
	vtkInteractorStyleWIT *is = vtkInteractorStyleWIT::New();

	if(!isDup)
		this->istyle = is;
	else
		this->dupIStyle = is;

	is->Init(fr);
	fr->getRenderWindow()->GetInteractor()->SetInteractorStyle(is);
	fr->setRenderer(fr->getRenderWindow()->GetRenderers()->GetFirstRenderer());
	connect(fr, SIGNAL(onSetCameraView(CameraView)), this, SLOT(onSetCameraView(CameraView)));
	connect(is, SIGNAL(filterByGesture(PCollModel)), WITApplication::getInstance().getPathwayController(), SLOT(onFilterByGesture(PCollModel)));
	connect(fr, SIGNAL(onToggleDuplicate()), this, SLOT(onToggleDuplicate()));
	connect(is, SIGNAL(doRender()), this, SLOT(onRender()));
	connect(is, SIGNAL(doMoveActiveVolumeSlice(int)), this, SLOT(onMoveActiveVolumeSlice(int)));
	if(!isDup)
		this->volumeViz = is->VolumeViz();
	else
		this->dupVolumeViz = is->VolumeViz();
}
void WITFrameController::onMoveActiveVolumeSlice(int d)
{
	this->volumeViz->MoveActiveImage(d);
	this->dupVolumeViz->MoveActiveImage(d);
}
void WITFrameController::onToggleDuplicate()
{
	this->dupFrame->setVisible(!this->dupFrame->isVisible());
}
void WITFrameController::onPathwayVizLoaded(WITPathwayViz *viz)
{
	// Generate and add pathway actor to first frame
	vtkActor *framePathwayActor = viz->getPathwayActor();

	this->frame->addActor(framePathwayActor);
	this->frame->setPathwayActor(framePathwayActor);

	// Do the same for the second frame
	vtkActor *a2 = viz->getPathwayActor();
	this->dupFrame->addActor(a2);
	this->dupFrame->setPathwayActor(a2);

	this->onRender();
}

void WITFrameController::onVolumeLoaded(VolumeInfo &info)
{
	this->volumeViz->AddVolume(info.Volume().get());
	this->dupVolumeViz->AddVolume(info.Volume().get());
	this->onRender();
}

void WITFrameController::onSetCameraView(CameraView view)
{
	// NOTE: frame's camera is shared with dupFrame's camera, so this is the same
	vtkCamera *cam = this->frame->getRenderer()->GetActiveCamera();

	//cam->SetPitch(0);
	//cam->SetYaw(0);
	
	switch(view)
	{
	case CameraView::TOP:
		cam->SetPosition(0,0,400);
		cam->SetViewUp(0,1,0);
		break;
	case CameraView::BOTTOM:
		cam->SetPosition(0,0,-400);
		cam->SetViewUp(0,1,0);
		break;
	case CameraView::LEFT:
		cam->SetPosition(-400,0,0);
		cam->SetViewUp(0,0,1);
		break;
	case CameraView::RIGHT:
		cam->SetPosition(400,0,0);
		cam->SetViewUp(0,0,1);
		break;
	case CameraView::FRONT:
		cam->SetPosition(0,400,0);
		break;
	case CameraView::BACK:
		cam->SetPosition(0,-400,0);
		break;
	}
	cam->SetFocalPoint(0,0,0);
	cam->SetRoll(0);
	this->istyle->GetCurrentRenderer()->ResetCameraClippingRange();
	this->dupIStyle->GetCurrentRenderer()->ResetCameraClippingRange();
	this->onRender();
}

void WITFrameController::onPathwayVizUpdated()
{
	//this->onRender();
	this->frame->updateGL();
	this->dupFrame->updateGL();
}

void WITFrameController::onRender()
{
	this->frame->GetRenderWindow()->Render();
	this->dupFrame->GetRenderWindow()->Render();
}