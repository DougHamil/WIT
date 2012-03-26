#include "CompleteMemento.h"
#include "../../WITApplication.h"
#include "PathwayMemento.h"
CompleteMemento::CompleteMemento(WITApplication *app)
{
	this->pMemento = new PathwayMemento(app->getPathwayController());
}