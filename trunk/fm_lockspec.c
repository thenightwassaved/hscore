
/* dist: public */

#include "asss.h"

local void fm(Player *p, int *ship, int *freq)
{
	*ship = SHIP_SPEC; *freq = CFG_DEF_SPEC_FREQ;
}

local Ifreqman fm_int =
{
	INTERFACE_HEAD_INIT(I_FREQMAN, "fm-lockspec")
	fm, fm, fm
};

EXPORT int MM_fm_lockspec(int action, Imodman *mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		mm->RegInterface(&fm_int, arena);
		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		mm->UnregInterface(&fm_int, arena);
		return MM_OK;
	}
	return MM_FAIL;
}

