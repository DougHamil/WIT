/*! \mainpage WIT
	\section intro_sec Introduction
	This is the introduction.
*/

#include "Model/WITApplication.h"

int main(int argc, char *argv[])
{
	WITApplication::getInstance().initialize(argc, argv);
	return WITApplication::getInstance().exec();
}
