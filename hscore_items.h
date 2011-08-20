#ifndef HSCORE_ITEMS_H
#define HSCORE_ITEMS_H

/* pyinclude: hscore/hscore_types.h */
/* pyinclude: hscore/hscore_items.h */
/* pytype: opaque, Item *, item */

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
	/* pyint: use */

	int (*getItemCount)(Player *p, Item *item, int ship);
	/* pyint: player, item, int -> int */
	int (*getItemCountNoLock)(Player *p, Item *item, int ship);
	int (*addItem)(Player *p, Item *item, int ship, int amount);
	/* pyint: player, item, int, int -> int */
	int (*addItemCheckLimits)(Player *p, Item *item, int ship, int amount);
	/* pyint: player, item, int, int -> int */
	int (*addItemCheckLimitsNoLock)(Player *p, Item *item, int ship, int amount);

	Item * (*getItemByName)(const char *name, Arena *arena);
	/* pyint: string, arena -> item */
	Item * (*getItemByPartialName)(const char *name, Arena *arena);
	/* pyint: string, arena -> item */

	int (*getPropertySum)(Player *p, int ship, const char *prop, int def); //properties ARE case sensitive
	/* pyint: player, int, string, int -> int */
	int (*getPropertySumNoLock)(Player *p, int ship, const char *prop, int def);

	void (*triggerEvent)(Player *p, int ship, const char *event);
	/* pyint: player, int, string -> void */
	void (*triggerEventOnItem)(Player *p, Item *item, int ship, const char *event);
	/* pyint: player, item, int, string -> void */

	int (*getFreeItemTypeSpotsNoLock)(Player *p, ItemType *type, int ship);

	int (*hasItemsLeftOnShip)(Player *p, int ship);
	/* pyint: player, int -> int */

	void (*recaclulateEntireCache)(Player *p, int ship); //call with lock held
} Ihscoreitems;

#define A_HSCORE_ITEMS "hscore_items-10"
typedef struct Ahscoreitems
{
	ADVISER_HEAD_DECL
	
	/*
	Called after regular checks. Return 1 to allow the
	target to be granted the item(s). Return 0 to deny.
	Send an appropriate message to the player *p using
	the grantitem command if you deny it. Note that if
	the granting player uses the 'ignore' flag (-i),
	this function is never called.
	*/
	
	int (*CanGrantItem)(Player *p /*Granting player*/, Player *t /*Target player who will receive the item*/, Item *item, int ship, int count);
} Ahscoreitems;

#endif //HSCORE_ITEMS_H
