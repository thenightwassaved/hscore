#include <string.h>
#include <stdlib.h>

#include "../asss.h"
#include "hscore_database.h"
#include "hscore_money.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Ihsmoney *money;
local Iplayerdata *pd;


EXPORT int MM_hscore_rewards(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		money = mm->GetInterface(I_HS_MONEY, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);


		if (!lm || !chat || !cfg || !money || !pd)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(money);
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
		mm->ReleaseInterface(money);
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

