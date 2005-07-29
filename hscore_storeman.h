#ifndef HSCORE_STOREMAN_H
#define HSCORE_STOREMAN_H

#define I_HSCORE_STOREMAN "hscore_storeman-1"

typedef struct Ihscorestoreman
{
	INTERFACE_HEAD_DECL

	int (*canBuyItem)(Player *p, Item *item); //1 if they can buy the item from their current location
	int (*canSellItem)(Player *p, Item *item); //1 if they can sell the item from their current location

	void (*buyingItem)(Player *p, Item *item); //called when the player buys an item
	void (*sellingItem)(Player *p, Item *item); //called when the player sells an item
} Ihscorestoreman;

#endif //HSCORE_STOREMAN_H
