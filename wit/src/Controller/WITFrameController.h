/*
  File:     WITFrameController.h
  Summary:  The controller for the WITFrame view.  Responds to user interactions
            by generating events and passing them to the EventInterpreter.
*/

#ifndef WITFRAMECONTROLLER_H
#define WITFRAMECONTROLLER_H
#include <QObject>
#include "../View/WITFrame.h"
#include "../Model/WITPathwayViz.h"
#include "../View/WITVolumeViz.h"
#include "vtkInteractorStyleWIT.h"
#include <util/SubjectData.h>
#include <vtkActor.h>

class WITFrameController : QObject
{
	Q_OBJECT
public:
	WITFrameController(WITFrame *fr);
public slots:
	void onPathwayVizUpdated();
	void onPathwayVizLoaded(WITPathwayViz *viz);
	void onVolumeLoaded(VolumeInfo &info);
	void onSetCameraView(CameraView view);
	void onRender();
signals:
	void moveVolumeSlice(DTISceneActorID slice, int pos);
private:
	void prepareFrame(WITFrame *fr, bool isDup);
	WITFrame *frame;

};

#endif