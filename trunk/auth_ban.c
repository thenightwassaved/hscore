
/* dist: public */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asss.h"


typedef struct
{
	TreapHead head;
	int count;
	time_t expire;
} ban_node_t;

local Imodman *mm;
local Iauth *oldauth;
local Icapman *capman;
local Ichat *chat;
local Icmdman *cmd;
local Iplayerdata *pd;
local Ilogman *lm;

local TreapHead *banroot = NULL;
local pthread_mutex_t banmtx = PTHREAD_MUTEX_INITIALIZER;


local void Authenticate(Player *p, struct LoginPacket *lp, int lplen,
		void (*Done)(Player *p, AuthData *data))
{
	ban_node_t *bn;

	pthread_mutex_lock(&banmtx);
	bn = (ban_node_t*)TrGet(banroot, (int)lp->macid);
	if (IS_STANDARD(p) && bn)
	{
		time_t now = time(NULL);
		if (now < bn->expire)
		{
			AuthData data = { 0, AUTH_LOCKEDOUT, 0 };
			bn->count++;
			if (lm) lm->Log(L_INFO, "<auth_ban> player [%s] tried to log in"
					" (try %d), banned for %ld more minutes",
					lp->name, bn->count, (bn->expire - now + 59) / 60);
			pthread_mutex_unlock(&banmtx);
			Done(p, &data);
			return;
		}
		else
			/* expired, remove and continue normally */
			TrRemove(&banroot, (int)lp->macid);
	}
	pthread_mutex_unlock(&banmtx);

	oldauth->Authenticate(p, lp, lplen, Done);
}


local helptext_t kick_help =
"Module: auth_ban\n"
"Targets: player\n"
"Args: [<timeout>]\n"
"Kicks the player off of the server, with an optional timeout (in minutes).\n";

local void Ckick(const char *tc, const char *params, Player *p, const Target *target)
{
	Player *t = target->u.p;

	if (target->type != T_PLAYER)
	{
		chat->SendMessage(p, "Only valid target is a single player");
		return;
	}

	if (t == p)
		return;

	if (!capman->HigherThan(p, t))
	{
		chat->SendMessage(p, "You don't have permission to use ?kick on that player.");
		chat->SendMessage(t, "%s tried to use ?kick on you.", p->name);
		return;
	}

	/* try timeout stuff while the player still exists */
	if (IS_STANDARD(t))
	{
		int mins = strtol(params, NULL, 0);
		if (mins)
		{
			ban_node_t *bn = amalloc(sizeof(*bn));
			bn->head.key = (int)t->macid;
			bn->count = 0;
			bn->expire = time(NULL) + mins * 60;
			pthread_mutex_lock(&banmtx);
			TrPut(&banroot, (TreapHead*)bn);
			pthread_mutex_unlock(&banmtx);
		}
	}

	pd->KickPlayer(t);
}


#define MAXDATA 4090

local void add_mid_ban(TreapHead *node, void *clos)
{
	ban_node_t *ban = (ban_node_t*)node;
	char *start = clos, *p;
	int l = strlen(start);
	p = start + l;
	if (ban->expire)
		snprintf(p, MAXDATA - l, ", %u (%ld min. left)",
				(unsigned)ban->head.key, (ban->expire - time(NULL) + 30) / 60);
	else
		snprintf(p, MAXDATA - l, ", %u",
				(unsigned)ban->head.key);
}

local void send_msg_cb(const char *line, void *clos)
{
	chat->SendMessage((Player*)clos, "  %s", line);
}

local helptext_t listmidbans_help =
"Module: auth_ban\n"
"Targets: none\n"
"Args: none\n"
"Lists the current machine id bans in effect.\n";

local void Clistmidbans(const char *tc, const char *params, Player *p, const Target *target)
{
	char data[MAXDATA+6];
	pthread_mutex_lock(&banmtx);
	TrEnum(banroot, add_mid_ban, data);
	pthread_mutex_unlock(&banmtx);
	chat->SendMessage(p, "Active machine id bans:");
	wrap_text(data+2, 80, ' ', send_msg_cb, p);
}


local helptext_t delmidban_help =
"Module: auth_ban\n"
"Targets: none\n"
"Args: <machine id>\n"
"Removes a machine id ban.\n";

local void Cdelmidban(const char *tc, const char *params, Player *p, const Target *target)
{
	int mid = (int)strtoul(params, NULL, 0);
	if (mid == 0)
		chat->SendMessage(p, "Invalid machine id.");
	else
	{
		pthread_mutex_lock(&banmtx);
		TrRemove(&banroot, mid);
		pthread_mutex_unlock(&banmtx);
	}
}


local Iauth myauth =
{
	INTERFACE_HEAD_INIT(I_AUTH, "auth-ban")
	Authenticate
};


EXPORT int MM_auth_ban(int action, Imodman *mm_, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = mm_;
		oldauth = mm->GetInterface(I_AUTH, ALLARENAS);
		capman = mm->GetInterface(I_CAPMAN, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);

		if (!oldauth || !capman || !cmd || !chat || !pd)
			return MM_FAIL;

		cmd->AddCommand("kick", Ckick, ALLARENAS, kick_help);
		cmd->AddCommand("listmidbans", Clistmidbans, ALLARENAS, listmidbans_help);
		cmd->AddCommand("delmidban", Cdelmidban, ALLARENAS, delmidban_help);

		mm->RegInterface(&myauth, ALLARENAS);
		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&myauth, ALLARENAS))
			return MM_FAIL;
		cmd->RemoveCommand("kick", Ckick, ALLARENAS);
		cmd->RemoveCommand("listmidbans", Clistmidbans, ALLARENAS);
		cmd->RemoveCommand("delmidban", Cdelmidban, ALLARENAS);
		TrEnum(banroot, tr_enum_afree, NULL);
		mm->ReleaseInterface(oldauth);
		mm->ReleaseInterface(capman);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(lm);
		return MM_OK;
	}
	return MM_FAIL;
}

