

#ifndef HSCORE_DATABASE_H
#define HSCORE_DATABASE_H

#define I_HSCORE_DATABASE "hscore_database-4"

typedef struct PerPlayerData
{
	int money;
	int moneyType[MONEY_TYPE_COUNT];

	int exp;

	ShipHull * hull[8];

	int loaded; //internal use only
	int shipsLoaded; //internal use only

	int id; //MySQL use
} PerPlayerData;

typedef struct PerArenaData
{
	LinkedList storeList;
	LinkedList categoryList;
} PerArenaData;

#define CB_ITEM_COUNT_CHANGED "itemcount-1"
//NOTE: *entry may be NULL if newCount is 0.
//called with lock held
typedef void (*ItemCountChanged)(Player *p, Item *item, InventoryEntry *entry, int newCount, int oldCount);



#define CB_HS_ITEMRELOAD "hs-itemreload-1"
typedef void (*HSItemReload)(void);



#define CB_SHIPS_LOADED "shipsloaded-1"
typedef void (*ShipsLoaded)(Player *p);

#define CB_SHIP_ADDED "shipadded-1"
typedef void (*ShipAdded)(Player *p, int ship);

typedef struct Ihscoredatabase
{
	INTERFACE_HEAD_DECL

	int (*areShipsLoaded)(Player *p); //returns 1 if player's per-arena ships are loaded, 0 otherwise.
	int (*isLoaded)(Player *p); //returns 1 if player's globals are loaded, 0 otherwise.

	LinkedList * (*getItemList)();
	LinkedList * (*getStoreList)(Arena *arena);
	LinkedList * (*getCategoryList)(Arena *arena);

	void (*lock)();
	void (*unlock)();

	//call whenever you want an item to be written back into SQL
	//a newCount of 0 will delete the item from the database
	void (*updateItem)(Player *p, int ship, Item *item, int newCount, int newData);
	void (*updateItemNoLock)(Player *p, int ship, Item *item, int newCount, int newData);
	void (*updateInventoryNoLock)(Player *p, int ship, InventoryEntry *entry, int newCount, int newData);

	void (*addShip)(Player *p, int ship);
	void (*removeShip)(Player *p, int ship); //NOTE: will destroy all items on the ship

	PerPlayerData * (*getPerPlayerData)(Player *p); //should only be used by hscore modules
} Ihscoredatabase;

#endif //HSCORE_DATABASE_H
