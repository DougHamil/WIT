#ifndef UNDOSYSTEM_H
#define UNDOSYSTEM_H

// WIT
#include "Memento.h"
// STL
#include <stack>

/**
* UndoSystem manages the undo stack of the application.  Objects may push a memento or pop the top
* memento from the stack.  The UndoSystem will tell the memento to act on the model when it is popped.

@author Doug Hamilton
*/
class UndoSystem
{
public:
	UndoSystem();

	/**
		Pushes a new stack onto the undo stack.  This is useful for operations that may be reversed
		such as previewing a possible query.
	*/
	void pushStack();

	/**
		Pops a previously pushed stack from the undo stack.  This is useful for canceling a set of operations
		since the last pushStack call.
	*/
	void popStack();

	/**
		Collapses the active stack onto the undo stack.  This is useful for making a temporary stack (created from pushStack) 
		permanent.
	*/
	void collapseStack();

	/**
		Pushes a memento onto the active stack.
		@param memento The memento to push onto the active stack.
	*/
	void push(Memento *memento);

	/**
		Returns to the state to the last memento.
	*/
	void undo();

	/**
		Redoes the last undone memento.
	*/
	void redo();
signals:

private:
	std::stack<std::stack<Memento*>*> undoStack;
	std::stack<std::stack<Memento*>*> redoStack;
};

#endif