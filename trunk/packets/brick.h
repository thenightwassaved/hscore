
/* dist: public */

#ifndef __PACKETS_BRICK_H
#define __PACKETS_BRICK_H


struct S2CBrickPacket
{
	u8 type; /* 0x21 */
	i16 x1;
	i16 y1;
	i16 x2;
	i16 y2;
	i16 freq;
	u16 brickid;
	u32 starttime;
};


#endif

