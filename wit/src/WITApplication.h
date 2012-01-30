/*
  File:     Model/Systems/WITApplication.h
  Summary:  The entire WITApplication.  Responsible for creating all of the various
            systems and hooking them together.  Basically composes the entire application
            state.
*/

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

class WITApplication
{
public:
	void initialize(int argc, char *argv[]);
	int exec();

	QApplication *getQApplication(){return qApplication;}
	WITMainWindow *getMainWindiow(){return win_Main;}
	GroupCollection *getGroups(){return groups;}
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
	// - Singleton
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