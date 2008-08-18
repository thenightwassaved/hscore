#ifndef HSCORE_ITEMS_H
#define HSCORE_ITEMS_H

#define I_HSCORE_ITEMS "hscore_items-10"

//callback
#define CB_EVENT_ACTION "eventaction"
#define CB_TRIGGER_EVENT "triggerevent"
#define CB_AMMO_ADDED "ammoaddded"
#define CB_AMMO_REMOVED "ammoremoved"
#define CB_ITEMS_CHANGED "hscb_items_changed"

//callback function prototype
typedef void (*eventActionFunction)(Player *p, int eventID);
typedef void (*triggerEventFunction)(Player *p, Item *triggerItem, int ship, const char *eventName);
typedef void (*ammoAddedFunction)(Player *p, int ship, Item *ammoUser); //warnings: cache is out of sync, and lock is held
typedef void (*ammoRemovedFunction)(Player *p, int ship, Item *ammoUser); //warnings: cache is out of sync, and lock is held
typedef void (*ItemsChangedFunction)(Player *p, int ship);


typedef struct Ihscoreitems
{
	INTERFACE_HEAD_DECL

	int (*getItemCount)(Player *p, Item *item, int ship);
	int (*getItemCountNoLock)(Player *p, Item *item, int ship);
	int (*addItem)(Player *p, Item *item, int ship, int amount);
	int (*addItemCheckLimits)(Player *p, Item *item, int ship, int amount);
	int (*addItemCheckLimitsNoLock)(Player *p, Item *item, int ship, int amount);

	Item * (*getItemByName)(const char *name, Arena *arena);
	Item * (*getItemByPartialName)(const char *name, Arena *arena);

	int (*getPropertySum)(Player *p, int ship, const char *prop, int def); //properties ARE case sensitive
	int (*getPropertySumNoLock)(Player *p, int ship, const char *prop, int def);

	void (*triggerEvent)(Player *p, int ship, const char *event);
	void (*triggerEventOnItem)(Player *p, Item *item, int ship, const char *event);

	int (*getFreeItemTypeSpotsNoLock)(Player *p, ItemType *type, int ship);

	int (*hasItemsLeftOnShip)(Player *p, int ship);

	void (*recaclulateEntireCache)(Player *p, int ship); //call with lock held
} Ihscoreitems;

#endif //HSCORE_ITEMS_H
