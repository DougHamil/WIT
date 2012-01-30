#ifndef GROUPCOLLECTIONMEMENTO_H
#define GROUPCOLLECTIONMEMENTO_H

#include "Memento.h"
#include "GroupMemento.h"

/**
	The GroupCollectionMemento stores a memento for each of the pathway groups
	as well as the number of pathway groups.

	This class is usually created when any action is taken on the pathway groups.
	Generally, the GroupMemento class should not be used outside of this class as it
	only stores the state of a single pathway group.

	@author Doug Hamilton
*/
class GroupCollectionMemento : public Memento
{

};

#endif