#ifndef SUBJECT_DATA_CONTROLLER_H
#define SUBJECT_DATA_CONTROLLER_H

#include <QtGui>
#include <string>
#include <util/SubjectData.h>
#include "../View/WITVolumeViz.h"
class WITApplication;

class SubjectDataController : public QObject
{
	Q_OBJECT
public:
	SubjectDataController(WITApplication *app);
	WITVolumeViz &getVolumeViz(){return this->volumeViz;}
public slots:
	void loadVolume(std::string filename);
	void onMoveImageSlice(int dist);
signals:
	void onVolumeLoaded(VolumeInfo &info, WITVolumeViz &viz);
	void moveImageSlice(DTISceneActorID id, int dist);
private:
	WITVolumeViz volumeViz;
	WITApplication *app;
	SubjectData subjectData;
};

#endif