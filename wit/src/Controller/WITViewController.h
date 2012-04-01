#ifndef WIT_VIEW_CONTROLLERH
#define WIT_VIEW_CONTROLLERH

#include <QtGui>

#include "../View/WITViewWidget.h"
#include "../View/WITVolumeViz.h"
#include "../View/WITFrame.h"
#include "../Model/WITPathwayViz.h"
#include "WITGestureController.h"
#include <util/typedefs.h>
#include <map>

class vtkInteractorStyleWIT;
class WITFrameController;

class WITViewController : public QObject
{
	Q_OBJECT
public:
	WITViewController(WITViewWidget *view);
signals:
	void filterByGesture(PCollModel model);
public slots:
	void onRender();
	void onBeginGesture(int,int);
	void onEndGesture(int,int,bool);
	void onAddGesturePoint(int,int);
	void onPathwayVizLoaded(WITPathwayViz *viz);
	void onVolumeLoaded(VolumeInfo &info, WITVolumeViz &viz);
private:
	WITViewWidget *view;
	vtkInteractorStyleWIT *istyle;
	std::map<WITFrame*, WITFrameController*> frameToFrameController;
	std::map<WITFrame*, WITGestureController*> frameToGestureController;
	WITGestureController *activeGestureController;
	WITFrame *activeGestureFrame;

	// Visualization
	vtkActor *pathwayActor;
};

#endif