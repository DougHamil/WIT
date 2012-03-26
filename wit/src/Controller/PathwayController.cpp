#include "PathwayController.h"

#include "../WITApplication.h"
#include "../Model/Undo/UndoSystem.h"
#include "../Model/Undo/PathwayMemento.h"

#include <util/DTIFilterROI.h>

PathwayController::PathwayController(WITApplication *app)
{
	this->app = app;
	this->undoSystem = app->getUndoSystem();
}

void PathwayController::loadPDB(std::string file)
{
	qDebug("Trying to load pdb file %s",file.c_str());

	DTIPathwayGroupID groupID = pdbHelper.Add(file);

	if(groupID <= (int)groupArray.size())
		groupArray[groupID].SetName(file);
	else
	{
		groupArray.Add(file);
		emit this->pathwayGroupAdded(groupID);
	}
	pathwayViz.GeneratePathways(pdbHelper, groupArray);

	emit this->pathwayVizLoaded(&pathwayViz);
}

void PathwayController::setActiveGroup(int index)
{
	if(index >= (int)groupArray.size() || index < 0)
		return;

	pdbHelper.Assignment().SetSelectedGroup(index);
	
	emit activeGroupSet(index);
}


void PathwayController::onFilterByGesture(PCollModel model)
{
	QTime t;
	// TEMP
	// create a voi from the collision model
	t.start();
	WITApplication::getInstance().getUndoSystem()->push(new PathwayMemento(this));
	qDebug("It took %d ms to store undo data.",t.elapsed());


	t.restart();
	DTIFilterROI voi(ROI_TYPE_GESTURE, model);
	qDebug("It took %d ms to generate DTIFilterROI.",t.elapsed());

	t.restart();
	BOOLEAN_OPERATION op = this->pathwayViz.FilterOperation();
	qDebug("It took %d ms to do FilterOperation.",t.elapsed());

	bool lock = false;

	t.restart();
	pdbHelper.Assignment().SetLocked(lock);
	DTIPathwaySelection selection = DTIPathwaySelection::PruneForGesture (op, &pdbHelper.Assignment(), groupArray);
	qDebug("It took %d ms to do Prune For Gesture.",t.elapsed());

	//qDebug("Selected group is %i.  %i eligible pathways for ROI selection...",pdbHelper.Assignment().SelectedGroup(), selection.NumSelectedPathways());
		
	// filter the pathways using this gesture
	t.restart();
	roiManager.Filter(selection, pdbHelper, &voi);
	qDebug("It took %d ms to do Filtering.",t.elapsed());
	
	// assign the fibers which have passed the test to the selected group or trash depending on the boolean operation 
	t.restart();
	pdbHelper.Assignment().AssignSelectedToGroup(selection, op != SUBTRACT ? pdbHelper.Assignment().SelectedGroup() : DTI_COLOR_UNASSIGNED, op != UNION);
	qDebug("It took %d ms to do assign to group.",t.elapsed());

	// update 3d data
	t.restart();
	this->pathwayViz.UpdatePathwaysColor(pdbHelper, groupArray);
	qDebug("It took %d ms to update pathways color.",t.elapsed());

	//Save the assignment
	t.restart();
	pdbHelper.AcceptFilterOperation(true);
	qDebug("It took %d ms to accept filter operation.",t.elapsed());

	t.restart();
	emit this->pathwayVizUpdated();
	qDebug("It took %d ms to handle this event signal.",t.elapsed());
}

void PathwayController::swapGroupArray(PathwayGroupArray &pgArray)
{
	std::swap(this->groupArray, pgArray);

	this->pathwayViz.UpdatePathwaysColor(pdbHelper, groupArray);

	emit this->pathwayGroupsChanged();
}

void PathwayController::swapAssignment(DTIPathwayAssignment &assn)
{
	DTIPathwayAssignment temp_assn = pdbHelper.Assignment();

	pdbHelper.SetAssignment(assn);

	assn = temp_assn;

	this->pathwayViz.UpdatePathwaysColor(pdbHelper, groupArray);

	emit this->pathwayAssignmentsChanged();
}