#include "WITGroupPanelController.h"
#include "../WITApplication.h"

// TEMP
#include <iostream>

WITGroupPanelController::WITGroupPanelController(WITGroupPanel *panel)
{
	this->panel = panel;

	// Connect us to the application's GroupCollection
	connect(WITApplication::getInstance().getGroups(), SIGNAL(groupAdded()), this, SLOT(onGroupAdded()));
	connect(WITApplication::getInstance().getGroups(), SIGNAL(activeGroupSet(int)), this, SLOT(onActiveGroupSet(int)));

	// Connect our panel to the GroupCollection
	connect(panel, SIGNAL(setActiveGroup(int)), WITApplication::getInstance().getGroups(), SLOT(setActiveGroup(int)));
}

void WITGroupPanelController::onGroupAdded()
{
	this->panel->setNumberOfButtons(WITApplication::getInstance().getGroups()->getGroups().size());
}

void WITGroupPanelController::onActiveGroupSet(int id)
{

	qDebug("Changed id to: %i",id);
	//std::cout << "Changed active group to: " << id << std::endl;
}