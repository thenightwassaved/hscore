#include "asss.h"
#include "hscore.h"
#include "hscore_database.h"

//modules
local Imodman *mm;
local Ilogman *lm;
local Ichat *chat;
local Iconfig *cfg;
local Icmdman *cmd;
local Ihscoredatabase *database;

local helptext_t shipsHelp =
"Targets: none or player\n"
"Args: none\n"
"Displays what ships you own in this arena.\n";

local void shipsCommand(const char *command, const char *params, Player *p, const Target *target)
{

}

local helptext_t shipStatusHelp =
"Targets: none or player\n"
"Args: [ship number]\n"
"Displays the specified ship's inventory.\n"
"If no ship is specified, your current ship is assumed.\n";

local void shipStatusCommand(const char *command, const char *params, Player *p, const Target *target)
{
	//note, watch out for speccers not passing in a ship number. Could easily cause an array bounds error
}

EXPORT int MM_hscore_commands(int action, Imodman *_mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = _mm;

		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		database = mm->GetInterface(I_HSCORE_DATABASE, ALLARENAS);

		if (!lm || !chat || !cfg || !cmd || !database)
		{
			mm->ReleaseInterface(lm);
			mm->ReleaseInterface(chat);
			mm->ReleaseInterface(cfg);
			mm->ReleaseInterface(cmd);
			mm->ReleaseInterface(database);

			return MM_FAIL;
		}

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(database);

		return MM_OK;
	}
	else if (action == MM_ATTACH)
	{
		cmd->AddCommand("ships", shipsCommand, arena, shipsHelp);
		cmd->AddCommand("shipstatus", shipStatusCommand, arena, shipStatusHelp);

		return MM_OK;
	}
	else if (action == MM_DETACH)
	{
		cmd->RemoveCommand("ships", shipsCommand, arena);
		cmd->RemoveCommand("shipstatus", shipStatusCommand, arena);

		return MM_OK;
	}
	return MM_FAIL;
}

