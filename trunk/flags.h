
/* dist: public */

#ifndef __FLAGS_H
#define __FLAGS_H

/* Iflags
 * this module will handle all flag-related network communication. it
 * implements the basic flag games. other flag games can be written as
 * extra modules.
 */

#include "settings/flaggames.h"

typedef enum
{
	FLAG_NONE,    /* the flag doesn't exist */
	FLAG_ONMAP,   /* the flag is dropped on the map */
	FLAG_CARRIED, /* the flag is being carried */
	FLAG_NEUTED   /* the flag carrier specced or left, and the flag
	               * hasn't been placed yet */
} flagstate_t;


/* called when a player picks up a flag (in turf games, this means he
 * claimed the flag) */
#define CB_FLAGPICKUP "flagpickup"
typedef void (*FlagPickupFunc)(Arena *arena, Player *p, int fid, int oldfreq, int carried);
/* pycb: arena, player, int, int, int */

/* called when a player drops his flags (regular games only) */
#define CB_FLAGDROP "flagdrop"
typedef void (*FlagDropFunc)(Arena *arena, Player *p, int count, int neut);
/* pycb: arena, player, int, int */

/* called when a flag is positioned on the map */
#define CB_FLAGPOS "flagpos"
typedef void (*FlagPosFunc)(Arena *arena, int fid, int x, int y, int freq);
/* pycb: arena, int, int, int, int */

/* called when a freq owns all the flags in an arena (in regular games
 * only) */
#define CB_FLAGWIN "flagwin"
typedef void (*FlagWinFunc)(Arena *arena, int freq);
/* pycb: arena, int */


struct FlagData
{
	flagstate_t state; /* the state of this flag */
	int x, y; /* the coordinates of the flag */
	int freq; /* the freq owning the flag, or -1 if neutral */
	Player *carrier; /* the pid carrying the flag, or NULL if down */
};

typedef struct ArenaFlagData
{
	int flagcount;
	/* the number of flags currently in play */
	struct FlagData *flags;
	/* points to an array of at least flagcount structs */
} ArenaFlagData;


#define I_FLAGS "flags-3"

typedef struct Iflags
{
	INTERFACE_HEAD_DECL
	/* pyint: use */

	void (*MoveFlag)(Arena *arena, int fid, int x, int y, int freq);
	/* moves the specified flag to the specified coordinates */
	/* pyint: arena, int, int, int, int -> void */

	void (*FlagVictory)(Arena *arena, int freq, int points);
	/* ends the flag game (freq=-1 to reset flags with no winner) */
	/* pyint: arena, int, int -> void */

	int (*GetCarriedFlags)(Player *p);
	/* a utility function to get the number of flags carried by a player */
	/* pyint: player -> int */

	int (*GetFreqFlags)(Arena *arena, int freq);
	/* a utility function to get the number of flags owned by a freq */
	/* pyint: arena, int -> int */

	ArenaFlagData * (*GetFlagData)(Arena *arena);
	void (*ReleaseFlagData)(Arena *arena);
	/* you must always release the flag data after using it */
} Iflags;


#endif

