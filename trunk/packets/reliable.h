
/* dist: public */

#ifndef __PACKETS_RELIABLE_H
#define __PACKETS_RELIABLE_H

/* reliable.h - reliable udp packets */

struct ReliablePacket
{
	u8 t1;
	u8 t2;
	i32 seqnum;
	byte data[MAXPACKET-6];
};

#endif

