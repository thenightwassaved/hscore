#ifndef HSCORE_DATABASE_H
#define HSCORE_DATABASE_H

#define I_HSCORE_DATABASE "hscore_database-1"

typedef struct PerPlayerData
{
	int money;
	int moneyType[MONEY_TYPE_COUNT];

	int exp;

	ShipHull hull[8];

	int loaded; //internal use only

	int id; //MySQL use
} PlayerData;

typedef struct PerArenaData
{
	LinkedList arenaList;
	LinkedList categoryList;
} PlayerData;

typedef struct Ihscoredatabase
{
	INTERFACE_HEAD_DECL

	int (*areShipsLoaded)(Player *p); //returns 1 if player's per-arena ships are loaded, 0 otherwise.
	int (*isLoaded)(Player *p); //returns 1 if player's globals are loaded, 0 otherwise.

	LinkedList * (*getItemList)();
	LinkedList * (*getStoreList)(Arena *arena);
	LinkedList * (*getCategoryList)(Arena *arena);

	void (*addShip)(Player *p, int ship, linkedList *itemList);
	void (*removeShip)(Player *p, int ship); //NOTE: will destroy all items on the ship

	PerPlayerData * (*getPlayerData)(Player *p); //should only be used by hscore modules
} Ihscoredatabase;

#endif //HSCORE_DATABASE_H
