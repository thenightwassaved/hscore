
/* dist: public */

#ifndef WIN32
#include <unistd.h>
#endif
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "asss.h"

local pthread_t thd;
local volatile int counter;

local void * thread_check(void *dummy)
{
	for (;;)
	{
		int seen = counter;
#ifndef WIN32
		struct timespec ts = { 10, 0 };
		/* this works correctly even when interrupted by signals */
		while (nanosleep(&ts, &ts) == -1)
			;
#else
		/* not sure how to do an accurate sleep on windows */
		sleep(10);
#endif
		if (counter == seen)
		{
			fprintf(stderr, "E <deadlock> deadlock detected, aborting\n");
			abort();
		}
	}
}

local void increment(void)
{
	counter++;
}

EXPORT int MM_deadlock(int action, Imodman *mm, Arena *arena)
{
	if (action == MM_LOAD)
	{
		pthread_create(&thd, NULL, thread_check, NULL);
		mm->RegCallback(CB_MAINLOOP, increment, ALLARENAS);
		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		mm->UnregCallback(CB_MAINLOOP, increment, ALLARENAS);
		pthread_cancel(thd);
		pthread_join(thd, NULL);
		return MM_OK;
	}
	return MM_FAIL;
}

