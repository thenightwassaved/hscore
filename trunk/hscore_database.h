#ifndef HSCORE_DATABASE_H
#define HSCORE_DATABASE_H

#define I_HSCORE_DB "hscore_db-1"



typedef struct Ihscoredb
{
	INTERFACE_HEAD_DECL

	int (*isLoaded)(Player *p); //returns 1 if loaded, 0 otherwise.

	LinkedList * (*getItemList)();
	LinkedList * (*getStoreList)(Arena *arena);
	LinkedList * (*getCategoryList)(Arena *arena);

	void (*addShip)(Player *p, int ship, linkedList *itemList);
	void (*removeShip)(Player *p, int ship); //NOTE: will destroy all items on the ship

} Ihscoredb;

#endif //HSCORE_DATABASE_H