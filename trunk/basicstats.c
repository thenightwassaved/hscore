
/* dist: public */

#include "asss.h"


local Iplayerdata *pd;
local Istats *stats;


local void mypa(Player *p, int action, Arena *arena)
{
	if (action == PA_ENTERARENA)
		stats->StartTimer(p, STAT_ARENA_TOTAL_TIME);
	else if (action == PA_LEAVEARENA)
		stats->StopTimer(p, STAT_ARENA_TOTAL_TIME);
}

local void mykill(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts)
{
	stats->IncrementStat(killer, STAT_KILLS, 1);
	stats->IncrementStat(killed, STAT_DEATHS, 1);

	if (killer->p_freq == killed->p_freq)
	{
		stats->IncrementStat(killer, STAT_TEAM_KILLS, 1);
		stats->IncrementStat(killed, STAT_TEAM_DEATHS, 1);
	}

	if (flags)
	{
		stats->IncrementStat(killer, STAT_FLAG_KILLS, 1);
		stats->IncrementStat(killed, STAT_FLAG_DEATHS, 1);
	}
}


local void myfpickup(Arena *arena, Player *p, int fid, int of, int carried)
{
	if (carried)
	{
		stats->StartTimer(p, STAT_FLAG_CARRY_TIME);
		stats->IncrementStat(p, STAT_FLAG_PICKUPS, 1);
	}
}

local void mydrop(Arena *arena, Player *p, int count, int neut)
{
	stats->StopTimer(p, STAT_FLAG_CARRY_TIME);
	if (!neut)
		stats->IncrementStat(p, STAT_FLAG_DROPS, count);
	else
		stats->IncrementStat(p, STAT_FLAG_NEUT_DROPS, count);
}


local void mybpickup(Arena *arena, Player *p, int bid)
{
	stats->StartTimer(p, STAT_BALL_CARRY_TIME);
	stats->IncrementStat(p, STAT_BALL_CARRIES, 1);
}

local void mybfire(Arena *arena, Player *p, int bid)
{
	stats->StopTimer(p, STAT_BALL_CARRY_TIME);
}

local void mygoal(Arena *arena, Player *p, int bid, int x, int y)
{
	stats->IncrementStat(p, STAT_BALL_GOALS, 1);
}


EXPORT int MM_basicstats(int action, Imodman *mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		stats = mm->GetInterface(I_STATS, ALLARENAS);
		if (!pd || !stats) return MM_FAIL;

		mm->RegCallback(CB_PLAYERACTION, mypa, ALLARENAS);
		mm->RegCallback(CB_KILL, mykill, ALLARENAS);

		mm->RegCallback(CB_FLAGPICKUP, myfpickup, ALLARENAS);
		mm->RegCallback(CB_FLAGDROP, mydrop, ALLARENAS);

		mm->RegCallback(CB_BALLPICKUP, mybpickup, ALLARENAS);
		mm->RegCallback(CB_BALLFIRE, mybfire, ALLARENAS);
		mm->RegCallback(CB_GOAL, mygoal, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->UnregCallback(CB_PLAYERACTION, mypa, ALLARENAS);
		mm->UnregCallback(CB_KILL, mykill, ALLARENAS);
		mm->UnregCallback(CB_FLAGPICKUP, myfpickup, ALLARENAS);
		mm->UnregCallback(CB_FLAGDROP, mydrop, ALLARENAS);
		mm->UnregCallback(CB_BALLPICKUP, mybpickup, ALLARENAS);
		mm->UnregCallback(CB_BALLFIRE, mybfire, ALLARENAS);
		mm->UnregCallback(CB_GOAL, mygoal, ALLARENAS);

		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(stats);
		return MM_OK;
	}
	return MM_FAIL;
}

