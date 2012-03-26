#ifndef WIT_MAIN_WINDOW_CONTROLLER
#define WIT_MAIN_WINDOW_CONTROLLER

#include <QObject>

#include "../View/WITMainWindow.h"

class WITApplication;

class WITMainWindowController : public QObject
{
	Q_OBJECT
public:
	WITMainWindowController();
	void showWindow();
private:
	WITMainWindow *window;
	WITApplication *app;
};

#endif