
/* dist: public */

#ifndef __GAME_H
#define __GAME_H

#include "mapdata.h"

/** @file
 * various functions and interfaces related to the game. important stuff
 * managed by this module includes ships and freqs, kills, position
 * packet routing, and prizes.
 */

/** this callback is be called whenever a kill occurs */
#define CB_KILL "kill-2"
/** the type of CB_KILL.
 * @param arena the arena the kill took place in
 * @param killer the player who made the kill
 * @param killed the player who got killed
 * @param bounty the number displayed in the kill message (not
 * necessarily equal to the killed->position.bounty)
 * @param pts a pointer to the total number of points this kill will be
 * worth. callbacks can do "*pts += x" to add points.
 * @param green a pointer to the kill green that will be placed for this
 * death
 */
typedef void (*KillFunc)(Arena *arena, Player *killer, Player *killed,
		int bounty, int flags, int *pts, int *green);
/* FIXMEpycb: arena, player, player, int, int, int inout */


/** this callback is called when a player changes his freq (but stays in
 * the same ship). */
#define CB_FREQCHANGE "freqchange"
/** the type of CB_FREQCHANGE
 * @param p the player changing freq
 * @param newfreq the freq he's changing to
 */
typedef void (*FreqChangeFunc)(Player *p, int newfreq);
/* pycb: player, int */


/** this callback is be called when a player changes ship. */
#define CB_SHIPCHANGE "shipchange"
/** the type of CB_SHIPCHANGE
 * @param p the player changing ship/freq
 * @param newship the ship he's changing to
 * @param newfreq the freq he's changing to
 */
typedef void (*ShipChangeFunc)(Player *p, int newship, int newfreq);
/* pycb: player, int, int */


/** this callback called when the game timer expires. */
#define CB_TIMESUP "timesup"
/** the type of CB_TIMESUP
 * @param arena the arena whose timer is ending
 */
typedef void (*GameTimerFunc)(Arena *arena);
/* pycb: arena */


/** this callback is called when someone enters or leaves a safe zone.
 * be careful about what you do in here; this runs in the unreliable
 * packet handler thread (for now). */
#define CB_SAFEZONE "safezone"
/** the type of CB_SAFEZONE
 * @param p the player entering/leaving the safe zone
 * @param x the player's location in pixels, x coordinate
 * @param y the player's location in pixels, y coordinate
 * @param entering true if entering, false if exiting
 */
typedef void (*SafeZoneFunc)(Player *p, int x, int y, int entering);
/* pycb: player, int, int, int */


/** this callback is called when someone enters or leaves a region. */
#define CB_REGION "region-1"
/** the type of CB_REGION
 * @param p the player entering/leaving the region
 * @param rgn the region being entered or left
 * @param x the player's location in pixels, x coordinate
 * @param y the player's location in pixels, y coordinate
 * @param entering true if entering, false if exiting
 */
typedef void (*RegionFunc)(Player *p, Region *rgn, int x, int y, int entering);
/* NOTYETpycb: player, void, int, int, int */


/** this callback is called whenever someone picks up a green. */
#define CB_GREEN "green-1"
/** the type of CB_GREEN
 * @param p the player who picked up the green
 * @param x the x coordinate of the player in tiles
 * @param y the y coordinate of the player in tiles
 * @param prize the type of green picked up
 */
typedef void (*GreenFunc)(Player *p, int x, int y, int prize);
/* pycb: player, int, int, int */


/** the game interface id */
#define I_GAME "game-6"

/** the game interface struct */
typedef struct Igame
{
	INTERFACE_HEAD_DECL
	/* pyint: use */

	/** Changes a player's freq.
	 * This is an unconditional change; it doesn't go through the freq
	 * manager.
	 * @param p the player to change
	 * @param freq the freq to change to
	 */
	void (*SetFreq)(Player *p, int freq);
	/* pyint: player, int -> void */

	/** Changes a player's ship.
	 * This is an unconditional change; it doesn't go through the freq
	 * manager.
	 * @param p the player to change
	 * @param ship the freq to change to
	 */
	void (*SetShip)(Player *p, int ship);
	/* pyint: player, int -> void */

	/** Changes a player's ship and freq together.
	 * This is an unconditional change; it doesn't go through the freq
	 * manager.
	 * @param p the player to change
	 * @param ship the ship to change to
	 * @param freq the freq to change to
	 */
	void (*SetFreqAndShip)(Player *p, int ship, int freq);
	/* pyint: player, int, int -> void */

	/** Moves a set of playes to a specific location.
	 * This uses the Continuum warp feature, so it causes the little
	 * flashy thing, and the affected ships won't be moving afterwards.
	 * @see Target
	 * @param target the things to warp
	 * @param x the destination of the warp in tiles, x coordinate
	 * @param y the destination of the warp in tiles, y coordinate
	 */
	void (*WarpTo)(const Target *target, int x, int y);
	/* pyint: target, int, int -> void */

	/** Gives out prizes to a set of players.
	 * @param target the things to give prizes to
	 * @param type the type of the prizes to give, or 0 for random
	 * @param count the number of prizes to give
	 */
	void (*GivePrize)(const Target *target, int type, int count);
	/* pyint: target, int, int -> void */

	/** Locks a set of players to their ship and freq.
	 * Note that this doesn't modify the default lock state for the
	 * arena, so this will not affect newly entering players at all. Use
	 * LockArena for that.
	 * @param t the players to lock
	 * @param notify whether to notify the affected players (with an
	 * arena message) if their lock status has changed
	 * @param spec whether to spec them before locking them to their
	 * ship/freq
	 * @param timeout the number of sections the lock should be
	 * effectice for, or zero for indefinite.
	 */
	void (*Lock)(const Target *t, int notify, int spec, int timeout);
	/* pyint: target, int, int, int -> void */

	/** Undoes the effect of Lock, allowing players to change ship and
	 ** freq again.
	 * Again, note that this doesn't affect the default lock state for
	 * the arena.
	 * @param t the players to unlock
	 * @param notify whether to notify the affected players if their
	 * lock status changed
	 */
	void (*Unlock)(const Target *t, int notify);
	/* pyint: target, int -> void */

	/** Locks all players in the arena to spectator mode, or to their
	 ** current ships.
	 * This modifies the arena lock state, and also has the effect of
	 * calling Lock on all the players in the arena.
	 * @param a the arena to apply changes to
	 * @param notify whether to notify affected players of their change
	 * in state
	 * @param onlyarenastate whether to apply changes to the default
	 * arena lock state only, and not change the state of current
	 * players
	 * @param initial whether entering players are only locked to their
	 * inital ships, rather than being forced into spectator mode and
	 * then being locked
	 * @param spec whether to force all current players into spec before
	 * locking them
	 */
	void (*LockArena)(Arena *a, int notify, int onlyarenastate, int initial, int spec);
	/* pyint: arena, int, int, int, int -> void */

	/** Undoes the effect of LockArena by changing the arena lock state
	 ** and unlocking current players.
	 * @param a the arena to apply changes to
	 * @param notify whether to notify affected players of their change
	 * in state
	 * @param onlyarenastate whether to apply changes to the default
	 */
	void (*UnlockArena)(Arena *a, int notify, int onlyarenastate);
	/* pyint: arena, int, int -> void */

	void (*FakePosition)(Player *p, struct C2SPosition *pos, int len);
	void (*FakeKill)(Player *killer, Player *killed, int pts, int flags);

	double (*GetIgnoreWeapons)(Player *p);
	void (*SetIgnoreWeapons)(Player *p, double proportion);
} Igame;


#endif

