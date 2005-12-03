#include "asss.h"
#include "hscore.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iplayerdata *pd;

//interface prototypes
local int getItemCount(Player *p, Item *item, int ship);
local void addItem(Player *p, Item *item, int ship, int amount);
local Item * getItemByName(const char *name, Arena *arena);
local int getPropertySum(Player *p, int ship, const char *prop);
local void triggerEvent(Player *p, int ship, const char *event);
local void triggerEventOnItem(Player *p, Item *item, int ship, const char *event);
local int getFreeItemTypeSpots(Player *p, ItemType *type, int ship);
local int hasItemsLeftOnShip(Player *p, int ship);


local int getItemCount(Player *p, Item *item, int ship)
{
	chat->SendMessage(p, "getItemCount should not be used by non-core modules!");

	return 0;
}

local void addItem(Player *p, Item *item, int ship, int amount)
{
	chat->SendMessage(p, "addItem should not be used by non-core modules!");
}

local Item * getItemByName(const char *name, Arena *arena)
{
	chat->SendArenaMessage(arena, "Tried to getItemByName(%s)", name);

	return NULL;
}

local int getPropertySum(Player *p, int ship, const char *propString)
{
	if (propString == NULL)
	{
		lm->LogP(L_ERROR, "hscore_itemsstub", p, "asked to get props for NULL string.");
		return 0;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_itemsstub", p, "asked to get props on ship %i", ship);
		return 0;
	}

	//change this value for testing
	return 0;
}

local void triggerEvent(Player *p, int ship, const char *event)
{
	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_itemsstub", p, "asked to get props on ship %i", ship);
	}

	if (event == NULL)
	{
		lm->LogP(L_ERROR, "hscore_itemsstub", p, "asked to trigger event with NULL string.");
		return;
	}

	chat->SendMessage(p, "triggered event %s!", event);
}

local void triggerEventOnItem(Player *p, Item *item, int ship, const char *event)
{
	chat->SendMessage(p, "triggerEventOnItem should not be used by non-core modules!");
}

local int getFreeItemTypeSpots(Player *p, ItemType *type, int ship)
{
	chat->SendMessage(p, "getFreeItemTypeSpots should not be used by non-core modules!");
	return 0;
}

local int hasItemsLeftOnShip(Player *p, int ship)
{
	chat->SendMessage(p, "hasItemsLeftOnShip should not be used by non-core modules!");
	return 0;
}

local Ihscoreitems interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_ITEMS, "hscore_itemsstub")
	getItemCount, addItem, getItemByName, getPropertySum,
	triggerEvent, triggerEventOnItem, getFreeItemTypeSpots, hasItemsLeftOnShip,
};

EXPORT int MM_hscore_itemsstub(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);

		if (!lm || !chat || !pd)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(pd);

			return MM_FAIL;
		}

		mm->RegInterface(&interface, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&interface, ALLARENAS))
		{
			return MM_FAIL;
		}

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(pd);

		return MM_OK;
	}
	return MM_FAIL;
}
