#ifndef PATHWAY_CONTROLLER_H
#define PATHWAY_CONTROLLER_H

#include <string>
#include <QObject>
#include <util/PDBHelper.h>
#include <util/PathwayGroup.h>
#include <util/ROIManager.h>
#include "../Model/WITPathwayViz.h"

class UndoSystem;
class WITApplication;

class PathwayController : public QObject
{
	Q_OBJECT
public:
	PathwayController(WITApplication *app);

	/**
		Returns the WITPathwayViz object for the pathway groups

		@return the WITPathwayViz object.
	*/
	WITPathwayViz &getPathwayViz(){return pathwayViz;}

	PathwayGroupArray *getGroupArray(){return &groupArray;}

	PDBHelper *getPDBHelper(){return &pdbHelper;}

	int getActiveGroupIndex(){return pdbHelper.Assignment().SelectedGroup();}

	void swapGroupArray(PathwayGroupArray &pgArray);
	void swapAssignment(DTIPathwayAssignment &assn);

signals:
	void pathwayVizLoaded(WITPathwayViz* viz);
	void pathwayVizUpdated();
	void pathwayGroupAdded(DTIPathwayGroupID id);
	void pathwayGroupsChanged();
	void pathwayAssignmentsChanged();
	void activeGroupSet(int index);
public slots:
	void loadPDB(std::string file);
	void setActiveGroup(int index);
	void onFilterByGesture(PCollModel model);
	
private:
	void addGroup(DTIPathwayGroupID id, std::string &name);
	PDBHelper pdbHelper;
	ROIManager roiManager;
	WITPathwayViz pathwayViz;
	PathwayGroupArray groupArray;
	WITApplication *app;
	UndoSystem *undoSystem;
};

#endif