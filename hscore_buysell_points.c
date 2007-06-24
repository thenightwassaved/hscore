#include <string.h>
#include <stdlib.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_mysql.h"
#include "hscore_storeman.h"
#include "hscore_database.h"
#include "hscore_shipnames.h"

#define SEE_HIDDEN_CATEGORIES "seehiddencat"

typedef struct PointsArenaData
{
	char arenaIdentifier[32];
	int useGlobal;
} PointsArenaData;

typedef struct PointsPlayerData
{
	int globalPoints;
	int shipPoints[8];
} PointsPlayerData;

local int adkey;
local int pdkey;

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Imainloop *ml;
local Icapman *capman;
local Iplayerdata *pd;
local Iarenaman *aman;
local Ihscoremysql *mysql;
local Ihscoreitems *items;
local Ihscoredatabase *database;

local const char * getArenaIdentifier(Arena *arena)
{
	/* cfghelp: Hyperspace:ArenaIdentifier, arena, string, mod: hscore_database
	 * String to compare to the arena field in the MySQL. Defaults to the arena base name. */
	const char *arenaIdent = cfg->GetStr(arena->cfg, "Hyperspace", "ArenaIdentifier");

	if (arenaIdent == NULL)
	{
		arenaIdent = arena->basename; //default fallback
	}

	return arenaIdent;
}

local int getShipPoints(Player *p, int ship)
{
	int points;
	PointsPlayerData *ppd = PPDATA(p, pdkey);

	points = 0;
	if(ship >= 0 && ship < 8)
	{
		points = ppd->shipPoints[ship];
	}

	return points;
}

local int getPoints(Player *p)
{
	int points;
	PointsArenaData *pad = P_ARENA_DATA(p->arena, adkey);
	PointsPlayerData *ppd = PPDATA(p, pdkey);

	points = 0;
	if(pad->useGlobal || p->pkt.ship == SHIP_SPEC)
	{
		points = ppd->globalPoints;
	}
	else
	{
		points = getShipPoints(p, p->pkt.ship);
	}

	return points;
}

local void setGlobalPoints(Player *p, int points)
{
	PointsPlayerData *ppd = PPDATA(p, pdkey);

	if(points >= 0)
	{
		ppd->globalPoints = points;
	}
	else
	{
		ppd->globalPoints = 0;
	}
}

local void setShipPoints(Player *p, int ship, int points)
{
	PointsPlayerData *ppd = PPDATA(p, pdkey);

	if(ship >= 0 && ship < 8 && points >= 0)
	{
		ppd->shipPoints[ship] = points;
	}
	else
	{
		ppd->shipPoints[ship] = 0;
	}
}

local void setPoints(Player *p, int points)
{
	PointsArenaData *pad = P_ARENA_DATA(p->arena, adkey);

	if(pad->useGlobal || p->pkt.ship == SHIP_SPEC)
	{
		setGlobalPoints(p, points);
	}
	else
	{
		setShipPoints(p, p->pkt.ship, points);
	}
}

local void givePoints(Player *p, int points)
{
	setPoints(p, getPoints(p) + points);
}

local void loadArenaData(Arena *a)
{
	PointsArenaData *pad = P_ARENA_DATA(a, adkey);

	astrncpy(pad->arenaIdentifier, getArenaIdentifier(a), sizeof(pad->arenaIdentifier));
	pad->useGlobal = cfg->GetInt(a->cfg, "Hyperspace", "UseGlobalPoints", 1);
}

local void loadDefaultPoints(Player *p)
{
	setGlobalPoints(p,  cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialGlobalPoints", 0));
	setShipPoints(p, 0, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialWarbirdPoints", 0));
	setShipPoints(p, 1, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialJavelinPoints", 0));
	setShipPoints(p, 2, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialSpiderPoints", 0));
	setShipPoints(p, 3, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialLeviathanPoints", 0));
	setShipPoints(p, 4, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialTerrierPoints", 0));
	setShipPoints(p, 5, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialWeaselPoints", 0));
	setShipPoints(p, 6, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialLancasterPoints", 0));
	setShipPoints(p, 7, cfg->GetInt(p->arena->cfg, "Hyperspace", "InitialSharkPoints", 0));
}

local void dbcb_loadPoints(int status, db_res *res, void *clos)
{
	Player *p = (Player *)clos;
	db_row *row;
	int results;
	PointsArenaData *pad = P_ARENA_DATA(p->arena, adkey);

	if (status != 0 || res == NULL)
	{
		lm->LogP(L_ERROR, "buysell_points", p, "Unexpected database error during player points load.");
		return;
	}

	results = mysql->GetRowCount(res);

	if (results > 1)
	{
		lm->LogP(L_ERROR, "buysell_points", p, "Multiple rows returned from MySQL: using first.");
	}

	if(results != 0)
	{
		row = mysql->GetRow(res);
		setGlobalPoints(p, atoi(mysql->GetField(row, 2)));
		for(int i = 0; i < 8; i++)
		{
			setShipPoints(p, i, atoi(mysql->GetField(row, 3 + i)));
		}
	}
	else
	{
		mysql->Query(NULL, NULL, 0,
			"INSERT INTO hs_player_points (player_id, arena) VALUES (#,?)",
			database->getPerPlayerData(p)->id,
			pad->arenaIdentifier);
	}
}

local void loadPlayerData(Player *p)
{
	PointsArenaData *pad = P_ARENA_DATA(p->arena, adkey);

	mysql->Query(dbcb_loadPoints, p, 0,
		"SELECT * FROM hs_player_points WHERE player_id = # AND arena = ?",
		database->getPerPlayerData(p)->id,
		pad->arenaIdentifier);
}

local void savePlayerData(Player *p)
{
	PointsArenaData *pad = P_ARENA_DATA(p->arena, adkey);
	PointsPlayerData *ppd = PPDATA(p, pdkey);

	mysql->Query(NULL, NULL, 0,
		"UPDATE hs_player_points SET points_global = #, points_1 = #, points_2 = #, points_3 = #, points_4 = #, points_5 = #, points_6 = #, points_7 = #, points_8 = # WHERE player_id = # AND arena = ?",
		ppd->globalPoints,
		ppd->shipPoints[0],
		ppd->shipPoints[1],
		ppd->shipPoints[2],
		ppd->shipPoints[3],
		ppd->shipPoints[4],
		ppd->shipPoints[5],
		ppd->shipPoints[6],
		ppd->shipPoints[7],
		database->getPerPlayerData(p)->id,
		pad->arenaIdentifier);
}

local void playerActionCallback(Player *p, int action, Arena *arena)
{
	if (action == PA_CONNECT)
	{
		//the player is connecting to the server. not arena-specific.
	}
	else if (action == PA_DISCONNECT)
	{
		//the player is disconnecting from the server. not arena-specific.
	}
	else if (action == PA_PREENTERARENA)
	{
		//this is called at the earliest point after a player indicates an
		//intention to enter an arena.
		//you can use this for some questionable stuff, like redirecting
		//the player to a different arena. but in general it's better to
		//use PA_ENTERARENA for general stuff that should happen on
	 	//entering arenas.
	}
	else if (action == PA_ENTERARENA)
	{
		//the player is entering an arena.
		loadDefaultPoints(p);
		loadPlayerData(p);
	}
	else if (action == PA_LEAVEARENA)
	{
		//the player is leaving an arena.

		savePlayerData(p);
	}
	else if (action == PA_ENTERGAME)
	{
		//this is called at some point after the player has sent his first
		//position packet (indicating that he's joined the game, as
		//opposed to still downloading a map).
	}
}

local int periodicStoreTimer(void *param)
{
	Player *p;
	Link *link;

    lm->Log(L_INFO, "<hscore_buysell_points> Storing player data.");
    FOR_EACH_PLAYER(p)
    {
		savePlayerData(p);
	}
    return 1;
}

local helptext_t hspointsHelp =
"Targets: none or player\n"
"Args: none\n"
"Displays your (non-score) points in this arena.\n";

local void hspointsCommand(const char *command, const char *params, Player *p, const Target *target)
{
	Player *t = (target->type == T_PLAYER) ? target->u.p : p;
	PointsArenaData *pad = P_ARENA_DATA(p->arena, adkey);
	PointsPlayerData *ppd = PPDATA(t, pdkey);

	if (pad->useGlobal)
	{
		if(t == p)
		{
			chat->SendMessage(p, "You have %i points in this arena.", ppd->globalPoints);
		}
		else
		{
			chat->SendMessage(p, "Player %s has %i points in this arena.", t->name, ppd->globalPoints);
		}
	}
	else
	{
		chat->SendMessage(p, "+---------+---------+--------+-----------+---------+--------+-----------+--------+");
		chat->SendMessage(p, "| %-7s | %-7s | %-6s | %-9s | %-7s | %-6s | %-9s | %-6s |", shipNames[0], shipNames[1], shipNames[2], shipNames[3], shipNames[4], shipNames[5], shipNames[6], shipNames[7]);
		chat->SendMessage(p, "+---------+---------+--------+-----------+---------+--------+-----------+--------+");
		chat->SendMessage(p, "| %-7i | %-7i | %-6i | %-9i | %-7i | %-6i | %-9i | %-6i |", ppd->shipPoints[0], ppd->shipPoints[1], ppd->shipPoints[2], ppd->shipPoints[3], ppd->shipPoints[4], ppd->shipPoints[5], ppd->shipPoints[6], ppd->shipPoints[7]);
		chat->SendMessage(p, "+---------+---------+--------+-----------+---------+--------+-----------+--------+");
	}
}

local void printAllCategories(Player *p)
{
	LinkedList *categoryList = database->getCategoryList(p->arena);
	Link *link;

	chat->SendMessage(p, "+----------------------------------+------------------------------------------------------------------+");
	chat->SendMessage(p, "| Category Name                    | Category Description                                             |");
	chat->SendMessage(p, "+----------------------------------+------------------------------------------------------------------+");
	chat->SendMessage(p, "| Ships                            | All the ship hulls you can buy in this arena.                    |");

	database->lock();
	for (link = LLGetHead(categoryList); link; link = link->next)
	{
		Category *category = link->data;
		if (category->hidden && capman->HasCapability(p, SEE_HIDDEN_CATEGORIES))
		{
			chat->SendMessage(p, "| (%-30s) | %-64s |", category->name, category->description);
		}
		else if (!category->hidden)
		{
			chat->SendMessage(p, "| %-32s | %-64s |", category->name, category->description);
		}
	}
	database->unlock();

	chat->SendMessage(p, "+----------------------------------+------------------------------------------------------------------+");
}

local void printCategoryItems(Player *p, Category *category) //call with lock held
{
	Link *link;
	Link plink = {NULL, p};
	LinkedList lst = { &plink, &plink };
	char messageType;

	chat->SendMessage(p, "+----------------------------------+");
	chat->SendMessage(p, "| %-32s |", category->name);
	chat->SendMessage(p, "+------------------+-----------+---+--------+-------+----------+-----+----------------------------------+");
	chat->SendMessage(p, "| Item Name        | Buy Price | Sell Price | Exp   | Ships    | Max | Item Description                 |");
	chat->SendMessage(p, "+------------------+-----------+------------+-------+----------+-----+----------------------------------+");

	if (!category->hidden || capman->HasCapability(p, SEE_HIDDEN_CATEGORIES))
	{
		for (link = LLGetHead(&category->itemList); link; link = link->next)
		{
			Item *item = link->data;

			char shipMask[] = "12345678";

			for (int i = 0; i < 8; i++)
			{
				if (!((item->shipsAllowed >> i) & 0x1))
				{
					shipMask[i] = ' ';
				}
			}

			if (getPoints(p) < item->buyPrice)
			{
				messageType =  MSG_SYSOPWARNING;
			}
			else
			{
				if (p->p_ship == SHIP_SPEC || (item->shipsAllowed >> p->p_ship) & 0x1)
				{
					messageType = MSG_ARENA;
				}
				else
				{
					messageType = MSG_SYSOPWARNING;
				}
			}

			chat->SendAnyMessage(&lst, messageType, 0, NULL, "| %-16s | %-9i | %-10i | %-5i | %-8s | %-3i | %-32s |", item->name, item->buyPrice, item->sellPrice, item->expRequired, shipMask, item->max, item->shortDesc);
		}
	}

	chat->SendMessage(p, "+------------------+-----------+------------+-------+----------+-----+----------------------------------+");
}

local void printShipList(Player *p)
{
	Link plink = {NULL, p};
	LinkedList lst = { &plink, &plink };
	char messageType;

	chat->SendMessage(p, "+-----------+-----------+------------+--------+----------------------------------------------------+");
	chat->SendMessage(p, "| Ship Name | Buy Price | Sell Price | Exp    | Ship Description                                   |");
	chat->SendMessage(p, "+-----------+-----------+------------+--------+----------------------------------------------------+");

	for (int i = 0; i < 8; i++)
	{
		/* cfghelp: All:BuyPrice, arena, int, def: 0, mod: hscore_buysell_points
		 * Cost for buying the ship hull. Zero means no purchace needed. */
		int buyPrice = cfg->GetInt(p->arena->cfg, shipNames[i], "BuyPrice", 0);
		/* cfghelp: All:SellPrice, arena, int, def: 0, mod: hscore_buysell_points
		 * Money earned from selling the ship hull. */
		int sellPrice = cfg->GetInt(p->arena->cfg, shipNames[i], "SellPrice", 0);
		/* cfghelp: All:ExpRequired, arena, int, def: 0, mod: hscore_buysell_points
		 * Experience required to buy the ship hull. */
		int expRequired = cfg->GetInt(p->arena->cfg, shipNames[i], "ExpRequired", 0);

		/* cfghelp: All:Description, arena, string, mod: hscore_buysell_points
		 * Text to put in the ship description column. */
		const char *description = cfg->GetStr(p->arena->cfg, shipNames[i], "Description");

		if (description == NULL)
		{
			description = "<No description available>";
		}

		if (buyPrice == 0)
		{
			continue; //dont list the ship unless it can be bought.
		}

		if(getPoints(p) >= buyPrice)
		{
			messageType = MSG_ARENA;
		}
		else
		{
			messageType = MSG_SYSOPWARNING;
		}

		chat->SendAnyMessage(&lst, messageType, 0, NULL, "| %-9s | %-9i | %-10i | %-6i | %-50s |", shipNames[i], buyPrice, sellPrice, expRequired, description);
	}


	chat->SendMessage(p, "+-----------+-----------+------------+--------+----------------------------------------------------+");
}

local void buyItem(Player *p, Item *item, int count, int ship)
{
	if ((item->shipsAllowed >> ship) & 0x1)
	{
		if (item->buyPrice)
		{
			if (getPoints(p) >= item->buyPrice * count)
			{
				if (item->max == 0 || items->getItemCount(p, item, ship) + count <= item->max)
				{
					Ihscorestoreman *storeman = mm->GetInterface(I_HSCORE_STOREMAN, p->arena);
					int storemanOk;
					LinkedList list;
					LLInit(&list);

					if (!storeman)
					{
						storemanOk = 1;
					}
					else
					{
						storemanOk = storeman->canSellItem(p, item);
						if (!storemanOk)
						{
							storeman->getStoreList(p, item, &list); //fills the linked list with stores that buy the item
						}
					}
					mm->ReleaseInterface(storeman);

					if (storemanOk)
					{
						Link *link;
						database->lock();
						for (link = LLGetHead(&item->itemTypeEntries); link; link = link->next)
						{
							ItemTypeEntry *entry = link->data;

							if (items->getFreeItemTypeSpotsNoLock(p, entry->itemType, ship) - (entry->delta * count) < 0) //have no free spots
							{
								chat->SendMessage(p, "You do not have enough free %s spots.", entry->itemType->name);
								database->unlock();
								return;
							}
						}
						database->unlock();

						items->addItem(p, item, ship, count);

						givePoints(p, -item->buyPrice * count);

						items->triggerEventOnItem(p, item, ship, "buy");

						for (int i = 0; i < count; i++)
						{
							items->triggerEventOnItem(p, item, ship, "add");
						}

						chat->SendMessage(p, "You purchased %i of item %s for %i points.", count, item->name, item->buyPrice * count);
					}
					else
					{
						int storeCount = LLCount(&list);
						if (storeCount == 0)
						{
							chat->SendMessage(p, "You cannot buy item %s at your current location. No known stores sell it!", item->name);
						}
						else if (storeCount == 1)
						{
							Store *store = LLGetHead(&list)->data;
							chat->SendMessage(p, "You cannot buy item %s here. Go to %s to buy it!", item->name, store->name);
						}
						else
						{
							Link *link;
							chat->SendMessage(p, "You cannot buy item %s here. The following stores sell it:", item->name);
							for (link = LLGetHead(&list); link; link = link->next)
							{
								Store *store = link->data;

								chat->SendMessage(p, "%s", store->name);
							}
						}

						LLEmpty(&list);
					}
				}
				else
				{
					chat->SendMessage(p, "You may only have %i of item %s on your ship.", item->max, item->name);
				}

			}
			else
			{
				chat->SendMessage(p, "You do not have enough points to buy item %s. You need %i more.", item->name, item->buyPrice * count - getPoints(p));
			}
		}
		else
		{
			chat->SendMessage(p, "Item %s is not for sale.", item->name);
		}
	}
	else
	{
		chat->SendMessage(p, "Item %s is not allowed on a %s.", item->name, shipNames[ship]);
	}
}

local void sellItem(Player *p, Item *item, int count, int ship)
{
	if (item->sellPrice)
	{
		if (items->getItemCount(p, item, ship) >= count)
		{
			Ihscorestoreman *storeman = mm->GetInterface(I_HSCORE_STOREMAN, p->arena);
			int storemanOk;
			LinkedList list;
			LLInit(&list);

			if (!storeman)
			{
				storemanOk = 1;
			}
			else
			{
				storemanOk = storeman->canSellItem(p, item);
				if (!storemanOk)
				{
					storeman->getStoreList(p, item, &list); //fills the linked list with stores that buy the item
				}
			}
			mm->ReleaseInterface(storeman);

			if (storemanOk)
			{
				Link *link;
				database->lock();
				for (link = LLGetHead(&item->itemTypeEntries); link; link = link->next)
				{
					ItemTypeEntry *entry = link->data;

					if (items->getFreeItemTypeSpotsNoLock(p, entry->itemType, ship) + (entry->delta * count) < 0) //have no free spots
					{
						chat->SendMessage(p, "You do not have enough free %s spots.", entry->itemType->name);
						database->unlock();
						return;
					}
				}
				database->unlock();

				//trigger before it's sold!
				items->triggerEventOnItem(p, item, ship, "sell");

				items->addItem(p, item, ship, -count); //change the count BEFORE the "del" event

				for (int i = 0; i < count; i++)
				{
					items->triggerEventOnItem(p, item, ship, "del");
				}

				givePoints(p, item->sellPrice * count);

				chat->SendMessage(p, "You sold %i of item %s for %i points.", count, item->name, item->sellPrice * count);
			}
			else
			{
				int storeCount = LLCount(&list);
				if (storeCount == 0)
				{
					chat->SendMessage(p, "You cannot sell item %s at your current location. No known stores buy it!", item->name);
				}
				else if (storeCount == 1)
				{
					Store *store = LLGetHead(&list)->data;
					chat->SendMessage(p, "You cannot sell item %s here. Go to %s to sell it!", item->name, store->name);
				}
				else
				{
					Link *link;
					chat->SendMessage(p, "You cannot sell item %s here. The following stores buy it:", item->name);
					for (link = LLGetHead(&list); link; link = link->next)
					{
						Store *store = link->data;

						chat->SendMessage(p, "%s", store->name);
					}
				}

				LLEmpty(&list);
			}
		}
		else
		{
			if (count == 1)
			{
				chat->SendMessage(p, "You do not have any of item %s to sell", item->name);
			}
			else
			{
				chat->SendMessage(p, "You do not have that many of item %s to sell", item->name);
			}
		}
	}
	else
	{
		chat->SendMessage(p, "Item %s cannot be sold.", item->name);
	}
}

local void buyShip(Player *p, int ship)
{
	int buyPrice = cfg->GetInt(p->arena->cfg, shipNames[ship], "BuyPrice", 0);

	if (buyPrice != 0)
	{
		if (database->areShipsLoaded(p))
		{
			PerPlayerData *playerData = database->getPerPlayerData(p);
			if (playerData->hull[ship] == NULL)
			{
				if (getPoints(p) >= buyPrice)
				{
					database->addShip(p, ship);

					//database will call the ship_added callback for init items

					givePoints(p, -buyPrice);

					chat->SendMessage(p, "You purchased a %s for %i points.", shipNames[ship], buyPrice);
				}
				else
				{
					chat->SendMessage(p, "You do not have enough points to buy a %s. You need %i more.", shipNames[ship], buyPrice - getPoints(p));
				}
			}
			else
			{
				chat->SendMessage(p, "You already own a %s.", shipNames[ship]);
			}
		}
		else
		{
			chat->SendMessage(p, "Your ships are not loaded.");
		}
	}
	else
	{
		chat->SendMessage(p, "%ss are not avalible for sale in this arena.", shipNames[ship]);
	}
}

local void sellShip(Player *p, int ship, int force)
{
	int sellPrice = cfg->GetInt(p->arena->cfg, shipNames[ship], "SellPrice", 0);

	if (database->areShipsLoaded(p))
	{
		PerPlayerData *playerData = database->getPerPlayerData(p);
		if (playerData->hull[ship] != NULL)
		{
			if (force || !items->hasItemsLeftOnShip(p, ship))
			{
				database->removeShip(p, ship);

				givePoints(p, sellPrice);

				chat->SendMessage(p, "You sold your %s for %i points.", shipNames[ship], sellPrice);
			}
			else
			{
				chat->SendMessage(p, "Your ship still have items on it. Use ?sell -f %s to sell anyway", shipNames[ship]);
			}
		}
		else
		{
			chat->SendMessage(p, "You do not own a %s.", shipNames[ship]);
		}
	}
	else
	{
		chat->SendMessage(p, "Your ships are not loaded.");
	}
}

local helptext_t buyHelp =
"Targets: none\n"
"Args: none or <item> or <category> or <ship>\n"
"If there is no arugment, this command will display a list of this arena's buy categories.\n"
"If the argument is a category, this command will display all the items in that category.\n"
"If the argument is an item, this command will attemt to buy it for the buy price.\n";

local void buyCommand(const char *command, const char *params, Player *p, const Target *target)
{
	LinkedList *categoryList = database->getCategoryList(p->arena);
	Link *link;

	int count = 1;
	const char *newParams;

	char *next; //for strtol

	while (params != NULL) //get the flags
	{
		if (*params == '-')
		{
			params++;
			if (*params == '\0')
			{
				newParams = params;
				break;
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
					chat->SendMessage(p, "Buy: invalid usage.");
					return;
				}

				count = strtol(params, &next, 0);

				if (next == params)
				{
					chat->SendMessage(p, "Buy: bad count.");
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
			newParams = params;
			break;
		}
		else
		{
			newParams = params;
			break;
		}
	}

	//finished parsing
	if (count < 1)
	{
		chat->SendMessage(p, "Nice try.");
		return;
	}

	if (strcasecmp(newParams, "") == 0) //no params
	{
		printAllCategories(p);
	}
	else //has params
	{
		if (strcasecmp(newParams, "ships") == 0) //print ship list
		{
			printShipList(p);
		}
		else
		{
			int matches;
			Category *category;

			//check if they're asking for a ship
			for (int i = 0; i < 8; i++)
			{
				if (strcasecmp(newParams, shipNames[i]) == 0)
				{
					buyShip(p, i);
					return;
				}
			}

			//check if they're asking for a category
			database->lock();
			matches = 0;
			for (link = LLGetHead(categoryList); link; link = link->next)
			{
				Category *c = link->data;

				if (strncasecmp(c->name, newParams, strlen(newParams)) == 0)
				{
					category = c;
					matches++;
				}
			}

			if (matches == 1)
			{
				printCategoryItems(p, category);

				database->unlock();
				return;
			}
			else if (matches > 1)
			{
				chat->SendMessage(p, "Too many partial matches! Try typing more of the name!");

				database->unlock();
				return;
			}
			database->unlock();

			//not a category. check for an item
			Item *item = items->getItemByPartialName(newParams, p->arena);
			if (item != NULL)
			{
				if (p->p_ship != SHIP_SPEC)
				{
					PerPlayerData *playerData = database->getPerPlayerData(p);
					if (playerData->hull[p->p_ship] != NULL)
					{
						//check - counts
						buyItem(p, item, count, p->p_ship);
					}
					else
					{
						chat->SendMessage(p, "No items can be loaded onto a %s in this arena.", shipNames[p->p_ship]);
					}
				}
				else
				{
					chat->SendMessage(p, "You cannot buy or sell items in spec.");
				}

				return;
			}

			//neither an item nor a ship nor a category
			chat->SendMessage(p, "No item %s in this arena.", params);
		}
	}
}

local helptext_t sellHelp =
"Targets: none\n"
"Args: <item> or [{-f}] <ship>\n"
"Removes the item from your ship and refunds you the item's sell price.\n"
"If selling a ship, the {-f} will force the ship to be sold, even if the ship\n"
"is not empty, destroying all items onboard.\n";

local void sellCommand(const char *command, const char *params, Player *p, const Target *target)
{
	int force = 0;
	int count = 1;
	const char *newParams;

	char *next; //for strtol

	while (params != NULL) //get the flags
	{
		if (*params == '-')
		{
			params++;
			if (*params == '\0')
			{
				newParams = params;
				break;
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
					chat->SendMessage(p, "Sell: invalid usage.");
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
					chat->SendMessage(p, "Sell: invalid usage.");
					return;
				}

				count = strtol(params, &next, 0);

				if (next == params)
				{
					chat->SendMessage(p, "Sell: bad count.");
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
			newParams = params;
			break;
		}
		else
		{
			newParams = params;
			break;
		}
	}

	//finished parsing
	if (count < 1)
	{
		chat->SendMessage(p, "Nice try.");
		return;
	}

	if (strcasecmp(newParams, "") == 0) //no params
	{
		chat->SendMessage(p, "Please use ?buy to find the item you wish to sell");
	}
	else //has params
	{
		//check if they're asking for a ship
		for (int i = 0; i < 8; i++)
		{
			if (strcasecmp(newParams, shipNames[i]) == 0)
			{
				if (i != p->p_ship)
				{
					sellShip(p, i, force);
				}
				else
				{
					chat->SendMessage(p, "You cannot sell the ship you are using. Switch to spec first.");
				}

				return;
			}
		}

		//check for an item
		Item *item = items->getItemByName(newParams, p->arena);
		if (item != NULL)
		{
			if (p->p_ship != SHIP_SPEC)
			{
				PerPlayerData *playerData = database->getPerPlayerData(p);
				if (playerData->hull[p->p_ship] != NULL)
				{
					//check - counts
					sellItem(p, item, count, p->p_ship);
				}
				else
				{
					chat->SendMessage(p, "No items can be loaded onto a %s in this arena.", shipNames[p->p_ship]);
				}
			}
			else
			{
				chat->SendMessage(p, "You cannot buy or sell items in spec.");
			}

			return;
		}

		//not a ship nor an item
		chat->SendMessage(p, "No item %s in this arena.", params);
	}
}

local void shipAddedCallback(Player *p, int ship)
{
	/* cfghelp: All:InitItem, arena, string, mod: hscore_buysell_points
	 * Comma seperated list of items to add when the ship is bought.
	 * There should be no spaces between the commas ('item1,item2,...').*/
	const char *initItem = cfg->GetStr(p->arena->cfg, shipNames[ship], "InitItems");

	if (initItem != NULL) //only bother if there are items to add
	{
		const char *tmp = NULL;
		char word[64];
		while (strsplit(initItem, ",", word, sizeof(word), &tmp))
		{
			//items should be in the format "item name" or "item name:count"
			char *colonLoc = strchr(word, ':');
			if (colonLoc == NULL)
			{
				//no count included
				//word should be just "item name"
				Item *item = items->getItemByName(word, p->arena);
				if (item != NULL)
				{
					items->addItem(p, item, ship, 1);
				}
				else
				{
					lm->LogP(L_ERROR, "hscore_buysell_points", p, "bad item %s", word);
				}
			}
			else
			{
				//null terminate the item name
				*colonLoc = '\0';

				//make sure a count follows the colon
				colonLoc++;
				if (*colonLoc != '\0')
				{
					int count = atoi(colonLoc);
					if (count > 0)
					{
						Item *item = items->getItemByName(word, p->arena);
						if (item != NULL)
						{
							items->addItem(p, item, ship, count);
						}
						else
						{
							lm->LogP(L_ERROR, "hscore_buysell_points", p, "bad item %s", word);
						}
					}
					else
					{
						lm->LogP(L_ERROR, "hscore_buysell_points", p, "initial count of %d for %s", count, word);
					}
				}
				else
				{
					lm->LogP(L_ERROR, "hscore_buysell_points", p, "colon on item %s not followed by a string", word);
				}
			}
		}
	}
}

EXPORT const char info_hscore_buysell_points[] = "v1.0 Dr Brain/D1st0rt <d1st0rter@gmail.com>";

EXPORT int MM_hscore_buysell_points(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);
		capman = mm->GetInterface(I_CAPMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		mysql = mm->GetInterface(I_HSCORE_MYSQL, ALLARENAS);
		items = mm->GetInterface(I_HSCORE_ITEMS, ALLARENAS);
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);


		if (!lm || !chat || !cfg || !cmd || !ml || !capman || !pd || !aman || !mysql || !items || !database || pdkey == -1 || adkey == -1)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(ml);
			mm->ReleaseInterface(capman);
			mm->ReleaseInterface(pd);
			mm->ReleaseInterface(aman);
			mm->ReleaseInterface(mysql);
			mm->ReleaseInterface(items);
			mm->ReleaseInterface(database);

			return MM_FAIL;
		}

		pdkey = pd->AllocatePlayerData(sizeof(PointsPlayerData));
		adkey = aman->AllocateArenaData(sizeof(PointsArenaData));
		ml->SetTimer(periodicStoreTimer, 30000, 30000, NULL, NULL);

		mm->RegCallback(CB_PLAYERACTION, playerActionCallback, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		pd->FreePlayerData(pdkey);
		aman->FreeArenaData(adkey);
		ml->ClearTimer(periodicStoreTimer, NULL);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(ml);
		mm->ReleaseInterface(capman);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(mysql);
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		cmd->AddCommand("buy", buyCommand, arena, buyHelp);
		cmd->AddCommand("sell", sellCommand, arena, sellHelp);
		cmd->AddCommand("hspoints", hspointsCommand, arena, hspointsHelp);

		mm->RegCallback(CB_SHIP_ADDED, shipAddedCallback, arena);

		loadArenaData(arena);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		cmd->RemoveCommand("buy", buyCommand, arena);
		cmd->RemoveCommand("sell", sellCommand, arena);
		cmd->RemoveCommand("hspoints", hspointsCommand, arena);

		mm->UnregCallback(CB_SHIP_ADDED, shipAddedCallback, arena);

		return MM_OK;
	}
	return MM_FAIL;
}
