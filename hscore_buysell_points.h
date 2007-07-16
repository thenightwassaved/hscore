#ifndef HSCORE_BUYSELL_POINTS_H
#define HSCORE_BUYSELL_POINTS_H

#define I_HSCORE_BUYSELL_POINTS "hscore_buysell_points-1"

typedef struct PointsArenaData
{
	int usingPoints;
	char arenaIdentifier[32];
	int useGlobal;
	int sellingAllowed;
} PointsArenaData;

typedef struct PointsPlayerData
{
	int globalPoints;
	int shipPoints[8];
} PointsPlayerData;

typedef struct Ihscorebuysellpoints
{
	INTERFACE_HEAD_DECL
	void (*setSellingAllowed)(Arena *a, int allowed);

} Ihscorebuysellpoints;

#endif //HSCORE_BUYSELL_POINTS_H
