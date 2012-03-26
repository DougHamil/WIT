#include "PathwayMemento.h"
#include "../../Controller/PathwayController.h"

PathwayMemento::PathwayMemento(PathwayController *con)
{
	this->pc = con;
	this->pathwayAssignment = con->getPDBHelper()->Assignment();
	this->pgArray = *con->getGroupArray();
}

void PathwayMemento::Redo()
{
	this->pc->swapGroupArray(this->pgArray);
	this->pc->swapAssignment(this->pathwayAssignment);
}
void PathwayMemento::Undo()
{
	this->pc->swapGroupArray(this->pgArray);
	this->pc->swapAssignment(this->pathwayAssignment);
}