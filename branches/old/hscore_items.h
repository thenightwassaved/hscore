#ifndef HS_ITEMS_H
#define HS_ITEMS_H

#define I_HS_ITEMS "hs_items-1"


typedef struct Ihsitems
{
	INTERFACE_HEAD_DECL

	int (*HasItem)(Player *p, Item *item);
	void (*AddItem)(Player *p, Item *item, int amount);
	void (*RemoveItem)(Player *p, Item *item, int amount);

	ShipItem * (*GetShipItem)(Player *p, Item *item); /* FIXME: need better functions */
	void (*ChangeItem)(Player *p, Item *item, int amount, int data);

	int (*GetPropertySum)(Player *p, const char *prop);

	int (*GetItemTypeSum)(Player *p, int type);

} Ihsitems;



#endif /* HS_ITEMS_H */
