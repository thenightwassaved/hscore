#ifndef HSCORE_TYPES_H
#define HSCORE_TYPES_H

typedef struct Event
{
	char event[16];
	Action action;

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
	int id; //MySQL use only

	char name[32];
	int max;
} ItemType;

typedef struct Item
{
	int id; //MySQL use only

	char name[16];
	char shortDesc[32];
	char longDesc[256];
	int buyPrice;
	int sellPrice;

	int expRequired;

	LinkedList *propertyList;

	ItemType *type1, type2;
} Item;

typedef struct InventoryEntry
{

} InventoryEntry;

typedef struct ShipHull
{
	int id; //MySQL use only

	//NOTE: no need for ship #, as it's defined by the array index (when loaded by hscore_database)

	LinkedList *inventoryEntryList

	//if we compile a hashmap of properties, it can go in here.
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
