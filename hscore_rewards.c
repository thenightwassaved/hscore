//HSCore Rewards
//D1st0rt and Bomook
//5/31/05

#include "asss.h"
#include "fg_wz.h"
#include "hscore.h"
#include <math.h>
#include "hscore_teamnames.h"

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
	 double amount, coeff, expn, pts;
	 int reward, exp;
	 Player *p;
	 Link *link;

	 //Read Settings
	 coeff = (double)cfg->GetInt(arena->cfg, "Flag", "HSFlagCoeff", 200);
	 expn = (double)cfg->GetInt(arena->cfg, "Flag", "HSFlagExp", 500) / 1000;
	 pts = (double)(*points);

	 //chat->SendArenaMessage(arena, "coeff=%f expn=%f pts=%f", coeff, expn, pts);

	 //Calculate Reward
	 amount = coeff * pts;
	 amount = pow(amount, expn);
	 amount *= (double)arena->playing;
	 reward = (int)amount;
	 exp = (int)(amount / 8);


	Iteamnames *teamnames = mm->GetInterface(I_TEAMNAMES, arena);
	if (teamnames)
	{
		const char *name = teamnames->getFreqTeamName(freq, arena);
		if (name != NULL)
		{
			chat->SendArenaMessage(arena, "%s won flag game. Reward: $%d (%d exp)", name, reward, exp);
		}
		else
		{
			chat->SendArenaMessage(arena, "Unidentified team won flag game. Reward: $%d (%d exp)", reward, exp);
		}
	}
	else
	{
		chat->SendArenaMessage(arena, "Reward: $%d (%d exp)", reward, exp);
	}

	 //Distribute Wealth
    pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		if(p->arena == arena && p->p_freq == freq && p->p_ship != SHIP_SPEC)
		{
			money->giveMoney(p, reward, MONEY_TYPE_FLAG);
			money->giveExp(p, exp);
			//no need to send message, as the team announcement works just fine
			//chat->SendMessage(p, "You received $%d and %d exp for a flag victory.", reward, exp);
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
	int reward, exp;
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
	exp = (int)(amount / 10);

	//Distribute Wealth
	pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		if(p->p_freq == scorer->p_freq && p->p_ship != SHIP_SPEC)
		{
			money->giveMoney(p, reward, MONEY_TYPE_BALL);
			money->giveExp(p, exp);
			chat->SendMessage(p, "You received $%d and %d exp for a team goal.", reward, exp);
		}
	}
	pd->Unlock();
}

local void killCallback(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts, int *green)
{
	/*
	 * --Old Formula--
	 * money = coefficient * (killeeBty + bonus) / (killerBty + bonus)) + minimum
	 * Defaults:
	 * Coefficient = 50, bonus = 5, minimum = 1
	 ****************************************************************************
	 * --New Formula--
	 * exp = coefficient * ( ln(killeeExp + 1) / ln(killerExp + 2) + minimum
	 * money = ( multiplier * exp ) + ( killerBty * bonus )
	 * Defaults:
	 * Coefficient = 10, minimum = 1, multiplier = 10, bonus = 1
	 *
     * --New Formula 2--
	 * exp = coefficient * e ^( -killerExp / (killeeExp + 1) )
	 */

	if(killer->p_freq == killed->p_freq)
	{
		chat->SendMessage(killer, "No reward for teamkill of %s.", killed->name);
	}
	else
	{
		//Variable Declarations
		double amount, coeff, bonus, min, mult, kexp, dexp;
		int hsbucks, xp, exp2;

		//Read Settings
		coeff = (double)cfg->GetInt(arena->cfg, "Kill", "HSKillCoeff", 10);
		min   = (double)cfg->GetInt(arena->cfg, "Kill", "HSKillMin",   1);
		bonus = (double)cfg->GetInt(arena->cfg, "Kill", "HSKillBonus", 1);
		mult  = (double)cfg->GetInt(arena->cfg, "Kill", "HSKillMult",  10);

		//Retrieve Experience
		kexp = (double) money->getExp(killer);
		dexp = (double) money->getExp(killed);

		//Calculate Earned Experience
		amount = log(dexp + 1) / log(kexp + 2);
		amount *= coeff;
		amount += min;
		xp = (int) amount;

		//Calculate Earned Money
		hsbucks = (mult * xp);
		hsbucks += (killer->position.bounty * bonus);

		//Calculate Exp2
		amount =  exp( -kexp / (dexp + 1));
		amount *= coeff;
		amount += min;
		exp2   =  (int)amount;

		//Distribute Wealth
		money->giveMoney(killer, hsbucks, MONEY_TYPE_KILL);
		money->giveExp(killer, xp);
		chat->SendMessage(killer, "You received $%d and %d exp (%d) for killing %s.", hsbucks, xp, exp2, killed->name);

		//give money to teammates
		Player *p;
		Link *link;
		double teammateRewardCoeff = (double)cfg->GetInt(arena->cfg, "Kill", "HSTeammateReward", 500); //50%
		double distanceFalloff = (double)cfg->GetInt(arena->cfg, "Kill", "HSDistFalloff", 1440000); //pixels^2
		double maxReward = (double)(hsbucks) * teammateRewardCoeff / 1000.0;
		pd->Lock();
		FOR_EACH_PLAYER(p)
		{
			if(p->p_freq == killer->p_freq && p->p_ship != SHIP_SPEC && p != killer && !(p->position.status & STATUS_SAFEZONE))
			{
				int xdelta = (p->position.x - killer->position.x);
				int ydelta = (p->position.y - killer->position.y);
				double distPercentage = ((double)(xdelta * xdelta + ydelta * ydelta)) / distanceFalloff;

				int reward = (int)(maxReward * exp(-distPercentage));

				money->giveMoney(p, reward, MONEY_TYPE_KILL);

				//check if they received more than %30. if they did, message them. otherwise, don't bother.
				if (reward > (int)(0.30 * maxReward))
				{
					chat->SendMessage(p, "You received $%d for %s's kill.", reward, killer->name);
				}
			}
		}
		pd->Unlock();
	}
}

local int getPeriodicPoints(Arena *arena, int freq, int freqplayers, int totalplayers, int flagsowned)
{
	/*
	 * money = (flagcoeff * flagcount) / (teamcoeff * teamsize)
	 * Defaults:
	 * flagcoeff = 200, teamcoeff = 1
	 */

	 //Variable Declarations
	double amount, fcoeff, tcoeff;
	int reward, exp;
	Player *p;
	Link *link;


	 //Read Settings
	fcoeff = (double)cfg->GetInt(arena->cfg, "Periodic", "HSFlagCoeff", 200);
	tcoeff = (double)cfg->GetInt(arena->cfg, "Periodic", "HSTeamCoeff", 200);


	//Calculate reward
	amount  = (fcoeff * (double)flagsowned);
	amount /= (tcoeff * (double)freqplayers);
	reward = (int)amount;
	exp = (int)(amount / 15);

	//Distribute Wealth
	pd->Lock();
	FOR_EACH_PLAYER(p)
	{
		if(p->p_freq == freq && p->p_ship != SHIP_SPEC)
		{
			money->giveMoney(p, reward, MONEY_TYPE_FLAG);
			money->giveExp(p, exp);
			chat->SendMessage(p, "You received $%d and %d exp for holding %d flag(s).", reward, exp, flagsowned);
		}
	}
	pd->Unlock();

	return reward;
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

		//chat->SendArenaMessage(arena, "OMGWTFBBQ!!!");

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
