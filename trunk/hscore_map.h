#ifndef HS_MAP_H
#define HS_MAP_H

#define I_HS_MAP "hs_map-1"


typedef struct Ihsmap
{
	INTERFACE_HEAD_DECL

	int (*ItemAvalible)(Player *p, Item *item);

	LinkedList * (*GetAvalibleStores)(Player *p, Item *item);

} Ihsmap;



#endif /* HS_MAP_H */
