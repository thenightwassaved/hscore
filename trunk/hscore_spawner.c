#include <string.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_storeman.h"
#include "hscore_database.h"
#include "hscore_shipnames.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Ihscoreitems *items;
local Iclientset *clientset;
local Ihscoredatabase *database;

local override_key_t

local void loadOverrides()
{
	 clientset->GetOverrideKey(const char *section, const char *key);
}

local void recalcSettings(Player *p)
{
repel
burst
thor
portal
decoy
brick
rocket
}

local void playerActionCallback(Player *p, int action, Arena *arena)
{
	else if (action == PA_ENTERARENA)
	{
		//the player is entering the arena.

		recalcSettings(p);
	}
	else if (action == PA_LEAVEARENA)
	{
		//do nothing?
	}
}

EXPORT int MM_hscore_spawner(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		items = mm->GetInterface(I_HSCORE_ITEMS, ALLARENAS);
		clientset = mm->GetInterface(I_CLIENTSET, ALLARENAS);
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);

		if (!lm || !chat || !cfg || !items || !clientset || !database)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(items);
			mm->ReleaseInterface(clientset);
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
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(clientset);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		return MM_OK;
	}
	return MM_FAIL;
}
