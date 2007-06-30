#ifndef HSCORE_TYPES_H
#define HSCORE_TYPES_H

typedef enum EventAction
{
	//No action
	ACTION_NO_ACTION = 0,

	//removes event->data amount of the items from the ship's inventory
	ACTION_REMOVE_ITEM,

	//removes event->data amount of the item's ammo type from inventory
	ACTION_REMOVE_ITEM_AMMO,

	//sends prize #event->data to the player
	ACTION_PRIZE,

	//sets the item's inventory data to event->data. This is useful with
	//the "init" event.
	ACTION_SET_INVENTORY_DATA,

	//does a ++ on inventory data.
	ACTION_INCREMENT_INVENTORY_DATA,

	//does a -- on inventory data. A "datazero" event may be generated as a result.
	ACTION_DECREMENT_INVENTORY_DATA,

	//Specs the player.
	ACTION_SPEC,

	//sends a shipreset packet and reprizes all items (antideath, really)
	ACTION_SHIP_RESET,

	//calls a callback passing an eventid of event->data.
	ACTION_CALLBACK
} EventAction;

typedef struct Event
{
	char event[17]; //something like "death" or "datazero"
	EventAction action;

	int data; //action dependent

	char message[201]; //if == to "" then nothing will be sent.
} Event;

typedef struct Property
{
	char name[17];
	int value;
} Property;

typedef struct ItemType
{
	char name[33];
	int max; //maximum total of this item type on a ship before ?buy denies purchace

	int id; //MySQL use only
} ItemType;

typedef struct ItemTypeEntry
{
	ItemType *itemType;
	int delta;
} ItemTypeEntry;

typedef struct Item
{
	char name[17];
	char shortDesc[33]; //displayed inline in the ?buy menu
	char longDesc[201]; //displayed as part of ?iteminfo
	int buyPrice;
	int sellPrice;

	int expRequired; //requirement to own

	int shipsAllowed; //bit positions represent each ship. bit 0 = warbird.

	LinkedList propertyList;

	LinkedList eventList;

	LinkedList itemTypeEntries;

	int max;

	//if changes to this item should be delayed until a complete save (like on exit).
	//This is a necessity when dealing with ammo. We don't want to update MySQL every
	//time a gun is fired.
	int delayStatusWrite;

	struct Item *ammo; //can be NULL, only for use by events.
	int ammoID; //used for post processing ONLY
	
	LinkedList ammoUsers; //a list of items that use THIS item as ammo
	
	//if this item needs ammo present to provide properties or events
	int needsAmmo;
	
	//the minimum amount of ammo that must be present for an item to provide props&events
	int minAmmo;
	
	//if any of this item's properties change the clientset packet
	int affectsSets;

	//if the settings packet should be resent immediatly following an item update
	int resendSets;

	int id; //MySQL use only
} Item;

typedef struct InventoryEntry
{
	Item *item;
	int count;

	int data; //persistent int for use by the event system.
} InventoryEntry;

typedef struct ShipHull
{
	LinkedList inventoryEntryList;

	//NOTE: no need for ship #, as it's defined by the array index (when loaded by hscore_database)

	//if we compile a hashmap of properties, it can go in here.
	HashTable *propertySums;

	int id; //MySQL use only
} ShipHull;

typedef struct Category
{
	char name[33]; //displayed on ?buy
	char description[65]; //displayed inline on the ?buy menu

	LinkedList itemList; //a list of member items that are displayed

	int hidden; //1 means a hidden category, 0 means visible

	int id; //mysql use
} Category;

typedef struct Store
{
	char name[33]; //displayed in ?buysell location errors
	char description[201]; //displayed in ?storeinfo
	char region[17]; //region that defines the store

	LinkedList itemList; //a list of items that can be purchaced here

	int id; //mysql use
} Store;

#endif //HSCORE_TYPES_H
