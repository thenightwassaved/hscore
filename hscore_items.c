#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"
#include "hscore_shipnames.h"
#include "hscore_spawner.h"

typedef struct RemovalEntry
{
	int count; //to remove, negative for add
	Item *item;
} RemovalEntry;

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Icmdman *cmd;
local Iplayerdata *pd;
local Igame *game;
local Ihscoredatabase *database;
local Imainloop *ml;
local Iconfig *cfg;

//a few internal functions
local int internalGetItemCount(Player *p, Item *item, int ship);
local void internalTriggerEvent(Player *p, int ship, const char *event);
local void internalTriggerEventOnItem(Player *p, Item *item, int ship, const char *event);

//interface prototypes
local int getItemCount(Player *p, Item *item, int ship);
local int addItem(Player *p, Item *item, int ship, int amount);
local Item * getItemByName(const char *name, Arena *arena);
local Item * getItemByPartialName(const char *name, Arena *arena);
local int getPropertySum(Player *p, int ship, const char *prop);
local void triggerEvent(Player *p, int ship, const char *event);
local void triggerEventOnItem(Player *p, Item *item, int ship, const char *event);
local int getFreeItemTypeSpotsNoLock(Player *p, ItemType *type, int ship);
local int hasItemsLeftOnShip(Player *p, int ship);


local helptext_t itemInfoHelp =
"Targets: none\n"
"Args: <item>\n"
"Shows you information about the specified item.\n";

local void itemInfoCommand(const char *command, const char *params, Player *p, const Target *target)
{
	Item *item;
	char shipMask[] = "12345678";
	int i;
	
	char itemTypes[256];
	int first = 1;
	
	char buf[256];
	const char *temp = NULL;
	Link *link;

	if (*params == '\0')
	{
		chat->SendMessage(p, "Please use the ?buy menu to look up items.");
		return;
	}

	item = getItemByPartialName(params, p->arena);

	if (item == NULL)
	{
		chat->SendMessage(p, "No item named %s in this arena", params);
		return;
	}

	//FIXME: add ammo

	chat->SendMessage(p, "+------------------+");
	chat->SendMessage(p, "| %-16s |", item->name);
	chat->SendMessage(p, "+-----------+------+-----+-------+----------+-----+---------------------------------------------------+");
	chat->SendMessage(p, "| Buy Price | Sell Price | Exp   | Ships    | Max | Item Types                                        |");

	//calculate ship mask
	for (i = 0; i < 8; i++)
	{
		if (!((item->shipsAllowed >> i) & 0x1))
		{
			shipMask[i] = ' ';
		}
	}

	//get item type string
	database->lock();
	itemTypes[0] = '\0';
	for (link = LLGetHead(&item->itemTypeEntries); link; link = link->next)
	{
		ItemTypeEntry *entry = link->data;
		
		if (first == 1)
		{
			first = 0;
			
			if (entry->delta == 1)
			{
				sprintf(buf, "%s", entry->itemType->name);
			}
			else
			{
				sprintf(buf, "%d %s", entry->delta, entry->itemType->name);
			}
		}
		else
		{
			if (entry->delta == 1)
			{
				sprintf(buf, ", %s", entry->itemType->name);
			}
			else
			{
				sprintf(buf, ", %d %s", entry->delta, entry->itemType->name);
			}
		}
		
		strcat(itemTypes, buf);
	}
	database->unlock();

	chat->SendMessage(p, "| $%-8i | $%-9i | %-5i | %s | %-3i | %-49s |", item->buyPrice, item->sellPrice, item->expRequired, shipMask, item->max, itemTypes);
	chat->SendMessage(p, "+-----------+------+-----+-------+----------+-----+---------------------------------------------------+");

	//print description
	while (strsplit(item->longDesc, "�", buf, 256, &temp))
	{
		chat->SendMessage(p, "| %-99s |", buf);
	}

	database->lock();
	link = LLGetHead(&item->propertyList);
	if (link)
	{
		//print item properties
		chat->SendMessage(p, "+------------------+----------------+-----------------------------------------------------------------+");
		chat->SendMessage(p, "| Property Name    | Property Value |");
		chat->SendMessage(p, "+------------------+----------------+");
		while (link)
		{
			Property *prop = link->data;
			chat->SendMessage(p, "| %-16s | %-14i |", prop->name, prop->value);
			link = link->next;
		}
		chat->SendMessage(p, "+------------------+----------------+");
	}
	else
	{
		//no item properties
		chat->SendMessage(p, "+-----------------------------------------------------------------------------------------------------+");
	}
	database->unlock();
}

local helptext_t grantItemHelp =
"Targets: player or freq or arena\n"
"Args: [-f] [-q] [-i] [-c <amount>] [-s <ship #>] <item>\n"
"Adds the specified item to the inventory of the targeted players.\n"
"If {-s} is not specified, the item is added to the player's current ship.\n"
"Will only effect speccers if {-s} is used. The added amount defaults to 1.\n"
"For typo safety, the {-f} must be specified when granting to more than one player.\n"
"The {-i} flag ignores ship requirements (single receipient only).";

local void grantItemCommand(const char *command, const char *params, Player *p, const Target *target)
{
	int force = 0;
	int quiet = 0;
	int count = 1;
	int ship = 0;
	int ignore = 0;
	const char *itemName;
	Item *item;
	int newItemCount;

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

			else if (*params == 'f')
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
			else if (*params == 'i')
			{
				ignore = 1;

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
			else if (*params == 'q')
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
			else if (*params == 'c')
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
			else if (*params == 's')
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
	item = getItemByName(itemName, p->arena);
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

				newItemCount = getItemCount(t, item, ship) + count;
				if (ignore || item->max == 0 || newItemCount <= item->max)
				{
					Link *link;
					database->lock();
					for (link = LLGetHead(&item->itemTypeEntries); link; link = link->next)
					{
						ItemTypeEntry *entry = link->data;
					
						if (entry->itemType != NULL)
						{
							if (!ignore && getFreeItemTypeSpotsNoLock(t, entry->itemType, ship) - (entry->delta * count) < 0) //have no free spots
							{
								chat->SendMessage(p, "Does not have enough free %s spots.", entry->itemType->name);
								
								database->unlock();
								return;
							}
						}
					}
					database->unlock();

					addItem(t, item, ship, count);
					if (ship == t->p_ship)
					{
						if (count > 0)
						{
							int i;
							for (i = 0; i < count; i++)
							{
								triggerEventOnItem(t, item, ship, "add");
							}
						}
						else
						{
							int i;
							for (i = 0; i < -count; i++)
							{
								triggerEventOnItem(t, item, ship, "del");
							}
						}
					}

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
					chat->SendMessage(p, "%i would exceed the max of %i for item %s.", newItemCount, item->max, item->name);
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
			int playerCount;
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
							int newItemCount = getItemCount(t, item, ship) + count;
							if (item->max == 0 || newItemCount <= item->max)
							{
								Link *link;
								database->lock();
								for (link = LLGetHead(&item->itemTypeEntries); link; link = link->next)
								{
									ItemTypeEntry *entry = link->data;
								
									if (entry->itemType != NULL)
									{
										if (!ignore && getFreeItemTypeSpotsNoLock(t, entry->itemType, ship) - (entry->delta * count) < 0) //have no free spots
										{
											chat->SendMessage(p, "Player %s does not have enough free %s spots.", t->name, entry->itemType->name);
											
											database->unlock();
											return;
										}
									}
								}
								database->unlock();

								addItem(t, item, t->p_ship, count);
								if (!quiet)
								{
									chat->SendMessage(t, "You were granted %i of item %s on your %s.", count, item->name, shipNames[t->p_ship]);
								}
							}
							else
							{
								chat->SendMessage(p, "Giving Player %s %i of item %s would exceed the max of %i.", t->name, newItemCount, item->name, item->max);
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
						triggerEventOnItem(t, item, ship, "init");
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

			playerCount = LLCount(&set);

			LLEmpty(&set);

			chat->SendMessage(p, "You gave %i of item %s to %i players.", count, item->name, playerCount);
		}
		else
		{
			chat->SendMessage(p, "For typo safety, the -f must be specified for arena and freq targets.");
		}
	}
}

local int timer_callback(void *clos)
{
	Player *p = clos;

	Target t;
	Ihscorespawner *spawner;
	t.type = T_PLAYER;
	t.u.p = p;

	game->ShipReset(&t);

	spawner = mm->GetInterface(I_HSCORE_SPAWNER, p->arena);
	if (spawner)
	{
		spawner->respawn(p);
		mm->ReleaseInterface(spawner);
	}
	
	return FALSE;
}
	
local void doEvent(Player *p, InventoryEntry *entry, Event *event, LinkedList *updateList) //called with lock held
{
	int action = event->action;

	//do the message
	if (event->message[0] != '\0')
	{
		chat->SendMessage(p, "%s", event->message);
	}

	//do the action
	if (action == ACTION_NO_ACTION) //do nothing
	{
		//nothing :)
	}
	else if (action == ACTION_REMOVE_ITEM) //removes event->data amount of the items from the ship's inventory
	{
		if (entry != NULL)
		{
			//add to the list for removal
			RemovalEntry *re = amalloc(sizeof(*re));
			re->count = event->data;
			re->item = entry->item;
			LLAdd(updateList, re);
		}
		else
		{
			lm->LogP(L_ERROR, "hscore_items", p, "Item tried to run event %d with null entry.", action);
		}
	}
	else if (action == ACTION_REMOVE_ITEM_AMMO) //removes event->data amount of the item's ammo type from inventory
	{
		if (entry != NULL)
		{
			RemovalEntry *re = amalloc(sizeof(*re));
			re->count = event->data;
			re->item = entry->item->ammo;
			LLAdd(updateList, re);
		}
		else
		{
			lm->LogP(L_ERROR, "hscore_items", p, "Item tried to run event %d with null entry.", action);
		}
	}
	else if (action == ACTION_PRIZE) //sends prize #event->data to the player
	{
		int prize = event->data % 100;
		int count = abs(event->data / 100);
		Target t;

		if (count == 0)
		{
			count = 1;
		}


		t.type = T_PLAYER;
		t.u.p = p;

		game->GivePrize(&t, prize, count);
	}
	else if (action == ACTION_SET_INVENTORY_DATA) //sets the item's inventory data to event->data.
	{
		if (entry != NULL)
		{
			database->updateInventoryNoLock(p, p->p_ship, entry, entry->count, event->data);
		}
		else
		{
			lm->LogP(L_ERROR, "hscore_items", p, "Item tried to run event %d with null entry.", action);
		}
	}
	else if (action == 	ACTION_INCREMENT_INVENTORY_DATA) //does a ++ on inventory data.
	{
		if (entry != NULL)
		{
			database->updateInventoryNoLock(p, p->p_ship, entry, entry->count, entry->data + 1);
		}
		else
		{
			lm->LogP(L_ERROR, "hscore_items", p, "Item tried to run event %d with null entry.", action);
		}
	}
	else if (action == ACTION_DECREMENT_INVENTORY_DATA) //does a -- on inventory data. A "datazero" event may be generated as a result.
	{
		if (entry != NULL)
		{
			int newData = entry->data - 1;
			if (newData < 0)
			{
				newData = 0;
			}

			database->updateInventoryNoLock(p, p->p_ship, entry, entry->count, newData);

			if (newData == 0)
			{
				Link *eventLink = LLGetHead(&entry->item->eventList);
				while (eventLink != NULL)
				{
					Event *eventToCheck = eventLink->data;
					eventLink = eventLink->next;

					if (strcmp(eventToCheck->event, "datazero") == 0)
					{
						doEvent(p, entry, eventToCheck, updateList);
					}
				}
			}
		}
		else
		{
			lm->LogP(L_ERROR, "hscore_items", p, "Item tried to run event %d with null entry.", action);
		}
	}
	else if (action == ACTION_SPEC) //Specs the player.
	{
		game->SetFreqAndShip(p, SHIP_SPEC, p->arena->specfreq);
	}
	else if (action == ACTION_SHIP_RESET) //sends a shipreset packet and reprizes all items (antideath, really)
	{
		int respawnTime = cfg->GetInt(p->arena->cfg, "Kill", "EnterDelay", 0);
		int delay = respawnTime - 50;
		if (delay < 1) delay = 1;
		
		ml->ClearTimer(timer_callback, p);	
		ml->SetTimer(timer_callback, delay, delay, p, p);
	}
	else if (action == ACTION_CALLBACK) //calls a callback passing an eventid of event->data.
	{
		DO_CBS(CB_EVENT_ACTION, p->arena, eventActionFunction, (p, event->data));
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_items", p, "Unknown action code %i", action);
	}
}

local int getItemCount(Player *p, Item *item, int ship) //call without lock held
{
	int count;
	database->lock();
	count = internalGetItemCount(p, item, ship);
	database->unlock();
	return count;
}

local int internalGetItemCount(Player *p, Item *item, int ship) //call with lock held
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	Link *link;
	LinkedList *inventoryList;

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

	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;

		if (entry->item == item)
		{
			return entry->count;
		}
	}

	return 0; //don't have it
}

local int addItem(Player *p, Item *item, int ship, int amount) //call with lock
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	Link *link;
	Link *propLink;
	LinkedList *inventoryList;
	int data = 0;
	int count = 0;
	int doInit = 0;
	int recalcCache = 0;

	if (item == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add a NULL item.");
		return 0;
	}

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add item to a player with unloaded ships");
		return 0;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add item to ship %i", ship);
		return 0;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to add item to unowned ship %i", ship);
		return 0;
	}
	
	if (ammount == 0)
	{
		lm->LogP(L_WARNING, "hscore_items", p, "asked to add 0 of item %s", item->name);
		//return 0;
	}

	inventoryList = &playerData->hull[ship]->inventoryEntryList;

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

	if (count == 0 && amount != 0)
	{
		//we need to do an init event.
		doInit = 1;
	}

	count += amount;

	if (count < 0)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to set item %s count to %i. Setting to 0.", item->name, count);
		count = 0; //no negative counts make sense
	}

	//recalc the related entries
	for (propLink = LLGetHead(&item->propertyList); propLink; propLink = propLink->next)
	{
		Property *prop = propLink->data;

		//cache it
		int *propertySum = (int*)HashGetOne(playerData->hull[ship]->propertySums, prop->name);
		if (propertySum != NULL)
		{
			int propDifference = prop->value * amount;
			*propertySum += propDifference;
		}
		else
		{
			//not in cache already

			//it's too much work to generate the entire entry only to update it.
			//instead we'll just leave it out of the cache, and it can be fully generated when needed
		}
	}
	
	database->updateItemNoLock(p, ship, item, count, data);

	//check other items that use this item as ammo
	if (count == 0 || doInit)
	{
		Link *ammoLink;
		for (ammoLink = LLGetHead(&item->ammoUsers); ammoLink; ammoLink = ammoLink->next)
		{
			Item *user = ammoLink->data;
			
			int userCount = internalGetItemCount(p, user, ship);
			
			if (userCount != 0)
			{
				if (doInit)
				{
					DO_CBS(CB_AMMO_ADDED, p->arena, ammoAddedFunction, (p, ship, user));
				}
				else
				{
					DO_CBS(CB_AMMO_REMOVED, p->arena, ammoRemovedFunction, (p, ship, user));
				}
				
				recalcCache = 1;
			}
		}
	}	
	
	if (recalcCache)
	{
		recalcEntireCache(p, ship);
	}
	
	if (doInit)
	{
		internalTriggerEventOnItem(p, item, ship, "init");
	}

	if (count == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

local Item * getItemByName(const char *name, Arena *arena) //call with no lock
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

local Item * getItemByPartialName(const char *name, Arena *arena) //call with no lock
{
	int matches = 0;
	Item *item;
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
			Item *i = itemLink->data;

			if (strncasecmp(i->name, name, strlen(name)) == 0)
			{
				item = i;
				matches++;
			}
		}
	}
	database->unlock();
	
	if (matches == 1)
	{
		return item;
	}
	else
	{
		return NULL;
	}
}

local int getPropertySum(Player *p, int ship, const char *propString) //call with no lock
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	int *propertySum;
	Link *link;
	LinkedList *inventoryList;
	int count = 0;

	if (propString == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to get props for NULL string.");
		return 0;
	}

	if (!database->areShipsLoaded(p))
	{
		//lm->LogP(L_ERROR, "hscore_items", p, "asked to get props from a player with unloaded ships");
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

	database->lock();

	//check if it's in the cache
	propertySum = (int*)HashGetOne(playerData->hull[ship]->propertySums, propString);
	if (propertySum != NULL)
	{
		database->unlock();
		return *propertySum;
	}

	//not in the cache, look it up
	inventoryList = &playerData->hull[ship]->inventoryEntryList;


	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		Link *propLink;

		InventoryEntry *entry = link->data;
		Item *item = entry->item;

		if (item->ammo != NULL)
		{
			int itemCount = internalGetItemCount(p, item->ammo, ship);

			if (itemCount <= 0)
			{
				continue; //out of ammo, ignore the item.
			}
		}

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

	//cache it
	propertySum = amalloc(sizeof(*propertySum));
	*propertySum = count;
	HashAdd(playerData->hull[ship]->propertySums, propString, propertySum);

	//then return
	return count;
}

local void processUpdateList(Player *p, int ship, LinkedList *updateList) //call with lock
{
	Link *link;
	for (link = LLGetHead(updateList); link; link = link->next)
	{
		RemovalEntry *re = link->data;
		
		addItem(Player *p, re->item, ship, -re->count);
		
		if (re->count > 0)
		{
			//removing
			int i;
			for (i = 0; i < re->count; i++)
			{
				internalTriggerEventOnItem(t, item, ship, "del");
			}			
		}
		else
		{
			//adding
			int i;
			for (i = 0; i < -re->count; i++)
			{
				internalTriggerEventOnItem(t, item, ship, "add");
			}				
		}
		
		afree(re);
	}
	
	LLEmpty(updateList);
}

local void triggerEvent(Player *p, int ship, const char *eventName) //call with no lock
{
	database->lock();
	internalTriggerEvent(p, ship, eventName);
	database->unlock();
}

local void internalTriggerEvent(Player *p, int ship, const char *eventName) //called with lock held
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	LinkedList *inventoryList;
	LinkedList updateList;
	Link *link;

	if (p->type == T_FAKE)
		return;

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

	LLInit(&updateList);
	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	link = LLGetHead(inventoryList);
	while (link != NULL)
	{
		Link *eventLink;
		InventoryEntry *entry = link->data;
		Item *item = entry->item;
		link = link->next; //the current node may be destroyed by doEvent.

		for (eventLink = LLGetHead(&item->eventList); eventLink; eventLink = eventLink->next)
		{
			Event *event = eventLink->data;

			if (strcmp(event->event, eventName) == 0)
			{
				doEvent(p, entry, event, &updateList);
			}
		}
	}
	
	processUpdateList(p, ship, updateList);
}

local void triggerEventOnItem(Player *p, Item *triggerItem, int ship, const char *eventName) //call with no lock
{
	database->lock();
	internalTriggerEventOnItem(p, triggerItem, ship, eventName);
	database->unlock();
}

local void internalTriggerEventOnItem(Player *p, Item *triggerItem, int ship, const char *eventName) //lock must be held
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	int foundItem = 0;
	LinkedList *inventoryList;
	LinkedList updateList;
	Link *link;

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

	LLInit(&updateList);
	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	link = LLGetHead(inventoryList);
	while (link != NULL)
	{
		InventoryEntry *entry = link->data;
		Item *item = entry->item;
		link = link->next; //do event might destroy current node

		if (item == triggerItem)
		{
			Link *eventLink;
			foundItem = 1;

			for (eventLink = LLGetHead(&item->eventList); eventLink; eventLink = eventLink->next)
			{
				Event *event = eventLink->data;

				if (strcmp(event->event, eventName) == 0)
				{
					doEvent(p, entry, event, &updateList);
				}
			}

			break; //found the item, stop looping
		}
	}

	//check if we didn't find the item
	if (!foundItem)
	{
		Link *eventLink;
		for (eventLink = LLGetHead(&triggerItem->eventList); eventLink; eventLink = eventLink->next)
		{
			Event *event = eventLink->data;

			if (strcmp(event->event, eventName) == 0)
			{
				doEvent(p, NULL, event, &updateList);
			}
		}
	}
	
	processUpdateList(p, ship, updateList);
	
	DO_CBS(CB_TRIGGER_EVENT, p->arena, triggerEventFunction, (p, triggerItem, ship, eventName));
}

local int getFreeItemTypeSpotsNoLock(Player *p, ItemType *type, int ship) //call with lock
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	Link *link;
	LinkedList *inventoryList;
	int count;

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

	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	count = type->max;

	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;
		Item *item = entry->item;

		Link *itemTypeLink;
		for (itemTypeLink = LLGetHead(&item->itemTypeEntries); itemTypeLink; itemTypeLink = itemTypeLink->next)
		{
			ItemTypeEntry *typeEntry = itemTypeLink->data;
			
			if (typeEntry->itemType == type)
			{
				count -= entry->count * typeEntry->delta;
			}
		}
	}

	return count;
}

local int hasItemsLeftOnShip(Player *p, int ship) //lock doesn't matter
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	LinkedList *inventoryList;

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to check items left from a player with unloaded ships");
		return 0;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to check items left on ship %i", ship);
		return 0;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to check items left on unowned ship %i", ship);
		return 0;
	}

	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	if (LLGetHead(inventoryList) == NULL)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

local int zeroCacheEntry(const char *key, void *val, void *clos)
{
	int *cacheEntry = val;
	*cacheEntry = 0;

	return 0;
}

local void recaclulateEntireCache(Player *p, int ship)
{
	PerPlayerData *playerData = database->getPerPlayerData(p);
	HashTable *table;
	Link *link;
	LinkedList *inventoryList;

	if (!database->areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to recalc entire cache for a player with unloaded ships");
		return;
	}

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to recalc entire cache on ship %i", ship);
		return;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_items", p, "asked to recalc entire cache on unowned ship %i", ship);
		return;
	}

	table = playerData->hull[ship]->propertySums;

	//first, set all cache entries to zero, so we can keep the 0 entries
	HashEnum(table, zeroCacheEntry, NULL);

	//then iterate every property and update the cache with it
	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;
		Item *item = entry->item;
		Link *propLink;

		if (item->ammo != NULL)
		{
			int itemCount = internalGetItemCount(p, item->ammo, ship);

			if (itemCount <= 0)
			{
				continue; //out of ammo, ignore the item.
			}
		}

		for (propLink = LLGetHead(&item->propertyList); propLink; propLink = propLink->next)
		{
			Property *prop = propLink->data;

			int propDifference = prop->value * entry->count;

			//get it out of the cache
			int *propertySum = (int*)HashGetOne(table, prop->name);
			if (propertySum != NULL)
			{
				*propertySum += propDifference;
			}
			else
			{
				//no cache entry; create one
				propertySum = amalloc(sizeof(*propertySum));
				*propertySum = propDifference;
				HashAdd(playerData->hull[ship]->propertySums, prop->name, propertySum);
			}
		}
	}
}

local void killCallback(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts, int *green)
{
	triggerEvent(killer, killer->p_ship, "kill");
	triggerEvent(killed, killed->p_ship, "death");
}

local Ihscoreitems interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_ITEMS, "hscore_items")
	getItemCount, addItem, getItemByName, getItemByPartialName, getPropertySum,
	triggerEvent, triggerEventOnItem, getFreeItemTypeSpotsNoLock, hasItemsLeftOnShip,
	recaclulateEntireCache,
};

EXPORT const char info_hscore_items[] = "v1.0 Dr Brain <drbrain@gmail.com>";

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
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);

		if (!lm || !chat || !cmd || !pd || !game || !database || !ml || !cfg)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(pd);
			mm->ReleaseInterface(game);
			mm->ReleaseInterface(database);
			mm->ReleaseInterface(ml);
			mm->ReleaseInterface(cfg);

			return MM_FAIL;
		}

		mm->RegInterface(&interface, ALLARENAS);

		mm->RegCallback(CB_KILL, killCallback, ALLARENAS);

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

		mm->UnregCallback(CB_KILL, killCallback, ALLARENAS);

		cmd->RemoveCommand("iteminfo", itemInfoCommand, ALLARENAS);
		cmd->RemoveCommand("grantitem", grantItemCommand, ALLARENAS);

		ml->ClearTimer(timer_callback, NULL);
		
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(game);
		mm->ReleaseInterface(database);
		mm->ReleaseInterface(ml);
		mm->ReleaseInterface(cfg);

		return MM_OK;
	}
	return MM_FAIL;
}
