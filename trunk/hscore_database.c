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
local void InitPerPlayerData(Player *p);
local void InitPerArenaData(Arena *arena);
local void UnloadPlayerGlobals(Player *p);
local void UnloadPlayerShips(Player *p);
local void UnloadCategoryList(Arena *arena);
local void UnloadStoreList(Arena *arena);
local void UnloadItemList();
local void UnloadItemTypeList();
local void UnloadAllPerArenaData();
local void UnloadAllPerPlayerData();
local void LoadPlayerGlobals(Player *p);
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

//interface prototypes
local int areShipsLoaded(Player *p);
local int isLoaded(Player *p);
local LinkedList * getItemList();
local LinkedList * getStoreList(Arena *arena);
local LinkedList * getCategoryList(Arena *arena);
local void addShip(Player *p, int ship, LinkedList *itemList);
local void removeShip(Player *p, int ship);
local PerPlayerData * getPerPlayerData(Player *p);

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
			LLAdd(&(item->propertyList), property);
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
			LLAdd(&(item->eventList), event);
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

		LLInit(&(item->propertyList));
		LLInit(&(item->eventList));

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
		item->delayStatusWrite = atoi(mysql->GetField(row, 12));	//delay_write
		item->ammoID = atoi(mysql->GetField(row, 13));				//ammo

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
	//FIXME
}

local void loadPlayerShipsQueryCallback(int status, db_res *result, void *passedData)
{
	//FIXME
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

		for (link = LLGetHead(&(arenaData->storeList)); link; link = link->next)
		{
			Store *store = link->data;

			if (store->id == storeID)
			{
				//add the item to the store's list
				LLAdd(&(store->itemList), item);
				break;
			}
		}

		//FIXME: Add error on bad storeID
	}

	lm->LogA(L_DRIVEL, "hscore_database", arena, "%i store items were loaded for arena %s from MySQL.", results);
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

		LLInit(&(store->itemList));

		store->id = atoi(mysql->GetField(row, 0));					//id
		astrncpy(store->name, mysql->GetField(row, 1), 32);			//name
		astrncpy(store->description, mysql->GetField(row, 2), 200);	//description
		astrncpy(store->region, mysql->GetField(row, 3), 16);		//region

		//add the item type to the list
		LLAdd(&(arenaData->storeList), store);
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

		for (link = LLGetHead(&(arenaData->categoryList)); link; link = link->next)
		{
			Category *category = link->data;

			if (category->id == categoryID)
			{
				//add the item to the category's list
				LLAdd(&(category->itemList), item);
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

		LLInit(&(category->itemList));

		category->id = atoi(mysql->GetField(row, 0));					//id
		astrncpy(category->name, mysql->GetField(row, 1), 32);			//name
		astrncpy(category->description, mysql->GetField(row, 2), 64);	//description

		//add the item type to the list
		LLAdd(&(arenaData->categoryList), category);
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

	LLInit(&(arenaData->categoryList));
	LLInit(&(arenaData->storeList));
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

local void UnloadPlayerShips(Player *p) //called to free any allocated data
{
	//FIXME

	lm->LogP(L_DRIVEL, "hscore_database", p, "Freed ship data.");
}

local void UnloadCategoryList(Arena *arena) //called when the arena is about to die
{
	PerArenaData *arenaData = getPerArenaData(arena);

	LLEnum(&(arenaData->categoryList), afree); //can simply free all the Category structs


	lm->LogA(L_DRIVEL, "hscore_database", arena, "Freed %i categories.", LLCount(&(arenaData->categoryList)));

	LLEmpty(&(arenaData->categoryList));
}

local void UnloadStoreList(Arena *arena)
{
	PerArenaData *arenaData = getPerArenaData(arena);

	LLEnum(&(arenaData->storeList), afree); //can simply free all the Store structs

	lm->LogA(L_DRIVEL, "hscore_database", arena, "Freed %i stores.", LLCount(&(arenaData->storeList)));

	LLEmpty(&(arenaData->storeList));
}

local void UnloadItemListEnumCallback(const void *ptr)
{
	Item *item = (Item*)ptr;

	LLEnum(&(item->propertyList), afree);
	LLEnum(&(item->eventList), afree);

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
	//FIXME
}

local void LoadPlayerShips(Player *p, Arena *arena) //fetch ships from MySQL
{
	//FIXME
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
	mysql->Query(loadItemsQueryCallback, NULL, 1, "SELECT id, name, short_description, long_description, buy_price, sell_price, exp_required, ships_allowed, type1, type2, type1_delta, type2_delta, delay_write, ammo FROM hs_items");
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

local void StorePlayerGlobals(Player *p) //store player globals. MUST FINISH IN ONE QUERY
{
	//FIXME
}

local void StorePlayerShips(Player *p, Arena *arena) //store player ships. MUST FINISH IN ONE QUERY
{
	//FIXME
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
	return &(arenaData->storeList);
}

local LinkedList * getCategoryList(Arena *arena)
{
	PerArenaData *arenaData = getPerArenaData(arena);
	return &(arenaData->categoryList);
}

local void addShip(Player *p, int ship, LinkedList *itemList)
{
	//FIXME
}

local void removeShip(Player *p, int ship)
{
	//FIXME
}

//getPerPlayerData declared elsewhere

local Ihscoredatabase interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_DATABASE, "hscore_database")
	areShipsLoaded, isLoaded, getItemList, getStoreList,
	getCategoryList, addShip, removeShip, getPerPlayerData,
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
