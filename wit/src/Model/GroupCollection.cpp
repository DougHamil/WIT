#include "GroupCollection.h"

GroupCollection::GroupCollection()
{
	activeGroup = NULL;

	// Add some groups
	for(int i = 0; i < INITIAL_GROUP_COUNT; i++)
	{
		if(!activeGroup)
		{
			activeGroup = new Group();
			this->addGroup(activeGroup, false);
		}
		else
		{
			this->addGroup(new Group(), false);
		}
	}

	emit groupAdded();
}

void GroupCollection::addGroup(Group *group, bool signal)
{
	this->groups.push_back(group);

	if(signal)
		emit groupAdded();
}

// TODO
void GroupCollection::removeGroup(int index)
{
}

// TODO
Memento* GroupCollection::generateMemento(){return 0;}

// TODO
void GroupCollection::restoreToMemento(Memento *memento){}

void GroupCollection::addNewGroup()
{
	// Just add an empty group, which will emit the onGroupAdded signal
	this->addGroup(new Group());
}

void GroupCollection::setActiveGroup(int index)
{
	this->activeGroup = this->groups.at(index);

	emit activeGroupSet(index);
}

Group *GroupCollection::getActiveGroup()
{
	return activeGroup;
}

int GroupCollection::getActiveGroupIndex()
{
	int index = 0;
	for(std::vector<Group*>::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		if((*it) == this->activeGroup)
			return index;

		++index;
	}

	return -1;
}

