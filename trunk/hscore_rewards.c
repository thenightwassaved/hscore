#include "asss.h"
#include "hscore.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Ihscoremoney *money;

local void flagWinCallback(Arena *arena, int freq)
{

}

local void goalCallback(Arena *arena, Player *p, int bid, int x, int y)
{

}

local void killCallback(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts, int *green)
{

}

local int getPeriodicPoints(Arena *arena, int freq, int freqplayers, int totalplayers, int flagsowned)
{

}

local Iperiodicpoints periodicInterface =
{
	INTERFACE_HEAD_INIT(I_PERIODIC_POINTS, "pp-basic")
	getPeriodicPoints
};

EXPORT int MM_hscore_rewards(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		money = mm->GetInterface(I_HSCORE_MONEY, ALLARENAS);

		if (!lm || !chat || !cfg || !money)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(money);

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

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		mm->RegInterface(&periodicInterface, arena);

		mm->RegCallback(CB_FLAGWIN, flagWinCallback, arena);
		mm->RegCallback(CB_GOAL, goalCallback, arena);
		mm->RegCallback(CB_KILL, killCallback, arena);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		mm->UnregInterface(&periodicInterface, arena);

		mm->UnregCallback(CB_FLAGWIN, flagWinCallback, arena);
		mm->UnregCallback(CB_GOAL, goalCallback, arena);
		mm->UnregCallback(CB_KILL, killCallback, arena);

		return MM_OK;
	}
	return MM_FAIL;
}

