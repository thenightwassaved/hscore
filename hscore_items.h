#ifndef HSCORE_ITEMS_H
#define HSCORE_ITEMS_H

#define I_HSCORE_ITEMS "hscore_items-7"

//callback
#define CB_EVENT_ACTION "eventaction"
#define CB_TRIGGER_EVENT "triggerevent"

//callback function prototype
typedef void (*eventActionFunction)(Player *p, int eventID);
typedef void (*triggerEventFunction)(Player *p, Item *triggerItem, int ship, const char *eventName);

typedef struct Ihscoreitems
{
	INTERFACE_HEAD_DECL

	int (*getItemCount)(Player *p, Item *item, int ship);
	int (*addItem)(Player *p, Item *item, int ship, int amount);

	Item * (*getItemByName)(const char *name, Arena *arena);
	Item * (*getItemByPartialName)(const char *name, Arena *arena);	

	int (*getPropertySum)(Player *p, int ship, const char *prop); //properties ARE case sensitive

	void (*triggerEvent)(Player *p, int ship, const char *event);
	void (*triggerEventOnItem)(Player *p, Item *item, int ship, const char *event);

	int (*getFreeItemTypeSpots)(Player *p, ItemType *type, int ship);

	int (*hasItemsLeftOnShip)(Player *p, int ship);

	void (*recaclulateEntireCache)(Player *p, int ship); //call with lock held
} Ihscoreitems;

#endif //HSCORE_ITEMS_H
