#include "WITApplication.h"
//#include "Test/UnitTests.h"
#include <QtGlobal>

void WITApplication::initialize(int argc, char *argv[])
{
	// Create qt application
	qApplication = new QApplication(argc, argv);

	// Construct the model
	this->undoSystem = new UndoSystem(this);
	this->pathwayController = new PathwayController(this);
	this->subjectDataController = new SubjectDataController(this);

	this->mainWinController = new WITMainWindowController();
	this->mainWinController->showWindow();


	for(int i = 0; i < argc;i++)
	{
		qDebug("Arg: %s",argv[i]);
		if(strcmp(argv[i],"-timepdb") == 0)
		{
			qDebug("Timing PDB loading:\n=====================");
			//WIT_Time_PDBLoad(this, std::string(argv[++i]));
		}

		if(strcmp(argv[i],"-pdb") == 0)
		{
			this->pathwayController->loadPDB(std::string(argv[++i]));
		}
		else if(strcmp(argv[i], "-vol") == 0)
		{
			this->subjectDataController->loadVolume(std::string(argv[++i]));
		}
	}
	
}

int WITApplication::exec()
{
	return this->qApplication->exec();
}