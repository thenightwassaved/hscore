
/* dist: public */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "asss.h"
#include "clientset.h"

/* structs */
#include "packets/brick.h"

#include "settings/game.h"

typedef struct
{
	u16 cbrickid;
	ticks_t lasttime;
	LinkedList list;
	pthread_mutex_t mtx;
	int brickspan, brickmode, countbricksaswalls, bricktime;
	/*  int        int        boolean             int */
} brickdata;


/* prototypes */
local void PlayerAction(Player *p, int action, Arena *arena);
local void ArenaAction(Arena *arena, int action);

local void SendOldBricks(Player *p);

local void expire_bricks(Arena *arena);


/* packet funcs */
local void PBrick(Player *, byte *, int);


/* interface */
local void DropBrick(Arena *arena, int freq, int x1, int y1, int x2, int y2);

local Ibricks _myint =
{
	INTERFACE_HEAD_INIT(I_BRICKS, "brick")
	DropBrick
};


/* global data */

local Iconfig *cfg;
local Inet *net;
local Ilogman *lm;
local Imodman *mm;
local Iarenaman *aman;
local Imapdata *mapdata;
local Imainloop *ml;

/* big arrays */
local int brickkey;


EXPORT int MM_bricks(int action, Imodman *mm_, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = mm_;
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		net = mm->GetInterface(I_NET, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);
		mapdata = mm->GetInterface(I_MAPDATA, ALLARENAS);
		if (!net || !cfg || !lm || !aman || !ml || !mapdata)
			return MM_FAIL;

		brickkey = aman->AllocateArenaData(sizeof(brickdata));
		if (brickkey == -1)
			return MM_FAIL;

		mm->RegCallback(CB_PLAYERACTION, PlayerAction, ALLARENAS);
		mm->RegCallback(CB_ARENAACTION, ArenaAction, ALLARENAS);

		net->AddPacket(C2S_BRICK, PBrick);

		mm->RegInterface(&_myint, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&_myint, ALLARENAS))
			return MM_FAIL;

		net->RemovePacket(C2S_BRICK, PBrick);

		mm->UnregCallback(CB_PLAYERACTION, PlayerAction, ALLARENAS);
		mm->UnregCallback(CB_ARENAACTION, ArenaAction, ALLARENAS);

		aman->FreeArenaData(brickkey);

		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(net);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(mapdata);
		mm->ReleaseInterface(ml);
		return MM_OK;
	}
	return MM_FAIL;
}


void ArenaAction(Arena *arena, int action)
{
	brickdata *bd = P_ARENA_DATA(arena, brickkey);

	if (action == AA_CREATE)
	{
		pthread_mutex_init(&bd->mtx, NULL);

		LLInit(&bd->list);
		bd->cbrickid = 0;
		bd->lasttime = current_ticks();
	}

	if (action == AA_CREATE || action == AA_CONFCHANGED)
	{
		/* cfghelp: Brick:CountBricksAsWalls, arena, bool, def: 1
		 * Whether bricks snap to the edges of other bricks (as opposed
		 * to only snapping to walls) */
		bd->countbricksaswalls = cfg->GetInt(arena->cfg, "Brick", "CountBricksAsWalls", 1);
		/* cfghelp: Brick:BrickSpan, arena, int, def: 10
		 * The maximum length of a dropped brick. */
		bd->brickspan = cfg->GetInt(arena->cfg, "Brick", "BrickSpan", 10);
		/* cfghelp: Brick:BrickMode, arena, int, def: $BRICK_VIE
		 * How bricks behave when they are dropped ($BRICK_VIE=improved
		 * SubGame, $BRICK_AHEAD=drop in a line ahead of player,
		 * $BRICK_LATERAL=drop laterally across player,
		 * $BRICK_CAGE=drop 4 bricks simultaneously to create a cage) */
		bd->brickmode = cfg->GetInt(arena->cfg, "Brick", "BrickMode", BRICK_VIE);
		/* cfghelp: Brick:BrickTime, arena, int, def: 6000
		 * How long bricks last (in ticks). */
		bd->bricktime = cfg->GetInt(arena->cfg, "Brick", "BrickTime", 6000) + 10; /* 100 ms added for lag */
	}
	else if (action == AA_DESTROY)
	{
		LLEnum(&bd->list, afree);
		LLEmpty(&bd->list);
		pthread_mutex_destroy(&bd->mtx);
	}
}


void PlayerAction(Player *p, int action, Arena *arena)
{
	if (action == PA_ENTERARENA)
		SendOldBricks(p);
}


/* call with mutex */
local void expire_bricks(Arena *arena)
{
	brickdata *bd = P_ARENA_DATA(arena, brickkey);
	LinkedList *list = &bd->list;
	ticks_t gtc = current_ticks(), timeout = bd->bricktime;
	Link *l, *next;

	for (l = LLGetHead(list); l; l = next)
	{
		struct S2CBrickPacket *pkt = l->data;
		next = l->next;

		if (TICK_GT(gtc, pkt->starttime + timeout))
		{
			if (bd->countbricksaswalls)
				mapdata->DoBrick(arena, 0, pkt->x1, pkt->y1, pkt->x2, pkt->y2);
			LLRemove(list, pkt);
			afree(pkt);
		}
	}
}


/* call with mutex */
local void drop_brick(Arena *arena, int freq, int x1, int y1, int x2, int y2)
{
	brickdata *bd = P_ARENA_DATA(arena, brickkey);
	struct S2CBrickPacket *pkt = amalloc(sizeof(*pkt));

	pkt->x1 = x1; pkt->y1 = y1;
	pkt->x2 = x2; pkt->y2 = y2;
	pkt->type = S2C_BRICK;
	pkt->freq = freq;
	pkt->brickid = bd->cbrickid++;
	pkt->starttime = current_ticks();
	/* workaround for stupid priitk */
	if (pkt->starttime <= bd->lasttime)
		pkt->starttime = ++bd->lasttime;
	else
		bd->lasttime = pkt->starttime;
	LLAdd(&bd->list, pkt);

	net->SendToArena(arena, NULL, (byte*)pkt, sizeof(*pkt), NET_RELIABLE | NET_URGENT);
	lm->Log(L_DRIVEL, "<game> {%s} brick dropped (%d,%d)-(%d,%d) (freq=%d) (id=%d)",
	        arena->name,
	        x1, y1, x2, y2, freq,
	        pkt->brickid);

	if (bd->countbricksaswalls)
		mapdata->DoBrick(arena, 1, x1, y1, x2, y2);
}


void DropBrick(Arena *arena, int freq, int x1, int y1, int x2, int y2)
{
	brickdata *bd = P_ARENA_DATA(arena, brickkey);

	pthread_mutex_lock(&bd->mtx);

	expire_bricks(arena);
	drop_brick(arena, freq, x1, y1, x2, y2);

	pthread_mutex_unlock(&bd->mtx);
}


void SendOldBricks(Player *p)
{
	Arena *arena = p->arena;
	brickdata *bd = P_ARENA_DATA(arena, brickkey);
	LinkedList *list = &bd->list;
	Link *l;

	pthread_mutex_lock(&bd->mtx);

	expire_bricks(arena);

	for (l = LLGetHead(list); l; l = l->next)
	{
		struct S2CBrickPacket *pkt = (struct S2CBrickPacket*)l->data;
		net->SendToOne(p, (byte*)pkt, sizeof(*pkt), NET_RELIABLE);
	}

	pthread_mutex_unlock(&bd->mtx);
}


void PBrick(Player *p, byte *pkt, int len)
{
	Arena *arena = p->arena;
	brickdata *bd = P_ARENA_DATA(arena, brickkey);
	int dx, dy;

	if (len != 5)
	{
		lm->LogP(L_MALICIOUS, "bricks", p, "bad packet len=%i", len);
		return;
	}

	if (p->status != S_PLAYING || p->p_ship == SHIP_SPEC || !arena)
	{
		lm->LogP(L_WARN, "bricks", p, "ignored request from bad state");
		return;
	}


	dx = ((struct SimplePacket*)pkt)->d1;
	dy = ((struct SimplePacket*)pkt)->d2;

	pthread_mutex_lock(&bd->mtx);

	expire_bricks(arena);

	if (bd->brickmode == BRICK_CAGE)
	{
		int x1[4], y1[4], x2[4], y2[4];

		if (mapdata->FindBrickEndpoints(arena, 3,
					dx, dy, p->position.rotation, bd->brickspan,
					x1, y1, x2, y2))
		{
			drop_brick(arena, p->p_freq, x1[0], y1[0], x2[0], y2[0]);
			drop_brick(arena, p->p_freq, x1[1], y1[1], x2[1], y2[1]);
			drop_brick(arena, p->p_freq, x1[2], y1[2], x2[2], y2[2]);
			drop_brick(arena, p->p_freq, x1[3], y1[3], x2[3], y2[3]);
		}
	}
	else
	{
		int x1, y1, x2, y2;

		if (mapdata->FindBrickEndpoints(arena, bd->brickmode,
					dx, dy, p->position.rotation, bd->brickspan,
					&x1, &y1, &x2, &y2))
			drop_brick(arena, p->p_freq, x1, y1, x2, y2);
	}

	pthread_mutex_unlock(&bd->mtx);
}

