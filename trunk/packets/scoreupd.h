
/* dist: public */

#ifndef __PACKETS_SCOREUPD_H
#define __PACKETS_SCOREUPD_H

struct ScorePacket
{
	u8 type;
	i16 pid;
	i32 killpoints;
	i32 flagpoints;
	i16 kills;
	i16 deaths;
};

#endif

