#include "WITViewController.h"
#include "../WITApplication.h"
#include "WITFrameController.h"
#include "vtkInteractorStyleWIT.h"

WITViewController::WITViewController(WITViewWidget *view)
{
	this->view = view;
	this->activeGestureController = 0;

	// Add controllers for each WITFrame
	WITFrame **frames = view->getFrames();

	for(int i = 0; i < view->getFrameCount(); i++)
	{
		this->frameToFrameController[frames[i]] = new WITFrameController(frames[i]);
		this->frameToGestureController[frames[i]] = new WITGestureController(frames[i]->getRenderer());
	}

	// Create our interactor style
	this->istyle = vtkInteractorStyleWIT::New();
	this->istyle->Init(this->view);

	// Hook up to istyle events
	connect(this->istyle, SIGNAL(beginGesture(int,int)), this, SLOT(onBeginGesture(int,int)));
	connect(this->istyle, SIGNAL(endGesture(int,int,bool)), this, SLOT(onEndGesture(int,int,bool)));
	connect(this->istyle, SIGNAL(addGesturePoint(int,int)), this, SLOT(onAddGesturePoint(int,int)));
	connect(this->istyle, SIGNAL(doMoveActiveVolumeSlice(int)), &WITApplication::getInstance().getSubjectDataController()->getVolumeViz(), SLOT(MoveActiveImage(int)));

	connect(WITApplication::getInstance().getPathwayController(), SIGNAL(pathwayVizLoaded(WITPathwayViz*)), this, SLOT(onPathwayVizLoaded(WITPathwayViz*)));

	// Make sure when an undo event occurs, we re-render
	connect(WITApplication::getInstance().getUndoSystem(),SIGNAL(onUndo()), this, SLOT(onRender()));
	connect(WITApplication::getInstance().getUndoSystem(),SIGNAL(onRedo()), this, SLOT(onRender()));

	//
	connect(this, SIGNAL(filterByGesture(PCollModel)), WITApplication::getInstance().getPathwayController(), SLOT(onFilterByGesture(PCollModel)));
}

void WITViewController::onPathwayVizLoaded(WITPathwayViz *viz)
{
	// Generate and add pathway actor to first frame
	this->pathwayActor = viz->getPathwayActor();

	// Add the actor to each frame
	WITFrame **frames = view->getFrames();
	for(int i = 0; i < view->getFrameCount(); i++)
	{
		frames[i]->getRenderer()->AddActor(this->pathwayActor);
	}
}

void WITViewController::onVolumeLoaded(VolumeInfo &info, WITVolumeViz &viz)
{
	// Add the actor to each frame
	WITFrame **frames = view->getFrames();
	for(int i = 0; i < view->getFrameCount(); i++)
	{
		viz.SetCameraToDefault(frames[i]->getRenderer());
	}
}

void WITViewController::onRender()
{
	this->view->update();
}

void WITViewController::onBeginGesture(int x, int y)
{
	qDebug("Gesture Pos: %d, %d",x,y);
	WITFrame *frame = this->view->getFrameAtPosition(x, this->view->height() - y);

	if(!frame)
		return;

	this->activeGestureController = this->frameToGestureController[frame];
	this->activeGestureFrame = frame;
	int *real = this->view->screenToFrame(this->activeGestureFrame,x,y);
	this->activeGestureController->BeginSelect(real[0],real[1]);
	delete real;
}

void WITViewController::onAddGesturePoint(int x, int y)
{
	if(!this->activeGestureController)
		return;
	int *real = this->view->screenToFrame(this->activeGestureFrame,x,y);
	this->activeGestureController->AddPoint(real[0],real[1]);
	delete real;
}

void WITViewController::onEndGesture(int x, int y, bool b)
{
	if(this->pathwayActor)
	{

		bool pathwayVisibility = this->pathwayActor->GetVisibility();
		//bool pointsVisiblity  = _pathwayViz->PointsVisibility();

		// Hide Pathways
		bool bfalse = false; 
		this->pathwayActor->SetVisibility(bfalse);

		//_pathwayViz->SetPointsVisibility(bfalse);

		PCollModel model;
		int *real = this->view->screenToFrame(this->activeGestureFrame,x,y);
		this->activeGestureController->EndSelect(real[0],real[1],b);
		delete real;
		// TODO: Handle either surface or touch mode
		if (true) {
			model = this->activeGestureController->PruneGesture2CollModel();
		} else if (false) {
			model = this->activeGestureController->SurfaceIntersectionGesture2CollModel();
		} else {
			qDebug("WARNING: Don't recognize gesture mode.");
		}

		this->pathwayActor->SetVisibility(pathwayVisibility);
		//_pathwayViz->SetPointsVisibility(pointsVisiblity);

		emit this->filterByGesture(model);
	}

	this->activeGestureController = 0;
	this->activeGestureFrame = 0;
}