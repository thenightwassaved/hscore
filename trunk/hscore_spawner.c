#include <string.h>
#include <stdlib.h>

#include "../asss.h"
#include "hscore_database.h"
#include "hscore_items.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Iconfig *cfg;
local Ihsdatabase *db;
local Ihsitems *items;
local Iplayerdata *pd;


EXPORT int MM_hscore_spawner(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		db = mm->GetInterface(I_HS_DATABASE, ALLARENAS);
		items = mm->GetInterface(I_HS_ITEMS, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);


		if (!lm || !cfg || !db || !items || !pd)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(db);
			mm->ReleaseInterface(items);
			mm->ReleaseInterface(pd);

			return MM_FAIL;
		}

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(db);
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(pd);

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

