#ifndef GROUPCOLLECTION_H
#define GROUPCOLLECTION_H

// - WIT
#include "Group.h"
#include "Undo/Memento.h"
// - Qt
#include <QObject>
// - STL
#include <vector>

/**
	GroupCollection maintains a collection of pathway groups.  It emits signals on various
	operations such as adding or removing a group that controllers will respond to.

	@author Doug Hamilton
*/
class GroupCollection : QObject
{
	Q_OBJECT
public:
	GroupCollection();

	/**
		Generates and returns a memento for the current states of the group collection.

		@return A <code>Memento</code> storing the current state of the group collection.
	*/
	Memento *generateMemento();

	/**
		Restores the GroupCollection to the state stored in the given <code>Memento</code>

		@param memento the <code>Memento</code> to restore the state of the GroupCollection to.
	*/
	void restoreToMemento(Memento *memento);

	/**
		Returns a std::vector of the groups

		@return <code>std::vector</code> of the groups
	*/
	std::vector<Group*>& getGroups(){return groups;}
signals:
	/**
		Signal that is emitted when a group is added to the collection
	*/
	void onGroupAdded();

	/**
		Signal that is emitted when a group is removed from the collection
	*/
	void onGroupRemoved();

public slots:
	/**
		Adds a new group to the collection
	*/
	void addNewGroup(){};

	/**
		Removes the group at a given index.
		@param index index of the group to remove
	*/
	void removeGroup(int index){};

private:
	std::vector<Group*> groups;

};

#endif