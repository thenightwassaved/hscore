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

local helptext_t moneyHelp =
"Targets: none or player\n"
"Args: [-d]\n"
"Shows you your money and exp.\n"
"The -d switch will give additional details.\n";

local void moneyCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t grantHelp =
"Targets: player or freq or arena\n"
"Args: [{-f}] <amount>\n"
"Adds the specified amount of money to all of the targeted player's account.\n"
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void grantCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t grantExpHelp =
"Targets: none\n"
"Args: [{-f}] <amount>\n"
"Adds the specified amount of exp to all of the targeted player's account.\n"
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void grantExpCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t setMoneyHelp =
"Targets: player or freq or arena\n"
"Args: [{-f}] <amount>\n"
"Set the player's money to the specified amount.\n"
"Please be sure this is what you want. You probably want /?grant.\n"
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void setMoneyCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t setExpHelp =
"Targets: player or freq or arena\n"
"Args: [{-f}] <amount>\n"
"Set the player's exp to the specified amount.\n"
"Please be sure this is what you want. You probably want /?grantexp.\n"
"For typo safety, the {-f} must be specified when granting to more than one player.\n";

local void setExpCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t giveHelp =
"Targets: player\n"
"Args: <amount>\n"
"Gives the target player the specified amount from your own account.\n"
"NOTE: You will not be able to give if it would leave you with less than"
"the minimum required give balance.\n";

local void giveCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t showMoneyHelp =
"Targets: player\n"
"Args: [{-e}]\n"
"Shows the target player your account's money.\n"
"If {-e} is added, it will show both money and exp.\n";

local void showMoneyCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t showExpHelp =
"Targets: player\n"
"Args: none\n"
"Shows the target player your account's experience.\n";

local void showExpCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local void giveMoney(Player *p, int amount, MoneyType type)
{

}

local void setMoney(Player *p, int amount, MoneyType type)
{
	giveMoney(p, amount - getMoney(p), type);
}

local int getMoney(Player *p)
{

}

local int getMoneyType(Player *p, MoneyType type)
{

}

local void giveExp(Player *p, int amount)
{

}

local void setExp(Player *p, int amount)
{
	giveExp(p, amount - getExp(p));
}

local int getExp(Player *p)
{

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
		cmd->AddCommand("grantexp", grantExpCommand, ALLARENAS, grantExp);
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

