
/* dist: public */

#ifndef __BRICKS_H
#define __BRICKS_H

/* these should be mostly self-explanatory. */

#define I_BRICKS "bricks-1"

typedef struct Ibricks
{
	INTERFACE_HEAD_DECL
	/* pyint: use */

	void (*DropBrick)(Arena *arena, int freq, int x1, int y1, int x2, int y2);
	/* pyint: arena, int, int, int, int, int -> void */
} Ibricks;

#endif

