#ifndef HSCORE_DATABASE_H
#define HSCORE_DATABASE_H

#define I_HSCORE_DB "hscore_db-1"

typedef struct HSPlayerData
{
	int money;
	int moneyType[MONEY_TYPE_COUNT];

	int exp;

	ShipHull hull[8];

	int id; //MySQL use
} HSPlayerData;

typedef struct HSArenaData
{
	LinkedList *arenaList;
	LinkedList *categoryList;
} HSPlayerData;

typedef struct Ihscoredb
{
	INTERFACE_HEAD_DECL

	int (*isLoaded)(Player *p); //returns 1 if loaded, 0 otherwise.

	LinkedList * (*getItemList)();
	LinkedList * (*getStoreList)(Arena *arena);
	LinkedList * (*getCategoryList)(Arena *arena);

	void (*addShip)(Player *p, int ship, linkedList *itemList);
	void (*removeShip)(Player *p, int ship); //NOTE: will destroy all items on the ship

	HSPlayerData * (*getHSPlayerData)(Player *p); //should only be used by hscore modules
} Ihscoredb;

#endif //HSCORE_DATABASE_H