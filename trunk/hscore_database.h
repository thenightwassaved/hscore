#ifndef HSCORE_DATABASE_H
#define HSCORE_DATABASE_H

#define I_HSCORE_DATABASE "hscore_database-3"

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

	void (*addShip)(Player *p, int ship);
	void (*removeShip)(Player *p, int ship); //NOTE: will destroy all items on the ship

	PerPlayerData * (*getPerPlayerData)(Player *p); //should only be used by hscore modules
} Ihscoredatabase;

#endif //HSCORE_DATABASE_H
