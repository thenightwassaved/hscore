#ifndef HSCORE_DATABASE_H
#define HSCORE_DATABASE_H

#define I_HSCORE_DATABASE "hscore_db-1"

typedef struct PlayerData
{
	int money;
	int moneyType[MONEY_TYPE_COUNT];

	int exp;

	ShipHull hull[8];

	int id; //MySQL use
} HSPlayerData;

typedef struct ArenaData
{
	LinkedList *arenaList;
	LinkedList *categoryList;
} HSPlayerData;

typedef struct Ihscore_database
{
	INTERFACE_HEAD_DECL

	int (*isLoaded)(Player *p); //returns 1 if loaded, 0 otherwise.

	LinkedList * (*getItemList)();
	LinkedList * (*getStoreList)(Arena *arena);
	LinkedList * (*getCategoryList)(Arena *arena);

	void (*addShip)(Player *p, int ship, linkedList *itemList);
	void (*removeShip)(Player *p, int ship); //NOTE: will destroy all items on the ship

	PlayerData * (*getPlayerData)(Player *p); //should only be used by hscore modules
} Ihscore_database;

#endif //HSCORE_DATABASE_H