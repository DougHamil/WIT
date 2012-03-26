#include "WITGroupPanelController.h"
#include "../WITApplication.h"

#include <util/PathwayGroup.h>

// TEMP
#include <iostream>

WITGroupPanelController::WITGroupPanelController(WITGroupPanel *panel)
{
	this->panel = panel;
	this->pathwayController = WITApplication::getInstance().getPathwayController();

	PathwayGroupArray *groupArray = pathwayController->getGroupArray();

	Colord **colors = new Colord*[groupArray->size()];

	for(int i = 0; i < groupArray->size(); i++)
	{
		colors[i] = &groupArray->at(i).Color();
	}

	this->panel->setNumberOfGroups(groupArray->size(), colors);

	connect(this->panel, SIGNAL(setActiveGroup(int)), this->pathwayController, SLOT(setActiveGroup(int)));
	connect(this->pathwayController, SIGNAL(activeGroupSet(int)), this->panel, SLOT(onActiveGroupSet(int)));
}