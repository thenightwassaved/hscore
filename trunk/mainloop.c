
/* dist: public */

#ifndef WIN32
#include <unistd.h>
#endif

#include "asss.h"



typedef struct TimerData
{
	TimerFunc func;
	ticks_t interval, when;
	void *param;
	void *key;
} TimerData;



local void StartTimer(TimerFunc, int, int, void *, void *);
local void ClearTimer(TimerFunc, void *);
local void CleanupTimer(TimerFunc func, void *key, CleanupFunc cleanup);

local int RunLoop(void);
local void KillML(int code);



local Imainloop _int =
{
	INTERFACE_HEAD_INIT(I_MAINLOOP, "mainloop")
	StartTimer, ClearTimer, CleanupTimer, RunLoop, KillML
};

local int privatequit;
local LinkedList timers;
local Imodman *mm;

local pthread_mutex_t tmrmtx = PTHREAD_MUTEX_INITIALIZER;
#define LOCK() pthread_mutex_lock(&tmrmtx)
#define UNLOCK() pthread_mutex_unlock(&tmrmtx)


EXPORT int MM_mainloop(int action, Imodman *mm_, Arena *arena)
{
	if (action == MM_LOAD)
	{
		mm = mm_;
		privatequit = 0;
		LLInit(&timers);
		mm->RegInterface(&_int, ALLARENAS);
		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		LLEnum(&timers, afree);
		LLEmpty(&timers);
		if (mm->UnregInterface(&_int, ALLARENAS))
			return MM_FAIL;
		return MM_OK;
	}
	return MM_FAIL;
}


int RunLoop(void)
{
	TimerData *td;
	Link *l;
	ticks_t gtc;

	while (!privatequit)
	{
		/* call all funcs */
		DO_CBS(CB_MAINLOOP, ALLARENAS, MainLoopFunc, ());

		/* do timers */
		LOCK();
startover:
		gtc = current_ticks();
		for (l = LLGetHead(&timers); l; l = l->next)
		{
			td = (TimerData*) l->data;
			if (td->func && TICK_GT(gtc, td->when))
			{
				int ret;
				UNLOCK();
				ret = td->func(td->param);
				LOCK();
				if (ret)
					td->when = gtc + td->interval;
				else
				{
					LLRemove(&timers, td);
					afree(td);
				}
				goto startover;
			}
		}
		UNLOCK();

		/* rest a bit: 1/100 sec */
#ifndef WIN32
		{
			struct timespec ts = { 0, 10000000 };
			while (nanosleep(&ts, &ts) == -1) /* retry */;
		}
#else
		usleep(10000);
#endif
	}

	return privatequit & 0xff;
}


void KillML(int code)
{
	privatequit = 0x100 | (code & 0xff);
}


void StartTimer(TimerFunc f, int startint, int interval, void *param, void *key)
{
	TimerData *data = amalloc(sizeof(TimerData));

	data->func = f;
	data->interval = interval;
	data->when = TICK_MAKE(current_ticks() + startint);
	data->param = param;
	data->key = key;
	LOCK();
	LLAdd(&timers, data);
	UNLOCK();
}


void CleanupTimer(TimerFunc func, void *key, CleanupFunc cleanup)
{
	Link *l, *next;

	LOCK();
	for (l = LLGetHead(&timers); l; l = next)
	{
		TimerData *td = l->data;
		next = l->next;

		if (td->func == func && (td->key == key || key == NULL))
		{
			if (cleanup)
				cleanup(td->param);
			LLRemove(&timers, td);
			afree(td);
		}
	}
	UNLOCK();
}

void ClearTimer(TimerFunc f, void *key)
{
	CleanupTimer(f, key, NULL);
}

