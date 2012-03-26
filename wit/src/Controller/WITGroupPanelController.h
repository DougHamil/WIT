#ifndef WITGROUPPANELCONTROLLER_H
#define WITGROUPPANELCONTROLLER_H

#include <QObject>
#include "../View/WITGroupPanel.h"
#include "PathwayController.h"

/**
	The WITGroupPanelController is the controller for the WITGroupPanel view.  The WITGroupPanel is displays all of the
	current pathway groups and allows the user to select the active one and change the visibility of each group.

	This controller hooks up the events from the WITGroupPanel to the underlying model.

	@author Doug Hamilton
*/
class WITGroupPanelController : public QObject
{
	Q_OBJECT
public:
	/**
		Constructor that requires the WITGroupPanel to control.
		@param panel the WITGroupPanel this controller will control.
	*/
	WITGroupPanelController(WITGroupPanel *panel);

public slots:

private:
	WITGroupPanel *panel;
	PathwayController *pathwayController;
};

#endif