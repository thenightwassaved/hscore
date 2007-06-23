#include "asss.h"
#include "hscore.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Inet *net;
local Ihscoreitems *items;

local void Pppk(Player *p, byte *p2, int len)
{
	struct C2SPosition *pos = (struct C2SPosition *)p2;

	if (len < 22)
		return;

	/* handle common errors */
	if (!p->arena) return;

	if (p->p_ship == SHIP_SPEC)
		return;

	switch (pos->weapon.type)
	{
		case W_NULL:
			return;
		case W_BULLET:
		case W_BOUNCEBULLET:
			items->triggerEvent(p, p->p_ship, "bullet");
			break;
		case W_BOMB:
		case W_PROXBOMB:
			items->triggerEvent(p, p->p_ship, "bomb");
			break;
		case W_REPEL:
			items->triggerEvent(p, p->p_ship, "repel");
			break;
		case W_DECOY:
			items->triggerEvent(p, p->p_ship, "decoy");
			break;
		case W_BURST:
			items->triggerEvent(p, p->p_ship, "burst");
			break;
		case W_THOR:
			items->triggerEvent(p, p->p_ship, "thor");
			break;
	}
}

EXPORT const char info_hscore_wepevents[] = "v1.0 Dr Brain <drbrain@gmail.com>";

EXPORT int MM_hscore_wepevents(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		net = mm->GetInterface(I_NET, ALLARENAS);
		items = mm->GetInterface(I_HSCORE_ITEMS, ALLARENAS);

		if (!lm || !net || !items)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(net);
			mm->ReleaseInterface(items);

			return MM_FAIL;
		}

		net->AddPacket(C2S_POSITION, Pppk);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		net->RemovePacket(C2S_POSITION, Pppk);

		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(net);
		mm->ReleaseInterface(items);

		return MM_OK;
	}
	return MM_FAIL;
}
