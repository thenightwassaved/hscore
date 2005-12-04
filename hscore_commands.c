#include <stdlib.h>

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

local helptext_t shipStatusHelp =
"Targets: none or player\n"
"Args: [ship number]\n"
"Displays the specified ship's inventory.\n"
"If no ship is specified, your current ship is assumed.\n";

local void shipStatusCommand(const char *command, const char *params, Player *p, const Target *target)
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
			chat->SendMessage(p, "+------------------+-------+------------+------------------+-------+------------------+-------+");
			chat->SendMessage(p, "| Item Name        | Count | Ammo Count | Item Type 1      | Usage | Item Type 2      | Usage |");
			chat->SendMessage(p, "+------------------+-------+------------+------------------+-------+------------------+-------+");

			Link *link;

			database->lock();
			for (link = LLGetHead(&playerData->hull[ship]->inventoryEntryList); link; link = link->next)
			{
				InventoryEntry *entry = link->data;
				Item *item = entry->item;

				int ammoCount;
				if (item->ammo != NULL)
				{

					Link *ammoLink;
					for (ammolink = LLGetHead(&playerData->hull[ship]->inventoryEntryList); ammolink; ammolink = ammolink->next)
					{
						InventoryEntry *ammoEntry = ammolink->data;

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

				char *type1, *type2;
				if (item->type1 == NULL)
				{
					type1 = "<None>";
				}
				else
				{
					type1 = item->type1->name;
				}

				if (item->type2 == NULL)
				{
					type2 = "<None>";
				}
				else
				{
					type2 = item->type2->name;
				}

				chat->SendMessage(p, "| %-16s | %5i | %10i | %-16s | %5i | %-16s | %5i |", item->name, entry->count, ammoCount, type1, item->typeDelta1 * entry->count, type2, item->typeDelta2 * entry->count);
			}
			database->unlock();

			chat->SendMessage(p, "+------------------+-------+------------+------------------+-------+------------------+-------+");
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

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		cmd->RemoveCommand("ships", shipsCommand, arena);
		cmd->RemoveCommand("shipstatus", shipStatusCommand, arena);

		return MM_OK;
	}
	return MM_FAIL;
}
