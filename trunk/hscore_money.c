#include <string.h>
#include <stdlib.h>

#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihscoredatabase *database;

//interface prototypes
local void giveMoney(Player *p, int amount, MoneyType type);
local void setMoney(Player *p, int amount, MoneyType type);
local int getMoney(Player *p);
local int getMoneyType(Player *p, MoneyType type);
local void giveExp(Player *p, int amount);
local void setExp(Player *p, int amount);
local int getExp(Player *p);

static char *moneyTypeNames[] =
{
	"Give",
	"Grant",
	"Buy&Sell",
	"Kill",
	"Flag",
	"Ball",
	"Event"
};

local helptext_t moneyHelp =
"Targets: none or player\n"
"Args: [{-d}]\n"
"Shows you your money and exp.\n"
"When sent to another player, the {-d} switch will give additional details.\n";

local void moneyCommand(const char *command, const char *params, Player *p, const Target *target)
{
	if (target->type == T_PLAYER) //private command
	{
		Player *t = target->u.p;

		if (database->isLoaded(t))
		{
			if (strstr(params, "-d")) //wants details
			{
				int total = 0;

				chat->SendMessage(p, "Player %s: money: %i, exp: %i", t->name, getMoney(t), getExp(t));

				for (int i = 0; i < MONEY_TYPE_COUNT; i++)
				{
					chat->SendMessage(p, "%s money: $%i", moneyTypeNames[i], getMoneyType(t, i));
					total += getMoneyType(t, i);
				}

				chat->SendMessage(p, "Difference: $%i", getMoney(t) - total);
			}
			else //no details
			{
				chat->SendMessage(p, "Player %s has $%i dollars in their account, and %i experience.", t->name, getMoney(t), getExp(t));
			}
		}
		else
		{
			chat->SendMessage(p, "Player %s has no data loaded.", t->name);
		}
	}
	else //not private, assume public
	{
		if (database->isLoaded(p))
		{
			chat->SendMessage(p, "You have $%i dollars in your account and %i experience.", getMoney(p), getExp(p));
		}
		else
		{
			chat->SendMessage(p, "You have no data loaded.");
		}
	}
}

local helptext_t grantHelp =
"Targets: player or freq or arena\n"
"Args: [{-f}] [{-q}] <amount> [<message>]\n"
"Adds the specified amount of money to all of the targeted player's account.\n"
"If the {-q} switch is specified, then the command will not notify the player(s)."
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void grantCommand(const char *command, const char *params, Player *p, const Target *target)
{
    char *next;
    int amount = strtol(params, &next, 0); //get the amount of money to give

	if (next != params) //good value
	{
		if (target->type == T_PLAYER) //private command
		{
			Player *t = target->u.p;

			if (database->isLoaded(t))
			{
				//FIXME
			}
			else
			{
				chat->SendMessage(p, "Player %s has no data loaded.", t->name);
			}
		}
		else //not private
		{
			chat->SendMessage(p, "Not implemented yet.");
		}
	}
	else
	{
		chat->SendMessage(p, "Error: bad amount.");
	}
}

local helptext_t grantExpHelp =
"Targets: none\n"
"Args: [{-f}] [{-q}] <amount> [<message>]\n"
"Adds the specified amount of exp to all of the targeted player's account.\n"
"If the {-q} switch is specified, then the command will not notify the player(s)."
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void grantExpCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

local helptext_t setMoneyHelp =
"Targets: player or freq or arena\n"
"Args: [{-f}] [{-q}] <amount> [<message>]\n"
"Set the player's money to the specified amount.\n"
"Please be sure this is what you want. You probably want /?grant.\n"
"If the {-q} switch is specified, then the command will not notify the player(s)."
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void setMoneyCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

local helptext_t setExpHelp =
"Targets: player or freq or arena\n"
"Args: [{-f}] [{-q}] <amount> [<message>]\n"
"Set the player's exp to the specified amount.\n"
"Please be sure this is what you want. You probably want /?grantexp.\n"
"If the {-q} switch is specified, then the command will not notify the player(s)."
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void setExpCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//FIXME
}

local helptext_t giveHelp =
"Targets: player\n"
"Args: <amount> [message]\n"
"Gives the target player the specified amount from your own account.\n"
"NOTE: You will not be able to give if it would leave you with less than"
"the minimum required give balance.\n";

local void giveCommand(const char *command, const char *params, Player *p, const Target *target)
{
	char *next;
	char *message;
	int amount = strtol(params, &next, 0);

	if (next == params)
	{
		chat->SendMessage(p, "Give: bad amount.");
		return;
	}

	while (*next == ',' || *next == ' ') next++; //remove whitespace before the message

	message = next;
	if (message[0] == '\0')
	{
		message = NULL;
	}

	if (target->type == T_PLAYER) //private command
	{
		Player *t = target->u.p;

		if (database->isLoaded(p))
		{
			if (database->isLoaded(t))
			{
	            int minMoney = cfg->GetInt(GLOBAL, "hyperspace", "minmoney", 1000);
	            int minGive = cfg->GetInt(GLOBAL, "hyperspace", "mingive", 1);
	            int maxGive = cfg->GetInt(GLOBAL, "hyperspace", "maxgive", 100000000);

	            if (getMoney(p) - amount >= minMoney)
	            {
					if (amount < maxGive)
					{
						if (amount > minGive)
						{
							giveMoney(t, amount, MONEY_GIVE);
							giveMoney(p, -amount, MONEY_GIVE);

							if (message == NULL)
							{
								chat->SendMessage(t, "Player %s gave you $%i.", p->name, amount);
							}
							else
							{
								chat->SendMessage(t, "Player %s gave you $%i. Message:\"%s\"", p->name, amount, message);
							}

							chat->SendMessage(p, "You gave %s $%i.", t->name, amount);
						}
						else
						{
							chat->SendMessage(p, "You cannot give that little. The minimum is %i.", minGive);
						}
					}
					else
					{
						chat->SendMessage(p, "You cannot give that much. The maximium is %i.", maxGive);
					}
				}
				else
				{
					chat->SendMessage(p, "You must leave at least %i in your own account.", minMoney);
				}
			}
			else
			{
				chat->SendMessage(p, "Player %s has no data loaded.", t->name);
			}
		}
		else
		{
			chat->SendMessage(p, "You have no data loaded.");
		}
	}
	else //not private
	{
		chat->SendMessage(p, "You must target a player.");
	}
}

local helptext_t showMoneyHelp =
"Targets: player\n"
"Args: [{-e}]\n"
"Shows the target player your account's money.\n"
"If {-e} is added, it will show both money and exp.\n";

local void showMoneyCommand(const char *command, const char *params, Player *p, const Target *target)
{
	if (target->type == T_PLAYER) //private command
	{
		Player *t = target->u.p;

		if (database->isLoaded(p))
		{
			if (strstr(params, "-e")) //wants exp too
			{
				chat->SendMessage(t, "Player %s has $%i dollars in their account and %i experience.", p->name, getMoney(p), getExp(p));
				chat->SendMessage(p, "Sent money and exp status to %s", t->name);
			}
			else //no exp
			{
				chat->SendMessage(t, "Player %s has $%i dollars in their account.", p->name, getMoney(p));
				chat->SendMessage(p, "Sent money status to %s", t->name);
			}
		}
		else
		{
			chat->SendMessage(p, "You have no data loaded.");
		}
	}
	else //not private
	{
		chat->SendMessage(p, "You must target a player.");
	}
}

local helptext_t showExpHelp =
"Targets: player\n"
"Args: none\n"
"Shows the target player your account's experience.\n";

local void showExpCommand(const char *command, const char *params, Player *p, const Target *target)
{
	if (target->type == T_PLAYER) //private command
	{
		Player *t = target->u.p;

		if (database->isLoaded(p))
		{
			chat->SendMessage(t, "Player %s has %i experience.", p->name, getExp(p));
			chat->SendMessage(p, "Sent exp status to %s", t->name);
		}
		else
		{
			chat->SendMessage(p, "You have no data loaded.");
		}
	}
	else //not private
	{
		chat->SendMessage(p, "You must target a player.");
	}
}

local void giveMoney(Player *p, int amount, MoneyType type)
{
	if (database->isLoaded(p))
	{
		PerPlayerData *playerData = database->getPerPlayerData(p);

		playerData->money += amount;
		playerData->moneyType[type] += amount;
	}
	else
	{
		lm->LogP(L_WARN, "hscore_money", p, "tried to give money for unloaded player.");
	}
}

local void setMoney(Player *p, int amount, MoneyType type)
{
	giveMoney(p, amount - getMoney(p), type);
}

local int getMoney(Player *p)
{
	if (database->isLoaded(p))
	{
		PerPlayerData *playerData = database->getPerPlayerData(p);

		return playerData->money;
	}
	else
	{
		lm->LogP(L_WARN, "hscore_money", p, "tried to get money for unloaded player; returning 0.");

		return 0;
	}
}

local int getMoneyType(Player *p, MoneyType type)
{
	if (database->isLoaded(p))
	{
		PerPlayerData *playerData = database->getPerPlayerData(p);

		return playerData->moneyType[type];
	}
	else
	{
		lm->LogP(L_WARN, "hscore_money", p, "tried to get money type for unloaded player; returning 0.");

		return 0;
	}
}

local void giveExp(Player *p, int amount)
{
	if (database->isLoaded(p))
	{
		PerPlayerData *playerData = database->getPerPlayerData(p);

		playerData->exp += amount;
	}
	else
	{
		lm->LogP(L_WARN, "hscore_money", p, "tried to give exp for unloaded player.");
	}
}

local void setExp(Player *p, int amount)
{
	giveExp(p, amount - getExp(p));
}

local int getExp(Player *p)
{
	if (database->isLoaded(p))
	{
		PerPlayerData *playerData = database->getPerPlayerData(p);

		return playerData->exp;
	}
	else
	{
		lm->LogP(L_WARN, "hscore_money", p, "tried to get exp for unloaded player; returning 0.");

		return 0;
	}
}

local Ihscoremoney interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_MONEY, "hscore_money")
	giveMoney, setMoney, getMoney, getMoneyType,
	giveExp, setExp, getExp,
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
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);

		if (!lm || !chat || !cfg || !cmd || !database)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(database);

			return MM_FAIL;
		}

		mm->RegInterface(&interface, ALLARENAS);

		cmd->AddCommand("money", moneyCommand, ALLARENAS, moneyHelp);
		cmd->AddCommand("grant", grantCommand, ALLARENAS, grantHelp);
		cmd->AddCommand("grantexp", grantExpCommand, ALLARENAS, grantExpHelp);
		cmd->AddCommand("setmoney", setMoneyCommand, ALLARENAS, setMoneyHelp);
		cmd->AddCommand("setexp", setExpCommand, ALLARENAS, setExpHelp);
		cmd->AddCommand("give", giveCommand, ALLARENAS, giveHelp);
		cmd->AddCommand("showmoney", showMoneyCommand, ALLARENAS, showMoneyHelp);
		cmd->AddCommand("showexp", showExpCommand, ALLARENAS, showExpHelp);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&interface, ALLARENAS))
		{
			return MM_FAIL;
		}

		cmd->RemoveCommand("money", moneyCommand, ALLARENAS);
		cmd->RemoveCommand("grant", grantCommand, ALLARENAS);
		cmd->RemoveCommand("grantexp", grantExpCommand, ALLARENAS);
		cmd->RemoveCommand("setmoney", setMoneyCommand, ALLARENAS);
		cmd->RemoveCommand("setexp", setExpCommand, ALLARENAS);
		cmd->RemoveCommand("give", giveCommand, ALLARENAS);
		cmd->RemoveCommand("showmoney", showMoneyCommand, ALLARENAS);
		cmd->RemoveCommand("showexp", showExpCommand, ALLARENAS);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	return MM_FAIL;
}
