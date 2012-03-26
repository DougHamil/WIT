#ifndef SUBJECT_DATA_CONTROLLER_H
#define SUBJECT_DATA_CONTROLLER_H

#include <QObject>
#include <string>
#include <util/SubjectData.h>

class WITApplication;

class SubjectDataController : public QObject
{
	Q_OBJECT
public:
	SubjectDataController(WITApplication *app);
public slots:
	void loadVolume(std::string filename);
signals:
	void onVolumeLoaded(VolumeInfo &info);
private:
	WITApplication *app;
	SubjectData subjectData;
};

#endif