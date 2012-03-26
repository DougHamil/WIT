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
		emit this->onVolumeLoaded(subjectData[subjectData.size()-1]);
	}
}