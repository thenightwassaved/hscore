
/* dist: public */

#include <string.h>

#include "asss.h"

#include "letters.inc"


local void Cbrickwrite(const char *tc, const char *params, Player *p, const Target *target);

local Iplayerdata *pd;
local Ibricks *bricks;
local Icmdman *cmd;


EXPORT int MM_bricklayer(int action, Imodman *mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		bricks = mm->GetInterface(I_BRICKS, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		cmd->AddCommand("brickwrite", Cbrickwrite, ALLARENAS, NULL);

		if (!bricks) return MM_FAIL;

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		cmd->RemoveCommand("brickwrite", Cbrickwrite, ALLARENAS);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(bricks);
		mm->ReleaseInterface(pd);
		return MM_OK;
	}
	return MM_FAIL;
}


void Cbrickwrite(const char *tc, const char *params, Player *p, const Target *target)
{
	int i, wid;
	Arena *arena = p->arena;
	int freq = p->p_freq;
	int x = p->position.x >> 4;
	int y = p->position.y >> 4;

	wid = 0;
	for (i = 0; i < (int)strlen(params); i++)
		if (params[i] >= ' ' && params[i] <= '~')
			wid += letterdata[(int)params[i] - ' '].width + 1;

	x -= wid / 2;
	y -= letterheight / 2;

	for (i = 0; i < (int)strlen(params); i++)
		if (params[i] >= ' ' && params[i] <= '~')
		{
			int c = params[i] - ' ';
			int bnum = letterdata[c].bricknum;
			struct bl_brick *brk = letterdata[c].bricks;
			for ( ; bnum ; bnum--, brk++)
			{
				bricks->DropBrick(
						arena,
						freq,
						x + brk->x1,
						y + brk->y1,
						x + brk->x2,
						y + brk->y2);
			}
			x += letterdata[c].width + 1;
		}
}

