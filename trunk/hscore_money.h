#ifndef HS_MONEY_H
#define HS_MONEY_H

#define I_HS_MONEY "hs_money-2"

typedef struct Ihsmoney
{
	INTERFACE_HEAD_DECL
	void (*GiveMoney)(Player *p, int amount, int type);
	void (*SetMoney)(Player *p, int amount, int type);
	int (*GetMoney)(Player *p);
	int (*GetMoneyType)(Player *p, int type);

	void (*GiveExp)(Player *p, int amount);
	void (*SetExp)(Player *p, int amount);
	int (*GetExp)(Player *p);

} Ihsmoney;

#endif /* HS_MONEY_H */
