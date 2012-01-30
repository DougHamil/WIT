#ifndef WITMAINWINDOW_H
#define WITMAINWINDOW_H

#define WITFRAME_COUNT 4

// - Qt
#include <QtGui/QMainWindow>
#include "./Qt/QuadSplit.h"
// - WIT
#include "WITFrame.h"
#include "WITGroupPanel.h"
#include "WITToolPanel.h"

/**
	The WITMainWindow is the main view element that holds all of the view elements of the application.
	It constructs all of its child view classes such as the WITGroupPanel and various dialogs.

	@author Doug Hamilton
*/
class WITMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	WITMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~WITMainWindow();
protected:

private:

	void createMenus();
	void createPanels();

	WITFrame *frames[WITFRAME_COUNT];
	QuadSplit *quadSplit;

	// - Menus
	QMenu *mnu_File;
	QMenu *mnu_Edit;
	QMenu *mnu_View;
	// - Panels
	WITGroupPanel *pnl_Group;
	WITToolPanel *pnl_Tool;
};

#endif // WITMAINWINDOW_H
