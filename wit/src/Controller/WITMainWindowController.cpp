#include "WITMainWindowController.h"
#include "../WITApplication.h"
#include "WITFrameController.h"
#include "../View/WITFrame.h"
#include "WITViewController.h"

WITMainWindowController::WITMainWindowController(WITMainWindow *window)
{
	this->window = window;
	this->app = &WITApplication::getInstance();
	this->connect(this->window, SIGNAL(loadPDB(std::string)), this->app->getPathwayController(), SLOT(loadPDB(std::string)));
	this->connect(this->window, SIGNAL(loadVolume(std::string)), this->app->getSubjectDataController(), SLOT(loadVolume(std::string)));
	this->connect(this->window, SIGNAL(undo()), this->app->getUndoSystem(), SLOT(undo()));
	this->connect(this->window, SIGNAL(redo()), this->app->getUndoSystem(), SLOT(redo()));

	// Add the controller for the view widget
	new WITViewController(this->window->getViewWidget());
}

void WITMainWindowController::showWindow()
{
	this->window->show();
}