#include <string.h>
#include <stdlib.h>

#include "../asss.h"
#include "hscore_database.h"
#include "hscore_map.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihsdatabase *db;

local int ItemAvalible(Player *p, Item *item);
local LinkedList * GetAvalibleStores(Player *p, Item *item);


local int ItemAvalible(Player *p, Item *item)
{
	return 0;
}

local LinkedList * GetAvalibleStores(Player *p, Item *item)
{
	return NULL;
}

local Ihsmap myint =
{
	INTERFACE_HEAD_INIT(I_HS_MAP, "hscore_map")
	ItemAvalible, GetAvalibleStores,
};

EXPORT int MM_hscore_map(int action, Imodman *_mm, Arena *arena)
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

		mm->RegInterface(&myint, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&myint, ALLARENAS))
			return MM_FAIL;

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(db);

		return MM_OK;
	}
	return MM_FAIL;
}
