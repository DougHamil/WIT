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

	connect(WITApplication::getInstance().getPathwayController(), SIGNAL(pathwayVizLoaded(WITPathwayViz*)), this, SLOT(onPathwayVizLoaded(WITPathwayViz*)));
	connect(WITApplication::getInstance().getPathwayController(), SIGNAL(pathwayVizUpdated()), this, SLOT(onPathwayVizUpdated()));
	connect(WITApplication::getInstance().getSubjectDataController(), SIGNAL(onVolumeLoaded(VolumeInfo&)), this, SLOT(onVolumeLoaded(VolumeInfo&)));

	// Register our frame's renderer with the volume visualization
	WITApplication::getInstance().getSubjectDataController()->getVolumeViz().RegisterRenderer(this->frame->getRenderer());
}

void WITFrameController::prepareFrame(WITFrame *fr, bool isDup)
{
	connect(fr, SIGNAL(onSetCameraView(CameraView)), this, SLOT(onSetCameraView(CameraView)));
}

void WITFrameController::onPathwayVizLoaded(WITPathwayViz *viz){}

void WITFrameController::onVolumeLoaded(VolumeInfo &info){}

void WITFrameController::onSetCameraView(CameraView view)
{
	qDebug("Set camera for frame %p",this->frame);
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
	this->frame->getRenderer()->ResetCameraClippingRange();
}

void WITFrameController::onPathwayVizUpdated()
{

}

void WITFrameController::onRender()
{

}