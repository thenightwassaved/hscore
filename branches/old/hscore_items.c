#include <string.h>
#include <stdlib.h>

#include "../asss.h"
#include "hscore_database.h"
#include "hscore_items.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihsdatabase *db;

local int HasItem(Player *p, Item *item);
local void AddItem(Player *p, Item *item, int amount);
local void RemoveItem(Player *p, Item *item, int amount);
local ShipItem* GetShipItem(Player *p, Item *item);
local void ChangeItem(Player *p, Item *item, int amount, int data);
local int GetPropertySum(Player *p, const char *prop);
local int GetItemTypeSum(Player *p, int type);


local int HasItem(Player *p, Item *item)
{
	return 0;
}

local void AddItem(Player *p, Item *item, int amount)
{

}

local void RemoveItem(Player *p, Item *item, int amount)
{

}

local ShipItem* GetShipItem(Player *p, Item *item)
{
	return NULL;
}

local void ChangeItem(Player *p, Item *item, int amount, int data)
{

}

local int GetPropertySum(Player *p, const char *prop)
{
	return 0;
}

local int GetItemTypeSum(Player *p, int type)
{
	return 0;
}

local Ihsitems myint =
{
	INTERFACE_HEAD_INIT(I_HS_ITEMS, "hscore_items")
	HasItem, AddItem, RemoveItem, GetShipItem,
	ChangeItem, GetPropertySum, GetItemTypeSum,
};

EXPORT int MM_hscore_items(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		db = mm->GetInterface(I_HS_DATABASE, ALLARENAS);

		if (!lm || !chat || !cfg || !cmd || !db)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(db);
			return MM_FAIL;
		}

		mm->RegInterface(&myint, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&myint, ALLARENAS))
			return MM_FAIL;

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(db);

		return MM_OK;
	}
	return MM_FAIL;
}
