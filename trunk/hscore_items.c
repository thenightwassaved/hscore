#include <string.h>

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
local int hasItem(Player *p, Item *item, int ship);
local void addItem(Player *p, Item *item, int ship, int amount);
local void removeItem(Player *p, Item *item, int ship, int amount);
local int getPropertySum(Player *p, int ship, const char *prop);
local void triggerEvent(Player *p, int ship, const char *event);
local int getFreeItemTypeSpots(Player *p, int ship, ItemType *type);

local helptext_t itemInfoHelp =
"Targets: none\n"
"Args: <item>\n"
"Shows you information about the specified item.\n";

local void itemInfoCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

local helptext_t grantItemHelp =
"Targets: player or freq or arena\n"
"Args: [-f] [-c <amount>] [-s <ship #>] <item>\n"
"Adds the specified item to the inventory of the targeted players.\n"
"If {-s} is not specified, the item is added to the player's current ship.\n"
"Will only effect speccers if {-s} is used. The added amount defaults to 1.\n"
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void grantItemCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

local helptext_t removeItemHelp =
"Targets: none\n"
"Args: [-f] [-c <amount>] [-s <ship #>] <item>\n"
"Removes the specified item from the inventory of the targeted players.\n"
"If {-s} is not specified, the item is removed from the player's current ship.\n"
"Will only effect speccers if {-s} is used. The removed amount defaults to 1.\n"
"For typo safety, the {-f} must be specified when removing from more than one player.\n";

local void removeItemCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

local int hasItem(Player *p, Item *item, int ship)
{
	//FIXME
	return 0;
}

local void addItem(Player *p, Item *item, int ship, int amount)
{
	//FIXME
}

local void removeItem(Player *p, Item *item, int ship, int amount)
{
	//FIXME
}

local Item * getItemByName(const char *name, Arena *arena)
{
	LinkedList *categoryList = database->getCategoryList(arena);
	Link *catLink;

	//to deal with the fact that an item name is only unique per arena,
	//we scan the items in the categories rather than the item list
	for (catLink = LLGetHead(categoryList); catLink; catLink = catLink->next)
	{
		Category *category = catLink->data;
		Link *itemLink;

		for (itemLink = LLGetHead(&(category->itemList)); itemLink; itemLink = itemLink->next)
		{
			Item *item = itemLink->data;

			if (strcasecmp(item->name, name) == 0)
			{
				return item;
			}
		}
	}

	return NULL;
}

local int getPropertySum(Player *p, int ship, const char *prop)
{
	//FIXME
	return 0;
}

local void triggerEvent(Player *p, int ship, const char *event)
{
	//FIXME
}

local int getFreeItemTypeSpots(Player *p, int ship, ItemType *type)
{
	//FIXME
	return 0;
}

local Ihscoreitems interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_ITEMS, "hscore_items")
	hasItem, addItem, removeItem, getItemByName, getPropertySum,
	triggerEvent, getFreeItemTypeSpots,
};

EXPORT int MM_hscore_items(int action, Imodman *_mm, Arena *arena)
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

		mm->RegInterface(&interface, ALLARENAS);

		cmd->AddCommand("iteminfo", itemInfoCommand, ALLARENAS, itemInfoHelp);
		cmd->AddCommand("grantitem", grantItemCommand, ALLARENAS, grantItemHelp);
		cmd->AddCommand("removeitem", removeItemCommand, ALLARENAS, removeItemHelp);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&interface, ALLARENAS))
		{
			return MM_FAIL;
		}

		cmd->RemoveCommand("iteminfo", itemInfoCommand, ALLARENAS);
		cmd->RemoveCommand("grantitem", grantItemCommand, ALLARENAS);
		cmd->RemoveCommand("removeitem", removeItemCommand, ALLARENAS);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	return MM_FAIL;
}
