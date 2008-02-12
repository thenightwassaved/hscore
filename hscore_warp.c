#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "asss.h"
#include "hscore.h"
#include "hs_interdict.h"
#include "akd_lag.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ilagquery *lagq;
local Iakd_lag *akd_lag;
local Inet *net;
local Iconfig *cfg;
local Igame *game;

local void DoChecksum(struct S2CWeapons *pkt)
{
	int i;
	u8 ck = 0;
	pkt->checksum = 0;
	for (i = 0; i < sizeof(struct S2CWeapons) - sizeof(struct ExtraPosData); i++)
		ck ^= ((unsigned char*)pkt)[i];
	pkt->checksum = ck;
}

local int GetDeltaTime(Player *p)
{
	int lag;
	
	if (lagq)
	{
		struct PingSummary pping;	
		lagq->QueryPPing(p, &pping);
		lag = pping.avg;
	}
	else if (akd_lag)
	{
		akd_lag_report report;
		akd_lag->lagReport(p, &report);
		lag = report.c2s_ping_ave;
	}
	else
	{
		lag = 0;
		lm->LogP(L_INFO, "hs_warp", p, "Defaulting to 0 because of no lag interfaces.");
	}
	
	return (int)round(((double)lag)/10.0);
}

void CreateWarpPacket(Player *p, int dest_x, int dest_y, int v_x, int v_y, int rotation, int status, int bounty, struct S2CWeapons *packet)
{

	int delta_time = GetDeltaTime(p);
	
	memset(packet, 0, sizeof(*packet));
	
	packet->type = S2C_WEAPON;
	packet->rotation = rotation;
	packet->time = (current_ticks() + delta_time) & 0xFFFF;
	packet->x = dest_x;
	packet->yspeed = v_y;
	packet->playerid = p->pid;
	packet->xspeed = v_x;
	packet->checksum = 0;
	packet->status = status;
	packet->c2slatency = 0;
	packet->y = dest_y;
	packet->bounty = bounty;

	DoChecksum(packet);
}

void WarpPlayerWithWeapon(Player *p, int dest_x, int dest_y, int v_x, int v_y, int rotation, int status, int bounty, struct Weapons *weapon)
{
	Ihsinterdict *hs_interdict;

	struct S2CWeapons packet;
	CreateWarpPacket(p, dest_x, dest_y, v_x, v_y, rotation, status, bounty, &packet);
	
	packet.weapon = *weapon;
	DoChecksum(&packet);
	
	hs_interdict = mm->GetInterface(I_HS_INTERDICT, p->arena);
	if (hs_interdict != NULL)
	{
		hs_interdict->RemoveInterdiction(p);
		mm->ReleaseInterface(hs_interdict);
	}

	p->position.bounty = packet.bounty;
	
	net->SendToArena(p->arena, NULL, (byte*)&packet, sizeof(struct S2CWeapons) - sizeof(struct ExtraPosData), NET_RELIABLE);	
}

local void WarpPlayerExtra(Player *p, int dest_x, int dest_y, int v_x, int v_y, int rotation, int status, int bounty)
{
	Ihsinterdict *hs_interdict;
	
	struct S2CWeapons packet;
	CreateWarpPacket(p, dest_x, dest_y, v_x, v_y, rotation, status, bounty, &packet);

	hs_interdict = mm->GetInterface(I_HS_INTERDICT, p->arena);
	if (hs_interdict != NULL)
	{
		hs_interdict->RemoveInterdiction(p);
		mm->ReleaseInterface(hs_interdict);
	}

	p->position.bounty = packet.bounty;
	
	net->SendToArena(p->arena, NULL, (byte*)&packet, sizeof(struct S2CWeapons) - sizeof(struct ExtraPosData), NET_RELIABLE);	
}

local void WarpPlayer(Player *p, int dest_x, int dest_y, int v_x, int v_y)
{
	Ihsinterdict *hs_interdict;
	
	struct S2CWeapons packet;
	CreateWarpPacket(p, dest_x, dest_y, v_x, v_y, p->position.rotation, p->position.status, p->position.bounty, &packet);
	
	hs_interdict = mm->GetInterface(I_HS_INTERDICT, p->arena);
	if (hs_interdict != NULL)
	{
		hs_interdict->RemoveInterdiction(p);
		mm->ReleaseInterface(hs_interdict);
	}
	
	net->SendToArena(p->arena, NULL, (byte*)&packet, sizeof(struct S2CWeapons) - sizeof(struct ExtraPosData), NET_RELIABLE);	
}

local void WarptoPlayer(Player *p, int tile_x, int tile_y)
{
	Target t;
	t.type = T_PLAYER;
	t.u.p = p;	
	
	game->WarpTo(&t, tile_x, tile_y);
}

EXPORT const char info_hscore_warp[] = "v1.1 Dr Brain <drbrain@gmail.com>";

local Ihscorewarp myint =
{
	INTERFACE_HEAD_INIT(I_HSCORE_WARP, "hscore_warp")
	WarpPlayer, WarpPlayerExtra, WarpPlayerWithWeapon, CreateWarpPacket, WarptoPlayer
};

EXPORT int MM_hscore_warp(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		lagq = mm->GetInterface(I_LAGQUERY, ALLARENAS);
		net = mm->GetInterface(I_NET, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		game = mm->GetInterface(I_GAME, ALLARENAS);

		if (!lm || !net || !cfg || !game) return MM_FAIL;
		
		if (!lagq)
		{
			akd_lag = mm->GetInterface(I_AKD_LAG, ALLARENAS);
		}

		mm->RegInterface(&myint, ALLARENAS);
		
		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(lagq);
		mm->ReleaseInterface(akd_lag);
		mm->ReleaseInterface(net);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(game);

		if (mm->UnregInterface(&myint, ALLARENAS))
			return MM_FAIL;

		return MM_OK;
	}

	return MM_FAIL;
}

