#ifndef PATHWAY_MEMENTO
#define PATHWAY_MEMENTO

#include "Memento.h"
#include <util/PDBHelper.h>
#include <util/PathwayGroup.h>

class PathwayController;

class PathwayMemento : public Memento
{
public:
	PathwayMemento(PathwayController *controller);
	void Redo();
	void Undo();
private:
	PathwayGroupArray pgArray;
	DTIPathwayAssignment pathwayAssignment;
	PathwayController *pc;
};

#endif