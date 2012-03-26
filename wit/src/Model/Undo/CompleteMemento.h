#ifndef COMPLETEMEMENTO_H
#define COMPLETEMEMENTO_H

#include "Memento.h"


class PathwayMemento;
class WITApplication;

/**
	This class stores the state of the entire application at once.
	This is generally used by the UndoSystem directly when dealing with
	temporary undo stacks, as it provides an easy way to "checkpoint" the 
	entire application at once and revert to that checkpoint with ease. This
	is useful for the UndoSystem when we need to push a temporary undo stack
	which may need to be rolled back, so we can just roll back to this checkpoint
	that is created when the temporary undo stack was pushed.

	@author Doug Hamilton
*/
class CompleteMemento : public Memento
{
public:
	CompleteMemento(WITApplication *app);
	virtual void Undo(){};
	virtual void Redo(){};
	virtual void toStream(std::ostream &os){}
private:
	PathwayMemento *pMemento;
};

#endif