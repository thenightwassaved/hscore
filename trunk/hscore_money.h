#ifndef HSCORE_MONEY_H
#define HSCORE_MONEY_H

#define I_HSCORE_MONEY "hscore_money-1"

typedef enum MoneyType
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

#define MONEY_TYPE_COUNT 7

typedef struct Ihscoremoney
{
	INTERFACE_HEAD_DECL

	void (*giveMoney)(Player *p, int amount, MoneyType type);
	void (*setMoney)(Player *p, int amount, MoneyType type); //beware. know what you're doing

	int (*getMoney)(Player *p);
	int (*getMoneyType)(Player *p, MoneyType type); //used only for /?money -d

	void (*giveExp)(Player *p, int amount);
	void (*setExp)(Player *p, int amount); //beware. know what you're doing

	int (*getExp)(Player *p);
} Ihscoremoney;



#endif //HSCORE_MONEY_H
