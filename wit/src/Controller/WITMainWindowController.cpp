#include "WITMainWindowController.h"
#include "../WITApplication.h"
#include "WITFrameController.h"
#include "../View/WITFrame.h"

WITMainWindowController::WITMainWindowController()
{
	this->window = new WITMainWindow();
	this->app = &WITApplication::getInstance();
	this->connect(this->window, SIGNAL(loadPDB(std::string)), this->app->getPathwayController(), SLOT(loadPDB(std::string)));
	this->connect(this->window, SIGNAL(loadVolume(std::string)), this->app->getSubjectDataController(), SLOT(loadVolume(std::string)));
	this->connect(this->window, SIGNAL(undo()), this->app->getUndoSystem(), SLOT(undo()));
	this->connect(this->window, SIGNAL(redo()), this->app->getUndoSystem(), SLOT(redo()));
	// Load controllers for each of the main window's frames
	WITFrame** frames = this->window->getFrames();
	for(int i = 0; i < this->window->getNumFrames(); i++)
	{
		WITFrameController *cont = new WITFrameController(frames[i]);
	}
}

void WITMainWindowController::showWindow()
{
	this->window->show();
}