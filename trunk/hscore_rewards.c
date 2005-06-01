//HSCore Rewards
//D1st0rt and Bomook
//5/31/05

#include "asss.h"
#include "fg_wz.h"
#include "hscore.h"
#include <math.h>

//modules
local Imodman *mm;
local Ilogman *lm;
local Iplayerdata *pd;
local Ichat *chat;
local Iconfig *cfg;
local Ihscoremoney *money;

//This is assuming we're using fg_wz.py
local void flagWinCallback(Arena *arena, int freq, int *points)
{
	/*
	 * money = (coefficient * jackpot)^exponent * pop
	 * Defaults:
	 * coefficient = 200, exponent = 0.5
	 */

	 //Variable Declarations
	 double amount, coeff, exp, pts;
	 int reward;
	 Player *p;
	 Link *link;

	 //Read Settings
	 coeff = (double)cfg->GetInt(arena->cfg, "Flag", "HSFlagCoeff", 200);
	 exp = (double)cfg->GetInt(arena->cfg, "Flag", "HSFlagExp", 500) / 1000;
	 pts = (double)(*points);

	 //Calculate Reward
	 amount = coeff * pts;
	 amount = pow(amount, exp);
	 amount *= (double)arena->playing;
	 reward = (int)amount;

	 //Distribute Wealth
    pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		if(p->p_freq == freq && p->p_ship != SHIP_SPEC)
		{
			money->giveMoney(p, reward, MONEY_TYPE_FLAG);
			chat->SendMessage(p, "You received $%d for a flag victory.", reward);
		}
	}
	pd->Unlock();
}

local void goalCallback(Arena *arena, Player *scorer, int bid, int x, int y)
{
	/*
	 * money = coefficient * (pop^0.5) + minimum
	 * Defaults:
	 * coefficient = 500, minimum = 300
	 */

	//Variable Declarations
	double amount, coeff, min;
	int reward;
	Player *p;
	Link *link;

	//Read Settings
	coeff = (double)cfg->GetInt(arena->cfg, "Soccer", "HSGoalCoeff", 500);
	min   = (double)cfg->GetInt(arena->cfg, "Soccer", "HSGoalMin",   300);

	//Calculate Reward
	amount = pow((double)arena->playing, 0.5);
	amount *= coeff;
	amount += min;
	reward  = (int)amount;

	//Distribute Wealth
	pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		if(p->p_freq == scorer->p_freq && p->p_ship != SHIP_SPEC)
		{
			money->giveMoney(p, reward, MONEY_TYPE_BALL);
			chat->SendMessage(p, "You received $%d for a team goal.", reward);
		}
	}
	pd->Unlock();
}

local void killCallback(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts, int *green)
{
	/*
	 * money = coefficient * (killeeBty + bonus) / (killerBty + bonus)) + minimum
	 * Defaults:
	 * Coefficient = 50, bonus = 5, minimum = 1
	 */

	//Variable Declarations
	double amount, coeff, bonus, min;
	int reward;

	//Read Settings
	coeff = (double)cfg->GetInt(arena->cfg, "Kill", "HSKillCoeff", 50);
	min   = (double)cfg->GetInt(arena->cfg, "Kill", "HSKillMin",   1);
	bonus = (double)cfg->GetInt(arena->cfg, "Kill", "HSKillBonus", 5);

	//Calculate Reward
	amount  = (killer->position.bounty + bonus);
	amount /= (bounty + bonus);
	amount *= coeff;
	amount += min;
	reward  = (int)amount;

	//Distribute Wealth
	money->giveMoney(killer, amount, MONEY_TYPE_KILL);
	chat->SendMessage(killer, "You received $%d for killing %s (%d bounty).", reward, killed->name, bounty);
}

local int getPeriodicPoints(Arena *arena, int freq, int freqplayers, int totalplayers, int flagsowned)
{
	//is this implemented?
	return 0;
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
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		money = mm->GetInterface(I_HSCORE_MONEY, ALLARENAS);

		if (!lm || !chat || !cfg || !money || !pd)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(pd);
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
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(money);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		mm->RegInterface(&periodicInterface, arena);

		mm->RegCallback(CB_WARZONEWIN, flagWinCallback, arena);
		mm->RegCallback(CB_GOAL, goalCallback, arena);
		mm->RegCallback(CB_KILL, killCallback, arena);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		mm->UnregInterface(&periodicInterface, arena);

		mm->UnregCallback(CB_WARZONEWIN, flagWinCallback, arena);
		mm->UnregCallback(CB_GOAL, goalCallback, arena);
		mm->UnregCallback(CB_KILL, killCallback, arena);

		return MM_OK;
	}
	return MM_FAIL;
}
