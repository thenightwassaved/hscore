
/* dist: public */

#ifndef __PACKETS_GREEN_H
#define __PACKETS_GREEN_H

struct GreenPacket
{
	u8 type;
	u32 time;
	i16 x;
	i16 y;
	i16 green;
	i16 pid;
};


#endif

