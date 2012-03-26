#include "WITGroupPanel.h"
#include "WITGroupButton.h"
#include "../Controller/WITGroupPanelController.h"
#include <QPalette>
#include "../WITApplication.h"

WITGroupPanel::WITGroupPanel(QWidget *panel)
{
	QHBoxLayout *thbox = new QHBoxLayout();
	hbox  = new QHBoxLayout();
	bgroup = new QButtonGroup();
	thbox->addLayout(hbox);
	QPushButton *wgb = new QPushButton("Add Group", this);

	thbox->addWidget(wgb);
    setLayout(thbox);

	this->signalMap = 0;
	this->activeIndex = 0;

	// Create our controller
	controller = new WITGroupPanelController(this);
}

void WITGroupPanel::onActiveGroupSet(int index)
{
	buttons.at(index)->setChecked(true);
}

void WITGroupPanel::setNumberOfGroups(int num, Colord **colors)
{

	// Remove the old buttons
	for(std::vector<WITGroupButton*>::iterator it = buttons.begin(); it != buttons.end(); ++it)
	{
		(*it)->disconnect();
		hbox->removeWidget(*it);
		bgroup->removeButton(*it);
		delete (*it);
	}

	buttons.clear();

	if(this->signalMap)
	{
		this->signalMap->disconnect();
		delete this->signalMap;
	}

	this->signalMap = new QSignalMapper(this);
	connect(this->signalMap, SIGNAL(mapped(int)), this, SLOT(onSetActiveGroup(int)));

	if(this->activeIndex >= num)
		activeIndex = num - 1;

	// Add the new buttons
	for(int i = 0; i < num; i++)
	{
		WITGroupButton *b = new WITGroupButton(i, colors[i]);
		this->signalMap->setMapping(b, i);
		connect(b, SIGNAL(clicked()), this->signalMap, SLOT(map()));
        b->setCheckable(true);
        bgroup->addButton(b);
        hbox->addWidget(b);
		if(i == activeIndex)
			b->setChecked(true);
		buttons.push_back(b);
	}
}