#ifndef HSCORE_MONEY_H
#define HSCORE_MONEY_H

#define I_HS_ITEMS "hscore_money-1"

enum
{
	//for /?give
	MONEY_TYPE_GIVE,

	//for /?grant
	MONEY_TYPE_GRANT,

	//for ?buy and ?sell
	MONEY_TYPE_BUYSELL,

	//for money from kills
	MONEY_TYPE_KILL,

	//for money from flag games
	MONEY_TYPE_FLAG,

	//for money from soccer games
	MONEY_TYPE_BALL,

	//for money from module driven events
	MONEY_TYPE_EVENT
} MoneyType;

typedef struct Ihscoremoney
{
	INTERFACE_HEAD_DECL

	void (*giveMoney)(Player *p, int amount, MoneyType type);
	void (*setMoney)(Player *p, int amount, MoneyType type);

	int (*getMoney)(Player *p);
	int (*getMoneyType)(Player *p, MoneyType type); //used only for /?money -d

	void (*giveExp)(Player *p, int amount);
	void (*setExp)(Player *p, int amount);

	int (*getExp)(Player *p);
} Ihscoremoney;



#endif //HSCORE_MONEY_H