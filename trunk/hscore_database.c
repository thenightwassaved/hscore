#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;

//interface prototypes
local int areShipsLoaded(Player *p);
local int isLoaded(Player *p);
local LinkedList * getItemList();
local LinkedList * getStoreList(Arena *arena);
local LinkedList * getCategoryList(Arena *arena);
local void addShip(Player *p, int ship, linkedList *itemList);
local void removeShip(Player *p, int ship);
local PlayerData * getPlayerData(Player *p);

//keys
local int playerDataKey;
local int arenaDataKey;

//lists
local LinkedList itemList;
local LinkedList itemTypeList;

local void InitPlayerData(Player *p) //called before the player data is touched
{

}

local void CleanupPlayerData(Player *p) //called to free any allocated data
{

}

local void LoadPlayerGlobals(Player *p) //fetch globals from MySQL
{

}

local void StorePlayerGlobals(Player *p) //store player globals
{

}

local void InitArenaData(Arena *arena) //called before the arena data is touched
{

}

local void CleanupArenaData(Arena *arena) //called when the arena is about to die
{

}

local void UnloadCategoryList(Arena *arena)
{

}

local void UnloadStoreList(Arena *arena)
{

}

local void UnloadItemList()
{

}

local void UnloadItemTypeList()
{

}

local void LoadCategoryList(Arena *arena)
{

}

local void LoadStoreList(Arena *arena)
{

}

local void LoadItemList()
{

}

local void LoadItemTypeList() //will call LoadItemList() when finished loading
{

}

local helptext_t reloadItemsHelp =
"Targets: none\n"
"Args: none\n"
"This command will reload all current and new items from the database.\n"
"NOTE: This command will *NOT* remove items no longer in the database.\n";

local void reloadItemsCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t storeAllHelp =
"Targets: none\n"
"Args: none\n"
"Stores all loaded player information back into MySQL.\n";

local void storeAllCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local int areShipsLoaded(Player *p)
{

}

local int isLoaded(Player *p)
{

}

local LinkedList * getItemList()
{

}

local LinkedList * getStoreList(Arena *arena)
{

}

local LinkedList * getCategoryList(Arena *arena)
{

}

local void addShip(Player *p, int ship, linkedList *itemList)
{

}

local void removeShip(Player *p, int ship)
{

}

local PlayerData * getPlayerData(Player *p)
{

}

local Ihscoredatabase interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_DATABASE, "hscore_database")
	areShipsLoaded, isLoaded, getItemList, getStoreList,
	getCategoryList, addShip, removeShip, getPlayerData,
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

		if (!lm || !chat || !cfg || !cmd)
		{
			goto fail;
		}

		playerDataKey = pd->AllocatePlayerData(sizeof(PlayerData));
		if (playerDataKey == -1)
		{
			goto fail;
		}

		arenaDataKey = aman->AllocateArenaData(sizeof(ArenaData));
		if (arenaDataKey == -1)
		{
			pd->FreePlayerData(playerDataKey);
			goto fail;
		}

		LLInit(&itemList);
		LLInit(&itemTypeList);

		//load itemTypes (which leads to loading items)

		//register arena callbacks

		mm->RegInterface(&interface, ALLARENAS);

		cmd->AddCommand("reloaditems", reloadItemsCommand, ALLARENAS, reloadItemsHelp);
		cmd->AddCommand("storeall", storeAllCommand, ALLARENAS, storeAllHelp);

		return MM_OK;

	fail:
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
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

		//unregister arena callbacks

		//save and unload all player data

		//cleanup all arena data (stores and categories)

		//cleanup itemList

		//cleanup itemTypeList

		pd->FreePlayerData(playerDataKey);
		aman->FreeArenaData(arenaDataKey);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);

		return MM_OK;
	}
	return MM_FAIL;
}

