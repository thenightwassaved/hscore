#ifndef HSCORE_ITEMS_H
#define HSCORE_ITEMS_H

#define I_HSCORE_ITEMS "hscore_items-2"

typedef struct Ihscoreitems
{
	INTERFACE_HEAD_DECL

	int (*getItemCount)(Player *p, Item *item, int ship);
	void (*addItem)(Player *p, Item *item, int ship, int amount);
	void (*removeItem)(Player *p, Item *item, int ship, int amount);

	Item * (*getItemByName)(const char *name, Arena *arena);

	int (*getPropertySum)(Player *p, int ship, const char *prop);

	void (*triggerEvent)(Player *p, int ship, const char *event);

	int (*getFreeItemTypeSpots)(Player *p, ItemType *type, int ship);

	//more required, i'm sure
} Ihscoreitems;

#endif //HSCORE_ITEMS_H
