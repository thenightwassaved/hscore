
/* dist: public */

#ifndef __KOTH_H
#define __KOTH_H


#define I_POINTS_KOTH "points-koth-1"

typedef struct Ipoints_koth
{
	INTERFACE_HEAD_DECL
	/* pyint: use, impl */
	int (*GetPoints)(
			Arena *arena,
			int totalplaying,
			int winners);
	/* this will be called by the koth module when some group of people
	 * wins a round of koth. it should return the number of points to be
	 * given to each player. */
	/* pyint: arena, int, int -> int */
} Ipoints_koth;

#endif

