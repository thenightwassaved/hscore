
/* dist: public */

#include <string.h>
#include <stdlib.h>

#include "asss.h"


local void LogSysop(const char *);
local void PA(Player *p, int action, Arena *arena);
local void Clastlog(const char *cmd, const char *params, Player *p, const Target *target);
local helptext_t lastlog_help;

enum { SEE_NONE, SEE_ARENA, SEE_ALL };
local int seewhatkey;

/* stuff for lastlog */
#define MAXLAST CFG_LAST_LINES
#define MAXLINE CFG_LAST_LENGTH

/* this is a circular buffer structure */
local int ll_pos;
local char ll_data[MAXLAST][MAXLINE]; /* 12.5k */
local pthread_mutex_t ll_mtx = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_LL() pthread_mutex_lock(&ll_mtx)
#define UNLOCK_LL() pthread_mutex_unlock(&ll_mtx)


local Iplayerdata *pd;
local Iconfig *cfg;
local Ilogman *lm;
local Ichat *chat;
local Icapman *capman;
local Icmdman *cmd;


EXPORT int MM_log_sysop(int action, Imodman *mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		capman = mm->GetInterface(I_CAPMAN, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		if (!cfg || !lm || !chat) return MM_FAIL;

		seewhatkey = pd->AllocatePlayerData(sizeof(int));
		if (seewhatkey == -1) return MM_FAIL;

		memset(ll_data, 0, sizeof(ll_data));
		ll_pos = 0;

		mm->RegCallback(CB_LOGFUNC, LogSysop, ALLARENAS);
		mm->RegCallback(CB_PLAYERACTION, PA, ALLARENAS);

		cmd->AddCommand("lastlog", Clastlog, ALLARENAS, lastlog_help);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		cmd->RemoveCommand("lastlog", Clastlog, ALLARENAS);
		mm->UnregCallback(CB_PLAYERACTION, PA, ALLARENAS);
		mm->UnregCallback(CB_LOGFUNC, LogSysop, ALLARENAS);
		pd->FreePlayerData(seewhatkey);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(capman);
		mm->ReleaseInterface(cmd);
		return MM_OK;
	}
	return MM_FAIL;
}


void LogSysop(const char *s)
{
	if (lm->FilterLog(s, "log_sysop"))
	{
		LinkedList set = LL_INITIALIZER;
		Player *p;
		Link *link;
		int *seewhat;

		pd->Lock();
		FOR_EACH_PLAYER_P(p, seewhat, seewhatkey)
		{
			if (*seewhat == SEE_ALL)
				LLAdd(&set, p);
			else if (*seewhat == SEE_ARENA)
			{
				/* now we have to check if the arena matches. it kinda
				 * sucks that it has to be done this way, but to do it
				 * the "right" way would require undesirable changes. */
				Arena *arena = p->arena;
				if (arena)
				{
					char *carena = arena->name;
					char *t = strchr(s, '{');
					if (t)
					{
						while (*carena && *t && *t == *carena)
							t++, carena++;
						if (*carena == '\0' && *t == '}')
							LLAdd(&set, p);
					}
				}
			}
		}
		pd->Unlock();
		chat->SendAnyMessage(&set, MSG_SYSOPWARNING, 0, NULL, "%s", s);
		LLEmpty(&set);
	}

	if (lm->FilterLog(s, "log_lastlog"))
	{
		LOCK_LL();
		astrncpy(ll_data[ll_pos], s, MAXLINE);
		ll_pos = (ll_pos+1) % MAXLAST;
		UNLOCK_LL();
	}
}


void PA(Player *p, int action, Arena *arena)
{
	int *seewhat = (int*)PPDATA(p, seewhatkey);
	if (action == PA_CONNECT || action == PA_ENTERARENA)
	{
		if (capman->HasCapability(p, CAP_SEESYSOPLOGALL))
			*seewhat = SEE_ALL;
		else if (capman->HasCapability(p, CAP_SEESYSOPLOGARENA))
			*seewhat = SEE_ARENA;
		else
			*seewhat = SEE_NONE;
	}
}


local helptext_t lastlog_help =
"Module: log_sysop\n"
"Targets: none\n"
"Args: [<number of lines>] [<limiting text>]\n"
"Prints out the last <number> lines in the server log (default: 10\n"
"lines). If you specify any text as an argument, besides a number, the\n"
"display will be limited to lines that contain that text. You can specify\n"
"both a number and limiting text, just put the number first.\n";

void Clastlog(const char *cmd, const char *params, Player *p, const Target *target)
{
	int count, c;
	char *end;
	Link link = { NULL, p };
	LinkedList lst = { &link, &link };

	count = strtol(params, &end, 0);
	if (count < 1) count = 10;
	if (count >= MAXLAST) count = MAXLAST-1;

	if (*end)
		while (*end == ' ' || *end == '\t') end++;

	c = (ll_pos - count + MAXLAST) % MAXLAST;

	LOCK_LL();
	while (c != ll_pos)
	{
		if (ll_data[c][0])
			if (*end == '\0' || strstr(ll_data[c], end))
				chat->SendAnyMessage(&lst, MSG_SYSOPWARNING, 0, NULL, "%s", ll_data[c]);
		c = (c+1) % MAXLAST;
	}
	UNLOCK_LL();
}

