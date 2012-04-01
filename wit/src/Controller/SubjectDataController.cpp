#include "SubjectDataController.h"
#include "../WITApplication.h"

#include <vtkImageReader.h>

SubjectDataController::SubjectDataController(WITApplication *app)
{
	this->app = app;
}

void SubjectDataController::loadVolume(std::string filename)
{
	if(subjectData.LoadVolume(filename))
	{
		this->volumeViz.AddVolume(subjectData[subjectData.size()-1].Volume().get());

		emit this->onVolumeLoaded(subjectData[subjectData.size()-1], this->volumeViz);
	}
}

void SubjectDataController::onMoveImageSlice(int dist)
{
	this->volumeViz.MoveActiveImage(dist);

	emit this->moveImageSlice(this->volumeViz.ActiveImage(), dist);
}