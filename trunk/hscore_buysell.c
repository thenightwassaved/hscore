#include <string.h>
#include <stdlib.h>

#include "../asss.h"
#include "hscore_database.h"
#include "hscore_money.h"
#include "hscore_map.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihsmoney *money;
local Ihsdatabase *db;
local Ihsmap *map;
local Iplayerdata *pd;

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

local void printAllCategories(Player *p, LinkedList *category_list)
{
	Link *link = LLGetHead(category_list);
	Category *cat;

	chat->SendMessage(p, "+----------------------+---------------------------------------------------------------------------+");
	chat->SendMessage(p, "| Category Name        | Category Description                                                      |");
	chat->SendMessage(p, "+----------------------+---------------------------------------------------------------------------+");
	chat->SendMessage(p, "| Ships                | All the ship hulls can you buy in this arena.                             |");

	while(link)
	{
		cat = link->data;

		chat->SendMessage(p, "| %-20s | %-73s |", cat->name, cat->description);

		link = link->next;
	}


	chat->SendMessage(p, "+----------------------+---------------------------------------------------------------------------+");
}

local void printCategoryItems(Player *p, Category *cat)
{
	Link player_link = { NULL, p };
	LinkedList lst = { &player_link, &player_link };

	int player_money = money->GetMoney(p);
	int player_exp = money->GetExp(p);

	Link *link = LLGetHead(cat->item_list);

	chat->SendMessage(p, "+-----------------+");
	chat->SendMessage(p, "| %-15s |", cat->name);
	chat->SendMessage(p, "+-----------------+-----------+------------+--------+----------+-----------------------------------+");
	chat->SendMessage(p, "| Item Name       | Buy Price | Sell Price | Exp    | Ships    | Item Description                  |");
	chat->SendMessage(p, "+-----------------+-----------+------------+--------+----------+-----------------------------------+");

	while(link)
	{
		int i;
		int type;

		char shipmask[] = "12345678";
		Item *item = link->data;

		for (i = 0; i < 8; i++)
		{
			if (!(1<<i & item->ship_mask))
				shipmask[i] = ' ';
		}

		if (player_money >= item->buy_price && player_exp >= item->exp_required)
		{
			type = MSG_ARENA;
		}
		else
		{
			type = MSG_PUB;
		}

		chat->SendAnyMessage(&lst, type, 0, NULL, "| %-15s | %9i | %10i | %6i | %s | %-33s |", item->name, item->buy_price, item->sell_price, item->exp_required, shipmask, item->short_description);

		link = link->next;
	}


	chat->SendMessage(p, "+-----------------+-----------+------------+--------+----------+-----------------------------------+");
}

local void printShipList(Player *p)
{
	int i;
	Link player_link = { NULL, p };
	LinkedList lst = { &player_link, &player_link };

	ConfigHandle conf = p->arena->cfg;

	int player_money = money->GetMoney(p);
	int player_exp = money->GetExp(p);

	chat->SendMessage(p, "+-----------+-----------+------------+--------+----------------------------------------------------+");
	chat->SendMessage(p, "| Ship Name | Buy Price | Sell Price | Exp    | Ship Description                                   |");
	chat->SendMessage(p, "+-----------+-----------+------------+--------+----------------------------------------------------+");

	for (i = 0; i < 8; i++)
	{
		char *shipname = ship_names[i];
		int type;

		int buy_price = cfg->GetInt(conf, shipname, "BuyPrice", 0);
		int sell_price = cfg->GetInt(conf, shipname, "SellPrice", 0);
		int exp_required = cfg->GetInt(conf, shipname, "ExpRequired", 0);

		const char *description = cfg->GetStr(conf, shipname, "Description");
		if (!description) description = "No description avalible.";

		if (!buy_price) return; //dont list the ship unless it can be bought.

		if (player_money >= buy_price && player_exp >= exp_required)
		{
			type = MSG_ARENA;
		}
		else
		{
			type = MSG_SYSOPWARNING;
		}

		chat->SendAnyMessage(&lst, type, 0, NULL, "| %-9s | $%-8i | $%-9i | %-6i | %-50s |", shipname, buy_price, sell_price, exp_required, description);
	}


	chat->SendMessage(p, "+-----------+-----------+------------+--------+----------------------------------------------------+");
}

/*
+------------------------+
|                        |
|   Buy/Sell Functions   |
|                        |
+------------------------+
*/

local void buyItem(Player *p, Item *item)
{
	int player_money = money->GetMoney(p);
	int player_exp = money->GetExp(p);

	if (player_money >= item->buy_price)
	{
		if (player_exp >= item->exp_required)
		{
			if (p->p_ship != SHIP_SPEC)
			{
				if (1<<p->p_ship & item->ship_mask)
				{
					if (map->ItemAvalible(p, item))
					{
						chat->SendMessage(p, "Ok, you could buy \"%s\" if the buy code actually worked.", item->name);
					}
					else
					{
						chat->SendMessage(p, "You cannot buy \"%s\" from your current location. <FIXME: Needs more info>", item->name);
					}
				}
				else
				{
					chat->SendMessage(p, "You cannot buy \"%s\" for your current ship.", item->name);
				}
			}
			else
			{
				chat->SendMessage(p, "You cannot buy \"%s\" in spec. Please enter the ship you wish to buy this item for and try again.", item->name);
			}
		}
		else
		{
			chat->SendMessage(p, "You do not have enough exp to buy \"%s\". You need %i more exp to reach %i.", item->name, item->exp_required - player_exp, item->exp_required);
		}
	}
	else
	{
		chat->SendMessage(p, "You do not have enough money to buy \"%s\". You need $%i more to reach $%i.", item->name, item->buy_price - player_money, item->buy_price);
	}
}

local void sellItem(Player *p, Item *item)
{
	if (map->ItemAvalible(p, item))
	{
		chat->SendMessage(p, "Ok, you could sell \"%s\" if the sell code actually worked.", item->name);
	}
	else
	{
		chat->SendMessage(p, "You cannot sell \"%s\" from your current location. <FIXME: Needs more info>", item->name);
	}
}

local void buyShip(Player *p, int ship)
{
	char *shipname = ship_names[ship];
	ConfigHandle conf = p->arena->cfg;

	int buy_price = cfg->GetInt(conf, shipname, "BuyPrice", 0);
	int exp_required = cfg->GetInt(conf, shipname, "ExpRequired", 0);

	if (!buy_price)
	{
		chat->SendMessage(p, "The %s hull is not for sale in this arena.", shipname);
		return;
	}

	int player_money = money->GetMoney(p);
	int player_exp = money->GetExp(p);

	if (player_money >= buy_price)
	{
		if (player_exp >= exp_required)
		{
			if (p->p_ship == SHIP_SPEC)
			{
				HSData *data = db->GetHSData(p);
				if (data->hull_status[ship] == HULL_NO_HULL)
				{
					db->AddShip(p, ship);
					money->GiveMoney(p, -buy_price, MONEY_TYPE_BUYSELL);
					chat->SendMessage(p, "%s hull purchaced for $%i. You may now enter with ESC+%i.", shipname, buy_price, ship+1);
				}
				else if (data->hull_status[ship] == HULL_LOADED)
				{
					chat->SendMessage(p, "You already have a %s hull.", shipname);
				}
				else
				{
					chat->SendMessage(p, "Your ships are not loaded. If you just entered the arena please wait a few seconds and try again.");
				}
			}
			else
			{
				chat->SendMessage(p, "You can only buy the %s hull in spec. Please enter spec and try again.", shipname);
			}
		}
		else
		{
			chat->SendMessage(p, "You do not have enough exp to buy the %s hull. You need %i more exp to reach %i.", shipname, exp_required - player_exp, exp_required);
		}
	}
	else
	{
		chat->SendMessage(p, "You do not have enough money to buy the %s hull. You need $%i more to reach $%i.", shipname, buy_price - player_money, buy_price);
	}
}

local void sellShip(Player *p, int ship)
{
	char *shipname = ship_names[ship];
	ConfigHandle conf = p->arena->cfg;

	int sell_price = cfg->GetInt(conf, shipname, "SellPrice", 0);

	if (p->p_ship == SHIP_SPEC)
	{
		HSData *data = db->GetHSData(p);
		if (data->hull_status[ship] == HULL_LOADED)
		{
			db->RemoveShip(p, ship);
			money->GiveMoney(p, sell_price, MONEY_TYPE_BUYSELL);
			chat->SendMessage(p, "%s hull sold for $%i.", shipname, sell_price);
		}
		else if (data->hull_status[ship] == HULL_NO_HULL)
		{
			chat->SendMessage(p, "You do not have a %s hull.", shipname);
		}
		else
		{
			chat->SendMessage(p, "Your ships are not loaded. If you just entered the arena please wait a few seconds and try again.");
		}
	}
	else
	{
		chat->SendMessage(p, "You can only sell the %s hull in spec. Please enter spec and try again.", shipname);
	}


}

/*
+-----------------+
|                 |
|   Buy Command   |
|                 |
+-----------------+
*/

local helptext_t buy_help =
"Targets: none\n"
"Args: <item>|<category> or none\n"
"If there is no arugment, this command will display a list of this arena's buy categories.\n"
"If the argument is a category, this command will display all the items in that category.\n"
"If the argument is an item, this command will attemt to buy it for the buy price.\n";

local void Cbuy(const char *command, const char *params, Player *p, const Target *target)
{
	if (db->Loaded(p))
	{
		LinkedList *category_list = db->GetCategoryList(p->arena);

		if (!category_list)
		{
			chat->SendMessage(p, "Unexpected error: unable to get category list.");
			return;
		}



		if (strcasecmp(params, "") == 0) //display all categories
		{
			printAllCategories(p, category_list);

			return;
		}
		else //has params
		{
			int i;

			Link *link = LLGetHead(category_list);
			Category *cat;

			while(link)
			{
				cat = link->data;

				if (strcasecmp(params, cat->name) == 0)
				{
					printCategoryItems(p, cat);

					return; //Exit out
				}

				link = link->next;
			}

			//none of the MySQL categories, check "ships"

			if (strcasecmp(params, "ships") == 0)
			{
				printShipList(p);

				return; //Exit out
			}

			//ok, not a category name, check all the category's items for the name

			link = LLGetHead(category_list);

			while(link)
			{
				cat = link->data;

				Link *item_link = LLGetHead(cat->item_list);

				while (item_link)
				{
					Item *item = item_link->data;

					if (strcasecmp(params, item->name) == 0)
					{
						buyItem(p, item);

						return;
					}

					item_link = item_link->next;
				}

				link = link->next;
			}

			//None of the MySQL items, check ships

			for (i = 0; i < 8; i++)
			{
				if (strcasecmp(params, ship_names[i]) == 0)
				{
					buyShip(p, i);

					return;
				}
			}

			chat->SendMessage(p, "Unknown item \"%s\".", params);
		}
	}
	else
	{
		chat->SendMessage(p, "Unexpected error: Your zone data is not loaded.");
	}
}

/*
+------------------+
|                  |
|   Sell Command   |
|                  |
+------------------+
*/

local helptext_t sell_help =
"Targets: player or none\n"
"Args: <item>\n"
"Sells the item from your ship.\n"
"Refunds you the item's sell price.\n";

local void Csell(const char *command, const char *params, Player *p, const Target *target)
{
	if (db->Loaded(p))
	{
		LinkedList *category_list = db->GetCategoryList(p->arena);

		if (!category_list)
		{
			chat->SendMessage(p, "Unexpected error: unable to get category list.");
			return;
		}

		if (strcasecmp(params, "") == 0) //asked for menu, refer to ?buy
		{
			chat->SendMessage(p, "To see a menu of items, please use ?buy.");
		}
		else
		{
			int i;

			Link *link = LLGetHead(category_list);
			Category *cat;

			while(link)
			{
				cat = link->data;

				Link *item_link = LLGetHead(cat->item_list);

				while (item_link)
				{
					Item *item = item_link->data;

					if (strcasecmp(params, item->name) == 0)
					{
						sellItem(p, item);

						return;
					}

					item_link = item_link->next;
				}

				link = link->next;
			}

			//None of the MySQL items, check ships

			for (i = 0; i < 8; i++)
			{
				if (strcasecmp(params, ship_names[i]) == 0)
				{
					sellShip(p, i);

					return;
				}
			}

			chat->SendMessage(p, "Unknown item \"%s\".", params);
		}
	}
	else
	{
		chat->SendMessage(p, "Unexpected error: Your zone data is not loaded.");
	}
}



EXPORT int MM_hscore_buysell(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		money = mm->GetInterface(I_HS_MONEY, ALLARENAS);
		db = mm->GetInterface(I_HS_DATABASE, ALLARENAS);
		map = mm->GetInterface(I_HS_MAP, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);


		if (!lm || !chat || !cfg || !cmd || !money || !db || !map || !pd)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(money);
			mm->ReleaseInterface(db);
			mm->ReleaseInterface(map);
			mm->ReleaseInterface(pd);
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
		mm->ReleaseInterface(money);
		mm->ReleaseInterface(db);
		mm->ReleaseInterface(map);
		mm->ReleaseInterface(pd);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		cmd->AddCommand2("buy", Cbuy, arena, buy_help);
		cmd->AddCommand2("sell", Csell, arena, sell_help);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		cmd->RemoveCommand2("buy", Cbuy, arena);
		cmd->RemoveCommand2("sell", Csell, arena);

		return MM_OK;
	}
	return MM_FAIL;
}

