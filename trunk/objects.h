
/* dist: public */

#ifndef __OBJECTS_H
#define __OBJECTS_H

/* Iobjects
 * this module will handle all object-related packets
 */

#define I_OBJECTS "objects-2"

typedef struct Iobjects
{
	INTERFACE_HEAD_DECL

	/* sends the current LVZ object state to a player */
	void (*SendState)(Player *p);

	/* if target is an arena, the defaults are changed */
	void (*Toggle)(const Target *t, short id, char on);
	void (*ToggleSet)(const Target *t, short *id, char *ons, int size);

	/* use last two parameters for rel_x, rel_y when it's a screen object */
	void (*Move)(const Target *t, short id, short x, short y, short rx, short ry);
	void (*Image)(const Target *t, short id, int image);
	void (*Layer)(const Target *t, short id, int layer);
	void (*Timer)(const Target *t, short id, int time);
	void (*Mode)(const Target *t, short id, int mode);
} Iobjects;

#endif

