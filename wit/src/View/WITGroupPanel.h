#ifndef WIT_GROUP_PANELH
#define WIT_GROUP_PANELH

#include <QWidget>
#include <QtGui>
#include <QSignalMapper>
#include <vector>

#include "WITGroupButton.h"

class WITGroupPanelController;

/**
	The WITGroupPanel is part of the view that displays the current pathway groups and
	allows the user to select an active pathway group as well as set the visibility of
	each group.

	@author Doug Hamilton
*/
class WITGroupPanel : public QWidget
{
	Q_OBJECT

public:
	explicit WITGroupPanel(QWidget *parent = 0);

	/**
		Sets the number of pathway group buttons.  This is called whenever
		the number of pathway groups changes which allows this panel to rebuild
		the buttons.

		@param num the number of pathway groups to create buttons for
	*/
	void setNumberOfButtons(int num);

private slots:
	
	/** 
		Receives a signal from one of the pathway group buttons with an associated 
		button index.  This simply emits the setActiveGroup signal.

		@param index the index of the pathway group button pressed.
	*/
	void onSetActiveGroup(int index){emit setActiveGroup(index);}

signals:

	/**
		This signal is emitted whenever the user changes the active group.

		@param index the index of the active group the user wants to set.
	*/
	void setActiveGroup(int index);

private:
	QHBoxLayout *hbox;
    QButtonGroup *bgroup;
	std::vector<WITGroupButton*> buttons;
	WITGroupPanelController *controller;
	QSignalMapper *signalMap;
};

#endif