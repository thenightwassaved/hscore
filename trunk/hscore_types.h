#ifndef HSCORE_TYPES_H
#define HSCORE_TYPES_H

enum
{
	//removes event->data amount of the items from the ship's inventory
	REMOVE_ITEM,

	//removes event->data amount of the item's ammo type from inventory
	REMOVE_ITEM_AMMO,

	//we need a lot more
} EventAction

typedef struct Event
{
	char event[16]; //something like "death"
	EventAction action;

	int data; //action dependent

	char message[128];
} Event;

typedef struct Property
{
	char name[32];
	int value;
} Property;

typedef struct ItemType
{
	char name[32];
	int max;

	int id; //MySQL use only
} ItemType;

typedef struct Item
{
	char name[16];
	char shortDesc[32];
	char longDesc[256];
	int buyPrice;
	int sellPrice;

	int expRequired;

	LinkedList *propertyList;

	LinkedList *eventList;

	ItemType *type1, type2;
	int typeDelta1, typeDelta2;

	Item *ammo; //can be NULL, only for use by events.

	int id; //MySQL use only
} Item;

typedef struct InventoryEntry
{
	Item *item;
	int count;

	int data; //persistant int for use by the event system.
} InventoryEntry;

typedef struct ShipHull
{
	//NOTE: no need for ship #, as it's defined by the array index (when loaded by hscore_database)

	LinkedList *inventoryEntryList

	//if we compile a hashmap of properties, it can go in here.

	int id; //MySQL use only
} ShipHull;

typedef struct Category
{
	char name[20];
	char description[75];

	LinkedList *itemList;
} Category;

typedef struct Store
{
	char name[32];
	char description[255];
	char region[20];

	LinkedList *itemList;
} Store;

#endif //HSCORE_TYPES_H
