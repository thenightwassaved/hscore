#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"
#include "hscore_shipnames.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihscoreitems *items;
local Ihscoredatabase *database;

local helptext_t shipsHelp =
"Targets: none or player\n"
"Args: none\n"
"Displays what ships you own in this arena.\n";

local void shipsCommand(const char *command, const char *params, Player *p, const Target *target)
{
	Player *t = (target->type == T_PLAYER) ? target->u.p : p;

	if (database->areShipsLoaded(t))
	{
		PerPlayerData *playerData = database->getPerPlayerData(t);

		char unowned[] = " ";
		char unaval[] = "Free";
		char *status[8];

		//+---------+---------+--------+-----------+---------+--------+-----------+--------+
		//| Warbird | Javelin | Spider | Leviathan | Terrier | Weasel | Lancaster | Shark  |
		//+---------+---------+--------+-----------+---------+--------+-----------+--------+


		for (int i = 0; i < 8; i++)
		{
			if (playerData->hull[i] != NULL)
			{
				status[i] = shipNames[i];
			}
			else
			{
				if (cfg->GetInt(p->arena->cfg, shipNames[i], "BuyPrice", 0) == 0)
				{
					//not for sale
					status[i] = unaval;
				}
				else
				{
					//unowned
					status[i] = unowned;
				}
			}
		}

		chat->SendMessage(p, "+---------+---------+--------+-----------+---------+--------+-----------+--------+");
		chat->SendMessage(p, "| %-7s | %-7s | %-6s | %-9s | %-7s | %-6s | %-9s | %-6s |", status[0], status[1], status[2], status[3], status[4], status[5], status[6], status[7]);
		chat->SendMessage(p, "+---------+---------+--------+-----------+---------+--------+-----------+--------+");
	}
	else
	{
		if (p == t)
		{
			chat->SendMessage(p, "No ships loaded.");
		}
		else
		{
			chat->SendMessage(p, "No ships loaded for %s.", t->name);
		}
	}
}

local int printCacheEntry(const char *key, void *val, void *clos)
{
	int *cacheEntry = val;
	Player *p = clos;

	if (cacheEntry != NULL && *cacheEntry != 0)
	{
		chat->SendMessage(p, "| %-16s | %14i |", key, *cacheEntry);
	}

	return 0;
}

local helptext_t shipItemsHelp =
"Targets: none or player\n"
"Args: [ship number]\n"
"Displays a short list of the ship's items.\n"
"If no ship is specified, current ship is assumed.\n";

local void shipItemsCommand(const char *command, const char *params, Player *p, const Target *target)
{
	Player *t = (target->type == T_PLAYER) ? target->u.p : p;
	
	int ship = atoi(params);
	if (ship == 0)
	{
		ship = t->p_ship;
	}
	else
	{
		ship--; //warbird is 0, not 1
	}

	if (ship == SHIP_SPEC)
	{
		chat->SendMessage(p, "Spectators do not have items. Please use ?shipitems <ship> to check items on a certain hull.");
		return;
	}

	if (ship >= 8 || ship < 0)
	{
		chat->SendMessage(p, "Ship out of range. Please choose a ship from 1 to 8.");
		return;
	}

	if (database->areShipsLoaded(t))
	{
		PerPlayerData *playerData = database->getPerPlayerData(t);
		Link *link;

		if (playerData->hull[ship] != NULL)
		{
			int first = 1;
			char line[100];
			char buffer[100];
			int lineLen = 0;
			int bufferLen;

			line[0] = '\0';
			
			chat->SendMessage(p, "+------------------+");
			chat->SendMessage(p, "| %-16s |", shipNames[ship]);
			chat->SendMessage(p, "+------------------+--------------------------------------------------------------------------+");

			database->lock();
			for (link = LLGetHead(&playerData->hull[ship]->inventoryEntryList); link; link = link->next)
			{
				InventoryEntry *entry = link->data;
				Item *item = entry->item;
				int count = entry->count;
				int last = (link->next == NULL);
				
				if (first && last)
				{
					first = 0;
					
					if (count == 1)
					{
						sprintf(buffer, "%s", item->name);
					}
					else
					{
						sprintf(buffer, "%d %s", count, item->name);
					}
				}
				else if (first)
				{
					first = 0;
					
					if (count == 1)
					{
						sprintf(buffer, "%s,", item->name);
					}
					else
					{
						sprintf(buffer, "%d %s,", count, item->name);
					}				
				}
				else if (last)
				{
					if (lineLen == 0)
					{
						if (count == 1)
						{
							sprintf(buffer, "%s", item->name);
						}
						else
						{
							sprintf(buffer, "%d %s", count, item->name);
						}
					}
					else
					{
						if (count == 1)
						{
							sprintf(buffer, " %s", item->name);
						}
						else
						{
							sprintf(buffer, " %d %s", count, item->name);
						}
					}
				}
				else
				{
					if (lineLen == 0)
					{
						if (count == 1)
						{
							sprintf(buffer, "%s,", item->name);
						}
						else
						{
							sprintf(buffer, "%d %s,", count, item->name);
						}
					}
					else
					{
						if (count == 1)
						{
							sprintf(buffer, " %s,", item->name);
						}
						else
						{
							sprintf(buffer, " %d %s,", count, item->name);
						}
					}			
				}
				
				//buffer is now filled with the properly formatted string.
				bufferLen = strlen(buffer);
				if (lineLen + bufferLen <= 91)
				{
					strcat(line, buffer);
					lineLen = strlen(line);
				}
				else
				{
					//need a new line
					chat->SendMessage(p, "| %-91s |", line);
					
					*line = '\0';
					if (*buffer == ' ')
					{
						strcat(line, buffer + 1);
					}
					else
					{
						strcat(line, buffer);					
					}
					
					lineLen = strlen(line);
				}
			}
			database->unlock();
			
			//check if there's still stuff left in the line
			if (lineLen != 0)
			{
				chat->SendMessage(p, "| %-91s |", line);
			}
			
			chat->SendMessage(p, "+---------------------------------------------------------------------------------------------+");
		}
		else
		{
			int buyPrice = cfg->GetInt(p->arena->cfg, shipNames[ship], "BuyPrice", 0);

			if (buyPrice == 0)
			{
				chat->SendMessage(p, "No items can be loaded onto a %s in this arena.", shipNames[ship]);
			}
			else
			{
				if (p == t)
					chat->SendMessage(p, "You do not own a %s.", shipNames[ship]);
				else
					chat->SendMessage(p, "Player %s does not own a %s.", t->name, shipNames[ship]);
			}
		}
	}
	else
	{
		if (p == t)
			chat->SendMessage(p, "Unexpected error: Your ships are not loaded.");
		else
			chat->SendMessage(p, "Unexpected error: %s's ships are not loaded.", t->name);
	}
}

local helptext_t shipStatusHelp =
"Targets: none or player\n"
"Args: [-v] [ship number]\n"
"Displays the specified ship's inventory.\n"
"If no ship is specified, current ship is assumed.\n"
"The -v flag will give you a list of all properties set on your ship.\n";

local void shipStatusCommand(const char *command, const char *params, Player *p, const Target *target)
{
	int verbose = 0;
	int ship;
	Player *t = (target->type == T_PLAYER) ? target->u.p : p;

	if (strncmp(params, "-v", 2) == 0)
	{
		verbose = 1;
		
		params = strchr(params, ' ');
		if (params) //check so that params can still == NULL
		{
			params++; //we want *after* the space
		}
	}
	
	ship = t->p_ship;
	if (params != NULL)
	{
		ship = atoi(params);
		if (ship == 0)
		{
			ship = t->p_ship;
		}
		else
		{
			ship--; //warbird is 0, not 1
		}
	}

	if (ship == SHIP_SPEC)
	{
		chat->SendMessage(p, "Spectators do not have a ship status. Please use ?shipstatus <ship> to check the status on a certain hull.");
		return;
	}

	if (ship >= 8 || ship < 0)
	{
		chat->SendMessage(p, "Ship out of range. Please choose a ship from 1 to 8.");
		return;
	}

	if (database->areShipsLoaded(t))
	{
		PerPlayerData *playerData = database->getPerPlayerData(t);

		if (playerData->hull[ship] != NULL)
		{
			chat->SendMessage(p, "+------------------+");
			chat->SendMessage(p, "| %-16s |", shipNames[ship]);
			chat->SendMessage(p, "+------------------+-------+------------+-----------------------------------------------------+");
			chat->SendMessage(p, "| Item Name        | Count | Ammo Count | Item Types                                          |");
			chat->SendMessage(p, "+------------------+-------+------------+-----------------------------------------------------+");

			Link *link;

			database->lock();
			for (link = LLGetHead(&playerData->hull[ship]->inventoryEntryList); link; link = link->next)
			{
				InventoryEntry *entry = link->data;
				Item *item = entry->item;
				int first = 1;
				char itemTypes[256];
				char buf[256];
				Link *itemTypeLink;

				int ammoCount;
				if (item->ammo != NULL)
				{

					Link *ammoLink;
					for (ammoLink = LLGetHead(&playerData->hull[ship]->inventoryEntryList); ammoLink; ammoLink = ammoLink->next)
					{
						InventoryEntry *ammoEntry = ammoLink->data;

						if (ammoEntry->item == item->ammo)
						{

							ammoCount = ammoEntry->count;
						}
					}
				}
				else
				{
					ammoCount = 0;
				}

				itemTypes[0] = '\0';
				for (itemTypeLink = LLGetHead(&item->itemTypeEntries); itemTypeLink; itemTypeLink = itemTypeLink->next)
				{
					ItemTypeEntry *entry = itemTypeLink->data;
					
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
				
				chat->SendMessage(p, "| %-16s | %5i | %10i | %-51s |", item->name, entry->count, ammoCount, itemTypes);
			}
			
			if (!verbose)
			{
				chat->SendMessage(p, "+------------------+-------+------------+-----------------------------------------------------+");
			}
			else
			{
				chat->SendMessage(p, "+------------------+-------+--------+---+-----------------------------------------------------+");
				chat->SendMessage(p, "| Property Name    | Property Value |");
				chat->SendMessage(p, "+------------------+----------------+");

				items->recaclulateEntireCache(t, ship);
				HashEnum(playerData->hull[ship]->propertySums, printCacheEntry, p);

				chat->SendMessage(p, "+------------------+----------------+");
			}
			database->unlock();
		}
		else
		{
			int buyPrice = cfg->GetInt(p->arena->cfg, shipNames[ship], "BuyPrice", 0);

			if (buyPrice == 0)
			{
				chat->SendMessage(p, "No items can be loaded onto a %s in this arena.", shipNames[ship]);
			}
			else
			{
				if (p == t)
					chat->SendMessage(p, "You do not own a %s.", shipNames[ship]);
				else
					chat->SendMessage(p, "Player %s does not own a %s.", t->name, shipNames[ship]);
			}
		}
	}
	else
	{
		if (p == t)
			chat->SendMessage(p, "Unexpected error: Your ships are not loaded.");
		else
			chat->SendMessage(p, "Unexpected error: %s's ships are not loaded.", t->name);
	}
}

EXPORT const char info_hscore_commands[] = "v1.0 Dr Brain <drbrain@gmail.com>";

EXPORT int MM_hscore_commands(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		items = mm->GetInterface(I_HSCORE_ITEMS, ALLARENAS);
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);

		if (!lm || !chat || !cfg || !cmd || !items || !database)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(items);
			mm->ReleaseInterface(database);

			return MM_FAIL;
		}

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		cmd->AddCommand("ships", shipsCommand, arena, shipsHelp);
		cmd->AddCommand("shipstatus", shipStatusCommand, arena, shipStatusHelp);
		cmd->AddCommand("shipitems", shipItemsCommand, arena, shipItemsHelp);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		cmd->RemoveCommand("ships", shipsCommand, arena);
		cmd->RemoveCommand("shipstatus", shipStatusCommand, arena);
		cmd->RemoveCommand("shipitems", shipItemsCommand, arena);

		return MM_OK;
	}
	return MM_FAIL;
}
