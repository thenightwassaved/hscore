
/* dist: public */

#ifndef __PACKETS_TIMESYNC_H
#define __PACKETS_TIMESYNC_H

/* timesync.h - time sync packets */

struct TimeSyncS2C
{
	i8 t1;
	i8 t2;
	u32 clienttime;
	u32 servertime;
};

struct TimeSyncC2S
{
	i8 t1;
	i8 t2;
	u32 time;
	u32 pktsent;
	u32 pktrecvd;
};

#endif

