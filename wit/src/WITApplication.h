#ifndef WIT_APPLICATION_H
#define WIT_APPLICATION_H

// - Qt
#include <QtGui/QApplication>
// - WIT
#include "View/WITMainWindow.h"
#include "Model/GroupCollection.h"
#include "Model/Undo/UndoSystem.h"
// - STL
#include <vector>

/**
	WITApplication is the main application class for WIT.  It initializes the models, controllers, and views.
	It is a singleton which can be easily accessed using the getInstance() method.  Various components can
	access one another through this class.

	@author Doug Hamilton
*/
class WITApplication
{
public:
	/**
		This method initializes the WITApplication with the given command line arguments
		
		@param argc number of command line arguments
		@param argv array of command line arguments
	*/
	void initialize(int argc, char *argv[]);

	/**
		Executes the WITApplication and returns the QApplication error code.

		@return QApplication error code
	*/
	int exec();

	/**
		Returns a reference to the underlying QApplication

		@return Reference to the underlying QApplication
	*/
	QApplication *getQApplication(){return qApplication;}

	/**
		Returns the WITMainWindow object.

		@return the WITMainWindow object
	*/
	WITMainWindow *getMainWindiow(){return win_Main;}

	/**
		Return the collection of Group objects, each representing a pathway group

		@return A GroupCollection object containing each of the pathway groups.
	*/
	GroupCollection *getGroups(){return groups;}

	/**
		Gets the UndoSystem for the application.

		@return The UndoSystem object for the application
	*/
	UndoSystem *getUndoSystem(){return undoSystem;}

private:
	QApplication *qApplication;
	WITMainWindow *win_Main;
	GroupCollection *groups;
	UndoSystem *undoSystem;


/*
	Singleton
*/
public:
	/**
		Returns the singleton instance of the WITApplication and creates one if 
		it isn't already created.

		@return the WITApplication singleton instance
	*/
	static WITApplication& getInstance()
	{
		static WITApplication instance;
		return instance;
	}

private:

	// - Singleton
	WITApplication(){};
	WITApplication(WITApplication const&);
	void operator=(WITApplication const&);
	


};

#endif