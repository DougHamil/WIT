#include "WITMainWindow.h"

// - Qt
#include <QtGui>

WITMainWindow::WITMainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	this->setWindowTitle("WIT");

	// Create each WITFrame
	for(int i = 0; i < WITFRAME_COUNT; i++)
	{
		this->frames[i] = new WITFrame();
	}

	// Add the WITFrames to the split
	quadSplit = new QuadSplit(frames[0], frames[1], frames[2], frames[3], this);

	// Make the quadsplit our central widget
	this->setCentralWidget(quadSplit);
	resize(800,600);
	quadSplit->show();

	this->createMenus();
	this->createPanels();
}

WITMainWindow::~WITMainWindow()
{

}

void WITMainWindow::createMenus()
{
	mnu_File = new QMenu(tr("&File"), this);

	this->menuBar()->addMenu(mnu_File);
}

void WITMainWindow::createPanels()
{
	/* 
		Tool Dock
	*/
	QDockWidget *toolDock = new QDockWidget("Tools", this);
    toolDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    toolDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    toolDock->setTitleBarWidget(new QWidget(this));

	pnl_Tool = new WITToolPanel(toolDock);

	toolDock->setWidget(pnl_Tool);

	addDockWidget(Qt::TopDockWidgetArea, toolDock);

	/*
		Group Dock
	*/
	QDockWidget *groupDock = new QDockWidget("Pathways", this);
	groupDock->setAllowedAreas(Qt::BottomDockWidgetArea);
	groupDock->setFeatures(QDockWidget::NoDockWidgetFeatures);

	// We don't want a title bar, so we set an empty QWidget object to remove it
	groupDock->setTitleBarWidget(new QWidget(this));
	
	pnl_Group = new WITGroupPanel(groupDock);
	groupDock->setWidget(pnl_Group);

	this->addDockWidget(Qt::BottomDockWidgetArea, groupDock);

}
