#ifndef HSCORE_WARP_H
#define HSCORE_WARP_H

#define I_HSCORE_WARP "hscore_warp-1"

typedef struct Ihscorewarp
{
	INTERFACE_HEAD_DECL
	void (*WarpPlayer)(Player *p, int dest_x, int dest_y, int v_x, int v_y); //pixels
	void (*WarpPlayerExtra)(Player *p, int dest_x, int dest_y, int v_x, int v_y, int rotation, int status, int bounty);
	void (*WarpPlayerWithWeapon)(Player *p, int dest_x, int dest_y, int v_x, int v_y, int rotation, int status, int bounty, struct Weapons *weapon);
	void (*CreateWarpPacket)(Player *p, int dest_x, int dest_y, int v_x, int v_y, int rotation, int status, int bounty, struct S2CWeapons *packet);
	void (*WarptoPlayer)(Player *p, int tile_x, int tile_y);

} Ihscorewarp;

#endif /* HSCORE_WARP_H */
