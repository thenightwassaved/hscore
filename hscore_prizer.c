#include <math.h>
#include "asss.h"
#include "hscore.h"

local Imodman *mm;
local Ichat *chat;
local Igame *game;
local Ihscoreitems *items;
local Inet *net;

local void prize(Player *p)
{
	Target *t;
	t->type = T_PLAYER;
	t->u.p = p;

	game->GivePrize(t, 9, items->getPropertySum(p, p->pkt.ship, "bomblevel"));
	game->GivePrize(t, 8, items->getPropertySum(p, p->pkt.ship, "gunlevel"));

	game->GivePrize(t, 21, items->getPropertySum(p, p->pkt.ship, "repel"));
	game->GivePrize(t, 22, items->getPropertySum(p, p->pkt.ship, "burst"));
	game->GivePrize(t, 23, items->getPropertySum(p, p->pkt.ship, "thor"));
	game->GivePrize(t, 24, items->getPropertySum(p, p->pkt.ship, "portal"));
	game->GivePrize(t, 25, items->getPropertySum(p, p->pkt.ship, "decoy"));
	game->GivePrize(t, 26, items->getPropertySum(p, p->pkt.ship, "brick"));
	game->GivePrize(t, 27, items->getPropertySum(p, p->pkt.ship, "rocket"));

	game->GivePrize(t, 6, items->getPropertySum(p, p->pkt.ship, "xradar"));
	game->GivePrize(t, 5, items->getPropertySum(p, p->pkt.ship, "cloak"));
	game->GivePrize(t, 4, items->getPropertySum(p, p->pkt.ship, "stealth"));
	game->GivePrize(t, 20, items->getPropertySum(p, p->pkt.ship, "antiwarp"));

	game->GivePrize(t, 11, items->getPropertySum(p, p->pkt.ship, "thrust"));
	game->GivePrize(t, 12, items->getPropertySum(p, p->pkt.ship, "speed"));
	game->GivePrize(t, 2, items->getPropertySum(p, p->pkt.ship, "energy"));
	game->GivePrize(t, 1, items->getPropertySum(p, p->pkt.ship, "recharge"));
	game->GivePrize(t, 3, items->getPropertySum(p, p->pkt.ship, "rotation"));

	game->GivePrize(t, 10, items->getPropertySum(p, p->pkt.ship, "bounce"));
	game->GivePrize(t, 16, items->getPropertySum(p, p->pkt.ship, "prox"));
	game->GivePrize(t, 19, items->getPropertySum(p, p->pkt.ship, "shrapnel"));
	game->GivePrize(t, 15, items->getPropertySum(p, p->pkt.ship, "multifire"));
}

local void Pppk(Player *p, byte *p2, int len)
{
    struct C2SPosition *pos = (struct C2SPosition *)p2;
    Target t;
    t.type = T_PLAYER;
    t.u.p = p;

    if (p->p_ship == SHIP_SPEC)
        return;

    if (500 < pos->x>>4 && pos->x>>4 < 550 && 500 < pos->y>>4 && pos->y>>4 < 550)
    {
		if (p->position.bounty == 0)
                prize(p);

		int x = (rand() % 250) + 300;
		int y = (rand() % 250) + 300;
        game->WarpTo(&t, x, y);
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

		if(!chat || !game || !items || !net)
			return MM_FAIL;

		return MM_OK;
	}
	else if(action == MM_POSTLOAD)
	{
		net->AddPacket(C2S_POSITION, Pppk);
		return MM_OK;
	}
	else if(action == MM_ATTACH)
	{
		return MM_OK;
	}
	else if(action == MM_DETACH)
	{
		return MM_OK;
	}
	else if(action == MM_PREUNLOAD)
	{
		net->RemovePacket(C2S_POSITION, Pppk);
		return MM_OK;
	}
	else if(action == MM_UNLOAD)
	{
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(game);
		mm->ReleaseInterface(items);
		mm->ReleaseInterface(net);

		return MM_OK;
	}
	return MM_FAIL;
}
