#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"

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

}

local int canBuyItem(Player *p, Item *item)
{

}

local int canSellItem(Player *p, Item *item)
{

}

local void buyingItem(Player *p, Item *item)
{

}

local void sellingItem(Player *p, Item *item)
{

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
