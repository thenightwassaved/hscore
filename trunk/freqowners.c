
/* dist: public */

#include "asss.h"


/* the delay (in ticks) between a ?freqkick and its effect */
#define KICKDELAY 300

/* commands */
local void Cgiveowner(const char *, const char *, Player *, const Target *);
local void Cfreqkick(const char *, const char *, Player *, const Target *);
local helptext_t giveowner_help, freqkick_help;

/* callbacks */
local void MyPA(Player *p, int action, Arena *arena);
local void MyFreqCh(Player *p, int newfreq);
local void MyShipCh(Player *p, int newship, int newfreq);

/* data */
local int ofkey;
#define OWNSFREQ(p) (*(char*)PPDATA(p, ofkey))

local Imainloop *ml;
local Iplayerdata *pd;
local Iarenaman *aman;
local Igame *game;
local Icmdman *cmd;
local Iconfig *cfg;
local Ichat *chat;
local Imodman *mm;


EXPORT int MM_freqowners(int action, Imodman *mm_, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = mm_;
		ml = mm->GetInterface(I_MAINLOOP, ALLARENAS);
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		game = mm->GetInterface(I_GAME, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		chat = mm->GetInterface(I_CHAT, ALLARENAS);
		if (!ml || !pd || !aman || !game || !cmd || !cfg) return MM_FAIL;

		ofkey = pd->AllocatePlayerData(sizeof(char));
		if (ofkey == -1) return MM_FAIL;

		mm->RegCallback(CB_PLAYERACTION, MyPA, ALLARENAS);
		mm->RegCallback(CB_FREQCHANGE, MyFreqCh, ALLARENAS);
		mm->RegCallback(CB_SHIPCHANGE, MyShipCh, ALLARENAS);

		cmd->AddCommand("giveowner", Cgiveowner, ALLARENAS, giveowner_help);
		cmd->AddCommand("freqkick", Cfreqkick, ALLARENAS, freqkick_help);

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		cmd->RemoveCommand("giveowner", Cgiveowner, ALLARENAS);
		cmd->RemoveCommand("freqkick", Cfreqkick, ALLARENAS);
		mm->UnregCallback(CB_PLAYERACTION, MyPA, ALLARENAS);
		mm->UnregCallback(CB_FREQCHANGE, MyFreqCh, ALLARENAS);
		mm->UnregCallback(CB_SHIPCHANGE, MyShipCh, ALLARENAS);
		pd->FreePlayerData(ofkey);
		mm->ReleaseInterface(ml);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(game);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(chat);
		return MM_OK;
	}
	return MM_FAIL;
}


local void count_freq(Arena *arena, int freq, Player *excl, int *total, int *hasowner)
{
	int t = 0;
	Player *i;
	Link *link;

	*hasowner = FALSE;
	pd->Lock();
	FOR_EACH_PLAYER(i)
		if (i->arena == arena &&
		    i->p_freq == freq &&
		    i != excl)
		{
			t++;
			if (OWNSFREQ(i))
				*hasowner = TRUE;
		}
	pd->Unlock();
	*total = t;
}


local int kick_timer(void *param)
{
	Player *t = param;
	chat->SendMessage(t, "You have been kicked off your freq.");
	game->SetFreqAndShip(t, SHIP_SPEC, t->arena->specfreq);
	return FALSE;
}


local helptext_t giveowner_help =
"Module: freqowners\n"
"Targets: player\n"
"Args: none\n"
"Allows you to share freq ownership with another player on your current\n"
"private freq. You can't remove ownership once you give it out, but you\n"
"are safe from being kicked off yourself, as long as you have ownership.\n";

void Cgiveowner(const char *tc, const char *params, Player *p, const Target *target)
{
	if (target->type != T_PLAYER)
		return;

	if (OWNSFREQ(p) &&
	    p->arena == target->u.p->arena &&
	    p->p_freq == target->u.p->p_freq)
		OWNSFREQ(target->u.p) = 1;
}


local helptext_t freqkick_help =
"Module: freqowners\n"
"Targets: player\n"
"Args: none\n"
"Kicks the player off of your freq. The player must be on your freq and\n"
"must not be an owner himself. The player giving the command, of course,\n"
"must be an owner.\n";

void Cfreqkick(const char *tc, const char *params, Player *p, const Target *target)
{
	Player *t = target->u.p;

	if (target->type != T_PLAYER)
		return;

	if (OWNSFREQ(p) && !OWNSFREQ(t) &&
	    p->arena == t->arena &&
	    p->p_freq == t->p_freq)
		ml->SetTimer(kick_timer, KICKDELAY, 0, t, t);
}


void MyPA(Player *p, int action, Arena *arena)
{
	if (action == PA_ENTERARENA || action == PA_LEAVEARENA)
	{
		OWNSFREQ(p) = 0;
		ml->ClearTimer(kick_timer, p);
	}
}


void MyFreqCh(Player *p, int newfreq)
{
	Arena *arena = p->arena;
	ConfigHandle ch = arena->cfg;

	OWNSFREQ(p) = 0;
	ml->ClearTimer(kick_timer, p);

	/* cfghelp: Team:AllowFreqOwners, arena, bool, def: 1
	 * Whether to enable the freq ownership feature in this arena. */
	if (cfg->GetInt(ch, "Team", "AllowFreqOwners", 1) &&
	    newfreq >= cfg->GetInt(ch, "Team", "PrivFreqStart", 100) &&
	    newfreq != arena->specfreq)
	{
		int total, hasowner;
		count_freq(arena, newfreq, p, &total, &hasowner);

		if (total == 0)
		{
			OWNSFREQ(p) = 1;
			chat->SendMessage(p, "You are the now the owner of freq %d. "
					"You can kick people off your freq by sending them "
					"the private message \"?freqkick\", and you can share "
					"your ownership by sending people \"?giveowner\".", newfreq);
		}
		else if (hasowner)
			chat->SendMessage(p, "This freq has an owner. You should be aware "
					"that you can be kicked off any time.");
	}
}


void MyShipCh(Player *p, int newship, int newfreq)
{
	MyFreqCh(p, newfreq);
}


