#include "asss.h"
#include "hscore.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iplayerdata *pd;

//interface prototypes
local void giveMoney(Player *p, int amount, MoneyType type);
local void setMoney(Player *p, int amount, MoneyType type);
local int getMoney(Player *p);
local int getMoneyType(Player *p, MoneyType type);
local void giveExp(Player *p, int amount);
local void setExp(Player *p, int amount);
local int getExp(Player *p);

local void giveMoney(Player *p, int amount, MoneyType type)
{
	chat->SendMessage(p, "hscore_moneystub.c: giveMoney(%i) called.", amount);
}

local void setMoney(Player *p, int amount, MoneyType type)
{
	chat->SendMessage(p, "setMoney should not be used.");
}

local int getMoney(Player *p)
{
	return 0; //change for testing
}

local int getMoneyType(Player *p, MoneyType type)
{
	chat->SendMessage(p, "getMoneyType should not be used.");
	return 0;
}

local void giveExp(Player *p, int amount)
{
	chat->SendMessage(p, "hscore_moneystub.c: giveExp(%i) called.", amount);
}

local void setExp(Player *p, int amount)
{
	chat->SendMessage(p, "setExp should not be used.");
}

local int getExp(Player *p)
{
	return 0; //change for testing
}

local Ihscoremoney interface =
{
	INTERFACE_HEAD_INIT(I_HSCORE_MONEY, "hscore_moneystub")
	giveMoney, setMoney, getMoney, getMoneyType,
	giveExp, setExp, getExp,
};

EXPORT int MM_hscore_moneystub(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);

		if (!lm || !chat || !pd)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(pd);

			return MM_FAIL;
		}

		mm->RegInterface(&interface, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&interface, ALLARENAS))
		{
			return MM_FAIL;
		}

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(pd);

		return MM_OK;
	}
	return MM_FAIL;
}
