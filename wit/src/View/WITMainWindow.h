#ifndef WITMAINWINDOW_H
#define WITMAINWINDOW_H


// - Qt
#include <QtGui/QMainWindow>
#include "./Qt/QuadSplit.h"
// - WIT
#include "WITViewWidget.h"
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
	WITViewWidget *getViewWidget(){return this->viewWidget;}
protected:

public slots:
	void onLoadPDB();
	void onLoadVolume();
	void onUndo(){ emit undo();}
	void onRedo(){ emit redo();}
signals:
	void loadPDB(std::string filename);
	void loadVolume(std::string filename);
	void undo();
	void redo();
private:

	void createMenus();
	void createPanels();

	QLayout *centralLayout;
	WITViewWidget *viewWidget;
	// - Menus
	QMenu *mnu_File;
	QMenu *mnu_Edit;
	QMenu *mnu_View;

	// - Panels
	WITGroupPanel *pnl_Group;
	WITToolPanel *pnl_Tool;
};

#endif // WITMAINWINDOW_H
