#include "WITApplication.h"


void WITApplication::initialize(int argc, char *argv[])
{
	// Construct the model
	// - The group collection
	this->groups = new GroupCollection();

	// Construct the view (which will construct its own controllers)
	qApplication = new QApplication(argc, argv);
	win_Main = new WITMainWindow();
	win_Main->show();
}

int WITApplication::exec()
{
	return this->qApplication->exec();
}