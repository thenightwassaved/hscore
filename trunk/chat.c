
/* dist: public */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "asss.h"
#include "persist.h"

#include "packets/chat.h"


#define KEY_CHAT 47


local void SendMessage_(Player *p, const char *str, ...);


/* global data */

local Iplayerdata *pd;
local Inet *net;
local Ichatnet *chatnet;
local Iconfig *cfg;
local Ilogman *lm;
local Icmdman *cmd;
local Imodman *mm;
local Iarenaman *aman;
local Icapman *capman;
local Ipersist *persist;

struct player_mask_t
{
	chat_mask_t mask;
	/* this is zero for a session-long mask, otherwise a time() value */
	time_t expires;
	/* a count of messages. this decays exponentially 50% per second */
	int msgs;
	ticks_t lastcheck;
};

local int cfg_msgrel, cfg_floodlimit, cfg_floodshutup, cfg_cmdlimit;
local int cmkey, pmkey;


/* call with player lock */
local void expire_mask(Player *p)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);
	int d;
	ticks_t now = current_ticks();

	/* handle expiring masks */
	if (pm->expires > 0)
		if (time(NULL) > pm->expires)
		{
			pm->mask = 0;
			pm->expires = 0;
		}

	/* handle exponential decay of msg count */
	d = TICK_DIFF(now, pm->lastcheck) / 100;
	pm->msgs >>= d;
	pm->lastcheck = now;
}

/* call with player lock */
local void check_flood(Player *p)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);

	pm->msgs++;

	if (pm->msgs >= cfg_floodlimit &&
	    cfg_floodlimit > 0 &&
	    !capman->HasCapability(p, CAP_CANSPAM))
	{
		pm->msgs >>= 1;
		pm->mask |=
			1 << MSG_PUBMACRO |
			1 << MSG_PUB |
			1 << MSG_FREQ |
			1 << MSG_NMEFREQ |
			1 << MSG_PRIV |
			1 << MSG_REMOTEPRIV |
			1 << MSG_CHAT |
			1 << MSG_MODCHAT |
			1 << MSG_BCOMMAND;
		if (pm->expires)
			/* already has a mask, add time */
			pm->expires += cfg_floodshutup;
		else
			pm->expires = time(NULL) + cfg_floodshutup;
		SendMessage_(p, "You have been shut up for %d seconds for flooding.", cfg_floodshutup);
		lm->LogP(L_INFO, "chat", p, "flooded chat, shut up for %d seconds", cfg_floodshutup);
	}
}


/* message sending funcs */

local const char *get_chat_type(int type)
{
	switch (type)
	{
		case MSG_ARENA: return "ARENA";
		case MSG_PUB: return "PUB";
		case MSG_PUBMACRO: return "PUBM";
		case MSG_PRIV: return "PRIV";
		case MSG_FREQ: return "FREQ";
		case MSG_SYSOPWARNING: return "SYSOP";
		case MSG_CHAT: return "CHAT";
		case MSG_MODCHAT: return "MOD";
		case MSG_REMOTEPRIV: return "PRIV";
		default: return NULL;
	}
}

local void v_send_msg(LinkedList *set, char type, char sound, Player *from, const char *str, va_list ap)
{
	int size;
	char _buf[256];
	const char *ctype = get_chat_type(type);
	struct ChatPacket *cp = (struct ChatPacket*)_buf;

	size = vsnprintf(cp->text, 250, str, ap) + 6;

	if (type == MSG_MODCHAT)
		type = MSG_SYSOPWARNING;

	cp->pktype = S2C_CHAT;
	cp->type = type;
	cp->sound = sound;
	cp->pid = from ? from->pid : -1;
	if (net) net->SendToSet(set, (byte*)cp, size, NET_RELIABLE);
	if (chatnet && ctype)
	{
		if (from)
			chatnet->SendToSet(set, "MSG:%s:%s:%s", ctype, from->name, cp->text);
		else
			chatnet->SendToSet(set, "MSG:%s:%s", ctype, cp->text);
	}
}

local void SendMessage_(Player *p, const char *str, ...)
{
	Link l = { NULL, p };
	LinkedList set = { &l, &l };
	va_list args;
	va_start(args, str);
	v_send_msg(&set, MSG_ARENA, 0, NULL, str, args);
	va_end(args);
}

local void SendSetMessage(LinkedList *set, const char *str, ...)
{
	va_list args;
	va_start(args, str);
	v_send_msg(set, MSG_ARENA, 0, NULL, str, args);
	va_end(args);
}

local void SendSoundMessage(Player *p, char sound, const char *str, ...)
{
	Link l = { NULL, p };
	LinkedList set = { &l, &l };
	va_list args;
	va_start(args, str);
	v_send_msg(&set, MSG_ARENA, sound, NULL, str, args);
	va_end(args);
}

local void SendSetSoundMessage(LinkedList *set, char sound, const char *str, ...)
{
	va_list args;
	va_start(args, str);
	v_send_msg(set, MSG_ARENA, sound, NULL, str, args);
	va_end(args);
}

local void get_arena_set(LinkedList *set, Arena *arena)
{
	Link *link;
	Player *p;
	pd->Lock();
	FOR_EACH_PLAYER(p)
		if (p->status == S_PLAYING &&
		    (arena == ALLARENAS || p->arena == arena))
			LLAdd(set, p);
	pd->Unlock();
}

local void get_cap_set(LinkedList *set, const char *cap, Player *except)
{
	Link *link;
	Player *p;
	pd->Lock();
	FOR_EACH_PLAYER(p)
		if (p->status == S_PLAYING &&
		    capman->HasCapability(p, cap) &&
		    p != except)
			LLAdd(set, p);
	pd->Unlock();
}

local void SendArenaMessage(Arena *arena, const char *str, ...)
{
	LinkedList set = LL_INITIALIZER;
	va_list args;

	get_arena_set(&set, arena);

	va_start(args, str);
	v_send_msg(&set, MSG_ARENA, 0, NULL, str, args);
	va_end(args);

	LLEmpty(&set);
}

local void SendArenaSoundMessage(Arena *arena, char sound, const char *str, ...)
{
	LinkedList set = LL_INITIALIZER;
	va_list args;

	get_arena_set(&set, arena);

	va_start(args, str);
	v_send_msg(&set, MSG_ARENA, sound, NULL, str, args);
	va_end(args);

	LLEmpty(&set);
}

local void SendAnyMessage(LinkedList *set, char type, char sound, Player *from, const char *str, ...)
{
	va_list args;
	va_start(args, str);
	v_send_msg(set, type, sound, from, str, args);
	va_end(args);
}

local void SendModMessage(const char *fmt, ...)
{
	LinkedList set = LL_INITIALIZER;
	va_list args;
	get_cap_set(&set, CAP_MODCHAT, NULL);
	va_start(args, fmt);
	v_send_msg(&set, MSG_ARENA, 0, NULL, fmt, args);
	va_end(args);
	LLEmpty(&set);
}

local void SendRemotePrivMessage(LinkedList *set, int sound,
		const char *squad, const char *sender, const char *msg)
{
	int size;
	char _buf[256];
	struct ChatPacket *cp = (struct ChatPacket*)_buf;

	cp->pktype = S2C_CHAT;
	cp->type = MSG_REMOTEPRIV;
	cp->sound = sound;
	cp->pid = -1;

	if (squad)
	{
		size = snprintf(cp->text, 250, "(#%s)(%s)>%s", squad, sender, msg) + 6;
		if (net) net->SendToSet(set, (byte*)cp, size, NET_RELIABLE);
		if (chatnet)
			chatnet->SendToSet(set, "MSG:SQUAD:%s:%s:%s", squad, sender, msg);
	}
	else
	{
		size = snprintf(cp->text, 250, "(%s)>%s", sender, msg) + 6;
		if (net) net->SendToSet(set, (byte*)cp, size, NET_RELIABLE);
		if (chatnet)
			chatnet->SendToSet(set, "MSG:PRIV:%s:%s", sender, msg);
	}
}


/* incoming chat handling functions */

#define CMD_CHAR_1 '?'
#define CMD_CHAR_2 '*'
#define MOD_CHAT_CHAR '\\'

#define OK(type) IS_ALLOWED(pm->mask | *am, type)


local void run_commands(const char *text, Player *p, Target *target)
{
	char buf[512], *b;
	int count = 0;

	if (!cmd) return;

	for (;;)
	{
		/* skip over *, ?, and | */
		while (*text == CMD_CHAR_1 || *text == CMD_CHAR_2 || *text == '|')
			text++;

		if (*text == '\0' || count++ >= cfg_cmdlimit)
			break;

		b = buf;
		while (*text && *text != '|' && (b-buf) < sizeof(buf))
			*b++ = *text++;
		*b = '\0';
		cmd->Command(buf, p, target);
	}
}


local void handle_pub(Player *p, const char *msg, int ismacro, int isallcmd, int sound)
{
	Arena *arena = p->arena;
	chat_mask_t *am = P_ARENA_DATA(arena, cmkey);
	struct player_mask_t *pm = PPDATA(p, pmkey);
	struct ChatPacket *to = alloca(strlen(msg) + 40);

	if (((msg[0] == CMD_CHAR_1 || msg[0] == CMD_CHAR_2) && msg[1] != '\0') ||
	    isallcmd)
	{
		if (OK(MSG_COMMAND))
		{
			Target target;
			target.type = T_ARENA;
			target.u.arena = p->arena;
			run_commands(msg, p, &target);
		}
	}
	else if (OK(ismacro ? MSG_PUBMACRO : MSG_PUB))
	{
		to->pktype = S2C_CHAT;
		to->type = ismacro ? MSG_PUBMACRO : MSG_PUB;
		to->sound = sound;
		to->pid = p->pid;
		strcpy(to->text, msg);

		if (net)
			net->SendToArena(arena, p, (byte*)to, strlen(msg)+6,
					ismacro ? cfg_msgrel | NET_PRI_N1 : cfg_msgrel);
		if (chatnet)
			chatnet->SendToArena(arena, p, "MSG:PUB:%s:%s", p->name, msg);

		DO_CBS(CB_CHATMSG, arena, ChatMsgFunc, (p, to->type, sound, NULL, -1, msg));

		lm->LogP(L_DRIVEL, "chat", p, "pub msg: %s", msg);
	}
}


local void handle_modchat(Player *p, const char *msg, int sound)
{
	Arena *arena = p->arena;
	chat_mask_t *am = P_ARENA_DATA(arena, cmkey);
	struct player_mask_t *pm = PPDATA(p, pmkey);
	LinkedList set = LL_INITIALIZER;
	struct ChatPacket *to = alloca(strlen(msg) + 40);

	to->pktype = S2C_CHAT;
	to->type = MSG_SYSOPWARNING;
	to->sound = sound;
	to->pid = p->pid;
	sprintf(to->text, "%s> %s", p->name, msg);

	if (capman)
	{
		if (capman->HasCapability(p, CAP_SENDMODCHAT) && OK(MSG_MODCHAT))
		{
			get_cap_set(&set, CAP_MODCHAT, p);
			if (net) net->SendToSet(&set, (byte*)to, strlen(to->text)+6, NET_RELIABLE);
			if (chatnet) chatnet->SendToSet(&set, "MSG:MOD:%s:%s",
					p->name, msg);
			LLEmpty(&set);
			DO_CBS(CB_CHATMSG, arena, ChatMsgFunc, (p, MSG_MODCHAT, sound, NULL, -1, msg));
			lm->LogP(L_DRIVEL, "chat", p, "mod chat: %s", msg);
		}
		else
		{
			SendMessage_(p, "You aren't allowed to use the staff chat. "
					"If you need to send a message to the zone staff, use ?cheater.");
			lm->LogP(L_DRIVEL, "chat", p, "attempted mod chat "
					"(missing cap or shutup): %s", msg);
		}
	}
	else
		SendMessage_(p, "Staff chat is currently disabled");
}


local void handle_freq(Player *p, int freq, const char *msg, int sound)
{
	Arena *arena = p->arena;
	chat_mask_t *am = P_ARENA_DATA(arena, cmkey);
	struct player_mask_t *pm = PPDATA(p, pmkey);
	struct ChatPacket *to = alloca(strlen(msg) + 40);

	if ((msg[0] == CMD_CHAR_1 || msg[0] == CMD_CHAR_2) && msg[1] != '\0')
	{
		if (OK(MSG_COMMAND))
		{
			Target target;
			target.type = T_FREQ;
			target.u.freq.arena = p->arena;
			target.u.freq.freq = freq;
			run_commands(msg, p, &target);
		}
	}
	else if (OK(p->p_freq == freq ? MSG_FREQ : MSG_NMEFREQ))
	{
		LinkedList set = LL_INITIALIZER;
		Link *link;
		Player *i;

		to->pktype = S2C_CHAT;
		to->type = p->p_freq == freq ? MSG_FREQ : MSG_NMEFREQ;
		to->sound = sound;
		to->pid = p->pid;
		strcpy(to->text, msg);

		pd->Lock();
		FOR_EACH_PLAYER(i)
			if (i->p_freq == freq &&
				i->arena == arena &&
				i != p)
				LLAdd(&set, i);
		pd->Unlock();

		if (net) net->SendToSet(&set, (byte*)to, strlen(to->text)+6, cfg_msgrel);
		if (chatnet) chatnet->SendToSet(&set, "MSG:FREQ:%s:%s",
				p->name,
				msg);
		DO_CBS(CB_CHATMSG, arena, ChatMsgFunc, (p, MSG_FREQ, sound, NULL, freq, msg));
		lm->LogP(L_DRIVEL, "chat", p, "freq msg (%d): %s", freq, msg);
	}
}


local void handle_priv(Player *p, Player *dst, const char *msg, int isallcmd, int sound)
{
	Arena *arena = p->arena;
	chat_mask_t *am = P_ARENA_DATA(arena, cmkey);
	struct player_mask_t *pm = PPDATA(p, pmkey);

	if (((msg[0] == CMD_CHAR_1 || msg[0] == CMD_CHAR_2) && msg[1] != '\0') ||
	    isallcmd)
	{
		if (OK(MSG_COMMAND))
		{
			Target target;
			target.type = T_PLAYER;
			target.u.p = dst;
			run_commands(msg, p, &target);
		}
	}
	else if (OK(MSG_PRIV))
	{
		if (IS_STANDARD(dst))
		{
			struct ChatPacket *to = alloca(strlen(msg) + 40);
			to->pktype = S2C_CHAT;
			to->type = MSG_PRIV;
			to->sound = sound;
			to->pid = p->pid;
			strcpy(to->text, msg);
			net->SendToOne(dst, (byte*)to, strlen(to->text)+6, NET_RELIABLE);
		}
		else if (IS_CHAT(dst))
			chatnet->SendToOne(dst, "MSG:PRIV:%s:%s",
					p->name, msg);
		DO_CBS(CB_CHATMSG, arena, ChatMsgFunc, (p, MSG_PRIV, sound, dst, -1, msg));
#ifdef CFG_LOG_PRIVATE
		lm->LogP(L_DRIVEL, "chat", p, "to [%s] priv msg: %s",
				dst->name, msg);
#endif
	}
}


local void handle_remote_priv(Player *p, const char *msg, int isallcmd, int sound)
{
	Arena *arena = p->arena;
	chat_mask_t *am = P_ARENA_DATA(arena, cmkey);
	struct player_mask_t *pm = PPDATA(p, pmkey);
	const char *t;
	char dest[24];

	t = delimcpy(dest, msg+1, 21, ':');
	if (msg[0] != ':' || !t)
		lm->LogP(L_MALICIOUS, "chat", p, "malformed remote private message");
	else if (((t[0] == CMD_CHAR_1 || t[0] == CMD_CHAR_2) && t[1] != '\0') ||
	         isallcmd)
	{
		if (OK(MSG_COMMAND))
		{
			Player *d = pd->FindPlayer(dest);
			if (d && d->status == S_PLAYING)
			{
				Target target;
				target.type = T_PLAYER;
				target.u.p = d;
				run_commands(t, p, &target);
			}
		}
	}
	else if (OK(MSG_REMOTEPRIV))
	{
		Player *d = pd->FindPlayer(dest);
		if (d)
		{
			if (d->status != S_PLAYING)
				return;
			if (IS_STANDARD(d))
			{
				struct ChatPacket *to = alloca(strlen(msg) + 40);
				to->pktype = S2C_CHAT;
				to->type = MSG_REMOTEPRIV;
				to->sound = sound;
				to->pid = -1;
				snprintf(to->text, strlen(msg)+30, "(%s)>%s", p->name, t);
				net->SendToOne(d, (byte*)to, strlen(to->text)+6, NET_RELIABLE);
			}
			else if (IS_CHAT(d))
				chatnet->SendToOne(d, "MSG:PRIV:%s:%s", p->name, t);
		}

		/* the billing module will catch these if dest is null. also: we
		 * overload the meaning of the text field: if dest is non-null,
		 * text is the text of the message. if it is null, text is
		 * ":target:msg". */
		DO_CBS(CB_CHATMSG, arena, ChatMsgFunc, (p, MSG_REMOTEPRIV, sound, d, -1, d ? t : msg));

#ifdef CFG_LOG_PRIVATE
		lm->LogP(L_DRIVEL, "chat", p, "to [%s] remote priv: %s", dest, t);
#endif
	}
}


local void handle_chat(Player *p, const char *msg, int sound)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);

	if (IS_ALLOWED(pm->mask, MSG_CHAT))
	{
		/* msg should look like "text" or "#;text" */
		DO_CBS(CB_CHATMSG, ALLARENAS, ChatMsgFunc, (p, MSG_CHAT, sound, NULL, -1, msg));
#ifdef CFG_LOG_PRIVATE
		lm->LogP(L_DRIVEL, "chat", p, "chat msg: %s", msg);
#endif
	}
}



local void PChat(Player *p, byte *pkt, int len)
{
	struct ChatPacket *from = (struct ChatPacket *)pkt;
	Arena *arena = p->arena;
	Player *targ;
	int freq = p->p_freq, sound = 0;
	unsigned char *t;

	if (len < 6 || len > 500)
	{
		lm->LogP(L_MALICIOUS, "chat", p, "bad chat packet len=%i", len);
		return;
	}

	/* clients such as continuum may send unterminated strings */
	pkt[len-1] = 0;

	if (!arena || p->status != S_PLAYING) return;

	expire_mask(p);

	/* remove control characters from the chat message */
	for (t = from->text; *t; t++)
		if (*t < 32 || *t == 127 || *t == 255)
			*t = '_';

	if (capman->HasCapability(p, CAP_SOUNDMESSAGES))
		sound = from->sound;

	switch (from->type)
	{
		case MSG_ARENA:
			lm->LogP(L_MALICIOUS, "chat", p, "recieved arena message");
			break;

		case MSG_PUBMACRO:
			/* fall through */
		case MSG_PUB:
			if (from->text[0] == MOD_CHAT_CHAR)
				handle_modchat(p, from->text + 1, sound);
			else
				handle_pub(p, from->text, from->type == MSG_PUBMACRO, FALSE, sound);
			break;

		case MSG_NMEFREQ:
			targ = pd->PidToPlayer(from->pid);
			if (!targ) break;

			if (targ->arena == arena)
				handle_freq(p, targ->p_freq, from->text, sound);
			else
				lm->LogP(L_MALICIOUS, "chat", p, "cross-arena nmefreq chat message");
			break;

		case MSG_FREQ:
			handle_freq(p, freq, from->text, sound);
			break;

		case MSG_PRIV:
			targ = pd->PidToPlayer(from->pid);
			if (!targ) break;

			if (targ->arena == arena)
				handle_priv(p, targ, from->text, FALSE, sound);
			else
				lm->LogP(L_MALICIOUS, "chat", p, "cross-arena private chat message");
			break;

		case MSG_REMOTEPRIV:
			handle_remote_priv(p, from->text, FALSE, sound);
			break;

		case MSG_SYSOPWARNING:
			lm->LogP(L_MALICIOUS, "chat", p, "recieved sysop message");
			break;

		case MSG_CHAT:
			handle_chat(p, from->text, sound);
			break;

		default:
			lm->LogP(L_MALICIOUS, "chat", p, "recieved undefined type %d chat message", from->type);
			break;
	}

	check_flood(p);
}


local void MChat(Player *p, const char *line)
{
	const char *t;
	char subtype[10], data[24];
	Player *i;

	if (!p->arena || p->status != S_PLAYING || strlen(line) > 500) return;

	expire_mask(p);

	t = delimcpy(subtype, line, sizeof(subtype), ':');
	if (!t) return;

	if (!strncasecmp(subtype, "PUB", 3) || !strcasecmp(subtype, "CMD"))
		handle_pub(p, t, toupper(subtype[3]) == 'M', toupper(subtype[0]) == 'C', 0);
	else if (!strcasecmp(subtype, "PRIV") || !strcasecmp(subtype, "PRIVCMD"))
	{
		t = delimcpy(data, t, sizeof(data), ':');
		if (!t) return;
		pd->Lock();
		i = pd->FindPlayer(data);
		if (i && i->arena == p->arena && i->status == S_PLAYING)
		{
			pd->Unlock();
			handle_priv(p, i, t, toupper(subtype[4]) == 'C', 0);
		}
		else
		{
			/* this is a little hacky: we want to pass in the colon
			 * right after PRIV or PRIVCMD because that's what
			 * handle_remote_priv expects. we know it's the first colon
			 * in the line, so use strchr to find it. */
			pd->Unlock();
			handle_remote_priv(p, strchr(line, ':'), toupper(subtype[4]) == 'C', 0);
		}
	}
	else if (!strcasecmp(subtype, "FREQ"))
	{
		int f;
		t = delimcpy(data, t, sizeof(data), ':');
		if (!t) return;
		f = atoi(data);
		handle_freq(p, f, t, 0);
	}
	else if (!strcasecmp(subtype, "CHAT"))
		handle_chat(p, t, 0);
	else if (!strcasecmp(subtype, "MOD"))
		handle_modchat(p, t, 0);

	check_flood(p);
}




/* chat mask stuff */

local chat_mask_t GetArenaChatMask(Arena *arena)
{
	chat_mask_t *cmp = P_ARENA_DATA(arena, cmkey);
	return arena ? *cmp : 0;
}

local void SetArenaChatMask(Arena *arena, chat_mask_t mask)
{
	chat_mask_t *cmp = P_ARENA_DATA(arena, cmkey);
	if (arena) *cmp = mask;
}

local chat_mask_t GetPlayerChatMask(Player *p)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);
	chat_mask_t ret = 0;

	if (p)
	{
		pd->LockPlayer(p);
		expire_mask(p);
		ret = pm->mask;
		pd->UnlockPlayer(p);
	}

	return ret;
}

local void SetPlayerChatMask(Player *p, chat_mask_t mask, int timeout)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);
	if (p)
	{
		pd->LockPlayer(p);
		pm->mask = mask;
		pm->expires = (timeout == 0) ? 0 : time(NULL) + timeout;
		pd->UnlockPlayer(p);
	}
}


local void aaction(Arena *arena, int action)
{
	if (action == AA_CREATE || action == AA_CONFCHANGED)
	{
		chat_mask_t *cmp = P_ARENA_DATA(arena, cmkey);
		/* cfghelp: Chat:RestrictChat, arena, int, def: 0
		 * This specifies an initial chat mask for the arena. Don't use
		 * this unless you know what you're doing. */
		*cmp = cfg->GetInt(arena->cfg, "Chat", "RestrictChat", 0);
	}
}


local void clear_data(Player *p, void *v)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);
	pd->LockPlayer(p);
	pm->mask = 0;
	pm->expires = 0;
	pd->UnlockPlayer(p);
}

local int get_data(Player *p, void *data, int len, void *v)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);
	int ret = 0;

	pd->LockPlayer(p);
	expire_mask(p);
	if (pm->expires)
	{
		memcpy(data, pm, sizeof(*pm));
		ret = sizeof(*pm);
	}
	pd->UnlockPlayer(p);

	return ret;
}

local void set_data(Player *p, void *data, int len, void *v)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);
	pd->LockPlayer(p);
	if (len == sizeof(*pm))
		memcpy(pm, data, sizeof(*pm));
	pd->UnlockPlayer(p);
}

local PlayerPersistentData pdata =
{
	KEY_CHAT, INTERVAL_FOREVER_NONSHARED, PERSIST_ALLARENAS,
	get_data, set_data, clear_data
};


local void paction(Player *p, int action, Arena *arena)
{
	struct player_mask_t *pm = PPDATA(p, pmkey);
	if (action == PA_PREENTERARENA)
	{
		pm->mask = pm->expires = pm->msgs = 0;
		pm->lastcheck = current_ticks();
	}
}



local Ichat _int =
{
	INTERFACE_HEAD_INIT(I_CHAT, "chat")
	SendMessage_, SendMessage_, SendSetMessage,
	SendSoundMessage, SendSetSoundMessage,
	SendAnyMessage, SendArenaMessage,
	SendArenaSoundMessage, SendModMessage,
	SendRemotePrivMessage,
	GetArenaChatMask, SetArenaChatMask,
	GetPlayerChatMask, SetPlayerChatMask
};


EXPORT int MM_chat(int action, Imodman *mm_, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = mm_;
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		net = mm->GetInterface(I_NET, ALLARENAS);
		chatnet = mm->GetInterface(I_CHATNET, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		capman = mm->GetInterface(I_CAPMAN, ALLARENAS);
		persist = mm->GetInterface(I_PERSIST, ALLARENAS);
		if (!cfg || !aman || !pd || !lm) return MM_FAIL;

		cmkey = aman->AllocateArenaData(sizeof(chat_mask_t));
		pmkey = pd->AllocatePlayerData(sizeof(struct player_mask_t));
		if (cmkey == -1 || pmkey == -1) return MM_FAIL;

		if (persist)
			persist->RegPlayerPD(&pdata);

		mm->RegCallback(CB_ARENAACTION, aaction, ALLARENAS);
		mm->RegCallback(CB_PLAYERACTION, paction, ALLARENAS);

		/* cfghelp: Chat:MessageReliable, global, bool, def: 1
		 * Whether to send chat messages reliably. */
		cfg_msgrel = cfg->GetInt(GLOBAL, "Chat", "MessageReliable", 1);
		/* cfghelp: Chat:FloodLimit, global, int, def: 10
		 * How many messages needed to be sent in a short period of time
		 * (about a second) to qualify for chat flooding. */
		cfg_floodlimit = cfg->GetInt(GLOBAL, "Chat", "FloodLimit", 10);
		/* cfghelp: Chat:FloodShutup, global, int, def: 60
		 * How many seconds to disable chat for a player that is
		 * flooding chat messages. */
		cfg_floodshutup = cfg->GetInt(GLOBAL, "Chat", "FloodShutup", 60);
		/* cfghelp: Chat:CommandLimit, global, int, def: 5
		 * How many commands are allowed on a single line. */
		cfg_cmdlimit = cfg->GetInt(GLOBAL, "Chat", "CommandLimit", 5);

		if (net)
			net->AddPacket(C2S_CHAT, PChat);

		if (chatnet)
			chatnet->AddHandler("SEND", MChat);

		mm->RegInterface(&_int, ALLARENAS);
		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		if (mm->UnregInterface(&_int, ALLARENAS))
			return MM_FAIL;
		if (net)
			net->RemovePacket(C2S_CHAT, PChat);
		if (chatnet)
			chatnet->RemoveHandler("SEND", MChat);
		mm->UnregCallback(CB_ARENAACTION, aaction, ALLARENAS);
		mm->UnregCallback(CB_PLAYERACTION, paction, ALLARENAS);
		if (persist)
			persist->UnregPlayerPD(&pdata);
		aman->FreeArenaData(cmkey);
		pd->FreePlayerData(pmkey);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(net);
		mm->ReleaseInterface(chatnet);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(capman);
		mm->ReleaseInterface(persist);
		return MM_OK;
	}
	return MM_FAIL;
}

