#include <stdlib.h>
#include <string.h>

#include "../asss.h"
#include "hscore_database.h"
#include "hscore_money.h"

//prototypes
local void GiveMoney(Player *p, int ammount, int type);
local void SetMoney(Player *p, int ammount, int type);
local int GetMoney(Player *p);
local int GetMoneyType(Player *p, int type);
local void GiveExp(Player *p, int ammount);
local void SetExp(Player *p, int ammount);
local int GetExp(Player *p);

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Iplayerdata *pd;
local Ihsdatabase *db;

local void GiveMoney(Player *p, int amount, int type)
{
	HSData *data = db->GetHSData(p);

	data->money += amount;

	if (type > MONEY_TYPE_COUNT || type < 0)
		return;

	data->money_type[type] += amount;
}

local void SetMoney(Player *p, int amount, int type)
{
	GiveMoney(p, amount - GetMoney(p), type);
}

local int GetMoney(Player *p)
{
	HSData *data = db->GetHSData(p);

	return data->money;
}

local int GetMoneyType(Player *p, int type)
{
	HSData *data = db->GetHSData(p);

	if (type > MONEY_TYPE_COUNT || type < 0)
		return 0;

	return data->money_type[type];
}

local void GiveExp(Player *p, int amount)
{
	HSData *data = db->GetHSData(p);

	data->exp += amount;
}

local void SetExp(Player *p, int amount)
{
	HSData *data = db->GetHSData(p);

	data->exp = amount;
}

local int GetExp(Player *p)
{
	HSData *data = db->GetHSData(p);

	return data->exp;
}

local helptext_t give_help =
"Targets: player\n"
"Args: {amount}\n"
"Give the player the specified amount of money from your own account.\n";

local void Cgive(const char *params, Player *p, const Target *target)
{
    char *next;
    int amount = strtol(params, &next, 0);

    if (next != params)
    {
        if (target->type == T_PLAYER) //Single player
        {
            int must_remain = cfg->GetInt(GLOBAL, "hyperspace", "minmoney", 1000);
            int min_give = cfg->GetInt(GLOBAL, "hyperspace", "mingive", 1);
            int max_give = cfg->GetInt(GLOBAL, "hyperspace", "maxgive", 100000000);

            if (!db->Loaded(p)) //Player has no data loaded
            {
                chat->SendMessage(p, "You have no data loaded.");
                return;
            }
            if (!db->Loaded(target->u.p)) //Target player has no data loaded
            {
                chat->SendMessage(p, "That player has no data loaded.");
                return;
            }

            if (GetMoney(p) - must_remain >= amount)
            {
                if (0 < amount) //hard coded.
                {
					if (min_give < amount)
					{
						if (amount < max_give)
						{
							if (GetMoney(target->u.p) + amount > 0)
							{
								GiveMoney(target->u.p, amount, MONEY_TYPE_GIVE);
								GiveMoney(p, -amount, MONEY_TYPE_GIVE);
								chat->SendMessage(target->u.p, "You were given $%i by %s.", amount, p->name);
								chat->SendMessage(p, "You gave $%i to %s.", amount, target->u.p->name);
							}
							else
							{
								chat->SendMessage(p, "Give: overflow error.", max_give);
								lm->LogP(L_MALICIOUS, "hscore_money", p, "Overflowing give to player %s.", target->u.p->name);
							}
						}
						else
						{
							chat->SendMessage(p, "Give: bad value. You cannot give more than $%i.", max_give);
						}
					}
					else
					{
						chat->SendMessage(p, "Give: bad value. You cannot give less than $%i.", min_give);
					}
                }
                else
                {
                    chat->SendMessage(p, "Give: bad value. You can only give positive amounts.");
                }
            }
            else
            {
				if (GetMoney(p) < amount)
				{
					chat->SendMessage(p, "You do not have that much money in your own account.");
				}
				else
				{
                	chat->SendMessage(p, "You must leave at least $%i in your own account.", must_remain);
				}
            }
        }
    }
    else
    {
        chat->SendMessage(p, "Give: bad value.");
    }
}

local helptext_t grant_help =
"Targets: player, freq, or arena\n"
"Args: {amount}\n"
"Grants the target specified amount of money.\n";

local void Cgrant(const char *params, Player *p, const Target *target)
{
    char *next;
    int amount = strtol(params, &next, 0);

    if (next != params)
    {
        if (target->type == T_PLAYER) //Single player
        {
            if (db->Loaded(target->u.p))
            {
                GiveMoney(target->u.p, amount, MONEY_TYPE_GRANT);
                chat->SendMessage(target->u.p, "You were granted $%i.", amount);
                chat->SendMessage(p, "You granted $%i to %s.", amount, target->u.p->name);
            }
            else
            {
                chat->SendMessage(p, "That player has no data loaded.");
            }
        }
        else if (target->type == T_FREQ)
        {
            LinkedList set = LL_INITIALIZER;
            Link *l;
            pd->TargetToSet(target, &set);

            for (l = LLGetHead(&set); l; l = l->next)
            {
                if (db->Loaded(l->data))
                {
                    GiveMoney(l->data, amount, MONEY_TYPE_GRANT);
                    chat->SendMessage(l->data, "You were granted $%i.", amount);
                }
                else
                {
                    chat->SendMessage(p, "Player %s has no data loaded.", ((Player*)l->data)->name);
                }
            }
            LLEmpty(&set);

            chat->SendMessage(p, "You granted freq $%i.", amount);
        }
    }
    else
    {
        chat->SendMessage(p, "Grant: bad value.");
    }
}

local helptext_t grantexp_help =
"Targets: player, freq, or arena\n"
"Args: {amount}\n"
"Grants the target specified amount of experience.\n";

local void Cgrantexp(const char *params, Player *p, const Target *target)
{
    char *next;
    int amount = strtol(params, &next, 0);

    if (next != params)
    {
        if (target->type == T_PLAYER) //Single player
        {
            if (db->Loaded(target->u.p))
            {
                GiveExp(target->u.p, amount);
                chat->SendMessage(target->u.p, "You were granted %i exp.", amount);
                chat->SendMessage(p, "You granted %i exp to %s.", amount, target->u.p->name);
            }
            else
            {
                chat->SendMessage(p, "That player has no data loaded.");
            }
        }
        else if (target->type == T_FREQ)
        {
            LinkedList set = LL_INITIALIZER;
            Link *l;
            pd->TargetToSet(target, &set);

            for (l = LLGetHead(&set); l; l = l->next)
            {
                if (db->Loaded(l->data))
                {
                    GiveExp(l->data, amount);
                    chat->SendMessage(l->data, "You were granted %i exp.", amount);
                }
                else
                {
                    chat->SendMessage(p, "Player %s has no data loaded.", ((Player*)l->data)->name);
                }
            }
            LLEmpty(&set);

            chat->SendMessage(p, "You granted target freq %i exp.", amount);
        }
    }
    else
    {
        chat->SendMessage(p, "GrantExp: bad value.");
    }
}

local helptext_t setexp_help =
"Targets: player\n"
"Args: {amount}\n"
"Sets the player's exp to the specified amount.\n";

local void Csetexp(const char *params, Player *p, const Target *target)
{
    char *next;
    int amount = strtol(params, &next, 0);

    if (next != params)
    {
        if (target->type == T_PLAYER) //Single player
        {
            if (db->Loaded(target->u.p))
            {
                SetExp(target->u.p, amount);
                chat->SendMessage(target->u.p, "Your exp was set to %i.", amount);
                chat->SendMessage(p, "You set %s's exp to %i.", target->u.p->name, amount);
            }
            else
            {
                chat->SendMessage(p, "That player has no data loaded.");
            }
        }
    }
    else
    {
        chat->SendMessage(p, "SetExp: bad value.");
    }
}

local helptext_t setmoney_help =
"Targets: player\n"
"Args: {amount}\n"
"Sets the player's money to the specified amount.\n";

local void Csetmoney(const char *params, Player *p, const Target *target)
{
    char *next;
    int amount = strtol(params, &next, 0);

    if (next != params)
    {
        if (target->type == T_PLAYER) //Single player
        {
            if (db->Loaded(target->u.p))
            {
                SetMoney(target->u.p, amount, MONEY_TYPE_GRANT);
                chat->SendMessage(target->u.p, "Your money was set to $%i.", amount);
                chat->SendMessage(p, "You set %s's money to $%i.", target->u.p->name, amount);
            }
            else
            {
                chat->SendMessage(p, "That player has no data loaded.");
            }
        }
    }
    else
    {
        chat->SendMessage(p, "SetMoney: bad value.");
    }
}

local helptext_t money_help =
"Targets: player, or none\n"
"Args: [-d]\n"
"Shows you the amount of money in your account.\n"
"MODERATOR ONLY: If directed to a player, this command will show their money and exp.\n"
"The -d flag will optionally show you their detailed money info.";

static char *money_types[] =
{
	"Give",
	"Grant",
	"Flag",
	"Ball",
	"Kill",
	"Buy/Sell"
};

local void Cmoney(const char *params, Player *p, const Target *target)
{
    if (target->type == T_PLAYER) //Single player. May look bad, but cmd_money and privcmd_money are different
    {
		if (db->Loaded(target->u.p))
		{
			if (strstr(params, "-d"))
			{
				int i;
				int money_total = 0;

				chat->SendMessage(p, "Player %s: Exp: %i", target->u.p->name, GetExp(target->u.p));
				chat->SendMessage(p, "Total money: $%i", GetMoney(target->u.p));

				for (i = 0; i < MONEY_TYPE_COUNT; i++)
				{
					chat->SendMessage(p, "%s money: $%i", money_types[i], GetMoneyType(target->u.p, i));
					money_total += GetMoneyType(target->u.p, i);
				}

				chat->SendMessage(p, "Difference: $%i", GetMoney(target->u.p) - money_total);
			}
			else
			{
				chat->SendMessage(p, "Player %s has $%i dollars in their account, and %i experience.", target->u.p->name, GetMoney(target->u.p), GetExp(target->u.p));
			}
		}
		else
		{
			chat->SendMessage(p, "Player has no data loaded.");
		}
    }
    else
    {
        if (db->Loaded(p))
        {
            chat->SendMessage(p, "You have $%i dollars in your account and %i experience.", GetMoney(p), GetExp(p));
        }
        else
        {
            chat->SendMessage(p, "You have no data loaded.");
        }
    }
}



local helptext_t showmoney_help =
"Targets: player\n"
"Args: none\n"
"Shows the target player the amount of money in your account.\n";

local void Cshowmoney(const char *params, Player *p, const Target *target)
{
    if (target->type == T_PLAYER) //Single player
    {
        if (db->Loaded(p))
        {
            chat->SendMessage(target->u.p, "Player %s has $%i in their account.", p->name, GetMoney(p));
            chat->SendMessage(p, "Sent account status to %s.", target->u.p->name);
        }
        else
        {
            chat->SendMessage(p, "You have no data loaded.");
        }
    }
}

local helptext_t showexp_help =
"Targets: player\n"
"Args: none\n"
"Shows the target player the amount of exp in your account.\n";

local void Cshowexp(const char *params, Player *p, const Target *target)
{
    if (target->type == T_PLAYER) //Single player
    {
        if (db->Loaded(p))
        {
            chat->SendMessage(target->u.p, "Player %s has %i experience.", p->name, GetExp(p));
            chat->SendMessage(p, "Sent experience status to %s.", target->u.p->name);
        }
        else
        {
            chat->SendMessage(p, "You have no data loaded.");
        }
    }
}



local Ihsmoney myint =
{
	INTERFACE_HEAD_INIT(I_HS_MONEY, "hscore_money")
	GiveMoney, SetMoney, GetMoney, GetMoneyType,
	GiveExp, SetExp, GetExp,
};

EXPORT int MM_hscore_money(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		db = mm->GetInterface(I_HS_DATABASE, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);

		if (!lm || !chat || !cfg || !cmd || !db || !pd)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(db);
			mm->ReleaseInterface(pd);

			return MM_FAIL;
		}

		mm->RegInterface(&myint, ALLARENAS);

		cmd->AddCommand("give", Cgive, give_help);
		cmd->AddCommand("grant", Cgrant, grant_help);
		cmd->AddCommand("grantexp", Cgrantexp, grantexp_help);
		cmd->AddCommand("setmoney", Csetmoney, setmoney_help);
		cmd->AddCommand("setexp", Csetexp, setexp_help);
		cmd->AddCommand("money", Cmoney, money_help);
		cmd->AddCommand("showmoney", Cshowmoney, showmoney_help);
		cmd->AddCommand("showexp", Cshowexp, showexp_help);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&myint, ALLARENAS))
			return MM_FAIL;

		cmd->RemoveCommand("give", Cgive);
		cmd->RemoveCommand("grant", Cgrant);
		cmd->RemoveCommand("grantexp", Cgrantexp);
		cmd->RemoveCommand("setmoney", Csetmoney);
		cmd->RemoveCommand("setexp", Csetexp);
		cmd->RemoveCommand("money", Cmoney);
		cmd->RemoveCommand("showmoney", Cshowmoney);
		cmd->RemoveCommand("showexp", Cshowexp);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(db);
		mm->ReleaseInterface(pd);

		return MM_OK;
	}
	return MM_FAIL;
}

