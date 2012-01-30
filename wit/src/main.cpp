/*! \mainpage WIT
	\section intro_sec Introduction
	The Wetware Investigation Tool (WIT) is a tool for investigating white matter pathway estimates using gestures, volumes of interest (VOIs), and
	pathway statistics.

	\section arch_sec Program Architecture
	WIT follows the Model-View-Controller (MVC) architectural pattern.  This pattern ensures a separation between the user interface elements and the
	underlying application model and logic.  The directory structure of the source code reflects the separation of the classes into either Model, View, 
	or Controller.

	\section library_sec External Libraries
	WIT is built using the popular Qt window system which offers useful features and platform agnostic development.  WIT uses the Visualization Toolkit
	(VTK) to handle rendering the complex data sets WIT is designed to interact with.
*/

#include "WITApplication.h"

int main(int argc, char *argv[])
{
	WITApplication::getInstance().initialize(argc, argv);
	return WITApplication::getInstance().exec();
}
