
/* dist: public */

#ifndef __BALLS_H
#define __BALLS_H

/* Iballs
 * this module will handle all ball-related network communication.
 */


typedef enum
{
	BALL_NONE,    /* the ball doesn't exist */
	BALL_ONMAP,   /* the ball is on the map or has been fired */
	BALL_CARRIED, /* the ball is being carried */
	BALL_WAITING  /* the ball is waiting to be spawned again */
} ballstate_t;


/* called when a player picks up a ball */
#define CB_BALLPICKUP "ballpickup"
typedef void (*BallPickupFunc)(Arena *arena, Player *p, int bid);
/* pycb: arena, player, int */

/* called when a player fires a ball */
#define CB_BALLFIRE "ballfire"
typedef void (*BallFireFunc)(Arena *arena, Player *p, int bid);
/* pycb: arena, player, int */

/* called when a player scores a goal */
#define CB_GOAL "goal"
typedef void (*GoalFunc)(Arena *arena, Player *p, int bid, int x, int y);
/* pycb: arena, player, int, int, int */


struct BallData
{
	ballstate_t state; /* the state of this ball */
	int x, y, xspeed, yspeed; /* the coordinates of the ball */
	Player *carrier; /* the player that is carrying or last touched the ball */
	int freq; /* freq of carrier */
	ticks_t time; /* the time that the ball was last fired (will be 0 for
	               * balls being held). for BALL_WAITING, this time is the
	               * time when the ball will be re-spawned. */
};

typedef struct ArenaBallData
{
	int ballcount;
	/* the number of balls currently in play. 0 if the arena has no ball
	 * game. */
	struct BallData *balls;
	/* points to an array of at least ballcount structs */
} ArenaBallData;


#define I_BALLS "balls-3"

typedef struct Iballs
{
	INTERFACE_HEAD_DECL

	/* pyint: use */

	void (*SetBallCount)(Arena *arena, int ballcount);
	/* sets the number of balls in the arena. if the new count is higher
	 * than the current one, new balls are spawned. if it's lower, the
	 * dead balls are "phased" in the upper left corner. */
	/* pyint: arena, int -> void  */

	void (*PlaceBall)(Arena *arena, int bid, struct BallData *newpos);
	/* sets the parameters of the ball to those in the given BallData
	 * struct */
	/* FIXMEpyint: arena, int, balldata -> void  */

	void (*EndGame)(Arena *arena);
	/* ends the ball game */
	/* pyint: arena -> void  */

	ArenaBallData * (*GetBallData)(Arena *arena);
	void (*ReleaseBallData)(Arena *arena);
	/* always release the ball data when you're done using it */
} Iballs;


#endif

