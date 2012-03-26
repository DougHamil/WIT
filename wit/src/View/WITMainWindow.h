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

	WITFrame** getFrames(){return frames;}
	int getNumFrames(){return WITFRAME_COUNT;}
protected:

public slots:
	void onLoadPDB();
	void onLoadVolume();
	void onUndo(){ emit undo();}
	void onRedo(){ emit redo();}
	void onSetViewCount(int count);
signals:
	void loadPDB(std::string filename);
	void loadVolume(std::string filename);
	void undo();
	void redo();
private:

	void createMenus();
	void createPanels();

	WITFrame *frames[WITFRAME_COUNT];
	QSplitter *activeSplitter;
	QuadSplit *quadSplit;
	QSplitter *dualSplit;
	QLayout *centralLayout;
	QWidget *centralWidget;
	// - Menus
	QMenu *mnu_File;
	QMenu *mnu_Edit;
	QMenu *mnu_View;

	// - Panels
	WITGroupPanel *pnl_Group;
	WITToolPanel *pnl_Tool;
};

#endif // WITMAINWINDOW_H
