#include <string.h>
#include <stdlib.h>

#include "../asss.h"
#include "hscore_database.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihsdatabase *db;

static char *ship_names[] =
{
	"Warbird",
	"Javelin",
	"Spider",
	"Leviathan",
	"Terrier",
	"Weasel",
	"Lancaster",
	"Shark"
};

local helptext_t ships_help =
"Targets: none\n"
"Args: none\n"
"Lists the ship hulls you currently own.\n";

local void Cships(const char *command, const char *params, Player *p, const Target *target)
{
	Player *t = (target->type == T_PLAYER) ? target->u.p : p;
	HSData *data = db->GetHSData(t);

	if (db->Loaded(t))
	{
		char unowned[] = " ";
		char unaval[] = "<Unaval>";
		char error[] = "<ERROR>";
		char *status[8];
		int i;

		for (i = 0; i < 8; i++)
		{
			if (data->hull_status[i] == HULL_LOADED)
			{
				status[i] = ship_names[i];
			}
			else if (data->hull_status[i] == HULL_NO_HULL)
			{
				if (cfg->GetInt(p->arena->cfg, ship_names[i], "BuyPrice", 0)) //if it has a price
				{
					status[i] = unowned;
				}
				else
				{
					status[i] = unaval;
				}
			}
			else
			{
				status[i] = error;
			}
		}

		chat->SendMessage(p, "+-----------+-----------+-----------+------------+-----------+-----------+------------+------------+");
		chat->SendMessage(p, "| %-9s | %-9s | %-9s | %-10s | %-9s | %-9s | %-10s | %-10s |", status[0], status[1], status[2], status[3], status[4], status[5], status[6], status[7] );
		chat->SendMessage(p, "+-----------+-----------+-----------+------------+-----------+-----------+------------+------------+");
	}
	else
	{
		if (p == t)
			chat->SendMessage(p, "Unexpected error: Your zone data is not loaded.");
		else
			chat->SendMessage(p, "Unexpected error: %s's zone data is not loaded.", t->name);

	}
}

local helptext_t shipstatus_help =
"Targets: player or none\n"
"Args: [ship] or none\n"
"Sells the item from your ship.\n"
"Refunds you the item's sell price.\n";

local void Cshipstatus(const char *command, const char *params, Player *p, const Target *target)
{
	Player *t = (target->type == T_PLAYER) ? target->u.p : p;

	if (db->Loaded(t))
	{
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
			chat->SendMessage(p, "Ship out of range. Please choose a ship between 1 and 8");
			return;
		}

		chat->SendMessage(p, "<FIXME: Ship status for player %s's %s>.", t->name, ship_names[ship]);
	}
	else
	{
		if (p == t)
			chat->SendMessage(p, "Unexpected error: Your zone data is not loaded.");
		else
			chat->SendMessage(p, "Unexpected error: %s's zone data is not loaded.", t->name);
	}
}

local void printStoreList(Player *p)
{
	Link* link = LLGetHead(db->GetStoreList(p->arena));

	chat->SendMessage(p, "+----------------------+");
	chat->SendMessage(p, "| Store Name           |");
	chat->SendMessage(p, "+----------------------+");

	while(link)
	{
		Store *store = link->data;

		chat->SendMessage(p, "| %-20s |", store->name);

		link = link->next;
	}

	chat->SendMessage(p, "+----------------------+");
}

local helptext_t storeinfo_help =
"Targets: none\n"
"Args: <store>\n"
"Displays all the info for the specified store.\n";

local void Cstoreinfo(const char *command, const char *params, Player *p, const Target *target)
{
	Link* link = LLGetHead(db->GetStoreList(p->arena));

	if (strcasecmp(params, "") == 0) //display all categories
	{
		printStoreList(p);

		return;
	}

	while(link)
	{
		Store *store = link->data;

		if (strcasecmp(params, store->name) == 0)
		{
			char buf[256];
			const char *temp = NULL;

			chat->SendMessage(p, "+----------------------+");
			chat->SendMessage(p, "| %-20s |", store->name);
			chat->SendMessage(p, "+----------------------+---------------------------------------------------------------------------+");

			while (strsplit(store->description, "~", buf, 256, &temp))
			{
				chat->SendMessage(p, "| %-96s |", buf);
			}

			chat->SendMessage(p, "+--------------------------------------------------------------------------------------------------+");

			return;
		}

		link = link->next;
	}

	chat->SendMessage(p, "Unknown store \"%s\".", params);
}

local helptext_t iteminfo_help =
"Targets: none\n"
"Args: <item>\n"
"Displays all the info for the specified item.\n";

local void Citeminfo(const char *command, const char *params, Player *p, const Target *target)
{
	Link* link = LLGetHead(db->GetCategoryList(p->arena));

	if (strcasecmp(params, "") == 0) //display all categories
	{
		chat->SendMessage(p, "Please use ?buy to get a list of items avalible in this arena.");

		return;
	}

	while(link)
	{
		Category *cat = link->data;

		Link *item_link = LLGetHead(cat->item_list);

		while (item_link)
		{
			Item *item = item_link->data;

			if (strcasecmp(params, item->name) == 0)
			{
				char buf[256];
				const char *temp = NULL;

				int i;
				char shipmask[] = "12345678";

				Link *prop_link = LLGetHead(item->property_list);

				for (i = 0; i < 8; i++)
				{
					if (!(1<<i & item->ship_mask))
						shipmask[i] = ' ';
				}

				chat->SendMessage(p, "+-----------------+-----------------------+------------------------+-------------+-----------------+");
				chat->SendMessage(p, "| %-15s | Buy Price: %10i | Sell Price: %10i | Exp: %6i | Ships: %s |", item->name, item->buy_price, item->sell_price, item->exp_required, shipmask);
				chat->SendMessage(p, "+-----------------+----+---------+--------+------------------------+-------------+-----------------+");

				strsplit(item->description, "~", buf, 256, &temp); //a bit hacky, but it works without a lot of code

				chat->SendMessage(p, "| Property             | Setting | %-63s |", buf);

				astrncpy(buf, "", 255);
				strsplit(item->description, "~", buf, 256, &temp);

				chat->SendMessage(p, "+----------------------+---------+ %-63s |", buf);

				while (prop_link)
				{
					Property *prop = prop_link->data;

					astrncpy(buf, "", 255);
					strsplit(item->description, "~", buf, 256, &temp);
					chat->SendMessage(p, "| %-20s | %7i | %-63s |", prop->name, prop->setting, buf);

					prop_link = prop_link->next;
				}

				if (strsplit(item->description, "~", buf, 256, &temp))
				{
					chat->SendMessage(p, "+----------------------+---------+ %-63s |", buf);
					while (strsplit(item->description, "~", buf, 256, &temp))
					{
						chat->SendMessage(p, "|                                | %-63s |", buf);
					}
				}

				chat->SendMessage(p, "+----------------------+---------+-----------------------------------------------------------------+");


				return;
			}

			item_link = item_link->next;
		}

		link = link->next;
	}

	chat->SendMessage(p, "Unknown item \"%s\".", params);
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

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(db);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		cmd->AddCommand2("ships", Cships, arena, ships_help);
		cmd->AddCommand2("shipstatus", Cshipstatus, arena, shipstatus_help);
		cmd->AddCommand2("storeinfo", Cstoreinfo, arena, storeinfo_help);
		cmd->AddCommand2("iteminfo", Citeminfo, arena, iteminfo_help);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		cmd->RemoveCommand2("ships", Cships, arena);
		cmd->RemoveCommand2("shipstatus", Cshipstatus, arena);
		cmd->RemoveCommand2("storeinfo", Cstoreinfo, arena);
		cmd->RemoveCommand2("iteminfo", Citeminfo, arena);

		return MM_OK;
	}
	return MM_FAIL;
}

