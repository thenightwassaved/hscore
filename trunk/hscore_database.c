#include <stdlib.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_mysql.h"
#include "hscore_database.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihscoremysql *mysql;
local Iarenaman *aman;
local Iplayerdata *pd;
local Imainloop *ml;

//local prototypes
local PerArenaData * getPerArenaData(Arena *arena);
local Item * getItemByID(int id);
local ItemType * getItemTypeByID(int id);
local const char * getArenaIdentifier(Arena *arena);
local void LinkAmmo();
local void loadPropertiesQueryCallback(int status, db_res *result, void *passedData);
local void loadEventsQueryCallback(int status, db_res *result, void *passedData);
local void loadItemsQueryCallback(int status, db_res *result, void *passedData);
local void loadItemTypesQueryCallback(int status, db_res *result, void *passedData);
local void loadPlayerGlobalsQueryCallback(int status, db_res *result, void *passedData);
local void loadPlayerShipItemsQueryCallback(int status, db_res *result, void *passedData);
local void loadPlayerShipsQueryCallback(int status, db_res *result, void *passedData);
local void loadStoreItemsQueryCallback(int status, db_res *result, void *passedData);
local void loadArenaStoresQueryCallback(int status, db_res *result, void *passedData);
local void loadCategoryItemsQueryCallback(int status, db_res *result, void *passedData);
local void loadArenaCategoriesQueryCallback(int status, db_res *result, void *passedData);
local void InitPerPlayerData(Player *p);
local void InitPerArenaData(Arena *arena);
local void UnloadPlayerGlobals(Player *p);
local void UnloadPlayerShip(ShipHull *ship);
local void UnloadPlayerShips(Player *p);
local void UnloadCategoryList(Arena *arena);
local void UnloadStoreList(Arena *arena);
local void UnloadItemListEnumCallback(const void *ptr);
local void UnloadItemList();
local void UnloadItemTypeList();
local void UnloadAllPerArenaData();
local void UnloadAllPerPlayerData();
local void LoadPlayerGlobals(Player *p);
local void LoadPlayerShipItems(Player *p, Arena *arena);
local void LoadPlayerShips(Player *p, Arena *arena);
local void LoadCategoryItems(Arena *arena);
local void LoadCategoryList(Arena *arena);
local void LoadStoreItems(Arena *arena);
local void LoadStoreList(Arena *arena);
local void LoadEvents();
local void LoadProperties();
local void LoadItemList();
local void LoadItemTypeList();
local void StorePlayerGlobals(Player *p);
local void StorePlayerShips(Player *p, Arena *arena);
local void StoreAllPerPlayerData();

//interface prototypes
local int areShipsLoaded(Player *p);
local int isLoaded(Player *p);
local LinkedList * getItemList();
local LinkedList * getStoreList(Arena *arena);
local LinkedList * getCategoryList(Arena *arena);
local void updateItem(Player *p, int ship, Item *item, int newCount, int newData);
local void addShip(Player *p, int ship);
local void removeShip(Player *p, int ship);
local PerPlayerData *getPerPlayerData(Player *p);

//keys
local int playerDataKey;
local int arenaDataKey;

//lists
local LinkedList itemList;
local LinkedList itemTypeList;

//+-------------------------+
//|                         |
//|  Data Access Functions  |
//|                         |
//+-------------------------+

local PerArenaData * getPerArenaData(Arena *arena)
{
	return P_ARENA_DATA(arena, arenaDataKey);
}

local PerPlayerData *getPerPlayerData(Player *p)
{
	return PPDATA(p, playerDataKey);
}

local Item * getItemByID(int id)
{
	Link *link;
	for (link = LLGetHead(&itemList); link; link = link->next)
	{
		Item *item = link->data;

		if (item->id == id)
		{
			return item;
		}
	}

	return NULL;
}

local ItemType * getItemTypeByID(int id)
{
	Link *link;
	for (link = LLGetHead(&itemTypeList); link; link = link->next)
	{
		ItemType *itemType = link->data;

		if (itemType->id == id)
		{
			return itemType;
		}
	}

	return NULL;
}

//+--------------------------+
//|                          |
//|  Misc Utility Functions  |
//|                          |
//+--------------------------+

local const char * getArenaIdentifier(Arena *arena)
{
	const char *arenaIdent = cfg->GetStr(arena->cfg, "Hyperspace", "ArenaIdentifier");

	if (arenaIdent == NULL)
	{
		arenaIdent = arena->basename; //default fallback
	}

	return arenaIdent;
}

//+------------------------+
//|                        |
//|  Post-Query Functions  |
//|                        |
//+------------------------+

local void LinkAmmo()
{
	Link *link;
	for (link = LLGetHead(&itemList); link; link = link->next)
	{
		Item *item = link->data;

		if (item->ammoID != 0)
		{
			Item *ammo = getItemByID(item->ammoID);
			item->ammo = ammo;

			if (item->ammo == NULL)
			{
				lm->Log(L_ERROR, "<hscore_database> No ammo matched id %i requested by item id %i.", item->ammoID, item->id);
			}
		}
	}
}

//+-------------------------+
//|                         |
//|  MySQL Query Callbacks  |
//|                         |
//+-------------------------+

local void loadPropertiesQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	if (status != 0 || result == NULL)
	{
		lm->Log(L_ERROR, "<hscore_database> Unexpected database error during property load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		lm->Log(L_WARN, "<hscore_database> No properties returned from MySQL query.");
	}

	while ((row = mysql->GetRow(result)))
	{
		int itemID = atoi(mysql->GetField(row, 0));					//item_id
		Item *item = getItemByID(itemID);

		if (item != NULL)
		{
			Property *property = amalloc(sizeof(*property));

			astrncpy(property->name, mysql->GetField(row, 1), 16);	//name
			property->value = atoi(mysql->GetField(row, 2));		//value

			//add the item type to the list
			LLAdd(&item->propertyList, property);
		}
		else
		{
			lm->Log(L_ERROR, "<hscore_database> property looking for item ID %i.", itemID);
		}
	}

	lm->Log(L_DRIVEL, "<hscore_database> %i properties were loaded from MySQL.", results);
}

local void loadEventsQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	if (status != 0 || result == NULL)
	{
		lm->Log(L_ERROR, "<hscore_database> Unexpected database error during event load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		lm->Log(L_WARN, "<hscore_database> No events returned from MySQL query.");
	}

	while ((row = mysql->GetRow(result)))
	{
		int itemID = atoi(mysql->GetField(row, 0));					//item_id
		Item *item = getItemByID(itemID);

		if (item != NULL)
		{
			Event *event = amalloc(sizeof(*event));

			astrncpy(event->event, mysql->GetField(row, 1), 16);	//event
			event->action = atoi(mysql->GetField(row, 2));			//action
			event->data = atoi(mysql->GetField(row, 3));			//data
			astrncpy(event->message, mysql->GetField(row, 4), 200);	//message


			//add the item type to the list
			LLAdd(&item->eventList, event);
		}
		else
		{
			lm->Log(L_ERROR, "<hscore_database> event looking for item ID %i.", itemID);
		}
	}

	lm->Log(L_DRIVEL, "<hscore_database> %i events were loaded from MySQL.", results);
}

local void loadItemsQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	if (status != 0 || result == NULL)
	{
		lm->Log(L_ERROR, "<hscore_database> Unexpected database error during items load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		lm->Log(L_WARN, "<hscore_database> No items returned from MySQL query.");
	}

	while ((row = mysql->GetRow(result)))
	{
		Item *item = amalloc(sizeof(*item));

		LLInit(&item->propertyList);
		LLInit(&item->eventList);

		item->id = atoi(mysql->GetField(row, 0)); 					//id
		astrncpy(item->name, mysql->GetField(row, 1), 16);			//name
		astrncpy(item->shortDesc, mysql->GetField(row, 2), 32);		//short_description
		astrncpy(item->longDesc, mysql->GetField(row, 3), 200);		//long_description
		item->buyPrice = atoi(mysql->GetField(row, 4));				//buy_price
		item->sellPrice = atoi(mysql->GetField(row, 5));			//sell_price
		item->expRequired = atoi(mysql->GetField(row, 6));			//exp_required
		item->shipsAllowed = atoi(mysql->GetField(row, 7));			//ships_allowed

		item->type1 = getItemTypeByID(atoi(mysql->GetField(row, 8)));	//type1
		item->type2 = getItemTypeByID(atoi(mysql->GetField(row, 9)));	//type2

		item->typeDelta1 = atoi(mysql->GetField(row, 10));			//type1_delta
		item->typeDelta2 = atoi(mysql->GetField(row, 11));			//type2_delta

		item->max = atoi(mysql->GetField(row, 12));					//max

		item->delayStatusWrite = atoi(mysql->GetField(row, 13));	//delay_write
		item->ammoID = atoi(mysql->GetField(row, 14));				//ammo

		//add the item type to the list
		LLAdd(&itemList, item);
	}

	lm->Log(L_DRIVEL, "<hscore_database> %i items were loaded from MySQL.", results);

	//now that all the items are in, load the properties & events.
	LoadProperties();
	LoadEvents();

	//process the ammo ids
	LinkAmmo();
}

local void loadItemTypesQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	if (status != 0 || result == NULL)
	{
		lm->Log(L_ERROR, "<hscore_database> Unexpected database error during item types load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		lm->Log(L_WARN, "<hscore_database> No item types returned from MySQL query.");
	}

	while ((row = mysql->GetRow(result)))
	{
		ItemType *itemType = amalloc(sizeof(*itemType));

		itemType->id = atoi(mysql->GetField(row, 0));			//id
		astrncpy(itemType->name, mysql->GetField(row, 1), 32);	//name
		itemType->max = atoi(mysql->GetField(row, 2));			//max

		//add the item type to the list
		LLAdd(&itemTypeList, itemType);
	}

	lm->Log(L_DRIVEL, "<hscore_database> %i item types were loaded from MySQL.", results);
	LoadItemList(); //now that all the item types are in, load the items.
}

local void loadPlayerGlobalsQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	Player *p = passedData;
	PerPlayerData *playerData = getPerPlayerData(p);

	if (status != 0 || result == NULL)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "Unexpected database error during player globals load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		//insert a new player into MySQL and then get it
	    int initialMoney = cfg->GetInt(GLOBAL, "hyperspace", "initialmoney", 1000);
	    int initialExp = cfg->GetInt(GLOBAL, "hyperspace", "initialexp", 0);
	    mysql->Query(NULL, NULL, 0, "INSERT INTO hs_players VALUES (NULL, ?, #, #, 0, 0, 0, 0, 0, 0, 0)", p->name, initialMoney, initialExp);
		LoadPlayerGlobals(p);
		return;
	}

	if (results > 1)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "Multiple rows returned from MySQL: using first.");
	}

	row = mysql->GetRow(result);

	playerData->id = atoi(mysql->GetField(row, 0)); 							//id
	playerData->money = atoi(mysql->GetField(row, 1)); 							//money
	playerData->exp = atoi(mysql->GetField(row, 2)); 							//exp
	playerData->moneyType[MONEY_TYPE_GIVE] = atoi(mysql->GetField(row, 3)); 	//money_give
	playerData->moneyType[MONEY_TYPE_GRANT] = atoi(mysql->GetField(row, 4)); 	//money_grant
	playerData->moneyType[MONEY_TYPE_BUYSELL] = atoi(mysql->GetField(row, 5)); 	//money_buysell
	playerData->moneyType[MONEY_TYPE_KILL] = atoi(mysql->GetField(row, 6)); 	//money_kill
	playerData->moneyType[MONEY_TYPE_FLAG] = atoi(mysql->GetField(row, 7)); 	//money_flag
	playerData->moneyType[MONEY_TYPE_BALL] = atoi(mysql->GetField(row, 8)); 	//money_ball
	playerData->moneyType[MONEY_TYPE_EVENT] = atoi(mysql->GetField(row, 9)); 	//money_event

	playerData->loaded = 1;

	lm->LogP(L_DRIVEL, "hscore_database", p, "loaded globals from MySQL.");
}

local void loadShipIDQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	ShipHull *hull = passedData;

	if (status != 0 || result == NULL)
	{
		lm->Log(L_ERROR, "<hscore_database> Unexpected database error during ship ID load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		lm->Log(L_ERROR, "<hscore_database> No ship ID results returned. Expect things to go to hell.");
		return;
	}

	if (results > 1)
	{
		lm->Log(L_ERROR, "<hscore_database> More than one ship ID returned. Using first.");
	}

	row = mysql->GetRow(result);

	int id = atoi(mysql->GetField(row, 0));

	hull->id = id;
}

local void loadPlayerShipItemsQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	Player *p = passedData;
	PerPlayerData *playerData = getPerPlayerData(p);

	if (status != 0 || result == NULL)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "Unexpected database error during player ship item load.");
		return;
	}

	results = mysql->GetRowCount(result);

	while ((row = mysql->GetRow(result)))
	{
		int itemID = atoi(mysql->GetField(row, 0));	//item_id
		int count = atoi(mysql->GetField(row, 1));	//count
		int data = atoi(mysql->GetField(row, 2));	//data
		int ship = atoi(mysql->GetField(row, 3));	//ship

		Item *item = getItemByID(itemID);

		if (0 <= ship && ship < 8)
		{
			if (playerData->hull[ship] != NULL)
			{
				if (item != NULL)
				{
					ShipHull *hull = playerData->hull[ship];

					InventoryEntry *inventoryEntry = amalloc(sizeof(*inventoryEntry));

					inventoryEntry->item = item;
					inventoryEntry->count = count;
					inventoryEntry->data = data;

					LLAdd(&hull->inventoryEntryList, inventoryEntry);
				}
				else
				{
					lm->LogP(L_ERROR, "hscore_database", p, "Bad item id (%i) attached to ship %i", itemID, ship);
				}
			}
			else
			{
				lm->LogP(L_ERROR, "hscore_database", p, "Item %i attached to ship %i, but no ship hull exists there!", itemID, ship);
			}
		}
		else
		{
			lm->LogP(L_ERROR, "hscore_database", p, "Extreme error! item %i attached to ship %i", itemID, ship);
		}
	}

	playerData->shipsLoaded = 1;

	lm->LogP(L_DRIVEL, "hscore_database", p, "%i ship items were loaded from MySQL.", results);
}

local void loadPlayerShipsQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	Player *p = passedData;
	PerPlayerData *playerData = getPerPlayerData(p);

	if (status != 0 || result == NULL)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "Unexpected database error during player ship load.");
		return;
	}

	results = mysql->GetRowCount(result);

	while ((row = mysql->GetRow(result)))
	{
		int id = atoi(mysql->GetField(row, 0));		//id
		int ship = atoi(mysql->GetField(row, 1));	//ship

		if (0 <= ship && ship < 8)
		{
			ShipHull *hull = amalloc(sizeof(*hull));

			LLInit(&hull->inventoryEntryList);

			hull->id = id;

			playerData->hull[ship] = hull;
		}
		else
		{
			lm->LogP(L_ERROR, "hscore_database", p, "ship id %i had bad ship number (%i)", id, ship);
		}
	}

	playerData->shipsLoaded = 1;

	lm->LogP(L_DRIVEL, "hscore_database", p, "%i ships were loaded from MySQL.", results);

	LoadPlayerShipItems(p, p->arena); //load the inventory for the ship
}

local void loadStoreItemsQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	Arena *arena = passedData;
	PerArenaData *arenaData = getPerArenaData(arena);

	if (status != 0 || result == NULL)
	{
		lm->LogA(L_ERROR, "hscore_database", arena, "Unexpected database error during store item load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		//no big deal
		return;
	}

	while ((row = mysql->GetRow(result)))
	{
		Link *link;
		int itemID = atoi(mysql->GetField(row, 0));		//item_id
		int storeID = atoi(mysql->GetField(row, 1));	//store_id

		Item *item = getItemByID(itemID);
		if (item == NULL)
		{
			lm->LogA(L_ERROR, "hscore_database", arena, "item id %i not found (linked to store id %i)", itemID, storeID);
			continue;
		}

		for (link = LLGetHead(&arenaData->storeList); link; link = link->next)
		{
			Store *store = link->data;

			if (store->id == storeID)
			{
				//add the item to the store's list
				LLAdd(&store->itemList, item);
				break;
			}
		}

		//FIXME: Add error on bad storeID
	}

	lm->LogA(L_DRIVEL, "hscore_database", arena, "%i store items were loaded from MySQL.", results);
}

local void loadArenaStoresQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	Arena *arena = passedData;
	PerArenaData *arenaData = getPerArenaData(arena);

	if (status != 0 || result == NULL)
	{
		lm->LogA(L_ERROR, "hscore_database", arena, "Unexpected database error during store load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		//no big deal
		return;
	}

	while ((row = mysql->GetRow(result)))
	{
		Store *store = amalloc(sizeof(*store));

		LLInit(&store->itemList);

		store->id = atoi(mysql->GetField(row, 0));					//id
		astrncpy(store->name, mysql->GetField(row, 1), 32);			//name
		astrncpy(store->description, mysql->GetField(row, 2), 200);	//description
		astrncpy(store->region, mysql->GetField(row, 3), 16);		//region

		//add the item type to the list
		LLAdd(&arenaData->storeList, store);
	}

	lm->LogA(L_DRIVEL, "hscore_database", arena, "%i stores were loaded from MySQL.", results);
	LoadStoreItems(arena); //now that all the stores are in, load the items into them.
}

local void loadCategoryItemsQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	Arena *arena = passedData;
	PerArenaData *arenaData = getPerArenaData(arena);

	if (status != 0 || result == NULL)
	{
		lm->LogA(L_ERROR, "hscore_database", arena, "Unexpected database error during category item load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		//no big deal
		return;
	}

	while ((row = mysql->GetRow(result)))
	{
		Link *link;
		int itemID = atoi(mysql->GetField(row, 0));		//item_id
		int categoryID = atoi(mysql->GetField(row, 1));	//category_id

		Item *item = getItemByID(itemID);
		if (item == NULL)
		{

			lm->LogA(L_ERROR, "hscore_database", arena, "item id %i not found (linked to category id %i)", itemID, categoryID);
			continue;
		}

		for (link = LLGetHead(&arenaData->categoryList); link; link = link->next)
		{
			Category *category = link->data;

			if (category->id == categoryID)
			{
				//add the item to the category's list
				LLAdd(&category->itemList, item);
				break;
			}
		}

		//FIXME: Add error on bad categoryID
	}

	lm->LogA(L_DRIVEL, "hscore_database", arena, "%i cateogry items were loaded from MySQL.", results);
}

local void loadArenaCategoriesQueryCallback(int status, db_res *result, void *passedData)
{
	int results;
	db_row *row;

	Arena *arena = passedData;
	PerArenaData *arenaData = getPerArenaData(arena);

	if (status != 0 || result == NULL)
	{
		lm->LogA(L_ERROR, "hscore_database", arena, "Unexpected database error during category load.");
		return;
	}

	results = mysql->GetRowCount(result);

	if (results == 0)
	{
		//no big deal
		return;
	}

	while ((row = mysql->GetRow(result)))
	{
		Category *category = amalloc(sizeof(*category));

		LLInit(&category->itemList);

		category->id = atoi(mysql->GetField(row, 0));					//id
		astrncpy(category->name, mysql->GetField(row, 1), 32);			//name
		astrncpy(category->description, mysql->GetField(row, 2), 64);	//description

		//add the item type to the list
		LLAdd(&arenaData->categoryList, category);
	}

	lm->LogA(L_DRIVEL, "hscore_database", arena, "%i categories were loaded from MySQL.", results);
	LoadCategoryItems(arena); //now that all the stores are in, load the items into them.
}

//+------------------+
//|                  |
//|  Init Functions  |
//|                  |
//+------------------+

local void InitPerPlayerData(Player *p) //called before data is touched
{
	PerPlayerData *playerData = getPerPlayerData(p);

	playerData->loaded = 0;
	playerData->shipsLoaded = 0;

	for (int i = 0; i < 8; i++)
	{
		playerData->hull[i] = NULL;
	}
}

local void InitPerArenaData(Arena *arena) //called before data is touched
{
	PerArenaData *arenaData = getPerArenaData(arena);

	LLInit(&arenaData->categoryList);
	LLInit(&arenaData->storeList);
}

//+--------------------+
//|                    |
//|  Unload Functions  |
//|                    |
//+--------------------+

local void UnloadPlayerGlobals(Player *p) //called to free any allocated data
{
	PerPlayerData *playerData = getPerPlayerData(p);

	playerData->loaded = 0; //nothing else needed

	lm->LogP(L_DRIVEL, "hscore_database", p, "Freed global data.");
}

local void UnloadPlayerShip(ShipHull *ship)
{
	LLEnum(&ship->inventoryEntryList, afree);

	afree(ship);
}

local void UnloadPlayerShips(Player *p) //called to free any allocated data
{
	PerPlayerData *playerData = getPerPlayerData(p);

	playerData->shipsLoaded = 0;

	for (int i = 0; i < 8; i++)
	{
		if (playerData->hull[i] != NULL)
		{
			UnloadPlayerShip(playerData->hull[i]);
			playerData->hull[i] = NULL;
		}
	}

	lm->LogP(L_DRIVEL, "hscore_database", p, "Freed ship data.");
}

local void UnloadCategoryList(Arena *arena) //called when the arena is about to die
{
	PerArenaData *arenaData = getPerArenaData(arena);

	LLEnum(&arenaData->categoryList, afree); //can simply free all the Category structs


	lm->LogA(L_DRIVEL, "hscore_database", arena, "Freed %i categories.", LLCount(&arenaData->categoryList));

	LLEmpty(&arenaData->categoryList);
}

local void UnloadStoreList(Arena *arena)
{
	PerArenaData *arenaData = getPerArenaData(arena);

	LLEnum(&arenaData->storeList, afree); //can simply free all the Store structs

	lm->LogA(L_DRIVEL, "hscore_database", arena, "Freed %i stores.", LLCount(&arenaData->storeList));

	LLEmpty(&arenaData->storeList);
}

local void UnloadItemListEnumCallback(const void *ptr)
{
	Item *item = (Item*)ptr;

	LLEnum(&item->propertyList, afree);
	LLEnum(&item->eventList, afree);

	afree(item);
}

local void UnloadItemList()
{
	LLEnum(&itemList, UnloadItemListEnumCallback);

	lm->Log(L_DRIVEL, "<hscore_database> Freed %i items.", LLCount(&itemList));

	LLEmpty(&itemList);
}

local void UnloadItemTypeList()
{
	LLEnum(&itemTypeList, afree); //can simply free all the ItemType structs

	lm->Log(L_DRIVEL, "<hscore_database> Freed %i item types.", LLCount(&itemTypeList));

	LLEmpty(&itemTypeList);
}

local void UnloadAllPerArenaData()
{
	//FIXME
}

local void UnloadAllPerPlayerData()
{
	//FIXME
}

//+------------------+
//|                  |
//|  Load Functions  |
//|                  |
//+------------------+

local void LoadPlayerGlobals(Player *p) //fetch globals from MySQL
{
	//FIXME: check if the player is already online

	mysql->Query(loadPlayerGlobalsQueryCallback, p, 1, "SELECT id, money, exp, money_give, money_grant, money_buysell, money_kill, money_flag, money_ball, money_event FROM hs_players WHERE name = ?", p->name);
}

local void LoadPlayerShipItems(Player *p, Arena *arena) //fetch ship items from MySQL
{
	if (areShipsLoaded(p))
	{
		PerPlayerData *playerData = getPerPlayerData(p);
		mysql->Query(loadPlayerShipItemsQueryCallback, p, 1, "SELECT item_id, count, data, ship FROM hs_player_ship_items, hs_player_ships WHERE player_id = # AND arena = ? AND ship_id = id", playerData->id, getArenaIdentifier(arena));
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_database", p, "Asked to load ship items for player without loaded ships.");
	}
}

local void LoadPlayerShips(Player *p, Arena *arena) //fetch ships from MySQL. Will call LoadPlayerShipItems()
{
	if (isLoaded(p))
	{
		PerPlayerData *playerData = getPerPlayerData(p);
		mysql->Query(loadPlayerShipsQueryCallback, p, 1, "SELECT id, ship FROM hs_player_ships WHERE player_id = # AND arena = ?", playerData->id, getArenaIdentifier(arena));
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_database", p, "Asked to load ships for unloaded player.");
	}
}

local void LoadCategoryItems(Arena *arena)
{
	mysql->Query(loadCategoryItemsQueryCallback, arena, 1, "SELECT item_id, category_id FROM hs_category_items, hs_categories WHERE category_id = id AND arena = ?", getArenaIdentifier(arena));
}

local void LoadCategoryList(Arena *arena) //leads to LoadCategoryItems() being called
{
	mysql->Query(loadArenaCategoriesQueryCallback, arena, 1, "SELECT id, name, description FROM hs_categories WHERE arena = ?", getArenaIdentifier(arena));
}

local void LoadStoreItems(Arena *arena)
{
	mysql->Query(loadStoreItemsQueryCallback, arena, 1, "SELECT item_id, store_id FROM hs_store_items, hs_stores WHERE store_id = id AND arena = ?", getArenaIdentifier(arena));
}

local void LoadStoreList(Arena *arena) //leads to LoadStoreItems() being called
{
	mysql->Query(loadArenaStoresQueryCallback, arena, 1, "SELECT id, name, description, region FROM hs_stores WHERE arena = ?", getArenaIdentifier(arena));
}

local void LoadEvents()
{
	mysql->Query(loadEventsQueryCallback, NULL, 1, "SELECT item_id, event, action, data, message FROM hs_item_events");
}

local void LoadProperties()
{
	mysql->Query(loadPropertiesQueryCallback, NULL, 1, "SELECT item_id, name, value FROM hs_item_properties");
}

local void LoadItemList() //will call LoadProperties() and LoadEvents() when finished
{
	mysql->Query(loadItemsQueryCallback, NULL, 1, "SELECT id, name, short_description, long_description, buy_price, sell_price, exp_required, ships_allowed, type1, type2, type1_delta, type2_delta, max, delay_write, ammo FROM hs_items");
}

local void LoadItemTypeList() //will call LoadItemList() when finished loading
{
	mysql->Query(loadItemTypesQueryCallback, NULL, 1, "SELECT id, name, max FROM hs_item_types");
}

//+-------------------+
//|                   |
//|  Store Functions  |
//|                   |
//+-------------------+

local void StorePlayerGlobals(Player *p) //store player globals. MUST FINISH IN ONE QUERY LEVEL
{
	PerPlayerData *playerData = getPerPlayerData(p);

	if (isLoaded(p))
	{
		mysql->Query(NULL, NULL, 0, "UPDATE hs_players SET money = #, exp = #, money_give = #, money_grant = #, money_buysell = #, money_kill = #, money_flag = #, money_ball = #, money_event = # WHERE id = #",
			playerData->money,
			playerData->exp,
			playerData->moneyType[MONEY_TYPE_GIVE],
			playerData->moneyType[MONEY_TYPE_GRANT],
			playerData->moneyType[MONEY_TYPE_BUYSELL],
			playerData->moneyType[MONEY_TYPE_KILL],
			playerData->moneyType[MONEY_TYPE_FLAG],
			playerData->moneyType[MONEY_TYPE_BALL],
			playerData->moneyType[MONEY_TYPE_EVENT],
			playerData->id);
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to store unloaded player");
	}
}

local void StorePlayerShips(Player *p, Arena *arena) //store player ships. MUST FINISH IN ONE QUERY LEVEL
{
	PerPlayerData *playerData = getPerPlayerData(p);

	if (areShipsLoaded(p))
	{
		for (int i = 0; i < 8; i++)
		{
			if (playerData->hull[i] != NULL)
			{
				Link *link;
				LinkedList *inventoryList = &playerData->hull[i]->inventoryEntryList;

				int shipID = playerData->hull[i]->id;

				if (shipID == -1)
				{
					lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

					while (shipID == -1)
					{
						shipID = playerData->hull[i]->id;
					}
				}

				for (link = LLGetHead(inventoryList); link; link = link->next)
				{
					InventoryEntry *entry = link->data;

					if (entry->item->delayStatusWrite)
					{
						mysql->Query(NULL, NULL, 0, "REPLACE INTO hs_player_ship_items VALUES (#,#,#,#)", shipID, entry->item->id, entry->count, entry->data);
					}
				}
			}
		}
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to store unloaded player");
	}
}

local void StoreAllPerPlayerData()
{
	//FIXME
}

//+---------------------+
//|                     |
//|  Command Functions  |
//|                     |
//+---------------------+

local helptext_t reloadItemsHelp =
"Targets: none\n"
"Args: none\n"
"This command will reload all current and new items from the database.\n"
"NOTE: This command will *NOT* remove items no longer in the database.\n";

local void reloadItemsCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

local helptext_t storeAllHelp =
"Targets: none\n"
"Args: none\n"
"Stores all loaded player information back into MySQL.\n";

local void storeAllCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

//+----------------------+
//|                      |
//|  Callback functions  |
//|                      |
//+----------------------+

local void allocatePlayerCallback(Player *p, int allocating)
{
	if (allocating) //player is being allocated
	{
		//make sure we "zero out" the necessary data
		InitPerPlayerData(p);
	}
	else //p is being deallocated
	{
		//already taken care of on disconnect
	}
}

local void playerActionCallback(Player *p, int action, Arena *arena)
{
	if (action == PA_CONNECT)
	{
		//the player is connecting to the server. not arena-specific.

		LoadPlayerGlobals(p);
	}
	else if (action == PA_DISCONNECT)
	{
		//the player is disconnecting from the server. not arena-specific.

		StorePlayerGlobals(p);
		UnloadPlayerGlobals(p);
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

		LoadPlayerShips(p, arena);
	}
	else if (action == PA_LEAVEARENA)
	{
		//the player is leaving an arena.

		StorePlayerShips(p, arena);
		UnloadPlayerShips(p);
	}
	else if (action == PA_ENTERGAME)
	{
		//this is called at some point after the player has sent his first
		//position packet (indicating that he's joined the game, as
		//opposed to still downloading a map).
	}
}

local void arenaActionCallback(Arena *arena, int action)
{
	if (action == AA_CREATE)
	{
		//arena is being created

		InitPerArenaData(arena);

		//in no special order...
		LoadStoreList(arena);
		LoadCategoryList(arena);
	}
	else if (action == AA_DESTROY)
	{
		//arena is being destroyed

		//in no special order...
		UnloadStoreList(arena);
		UnloadCategoryList(arena);

		//no need to deallocate the lists, as they weren't allocated
	}
	else if (action == AA_CONFCHANGED)
	{
		//do we need to do anything?
	}
}

//+-----------------------+
//|                       |
//|  Interface Functions  |
//|                       |
//+-----------------------+

local int areShipsLoaded(Player *p)
{
	PerPlayerData *playerData = getPerPlayerData(p);
	return playerData->shipsLoaded;
}

local int isLoaded(Player *p)
{
	PerPlayerData *playerData = getPerPlayerData(p);
	return playerData->loaded;
}

local LinkedList * getItemList()
{
	return &itemList;
}

local LinkedList * getStoreList(Arena *arena)
{
	PerArenaData *arenaData = getPerArenaData(arena);
	return &arenaData->storeList;
}

local LinkedList * getCategoryList(Arena *arena)
{
	PerArenaData *arenaData = getPerArenaData(arena);
	return &arenaData->categoryList;
}

local void updateItem(Player *p, int ship, Item *item, int newCount, int newData)
{
	PerPlayerData *playerData = getPerPlayerData(p);

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to update item on ship %i", ship);
		return;
	}

	if (!areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to update item on unloaded ships");
		return;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to update item on unowned ship %i", ship);
		return;
	}

	Link *link;
	LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;

		if (entry->item == item)
		{
			if (newCount != 0)
			{
				if (entry->count == newCount && entry->data == newData)
				{
					lm->LogP(L_ERROR, "hscore_database", p, "asked to update item %s with no change", item->name);
				}
				else
				{
					entry->count = newCount;
					entry->data = newData;

					if (!item->delayStatusWrite)
					{
						int shipID = playerData->hull[ship]->id;

						if (shipID == -1)
						{
							lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

							while (shipID == -1)
							{
								shipID = playerData->hull[ship]->id;
							}
						}

						mysql->Query(NULL, NULL, 0, "REPLACE INTO hs_player_ship_items VALUES (#,#,#,#)", shipID, item->id, newCount, newData);
					}
				}
			}
			else
			{
				int shipID = playerData->hull[ship]->id;

				if (shipID == -1)
				{
					lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

					while (shipID == -1)
					{
						shipID = playerData->hull[ship]->id;
					}
				}

				mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ship_items WHERE ship_id = # AND item_id = #", shipID, item->id);

				LLRemove(inventoryList, entry);
				afree(entry);
			}

			return;
		}
	}

	//if we leave the for loop, we'll have to add it.

	if (newCount != 0)
	{
		int shipID = playerData->hull[ship]->id;

		if (shipID == -1)
		{
			lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

			while (shipID == -1)
			{
				shipID = playerData->hull[ship]->id;
			}
		}

		mysql->Query(NULL, NULL, 0, "REPLACE INTO hs_player_ship_items VALUES (#,#,#,#)", shipID, item->id, newCount, newData);

		InventoryEntry *entry = amalloc(sizeof(*entry));

		entry->count = newCount;
		entry->data = newData;
		entry->item = item;

		LLAdd(inventoryList, entry);
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to remove item %s not on ship %i", item->name, ship);
	}
}

local void addShip(Player *p, int ship) //the ships id may not be valid until later
{
	PerPlayerData *playerData = getPerPlayerData(p);

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to add ship %i", ship);
		return;
	}

	if (!isLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to add ship to unloaded player");
		return;
	}

	if (playerData->hull[ship] != NULL)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to add owned ship %i", ship);
		return;
	}

	ShipHull *hull = amalloc(sizeof(*hull));

	LLInit(&hull->inventoryEntryList);
	hull->id = -1;

	playerData->hull[ship] = hull;

	mysql->Query(NULL, NULL, 0, "INSERT INTO hs_player_ships VALUES (NULL, #, #, ?)", playerData->id, ship, getArenaIdentifier(p->arena));
	mysql->Query(loadShipIDQueryCallback, hull, 1, "SELECT id FROM hs_player_ships WHERE player_id = # AND ship = #", playerData->id, ship);
}

local void removeShip(Player *p, int ship)
{
	PerPlayerData *playerData = getPerPlayerData(p);

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to add ship %i", ship);
		return;
	}

	if (!isLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to remove ship from unloaded player");
		return;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to removed unowned ship %i", ship);
		return;
	}

	int shipID = playerData->hull[ship]->id;

	if (shipID == -1)
	{
		lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

		while (shipID == -1)
		{
			shipID = playerData->hull[ship]->id;
		}
	}

	mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ship_items WHERE ship_id = #", shipID); //empty the ship
	mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ships WHERE id = #", shipID);

	UnloadPlayerShip(playerData->hull[ship]);

	playerData->hull[ship] = NULL;
}

//getPerPlayerData declared elsewhere

local Ihscoredatabase interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_DATABASE, "hscore_database")
	areShipsLoaded, isLoaded, getItemList, getStoreList,
	getCategoryList, updateItem, addShip, removeShip, getPerPlayerData,
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
		mysql = mm->GetInterface(I_HSCORE_MYSQL, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);

		if (!lm || !chat || !cfg || !cmd || !mysql || !aman || !pd || !ml)
		{
			goto fail;
		}

		playerDataKey = pd->AllocatePlayerData(sizeof(PerPlayerData));
		if (playerDataKey == -1)
		{
			goto fail;
		}

		arenaDataKey = aman->AllocateArenaData(sizeof(PerArenaData));
		if (arenaDataKey == -1)
		{
			pd->FreePlayerData(playerDataKey);
			goto fail;
		}

		LLInit(&itemList);
		LLInit(&itemTypeList);

		LoadItemTypeList();

		mm->RegCallback(CB_NEWPLAYER, allocatePlayerCallback, ALLARENAS);
		mm->RegCallback(CB_PLAYERACTION, playerActionCallback, ALLARENAS);
		mm->RegCallback(CB_ARENAACTION, arenaActionCallback, ALLARENAS);

		mm->RegInterface(&interface, ALLARENAS);

		cmd->AddCommand("reloaditems", reloadItemsCommand, ALLARENAS, reloadItemsHelp);
		cmd->AddCommand("storeall", storeAllCommand, ALLARENAS, storeAllHelp);

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
		if (mm->UnregInterface(&interface, ALLARENAS))
		{
			return MM_FAIL;
		}

		cmd->RemoveCommand("reloaditems", reloadItemsCommand, ALLARENAS);
		cmd->RemoveCommand("storeall", storeAllCommand, ALLARENAS);

		mm->UnregCallback(CB_NEWPLAYER, allocatePlayerCallback, ALLARENAS);
		mm->UnregCallback(CB_PLAYERACTION, playerActionCallback, ALLARENAS);
		mm->UnregCallback(CB_ARENAACTION, arenaActionCallback, ALLARENAS);

		StoreAllPerPlayerData();

		UnloadAllPerPlayerData();
		UnloadAllPerArenaData();

		UnloadItemList();
		UnloadItemTypeList();

		pd->FreePlayerData(playerDataKey);
		aman->FreeArenaData(arenaDataKey);

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
