#ifndef VOIMEMENTO_H
#define VOIMEMENTO_H

#include "Memento.h"

/**
	This class stores the state of a single VOI in the scene.

	Generally, this class should not be used directly but instead
	should be used by the VOICollectionMemento class which stores
	the state of all of the VOIs in the scene.

	@author Doug Hamilton
*/
class VOIMemento : public Memento
{
};

#endif