

#include <stdlib.h>
#include <string.h>

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
local int hash_enum_remove(const char *key, void *val, void *d);
local PerArenaData * getPerArenaData(Arena *arena);
local Item * getItemByID(int id);
local Item * getItemByIDNoLock(int id);
local ItemType * getItemTypeByID(int id);
local ItemType * getItemTypeByIDNoLock(int id);
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
local void lock();
local void unlock();
local void updateItem(Player *p, int ship, Item *item, int newCount, int newData);
local void updateItemNoLock(Player *p, int ship, Item *item, int newCount, int newData);
local void updateInventoryNoLock(Player *p, int ship, InventoryEntry *entry, int newCount, int newData);
local void addShip(Player *p, int ship);
local void removeShip(Player *p, int ship);
local PerPlayerData *getPerPlayerData(Player *p);

//keys
local int playerDataKey;
local int arenaDataKey;

//lists
local LinkedList itemList;
local LinkedList itemTypeList;

//mutex
local pthread_mutex_t itemMutex = PTHREAD_MUTEX_INITIALIZER;

//+-------------------------+
//|                         |
//| Miscellaneous Functions |
//|                         |
//+-------------------------+
local int hash_enum_remove(const char *key, void *val, void *d)
{
	return 1;
}

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
	Item *returnValue = NULL;

	lock();
	for (link = LLGetHead(&itemList); link; link = link->next)
	{
		Item *item = link->data;

		if (item->id == id)
		{
			returnValue = item;
			break;
		}
	}
	unlock();

	return returnValue;
}

local Item * getItemByIDNoLock(int id)
{
	Link *link;
	Item *returnValue = NULL;

	for (link = LLGetHead(&itemList); link; link = link->next)
	{
		Item *item = link->data;

		if (item->id == id)
		{
			returnValue = item;
			break;
		}
	}

	return returnValue;
}

local ItemType * getItemTypeByID(int id)
{
	Link *link;
	ItemType *returnValue = NULL;

	lock();
	for (link = LLGetHead(&itemTypeList); link; link = link->next)
	{
		ItemType *itemType = link->data;

		if (itemType->id == id)
		{
			returnValue = itemType;
			break;
		}
	}
	unlock();

	return returnValue;
}

local ItemType * getItemTypeByIDNoLock(int id)
{
	Link *link;
	ItemType *returnValue = NULL;

	for (link = LLGetHead(&itemTypeList); link; link = link->next)
	{
		ItemType *itemType = link->data;

		if (itemType->id == id)
		{
			returnValue = itemType;
			break;
		}
	}

	return returnValue;
}

//+--------------------------+
//|                          |
//|  Misc Utility Functions  |
//|                          |
//+--------------------------+

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

//+------------------------+
//|                        |
//|  Post-Query Functions  |
//|                        |
//+------------------------+

local void LinkAmmo()
{
	Link *link;

	lock();
	for (link = LLGetHead(&itemList); link; link = link->next)
	{
		Item *item = link->data;

		if (item->ammoID != 0)
		{
			Item *ammo;
			//unlock();
			//ammo = getItemByID(item->ammoID);
			//lock();
			ammo = getItemByIDNoLock(item->ammoID);

			item->ammo = ammo;
			LLAdd(&ammo->ammoUsers, item);

			if (item->ammo == NULL)
			{
				lm->Log(L_ERROR, "<hscore_database> No ammo matched id %i requested by item id %i.", item->ammoID, item->id);
			}
		}
	}
	unlock();
}

//+----------------------------------+
//|                                  |
//|  MySQL Table Creation Functions  |
//|                                  |
//+----------------------------------+

#define CREATE_CATEGORIES_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_categories` (" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  `name` varchar(32) NOT NULL default ''," \
"  `description` varchar(64) NOT NULL default ''," \
"  `arena` varchar(32) NOT NULL default ''," \
"  `order` tinyint(4) NOT NULL default '0'," \
"  `hidden` tinyint(4) NOT NULL default '0'," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_CATEGORY_ITEMS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_category_items` (" \
"  `item_id` int(10) unsigned NOT NULL default '0'," \
"  `category_id` int(10) unsigned NOT NULL default '0'," \
"  `order` tinyint(4) NOT NULL default '0'" \
")"

#define CREATE_ITEM_EVENTS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_item_events` (" \
"  `item_id` int(10) unsigned NOT NULL default '0'," \
"  `event` varchar(16) NOT NULL default ''," \
"  `action` mediumint(9) NOT NULL default '0'," \
"  `data` int(11) NOT NULL default '0'," \
"  `message` varchar(200) NOT NULL default ''" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_ITEM_PROPERTIES_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_item_properties` (" \
"  `item_id` int(10) unsigned NOT NULL default '0'," \
"  `name` varchar(32) NOT NULL default ''," \
"  `value` int(11) NOT NULL default '0'" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_ITEM_TYPES_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_item_types` (" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  `name` varchar(32) NOT NULL default ''," \
"  `max` int(11) NOT NULL default '0'," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_ITEMS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_items` (" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  `name` varchar(16) NOT NULL default ''," \
"  `short_description` varchar(32) NOT NULL default ''," \
"  `long_description` varchar(200) NOT NULL default ''," \
"  `buy_price` int(11) NOT NULL default '0'," \
"  `sell_price` int(11) NOT NULL default '0'," \
"  `exp_required` int(11) NOT NULL default '0'," \
"  `ships_allowed` int(11) NOT NULL default '0'," \
"  `item_types` varchar(250) NOT NULL default ''," \
"  `max` int(11) NOT NULL default '0'," \
"  `delay_write` tinyint(4) NOT NULL default '0'," \
"  `ammo` int(10) unsigned NOT NULL default '0'," \
"  `affects_sets` tinyint(4) NOT NULL default '0'," \
"  `resend_sets` tinyint(4) NOT NULL default '0'," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_PLAYER_SHIP_ITEMS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_player_ship_items` (" \
"  `ship_id` int(10) unsigned NOT NULL default '0'," \
"  `item_id` int(10) unsigned NOT NULL default '0'," \
"  `count` int(11) NOT NULL default '0'," \
"  `data` int(11) NOT NULL default '0'," \
"  PRIMARY KEY  (`ship_id`,`item_id`)" \
")"

#define CREATE_PLAYER_SHIPS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_player_ships` (" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  `player_id` int(10) unsigned NOT NULL default '0'," \
"  `ship` tinyint(4) NOT NULL default '0'," \
"  `arena` varchar(32) NOT NULL default ''," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_PLAYERS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_players` (" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  `name` varchar(32) NOT NULL default ''," \
"  `money` int(11) NOT NULL default '0'," \
"  `exp` int(11) NOT NULL default '0'," \
"  `money_give` int(11) NOT NULL default '0'," \
"  `money_grant` int(11) NOT NULL default '0'," \
"  `money_buysell` int(11) NOT NULL default '0'," \
"  `money_kill` int(11) NOT NULL default '0'," \
"  `money_flag` int(11) NOT NULL default '0'," \
"  `money_ball` int(11) NOT NULL default '0'," \
"  `money_event` int(11) NOT NULL default '0'," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_STORE_ITEMS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_store_items` (" \
"  `item_id` int(10) unsigned NOT NULL default '0'," \
"  `store_id` int(10) unsigned NOT NULL default '0'," \
"  `order` tinyint(4) NOT NULL default '0'" \
")"

#define CREATE_STORES_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_stores` (" \
"  `id` int(10) unsigned NOT NULL auto_increment," \
"  `name` varchar(32) NOT NULL default ''," \
"  `description` varchar(200) NOT NULL default ''," \
"  `region` varchar(16) NOT NULL default ''," \
"  `arena` varchar(32) NOT NULL default ''," \
"  `order` tinyint(4) NOT NULL default '0'," \
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_TRANSACTIONS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_transactions` (" \
"  `id` int(10) NOT NULL auto_increment," \
"  `srcplayer` int(11) NOT NULL default '0'," \
"  `tgtplayer` int(11) NOT NULL default '0'," \
"  `action` tinyint(4) NOT NULL default '0', " \
"  `amount` int(11) NOT NULL default '0'," \
"  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,  "\
"  PRIMARY KEY  (`id`)" \
")"

#define CREATE_PLAYER_POINTS_TABLE \
"CREATE TABLE IF NOT EXISTS `hs_player_points` (" \
"  `player_id` int(10) unsigned NOT NULL default '0'," \
"  `arena` varchar(32) NOT NULL," \
"  `points_global` int(11) default '0'," \
"  `points_1` int(11) default NULL," \
"  `points_2` int(11) default NULL," \
"  `points_3` int(11) default NULL," \
"  `points_4` int(11) default NULL," \
"  `points_5` int(11) default NULL," \
"  `points_6` int(11) default NULL," \
"  `points_7` int(11) default NULL," \
"  `points_8` int(11) default NULL," \
"  UNIQUE KEY `index` (`player_id`,`arena`)" \
")"

local void initTables()
{
	mysql->Query(NULL, NULL, 0, CREATE_CATEGORIES_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_CATEGORY_ITEMS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_ITEM_EVENTS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_ITEM_PROPERTIES_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_ITEM_TYPES_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_ITEMS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_PLAYER_SHIP_ITEMS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_PLAYER_SHIPS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_PLAYERS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_STORE_ITEMS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_STORES_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_TRANSACTIONS_TABLE);
	mysql->Query(NULL, NULL, 0, CREATE_PLAYER_POINTS_TABLE);
}

//+-------------------------+
//|                         |
//|  MySQL Query Callbacks  |
//|                         |
//+-------------------------+

local void loadPropertiesQueryCallback(int status, db_res *result, void *passedData)
{
	Player *p;
	Link *link;
	int results;
	int x;
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


	lock();

	for (link = LLGetHead(&itemList); link; link = link->next)
	{
		Item *i = link->data;
		LLEnum(&i->propertyList, afree);
		LLEmpty(&i->propertyList);
	}

	pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		PerPlayerData *playerData = getPerPlayerData(p);
		for (x = 0; x < 8; ++x)
		{
			if (!playerData->hull[x])
				continue;

			HashEnum(playerData->hull[x]->propertySums, hash_enum_afree, 0);
			HashEnum(playerData->hull[x]->propertySums, hash_enum_remove, 0);
		}
	}
	pd->Unlock();

	while ((row = mysql->GetRow(result)))
	{
		int itemID = atoi(mysql->GetField(row, 0));					//item_id
		Item *item;

		item = getItemByIDNoLock(itemID);

		if (item != NULL)
		{
			Property *property = NULL;
			/*for (link = LLGetHead(&item->propertyList); link; link = link->next)
			{
				if (!strcmp(link->data->name, mysql->GetField(row, 1)))
				{
					property = link->data;
					break;
				}
			}*/

			//if the property doesn't exist in this item yet, create it. otherwise, just change the value.
			if (!property)
			{
				property = amalloc(sizeof(*property));
				astrncpy(property->name, mysql->GetField(row, 1), 17);
				LLAdd(&item->propertyList, property);
			}

			property->value = atoi(mysql->GetField(row, 2));		//value

		}
		else
		{
			lm->Log(L_ERROR, "<hscore_database> property looking for item ID %i.", itemID);
		}
	}
	unlock();

	DO_CBS(CB_HS_ITEMRELOAD, ALLARENAS, HSItemReload, ());

	lm->Log(L_DRIVEL, "<hscore_database> %i properties were loaded from MySQL.", results);

}

local void loadEventsQueryCallback(int status, db_res *result, void *passedData)
{
	Link *link;
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

	lock();
	for (link = LLGetHead(&itemList); link; link = link->next)
	{
		Item *i = link->data;
		LLEnum(&i->eventList, afree);
		LLEmpty(&i->eventList);
	}

	while ((row = mysql->GetRow(result)))
	{
		int itemID = atoi(mysql->GetField(row, 0));					//item_id
		Item *item;
		//unlock();
		//item = getItemByID(itemID);
		//lock();
		item = getItemByIDNoLock(itemID);

		if (item != NULL)
		{
			Event *event = NULL;
			/*for (link = LLGetHead(&item->eventList); link; link = link->next)
			{
				if (!strcmp(link->data->event, mysql->GetField(row, 1)))
				{
					event = link->data;
					break;
				}
			}*/

			//if the event doesn't exist in this item yet, create it. otherwise, just change the values.
			if (!event)
			{
				event = amalloc(sizeof(*event));
				astrncpy(event->event, mysql->GetField(row, 1), 17);
				LLAdd(&item->eventList, event);
			}



			event->action = atoi(mysql->GetField(row, 2));			//action
			event->data = atoi(mysql->GetField(row, 3));			//data
			astrncpy(event->message, mysql->GetField(row, 4), 201);	//message


		}
		else
		{
			lm->Log(L_ERROR, "<hscore_database> event looking for item ID %i.", itemID);
		}
	}
	unlock();

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
		Link *link;
		Item *item = NULL;
		int id = atoi(mysql->GetField(row, 0));

		char itemTypes[256];

		lock();
		for (link = LLGetHead(&itemList); link; link = link->next)
		{
			Item *i = link->data;
			if (i->id == id)
			{
				item = i;
				break;
			}
		}

		//if the Item doesn't exist yet, create it. otherwise, just change the values.
		if (!item)
		{
			item = amalloc(sizeof(*item));
			LLInit(&item->propertyList);
			LLInit(&item->eventList);
			LLInit(&item->itemTypeEntries);
			LLInit(&item->ammoUsers);
			item->id = id;
			LLAdd(&itemList, item);
		}
		else
		{
			//empty the item types
			LLEnum(&item->itemTypeEntries, afree);
			LLEmpty(&item->itemTypeEntries);
		}

		astrncpy(item->name, mysql->GetField(row, 1), 17);			//name
		astrncpy(item->shortDesc, mysql->GetField(row, 2), 33);		//short_description
		astrncpy(item->longDesc, mysql->GetField(row, 3), 201);		//long_description
		item->buyPrice = atoi(mysql->GetField(row, 4));				//buy_price
		item->sellPrice = atoi(mysql->GetField(row, 5));			//sell_price
		item->expRequired = atoi(mysql->GetField(row, 6));			//exp_required
		item->shipsAllowed = atoi(mysql->GetField(row, 7));			//ships_allowed

		astrncpy(itemTypes, mysql->GetField(row, 8), 255);			//item_types

		item->max = atoi(mysql->GetField(row, 9));					//max

		item->delayStatusWrite = atoi(mysql->GetField(row, 10));	//delay_write
		item->ammoID = atoi(mysql->GetField(row, 11));				//ammo
		item->affectsSets = atoi(mysql->GetField(row, 12));			//affects_sets
		item->resendSets = atoi(mysql->GetField(row, 13));			//resend_sets

		if (itemTypes != NULL) //parse itemTypes
		{
			const char *tmp = NULL;
			char word[64];
			while (strsplit(itemTypes, ",", word, sizeof(word), &tmp))
			{
				//items should be in the format "item type id" or "item type id:count"
				char *colonLoc = strchr(word, ':');
				if (colonLoc == NULL)
				{
					//no count included
					//word should be just "item type id"
					ItemType *itemType = getItemTypeByIDNoLock(atoi(word));
					if (itemType != NULL)
					{
						ItemTypeEntry *entry = amalloc(sizeof(*entry));
						entry->itemType = itemType;
						entry->delta = 1;

						LLAdd(&item->itemTypeEntries, entry);
					}
					else
					{
						lm->Log(L_ERROR, "<hscore_database> bad item type %s on item %s", word, item->name);
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
						if (count != 0)
						{
							ItemType *itemType = getItemTypeByIDNoLock(atoi(word));
							if (item != NULL)
							{
								ItemTypeEntry *entry = amalloc(sizeof(*entry));
								entry->itemType = itemType;
								entry->delta = count;

								LLAdd(&item->itemTypeEntries, entry);
							}
							else
							{
								lm->Log(L_ERROR, "<hscore_database> bad item type %s on item %s", word, item->name);
							}
						}
						else
						{
							lm->Log(L_ERROR, "<hscore_database> initial count of %d for %s on %s", count, word, item->name);
						}
					}
					else
					{
						lm->Log(L_ERROR, "<hscore_database> colon on item type %s not followed by a string on item %s", word, item->name);
					}
				}
			}
		}

		unlock();

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
		int id = atoi(mysql->GetField(row, 0));
		Link *link;
		ItemType *itemType = NULL;
		lock();
		for (link = LLGetHead(&itemTypeList); link; link = link->next)
		{
			ItemType *it = link->data;
			if (it->id == id)
			{
				itemType = it;
				break;
			}
		}

		//if the ItemType doesn't exist yet, create it, otherwise just change the values
		if (!itemType)
		{
			itemType = amalloc(sizeof(*itemType));
			itemType->id = id;
			LLAdd(&itemTypeList, itemType);
		}

		astrncpy(itemType->name, mysql->GetField(row, 1), 33);	//name
		itemType->max = atoi(mysql->GetField(row, 2));			//max

		unlock();
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
		/* cfghelp: Hyperspace:InitialMoney, global, int, mod: hscore_database
		 * The amount of money that is given to a new player. */
		int initialMoney = cfg->GetInt(GLOBAL, "hyperspace", "initialmoney", 1000);
		/* cfghelp: Hyperspace:InitialExp, global, int, mod: hscore_database
		 * The amount of exp that is given to a new player. */
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
	int id;
	int ship;
	ShipHull *hull;

	Player *p = passedData;
	PerPlayerData *playerData = getPerPlayerData(p);

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

	id = atoi(mysql->GetField(row, 0));
	ship = atoi(mysql->GetField(row, 1));


	hull = playerData->hull[ship];

	hull->id = id;

	DO_CBS(CB_SHIP_ADDED, p->arena, ShipAdded, (p, ship));
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

					lock();
					LLAdd(&hull->inventoryEntryList, inventoryEntry);
					unlock();
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

	DO_CBS(CB_SHIPS_LOADED, p->arena, ShipsLoaded, (p));

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
			hull->propertySums = HashAlloc();

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
				lock();
				LLAdd(&store->itemList, item);
				unlock();
				break;
			}
		}

		//bad store id will give no error!
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

	lock();
	while ((row = mysql->GetRow(result)))
	{
		Store *store = NULL;
		Link *link;
		int id = atoi(mysql->GetField(row, 0));

		for (link = LLGetHead(&arenaData->storeList); link; link = link->next)
		{
			Store *s = link->data;
			if (s->id == id)
			{
				store = s;
				break;
			}
		}

		//if the Store does not exist in this arena yet, create it. otherwise just modify the values, and reset the item lists.
		if (!store)
		{
			store = amalloc(sizeof(*store));
			LLInit(&store->itemList);
			store->id = id;
			store->name[0] = 0;
			store->description[0] = 0;
			store->region[0] = 0;
			LLAdd(&arenaData->storeList, store);
		}
		else
		{
			LLEmpty(&store->itemList);
		}

		astrncpy(store->name, mysql->GetField(row, 1), 33);			//name
		astrncpy(store->description, mysql->GetField(row, 2), 201);	//description
		astrncpy(store->region, mysql->GetField(row, 3), 17);		//region
	}
	unlock();

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
				lock();
				LLAdd(&category->itemList, item);
				unlock();
				break;
			}
		}

		//bad category id will give no error!
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

	lock();
	while ((row = mysql->GetRow(result)))
	{
		Category *category = NULL;
		Link *link;
		int id = atoi(mysql->GetField(row, 0));

		for (link = LLGetHead(&arenaData->categoryList); link; link = link->next)
		{
			Category *c = link->data;
			if (c->id == id)
			{
				category = c;
				break;
			}
		}

		//if the Category doesn't exist, create it, otherwise just change the values and reset the lists
		if (!category)
		{
			category = amalloc(sizeof(*category));
			LLInit(&category->itemList);
			category->id = id;
			category->name[0] = 0;
			category->description[0] = 0;
			category->hidden = 0;
			LLAdd(&arenaData->categoryList, category);
		}
		else
		{
			LLEmpty(&category->itemList);
		}

		astrncpy(category->name, mysql->GetField(row, 1), 33);			//name
		astrncpy(category->description, mysql->GetField(row, 2), 65);	//description
		category->hidden = atoi(mysql->GetField(row, 3));				//hidden
	}
	unlock();

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
	int i;

	playerData->loaded = 0;
	playerData->shipsLoaded = 0;

	for (i = 0; i < 8; i++)
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

local void UnloadPlayerShip(ShipHull *ship) //call with lock() held
{
	LLEnum(&ship->inventoryEntryList, afree);
	LLEmpty(&ship->inventoryEntryList);

	HashEnum(ship->propertySums, hash_enum_afree, NULL);
	HashFree(ship->propertySums);

	afree(ship);
}

local void UnloadPlayerShips(Player *p) //called to free any allocated data
{
	PerPlayerData *playerData = getPerPlayerData(p);
	int i;

	lock();

	playerData->shipsLoaded = 0;

	for (i = 0; i < 8; i++)
	{
		if (playerData->hull[i] != NULL)
		{
			UnloadPlayerShip(playerData->hull[i]);
			playerData->hull[i] = NULL;
		}
	}

	unlock();

	lm->LogP(L_DRIVEL, "hscore_database", p, "Freed ship data.");
}

local void UnloadCategoryList(Arena *arena) //called when the arena is about to die
{
	PerArenaData *arenaData = getPerArenaData(arena);

	lock();
	LLEnum(&arenaData->categoryList, afree); //can simply free all the Category structs

	lm->LogA(L_DRIVEL, "hscore_database", arena, "Freed %i categories.", LLCount(&arenaData->categoryList));

	LLEmpty(&arenaData->categoryList);
	unlock();
}

local void UnloadStoreList(Arena *arena)
{
	PerArenaData *arenaData = getPerArenaData(arena);

	lock();
	LLEnum(&arenaData->storeList, afree); //can simply free all the Store structs

	lm->LogA(L_DRIVEL, "hscore_database", arena, "Freed %i stores.", LLCount(&arenaData->storeList));

	LLEmpty(&arenaData->storeList);
	unlock();
}

local void UnloadItemListEnumCallback(const void *ptr) //called with lock held
{
	Item *item = (Item*)ptr;

	LLEnum(&item->propertyList, afree);
	LLEmpty(&item->propertyList);

	LLEnum(&item->eventList, afree);
	LLEmpty(&item->eventList);

	LLEnum(&item->itemTypeEntries, afree);
	LLEmpty(&item->itemTypeEntries);

	afree(item);
}

local void UnloadItemList()
{
	lock();
	LLEnum(&itemList, UnloadItemListEnumCallback);

	lm->Log(L_DRIVEL, "<hscore_database> Freed %i items.", LLCount(&itemList));

	LLEmpty(&itemList);
	unlock();
}

local void UnloadItemTypeList()
{
	lock();
	LLEnum(&itemTypeList, afree); //can simply free all the ItemType structs

	lm->Log(L_DRIVEL, "<hscore_database> Freed %i item types.", LLCount(&itemTypeList));

	LLEmpty(&itemTypeList);
	unlock();
}

local void UnloadAllPerArenaData()
{
	Arena *arena;
	Link *link;

	aman->Lock();
	FOR_EACH_ARENA(arena)
	{
		UnloadStoreList(arena);
		UnloadCategoryList(arena);
	}
	aman->Unlock();
}

local void UnloadAllPerPlayerData()
{
	Player *p;
	Link *link;
	pd->Lock();
	FOR_EACH_PLAYER(p)
		if (isLoaded(p))
		{
			UnloadPlayerShips(p);
			UnloadPlayerGlobals(p);
		}
	pd->Unlock();
}

//+------------------+
//|                  |
//|  Load Functions  |
//|                  |
//+------------------+

local void LoadPlayerGlobals(Player *p) //fetch globals from MySQL
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
	mysql->Query(loadCategoryItemsQueryCallback, arena, 1, "SELECT item_id, category_id FROM hs_category_items, hs_categories WHERE category_id = id AND arena = ? ORDER BY hs_category_items.order ASC", getArenaIdentifier(arena));
}

local void LoadCategoryList(Arena *arena) //leads to LoadCategoryItems() being called
{
	mysql->Query(loadArenaCategoriesQueryCallback, arena, 1, "SELECT id, name, description, hidden FROM hs_categories WHERE arena = ? ORDER BY hs_categories.order ASC", getArenaIdentifier(arena));
}

local void LoadStoreItems(Arena *arena)
{
	mysql->Query(loadStoreItemsQueryCallback, arena, 1, "SELECT item_id, store_id FROM hs_store_items, hs_stores WHERE store_id = id AND arena = ? ORDER BY hs_store_items.order ASC", getArenaIdentifier(arena));
}

local void LoadStoreList(Arena *arena) //leads to LoadStoreItems() being called
{
	mysql->Query(loadArenaStoresQueryCallback, arena, 1, "SELECT id, name, description, region FROM hs_stores WHERE arena = ? ORDER BY hs_stores.order ASC", getArenaIdentifier(arena));
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
	mysql->Query(loadItemsQueryCallback, NULL, 1, "SELECT id, name, short_description, long_description, buy_price, sell_price, exp_required, ships_allowed, item_types, max, delay_write, ammo, affects_sets, resend_sets FROM hs_items");
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
		lock();

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

		unlock();
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
		int i;
		lock();

		for (i = 0; i < 8; i++)
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
						shipID = playerData->hull[i]->id; //fixme: bad
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

		unlock();
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to store unloaded ships");
	}
}

local void StoreAllPerPlayerData()
{
	Player *p;
	Link *link;
	pd->Lock();
	FOR_EACH_PLAYER(p)
		if (isLoaded(p))
		{
			if (areShipsLoaded(p))
			{
				StorePlayerShips(p, p->arena);
			}
			StorePlayerGlobals(p);
		}
	pd->Unlock();
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
"It will also reload all arenas stores and categories.\n"
"NOTE: This command will *NOT* remove items no longer in the database.\n";

local void reloadItemsCommand(const char *command, const char *params, Player *p, const Target *target)
{
	Link *link;
	Arena *a;
	chat->SendMessage(p, "Reloading items...");

	LoadItemTypeList();
	chat->SendMessage(p, "...Ran items queries.");

	aman->Lock();
	FOR_EACH_ARENA(a)
	{
		LoadStoreList(a);
		LoadCategoryList(a);
		chat->SendMessage(p, "...Ran stores/categories queries for arena '%s'", a->name);
	}
	aman->Unlock();

	chat->SendMessage(p, "Ran all queries.");
}

local helptext_t refundHelp =
"Targets: none\n"
"Args: none\n"
"This command will check all item ship limitations and refund\n"
"max(buy price, sell price). It checks both online and offline players.\n";

local void refundCommand(const char *command, const char *params, Player *p, const Target *target)
{
	LinkedList list;
	Link *link;
	Player *i;

	chat->SendMessage(p, "Refunding items of online players...");

	LLInit(&list);

	pd->Lock();
	lock();
	FOR_EACH_PLAYER(i)
		if (isLoaded(i))
		{
			if (areShipsLoaded(i))
			{
				int ship;
				PerPlayerData *playerData = getPerPlayerData(i);

				for (ship = 0; ship < 8; ship++)
				{
					if (playerData->hull[ship] != NULL)
					{
						LinkedList *inventoryList = &playerData->hull[ship]->inventoryEntryList;

						Link *item_link;
						for (item_link = LLGetHead(inventoryList); item_link; item_link = item_link->next)
						{
							InventoryEntry *entry = item_link->data;
							if (!(entry->item->shipsAllowed & 1 << ship))
							{
								//not allowed
								int price = entry->item->buyPrice > entry->item->sellPrice?entry->item->buyPrice:entry->item->sellPrice; //max
								playerData->money += price * entry->count;
								playerData->moneyType[MONEY_TYPE_BUYSELL] += price * entry->count;
								LLAdd(&list, entry);
							}
						}

						//now remove all of the items in the list
						for (item_link = LLGetHead(&list); item_link; item_link = item_link->next)
						{
							InventoryEntry *entry = item_link->data;
							Item *item = entry->item;

							int shipID = playerData->hull[ship]->id;
							int oldCount = entry->count;

							mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ship_items WHERE ship_id = # AND item_id = #", shipID, item->id);

							LLRemove(inventoryList, entry);
							afree(entry);

							DO_CBS(CB_ITEM_COUNT_CHANGED, i->arena, ItemCountChanged, (i, item, NULL, 0, oldCount));
						}

						LLEmpty(&list);
					}
				}
			}
		}
	unlock();
	pd->Unlock();

	chat->SendMessage(p, "Refunding items of offline players...");

	mysql->Query(NULL, NULL, 0, "UPDATE hs_players, (SELECT player_id, SUM(price) as money FROM (SELECT hs_players.id as player_id, GREATEST(hs_items.buy_price, hs_items.sell_price) as price, hs_items.id as item_id FROM hs_players INNER JOIN (hs_player_ships, hs_player_ship_items, hs_items) ON (hs_player_ships.player_id = hs_players.id AND hs_player_ship_items.ship_id = hs_player_ships.id AND hs_player_ship_items.item_id = hs_items.id) WHERE ((1 << hs_player_ships.ship) & hs_items.ships_allowed) = 0) as temp GROUP BY player_id) AS to_update SET hs_players.money = hs_players.money + to_update.money, hs_players.money_buysell = hs_players.money.buy_sell + to_update.money WHERE hs_players.id = to_update.player_id;");
	mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ship_items USING hs_items, hs_players, hs_player_ship_items, hs_player_ships WHERE hs_player_ship_items.ship_id = hs_player_ships.id AND hs_players.id = hs_player_ships.player_id AND hs_items.id = hs_player_ship_items.item_id AND ((1 << hs_player_ships.ship) & hs_items.ships_allowed) = 0;");

	chat->SendMessage(p, "Done!");
}

local helptext_t storeAllHelp =
"Targets: none\n"
"Args: none\n"
"Stores all loaded player information back into MySQL.\n";

local void storeAllCommand(const char *command, const char *params, Player *p, const Target *target)
{
	StoreAllPerPlayerData();
	chat->SendMessage(p, "Executed.");
}

local helptext_t resetHelp =
"Targets: none or player\n"
"Args: none\n"
"Resets money, exp, items, ships and score. No undo!\n";

local void resetCommand(const char *command, const char *params, Player *p, const Target *target)
{
	Player *t = (target->type == T_PLAYER) ? target->u.p : p;
	PerPlayerData *playerData = getPerPlayerData(t);
	Istats *stats;
	int i;

	if (isLoaded(t))
	{
		lock();
		//do money+exp first

		//insert a new player into MySQL and then get it
		/* cfghelp: Hyperspace:InitialMoney, global, int, mod: hscore_database
		 * The amount of money that is given to a new player. */
		playerData->money = cfg->GetInt(GLOBAL, "hyperspace", "initialmoney", 1000);
		/* cfghelp: Hyperspace:InitialExp, global, int, mod: hscore_database
		 * The amount of exp that is given to a new player. */
		playerData->exp = cfg->GetInt(GLOBAL, "hyperspace", "initialexp", 0);

		playerData->moneyType[MONEY_TYPE_GIVE] = 0;
		playerData->moneyType[MONEY_TYPE_GRANT] = 0;
		playerData->moneyType[MONEY_TYPE_BUYSELL] = 0;
		playerData->moneyType[MONEY_TYPE_KILL] = 0;
		playerData->moneyType[MONEY_TYPE_FLAG] = 0;
		playerData->moneyType[MONEY_TYPE_BALL] = 0;
		playerData->moneyType[MONEY_TYPE_EVENT] = 0;

		mysql->Query(NULL, NULL, 0, "UPDATE hs_players SET money = #, exp = #, money_give = 0, money_grant = 0, money_buysell = 0, money_kill = 0, money_flag = 0, money_ball = 0, money_event = 0 WHERE id = #",
			playerData->money,
			playerData->exp,
			playerData->id);

		//do ships and items now
		mysql->Query(NULL, NULL, 0, "DELETE hs_player_ships, hs_player_ship_items FROM hs_player_ships, hs_player_ship_items WHERE hs_player_ships.id = hs_player_ship_items.ship_id AND hs_player_ships.player_id = #", playerData->id);

		//unload current ships
		for (i = 0; i < 8; i++)
		{
			if (playerData->hull[i] != NULL)
			{
				ShipHull *ship = playerData->hull[i];

				LLEnum(&ship->inventoryEntryList, afree);
				LLEmpty(&ship->inventoryEntryList);
				HashEnum(ship->propertySums, hash_enum_afree, NULL);
				HashFree(ship->propertySums);

				afree(ship);

				playerData->hull[i] = NULL;
			}
		}

		unlock();

		//reset score too

		stats = mm->GetInterface(I_STATS, t->arena);
		if (stats != NULL)
		{
			stats->ScoreReset(t, INTERVAL_RESET);
			stats->SendUpdates(NULL);
			mm->ReleaseInterface(stats);
		}

		if (t != p)
		{
			chat->SendMessage(t, "Reset!");
			chat->SendMessage(p, "Player %s reset!", t->name);
		}
		else
		{
			chat->SendMessage(p, "Reset!");
		}
	}
	else
	{
		chat->SendMessage(p, "Data not loaded!");
	}
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


local int periodicStoreTimer(void *param)
{
    lm->Log(L_INFO, "<hscore_database> Storing player data.");
    StoreAllPerPlayerData();
    return 1;
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

local void lock()
{
	pthread_mutex_lock(&itemMutex);
}

local void unlock()
{
	pthread_mutex_unlock(&itemMutex);
}

local void updateItem(Player *p, int ship, Item *item, int newCount, int newData)
{
	lock();
	updateItemNoLock(p, ship, item, newCount, newData);
	unlock();
}

local void updateItemNoLock(Player *p, int ship, Item *item, int newCount, int newData)
{
	PerPlayerData *playerData = getPerPlayerData(p);
	Link *link;
	LinkedList *inventoryList;

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

	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	for (link = LLGetHead(inventoryList); link; link = link->next)
	{
		InventoryEntry *entry = link->data;

		if (entry->item == item)
		{
			if (newCount != 0)
			{
				int oldCount = entry->count;
				if (oldCount == newCount && entry->data == newData)
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
								shipID = playerData->hull[ship]->id; //fixme: bad?
							}
						}

						mysql->Query(NULL, NULL, 0, "REPLACE INTO hs_player_ship_items VALUES (#,#,#,#)", shipID, item->id, newCount, newData);
					}
				}

				if (newCount != oldCount)
				{
					DO_CBS(CB_ITEM_COUNT_CHANGED, p->arena, ItemCountChanged, (p, item, entry, newCount, oldCount));
				}
			}
			else
			{
				int shipID = playerData->hull[ship]->id;
				int oldCount = entry->count;

				if (shipID == -1)
				{
					lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

					while (shipID == -1)
					{
						shipID = playerData->hull[ship]->id; //fixme: bad?
					}
				}

				mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ship_items WHERE ship_id = # AND item_id = #", shipID, item->id);

				LLRemove(inventoryList, entry);
				afree(entry);

				DO_CBS(CB_ITEM_COUNT_CHANGED, p->arena, ItemCountChanged, (p, item, NULL, 0, oldCount));
			}

			return;
		}
	}

	//if we leave the for loop, we'll have to add it.

	if (newCount != 0)
	{
		InventoryEntry *entry;
		int shipID = playerData->hull[ship]->id;

		if (shipID == -1)
		{
			lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

			while (shipID == -1)
			{
				shipID = playerData->hull[ship]->id; //fixme: bad
			}
		}

		mysql->Query(NULL, NULL, 0, "REPLACE INTO hs_player_ship_items VALUES (#,#,#,#)", shipID, item->id, newCount, newData);

		entry = amalloc(sizeof(*entry));

		entry->count = newCount;
		entry->data = newData;
		entry->item = item;

		LLAdd(inventoryList, entry);

		//do item changed callback
		DO_CBS(CB_ITEM_COUNT_CHANGED, p->arena, ItemCountChanged, (p, item, entry, newCount, 0));
	}
	else
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to remove item %s not on ship %i", item->name, ship);
	}
}

local void updateInventoryNoLock(Player *p, int ship, InventoryEntry *entry, int newCount, int newData)
{
	PerPlayerData *playerData = getPerPlayerData(p);
	LinkedList *inventoryList;

	if (ship < 0 || 7 < ship)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to update inventory on ship %i", ship);
		return;
	}

	if (!areShipsLoaded(p))
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to update inventory on unloaded ships");
		return;
	}

	if (playerData->hull[ship] == NULL)
	{
		lm->LogP(L_ERROR, "hscore_database", p, "asked to update inventory on unowned ship %i", ship);
		return;
	}

	inventoryList = &playerData->hull[ship]->inventoryEntryList;

	if (newCount != 0)
	{
		if (entry->count == newCount && entry->data == newData)
		{
			lm->LogP(L_ERROR, "hscore_database", p, "asked to update inventory entry %s with no change", entry->item->name);
		}
		else
		{
			int oldCount = entry->count;
			entry->count = newCount;
			entry->data = newData;

			if (!entry->item->delayStatusWrite)
			{
				int shipID = playerData->hull[ship]->id;

				if (shipID == -1)
				{
					lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

					while (shipID == -1)
					{
						shipID = playerData->hull[ship]->id; //fixme: bad
					}
				}

				mysql->Query(NULL, NULL, 0, "REPLACE INTO hs_player_ship_items VALUES (#,#,#,#)", shipID, entry->item->id, newCount, newData);
			}

			if (newCount != oldCount)
			{
				DO_CBS(CB_ITEM_COUNT_CHANGED, p->arena, ItemCountChanged, (p, entry->item, entry, newCount, oldCount));
			}
		}
	}
	else
	{
		int shipID = playerData->hull[ship]->id;

		Item *item = entry->item; //save it for later
		int oldCount = entry->count; //save it for later

		if (shipID == -1)
		{
			lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

			while (shipID == -1)
			{
				shipID = playerData->hull[ship]->id; //fixme: bad
			}
		}

		mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ship_items WHERE ship_id = # AND item_id = #", shipID, entry->item->id);

		LLRemove(inventoryList, entry);
		afree(entry);

		DO_CBS(CB_ITEM_COUNT_CHANGED, p->arena, ItemCountChanged, (p, item, NULL, 0, oldCount));
	}
}

local void addShip(Player *p, int ship) //the ships id may not be valid until later
{
	PerPlayerData *playerData = getPerPlayerData(p);
	ShipHull *hull;

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

	lock();

	hull = amalloc(sizeof(*hull));

	LLInit(&hull->inventoryEntryList);
	hull->propertySums = HashAlloc();
	hull->id = -1;

	playerData->hull[ship] = hull;

	mysql->Query(NULL, NULL, 0, "INSERT INTO hs_player_ships VALUES (NULL, #, #, ?)", playerData->id, ship, getArenaIdentifier(p->arena));
	mysql->Query(loadShipIDQueryCallback, p, 1, "SELECT id, ship FROM hs_player_ships WHERE player_id = # AND ship = # AND arena = ?", playerData->id, ship, getArenaIdentifier(p->arena));

	unlock();
}

local void removeShip(Player *p, int ship)
{
	PerPlayerData *playerData = getPerPlayerData(p);
	int shipID;

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

	shipID = playerData->hull[ship]->id;

	if (shipID == -1)
	{
		lm->LogP(L_DRIVEL, "hscore_database", p, "waiting for ship id load");

		while (shipID == -1)
		{
			shipID = playerData->hull[ship]->id; //fixme: bad
		}
	}

	mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ship_items WHERE ship_id = #", shipID); //empty the ship
	mysql->Query(NULL, NULL, 0, "DELETE FROM hs_player_ships WHERE id = #", shipID);

	lock();
	UnloadPlayerShip(playerData->hull[ship]);
	unlock();

	playerData->hull[ship] = NULL;
}

//getPerPlayerData declared elsewhere

local Ihscoredatabase interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_DATABASE, "hscore_database")
	areShipsLoaded, isLoaded, getItemList, getStoreList,
	getCategoryList, lock, unlock, updateItem,
	updateItemNoLock, updateInventoryNoLock,
	addShip, removeShip, getPerPlayerData,
};

EXPORT const char info_hscore_database[] = "v1.0 Dr Brain <drbrain@gmail.com>";

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

		initTables();

		LoadItemTypeList();

		mm->RegCallback(CB_NEWPLAYER, allocatePlayerCallback, ALLARENAS);
		mm->RegCallback(CB_PLAYERACTION, playerActionCallback, ALLARENAS);
		mm->RegCallback(CB_ARENAACTION, arenaActionCallback, ALLARENAS);

		mm->RegInterface(&interface, ALLARENAS);

		cmd->AddCommand("reloaditems", reloadItemsCommand, ALLARENAS, reloadItemsHelp);
		cmd->AddCommand("storeall", storeAllCommand, ALLARENAS, storeAllHelp);
		cmd->AddCommand("resetyesiknowwhatimdoing", resetCommand, ALLARENAS, resetHelp);
		cmd->AddCommand("refund", refundCommand, ALLARENAS, refundHelp);

		ml->SetTimer(periodicStoreTimer, 30000, 30000, NULL, NULL);

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

		ml->ClearTimer(periodicStoreTimer, NULL);

		cmd->RemoveCommand("reloaditems", reloadItemsCommand, ALLARENAS);
		cmd->RemoveCommand("storeall", storeAllCommand, ALLARENAS);
		cmd->RemoveCommand("resetyesiknowwhatimdoing", resetCommand, ALLARENAS);
		cmd->RemoveCommand("refund", refundCommand, ALLARENAS);

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
