#include <string.h>
#include <stdlib.h>

#include "../asss.h"
#include "hscore_database.h"
#include "hscore_mysql.h"


//local structs
typedef struct HSArenaData
{
	LinkedList *store_list;
	LinkedList *category_list;
} HSArenaData;

//interface prototypes
local HSData * GetHSData(Player *p);
local int Loaded(Player *p);
local LinkedList * GetItemList();
local LinkedList * GetStoreList(Arena *arena);
local LinkedList * GetCategoryList(Arena *arena);
local void AddShip(Player *p, int ship);
local void RemoveShip(Player *p, int ship);

//other prototypes
local HSArenaData * GetHSArenaData(Arena *arena);
local void dbcb_loadplayer(int status, db_res *res, void *clos);
local void dbcb_loadships(int status, db_res *res, void *clos);
local void dbcb_loadshipitems(int status, db_res *res, void *clos);
local void dbcb_loadcategories(int status, db_res *res, void *clos);
local void dbcb_loadstores(int status, db_res *res, void *clos);
local void dbcb_loadproperties(int status, db_res *res, void *clos);
local void dbcb_loaditems(int status, db_res *res, void *clos);
local void dbcb_loadcategoryitems(int status, db_res *res, void *clos);
local void dbcb_loadstoreitems(int status, db_res *res, void *clos);
local void ClearItemListEnum(const void *ptr);
local void ClearStoreListEnum(const void *ptr);
local void ClearCategoryListEnum(const void *ptr);
local void LoadPlayer(Player *p);
local void LoadShips(Player *p);
local void LoadArenaData(Arena *a);
local void LoadItemPropertyList();
local void LoadItemList();
local void StorePlayer(Player *p);
local void StoreShips(Player *p);
local void UnloadPlayer(Player *p);
local void UnloadShips(Player *p);
local void UnloadArenaData(Arena *a);
local void UnloadItemList();
local void StoreAll();
local void UnloadAll();
local void NewPlayer(Player *p, int isnew);
local void PlayerAction(Player *p, int action, Arena *arena);
local void ArenaAction(Arena *a, int action);

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihsmysql *mysql;
local Iarenaman *aman;
local Iplayerdata *pd;
local Imainloop *ml;

//keys
local int hs_data_key;
local int hs_arena_data_key;

//lists
local LinkedList *item_list;
local char items_loaded = 0;


/*
+-------------------------+
|                         |
|   Interface Functions   |
|                         |
+-------------------------+
*/

local HSData * GetHSData(Player *p)
{
	return PPDATA(p, hs_data_key);
}

local int Loaded(Player *p)
{
	HSData *data = GetHSData(p);

	return (data->loading_status == LOADED);
}

local LinkedList * GetStoreList(Arena *arena)
{
	HSArenaData *adata = GetHSArenaData(arena);

	return adata->store_list;
}

local LinkedList * GetCategoryList(Arena *arena)
{
	HSArenaData *adata = GetHSArenaData(arena);

	return adata->category_list;
}

local LinkedList * GetItemList()
{
	return item_list;
}

local void AddShip(Player *p, int ship)
{
	HSData *data = GetHSData(p);

	if (ship >= 8 || ship < 0)
	{
		lm->LogP(L_WARN, "hscore_database", p, "Ship number out of bounds (%i) in AddShip.", ship);
		return;
	}

	if (!Loaded(p))
	{
		lm->LogP(L_WARN, "hscore_database", p, "Asked to add ship to unloaded player.");
		return;
	}

	if (data->hulls[ship])
	{
		lm->LogP(L_WARN, "hscore_database", p, "Asked to add ship when ship hull already exists.");
		return;
	}

	if (data->hull_status[ship] != HULL_NO_HULL)
	{
		lm->LogP(L_WARN, "hscore_database", p, "Asked to add ship when ship hull status was not HULL_NO_HULL.");
		return;
	}

	mysql->Query(NULL, NULL, 0, "INSERT INTO hs_player_ships VALUES (NULL, #, #, ?)", data->id, ship, p->arena->basename);
	mysql->Query(dbcb_loadships, p, 1, "SELECT id,ship FROM hs_player_ships WHERE player_id=# AND arena=? AND ship=#", data->id, p->arena->basename, ship);
}

local void RemoveShip(Player *p, int ship)
{
	HSData *data = GetHSData(p);

	if (ship >= 8 || ship < 0)
	{
		lm->LogP(L_WARN, "hscore_database", p, "Ship number out of bounds (%i) in RemoveShip.", ship);
		return;
	}

	if (!Loaded(p))
	{
		lm->LogP(L_WARN, "hscore_database", p, "Asked to remove ship from unloaded player.");
		return;
	}

	if (!data->hulls[ship])
	{
		lm->LogP(L_WARN, "hscore_database", p, "Asked to remove ship when ship hull doesn't exist.");
		return;
	}

	if (data->hull_status[ship] == HULL_NO_HULL)
	{
		lm->LogP(L_WARN, "hscore_database", p, "Asked to remove ship when ship hull status was HULL_NO_HULL.");
		return;
	}

	mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ships WHERE id = #", data->hulls[ship]->id);
	mysql->Query(NULL, NULL, 0, "DELETE FROM hs_ship_items WHERE ship_id = #", data->hulls[ship]->id);

	HullData *hull = data->hulls[ship];

	data->hull_status[ship] = HULL_NO_HULL;

	Link *link = LLGetHead(hull->inventory_list);
	while (link)
	{
		ShipItem *ship_item = link->data;

		afree(ship_item);

		link = link->next;
	}

	LLFree(hull->inventory_list);

	afree(hull);
	data->hulls[ship] = 0;
}

/*
+-----------------------+
|                       |
|   Wrapper Functions   |
|                       |
+-----------------------+
*/

local HSArenaData * GetHSArenaData(Arena *arena)
{
	return P_ARENA_DATA(arena, hs_arena_data_key);
}


/*
+----------------------------+
|                            |
|   MySQL Return Functions   |
|                            |
+----------------------------+
*/

local void dbcb_loadplayer(int status, db_res *res, void *clos)
{
    Player *p = (Player*)clos;

	if (status != 0 || res == NULL)
	{
		lm->LogP(L_WARN, "hscore_database", p, "Unexpected database error during player data load.");
		return;
	}

	if (mysql->GetRowCount(res) == 0)
	{
	    int initial_money = cfg->GetInt(GLOBAL, "hyperspace", "initialmoney", 0);
	    int initial_exp = cfg->GetInt(GLOBAL, "hyperspace", "initialexp", 0);
	    mysql->Query(NULL, NULL, 0, "INSERT INTO hs_players VALUES (NULL, ?, #, #, 0, 0, 0, 0, 0, 0)", p->name, initial_money, initial_exp);
        mysql->Query(dbcb_loadplayer, p, 1, "SELECT id,money,exp,money_grant,money_give,money_kill,money_ball,money_flag,money_buysell FROM hs_players WHERE name=?", p->name);
	    return;
	}
	else
	{
	    HSData *data = GetHSData(p);

	    db_row *row = mysql->GetRow(res);

		data->id = atoi(mysql->GetField(row, 0));
        data->money = atoi(mysql->GetField(row, 1));
		data->exp = atoi(mysql->GetField(row, 2));

		data->money_type[MONEY_TYPE_GRANT] = atoi(mysql->GetField(row, 3));
		data->money_type[MONEY_TYPE_GIVE] = atoi(mysql->GetField(row, 4));
		data->money_type[MONEY_TYPE_KILL] = atoi(mysql->GetField(row, 5));
		data->money_type[MONEY_TYPE_BALL] = atoi(mysql->GetField(row, 6));
		data->money_type[MONEY_TYPE_FLAG] = atoi(mysql->GetField(row, 7));
		data->money_type[MONEY_TYPE_BUYSELL] = atoi(mysql->GetField(row, 8));

		data->loading_status = LOADED;
	}
}

local void dbcb_loadships(int status, db_res *res, void *clos)
{
	int i;
	Player *p = (Player*)clos;
    HSData *data = GetHSData(p);
	db_row *row;

	if (status != 0 || res == NULL)
	{
		lm->LogP(L_WARN, "hscore_database", p, "Unexpected database error during ship load.");
		return;
	}

	for (i = 0; i < 8; i++)
	{
		if (data->hull_status[i] != HULL_LOADED)
		{
			data->hull_status[i] = HULL_NO_HULL;
		}
	}

	if (mysql->GetRowCount(res) == 0)
	{
		//Not a big deal. Arena probably doesnt use ships
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
	    HullData *hull = amalloc(sizeof(*hull));

	    hull->inventory_list = LLAlloc();

	    hull->id = atoi(mysql->GetField(row, 0));					//id
		int ship = atoi(mysql->GetField(row, 1));					//ship

		if (ship >= 8 || ship < 0)
		{
			lm->LogP(L_WARN, "hscore_database", p, "Ship number out of bounds (%i).", ship);
			continue;
		}

        //add the hull to the list
        data->hulls[ship] = hull;
        data->hull_status[ship] = HULL_LOADED;
	}

	mysql->Query(dbcb_loadshipitems, p, 1, "SELECT hs_ship_items.id, hs_ship_items.item_id, hs_ship_items.amount, hs_ship_items.data, hs_player_ships.ship FROM hs_ship_items, hs_player_ships, hs_items WHERE hs_ship_items.item_id = hs_items.id AND hs_ship_items.ship_id = hs_player_ships.id AND hs_player_ships.player_id = # AND hs_player_ships.arena = ?", data->id, p->arena->basename);
}

local void dbcb_loadshipitems(int status, db_res *res, void *clos)
{
	db_row *row;
	Player *p = (Player*)clos;
    HSData *data = GetHSData(p);

	if (status != 0 || res == NULL)
	{
		lm->LogP(L_WARN, "hscore_database", p, "Unexpected database error during ship item load.");
		return;
	}

	if (mysql->GetRowCount(res) == 0)
	{
	    //player has no items. nothing to worry about.
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
		Link *link;
		Item *item;

	    ShipItem *shipitem = amalloc(sizeof(*shipitem));

		shipitem->id = atoi(mysql->GetField(row, 0));				//id
		int item_id = atoi(mysql->GetField(row, 1));				//item_id
		shipitem->amount = atoi(mysql->GetField(row, 2));			//amount
	    shipitem->data = atoi(mysql->GetField(row, 3));				//data
	    int ship = atoi(mysql->GetField(row, 4));					//ship

		if (ship >= 8 || ship < 0)
		{
			lm->LogP(L_WARN, "hscore_database", p, "Ship item for item %i was present for out of bounds (%i) hull.", item_id, ship);
			afree(shipitem);
			break;
		}

		if (data->hull_status[ship] != HULL_LOADED)
		{
			lm->LogP(L_WARN, "hscore_database", p, "Ship item for item %i was present for unowned hull.", item_id);
			afree(shipitem);
			break;
		}

		link = LLGetHead(item_list);
        while(link)
        {
			item = link->data;

			if (item->id == item_id)
			{
				LLAdd(data->hulls[ship]->inventory_list, shipitem);
				break;
			}

			link = link->next;

			if (!link)
			{
				lm->LogP(L_WARN, "hscore_database", p, "Ship item for item %i was unable to find parent.", item_id);
				afree(shipitem);
				break;
			}
		}
	}
}

local void dbcb_loadcategories(int status, db_res *res, void *clos)
{
	db_row *row;

	Arena *a = (Arena*)clos;
	HSArenaData *adata = GetHSArenaData(a);

	if (status != 0 || res == NULL)
	{
		lm->LogA(L_WARN, "hscore_database", a, "Unexpected database error during category load.");
		return;
	}

	if (mysql->GetRowCount(res) == 0)
	{
		//Not a big deal. Arena probably doesnt use buying
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
	    Category *cat = amalloc(sizeof(*cat));

	    cat->item_list = LLAlloc();

	    cat->id = atoi(mysql->GetField(row, 0));					//id
        astrncpy(cat->name, mysql->GetField(row, 1), 20);			//name
        astrncpy(cat->description, mysql->GetField(row, 2), 75);	//description

        //add the item to the list
        LLAdd(adata->category_list, cat);
	}

	mysql->Query(dbcb_loadcategoryitems, a, 1, "SELECT hs_category_items.item_id, hs_category_items.category_id FROM hs_category_items, hs_categories, hs_items WHERE hs_category_items.item_id = hs_items.id AND hs_category_items.category_id = hs_categories.id AND hs_categories.arena = ?", a->basename);
}

local void dbcb_loadstores(int status, db_res *res, void *clos)
{
	db_row *row;

	Arena *a = (Arena*)clos;
	HSArenaData *adata = GetHSArenaData(a);

	if (status != 0 || res == NULL)
	{
		lm->LogA(L_WARN, "hscore_database", a, "Unexpected database error during store load.");
		return;
	}

	if (mysql->GetRowCount(res) == 0)
	{
		//Not a big deal. Arena probably doesnt use buying
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
	    Store *store = amalloc(sizeof(*store));

	    store->item_list = LLAlloc();

	    store->id = atoi(mysql->GetField(row, 0));					//id
        astrncpy(store->name, mysql->GetField(row, 1), 20);			//name
        astrncpy(store->description, mysql->GetField(row, 2), 255);	//description
        astrncpy(store->region, mysql->GetField(row, 3), 20);		//region

        //add the item to the list
        LLAdd(adata->store_list, store);
	}

	mysql->Query(dbcb_loadstoreitems, a, 1, "SELECT hs_store_items.item_id, hs_store_items.store_id FROM hs_store_items, hs_stores, hs_items WHERE hs_store_items.item_id = hs_items.id AND hs_store_items.store_id = hs_stores.id AND hs_stores.arena = ?", a->basename);
}

local void dbcb_loadproperties(int status, db_res *res, void *clos)
{
	int results;
	int item_prop_count = 0;
	db_row *row;

	if (status != 0 || res == NULL)
	{
		lm->Log(L_WARN, "<hscore_database> Unexpected database error during item property load.");
		return;
	}

	results = mysql->GetRowCount(res);

	if (results == 0)
	{
	    lm->Log(L_WARN, "<hscore_database> No item properties returned from MySQL query.");
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
		Link *link;
		Item *item;

	    Property *prop = amalloc(sizeof(*prop));

		int item_id = atoi(mysql->GetField(row, 0));				//item_id
		astrncpy(prop->name, mysql->GetField(row, 1), 20);			//name
	    prop->setting = atoi(mysql->GetField(row, 2));				//setting

		link = LLGetHead(item_list);
        while(link)
        {
			item = link->data;

			if (item->id == item_id)
			{
				LLAdd(item->property_list, prop);
				item_prop_count++;
				break;
			}

			link = link->next;

			if (!link)
			{
				lm->Log(L_WARN, "<hscore_database> Property for item %i was unable to find parent.", item_id);
				afree(prop);
				break;
			}
		}
	}

	lm->Log(L_DRIVEL, "<hscore_database> %i item properties were loaded from MySQL.", item_prop_count);

	items_loaded = 1;
}

local void dbcb_loaditems(int status, db_res *res, void *clos)
{
	int results;
	int item_count = 0;
	db_row *row;

	if (status != 0 || res == NULL)
	{
		lm->Log(L_WARN, "<hscore_database> Unexpected database error during item load.");
		return;
	}

	results = mysql->GetRowCount(res);

	if (results == 0)
	{
	    lm->Log(L_WARN, "<hscore_database> No items returned from MySQL query.");
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
	    Item *item = amalloc(sizeof(*item));
	    item->property_list = LLAlloc();

	    item->id = atoi(mysql->GetField(row, 0));					//id
        astrncpy(item->name, mysql->GetField(row, 1), 20);			//name
        astrncpy(item->short_description, mysql->GetField(row, 2), 35);	//short_description
        astrncpy(item->description, mysql->GetField(row, 3), 255);	//description
        item->buy_price = atoi(mysql->GetField(row, 4));			//buy_price
        item->sell_price = atoi(mysql->GetField(row, 5));			//sell_price
        item->item_type = atoi(mysql->GetField(row, 6));			//item_type
        item->item_max = atoi(mysql->GetField(row, 7));				//item_max
        item->ship_mask = atoi(mysql->GetField(row, 8));			//ship_mask
        item->exp_required = atoi(mysql->GetField(row, 9));			//exp_required
        item->frequent_changes = atoi(mysql->GetField(row, 10)); 	//frequent_changes

        //add the item to the list
        LLAdd(item_list, item);

        item_count++;
	}

	lm->Log(L_DRIVEL, "<hscore_database> %i items were loaded from MySQL.", item_count);
	LoadItemPropertyList(); //now that all the items are in, load their properties.
}

local void dbcb_loadcategoryitems(int status, db_res *res, void *clos)
{
	int category_item_count = 0;
	db_row *row;

	Arena *a = (Arena*)clos;
	HSArenaData *adata = GetHSArenaData(a);

	if (status != 0 || res == NULL)
	{
		lm->LogA(L_WARN, "hscore_database", a, "Unexpected database error during category item load.");
		return;
	}

	if (mysql->GetRowCount(res) == 0)
	{
	    lm->LogA(L_WARN, "hscore_database", a, "No category items returned from MySQL query.");
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
		Link *link;
		Item *item;
		Category *cat;

		int item_id = atoi(mysql->GetField(row, 0));				//item_id
	    int category_id = atoi(mysql->GetField(row, 1));			//category_id

		// get the Item
		link = LLGetHead(item_list);
        while(link)
        {
			item = link->data;

			if (item->id == item_id)
			{
				break;
			}

			item = 0;

			link = link->next;
		}

		if (!item) break;

		//put the item into the Category
		link = LLGetHead(adata->category_list);
		while(link)
		{
			cat = link->data;

			if (cat->id == category_id)
			{
				LLAdd(cat->item_list, item);
				category_item_count++;
				break;
			}

			link = link->next;
		}
	}

	lm->LogA(L_DRIVEL, "hscore_database", a, "%i category items were loaded from MySQL.", category_item_count);
}

local void dbcb_loadstoreitems(int status, db_res *res, void *clos)
{
	int store_item_count = 0;
	db_row *row;

	Arena *a = (Arena*)clos;
	HSArenaData *adata = GetHSArenaData(a);


	if (status != 0 || res == NULL)
	{
		lm->LogA(L_WARN, "hscore_database", a, "Unexpected database error during store item load.");
		return;
	}

	if (mysql->GetRowCount(res) == 0)
	{
	    lm->LogA(L_WARN, "hscore_database", a, "No store items returned from MySQL query.");
	    return;
	}

	while ((row = mysql->GetRow(res)))
	{
		Link *link;
		Item *item;
		Store *store;

		int item_id = atoi(mysql->GetField(row, 0));				//item_id
	    int store_id = atoi(mysql->GetField(row, 1));				//store_id

		// get the Item
		link = LLGetHead(item_list);
        while(link)
        {
			item = link->data;

			if (item->id == item_id)
			{
				break;
			}

			item = 0;

			link = link->next;
		}

		if (!item) break;

		//put the item into the Store
		link = LLGetHead(adata->store_list);
		while(link)
		{
			store = link->data;

			if (store->id == store_id)
			{
				LLAdd(store->item_list, item);
				store_item_count++;
				break;
			}

			link = link->next;
		}
	}

	lm->LogA(L_DRIVEL, "hscore_database", a, "%i store items were loaded from MySQL.", store_item_count);
}

/*
+--------------------+
|                    |
|   Enum Functions   |
|                    |
+--------------------+
*/

local void ClearItemListEnum(const void *ptr)
{
	const Item *item = ptr;

	lm->Log(L_DRIVEL, "<hscore_database> Item '%s' unloaded.", item->name);

	LLEnum(item->property_list, afree); //delete all properties
	LLFree(item->property_list);
	afree(item);
}

local void ClearStoreListEnum(const void *ptr)
{
	const Store *store = ptr;

	lm->Log(L_DRIVEL, "<hscore_database> Store '%s' unloaded.", store->name);

	LLFree(store->item_list);
	afree(store);
}

local void ClearCategoryListEnum(const void *ptr)
{
	const Category *cat = ptr;

	lm->Log(L_DRIVEL, "<hscore_database> Category '%s' unloaded.", cat->name);

	LLFree(cat->item_list);
	afree(cat);
}

/*
+--------------------+
|                    |
|   Load Functions   |
|                    |
+--------------------+
*/

local void LoadPlayer(Player *p) //gets data from MySQL
{
	Player *p2;
	Link *link;
	pd->Lock();
	FOR_EACH_PLAYER(p2)
		if (p != p2 && strcasecmp(p->name, p2->name) == 0)
		{
			lm->LogP(L_MALICIOUS, "hscore_database", p, "Player %s already logged on. Data loading aborted.", p->name);
			pd->Unlock();
			return;
		}
	pd->Unlock();

	mysql->Query(dbcb_loadplayer, p, 1, "SELECT id,money,exp,money_grant,money_give,money_kill,money_ball,money_flag,money_buysell FROM hs_players WHERE name=?", p->name);
}

local void LoadShips(Player *p)
{
	HSData *data = GetHSData(p);
	mysql->Query(dbcb_loadships, p, 1, "SELECT id,ship FROM hs_player_ships WHERE player_id=# AND arena=?", data->id, p->arena->basename);
}

local void LoadArenaData(Arena *a)
{
	HSArenaData *adata = GetHSArenaData(a);

	adata->category_list = LLAlloc();
	adata->store_list = LLAlloc();

	mysql->Query(dbcb_loadcategories, a, 1, "SELECT id,name,description FROM hs_categories WHERE arena=?", a->basename);
	mysql->Query(dbcb_loadstores, a, 1, "SELECT id,name,description,region FROM hs_stores WHERE arena=?", a->basename);
}

local void LoadItemPropertyList()
{
	mysql->Query(dbcb_loadproperties, 0, 1, "SELECT hs_item_properties.item_id, hs_item_properties.name, hs_item_properties.setting FROM hs_item_properties, hs_items WHERE hs_items.id = hs_item_properties.item_id");
}

local void LoadItemList()
{
	mysql->Query(dbcb_loaditems, 0, 1, "SELECT id, name, short_description, description, buyprice, sellprice, item_type, item_max, ship_mask, exp_required, frequent_changes FROM hs_items");
}

/*
+---------------------+
|                     |
|   Store Functions   |
|                     |
+---------------------+
*/

local void StorePlayer(Player *p) //writes data to MySQL
{
    HSData *data = GetHSData(p);
    if (data->loading_status == LOADED)
    {
		StoreShips(p);
        mysql->Query(NULL, NULL, 0, "UPDATE hs_players SET money = #, exp = #, money_grant = #, money_give = #, money_kill = #, money_ball = #, money_flag = #, money_buysell = # WHERE name = ?",
        		data->money, data->exp,
        		data->money_type[MONEY_TYPE_GRANT],
        		data->money_type[MONEY_TYPE_GIVE],
        		data->money_type[MONEY_TYPE_KILL],
        		data->money_type[MONEY_TYPE_BALL],
        		data->money_type[MONEY_TYPE_FLAG],
        		data->money_type[MONEY_TYPE_BUYSELL],
        		p->name);
    }
    else
    {
        lm->LogP(L_INFO, "hscore_database", p, "Asked to store unloaded data.");
    }
}

local void StoreShips(Player *p)
{
	int i;
	HSData *data = GetHSData(p);

	if (!Loaded(p)) return;

	for (i = 0; i < 8; i++)
	{
		HullData *hull = data->hulls[i];

		if (!hull || data->hull_status[i] != HULL_LOADED) continue;


		Link *link = LLGetHead(hull->inventory_list);
		while (link)
		{
			ShipItem *ship_item = link->data;

			if (ship_item->item->frequent_changes)
			{
				lm->LogP(L_INFO, "hscore_database", p, "Would store item %s.", ship_item->item->name);
			}

			link = link->next;
		}
	}
}

/*
+----------------------+
|                      |
|   Unload Functions   |
|                      |
+----------------------+
*/

local void UnloadPlayer(Player *p) //should only be called if a player is leaving
{
    HSData *data = GetHSData(p);

	data->loading_status = NOT_LOADED;

	lm->LogP(L_INFO, "hscore_database", p, "Data freed.");
}

local void UnloadShips(Player *p)
{
	int i;
	HSData *data = GetHSData(p);

	if (!Loaded(p)) return;

	for (i = 0; i < 8; i++)
	{
		HullData *hull = data->hulls[i];

		data->hull_status[i] = HULL_NOT_LOADED;

		if (!hull) continue;


		Link *link = LLGetHead(hull->inventory_list);
		while (link)
		{
			ShipItem *ship_item = link->data;

			afree(ship_item);

			link = link->next;
		}

		LLFree(hull->inventory_list);

		afree(hull);
		data->hulls[i] = 0;
	}
}

local void UnloadArenaData(Arena *a)
{
	HSArenaData *adata = GetHSArenaData(a);

	LLEnum(adata->category_list, ClearCategoryListEnum);
	LLEnum(adata->store_list, ClearStoreListEnum);

	LLFree(adata->category_list);
	LLFree(adata->store_list);
}

local void UnloadItemList()
{
	LLEnum(item_list, ClearItemListEnum);
}

/*
+-------------------------------------+
|                                     |
|   Load/Unload/Store All Functions   |
|                                     |
+-------------------------------------+
*/

local void StoreAll() //writes all player data to MySQL, can be called periodicly
{
	Player *p;
	Link *link;
	pd->Lock();
	FOR_EACH_PLAYER(p)
		if (Loaded(p))
			StorePlayer(p);
	pd->Unlock();
}

local void UnloadAll() //deallocate the memory for ALL online players
{
	Player *p;
	Link *link;
	pd->Lock();
	FOR_EACH_PLAYER(p)
		if (Loaded(p))
			UnloadPlayer(p);
	pd->Unlock();
}

/*
+---------------------------------+
|                                 |
|   External Callback Functions   |
|                                 |
+---------------------------------+
*/

local void NewPlayer(Player *p, int isnew)
{
	HSData *data = GetHSData(p);
	int i;

	data->loading_status = HULL_NOT_LOADED;

	for (i = 0; i < 8; i++)
	{
		data->hull_status[i] = HULL_NOT_LOADED;
		data->hulls[i] = NULL;
	}
}

local void PlayerAction(Player *p, int action, Arena *arena)
{
    if (action == PA_CONNECT)
    {
		lm->LogP(L_DRIVEL, "hscore_database", p, "Loading player data.");
		LoadPlayer(p);
    }
    else if (action == PA_DISCONNECT)
    {
		lm->LogP(L_DRIVEL, "hscore_database", p, "Writing and unloading data.");
		StorePlayer(p);
		UnloadPlayer(p);
    }
	else if (action == PA_ENTERARENA)
	{
		LoadShips(p);
	}
	else if (action == PA_LEAVEARENA)
	{
		StoreShips(p);
		UnloadShips(p);
	}

}

local void ArenaAction(Arena *a, int action)
{
	if (action == AA_CREATE)
	{
		LoadArenaData(a);
	}
	else if (action == AA_DESTROY)
	{
		UnloadArenaData(a);
	}
}

/*
+-----------------------------+
|                             |
|   MM_module and interface   |
|                             |
+-----------------------------+
*/

local Ihsdatabase myint =
{
	INTERFACE_HEAD_INIT(I_HS_DATABASE, "hscore_database")
	GetHSData, Loaded, GetItemList, GetStoreList,
	GetCategoryList,AddShip,RemoveShip,
};

EXPORT int MM_hscore_database(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		mysql = mm->GetInterface(I_HS_MYSQL, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);

		if (!lm || !chat || !cfg || !cmd || !mysql || !aman || !pd || !ml)
		{
			goto fail;
		}

		hs_data_key = pd->AllocatePlayerData(sizeof(HSData));
		if (hs_data_key == -1)
		{
			goto fail;
		}

		hs_arena_data_key = aman->AllocateArenaData(sizeof(HSArenaData));
		if (hs_arena_data_key == -1)
		{
			pd->FreePlayerData(hs_data_key);
			goto fail;
		}

		item_list = LLAlloc();
		LoadItemList();

		mm->RegCallback(CB_PLAYERACTION, PlayerAction, ALLARENAS);
		mm->RegCallback(CB_NEWPLAYER, NewPlayer, ALLARENAS);
		mm->RegCallback(CB_ARENAACTION, ArenaAction, ALLARENAS);

		mm->RegInterface(&myint, ALLARENAS);

		return MM_OK;

	fail:
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(mysql);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(ml);
		return MM_FAIL;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&myint, ALLARENAS))
			return MM_FAIL;

		StoreAll();
		UnloadAll();
		UnloadItemList();

		LLFree(item_list);

		mm->UnregCallback(CB_PLAYERACTION, PlayerAction, ALLARENAS);
		mm->UnregCallback(CB_NEWPLAYER, NewPlayer, ALLARENAS);
		mm->UnregCallback(CB_ARENAACTION, ArenaAction, ALLARENAS);

		pd->FreePlayerData(hs_data_key);
		aman->FreeArenaData(hs_arena_data_key);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(mysql);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(ml);

		return MM_OK;
	}
	return MM_FAIL;
}

