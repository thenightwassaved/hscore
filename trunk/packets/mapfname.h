
/* dist: public */

#ifndef __PACKETS_MAPFNAME_H
#define __PACKETS_MAPFNAME_H

/* mapfname.h - map filename packet */


struct MapFilename
{
	u8 type;
	struct
	{
		char filename[16];
		u32 checksum;
		u32 size; /* cont only */
	} files[1];
};

#endif

