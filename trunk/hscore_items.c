#include <string.h>
#include <stdlib.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"
#include "hscore_shipnames.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Icmdman *cmd;
local Iplayerdata *pd;
local Igame *game;
local Ihscoredatabase *database;

//interface prototypes
local int getItemCount(Player *p, Item *item, int ship);
local void addItem(Player *p, Item *item, int ship, int amount);
local Item * getItemByName(const char *name, Arena *arena);
local int getPropertySum(Player *p, int ship, const char *prop);
local void triggerEvent(Player *p, int ship, const char *event);
local void triggerEventOnItem(Player *p, Item *item, int ship, const char *event);
local int getFreeItemTypeSpots(Player *p, ItemType *type, int ship);


local helptext_t itemInfoHelp =
"Targets: none\n"
"Args: <item>\n"
"Shows you information about the specified item.\n";

local void itemInfoCommand(const char *command, const char *params, Player *p, const Target *target)
{
	if (*params == '\0')
	{
		chat->SendMessage(p, "Please use the ?buy menu to look up items.");
		return;
	}

	Item *item = getItemByName(params, p->arena);

	if (item == NULL)
	{
		chat->SendMessage(p, "No item named %s in this arena", params);
		return;
	}

	//FIXME: Write a better item info
	chat->SendMessage(p, "Item %s: %s", item->name, item->longDesc);
}

local helptext_t grantItemHelp =
"Targets: player or freq or arena\n"
"Args: [-f] [-q] [-c <amount>] [-s <ship #>] <item>\n"
"Adds the specified item to the inventory of the targeted players.\n"
"If {-s} is not specified, the item is added to the player's current ship.\n"
"Will only effect speccers if {-s} is used. The added amount defaults to 1.\n"
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void grantItemCommand(const char *command, const char *params, Player *p, const Target *target)
{
	int force = 0;
	int quiet = 0;
	int count = 1;
	int ship = 0;
	const char *itemName;

	char *next; //for strtol

	while (params != NULL) //get the flags
	{
		if (*params == '-')
		{
			params++;
			if (*params == '\0')
			{
				chat->SendMessage(p, "Grantitem: invalid usage.");
				return;
			}

			if (*params == 'f')
			{
				force = 1;

				params = strchr(params, ' ');
				if (params) //check so that params can still == NULL
				{
					params++; //we want *after* the space
				}
				else
				{
					chat->SendMessage(p, "Grantitem: invalid usage.");
					return;
				}
			}
			if (*params == 'q')
			{
				quiet = 1;

				params = strchr(params, ' ');
				if (params) //check so that params can still == NULL
				{
					params++; //we want *after* the space
				}
				else
				{
					chat->SendMessage(p, "Grantitem: invalid usage.");
					return;
				}
			}
			if (*params == 'c')
			{
				params = strchr(params, ' ');
				if (params) //check so that params can still == NULL
				{
					params++; //we want *after* the space
				}
				else
				{
					chat->SendMessage(p, "Grantitem: invalid usage.");
					return;
				}

				count = strtol(params, &next, 0);

				if (next == params)
				{
					chat->SendMessage(p, "Grantitem: bad count.");
					return;
				}

				params = next;
			}
			if (*params == 's')
			{
				params = strchr(params, ' ');
				if (params) //check so that params can still == NULL
				{
					params++; //we want *after* the space
				}
				else
				{
					chat->SendMessage(p, "Grantitem: invalid usage.");
					return;
				}

				ship = strtol(params, &next, 0);

				if (next == params)
				{
					chat->SendMessage(p, "Grantitem: bad ship.");
					return;
				}

				params = next;
			}
		}
		else if (*params == ' ')
		{
			params++;
		}
		else if (*params == '\0')
		{
			chat->SendMessage(p, "Grantitem: invalid usage.");
			return;
		}
		else
		{
			itemName = params;
			break;
		}
	}

	//finished parsing

	//set ship from 0-7
	ship--;

	//verify ship
	if (ship < -1 || 7 < ship) //-1 is special, means current ship.
	{
		chat->SendMessage(p, "Grantitem: Ship out of range. Please choose a ship from 1 to 8.");
		return;
	}

	//verify count
	if (count == 0)
	{
		chat->SendMessage(p, "Grantitem: Bad count.");
		return;
	}

	//verify item
	Item *item = getItemByName(itemName, p->arena);
	if (item == NULL)
	{
		chat->SendMessage(p, "Grantitem: No item %s in this arena.", itemName);
		return;
	}

	if (target->type == T_PLAYER) //private command
	{
		Player *t = target->u.p;

		if (!force)
		{
			if (database->areShipsLoaded(t))
			{
				if (ship == -1)
				{
					if (t->p_ship != SHIP_SPEC)
					{
						ship = t->p_ship;
					}
					else
					{
						chat->SendMessage(p, "Cannot grant item: player %s is in spec.", t->name);
						return;
					}
				}

				addItem(t, item, ship, count);

				if (!quiet)
				{
					chat->SendMessage(t, "You were granted %i of item %s on your %s.", count, item->name, shipNames[ship]);
					chat->SendMessage(p, "You granted %i of item %s to player %s", count, item->name, t->name);
				}
				else
				{
					chat->SendMessage(p, "You quietly granted %i of item %s to player %s", count, item->name, t->name);
				}
			}
			else
			{
				chat->SendMessage(p, "Player %s has no ships loaded.", t->name);
			}
		}
		else
		{
			chat->SendMessage(p, "Whoa there, bud. The -f is only for arena and freq messages.");
		}
	}
	else //not private
	{
		if (force)
		{
			LinkedList set = LL_INITIALIZER;
			Link *link;
			pd->TargetToSet(target, &set);

			for (link = LLGetHead(&set); link; link = link->next)
			{
				Player *t = link->data;

				if (database->areShipsLoaded(t))
				{
					if (ship == -1)
					{
						if (t->p_ship != SHIP_SPEC)
						{
							addItem(t, item, t->p_ship, count);
							if (!quiet)
							{
								chat->SendMessage(t, "You were granted %i of item %s on your %s.", count, item->name, shipNames[t->p_ship]);
							}
						}
						else
						{
							chat->SendMessage(p, "Player %s is in spec.", t->name);
						}
					}
					else
					{
						addItem(t, item, ship, count);
						if (!quiet)
						{
							chat->SendMessage(t, "You were granted %i of item %s on your %s.", count, item->name, shipNames[ship]);
						}
					}
				}
				else
				{
					chat->SendMessage(p, "Player %s has no ships loaded.", t->name);
				}
			}

			int playerCount = LLCount(&set);

			LLEmpty(&set);

			chat->SendMessage(p, "You gave %i of item %s to %i players.", count, item->name, playerCount);
		}
		else
		{
			chat->SendMessage(p, "For typo safety, the -f must be specified for arena and freq targets.");
		}
	}
}

local void doEvent(Player *p, InventoryEntry *entry, Event *event) //called with lock held
{
	Item *item = entry->item;

	if (event->action == ACTION_REMOVE_ITEM) //removes event->data amount of the items from the ship's inventory
	{
		//FIXME
	}
	else if (event->action == ACTION_REMOVE_ITEM_AMMO) //removes event->data amount of the item's ammo type from inventory
	{
		//FIXME
	}
	else if (event->action == ACTION_PRIZE) //sends prize #event->data to the player
	{
		int prize = event->data && 0x1F;
		int count = event->data >> 5;

		if (count == 0)
		{
			count = 1;
		}

		Target t;
		t.type = T_PLAYER;
		t.u.p = p;

		game->GivePrize(&t, prize, count);
	}
	else if (event->action == ACTION_SET_INVENTORY_DATA) //sets the item's inventory data to event->data.
	{
		entry->data = event->data;
	}
	else if (event->action == 	ACTION_INCREMENT_INVENTORY_DATA) //does a ++ on inventory data.
	{
		entry->data++;
	}
	else if (event->action == ACTION_DECREMENT_INVENTORY_DATA) //does a -- on inventory data. A "datazero" event may be generated as a result.
	{
		entry->data--;
	}
	else if (event->action == ACTION_SPEC) //Specs the player.
	{
		game->SetFreqAndShip(p, SHIP_SPEC, p->arena->specfreq);
	}
	else if (event->action == ACTION_SHIP_RESET) //sends a shipreset packet and reprizes all items (antideath, really)
	{
		Target t;
		t.type = T_PLAYER;
		t.u.p = p;

		game->ShipReset(&t);
		//FIXME
	}

}

local int getItemCount(Player *p, Item *item, int ship)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);

	if (item == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get item count of NULL item");
		return 0;
	}

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get item count on a player with unloaded ships");
		return 0;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get item count on ship %i", ship);
		return 0;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get item count on unowned ship %i", ship);
		return 0;
	}

	Link *link;
	LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

	database->lock();
	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;

		if (entry->item == item)
		{
			database->unlock();
			return entry->count;
		}
	}
	database->unlock();

	return 0; //don't have it
}

local void addItem(Player *p, Item *item, int ship, int amount)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);

	if (item == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add a NULL item.");
		return;
	}

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add item to a player with unloaded ships");
		return;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add item to ship %i", ship);
		return;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add item to unowned ship %i", ship);
		return;
	}

	Link *link;
	LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

	int data = 0;
	int count = 0;

	database->lock();
	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;

		if (entry->item == item)
		{
			data = entry->data;
			count = entry->count;
			break;
		}
	}
	database->unlock();

	count += amount;

	if (count < 0)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to set item %s count to %i", item->name, count);
		count = 0; //no negative counts make sense
	}

	database->updateItem(p, ship, item, count, data);
}

local Item * getItemByName(const char *name, Arena *arena)
{
	LinkedList *categoryList = database->getCategoryList(arena);
	Link *catLink;

	//to deal with the fact that an item name is only unique per arena,
	//we scan the items in the categories rather than the item list
	database->lock();
	for (catLink = LLGetHead(categoryList); catLink; catLink = catLink->next)
	{
		Category *category = catLink->data;
		Link *itemLink;

		for (itemLink = LLGetHead(&category->itemList); itemLink; itemLink = itemLink->next)
		{
			Item *item = itemLink->data;

			if (strcasecmp(item->name, name) == 0)
			{
				database->unlock();
				return item;
			}
		}
	}
	database->unlock();

	return NULL;
}

local int getPropertySum(Player *p, int ship, const char *propString)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);

	if (propString == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get props for NULL string.");
		return 0;
	}

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get props from a player with unloaded ships");
		return 0;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get props on ship %i", ship);
		return 0;
	}

	if (playerData->hull[ship] == NULL)
	{
		//not unusual or dangerous
		//lm->LogP(L_ERROR, "hscore_items", p, "asked to get props from unowned ship %i", ship);
		return 0;
	}

	Link *link;
	LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

	int count = 0;

	database->lock();
	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;
		Item *item = entry->item;

		if (item->ammo != NULL)
		{
			database->unlock(); //so we can call getItemCount
			int itemCount = getItemCount(p, item->ammo, ship); //POSSIBLE FIXME?
			database->lock(); //relock

			if (itemCount <= 0)
			{
				continue; //out of ammo, ignore the item.
			}
		}

		Link *propLink;
		for (propLink = LLGetHead(&item->propertyList); propLink; propLink = propLink->next)
		{
			Property *prop = propLink->data;

			if (strcmp(prop->name, propString) == 0)
			{
				count += prop->value * entry->count;
				break;
			}
		}
	}
	database->unlock();

	return count;
}

local void triggerEvent(Player *p, int ship, const char *eventName)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);

	if (eventName == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to trigger event with NULL string.");
		return;
	}

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to trigger event on a player with unloaded ships");
		return;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to trigger event on ship %i", ship);
		return;
	}

	if (playerData->hull[ship] == NULL)
	{
		//not unusual or dangerous
		return;
	}

	Link *link;
	LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

	database->lock();
	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;
		Item *item = entry->item;

		Link *eventLink;
		for (eventLink = LLGetHead(&item->eventList); eventLink; eventLink = eventLink->next)
		{
			Event *event = eventLink->data;

			if (strcmp(event->event, eventName) == 0)
			{
				doEvent(p, entry, event);
				break;
			}
		}
	}
	database->unlock();
}

local void triggerEventOnItem(Player *p, Item *triggerItem, int ship, const char *eventName)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);

	if (eventName == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to trigger item event with NULL string.");
		return;
	}

	if (triggerItem == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to trigger item event with NULL item.");
		return;
	}

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to trigger item event on a player with unloaded ships");
		return;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to trigger item event on ship %i", ship);
		return;
	}

	if (playerData->hull[ship] == NULL)
	{
		//not unusual or dangerous
		return;
	}

	Link *link;
	LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

	database->lock();
	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;
		Item *item = entry->item;
		if (item == triggerItem)
		{
			Link *eventLink;
			for (eventLink = LLGetHead(&item->eventList); eventLink; eventLink = eventLink->next)
			{
				Event *event = eventLink->data;

				if (strcmp(event->event, eventName) == 0)
				{
					doEvent(p, entry, event);
					break;
				}
			}
		}
	}
	database->unlock();
}

local int getFreeItemTypeSpots(Player *p, ItemType *type, int ship)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);

	if (type == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get slots for NULL item type.");
		return 0;
	}

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get slots from a player with unloaded ships");
		return 0;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get item type on ship %i", ship);
		return 0;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get item type from unowned ship %i", ship);
		return 0;
	}

	Link *link;
	LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

	int count = type->max;

	database->lock();
	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;
		Item *item = entry->item;

		if (item->type1 == type)
		{
			count -= entry->count * item->typeDelta1;
		}
		if (item->type2 == type)
		{
			count -= entry->count * item->typeDelta2;
		}
	}
	database->unlock();

	return count;
}

local Ihscoreitems interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_ITEMS, "hscore_items")
	getItemCount, addItem, getItemByName, getPropertySum,
	triggerEvent, triggerEventOnItem, getFreeItemTypeSpots,
};

EXPORT int MM_hscore_items(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		game = mm->GetInterface(I_GAME, ALLARENAS);
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);

		if (!lm || !chat || !cmd || !pd || !game || !database)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(pd);
			mm->ReleaseInterface(game);
			mm->ReleaseInterface(database);

			return MM_FAIL;
		}

		mm->RegInterface(&interface, ALLARENAS);

		cmd->AddCommand("iteminfo", itemInfoCommand, ALLARENAS, itemInfoHelp);
		cmd->AddCommand("grantitem", grantItemCommand, ALLARENAS, grantItemHelp);

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

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(game);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	return MM_FAIL;
}
