#ifndef HSCORE_MONEY_H
#define HSCORE_MONEY_H

/* pyinclude: hscore/hscore_money.h */

#define I_HSCORE_MONEY "hscore_money-1"

typedef enum MoneyType
{
	/* pyconst: enum, "MONEY_TYPE_*" */
	//for /?give
	MONEY_TYPE_GIVE = 0,

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
	/* pyint: use, impl */

	void (*giveMoney)(Player *p, int amount, MoneyType type);
	/* pyint: player, int, int -> void */
	void (*setMoney)(Player *p, int amount, MoneyType type); //beware. know what you're doing
	/* pyint: player, int, int -> void */

	int (*getMoney)(Player *p);
	/* pyint: player -> int */
	int (*getMoneyType)(Player *p, MoneyType type); //used only for /?money -d
	/* pyint: player, int -> int */

	void (*giveExp)(Player *p, int amount);
	/* pyint: player, int -> void */
	void (*setExp)(Player *p, int amount); //beware. know what you're doing
	/* pyint: player, int -> void */

	int (*getExp)(Player *p);
	/* pyint: player -> int */
} Ihscoremoney;

#endif //HSCORE_MONEY_H
