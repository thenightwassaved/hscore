
/* dist: public */

#ifndef __PACKETS_SHIPCHANGE_h
#define __PACKETS_SHIPCHANGE_h


struct ShipChangePacket
{
	u8 type;
	i8 shiptype;
	i16 pnum;
	i16 freq;
};

#endif

