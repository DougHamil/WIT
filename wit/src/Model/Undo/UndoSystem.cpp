#include "UndoSystem.h"

#include "../../WITApplication.h"
#include "CompleteMemento.h"

UndoSystem::UndoSystem(WITApplication *app)
{
	this->app = app;

	// Push the bottom most stack
	this->undoStack.push(new std::stack<Memento*>());
	this->redoStack.push(new std::stack<Memento*>());
}

void UndoSystem::pushStack()
{
	// First, create a complete memento
	this->push(new CompleteMemento(this->app));

	// Push a new stack on our undo and redo stack
	this->undoStack.push(new std::stack<Memento*>());
	this->redoStack.push(new std::stack<Memento*>());
}

void UndoSystem::undo()
{
	// Make sure there is something to undo!
	if(this->undoStack.top()->size() <= 0)
		return;

	Memento *topmost = this->undoStack.top()->top();
	topmost->Undo();
	this->undoStack.top()->pop();

	this->redoStack.top()->push(topmost);

	emit this->onUndo();
}

void UndoSystem::redo()
{
	// Make sure there is something to redo!
	if(this->redoStack.top()->size() <= 0)
		return;

	Memento *topmost = this->redoStack.top()->top();
	topmost->Redo();
	this->redoStack.top()->pop();

	this->undoStack.top()->push(topmost);

	emit this->onRedo();
}

void UndoSystem::popStack()
{
	// remove the stack and delete
	std::stack<Memento*> *topmost = this->undoStack.top();

	topmost->empty();

	delete topmost;

	// Undo the checkpoint
	Memento* checkpoint = this->undoStack.top()->top();
	checkpoint->Undo();
	this->undoStack.top()->pop();
	delete checkpoint;

	// Remove the redo stack
	while(!this->redoStack.top()->empty())
	{
		delete this->redoStack.top()->top();
		this->redoStack.top()->pop();
	}

	delete this->redoStack.top();
	this->redoStack.top()->pop();
}

void UndoSystem::push(Memento *mem)
{
	// Push the mem onto the top of the active stack
	this->undoStack.top()->push(mem);
	
	// Clear the redo stack
	while(!this->redoStack.top()->empty())
	{
		delete this->redoStack.top()->top();
		this->redoStack.top()->pop();
	}
}