#include <string.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"
#include "hscore_storeman.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Icmdman *cmd;
local Ihscoredatabase *database;

//interface prototypes
local int canBuyItem(Player *p, Item *item);
local int canSellItem(Player *p, Item *item);
local void buyingItem(Player *p, Item *item);
local void sellingItem(Player *p, Item *item);

local helptext_t storeInfoHelp =
"Targets: none\n"
"Args: none or <store>\n"
"If a store is specified, this shows you information about it.\n"
"Otherwise, this command gives a list of stores in the arena.\n";

local void storeInfoCommand(const char *command, const char *params, Player *p, const Target *target)
{
	Link *link;
	if (strcasecmp(params, "") == 0)
	{
		//print store list

		chat->SendMessage(p, "+----------------------------------+");
		chat->SendMessage(p, "| Stores                           |");
		chat->SendMessage(p, "+----------------------------------+");

		database->lock();
		for (link = LLGetHead(database->getStoreList(p->arena)); link; link = link->next)
		{
			Store *store = link->data;

			chat->SendMessage(p, "| %-32s |", store->name);
		}
		database->unlock();

		chat->SendMessage(p, "+----------------------------------+");
	}
	else
	{
		database->lock();
		for (link = LLGetHead(database->getStoreList(p->arena)); link; link = link->next)
		{
			Store *store = link->data;

			if (strcasecmp(params, store->name) == 0)
			{
				chat->SendMessage(p, "+----------------------------------+");
				chat->SendMessage(p, "| %-32s |", store->name);
				chat->SendMessage(p, "+----------------------------------+---------------------------------------------------------------+");

				char buf[256];
				const char *temp = NULL;

				while (strsplit(store->description, "ß", buf, 256, &temp))
				{
					chat->SendMessage(p, "| %-96s |", buf);
				}

				chat->SendMessage(p, "+--------------------------------------------------------------------------------------------------+");

				database->unlock();
				return;
			}
		}
		database->unlock();

		//didn't find it
		chat->SendMessage(p, "No stored named %s in this arena.", params);
	}
}

local int canBuyItem(Player *p, Item *item)
{
	//FIXME
	return 0;
}

local int canSellItem(Player *p, Item *item)
{
	//FIXME
	return 0;
}

local void buyingItem(Player *p, Item *item)
{
	//nothing to do for the standard module
}

local void sellingItem(Player *p, Item *item)
{
	//nothing to do for the standard module
}

local Ihscorestoreman interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_STOREMAN, "hscore_storeman")
	canBuyItem, canSellItem, buyingItem, sellingItem,
};

EXPORT int MM_hscore_storeman(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);

		if (!lm || !chat || !cmd || !database)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(database);

			return MM_FAIL;
		}

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		mm->RegInterface(&interface, ALLARENAS);

		cmd->AddCommand("storeinfo", storeInfoCommand, ALLARENAS, storeInfoHelp);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		if (mm->UnregInterface(&interface, ALLARENAS))
		{
			return MM_FAIL;
		}

		cmd->RemoveCommand("storeinfo", storeInfoCommand, ALLARENAS);

		return MM_OK;
	}
	return MM_FAIL;
}
