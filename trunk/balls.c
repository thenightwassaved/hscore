
/* dist: public */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "asss.h"
#include "persist.h"

/* extra includes */
#include "packets/balls.h"


/* defines */

#define BALL_SEND_PRI NET_PRI_P4

#define LOCK_STATUS(arena) \
	pthread_mutex_lock((pthread_mutex_t*)P_ARENA_DATA(arena, mtxkey))
#define UNLOCK_STATUS(arena) \
	pthread_mutex_unlock((pthread_mutex_t*)P_ARENA_DATA(arena, mtxkey))


/* internal structs */
typedef struct MyBallData
{
	/* these are in centiseconds. the timer event runs with a resolution
	 * of 50 centiseconds, though, so that's the best resolution you're
	 * going to get. */
	int sendtime;
	ticks_t lastsent;
	int spawnx, spawny, spawnr;
	/* this is the delay between a goal and the ball respawning. */
	int goaldelay;
	/* this controls whether a death on a goal tile scores or not */
	int deathgoal;
} MyBallData;

/* prototypes */
local void SpawnBall(Arena *arena, int bid);
local void AABall(Arena *arena, int action);
local void PABall(Player *p, int action, Arena *arena);
local void ShipChange(Player *, int, int);
local void FreqChange(Player *, int);
local void BallKill(Arena *, Player *, Player *, int, int, int *);

/* timers */
local int BasicBallTimer(void *);

/* packet funcs */
local void PPickupBall(Player *, byte *, int);
local void PFireBall(Player *, byte *, int);
local void PGoal(Player *, byte *, int);

/* interface funcs */
local void SetBallCount(Arena *arena, int ballcount);
local void PlaceBall(Arena *arena, int bid, struct BallData *newpos);
local void EndGame(Arena *arena);
local ArenaBallData * GetBallData(Arena *arena);
local void ReleaseBallData(Arena *arena);


/* local data */
local Imodman *mm;
local Inet *net;
local Iconfig *cfg;
local Ilogman *logm;
local Iplayerdata *pd;
local Iarenaman *aman;
local Imainloop *ml;
local Imapdata *mapdata;
local Iprng *prng;

/* per arena data keys */
local int abdkey, pbdkey, mtxkey;

local Iballs _myint =
{
	INTERFACE_HEAD_INIT(I_BALLS, "ball-core")
	SetBallCount, PlaceBall, EndGame,
	GetBallData, ReleaseBallData
};



EXPORT int MM_balls(int action, Imodman *mm_, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = mm_;
		net = mm->GetInterface(I_NET, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		logm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);
		mapdata = mm->GetInterface(I_MAPDATA, ALLARENAS);
		prng = mm->GetInterface(I_PRNG, ALLARENAS);
		if (!net || !cfg || !logm || !pd || !aman || !ml || !mapdata || !prng)
			return MM_FAIL;

		abdkey = aman->AllocateArenaData(sizeof(ArenaBallData));
		pbdkey = aman->AllocateArenaData(sizeof(MyBallData));
		mtxkey = aman->AllocateArenaData(sizeof(pthread_mutex_t));
		if (abdkey == -1 || pbdkey == -1 || mtxkey == -1)
			return MM_FAIL;

		mm->RegCallback(CB_ARENAACTION, AABall, ALLARENAS);
		mm->RegCallback(CB_PLAYERACTION, PABall, ALLARENAS);
		mm->RegCallback(CB_SHIPCHANGE, ShipChange, ALLARENAS);
		mm->RegCallback(CB_FREQCHANGE, FreqChange, ALLARENAS);
		mm->RegCallback(CB_KILL, BallKill, ALLARENAS);

		net->AddPacket(C2S_PICKUPBALL, PPickupBall);
		net->AddPacket(C2S_SHOOTBALL, PFireBall);
		net->AddPacket(C2S_GOAL, PGoal);

		/* timers */
		ml->SetTimer(BasicBallTimer, 300, 50, NULL, NULL);

		mm->RegInterface(&_myint, ALLARENAS);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&_myint, ALLARENAS))
			return MM_FAIL;
		ml->ClearTimer(BasicBallTimer, NULL);
		net->RemovePacket(C2S_GOAL, PGoal);
		net->RemovePacket(C2S_SHOOTBALL, PFireBall);
		net->RemovePacket(C2S_PICKUPBALL, PPickupBall);
		mm->UnregCallback(CB_KILL, BallKill, ALLARENAS);
		mm->UnregCallback(CB_FREQCHANGE, FreqChange, ALLARENAS);
		mm->UnregCallback(CB_SHIPCHANGE, ShipChange, ALLARENAS);
		mm->UnregCallback(CB_PLAYERACTION, PABall, ALLARENAS);
		mm->UnregCallback(CB_ARENAACTION, AABall, ALLARENAS);
		aman->FreeArenaData(abdkey);
		aman->FreeArenaData(pbdkey);
		aman->FreeArenaData(mtxkey);
		mm->ReleaseInterface(mapdata);
		mm->ReleaseInterface(ml);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(logm);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(net);
		mm->ReleaseInterface(prng);
		return MM_OK;
	}
	return MM_FAIL;
}



local void send_ball_packet(Arena *arena, int bid, int rel)
{
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);
	struct BallPacket bp;
	struct BallData *bd = abd->balls + bid;

	LOCK_STATUS(arena);
	bp.type = S2C_BALL;
	bp.ballid = bid;
	bp.x = bd->x;
	bp.y = bd->y;
	bp.xspeed = bd->xspeed;
	bp.yspeed = bd->yspeed;
	bp.player = bd->carrier ? bd->carrier->pid : -1;
	if (bd->state == BALL_CARRIED)
		bp.time = 0;
	else if (bd->state == BALL_ONMAP)
		bp.time = bd->time;
	else
	{
		UNLOCK_STATUS(arena);
		return;
	}
	UNLOCK_STATUS(arena);

	net->SendToArena(arena, NULL, (byte*)&bp, sizeof(bp), rel);
}


local void phase_ball(Arena *arena, int bid, int relflags)
{
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);
	struct BallData *bd = abd->balls + bid;

	LOCK_STATUS(arena);
	bd->state = BALL_ONMAP;
	bd->x = bd->y = 30000;
	bd->xspeed = bd->yspeed = 0;
	bd->time = (ticks_t)(-1); /* this is the key for making it phased */
	bd->carrier = NULL;
	send_ball_packet(arena, bid, relflags);
	UNLOCK_STATUS(arena);
}


void SpawnBall(Arena *arena, int bid)
{
	MyBallData *pbd = P_ARENA_DATA(arena, pbdkey);

	int cx, cy, rad, x, y;
	struct BallData d;

	d.state = BALL_ONMAP;
	d.xspeed = d.yspeed = 0;
	d.carrier = NULL;
	d.time = current_ticks();

	cx = pbd->spawnx;
	cy = pbd->spawny;
	rad = pbd->spawnr;

	/* pick random tile */
	{
		double rndrad, rndang;
		rndrad = prng->Uniform() * (double)rad;
		rndang = prng->Uniform() * M_PI * 2.0;
		x = cx + (int)(rndrad * cos(rndang));
		y = cy + (int)(rndrad * sin(rndang));
		/* wrap around, don't clip, so radii of 2048 from a corner
		 * work properly. */
		while (x < 0) x += 1024;
		while (x > 1023) x -= 1024;
		while (y < 0) y += 1024;
		while (y > 1023) y -= 1024;

		/* ask mapdata to move it to nearest empty tile */
		mapdata->FindEmptyTileNear(arena, &x, &y);
	}

	/* place it randomly within the chosen tile */
	x <<= 4;
	y <<= 4;
	rad = prng->Get32() & 0xff;
	x |= rad / 16;
	y |= rad % 16;

	/* whew, finally place the thing */
	d.x = x; d.y = y;
	PlaceBall(arena, bid, &d);
}


void SetBallCount(Arena *arena, int ballcount)
{
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);

	struct BallData *newbd;
	int oldc, i;

	if (ballcount < 0 || ballcount > 255)
		return;

	LOCK_STATUS(arena);

	oldc = abd->ballcount;

	if (ballcount < oldc)
	{
		/* we have to remove some balls. there is no clean way to do
		 * this (as of now). what we do is "phase" the ball so it can't
		 * be picked up by clients by setting it's last-updated time to
		 * be the highest possible time. then we send it's position to
		 * currently-connected clients. then we forget about the ball
		 * entirely so that updates are never sent again. to new
		 * players, it will look like the ball doesn't exist. */
		/* send it reliably, because clients are never going to see this
		 * ball ever again. */
		for (i = ballcount; i < oldc; i++)
			phase_ball(arena, i, NET_RELIABLE);
	}

	/* do the realloc here so that if we have to phase, we do it before
	 * cutting down the memory, and if we have to grow, we spawn the
	 * balls into new memory. */
	newbd = realloc(abd->balls, ballcount * sizeof(struct BallData));
	if (!newbd && ballcount > 0)
	{
		abd->ballcount = 0;
		abd->balls = NULL;
		logm->Log(L_ERROR, "<balls> realloc failed!");
		UNLOCK_STATUS(arena);
		return;
	}
	abd->ballcount = ballcount;
	abd->balls = newbd;

	if (ballcount > oldc)
	{
		for (i = oldc; i < ballcount; i++)
			SpawnBall(arena, i);
	}

	UNLOCK_STATUS(arena);
}


void PlaceBall(Arena *arena, int bid, struct BallData *newpos)
{
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);

	if (!newpos) return;

	LOCK_STATUS(arena);
	if (bid >= 0 && bid < abd->ballcount)
	{
		abd->balls[bid] = *newpos;
		send_ball_packet(arena, bid, NET_UNRELIABLE);
	}
	UNLOCK_STATUS(arena);

	logm->Log(L_DRIVEL, "<balls> {%s} ball %d is at (%d, %d)",
			arena->name, bid, newpos->x, newpos->y);
}


void EndGame(Arena *arena)
{
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);

	int i, newgame;
	ticks_t now = current_ticks();
	ConfigHandle c = arena->cfg;

	LOCK_STATUS(arena);

	for (i = 0; i < abd->ballcount; i++)
	{
		phase_ball(arena, i, NET_RELIABLE);
		abd->balls[i].state = BALL_WAITING;
		abd->balls[i].carrier = NULL;
	}

	/* cfghelp: Soccer:NewGameDelay, arena, int, def: -3000
	 * How long to wait between games. If this is negative, the actual
	 * delay is random, between zero and the absolute value. Units:
	 * ticks. */
	newgame = cfg->GetInt(c, "Soccer", "NewGameDelay", -3000);
	if (newgame < 0)
		newgame = prng->Number(0, -newgame);

	for (i = 0; i < abd->ballcount; i++)
		abd->balls[i].time = TICK_MAKE(now + newgame);

	UNLOCK_STATUS(arena);

	{
		Ipersist *persist = mm->GetInterface(I_PERSIST, ALLARENAS);
		if (persist)
			persist->EndInterval(NULL, arena, INTERVAL_GAME);
		mm->ReleaseInterface(persist);
	}
}


ArenaBallData * GetBallData(Arena *arena)
{
	LOCK_STATUS(arena);
	return P_ARENA_DATA(arena, abdkey);
}

void ReleaseBallData(Arena *arena)
{
	UNLOCK_STATUS(arena);
}


local void load_ball_settings(Arena *arena, int spawnballs)
{
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);
	MyBallData *pbd = P_ARENA_DATA(arena, pbdkey);

	ConfigHandle c = arena->cfg;
	int bc, i;

	/* cfghelp: Soccer:BallCount, arena, int, def: 0
	 * The number of balls in this arena. */
	bc = cfg->GetInt(c, "Soccer", "BallCount", 0);

	/* and initialize settings for that type */
	if (bc)
	{
		LOCK_STATUS(arena);
		/* cfghelp: Soccer:SpawnX, arena, int, range: 0-1023, def: 512
		 * The X coordinate that the ball spawns at (in tiles). */
		pbd->spawnx = cfg->GetInt(c, "Soccer", "SpawnX", 512);
		/* cfghelp: Soccer:SpawnY, arena, int, range: 0-1023, def: 512
		 * The Y coordinate that the ball spawns at (in tiles). */
		pbd->spawny = cfg->GetInt(c, "Soccer", "SpawnY", 512);
		/* cfghelp: Soccer:SpawnRadius, arena, int, def: 20
		 * How far from the spawn center the ball can spawn (in tiles). */
		pbd->spawnr = cfg->GetInt(c, "Soccer", "SpawnRadius", 20);
		/* cfghelp: Soccer:SendTime, arena, int, range: 100-3000, def: 1000
		 * How often the server sends ball positions (in ticks). */
		pbd->sendtime = cfg->GetInt(c, "Soccer", "SendTime", 1000);
		/* cfghelp: Soccer:GoalDelay, arena, int, def: 0
		 * How long after a goal before the ball appears (in ticks). */
		pbd->goaldelay = cfg->GetInt(c, "Soccer", "GoalDelay", 0);
		/* cfghelp: Soccer:AllowGoalByDeath, arena, bool, def: 0
		 * Whether a goal is scored if a player dies carrying the ball
		 * on a goal tile. */
		pbd->deathgoal = cfg->GetInt(c, "Soccer", "AllowGoalByDeath", 0);

		if (spawnballs)
		{
			pbd->lastsent = current_ticks();
			abd->ballcount = bc;

			/* allocate array for public ball data */
			abd->balls = amalloc(bc * sizeof(struct BallData));

			for (i = 0; i < bc; i++)
				SpawnBall(arena, i);

			logm->LogA(L_INFO, "balls", arena, "arena has %d balls", bc);
		}

		UNLOCK_STATUS(arena);
	}

	/* ball count has changed (settings changed), update pballs */
	if (abd->ballcount != bc)
		SetBallCount(arena, bc);
}


void AABall(Arena *arena, int action)
{
	/* create the mutex if necessary */
	if (action == AA_CREATE)
	{
		pthread_mutexattr_t attr;

		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init((pthread_mutex_t*)P_ARENA_DATA(arena, mtxkey), &attr);
		pthread_mutexattr_destroy(&attr);
	}

	LOCK_STATUS(arena);
	if (action == AA_CREATE || action == AA_DESTROY)
	{
		ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);

		/* clean up old ball data */
		if (abd->balls)
		{
			afree(abd->balls);
			abd->balls = NULL;
		}
		abd->ballcount = 0;
	}
	if (action == AA_CREATE)
	{
		/* only if we're creating, load the data */
		load_ball_settings(arena, 1);
	}
	else if (action == AA_CONFCHANGED)
	{
		/* reload only settings, don't reset balls */
		load_ball_settings(arena, 0);
	}
	UNLOCK_STATUS(arena);
}


local void CleanupAfter(Arena *arena, Player *p, int neut)
{
	/* make sure that if someone leaves, his ball drops */
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);
	int i;
	struct BallData *b = abd->balls;

	LOCK_STATUS(arena);
	for (i = 0; i < abd->ballcount; i++, b++)
		if (b->state == BALL_CARRIED &&
			b->carrier == p)
		{
			b->state = BALL_ONMAP;
			b->x = p->position.x;
			b->y = p->position.y;
			b->xspeed = b->yspeed = 0;
			if (neut) b->carrier = NULL;
			b->time = current_ticks();
			send_ball_packet(arena, i, NET_UNRELIABLE);
			/* don't forget fire callbacks */
			DO_CBS(CB_BALLFIRE, arena, BallFireFunc,
					(arena, p, i));
		}
		else if (neut && b->carrier == p)
		{
			/* if it's on the map, but last touched by the person, reset
			 * it's last touched pid to -1 so that the last touched pid
			 * always refers to a valid player. */
			b->carrier = NULL;
			send_ball_packet(arena, i, NET_UNRELIABLE);
		}
	UNLOCK_STATUS(arena);
}

void PABall(Player *p, int action, Arena *arena)
{
	/* if he's entering arena, the timer event will send him the ball
	 * info. */
	if (action == PA_LEAVEARENA)
		CleanupAfter(arena, p, 1);
}

void ShipChange(Player *p, int ship, int newfreq)
{
	CleanupAfter(p->arena, p, 1);
}

void FreqChange(Player *p, int newfreq)
{
	CleanupAfter(p->arena, p, 1);
}

void BallKill(Arena *arena, Player *killer, Player *killed, int bounty, int flags, int *pts)
{
	MyBallData *pbd = P_ARENA_DATA(arena, pbdkey);
	CleanupAfter(arena, killed, !pbd->deathgoal);
}


void PPickupBall(Player *p, byte *pkt, int len)
{
	Arena *arena = p->arena;
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);

	int i;
	struct BallData *bd;
	struct C2SPickupBall *bp = (struct C2SPickupBall*)pkt;
	
	if (len != sizeof(struct C2SPickupBall))
	{
		logm->Log(L_MALICIOUS, "<balls> [%s] Bad size for ball pickup packet", p->name);
		return;
	}

	if (!arena || p->status != S_PLAYING)
	{
		logm->Log(L_WARN, "<balls> [%s] ball pickup packet from bad arena or status", p->name);
		return;
	}

	if (p->p_ship >= SHIP_SPEC)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "ball pickup packet from spec");
		return;
	}

	/* this player is too lagged to have a ball */
	if (p->flags.no_flags_balls)
	{
		logm->LogP(L_INFO, "balls", p, "too lagged to pick up ball %d", bp->ballid);
		return;
	}

	LOCK_STATUS(arena);

	if (bp->ballid >= abd->ballcount)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "tried to pick up a nonexistent ball");
		UNLOCK_STATUS(arena);
		return;
	}

	bd = abd->balls + bp->ballid;

	/* make sure someone else didn't get it first */
	if (bd->state != BALL_ONMAP)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "tried to pick up a carried ball");
		UNLOCK_STATUS(arena);
		return;
	}

	if (bp->time != bd->time)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "tried to pick up a ball from stale coords");
		UNLOCK_STATUS(arena);
		return;
	}

	/* make sure player doesnt carry more than one ball */
	for (i = 0; i < abd->ballcount; i++)
		if (abd->balls[i].carrier == p &&
		    abd->balls[i].state == BALL_CARRIED)
		{
			UNLOCK_STATUS(arena);
			return;
		}

	bd->state = BALL_CARRIED;
	bd->x = p->position.x;
	bd->y = p->position.y;
	bd->xspeed = 0;
	bd->yspeed = 0;
	bd->carrier = p;
	bd->freq = p->p_freq;
	bd->time = 0;
	send_ball_packet(arena, bp->ballid, NET_UNRELIABLE | BALL_SEND_PRI);

	/* now call callbacks */
	DO_CBS(CB_BALLPICKUP, arena, BallPickupFunc,
			(arena, p, bp->ballid));

	UNLOCK_STATUS(arena);

	logm->Log(L_DRIVEL, "<balls> {%s} [%s] player picked up ball %d",
			arena->name,
			p->name,
			bp->ballid);
}


void PFireBall(Player *p, byte *pkt, int len)
{
	Arena *arena = p->arena;
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);

	struct BallData *bd;
	struct BallPacket *fb = (struct BallPacket *)pkt;
	int bid = fb->ballid;

	if (len != sizeof(struct BallPacket))
	{
		logm->LogP(L_MALICIOUS, "balls", p, "bad size for ball fire packet");
		return;
	}

	if (!arena || p->status != S_PLAYING)
	{
		logm->LogP(L_WARN, "balls", p, "ball fire packet from bad arena or status");
		return;
	}

	if (p->p_ship >= SHIP_SPEC)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "ball fire packet from spec");
		return;
	}

	LOCK_STATUS(arena);

	if (bid < 0 || bid >= abd->ballcount)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "tried to fire up a nonexistent ball");
		UNLOCK_STATUS(arena);
		return;
	}

	bd = abd->balls + bid;

	if (bd->state != BALL_CARRIED || bd->carrier != p)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "player tried to fire ball he wasn't carrying");
		UNLOCK_STATUS(arena);
		return;
	}

	bd->state = BALL_ONMAP;
	bd->x = fb->x;
	bd->y = fb->y;
	bd->xspeed = fb->xspeed;
	bd->yspeed = fb->yspeed;
	bd->freq = p->p_freq;
	bd->time = fb->time;
	send_ball_packet(arena, bid, NET_UNRELIABLE | BALL_SEND_PRI);

	/* finally call callbacks */
	DO_CBS(CB_BALLFIRE, arena, BallFireFunc, (arena, p, bid));

	UNLOCK_STATUS(arena);

	logm->LogP(L_DRIVEL, "balls", p, "player fired ball %d", bid);
}


void PGoal(Player *p, byte *pkt, int len)
{
	Arena *arena = p->arena;
	ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);
	MyBallData *pbd = P_ARENA_DATA(arena, pbdkey);
	int bid;
	struct C2SGoal *g = (struct C2SGoal*)pkt;
	struct BallData *bd;

	if (len != sizeof(struct C2SGoal))
	{
		logm->LogP(L_MALICIOUS, "balls", p, "bad size for goal packet");
		return;
	}

	if (!arena || p->status != S_PLAYING)
	{
		logm->LogP(L_WARN, "balls", p, "goal packet from bad arena or status");
		return;
	}

	LOCK_STATUS(arena);

	bid = g->ballid;

	if (bid < 0 || bid >= abd->ballcount)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "sent a goal for a nonexistent ball");
		UNLOCK_STATUS(arena);
		return;
	}

	bd = abd->balls + bid;

	/* we use this as a flag to check for dupilicated goals */
	if (bd->carrier == NULL)
	{
		UNLOCK_STATUS(arena);
		return;
	}

	if (bd->state != BALL_ONMAP)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "sent goal for carried ball");
		UNLOCK_STATUS(arena);
		return;
	}

	if (p != bd->carrier)
	{
		logm->LogP(L_MALICIOUS, "balls", p, "sent goal for ball he didn't fire");
		UNLOCK_STATUS(arena);
		return;
	}

	/* do callbacks before spawning */
	DO_CBS(CB_GOAL, arena, GoalFunc, (arena, p, g->ballid, g->x, g->y));

	/* send ball update */
	if (bd->state != BALL_ONMAP)
	{
		/* don't respawn ball */
	}
	else if (pbd->goaldelay == 0)
	{
		/* we don't want a delay */
		SpawnBall(arena, bid);
	}
	else
	{
		/* phase it, then set it to waiting */
		phase_ball(arena, bid, NET_UNRELIABLE);
		bd->state = BALL_WAITING;
		bd->carrier = NULL;
		bd->time = TICK_MAKE(current_ticks() + pbd->goaldelay);
	}

	UNLOCK_STATUS(arena);

	logm->LogP(L_DRIVEL, "balls", p, "goal with ball %d", g->ballid);
}


int BasicBallTimer(void *dummy)
{
	Arena *arena;
	Link *link;

	aman->Lock();
	FOR_EACH_ARENA(arena)
	{
		ArenaBallData *abd = P_ARENA_DATA(arena, abdkey);
		MyBallData *pbd = P_ARENA_DATA(arena, pbdkey);

		LOCK_STATUS(arena);
		if (abd->ballcount > 0)
		{
			/* see if we are ready to send packets */
			ticks_t gtc = current_ticks();

			if ( TICK_DIFF(gtc, pbd->lastsent) > pbd->sendtime)
			{
				int bid, bc = abd->ballcount;
				struct BallData *b = abd->balls;

				/* now check the balls up to bc */
				for (bid = 0; bid < bc; bid++, b++)
					if (b->state == BALL_ONMAP)
					{
						/* it's on the map, just send the position
						 * update */
						send_ball_packet(arena, bid, NET_UNRELIABLE);
					}
					else if (b->state == BALL_CARRIED && b->carrier)
					{
						/* it's being carried, update it's x,y coords */
						struct PlayerPosition *pos = &b->carrier->position;
						b->x = pos->x;
						b->y = pos->y;
						send_ball_packet(arena, bid, NET_UNRELIABLE);
					}
					else if (b->state == BALL_WAITING)
					{
						if (TICK_GT(gtc, b->time))
							SpawnBall(arena, bid);
					}
				pbd->lastsent = gtc;
			}
		}
		UNLOCK_STATUS(arena);
	}
	aman->Unlock();

	return TRUE;
}


