#include <math.h>
#include "asss.h"
#include "hscore.h"

local Imodman *mm;
local Ichat *chat;
local Igame *game;
local Ihscoreitems *items;
local Inet *net;
local Iarenaman *aman;

local HashTable *inCenterWarper;

local int spawnkey;

typedef struct SpawnData
{
	int checkx1;
	int checky1;
	int checkx2;
	int checky2;

	int spawnx1;
	int spawny1;
	int spawnx2;
	int spawny2;
} SpawnData;

local void prize(Player *p)
{
	Target *t;
	t->type = T_PLAYER;
	t->u.p = p;

	int bomblevel = items->getPropertySum(p, p->pkt.ship, "bomblevel");
	if (bomblevel) game->GivePrize(t, 9, bomblevel);
	int gunlevel = items->getPropertySum(p, p->pkt.ship, "gunlevel");
	if (gunlevel) game->GivePrize(t, 8, gunlevel);

	int repel = items->getPropertySum(p, p->pkt.ship, "repel");
	if (repel) game->GivePrize(t, 21, repel);
	int burst = items->getPropertySum(p, p->pkt.ship, "burst");
	if (burst) game->GivePrize(t, 22, burst);
	int thor = items->getPropertySum(p, p->pkt.ship, "thor");
	if (thor) game->GivePrize(t, 23, thor);
	int portal = items->getPropertySum(p, p->pkt.ship, "portal");
	if (portal) game->GivePrize(t, 24, portal);
	int decoy = items->getPropertySum(p, p->pkt.ship, "decoy");
	if (decoy) game->GivePrize(t, 25, decoy);
	int brick = items->getPropertySum(p, p->pkt.ship, "brick");
	if (brick) game->GivePrize(t, 26, brick);
	int rocket = items->getPropertySum(p, p->pkt.ship, "rocket");
	if (rocket) game->GivePrize(t, 27, rocket);

	int xradar = items->getPropertySum(p, p->pkt.ship, "xradar");
	if (xradar) game->GivePrize(t, 6, xradar);
	int cloak = items->getPropertySum(p, p->pkt.ship, "cloak");
	if (cloak) game->GivePrize(t, 5, cloak);
	int stealth = items->getPropertySum(p, p->pkt.ship, "stealth");
	if (stealth) game->GivePrize(t, 4, stealth);
	int antiwarp = items->getPropertySum(p, p->pkt.ship, "antiwarp");
	if (antiwarp) game->GivePrize(t, 20, antiwarp);

	int thrust = items->getPropertySum(p, p->pkt.ship, "thrust");
	if (thrust) game->GivePrize(t, 11, thrust);
	int speed = items->getPropertySum(p, p->pkt.ship, "speed");
	if (speed) game->GivePrize(t, 12, speed);
	int energy = items->getPropertySum(p, p->pkt.ship, "energy");
	if (energy) game->GivePrize(t, 2, energy);
	int recharge = items->getPropertySum(p, p->pkt.ship, "recharge");
	if (recharge) game->GivePrize(t, 1, recharge);
	int rotation = items->getPropertySum(p, p->pkt.ship, "rotation");
	if (rotation) game->GivePrize(t, 3, rotation);

	int bounce = items->getPropertySum(p, p->pkt.ship, "bounce");
	if (bounce) game->GivePrize(t, 10, bounce);
	int prox = items->getPropertySum(p, p->pkt.ship, "prox");
	if (prox) game->GivePrize(t, 16, prox);
	int shrapnel = items->getPropertySum(p, p->pkt.ship, "shrapnel");
	if (shrapnel) game->GivePrize(t, 19, shrapnel);
	int multifire = items->getPropertySum(p, p->pkt.ship, "multifire");
	if (multifire) game->GivePrize(t, 15, multifire);
}

local void Pppk(Player *p, byte *p2, int len)
{
    struct C2SPosition *pos = (struct C2SPosition *)p2;

	Arena *arena = p->arena;
	SpawnData *sd = P_ARENA_DATA(arena, spawnkey);
	Target t;
	t.type = T_PLAYER;
	t.u.p = p;

	/* handle common errors */
	if (!arena) return;

	/* speccers don't get their position sent to anyone */
	if (p->p_ship == SHIP_SPEC)
		return;

	if (sd->checkx1 != 0) //module attached to the arena? (maybe a bad test, eh?)
	{
		if (sd->checkx1 < pos->x>>4 && pos->x>>4 < sd->checkx2 && sd->checky1 < pos->y>>4 && pos->y>>4 < sd->checky2)
		{
			int *last = HashGetOne(inCenterWarper, p->name);
			if (!last || *last < current_ticks())
			{
				afree(last);
				int * time = amalloc(sizeof(*time));
				*time = current_ticks() + 200;
				HashReplace(inCenterWarper, p->name, time);

				int x = (rand() % (sd->spawnx2 - sd->spawnx1)) + sd->spawnx1;
				int y = (rand() % (sd->spawny2 - sd->spawny1)) + sd->spawny1;

				game->WarpTo(&t, x, y);
				if (p->position.bounty == 0)
				{
					prize(p);
				}
			}
		}
	}
}

EXPORT int MM_hscore_prizer(int action, Imodman *_mm, Arena *arena)
{
	if(action == MM_LOAD)
	{
		mm = _mm;

		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		game = mm->GetInterface(I_GAME, ALLARENAS);
		items = mm->GetInterface(I_HSCORE_ITEMS, ALLARENAS);
		net = mm->GetInterface(I_NET, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);

		if(!chat || !game || !items || !net || !aman)
			return MM_FAIL;

		spawnkey = aman->AllocateArenaData(sizeof(SpawnData));
		if (spawnkey == -1)
            return MM_FAIL;

		net->AddPacket(C2S_POSITION, Pppk);

		inCenterWarper = HashAlloc();

		return MM_OK;
	}
	else if(action == MM_UNLOAD)
	{
		net->RemovePacket(C2S_POSITION, Pppk);

		aman->FreeArenaData(spawnkey);

		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(game);
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(net);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
    {
		SpawnData *sd = P_ARENA_DATA(arena, spawnkey);

		sd->checkx1 = cfg->GetInt(arena->cfg, "hyperspace", "checkx1", 502);
		sd->checky1 = cfg->GetInt(arena->cfg, "hyperspace", "checky1", 502);
		sd->checkx2 = cfg->GetInt(arena->cfg, "hyperspace", "checkx2", 520);
		sd->checky2 = cfg->GetInt(arena->cfg, "hyperspace", "checky2", 520);

		sd->spawnx1 = cfg->GetInt(arena->cfg, "hyperspace", "spawnx1", 322);
		sd->spawny1 = cfg->GetInt(arena->cfg, "hyperspace", "spawny1", 322);
		sd->spawnx2 = cfg->GetInt(arena->cfg, "hyperspace", "spawnx2", 700);
		sd->spawny2 = cfg->GetInt(arena->cfg, "hyperspace", "spawny2", 700);

        return MM_OK;
    }
    else if (action == MM_DETACH)
    {
        return MM_OK;
    }
	return MM_FAIL;
}
