#ifndef HS_DATABASE_H
#define HS_DATABASE_H

#define I_HS_DATABASE "hs_database-1"

enum
{
	MONEY_TYPE_GIVE = 0,
	MONEY_TYPE_GRANT,
	MONEY_TYPE_FLAG,
	MONEY_TYPE_BALL,
	MONEY_TYPE_KILL,
	MONEY_TYPE_BUYSELL,
};

#define MONEY_TYPE_COUNT 6

typedef enum
{
	NOT_LOADED=0,			/* Not loaded from the database. */
	LOADED,					/* Loaded from the database */
} loading_status_t;

typedef enum
{
	HULL_NOT_LOADED=0,		/* Not fetched from the DB. */
	HULL_LOADED,			/* Loaded and present. */
	HULL_NO_HULL,		    /* Loaded but not owned */
} hull_status_t;

typedef struct Property
{
	char name[20];
	int setting;
} Property;

typedef struct Item
{
	int id;
	char name[20];
	char short_description[35];
	char description[255];
	int buy_price;
	int sell_price;

	int item_type;
	int item_max;

	int ship_mask;

	int exp_required;

	char frequent_changes;

	LinkedList *property_list;
} Item;

typedef struct ShipItem
{
	int id; //needed?
	Item *item;
	int amount;
	int data;
} ShipItem;

typedef struct HullData
{
	int id;
	LinkedList *inventory_list;
} HullData;

typedef struct HSData
{
	int id;

	loading_status_t loading_status;

	int money;

	int money_type[MONEY_TYPE_COUNT];

	int exp;

	hull_status_t hull_status[8];
	HullData * hulls[8];

} HSData;

typedef struct Store
{
	int id;
	char name[20];
	char description[255];
	char region[20];

	LinkedList *item_list;
} Store;

typedef struct Category
{
	int id;
	char name[20];
	char description[75];

	LinkedList *item_list;
} Category;

typedef struct Ihsdatabase
{
	INTERFACE_HEAD_DECL
	HSData * (*GetHSData)(Player *p);
	int (*Loaded)(Player *p);
	LinkedList * (*GetItemList)();
	LinkedList * (*GetStoreList)(Arena *arena);
	LinkedList * (*GetCategoryList)(Arena *arena);

	void (*AddShip)(Player *p, int ship);
	void (*RemoveShip)(Player *p, int ship);

} Ihsdatabase;



#endif /* HS_DATABASE_H */
