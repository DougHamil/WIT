#include "../WITApplication.h"
#include <string>
#include <QTime>
#include <util/DTIPathwayDatabase.h>
#include <QtGlobal>

void WIT_Time_PDBLoad(WITApplication *app, std::string file)
{
	QTime t;
	t.start();
	app->getPathwayController()->loadPDB(file);
	
#ifdef USE_RAPID
	qDebug("Using RAPID, loading %d fibers took %d ms", app->getPathwayController()->getPDBHelper()->PDB().get()->getNumFibers(), t.elapsed() );
#else
	qDebug("Using OPCODE, loading %d took %d ms", app->getPathwayController()->getPDBHelper()->PDB().get()->getNumFibers(), t.elapsed());
#endif
}