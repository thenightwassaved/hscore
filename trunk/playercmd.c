
/* dist: public */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef WIN32
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <unistd.h>
#endif


#include "asss.h"
#include "jackpot.h"
#include "persist.h"


/* global data */

local Iplayerdata *pd;
local Ichat *chat;
local Ilogman *lm;
local Icmdman *cmd;
local Inet *net;
local Iconfig *cfg;
local Icapman *capman;
local Igroupman *groupman;
local Imainloop *ml;
local Iarenaman *aman;
local Igame *game;
local Ijackpot *jackpot;
local Iflags *flags;
local Iballs *balls;
local Ilagquery *lagq;
local Ipersist *persist;
local Istats *stats;
local Imapdata *mapdata;
local Imodman *mm;

static ticks_t startedat;



local void translate_arena_packet(Player *p, char *pkt, int len)
{
	const char *pos = pkt + 1;

	chat->SendMessage(p, "Available arenas:");
	while (pos-pkt < len)
	{
		const char *next = pos + strlen(pos) + 3;
		int count = ((byte)next[-1] << 8) | (byte)next[-2];
		/* manually two's complement. yuck. */
		if (count & 0x8000)
			chat->SendMessage(p, "  %-16s %3d (current)", pos, (count ^ 0xffff) + 1);
		else
			chat->SendMessage(p, "  %-16s %3d", pos, count);
		pos = next;
	}
}

/* returns 0 if found, 1 if not */
local int check_arena(char *pkt, int len, char *check)
{
	char *pos = pkt + 1;
	while (pos-pkt < len)
	{
		if (strcasecmp(pos, check) == 0)
			return 0;
		/* skip over string, null terminator, and two bytes of count */
		pos += strlen(pos) + 3;
	}
	return 1;
}

local helptext_t arena_help =
"Targets: none\n"
"Args: [{-a}] [{-t}]\n"
"Lists the available arenas. Specifying {-a} will also include\n"
"empty arenas that the server knows about. The {-t} switch forces\n"
"the output to be in text even for regular clients (useful when using\n"
"the Continuum chat window).\n";

local void Carena(const char *tc, const char *params, Player *p, const Target *target)
{
	byte buf[MAXPACKET];
	byte *pos = buf;
	int l, seehid;
	Arena *a;
	Link *link;

	*pos++ = S2C_ARENA;

	aman->Lock();

	aman->GetPopulationSummary(NULL, NULL);

	/* build arena info packet */
	seehid = capman && capman->HasCapability(p, CAP_SEEPRIVARENA);
	FOR_EACH_ARENA(a)
	{
		if ((pos-buf) > 480) break;

		if (a->status == ARENA_RUNNING &&
		    ( a->name[0] != '#' || seehid || p->arena == a ))
		{
			int count = a->total;
			/* signify current arena */
			if (p->arena == a)
				count = -count;
			l = strlen(a->name) + 1;
			memcpy(pos, a->name, l);
			pos += l;
			*pos++ = (count >> 0) & 0xFF;
			*pos++ = (count >> 8) & 0xFF;
		}
	}

	aman->Unlock();

#ifdef CFG_DO_EXTRAARENAS
	/* add in more arenas if requested */
	if (strstr(params, "-a"))
	{
		char aconf[PATH_MAX];
		DIR *dir = opendir("arenas");
		if (dir)
		{
			struct dirent *de;
			while ((de = readdir(dir)))
			{
				/* every arena must have an arena.conf. this filters out
				 * ., .., CVS, etc. */
				snprintf(aconf, sizeof(aconf), "arenas/%s/arena.conf", de->d_name);
				if (
						(pos-buf+strlen(de->d_name)) < 480 &&
						de->d_name[0] != '(' &&
						access(aconf, R_OK) == 0 &&
						(de->d_name[0] != '#' || seehid) &&
						check_arena(buf, pos-buf, de->d_name)
				   )
				{
					l = strlen(de->d_name) + 1;
					strncpy(pos, de->d_name, l);
					pos += l;
					*pos++ = 0;
					*pos++ = 0;
				}
			}
			closedir(dir);
		}
	}
#endif

	/* send it */
	if (IS_CHAT(p) || strstr(params, "-t"))
		translate_arena_packet(p, buf, pos-buf);
	else if (IS_STANDARD(p))
		net->SendToOne(p, buf, pos-buf, NET_RELIABLE);
}


local helptext_t shutdown_help =
"Targets: none\n"
"Args: [{-r}]\n"
"Immediately shuts down the server, exiting with {EXIT_NONE}. If\n"
"{-r} is specified, exit with {EXIT_RECYCLE} instead. The {run-asss}\n"
"script, if it is being used, will notice {EXIT_RECYCLE} and restart\n"
"the server.\n";

local void Cshutdown(const char *tc, const char *params, Player *p, const Target *target)
{
	int code = EXIT_NONE;

	if (!strcmp(params, "-r"))
		code = EXIT_RECYCLE;

	ml->Quit(code);
}


local helptext_t flagreset_help =
"Targets: none\n"
"Args: none\n"
"Causes the flag game to immediately reset.\n";

local void Cflagreset(const char *tc, const char *params, Player *p, const Target *target)
{
	flags->FlagVictory(p->arena, -1, 0);
}


local helptext_t ballcount_help =
"Targets: none\n"
"Args: [<new # of balls> | +<balls to add> | -<balls to remove>]\n"
"Displays or changes the number of balls in the arena. A number without\n"
"a plus or minus sign is taken as a new count. A plus signifies adding\n"
"that many, and a minus removes that many. Continuum currently supports\n"
"only eight balls.\n";

local void Cballcount(const char *tc, const char *params, Player *p, const Target *target)
{
	Arena *arena = p->arena;
	if (arena)
	{
		ArenaBallData *abd = balls->GetBallData(arena);
		if (*params == '\0')
			chat->SendMessage(p, "%d balls.", abd->ballcount);
		else if (*params == '+' || *params == '-')
			balls->SetBallCount(arena, abd->ballcount + strtol(params, NULL, 0));
		else
			balls->SetBallCount(arena, strtol(params, NULL, 0));
		balls->ReleaseBallData(arena);
	}
}


local helptext_t setfreq_help =
"Targets: player, freq, or arena\n"
"Args: <freq number>\n"
"Moves the targets to the specified freq.\n";

local void Csetfreq(const char *tc, const char *params, Player *p, const Target *target)
{
	if (!*params)
		return;

	if (target->type == T_PLAYER)
		game->SetFreq(target->u.p, atoi(params));
	else
	{
		int freq = atoi(params);
		LinkedList set = LL_INITIALIZER;
		Link *l;

		pd->TargetToSet(target, &set);
		for (l = LLGetHead(&set); l; l = l->next)
			game->SetFreq(l->data, freq);
		LLEmpty(&set);
	}
}


local helptext_t setship_help =
"Targets: player, freq, or arena\n"
"Args: <ship number>\n"
"Sets the targets to the specified ship. The argument must be a\n"
"number from 1 (Warbird) to 8 (Shark), or 9 (Spec).\n";

local void Csetship(const char *tc, const char *params, Player *p, const Target *target)
{
	if (!*params)
		return;

	if (target->type == T_PLAYER)
		game->SetShip(target->u.p, atoi(params) - 1);
	else
	{
		int ship = atoi(params) - 1;
		LinkedList set = LL_INITIALIZER;
		Link *l;

		if (ship < SHIP_WARBIRD || ship > SHIP_SPEC)
			return;

		pd->TargetToSet(target, &set);
		for (l = LLGetHead(&set); l; l = l->next)
			game->SetShip(l->data, ship);
		LLEmpty(&set);
	}
}


local helptext_t version_help =
"Targets: none\n"
"Args: none\n"
"Prints out the version and compilation date of the server. It might also\n"
"print out some information about the machine that it's running on.\n";

local void Cversion(const char *tc, const char *params, Player *p, const Target *target)
{
	chat->SendMessage(p, "asss %s built on %s", ASSSVERSION, BUILDDATE);
#ifdef CFG_EXTRA_VERSION_INFO
#ifndef WIN32
	{
		struct utsname un;
		uname(&un);
		chat->SendMessage(p, "running on %s %s, host: %s, machine: %s",
				un.sysname, un.release, un.nodename, un.machine);
	}
#else
	{
		OSVERSIONINFO vi;
		DWORD len;
		char name[MAX_COMPUTERNAME_LENGTH + 1];

		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&vi);

		len = MAX_COMPUTERNAME_LENGTH + 1;
		GetComputerName(name, &len);

		chat->SendMessage(p, "running on %s %s (version %ld.%ld.%ld), host: %s",
			vi.dwPlatformId == VER_PLATFORM_WIN32s ? "Windows 3.11" : 
				vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ? 
					(vi.dwMinorVersion == 0 ? "Windows 95" : "Windows 98") :
				vi.dwPlatformId == VER_PLATFORM_WIN32_NT ? "Windows NT" : "Unknown",
			vi.szCSDVersion,
			vi.dwMajorVersion, vi.dwMinorVersion,
			vi.dwBuildNumber,
			name);
	}
#endif
#endif

#ifdef CFG_LOG_PRIVATE
	chat->SendMessage(p, "This server IS logging private and chat messages.");
#endif
}


#define MAXDATA 4090

local void add_mod(const char *name, const char *info, void *buf)
{
	char *start = buf, *p;
	int l = strlen(buf);
	p = start + l;
	snprintf(p, MAXDATA - l, ", %s", name);
}

local void send_msg_cb(const char *line, void *clos)
{
	chat->SendMessage((Player*)clos, "  %s", line);
}

local helptext_t lsmod_help =
"Targets: none\n"
"Args: [{-a}]\n"
"Lists all the modules currently loaded into the server. With {-a}, lists\n"
"only modules attached to this arena.\n";

local void Clsmod(const char *tc, const char *params, Player *p, const Target *target)
{
	char data[MAXDATA+6];
	memset(data, 0, sizeof(data));
	if (strstr(params, "-a"))
	{
		mm->EnumModules(add_mod, (void*)data, p->arena);
		chat->SendMessage(p, "Modules attached to arena %s:", p->arena->name);
	}
	else
	{
		mm->EnumModules(add_mod, (void*)data, NULL);
		chat->SendMessage(p, "Loaded modules:");
	}
	wrap_text(data+2, 80, ' ', send_msg_cb, p);
}


local helptext_t modinfo_help =
"Targets: none\n"
"Args: <module name>\n"
"Displays information about the specified module. This might include a\n"
"version number, contact information for the author, and a general\n"
"description of the module.\n";

local void Cmodinfo(const char *tc, const char *params, Player *p, const Target *target)
{
	const char *info = mm->GetModuleInfo(params);
	if (!info)
		chat->SendMessage(p, "No information for module '%s'", params);
	else
	{
		const char *tmp = NULL;
		char buf[100];
		chat->SendMessage(p, "Info for module '%s':", params);
		while (strsplit(info, "\n", buf, sizeof(buf), &tmp))
			chat->SendMessage(p, "  %s", buf);
	}
}


#ifndef CFG_NO_RUNTIME_LOAD
local helptext_t insmod_help =
"Targets: none\n"
"Args: <module specifier>\n"
"Immediately loads the specified module into the server.\n";

local void Cinsmod(const char *tc, const char *params, Player *p, const Target *target)
{
	int ret;
	ret = mm->LoadModule(params);
	if (ret == MM_OK)
		chat->SendMessage(p, "Module %s loaded successfully", params);
	else
		chat->SendMessage(p, "Loading module %s failed", params);
}
#endif


local helptext_t rmmod_help =
"Targets: none\n"
"Args: <module name>\n"
"Attempts to unload the specified module from the server.\n";

local void Crmmod(const char *tc, const char *params, Player *p, const Target *target)
{
	int ret;
	ret = mm->UnloadModule(params);
	if (ret == MM_OK)
		chat->SendMessage(p, "Module %s unloaded successfully", params);
	else
		chat->SendMessage(p, "Unloading module %s failed", params);
}


local helptext_t attmod_help =
"Targets: none\n"
"Args: [{-d}] <module name>\n"
"Attaches the specified module to this arena. Or with {-d},\n"
"detaches the module from the arena.\n";

local void Cattmod(const char *tc, const char *params, Player *p, const Target *target)
{
	if (strncmp(params, "-d", 2) == 0)
	{
		const char *t = params + 2;
		while (isspace(*t)) t++;
		mm->DetachModule(t, p->arena);
	}
	else
		mm->AttachModule(params, p->arena);
}


local helptext_t find_help =
"Targets: none\n"
"Args: <all or part of a player name>\n"
"Tells you where the specified player is right now. If you specify\n"
"only part of a player name, it will try to find a matching name\n"
"using a case insensitive substring search.\n";

local void Cfind(const char *tc, const char *params, Player *p, const Target *target)
{
	Link *link;
	Player *i;
	char newcmd[64];

	if (target->type != T_ARENA || !*params) return;

	pd->Lock();
	FOR_EACH_PLAYER(i)
	{
		if (i->status == S_PLAYING &&
		    strcasestr(i->name, params))
		{
			if (i->arena->name[0] != '#' ||
			    capman->HasCapability(p, CAP_SEEPRIVARENA) ||
			    p->arena == i->arena)
				chat->SendMessage(p, "%s is in arena %s.", i->name, i->arena->name);
			else
				chat->SendMessage(p, "%s is in a private arena.", i->name);
			pd->Unlock();
			return;
		}
	}
	pd->Unlock();

	/* if not found, fall back to the default */
	snprintf(newcmd, sizeof(newcmd), "\\find %s", params);
	cmd->Command(newcmd, p, target);
}


local helptext_t getgroup_help =
"Targets: player or none\n"
"Args: none\n"
"Displays the group of the player, or if none specified, you.\n";

local void Cgetgroup(const char *tc, const char *params, Player *p, const Target *target)
{
	if (target->type == T_PLAYER)
		chat->SendMessage(p, "%s is in group %s",
				target->u.p->name,
				groupman->GetGroup(target->u.p));
	else if (target->type == T_ARENA)
		chat->SendMessage(p, "You are in group %s",
				groupman->GetGroup(p));
	else
		chat->SendMessage(p, "Bad target");
}


local helptext_t setgroup_help =
"Targets: player\n"
"Args: [{-a}] [{-p}] <group name>\n"
"Assigns the group given as an argument to the target player. The player\n"
"must be in group {default}, or the server will refuse to change his\n"
"group. Additionally, the player giving the command must have an\n"
"appropriate capability: {setgroup_foo}, where {foo} is the\n"
"group that he's trying to set the target to.\n"
"\n"
"The optional {-p} means to assign the group permanently. Otherwise, when\n"
"the target player logs out or changes arenas, the group will be lost.\n"
"\n"
"The optional {-a} means to make the assignment local to the current\n"
"arena, rather than being valid in the entire zone.\n";

local void Csetgroup(const char *tc, const char *params, Player *p, const Target *target)
{
	int perm = 0, global = 1;
	Player *t = target->u.p;
	char cap[MAXGROUPLEN+16];

	if (!*params) return;
	if (target->type != T_PLAYER) return;

	while (*params && strchr(params, ' '))
	{
		if (!strncmp(params, "perm", 4) || !strncmp(params, "-p", 2))
			perm = 1;
		if (!strncmp(params, "arena", 5) || !strncmp(params, "-a", 2))
			global = 0;
		params = strchr(params, ' ') + 1;
	}
	if (!*params) return;

	/* make sure the setter has permissions to set people to this group */
	snprintf(cap, sizeof(cap), "higher_than_%s", params);
	if (!capman->HasCapability(p, cap))
	{
		chat->SendMessage(p, "You don't have permission to give people group %s.", params);
		lm->LogP(L_WARN, "playercmd", p, "doesn't have permission to set to group '%s'",
				params);
		return;
	}

	/* make sure the target isn't in a group already */
	if (strcasecmp(groupman->GetGroup(t), "default"))
	{
		chat->SendMessage(p, "Player %s already has a group. You need to use ?rmgroup first.", t->name);
		lm->LogP(L_WARN, "playercmd", p, "tried to set the group of [%s],"
				"who is in '%s' already, to '%s'",
				t->name, groupman->GetGroup(t), params);
		return;
	}

	if (perm)
	{
		time_t tm = time(NULL);
		char info[128];

		snprintf(info, sizeof(info), "set by %s on ", p->name);
		ctime_r(&tm, info + strlen(info));
		RemoveCRLF(info);

		groupman->SetPermGroup(t, params, global, info);
		chat->SendMessage(p, "%s is now in group %s",
				t->name, params);
		chat->SendMessage(t, "You have been assigned to group %s by %s",
				params, p->name);
	}
	else
	{
		groupman->SetTempGroup(t, params);
		chat->SendMessage(p, "%s is now temporarily in group %s",
				t->name, params);
		chat->SendMessage(t, "You have temporarily been assigned to group %s by %s",
				params, p->name);
	}
}


local helptext_t rmgroup_help =
"Targets: player\n"
"Args: none\n"
"Removes the group from a player, returning him to group 'default'. If\n"
"the group was assigned for this session only, then it will be removed\n"
"for this session; if it is a global group, it will be removed globally;\n"
"and if it is an arena group, it will be removed for this arena.\n";

local void Crmgroup(const char *tc, const char *params, Player *p, const Target *target)
{
	Player *t = target->u.p;
	char cap[MAXGROUPLEN+16], info[128];
	const char *grp;
	time_t tm = time(NULL);

	if (!*params) return;
	if (target->type != T_PLAYER) return;

	while (*params && strchr(params, ' '))
	{
		params = strchr(params, ' ') + 1;
	}
	if (!*params) return;

	grp = groupman->GetGroup(t);
	snprintf(cap, sizeof(cap), "higher_than_%s", grp);

	if (!capman->HasCapability(p, cap))
	{
		chat->SendMessage(p, "You don't have permission to take away group %s.", grp);
		lm->LogP(L_WARN, "playercmd", p, "doesn't have permission to take away group '%s'",
				grp);
		return;
	}

	chat->SendMessage(p, "%s has been removed from group %s", t->name, grp);
	chat->SendMessage(t, "You have been removed group %s.", grp);

	snprintf(info, sizeof(info), "set by %s on ", p->name);
	ctime_r(&tm, info + strlen(info));
	RemoveCRLF(info);

	/* groupman keeps track of the source of the group, so we just have
	 * to call this. */
	groupman->RemoveGroup(t, info);
}


local helptext_t grplogin_help =
"Targets: none\n"
"Args: <group name> <password>\n"
"Logs you in to the specified group, if the password is correct.\n";

local void Cgrplogin(const char *tc, const char *params, Player *p, const Target *target)
{
	char grp[MAXGROUPLEN+1];
	const char *pw;

	pw = delimcpy(grp, params, MAXGROUPLEN, ' ');
	if (grp[0] == '\0' || pw == NULL)
		chat->SendMessage(p, "You must specify a group name and password");
	else if (groupman->CheckGroupPassword(grp, pw))
	{
		groupman->SetTempGroup(p, grp);
		chat->SendMessage(p, "You are now in group %s", grp);
	}
	else
		chat->SendMessage(p, "Bad password for group %s", grp);
}


local helptext_t listmod_help =
"Targets: none\n"
"Args: none\n"
"Lists all staff members logged on, which arena they are in, and\n"
"which group they belong to.\n";

local void Clistmod(const char *tc, const char *params, Player *p, const Target *target)
{
	int seehid = capman->HasCapability(p, CAP_SEEPRIVARENA);
	int seeallstaff = capman->HasCapability(p, CAP_SEE_ALL_STAFF);
	Player *i;
	Link *link;
	const char *grp, *fmt;

	pd->Lock();
	FOR_EACH_PLAYER(i)
	{
		if (i->status != S_PLAYING)
			continue;

		grp = groupman->GetGroup(i);

		if (capman->HasCapability(i, CAP_IS_STAFF))
			fmt = ": %20s %10s %10s";
		else if (seeallstaff && strcmp(grp, "default") != 0)
			fmt = ": %20s %10s (%8s)";
		else
			fmt = NULL;

		if (fmt)
			chat->SendMessage( p, fmt, i->name,
					(i->arena->name[0] != '#' || seehid || p->arena == i->arena) ?
						i->arena->name : "(private)", grp);
	}
	pd->Unlock();
}


local helptext_t netstats_help =
"Targets: none\n"
"Args: none\n"
"Prints out some statistics from the network layer.\n";

local void Cnetstats(const char *tc, const char *params, Player *p, const Target *target)
{
	ticks_t secs = TICK_DIFF(current_ticks(), startedat) / 100;
	unsigned long bwin, bwout;
	struct net_stats stats;

	net->GetStats(&stats);

	chat->SendMessage(p, "netstats: pings=%lu  pkts sent=%lu  pkts recvd=%lu",
			stats.pcountpings, stats.pktsent, stats.pktrecvd);
	bwout = (stats.bytesent + stats.pktsent * 28) / secs;
	bwin = (stats.byterecvd + stats.pktrecvd * 28) / secs;
	chat->SendMessage(p, "netstats: bw out=%lu  bw in=%lu", bwout, bwin);
	chat->SendMessage(p, "netstats: buffers used=%lu/%lu (%.1f%%)",
			stats.buffersused, stats.buffercount,
			(double)stats.buffersused/(double)stats.buffercount*100.0);
}


local helptext_t info_help =
"Targets: player\n"
"Args: none\n"
"Displays various information on the target player, including which\n"
"client they are using, their resolution, IP address, how long they have\n"
"been connected, and bandwidth usage information.\n";

local void Cinfo(const char *tc, const char *params, Player *p, const Target *target)
{
	if (target->type != T_PLAYER)
		chat->SendMessage(p, "info: must use on a player");
	else
	{
		const char *prefix;
		int tm;
		Player *t = target->u.p;

		prefix = params[0] ? params : "info";
		tm = TICK_DIFF(current_ticks(), t->connecttime);

		chat->SendMessage(p,
				"%s: pid=%d  name='%s'  squad='%s'  auth=%c  ship=%d  freq=%d",
				prefix, t->pid, t->name, t->squad, t->flags.authenticated ? 'y' : 'n',
				t->p_ship, t->p_freq);
		chat->SendMessage(p,
				"%s: arena=%s  client=%s  res=%dx%d  onfor=%d  connectas=%s",
				prefix, t->arena ? t->arena->name : "(none)", t->clientname, t->xres,
				t->yres, tm / 100, p->connectas ? p->connectas : "<default>");
		if (IS_STANDARD(t))
		{
			struct net_client_stats s;
			int ignoring;
			net->GetClientStats(t, &s);
			chat->SendMessage(p,
					"%s: ip=%s  port=%d  encname=%s  macid=%u  permid=%u",
					prefix, s.ipaddr, s.port, s.encname, t->macid, t->permid);
			ignoring = (int)(100.0 * game->GetIgnoreWeapons(t));
			chat->SendMessage(p,
					"%s: limit=%d  avg bw in/out=%ld/%ld  ignoringwpns=%d%%  dropped=%ld",
					prefix, s.limit, s.byterecvd*100/tm, s.bytesent*100/tm,
					ignoring, s.pktdropped);
		}
		else if (IS_CHAT(t))
		{
			Ichatnet *chatnet = mm->GetInterface(I_CHATNET, ALLARENAS);
			if (chatnet)
			{
				struct chat_client_stats s;
				chatnet->GetClientStats(t, &s);
				chat->SendMessage(p,
						"%s: ip=%s  port=%d",
						prefix, s.ipaddr, s.port);
				mm->ReleaseInterface(chatnet);
			}
		}
		if (t->flags.no_ship)
			chat->SendMessage(p, "%s: lag too high to play", prefix);
		if (t->flags.no_flags_balls)
			chat->SendMessage(p, "%s: lag too high to carry flags or balls", prefix);
		if (t->flags.see_all_posn)
			chat->SendMessage(p, "%s: requested all position packets", prefix);
		if (t->status != S_PLAYING)
			chat->SendMessage(p, "%s: status=%d", prefix, t->status);
	}
}


local helptext_t setcm_help =
"Targets: player or arena\n"
"Args: see description\n"
"Modifies the chat mask for the target player, or if no target, for the\n"
"current arena. The arguments must all be of the form\n"
"{(-|+)(pub|pubmacro|freq|nmefreq|priv|chat|modchat|all)} or {-time <seconds>}.\n"
"A minus sign and then a word disables that type of chat, and a plus sign\n"
"enables it. The special type {all} means to apply the plus or minus to\n"
"all of the above types. {-time} lets you specify a timeout in seconds.\n"
"The mask will be effective for that time, even across logouts.\n"
"\n"
"Examples:\n"
" * If someone is spamming public macros: {:player:?setcm -pubmacro -time 600}\n"
" * To disable all blue messages for this arena: {?setcm -pub -pubmacro}\n"
" * An equivalent to *shutup: {:player:?setcm -all}\n"
" * To restore chat to normal: {?setcm +all}\n"
"\n"
"Current limitations: You can't currently restrict a particular\n"
"frequency. Leaving and entering an arena will remove a player's chat\n"
"mask, unless it has a timeout.\n";

local void Csetcm(const char *tc, const char *params, Player *p, const Target *target)
{
	chat_mask_t mask;
	int timeout = 0;
	const char *c = params;

	/* grab the original mask */
	if (target->type == T_ARENA)
		mask = chat->GetArenaChatMask(target->u.arena);
	else if (target->type == T_PLAYER)
		mask = chat->GetPlayerChatMask(target->u.p);
	else
	{
		chat->SendMessage(p, "setcm: Bad target");
		return;
	}

	/* change it */
	for (;;)
	{
		chat_mask_t newmask = 0;
		int all = 0;

		/* move to next + or - */
		while (*c != '\0' && *c != '-' && *c != '+')
			c++;
		if (*c == '\0')
			break;

		/* figure out which thing to change */
		c++;
		if (!strncasecmp(c, "all", 3))
			all = 1;
		if (all || !strncasecmp(c, "pubmacro", 8))
			newmask |= 1 << MSG_PUBMACRO;
		if (all || !strncasecmp(c, "pub", 3))
			newmask |= 1 << MSG_PUB;
		if (all || !strncasecmp(c, "freq", 4))
			newmask |= 1 << MSG_FREQ;
		if (all || !strncasecmp(c, "nmefreq", 7))
			newmask |= 1 << MSG_NMEFREQ;
		if (all || !strncasecmp(c, "priv", 4))
			newmask |= (1 << MSG_PRIV) | (1 << MSG_REMOTEPRIV);
		if (all || !strncasecmp(c, "chat", 4))
			newmask |= 1 << MSG_CHAT;
		if (all || !strncasecmp(c, "modchat", 7))
			newmask |= 1 << MSG_MODCHAT;

		if (!strncasecmp(c, "time", 4))
			timeout = strtol(c+4, NULL, 0);

		/* change it */
		if (c[-1] == '+')
			mask &= ~newmask;
		else
			mask |= newmask;
	}

	/* and install it back where it came from */
	if (target->type == T_ARENA)
		chat->SetArenaChatMask(target->u.arena, mask);
	else
		chat->SetPlayerChatMask(target->u.p, mask, timeout);
}

local helptext_t getcm_help =
"Targets: player or arena\n"
"Args: none\n"
"Prints out the chat mask for the target player, or if no target, for the\n"
"current arena. The chat mask specifies which types of chat messages are\n"
"allowed.\n";

local void Cgetcm(const char *tc, const char *params, Player *p, const Target *target)
{
	chat_mask_t mask;

	if (target->type == T_ARENA)
		mask = chat->GetArenaChatMask(target->u.arena);
	else if (target->type == T_PLAYER)
		mask = chat->GetPlayerChatMask(target->u.p);
	else
	{
		chat->SendMessage(p, "getcm: Bad target");
		return;
	}

	chat->SendMessage(p,
			"getcm: %cpub %cpubmacro %cfreq %cnmefreq %cpriv %cchat %cmodchat",
			IS_RESTRICTED(mask, MSG_PUB) ? '-' : '+',
			IS_RESTRICTED(mask, MSG_PUBMACRO) ? '-' : '+',
			IS_RESTRICTED(mask, MSG_FREQ) ? '-' : '+',
			IS_RESTRICTED(mask, MSG_NMEFREQ) ? '-' : '+',
			IS_RESTRICTED(mask, MSG_PRIV) ? '-' : '+',
			IS_RESTRICTED(mask, MSG_CHAT) ? '-' : '+',
			IS_RESTRICTED(mask, MSG_MODCHAT) ? '-' : '+'
			);
}


local helptext_t a_help =
"Targets: player, freq, or arena\n"
"Args: <text>\n"
"Displays the text as an arena (green) message to the targets.\n";

local void Ca(const char *tc, const char *params, Player *p, const Target *target)
{
	LinkedList set = LL_INITIALIZER;
	pd->TargetToSet(target, &set);
	chat->SendSetMessage(&set, "%s  -%s", params, p->name);
	LLEmpty(&set);
}


local helptext_t aa_help =
"Targets: player, freq, or arena\n"
"Args: <text>\n"
"Displays the text as an anonymous arena (green) message to the targets.\n";

local void Caa(const char *tc, const char *params, Player *p, const Target *target)
{
	LinkedList set = LL_INITIALIZER;
	pd->TargetToSet(target, &set);
	chat->SendSetMessage(&set, "%s", params);
	LLEmpty(&set);
}


local helptext_t z_help =
"Targets: none\n"
"Args: <text>\n"
"Displays the text as an arena (green) message to the whole zone.\n";

local void Cz(const char *tc, const char *params, Player *p, const Target *target)
{
	chat->SendArenaMessage(NULL, "%s  -%s", params, p->name);
}


local helptext_t az_help =
"Targets: none\n"
"Args: <text>\n"
"Displays the text as an anonymous arena (green) message to the whole zone.\n";

local void Caz(const char *tc, const char *params, Player *p, const Target *target)
{
	chat->SendArenaMessage(NULL, "%s", params);
}


local helptext_t cheater_help =
"Targets: none\n"
"Args: <message>\n"
"Sends the message to all online staff members.\n";

local void Ccheater(const char *tc, const char *params, Player *p, const Target *target)
{
	Arena *arena = p->arena;
	if (IS_ALLOWED(chat->GetPlayerChatMask(p), MSG_MODCHAT))
	{
		chat->SendModMessage("cheater {%s} %s: %s",
				arena->name, p->name, params);
		chat->SendMessage(p, "Message has been sent to online staff");
	}
}


local helptext_t warn_help =
"Targets: player\n"
"Args: <message>\n"
"Send a red warning message to a player.\n";

local void Cwarn(const char *tc, const char *params, Player *p, const Target *target)
{
	if (target->type != T_PLAYER)
		chat->SendMessage(p, "You must target a player.");
	else
	{
		Link link = { NULL, target->u.p };
		LinkedList lst = { &link, &link };
		chat->SendAnyMessage(&lst, MSG_SYSOPWARNING, SOUND_BEEP1, NULL,
				"WARNING: %s  -%s", params, p->name);
		chat->SendMessage(p, "Player warned");
	}
}


local helptext_t warpto_help =
"Targets: player, freq, or arena\n"
"Args: <x coord> <y coord>\n"
"Warps target player to coordinate x,y.\n";

local void Cwarpto(const char *tc, const char *params, Player *p, const Target *target)
{
	char *next;
	int x, y;

	x = strtol(params, &next, 0);
	if (next == params) return;
	while (*next == ',' || *next == ' ') next++;
	y = strtol(next, NULL, 0);
	if (x == 0 || y == 0) return;
	game->WarpTo(target, x, y);
}


local helptext_t send_help =
"Targets: player\n"
"Args: <arena name>\n"
"Sends target player to the named arena. (Works on Continuum users only.)\n";

local void Csend(const char *tc, const char *params, Player *p, const Target *target)
{
	Player *t = target->u.p;
	if (target->type != T_PLAYER || *params == '\0')
		return;
	if (t->type == T_CONT)
		aman->SendToArena(t, params, 0, 0);
	else
		chat->SendMessage(p, "You can only use ?send on players using Continuum");
}


local helptext_t recyclearena_help =
"Targets: none\n"
"Args: none\n"
"Recycles the current arena without kicking players off.\n";

local void Crecyclearena(const char *tc, const char *params, Player *p, const Target *target)
{
	if (aman->RecycleArena(p->arena) == MM_FAIL)
		chat->SendMessage(p, "Arena recycle failed; check the log for details.");
}


local helptext_t shipreset_help =
"Targets: player, freq, or arena\n"
"Args: none\n"
"Resets the target players' ship(s).\n";

local void Cshipreset(const char *tc, const char *params, Player *p, const Target *target)
{
	byte pkt = S2C_SHIPRESET;
	net->SendToTarget(target, &pkt, 1, NET_RELIABLE);
}


local helptext_t sheep_help = NULL;

local void Csheep(const char *tc, const char *params, Player *p, const Target *target)
{
	Arena *arena = p->arena;
	const char *sheepmsg = NULL;

	if (target->type != T_ARENA)
		return;

	/* cfghelp: Misc:SheepMessage, arena, string
	 * The message that appears when someone says ?sheep */
	if (arena)
		sheepmsg = cfg->GetStr(arena->cfg, "Misc", "SheepMessage");

	if (sheepmsg)
		chat->SendSoundMessage(p, 24, "%s", sheepmsg);
	else
		chat->SendSoundMessage(p, 24, "Sheep successfully cloned -- hello Dolly");
}


local helptext_t specall_help =
"Targets: player, freq, or arena\n"
"Args: none\n"
"Sends all of the targets to spectator mode.\n";

local void Cspecall(const char *tc, const char *params, Player *p, const Target *target)
{
	Arena *arena = p->arena;
	LinkedList set = LL_INITIALIZER;
	Link *l;

	if (!arena) return;

	pd->TargetToSet(target, &set);
	for (l = LLGetHead(&set); l; l = l->next)
		game->SetFreqAndShip(l->data, SHIP_SPEC, arena->specfreq);
	LLEmpty(&set);
}


local helptext_t getg_help =
"Targets: none\n"
"Args: section:key\n"
"Displays the value of a global setting. Make sure there are no\n"
"spaces around the colon.\n";

local void Cgetg(const char *tc, const char *params, Player *p, const Target *target)
{
	const char *res = cfg->GetStr(GLOBAL, params, NULL);
	if (res)
		chat->SendMessage(p, "%s=%s", params, res);
	else
		chat->SendMessage(p, "%s not found", params);
}


local helptext_t setg_help =
"Targets: none\n"
"Args: [{-t}] section:key=value\n"
"Sets the value of a global setting. Make sure there are no\n"
"spaces around either the colon or the equals sign. A {-t} makes\n"
"the setting temporary.\n";

local void Csetg(const char *tc, const char *params, Player *p, const Target *target)
{
	time_t tm = time(NULL);
	char info[128], key[MAXSECTIONLEN+MAXKEYLEN+2], *k = key;
	const char *t = params;
	int perm = TRUE;

	snprintf(info, sizeof(info), "set by %s on ", p->name);
	ctime_r(&tm, info + strlen(info));
	RemoveCRLF(info);

	if (strncmp(t, "-t", 2) == 0)
	{
		perm = FALSE;
		t += 2;
		while (*t && *t == ' ') t++;
	}

	while (*t && *t != '=' && (k-key) < (MAXSECTIONLEN+MAXKEYLEN))
		*k++ = *t++;
	if (*t != '=') return;
	*k = '\0'; /* terminate key */
	t++; /* skip over = */

	cfg->SetStr(GLOBAL, key, NULL, t, info, perm);
}

local helptext_t geta_help =
"Targets: none\n"
"Args: section:key\n"
"Displays the value of an arena setting. Make sure there are no\n"
"spaces around the colon.\n";

local void Cgeta(const char *tc, const char *params, Player *p, const Target *target)
{
	Arena *arena = p->arena;
	const char *res;

	if (!arena) return;

	res = cfg->GetStr(arena->cfg, params, NULL);
	if (res)
		chat->SendMessage(p, "%s=%s", params, res);
	else
		chat->SendMessage(p, "%s not found", params);
}

local helptext_t seta_help =
"Targets: none\n"
"Args: [{-t}] section:key=value\n"
"Sets the value of an arena setting. Make sure there are no\n"
"spaces around either the colon or the equals sign. A {-t} makes\n"
"the setting temporary.\n";

local void Cseta(const char *tc, const char *params, Player *p, const Target *target)
{
	Arena *arena = p->arena;
	time_t tm = time(NULL);
	char info[128], key[MAXSECTIONLEN+MAXKEYLEN+2], *k = key;
	const char *t = params;
	int perm = TRUE;

	if (!arena) return;

	snprintf(info, sizeof(info), "set by %s on ", p->name);
	ctime_r(&tm, info + strlen(info));
	RemoveCRLF(info);

	if (strncmp(t, "-t", 2) == 0)
	{
		perm = FALSE;
		t += 2;
		while (*t && *t == ' ') t++;
	}

	while (*t && *t != '=' && (k-key) < (MAXSECTIONLEN+MAXKEYLEN))
		*k++ = *t++;
	if (*t != '=') return;
	*k = '\0'; /* terminate key */
	t++; /* skip over = */

	cfg->SetStr(arena->cfg, key, NULL, t, info, perm);
}


local helptext_t prize_help =
"Targets: player, freq, or arena\n"
"Args: see description\n"
"Gives the specified prizes to the target player(s).\n"
"\n"
"Prizes are specified with an optional count, and then a prize name (e.g.\n"
"{3 reps}, {anti}). Negative prizes can be specified with a '-' before\n"
"the prize name or the count (e.g. {-prox}, {-3 bricks}, {5 -guns}). More\n"
"than one prize can be specified in one command. A count without a prize\n"
"name means {random}. For compatability, numerical prize ids with {#} are\n"
"supported.\n";

local void Cprize(const char *tc, const char *params, Player *p, const Target *target)
{
#define BAD_TYPE 10000
	const char *tmp = NULL;
	char word[32];
	int i, type, count = 1, t;
	enum { last_none, last_count, last_word } last = last_none;
	struct
	{
		const char *string;
		int type;
	}
	lookup[] =
	{
		{ "random",    0 },
		{ "charge",   13 }, /* must come before "recharge" */
		{ "x",         6 }, /* must come before "prox" */
		{ "recharge",  1 },
		{ "energy",    2 },
		{ "rot",       3 },
		{ "stealth",   4 },
		{ "cloak",     5 },
		{ "warp",      7 },
		{ "gun",       8 },
		{ "bomb",      9 },
		{ "bounce",   10 },
		{ "thrust",   11 },
		{ "speed",    12 },
		{ "shutdown", 14 },
		{ "multi",    15 },
		{ "prox",     16 },
		{ "super",    17 },
		{ "shield",   18 },
		{ "shrap",    19 },
		{ "anti",     20 },
		{ "rep",      21 },
		{ "burst",    22 },
		{ "decoy",    23 },
		{ "thor",     24 },
		{ "mprize",   25 },
		{ "brick",    26 },
		{ "rocket",   27 },
		{ "port",     28 },
	};

	while (strsplit(params, " ,", word, sizeof(word), &tmp))
		if ((t = strtol(word, NULL, 0)) != 0)
		{
			/* this is a count */
			count = t;
			last = last_count;
		}
		else /* try a word */
		{
			/* negative prizes are marked with negative counts, for now */
			if (word[0] == '-')
				count = -count;

			/* now try to find the word */
			type = BAD_TYPE;
			if (word[0] == '#')
				type = strtol(word+1, NULL, 0);
			else
				for (i = 0; i < sizeof(lookup)/sizeof(lookup[0]); i++)
					if (strstr(word, lookup[i].string))
						type = lookup[i].type;

			if (type != BAD_TYPE)
			{
				/* but for actually sending them, they must be marked with
				 * negative types and positive counts. */
				if (count < 0)
				{
					type = -type;
					count = -count;
				}

				game->GivePrize(target, type, count);

				/* reset count to 1 once we hit a successful word */
				count = 1;
			}

			last = last_word;
		}

	if (last == last_count)
		/* if the line ends in a count, do that many of random */
		game->GivePrize(target, 0, count);

#undef BAD_TYPE
}


/* locking commands */

local helptext_t lock_help =
"Targets: player, freq, or arena\n"
"Args: [-n] [-s] [-t <timeout>]\n"
"Locks the specified targets so that they can't change ships. Use ?unlock\n"
"to unlock them. By default, ?lock won't change anyone's ship. If {-s} is\n"
"present, it will spec the targets before locking them. If {-n} is present,\n"
"it will notify players of their change in status. If {-t} is present, you\n"
"can specify a timeout in seconds for the lock to be effective.\n";

local void Clock(const char *tc, const char *params, Player *p, const Target *target)
{
	const char *t = strstr(params, "-t");
	game->Lock(
			target,
			strstr(params, "-n") != NULL,
			strstr(params, "-s") != NULL,
			t ? strtol(t+2, NULL, 0) : 0);
}


local helptext_t unlock_help =
"Targets: player, freq, or arena\n"
"Args: [-n]\n"
"Unlocks the specified targets so that they can now change ships. An optional\n"
"{-n} notifies players of their change in status.\n";

local void Cunlock(const char *tc, const char *params, Player *p, const Target *target)
{
	game->Unlock(target, strstr(params, "-n") != NULL);
}


local helptext_t lockarena_help =
"Targets: arena\n"
"Args: [-n] [-a] [-i] [-s]\n"
"Changes the default locked state for the arena so entering players will be locked\n"
"to spectator mode. Also locks everyone currently in the arena to their ships. The {-n}\n"
"option means to notify players of their change in status. The {-a} options means to\n"
"only change the arena's state, and not lock current players. The {-i} option means to\n"
"only lock entering players to their initial ships, instead of spectator mode. The {-s}\n"
"means to spec all players before locking the arena.\n";

local void Clockarena(const char *tc, const char *params, Player *p, const Target *target)
{
	if (target->type != T_ARENA) return;
	game->LockArena(target->u.arena,
			strstr(params, "-n") != NULL,
			strstr(params, "-a") != NULL,
			strstr(params, "-i") != NULL,
			strstr(params, "-s") != NULL);
}


local helptext_t unlockarena_help =
"Targets: arena\n"
"Args: [-n] [-a]\n"
"Changes the default locked state for the arena so entering players will not be\n"
"locked to spectator mode. Also unlocks everyone currently in the arena to their ships\n"
"The {-n} options means to notify players of their change in status. The {-a} option\n"
"means to only change the arena's state, and not unlock current players.\n";

local void Cunlockarena(const char *tc, const char *params, Player *p, const Target *target)
{
	if (target->type != T_ARENA) return;
	game->UnlockArena(target->u.arena,
			strstr(params, "-n") != NULL,
			strstr(params, "-a") != NULL);
}


local helptext_t flaginfo_help =
"Targets: none\n"
"Args: none\n"
"Displays information (status, location, carrier) about all the flags in\n"
"the arena.\n";

local void Cflaginfo(const char *tc, const char *params, Player *p, const Target *target)
{
	ArenaFlagData *fd;
	Arena *arena;
	int i;

	if (target->type != T_ARENA)
		return;

	arena = p->arena;

	fd = flags->GetFlagData(arena);

	for (i = 0; i < fd->flagcount; i++)
		switch (fd->flags[i].state)
		{
			case FLAG_NONE:
				chat->SendMessage(p,
						"flag %d: doesn't exist",
						i);
				break;

			case FLAG_ONMAP:
				{
					unsigned short x = fd->flags[i].x * 20 / 1024;
					unsigned short y = fd->flags[i].y * 20 / 1024;

					chat->SendMessage(p,
							"flag %d: on the map at %c%d (%d,%d), owned by freq %d",
							i, 'A'+x, y+1, fd->flags[i].x, fd->flags[i].y, fd->flags[i].freq);
				}
				break;

			case FLAG_CARRIED:
				if (fd->flags[i].carrier)
					chat->SendMessage(p,
							"flag %d: carried by %s",
							i, fd->flags[i].carrier->name);
				break;

			case FLAG_NEUTED:
				chat->SendMessage(p,
						"flag %d: neuted and has not reappeared yet",
						i);
				break;
		}

	flags->ReleaseFlagData(arena);
}


local helptext_t neutflag_help =
"Targets: none\n"
"Args: <flag id>\n"
"Neuts the specified flag in the middle of the arena.\n";

local void Cneutflag(const char *tc, const char *params, Player *p, const Target *target)
{
	ArenaFlagData *fd;
	Arena *arena = p->arena;
	int flagid;
	char *next;

	fd = flags->GetFlagData(arena);

	flagid = strtol(params, &next, 0);
	if (next == params || flagid < 0 || flagid >= fd->flagcount)
		chat->SendMessage(p, "neutflag: bad flag id");
	else if (fd->flags[flagid].state == FLAG_ONMAP ||
	         fd->flags[flagid].state == FLAG_NEUTED ||
	         /* undocumented flag letting you force a flag away from a
	          * player. this will leave the clients out of sync for a
	          * while, so it's dangerous. */
	         strcmp(next, "force") == 0)
		/* set flag state to none, so that the flag timer will neut it
		 * next time it runs. */
		fd->flags[flagid].state = FLAG_NONE;
	else
		chat->SendMessage(p, "neutflag: that flag isn't currently on the map");

	flags->ReleaseFlagData(arena);
}


local helptext_t moveflag_help =
"Targets: none\n"
"Args: <flag id> <owning freq> [<x coord> <y coord>]\n"
"Moves the specified flag. You must always specify the freq that will own\n"
"the flag. The coordinates are optional: if they are specified, the flag\n"
"will be moved there, otherwise it will remain where it is.\n";

local void Cmoveflag(const char *tc, const char *params, Player *p, const Target *target)
{
	Arena *arena = p->arena;
	ArenaFlagData *fd;
	char *next, *next2;
	int flagid, x, y, freq;

	if (!arena) return;

	fd = flags->GetFlagData(arena);

	flagid = strtol(params, &next, 0);
	if (next == params || flagid < 0 || flagid >= fd->flagcount)
	{
		chat->SendMessage(p, "moveflag: Bad flag id");
		goto mf_unlock;
	}

	freq = strtol(next, &next2, 0);
	if (next == next2)
	{
		/* bad freq */
		chat->SendMessage(p, "moveflag: Bad freq");
		goto mf_unlock;
	}

	x = strtol(next2, &next, 0);
	while (*next == ',' || *next == ' ') next++;
	y = strtol(next, NULL, 0);
	if (x == 0 || y == 0)
	{
		/* missing coords */
		x = fd->flags[flagid].x;
		y = fd->flags[flagid].y;
	}

	/* make sure it's not in a wall or off the map */
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > 1023) x = 1023;
	if (y > 1023) y = 1023;

	if (mapdata)
		mapdata->FindEmptyTileNear(arena, &x, &y);

	flags->MoveFlag(arena, flagid, x, y, freq);

mf_unlock:
	flags->ReleaseFlagData(arena);
}


local helptext_t reloadconf_help =
"Targets: none\n"
"Args: none\n"
"Causes the server to check all config files for modifications since\n"
"they were last loaded, and reload any modified files.\n";

local void Creloadconf(const char *tc, const char *params, Player *p, const Target *target)
{
	cfg->CheckModifiedFiles();
	chat->SendMessage(p, "Reloading all modified config files");
}



local helptext_t jackpot_help =
"Targets: none\n"
"Args: none or <arena name> or {all}\n"
"Displays the current jackpot for this arena, the named arena, or all arenas.\n";

local void Cjackpot(const char *tc, const char *params, Player *p, const Target *target)
{
	if (!strcasecmp(params, "all"))
	{
		Arena *arena;
		Link *link;
		int jp, seehid = capman->HasCapability(p, CAP_SEEPRIVARENA);

		aman->Lock();
		FOR_EACH_ARENA(arena)
			if (arena->status == ARENA_RUNNING &&
			    (arena->name[0] != '#' || seehid || p->arena == arena) &&
			    (jp = jackpot->GetJP(arena)) > 0)
				chat->SendMessage(p, "jackpot in %s: %d", arena->name, jp);
		aman->Unlock();
	}
	else if (*params)
	{
		Arena *arena = aman->FindArena(params, NULL, NULL);
		if (arena)
			chat->SendMessage(p, "jackpot in %s: %d", arena->name, jackpot->GetJP(arena));
		else
			chat->SendMessage(p, "arena '%s' doesn't exist", params);
	}
	else
		chat->SendMessage(p, "jackpot: %d", jackpot->GetJP(p->arena));
}


local helptext_t setjackpot_help =
"Targets: none\n"
"Args: <new jackpot value>\n"
"Sets the jackpot for this arena to a new value.\n";

local void Csetjackpot(const char *tc, const char *params, Player *p, const Target *target)
{
	char *next;
	int new = strtol(params, &next, 0);

	if (next != params)
	{
		jackpot->SetJP(p->arena, new);
		chat->SendMessage(p, "jackpot: %d", jackpot->GetJP(p->arena));
	}
	else
		chat->SendMessage(p, "setjackpot: bad value");
}


local helptext_t uptime_help =
"Targets: none\n"
"Args: none\n"
"Displays how long the server has been running.\n";

local void Cuptime(const char *tc, const char *params, Player *p, const Target *target)
{
	ticks_t secs = TICK_DIFF(current_ticks(), startedat) / 100;
	int days, hours, mins;

	days = secs / 86400;
	secs %= 86400;
	hours = secs / 3600;
	secs %= 3600;
	mins = secs / 60;
	secs %= 60;

	chat->SendMessage(p, "uptime: %d days %d hours %d minutes %d seconds",
			days, hours, mins, secs);
}


/* lag commands */

local helptext_t lag_help =
"Targets: none or player\n"
"Args: none\n"
"Displays basic lag information about you or a target player.\n";

local void Clag(const char *tc, const char *params, Player *p, const Target *target)
{
	struct PingSummary pping, cping;
	struct PLossSummary ploss;
	Player *t = (target->type) == T_PLAYER ? target->u.p : p;

	lagq->QueryPPing(t, &pping);
	lagq->QueryCPing(t, &cping);
	lagq->QueryPLoss(t, &ploss);

	if (t == p)
		chat->SendMessage(p,
			"ping: s2c: %d (%d-%d) c2s: %d (%d-%d)  ploss: s2c: %.2f c2s: %.2f",
			cping.avg, cping.min, cping.max,
			pping.avg, pping.min, pping.max,
			100.0*ploss.s2c, 100.0*ploss.c2s);
	else
		chat->SendMessage(p,
			"%s: ping: s2c: %d (%d-%d) c2s: %d (%d-%d)  ploss: s2c: %.2f c2s: %.2f",
			t->name,
			cping.avg, cping.min, cping.max,
			pping.avg, pping.min, pping.max,
			100.0*ploss.s2c, 100.0*ploss.c2s);
}


local helptext_t laginfo_help =
"Targets: none or player\n"
"Args: none\n"
"Displays tons of lag information about a player.\n";

local void Claginfo(const char *tc, const char *params, Player *p, const Target *target)
{
	struct PingSummary pping, cping, rping;
	struct PLossSummary ploss;
	struct ReliableLagData rlag;
	Player *t = (target->type) == T_PLAYER ? target->u.p : p;

	lagq->QueryPPing(t, &pping);
	lagq->QueryCPing(t, &cping);
	lagq->QueryRPing(t, &rping);
	lagq->QueryPLoss(t, &ploss);
	lagq->QueryRelLag(t, &rlag);

	chat->SendMessage(p, "%s: s2c ping: %d %d (%d-%d) (reported by client)",
		t->name, cping.cur, cping.avg, cping.min, cping.max);
	chat->SendMessage(p, "%s: c2s ping: %d %d (%d-%d) (from position pkt times)",
		t->name, pping.cur, pping.avg, pping.min, pping.max);
	chat->SendMessage(p, "%s: rel ping: %d %d (%d-%d) (reliable ping)",
		t->name, rping.cur, rping.avg, rping.min, rping.max);
	chat->SendMessage(p, "%s: ploss: s2c: %.2f c2s: %.2f s2cwpn: %.2f",
		t->name, 100.0*ploss.s2c, 100.0*ploss.c2s, 100.0*ploss.s2cwpn);
	chat->SendMessage(p, "%s: reliable dups: %.2f%%  reliable resends: %.2f%%",
		t->name, 100.0*(double)rlag.reldups/(double)rlag.c2sn,
		100.0*(double)rlag.retries/(double)rlag.s2cn);
	chat->SendMessage(p, "%s: s2c slow: %d/%d  s2c fast: %d/%d",
		t->name, cping.s2cslowcurrent, cping.s2cslowtotal,
		cping.s2cfastcurrent, cping.s2cfasttotal);
}


local helptext_t laghist_help =
"Targets: none or player\n"
"Args: [{-r}]\n"
"Displays lag histograms. If a {-r} is given, do this histogram for\n"
"\"reliable\" latency instead of c2s pings.\n";

local void Claghist(const char *tc, const char *params, Player *p, const Target *target)
{
	/* FIXME: write this */
}


local helptext_t listarena_help =
"Targets: none\n"
"Args: <arena name>\n"
"Lists the players in the given arena.\n";

local void Clistarena(const char *tc, const char *params, Player *p, const Target *target)
{
	char text[850], *pos = text;
	int total = 0, playing = 0, donedots = 0;
	Arena *a;
	Player *p2;
	Link *link;

	if (params[0] == '#' && !capman->HasCapability(p, CAP_SEEPRIVARENA))
	{
		chat->SendMessage(p, "You don't have permission to view private arenas.");
		return;
	}

	if (params[0] == '\0')
		params = p->arena->name;

	a = aman->FindArena(params, NULL, NULL);
	if (!a)
	{
		chat->SendMessage(p, "Arena '%s' doesn't exist.", params);
		return;
	}

	pd->Lock();
	FOR_EACH_PLAYER(p2)
		if (p2->status == S_PLAYING && p2->arena == a)
		{
			total++;
			if (p2->p_ship != SHIP_SPEC)
				playing++;
			if ((pos - text) < (sizeof(text) - 10))
			{
				snprintf(pos, sizeof(text)-(pos-text), ", %s", p2->name);
				pos = pos + strlen(pos);
			}
			else if (!donedots)
			{
				strcpy(pos, ", ...");
				donedots = 1;
			}
		}
	pd->Unlock();

	chat->SendMessage(p, "Arena '%s': %d total, %d playing", a->name, total, playing);
	wrap_text(text+2, 80, ' ', send_msg_cb, p);
}


local helptext_t endinterval_help =
"Targets: none\n"
"Args: [-g] [-a <arena group name>] <interval name>\n"
"Causes the specified interval to be reset. If {-g} is specified, reset the interval\n"
"at the global scope. If {-a} is specified, use the named arena group. Otherwise, use\n"
"the current arena's scope. Interval names can be \"game\", \"reset\", or \"maprotation\".\n";

local void Cendinterval(const char *tc, const char *params, Player *p, const Target *target)
{
	char word[128];
	const char *tmp = NULL;
	int interval = -1, dasha = 0;
	char ag[MAXAGLEN] = { '\0' };

	while (strsplit(params, " \t", word, sizeof(word), &tmp))
		if (dasha)
		{
			astrncpy(ag, word, sizeof(ag));
			dasha = 0;
		}
		else if (!strcmp(word, "-g"))
			strcpy(ag, AG_PUBLIC);
		else if (!strcmp(word, "-a"))
			dasha = 1;
		else if (!strcmp(word, "game"))
			interval = INTERVAL_GAME;
		else if (!strcmp(word, "reset"))
			interval = INTERVAL_RESET;
		else if (!strcmp(word, "maprotation"))
			interval = INTERVAL_MAPROTATION;
		else
		{
			chat->SendMessage(p, "Bad argument: %s", word);
			return;
		}

	if (dasha)
	{
		chat->SendMessage(p, "You must specify an arena group name after -a.");
		return;
	}

	if (interval == -1)
	{
		chat->SendMessage(p, "You must specify an interval to reset.");
		return;
	}

	if (ag[0])
		persist->EndInterval(ag, NULL, interval);
	else if (p->arena)
		persist->EndInterval(NULL, p->arena, interval);
}


local helptext_t scorereset_help =
"Targets: none or player\n"
"Args: none\n"
"Resets your own score, or the target player's score.\n";

local void Cscorereset(const char *tc, const char *params, Player *p, const Target *target)
{
	/* for now, only reset INTERVAL_RESET scores, since those are
	 * the only ones that cont sees. */
	if (target->type == T_ARENA)
	{
		/* cfghelp: Misc:SelfScoreReset, arena, bool, def: 0
		 * Whether players can reset their own scores using ?scorereset.
		 */
		if (cfg->GetInt(p->arena->cfg, "Misc", "SelfScoreReset", 0))
		{
			stats->ScoreReset(p, INTERVAL_RESET);
			stats->SendUpdates();
		}
		else
			chat->SendMessage(p,
					"This arena doesn't allow you to reset your own scores.");
	}
	else if (target->type == T_PLAYER)
	{
		stats->ScoreReset(target->u.p, INTERVAL_RESET);
		stats->SendUpdates();
	}
}


local helptext_t mapinfo_help =
"Targets: none\n"
"Args: none\n"
"Displays some information about the map in this arena.\n";

local void mapinfo_count_rgns(void *clos, Region *reg)
{
	(*(int*)clos)++;
}

local void Cmapinfo(const char *tc, const char *params, Player *p, const Target *target)
{
	struct mapdata_memory_stats_t stats;
	const char *name, *vers, *mapc, *tsetc, *prog;
	char fname[128];
	int regs = 0;
	int hasfn = mapdata->GetMapFilename(p->arena, fname, sizeof(fname), NULL);

	chat->SendMessage(p, "loaded lvl file from '%s'", hasfn ? fname : "<nowhere>");

	name = mapdata->GetAttr(p->arena, "NAME");
	vers = mapdata->GetAttr(p->arena, "VERSION");
	mapc = mapdata->GetAttr(p->arena, "MAPCREATOR");
	tsetc = mapdata->GetAttr(p->arena, "TILESETCREATOR");
	prog = mapdata->GetAttr(p->arena, "PROGRAM");

	chat->SendMessage(p,
			"name: %s  "
			"version: %s",
			name ? name : "<not set>",
			vers ? vers : "<not set>");
	chat->SendMessage(p,
			"map creator: %s  "
			"tileset creator: %s  "
			"program: %s",
			mapc ? mapc : "<not set>",
			tsetc ? tsetc : "<not set>",
			prog ? prog : "<not set>");

	mapdata->EnumContaining(p->arena, -1, -1, mapinfo_count_rgns, &regs);
	chat->SendMessage(p, "regions: %d", regs);

	mapdata->GetMemoryStats(p->arena, &stats);
	chat->SendMessage(p, "memory: lvl=%ld/%ld  rgn=%ld/%ld",
			stats.lvlbytes, stats.lvlblocks, stats.rgnbytes, stats.rgnblocks);
}



/* command group system */

/* declarations */
struct cmd_info
{
	const char *cmdname;
	CommandFunc func;
	helptext_t *phelptext;
};

struct interface_info
{
	void **ptr;
	const char *iid;
};

struct cmd_group
{
	const char *groupname;
	const struct interface_info *ifaces;
	const struct cmd_info *cmds;
	int loaded;
};


/* loading/unloading funcs */

local int load_cmd_group(struct cmd_group *grp)
{
	const struct interface_info *ii;
	const struct cmd_info *ci;

	for (ii = grp->ifaces; ii->iid; ii++)
	{
		*(ii->ptr) = mm->GetInterface(ii->iid, ALLARENAS);
		if (*(ii->ptr) == NULL)
		{
			/* if we can't get one, roll back all the others */
			for (ii--; ii >= grp->ifaces; ii--)
				mm->ReleaseInterface(*(ii->ptr));
			return MM_FAIL;
		}
	}

	for (ci = grp->cmds; ci->cmdname; ci++)
		cmd->AddCommand(ci->cmdname, ci->func, ALLARENAS, *ci->phelptext);

	grp->loaded = 1;

	return MM_OK;
}

local void unload_cmd_group(struct cmd_group *grp)
{
	const struct interface_info *ii;
	const struct cmd_info *ci;

	if (!grp->loaded)
		return;

	for (ii = grp->ifaces; ii->iid; ii++)
		mm->ReleaseInterface(*(ii->ptr));

	for (ci = grp->cmds; ci->cmdname; ci++)
		cmd->RemoveCommand(ci->cmdname, ci->func, ALLARENAS);

	grp->loaded = 0;
}

/* loading/unloading commands */

local struct cmd_group *find_group(const char *name);

local helptext_t enablecmdgroup_help =
"Targets: none\n"
"Args: <command group>\n"
"Enables all the commands in the specified command group. This is only\n"
"useful after using ?disablecmdgroup.\n";

local void Cenablecmdgroup(const char *tc, const char *params, Player *p, const Target *target)
{
	struct cmd_group *grp = find_group(params);
	if (grp)
	{
		if (grp->loaded)
			chat->SendMessage(p, "Command group %s already enabled", params);
		else if (load_cmd_group(grp) == MM_OK)
			chat->SendMessage(p, "Command group %s enabled", params);
		else
			chat->SendMessage(p, "Error enabling command group %s", params);
	}
	else
		chat->SendMessage(p, "Command group %s not found", params);
}


local helptext_t disablecmdgroup_help =
"Targets: none\n"
"Args: <command group>\n"
"Disables all the commands in the specified command group and released the\n"
"modules that they require. This can be used to release interfaces so that\n"
"modules can be unloaded or upgraded without unloading playercmd (which would\n"
"be irreversable).\n";

local void Cdisablecmdgroup(const char *tc, const char *params, Player *p, const Target *target)
{
	struct cmd_group *grp = find_group(params);
	if (grp)
	{
		if (grp->loaded)
		{
			unload_cmd_group(grp);
			chat->SendMessage(p, "Command group %s disabled", params);
		}
		else
			chat->SendMessage(p, "Command group %s not loaded", params);
	}
	else
		chat->SendMessage(p, "Command group %s not found", params);
}


local helptext_t owner_help =
"Targets: none\n"
"Args: none\n"
"Displays the arena owner.\n";

local void Cowner(const char *tc, const char *params, Player *p, const Target *target)
{
	const char *owner_str;

	/* get the name from the arena conf that this player is in */
	owner_str = cfg->GetStr(p->arena->cfg, "Owner", "Name");

	chat->SendMessage(p, "arena owner: %s", owner_str ? owner_str : "none");
}


/* actual group definitions */

#define CMD(x) {#x, C ## x, & x ## _help},
#define CMD_GROUP(x) {#x, x ## _requires, x ## _commands, 0},
#define REQUIRE(name, iid) {(void**)&name, iid},
#define END() {0}

local const struct interface_info core_requires[] =
{
	REQUIRE(aman, I_ARENAMAN)
	REQUIRE(net, I_NET)
	REQUIRE(ml, I_MAINLOOP)
	END()
};
local const struct cmd_info core_commands[] =
{
	CMD(enablecmdgroup)
	CMD(disablecmdgroup)
	CMD(arena)
	CMD(shutdown)
	CMD(owner)
	CMD(version)
	CMD(uptime)
	CMD(lsmod)
	CMD(modinfo)
#ifndef CFG_NO_RUNTIME_LOAD
	CMD(insmod)
#endif
	CMD(rmmod)
	CMD(attmod)
	CMD(info)
	CMD(a)
	CMD(aa)
	CMD(z)
	CMD(az)
	CMD(cheater)
	CMD(warn)
	CMD(netstats)
	CMD(send)
	CMD(recyclearena)
	END()
};


local const struct interface_info game_requires[] =
{
	REQUIRE(net, I_NET)
	REQUIRE(aman, I_ARENAMAN)
	REQUIRE(game, I_GAME)
	REQUIRE(cfg, I_CONFIG)
	END()
};
local const struct cmd_info game_commands[] =
{
	CMD(setfreq)
	CMD(setship)
	CMD(specall)
	CMD(warpto)
	CMD(shipreset)
	CMD(prize)

	CMD(lock)
	CMD(unlock)
	CMD(lockarena)
	CMD(unlockarena)

	END()
};


local const struct interface_info jackpot_requires[] =
{
	REQUIRE(aman, I_ARENAMAN)
	REQUIRE(jackpot, I_JACKPOT)
	END()
};
local const struct cmd_info jackpot_commands[] =
{
	CMD(jackpot)
	CMD(setjackpot)
	END()
};


local const struct interface_info config_requires[] =
{
	REQUIRE(aman, I_ARENAMAN)
	REQUIRE(cfg, I_CONFIG)
	END()
};
local const struct cmd_info config_commands[] =
{
	CMD(setg)
	CMD(getg)
	CMD(seta)
	CMD(geta)
	CMD(reloadconf)
	END()
};


local const struct interface_info flag_requires[] =
{
	REQUIRE(flags, I_FLAGS)
	END()
};
local const struct cmd_info flag_commands[] =
{
	CMD(flagreset)
	CMD(flaginfo)
	CMD(neutflag)
	CMD(moveflag)
	END()
};


local const struct interface_info ball_requires[] =
{
	REQUIRE(balls, I_BALLS)
	END()
};
local const struct cmd_info ball_commands[] =
{
	CMD(ballcount)
	END()
};


local const struct interface_info lag_requires[] =
{
	REQUIRE(lagq, I_LAGQUERY)
	END()
};
local const struct cmd_info lag_commands[] =
{
	CMD(lag)
	CMD(laginfo)
	CMD(laghist)
	END()
};


local const struct interface_info stats_requires[] =
{
	REQUIRE(persist, I_PERSIST)
	REQUIRE(stats, I_STATS)
	END()
};

local const struct cmd_info stats_commands[] =
{
	CMD(scorereset)
	CMD(endinterval)
	END()
};


local const struct interface_info misc_requires[] =
{
	REQUIRE(capman, I_CAPMAN)
	REQUIRE(groupman, I_GROUPMAN)
	REQUIRE(aman, I_ARENAMAN)
	REQUIRE(lm, I_LOGMAN)
	REQUIRE(cfg, I_CONFIG)
	REQUIRE(mapdata, I_MAPDATA)
	END()
};
local const struct cmd_info misc_commands[] =
{
	CMD(getgroup)
	CMD(setgroup)
	CMD(rmgroup)
	CMD(grplogin)
	CMD(listmod)
	CMD(find)
	CMD(setcm)
	CMD(getcm)
	CMD(listarena)
	CMD(sheep)
	CMD(mapinfo)
	END()
};


/* list of groups */
local struct cmd_group all_cmd_groups[] =
{
	CMD_GROUP(core)
	CMD_GROUP(game)
	CMD_GROUP(jackpot)
	CMD_GROUP(config)
	CMD_GROUP(flag)
	CMD_GROUP(ball)
	CMD_GROUP(lag)
	CMD_GROUP(stats)
	CMD_GROUP(misc)
	END()
};

#undef CMD
#undef CMD_GROUP
#undef REQUIRE
#undef END

struct cmd_group *find_group(const char *name)
{
	struct cmd_group *grp;
	for (grp = all_cmd_groups; grp->groupname; grp++)
		if (!strcasecmp(grp->groupname, name))
			return grp;
	return NULL;
}



EXPORT int MM_playercmd(int action, Imodman *mm_, Arena *arena)
{
	struct cmd_group *grp;

	if (action == MM_LOAD)
	{
		mm = mm_;
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);

		if (!pd || !chat || !cmd) return MM_FAIL;

		startedat = current_ticks();

		for (grp = all_cmd_groups; grp->groupname; grp++)
			load_cmd_group(grp);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		for (grp = all_cmd_groups; grp->groupname; grp++)
			unload_cmd_group(grp);

		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(chat);
		mm->ReleaseInterface(cmd);
		return MM_OK;
	}
	return MM_FAIL;
}




