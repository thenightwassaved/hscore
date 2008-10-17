//HSCore Rewards
//D1st0rt and Bomook
//5/31/05

#include "asss.h"
#include "fg_wz.h"
#include "hscore.h"
#include <math.h>
#include "hscore_teamnames.h"
#include "jackpot.h"
#include "persist.h"
#include "formula.h"

typedef struct PData
{
	int min_kill_money_to_notify;
	int min_kill_exp_to_notify;
	int min_shared_money_to_notify;

	int periodic_tally;
} PData;

typedef struct AData
{
	int on;
	Formula *kill_money_formula;
	Formula *kill_exp_formula;
	Formula *flag_money_formula;
	Formula *flag_exp_formula;
	Formula *periodic_money_formula;
	Formula *periodic_exp_formula;

	Region *periodic_include_region;
	Region *periodic_exclude_region;

	int periodic_tally;
	int reset;
} AData;

//modules
local Imodman *mm;
local Ilogman *lm;
local Iplayerdata *pd;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihscoremoney *hsmoney;
local Ipersist *persist;
local Iformula *formula;
local Iarenaman *aman;
local Imapdata *mapdata;
local Imainloop *ml;

local int pdkey;
local int adkey;

local int minmax(int min, int x, int max)
{
	if (min > x)
	{
		x = min;
	}
	else if (max < x)
	{
		x = max;
	}

	return x;
}

local helptext_t killmessages_help =
"Targets: none\n"
"Args: none\n"
"Toggles the kill reward messages on and off";

local void Ckillmessages(const char *command, const char *params, Player *p, const Target *target)
{
	PData *pdata = PPDATA(p, pdkey);

	if (pdata->min_kill_money_to_notify == 0)
	{
		pdata->min_kill_money_to_notify = -1;
		pdata->min_kill_exp_to_notify = -1;
		pdata->min_shared_money_to_notify = -1;

		chat->SendMessage(p, "Kill messages disabled!");
	}
	else
	{
		pdata->min_kill_money_to_notify = 0;
		pdata->min_kill_exp_to_notify = 0;
		pdata->min_shared_money_to_notify = 20;

		chat->SendMessage(p, "Kill messages enabled!");
	}
}

local int GetPersistData(Player *p, void *data, int len, void *clos)
{
	PData *pdata = PPDATA(p, pdkey);

	PData *persist_data = (PData*)data;

	persist_data->min_kill_money_to_notify = pdata->min_kill_money_to_notify;
	persist_data->min_kill_exp_to_notify = pdata->min_kill_exp_to_notify;
	persist_data->min_shared_money_to_notify = pdata->min_shared_money_to_notify;

	return sizeof(PData);
}

local void SetPersistData(Player *p, void *data, int len, void *clos)
{
	PData *pdata = PPDATA(p, pdkey);

	PData *persist_data = (PData*)data;

	pdata->min_kill_money_to_notify = persist_data->min_kill_money_to_notify;
	pdata->min_kill_exp_to_notify = persist_data->min_kill_exp_to_notify;
	pdata->min_shared_money_to_notify = persist_data->min_shared_money_to_notify;
}

local void ClearPersistData(Player *p, void *clos)
{
	PData *pdata = PPDATA(p, pdkey);

	pdata->min_kill_money_to_notify = 0;
	pdata->min_kill_exp_to_notify = 0;
	pdata->min_shared_money_to_notify = 20;
}

local PlayerPersistentData my_persist_data =
{
	11504, INTERVAL_FOREVER, PERSIST_GLOBAL,
	GetPersistData, SetPersistData, ClearPersistData
};

//This is assuming we're using fg_wz.py
local void flagWinCallback(Arena *arena, int freq, int *pts)
{
	AData *adata = P_ARENA_DATA(arena, adkey);

	if (adata->flag_money_formula || adata->flag_exp_formula)
	{
		double players = 0;
		double freq_size = 0;
		double jackpot_points = 0;
		double totalexp = 0;
		double teamexp = 0;
		int money = 0;
		int exp = 0;

		HashTable *vars = HashAlloc();

		Player *i;
		Link *link;
		Ijackpot *jackpot;
		Iteamnames *teamnames;

		pd->Lock();
		FOR_EACH_PLAYER(i)
			if (i->status == S_PLAYING &&
				i->arena == arena &&
				i->p_ship != SHIP_SPEC &&
				IS_HUMAN(i))
			{
				players++;
				totalexp += hsmoney->getExp(i);
				if (i->p_freq == freq)
				{
					freq_size++;
					teamexp += hsmoney->getExp(i);
				}
			}
		pd->Unlock();

		jackpot = mm->GetInterface(I_JACKPOT, arena);
		if (jackpot)
		{
			jackpot_points = jackpot->GetJP(arena);
			mm->ReleaseInterface(jackpot);
		}

		HashAdd(vars, "totalsize", &players);
		HashAdd(vars, "teamsize", &freq_size);
		HashAdd(vars, "jackpot", &jackpot_points);
		HashAdd(vars, "totalexp", &totalexp);
		HashAdd(vars, "teamexp", &teamexp);

		if (adata->flag_money_formula)
		{
			money = formula->EvaluateFormulaInt(adata->flag_money_formula, vars, 0);
		}

		if (adata->flag_exp_formula)
		{
			exp = formula->EvaluateFormulaInt(adata->flag_exp_formula, vars, 0);
		}

		HashFree(vars);

		teamnames = mm->GetInterface(I_TEAMNAMES, arena);
		if (teamnames)
		{
			const char *name = teamnames->getFreqTeamName(freq, arena);
			if (name != NULL)
			{
				chat->SendArenaMessage(arena, "%s won flag game. Reward: $%d (%d exp)", name, money, exp);
			}
			else
			{
				chat->SendArenaMessage(arena, "Unidentified team won flag game. Reward: $%d (%d exp)", money, exp);
			}
		}
		else
		{
			chat->SendArenaMessage(arena, "Reward: $%d (%d exp)", money, exp);
		}

		 //Distribute Wealth
		pd->Lock();
		FOR_EACH_PLAYER(i)
		{
			if(i->arena == arena && i->p_freq == freq && i->p_ship != SHIP_SPEC)
			{
				hsmoney->giveMoney(i, money, MONEY_TYPE_FLAG);
				hsmoney->giveExp(i, exp);
				//no need to send message, as the team announcement works just fine
				//chat->SendMessage(p, "You received $%d and %d exp for a flag victory.", reward, exp);
			}
		}
		pd->Unlock();
	}
}

local int calculateExpReward(Player *killer, Player *killed)
{
	/* cfghelp: Hyperspace:UseDiscrete, arena, int, def: 1, mod: hscore_rewards
	 * 1 = Use discrete method of calculating exp rewards.
	 * 0 = Use old exponential method.*/
	int useDiscrete = cfg->GetInt(killer->arena->cfg, "Hyperspace", "UseDiscrete",  1);

	//get exp
	//add one to prevent divide by zero errors
	double kexp = (double) hsmoney->getExp(killer) + 1;
	double dexp = (double) hsmoney->getExp(killed) + 1;

	int reward = 0;

	if (useDiscrete)
	{
		/* cfghelp: Hyperspace:ExpSlope, arena, int, def: 2000, mod: hscore_rewards
		 * Discrete slope. The m in exp = floor(m*x + b). 1000 = 1.0*/
		double m = (double)cfg->GetInt(killer->arena->cfg, "Hyperspace", "ExpSlope", 2000) / 1000.0;
		/* cfghelp: Hyperspace:ExpIntercept, arena, int, def: 500, mod: hscore_rewards
		 * Discrete intercept. The b in exp = floor(m*x + b). 1000 = 1.0*/
		double b = (double)cfg->GetInt(killer->arena->cfg, "Hyperspace", "ExpIntercept", 500) / 1000.0;
		/* cfghelp: Hyperspace:MaxExp, arena, int, def: 6, mod: hscore_rewards
		 * Maximum exp reward that can be distributed. */
		int max = cfg->GetInt(killer->arena->cfg, "Hyperspace", "MaxExp", 6);
		/* cfghelp: Hyperspace:MinExp, arena, int, def: 0, mod: hscore_rewards
		 * Minimum exp reward that can be distributed. */
		int min = cfg->GetInt(killer->arena->cfg, "Hyperspace", "MinExp", 0);

		double ratio = dexp / kexp;

		reward = (int)(floor(m * ratio + b));
		reward = minmax(min, reward, max);
	}
	else
	{
		/* cfghelp: Hyperspace:KillCoeff, arena, int, def: 10, mod: hscore_rewards
		 * Kill reward coefficient (if discrete = 0). */
		double coeff = (double)cfg->GetInt(killer->arena->cfg, "Hyperspace", "KillCoeff", 10);
		/* cfghelp: Hyperspace:KillMin, arena, int, def: 1, mod: hscore_rewards
		 * Amount added to exp (if discrete = 0). */
		double min   = (double)cfg->GetInt(killer->arena->cfg, "Hyperspace", "KillMin",   1);

		//Calculate Earned Experience
		double amount = log(dexp + 1) / log(kexp + 2);
		amount *= coeff;
		amount += min;
		reward = (int)amount;
	}

	return reward;
}

//bonus money is shared between teammates
local int calculateBonusMoneyReward(Player *killer, Player *killed, int bounty)
{
	/* cfghelp: Hyperspace:KillerBountyMult, arena, int, def: 1, mod: hscore_rewards
	 * Amount to multiply by killer's bounty to add to the money reward.  1000 = 100%*/
	double killerBountyMult = (double)cfg->GetInt(killer->arena->cfg, "Hyperspace", "KillerBountyMult",  1000) / 1000.0;
	/* cfghelp: Hyperspace:KilleeBountyMult, arena, int, def: 1, mod: hscore_rewards
	 * Amount to multiply by killee's bounty to add to the money reward. 1000 = 100% */
	double killeeBountyMult = (double)cfg->GetInt(killer->arena->cfg, "Hyperspace", "KilleeBountyMult",  1000) / 1000.0;
	/* cfghelp: Hyperspace:AddToBonus, arena, int, def: 0, mod: hscore_rewards
	 * Added directly to the bonus calculation */
	int addToBonus = cfg->GetInt(killer->arena->cfg, "Hyperspace", "AddToBonus",  0);

	double bountyBonus = (double)(killer->position.bounty) * killerBountyMult;
	bountyBonus += (double)(bounty) * killeeBountyMult;

	bountyBonus += addToBonus;

	//other bonuses can be added here in the future

	if (bountyBonus > 40000) bountyBonus = 0;

	return (int)bountyBonus;
}

//base money is for the killing player only
local int calculateBaseMoneyReward(Player *killer, Player *killed)
{
	/* cfghelp: Hyperspace:KilleeBountyMult, arena, int, def: 75, mod: hscore_rewards
	 * base money = base multipiler * exponental formula. */
	double baseMultiplier = (double)cfg->GetInt(killer->arena->cfg, "Hyperspace", "BaseMultiplier",  75);

	double kexp = (double) hsmoney->getExp(killer) + 1;
	double dexp = (double) hsmoney->getExp(killed) + 1;

	double baseMoney = exp( -kexp / (dexp + 1)) * baseMultiplier;

	return (int)baseMoney;
}

local void killCallback(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts, int *green)
{
	PData *pdata = PPDATA(killer, pdkey);

	if(killer->p_freq == killed->p_freq)
	{
		chat->SendMessage(killer, "No reward for teamkill of %s.", killed->name);
	}
	else
	{
		/* cfghelp: Hyperspace:MinBonusPlayers, arena, int, def: 4, mod: hscore_rewards
		 * Minimum number of players in game required for bonus money. */
		int minBonusPlayers  = cfg->GetInt(arena->cfg, "Hyperspace", "MinBonusPlayers",  4);
		/* cfghelp: Hyperspace:DisableMoneyRewards, arena, int, def: 0, mod: hscore_rewards
		 * If no money should be awarded for kills. */
		int disableMoneyRewards  = cfg->GetInt(arena->cfg, "Hyperspace", "DisableMoneyRewards",  0);
		/* cfghelp: Hyperspace:DisableExpRewards, arena, int, def: 0, mod: hscore_rewards
		 * If no exp should be awarded for kills. */
		int disableExpRewards  = cfg->GetInt(arena->cfg, "Hyperspace", "DisableExpRewards",  0);

		//Calculate Earned Money
		int experience = calculateExpReward(killer, killed);
		int baseMoney = calculateBaseMoneyReward(killer, killed);
		int bonusMoney = calculateBonusMoneyReward(killer, killed, bounty);

		if (arena->playing < minBonusPlayers)
		{
			bonusMoney = 0;
		}

		int notify_for_exp = pdata->min_kill_exp_to_notify != -1 && pdata->min_kill_exp_to_notify <= experience;
		int notify_for_money = pdata->min_kill_money_to_notify != -1 && pdata->min_kill_money_to_notify <= baseMoney + bonusMoney;

		//Distribute Wealth
		if (!disableExpRewards)
		{
			hsmoney->giveExp(killer, experience);

			if (disableMoneyRewards)
			{
				if (notify_for_exp)
				{
					chat->SendMessage(killer, "You received %d exp for killing %s.", experience, killed->name);
				}
			}
		}

		if (!disableMoneyRewards)
		{
			hsmoney->giveMoney(killer, baseMoney + bonusMoney, MONEY_TYPE_KILL);

			if (disableExpRewards)
			{
				if (notify_for_money)
				{
					chat->SendMessage(killer, "You received $%d for killing %s.", baseMoney + bonusMoney, killed->name);
				}
			}
			else
			{
				if (notify_for_money || notify_for_exp)
				{
					chat->SendMessage(killer, "You received $%d and %d exp for killing %s.", baseMoney + bonusMoney, experience, killed->name);
				}
			}

			//give money to teammates
			Player *p;
			Link *link;
			/* cfghelp: Hyperspace:TeammateReward, arena, int, def: 500, mod: hscore_rewards
			 * The percentage (max) that a teammate can receive from a kill.
			 * 1000 = 100%*/
			double teammateRewardCoeff = (double)cfg->GetInt(arena->cfg, "Hyperspace", "TeammateReward", 500) / 1000.0; //50%
			/* cfghelp: Hyperspace:DistFalloff, arena, int, def: 1440000, mod: hscore_rewards
			 * Distance falloff divisor in pixels^2. */
			double distanceFalloff = (double)cfg->GetInt(arena->cfg, "Hyperspace", "DistFalloff", 1440000); //pixels^2

			pd->Lock();
			FOR_EACH_PLAYER(p)
			{
				if(p->arena == killer->arena && p->p_freq == killer->p_freq && p->p_ship != SHIP_SPEC && p != killer && !(p->position.status & STATUS_SAFEZONE))
				{
					double maxReward = teammateRewardCoeff * (double)(bonusMoney + calculateBaseMoneyReward(p, killed));

					int xdelta = (p->position.x - killer->position.x);
					int ydelta = (p->position.y - killer->position.y);
					double distPercentage = ((double)(xdelta * xdelta + ydelta * ydelta)) / distanceFalloff;

					int reward = (int)(maxReward * exp(-distPercentage));

					hsmoney->giveMoney(p, reward, MONEY_TYPE_KILL);

					PData *tdata = PPDATA(p, pdkey);
					//check if they received more than %30. if they did, message them. otherwise, don't bother.
					if (tdata->min_shared_money_to_notify != -1 && tdata->min_shared_money_to_notify <= reward)
					{
						chat->SendMessage(p, "You received $%d for %s's kill of %s.", reward, killer->name, killed->name);
					}
				}
			}
			pd->Unlock();
		}
	}
}

local int periodic_tick(void *clos)
{
	Arena *arena = clos;
	AData *adata = P_ARENA_DATA(arena, adkey);
	Player *p;
	Link *link;
	int reset = 0;

	pd->Lock();
	if (adata->reset)
	{
		reset = 1;
		adata->reset = 0;
		adata->periodic_tally = 0;
	}
	else
	{
		adata->periodic_tally++;
	}

	FOR_EACH_PLAYER(p)
	{
		if(p->arena == arena)
		{
			PData *pdata = PPDATA(p, pdkey);
			if (reset)
			{
				pdata->periodic_tally = 0;
			}
			else
			{
				if (adata->periodic_include_region != NULL)
				{
					if (mapdata->Contains(adata->periodic_include_region, p->position.x >> 4, p->position.y >> 4))
					{
						pdata->periodic_tally++;
					}
				}
				else if (adata->periodic_exclude_region != NULL)
				{
					if (!mapdata->Contains(adata->periodic_exclude_region, p->position.x >> 4, p->position.y >> 4))
					{
						pdata->periodic_tally++;
					}
				}
				else
				{
					pdata->periodic_tally++;
				}
			}
		}
	}
	pd->Unlock();

	return TRUE;
}

local void freqChangeCallback(Player *p, int newfreq)
{
	PData *pdata = PPDATA(p, pdkey);
	pdata->periodic_tally = 0;
}

local void paction(Player *p, int action, Arena *arena)
{
	if (action == PA_ENTERARENA)
	{
		PData *pdata = PPDATA(p, pdkey);
		pdata->periodic_tally = 0;
	}
}

local int getPeriodicPoints(Arena *arena, int freq, int freqplayers, int totalplayers, int flagsowned)
{
	AData *adata = P_ARENA_DATA(arena, adkey);

	if (adata->periodic_money_formula || adata->periodic_exp_formula)
	{
		char *flagstring;
		int money = 0;
		int exp = 0;
		Player *p;
		Link *link;
		HashTable *vars = HashAlloc();
		double totalsize = (double)totalplayers;
		double teamsize = (double)freqplayers;
		double flags = (double)flagsowned;

		HashAdd(vars, "totalsize", &totalsize);
		HashAdd(vars, "teamsize", &teamsize);
		HashAdd(vars, "flags", &flags);

		if (adata->periodic_money_formula)
		{
			money = formula->EvaluateFormulaInt(adata->periodic_money_formula, vars, 0);
		}

		if (adata->periodic_exp_formula)
		{
			exp = formula->EvaluateFormulaInt(adata->periodic_exp_formula, vars, 0);
		}

		HashFree(vars);

		if (flagsowned == 1)
		{
			flagstring = "flag";
		}
		else
		{
			flagstring = "flags";
		}

		pd->Lock();
		FOR_EACH_PLAYER(p)
		{
			if(p->arena == arena && p->p_freq == freq && p->p_ship != SHIP_SPEC)
			{
				PData *pdata = PPDATA(p, pdkey);
				int p_money, p_exp;

				if (adata->periodic_tally)
				{
					p_money = (money * pdata->periodic_tally) / adata->periodic_tally;
					p_exp = (exp * pdata->periodic_tally) / adata->periodic_tally;
				}
				else
				{
					p_money = money;
					p_exp = exp;
				}

				hsmoney->giveMoney(p, p_money, MONEY_TYPE_FLAG);
				hsmoney->giveExp(p, p_exp);
				chat->SendMessage(p, "You received $%d ($%d) and %d exp (%d) for holding %d %s.", p_money, money, p_exp, exp, flagsowned, flagstring);
			}
		}
		
		adata->reset = 1;
		pd->Unlock();

		return money;
	}

	return 0;
}

local void free_formulas(Arena *arena)
{
	AData *adata = P_ARENA_DATA(arena, adkey);

	if (adata->kill_money_formula)
	{
		formula->FreeFormula(adata->kill_money_formula);
		adata->kill_money_formula = NULL;
	}

	if (adata->kill_exp_formula)
	{
		formula->FreeFormula(adata->kill_exp_formula);
		adata->kill_exp_formula = NULL;
	}

	if (adata->flag_money_formula)
	{
		formula->FreeFormula(adata->flag_money_formula);
		adata->flag_money_formula = NULL;
	}

	if (adata->flag_exp_formula)
	{
		formula->FreeFormula(adata->flag_exp_formula);
		adata->flag_exp_formula = NULL;
	}

	if (adata->periodic_money_formula)
	{
		formula->FreeFormula(adata->periodic_money_formula);
		adata->periodic_money_formula = NULL;
	}

	if (adata->periodic_exp_formula)
	{
		formula->FreeFormula(adata->periodic_exp_formula);
		adata->periodic_exp_formula = NULL;
	}
}

local void get_formulas(Arena *arena)
{
	AData *adata = P_ARENA_DATA(arena, adkey);
	const char *kill_money, *kill_exp;
	const char *flag_money, *flag_exp;
	const char *periodic_money, *periodic_exp;
	const char *include_rgn, *exclude_rgn;
	char error[200];
	error[0] = '\0';


	// free the formulas if they already exist
	free_formulas(arena);

	kill_money = cfg->GetStr(arena->cfg, "Hyperspace", "KillMoneyFormula");
	kill_exp = cfg->GetStr(arena->cfg, "Hyperspace", "KillExpFormula");
	flag_money = cfg->GetStr(arena->cfg, "Hyperspace", "FlagMoneyFormula");
	flag_exp = cfg->GetStr(arena->cfg, "Hyperspace", "FlagExpFormula");
	periodic_money = cfg->GetStr(arena->cfg, "Hyperspace", "PeriodicMoneyFormula");
	periodic_exp = cfg->GetStr(arena->cfg, "Hyperspace", "PeriodicExpFormula");

	include_rgn = cfg->GetStr(arena->cfg, "Hyperspace", "PeriodicIncludeRegion");
	if (include_rgn)
	{
		adata->periodic_include_region = mapdata->FindRegionByName(arena, include_rgn);
	}
	else
	{
		adata->periodic_include_region = NULL;
	}

	exclude_rgn = cfg->GetStr(arena->cfg, "Hyperspace", "PeriodicExcludeRegion");
	if (exclude_rgn)
	{
		adata->periodic_exclude_region = mapdata->FindRegionByName(arena, exclude_rgn);
	}
	else
	{
		adata->periodic_exclude_region = NULL;
	}

	if (kill_money && *kill_money)
	{
		adata->kill_money_formula = formula->ParseFormula(kill_money, error, sizeof(error));
		if (adata->kill_money_formula == NULL)
		{
			lm->LogA(L_WARN, "hscore_rewards", arena, "Error parsing kill money reward formula: %s", error);
		}
	}

	if (kill_exp && *kill_exp)
	{
		adata->kill_exp_formula = formula->ParseFormula(kill_exp, error, sizeof(error));
		if (adata->kill_exp_formula == NULL)
		{
			lm->LogA(L_WARN, "hscore_rewards", arena, "Error parsing kill exp reward formula: %s", error);
		}
	}

	if (flag_money && *flag_money)
	{
		adata->flag_money_formula = formula->ParseFormula(flag_money, error, sizeof(error));
		if (adata->flag_money_formula == NULL)
		{
			lm->LogA(L_WARN, "hscore_rewards", arena, "Error parsing flag money reward formula: %s", error);
		}
	}

	if (flag_exp && *flag_exp)
	{
		adata->flag_exp_formula = formula->ParseFormula(flag_exp, error, sizeof(error));
		if (adata->flag_exp_formula == NULL)
		{
			lm->LogA(L_WARN, "hscore_rewards", arena, "Error parsing flag exp reward formula: %s", error);
		}
	}

	if (periodic_money && *periodic_money)
	{
		adata->periodic_money_formula = formula->ParseFormula(periodic_money, error, sizeof(error));
		if (adata->periodic_money_formula == NULL)
		{
			lm->LogA(L_WARN, "hscore_rewards", arena, "Error parsing periodic money reward formula: %s", error);
		}
	}

	if (periodic_exp && *periodic_exp)
	{
		adata->periodic_exp_formula = formula->ParseFormula(periodic_exp, error, sizeof(error));
		if (adata->periodic_exp_formula == NULL)
		{
			lm->LogA(L_WARN, "hscore_rewards", arena, "Error parsing periodic exp reward formula: %s", error);
		}
	}
}

local void aaction(Arena *arena, int action)
{
	if (action == AA_CONFCHANGED)
	{
		get_formulas(arena);
	}
}

local Iperiodicpoints periodicInterface =
{
	INTERFACE_HEAD_INIT(I_PERIODIC_POINTS, "pp-basic")
	getPeriodicPoints
};

EXPORT const char info_hscore_rewards[] = "v1.2 D1st0rt & Dr Brain";

EXPORT int MM_hscore_rewards(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		hsmoney = mm->GetInterface(I_HSCORE_MONEY, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		persist = mm->GetInterface(I_PERSIST, ALLARENAS);
		formula = mm->GetInterface(I_FORMULA, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		mapdata = mm->GetInterface(I_MAPDATA, ALLARENAS);
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);

		if (!lm || !chat || !cfg || !hsmoney || !pd || !cmd || !persist || !formula || !aman || !mapdata || !ml)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(pd);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(hsmoney);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(persist);
			mm->ReleaseInterface(formula);
			mm->ReleaseInterface(aman);
			mm->ReleaseInterface(mapdata);
			mm->ReleaseInterface(ml);

			return MM_FAIL;
		}

		pdkey = pd->AllocatePlayerData(sizeof(PData));
		if (pdkey == -1) return MM_FAIL;

		adkey = aman->AllocateArenaData(sizeof(AData));
		if (adkey == -1) return MM_FAIL;

		persist->RegPlayerPD(&my_persist_data);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		persist->UnregPlayerPD(&my_persist_data);

		pd->FreePlayerData(pdkey);
		aman->FreeArenaData(adkey);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(hsmoney);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(persist);
		mm->ReleaseInterface(formula);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(mapdata);
		mm->ReleaseInterface(ml);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		AData *adata = P_ARENA_DATA(arena, adkey);
		mm->RegInterface(&periodicInterface, arena);

		adata->on = 1;
		adata->kill_money_formula = NULL;
		adata->kill_exp_formula = NULL;
		adata->flag_money_formula = NULL;
		adata->flag_exp_formula = NULL;
		adata->periodic_money_formula = NULL;
		adata->periodic_exp_formula = NULL;
		get_formulas(arena);

		mm->RegCallback(CB_WARZONEWIN, flagWinCallback, arena);
		mm->RegCallback(CB_KILL, killCallback, arena);
		mm->RegCallback(CB_ARENAACTION, aaction, arena);
		mm->RegCallback(CB_FREQCHANGE, freqChangeCallback, arena);
		mm->RegCallback(CB_PLAYERACTION, paction, arena);

		cmd->AddCommand("killmessages", Ckillmessages, arena, killmessages_help);

		adata->reset = 1;
		ml->SetTimer(periodic_tick, 0, 100, arena, arena);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		AData *adata = P_ARENA_DATA(arena, adkey);
		mm->UnregInterface(&periodicInterface, arena);

		cmd->RemoveCommand("killmessages", Ckillmessages, arena);

		mm->UnregCallback(CB_WARZONEWIN, flagWinCallback, arena);
		mm->UnregCallback(CB_KILL, killCallback, arena);
		mm->UnregCallback(CB_ARENAACTION, aaction, arena);
		mm->UnregCallback(CB_FREQCHANGE, freqChangeCallback, arena);
		mm->UnregCallback(CB_PLAYERACTION, paction, arena);

		ml->ClearTimer(periodic_tick, arena);
		
		adata->on = 0;
		free_formulas(arena);

		return MM_OK;
	}
	return MM_FAIL;
}
