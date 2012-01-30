#ifndef GROUPCOLLECTION_H
#define GROUPCOLLECTION_H

// - WIT
#include "Group.h"
#include "Undo/Memento.h"
// - Qt
#include <QObject>
// - STL
#include <vector>

#define INITIAL_GROUP_COUNT 8

/**
	GroupCollection maintains a collection of pathway groups.  It emits signals on various
	operations such as adding or removing a group that controllers will respond to.

	@author Doug Hamilton
*/
class GroupCollection : public QObject
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

	/**
		Returns the currently active group

		@return the active Group object
	*/
	Group *getActiveGroup();

	/**
		Returns the index of the currently active group.

		@return the index of the active Group object.
	*/
	int getActiveGroupIndex();
signals:
	/**
		Signal that is emitted when a group is added to the collection
	*/
	void groupAdded();

	/**
		Signal that is emitted when a group is removed from the collection
	*/
	void groupRemoved();

	/**
		Signal that is emitted when the active group is set

		@param index the index of the active group
	*/
	void activeGroupSet(int index);

public slots:
	/**
		Adds a new group to the collection
	*/
	void addNewGroup();

	/**
		Removes the group at a given index.
		@param index index of the group to remove
	*/
	void removeGroup(int index);

	/**
		Sets the active Group object to the Group at the specified index
		@param index index of the Group to make active
	*/
	void setActiveGroup(int index);

private:

	/**
		Adds the given group to the collection
		@param group the Group to add.
	*/
	void addGroup(Group *group, bool signal = true);
	std::vector<Group*> groups;
	Group *activeGroup;

};

#endif