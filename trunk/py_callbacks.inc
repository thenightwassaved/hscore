
/* THIS IS AN AUTOMATICALLY GENERATED FILE */

local void py_cb_CB_BALLPICKUP(Arena *arg1_in, Player *arg2_in, int arg3_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&i)", cvt_c2p_arena, arg1_in, cvt_c2p_player, arg2_in, arg3_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_BALLPICKUP");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_BALLPICKUP, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_BALLPICKUP(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	Player *arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&O&i", cvt_p2c_arena, &arg1_in, cvt_p2c_player, &arg2_in, &arg3_in))
		return;
	DO_CBS(CB_BALLPICKUP, arena, BallPickupFunc, (arg1_in, arg2_in, arg3_in));
}

local void py_cb_CB_BALLFIRE(Arena *arg1_in, Player *arg2_in, int arg3_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&i)", cvt_c2p_arena, arg1_in, cvt_c2p_player, arg2_in, arg3_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_BALLFIRE");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_BALLFIRE, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_BALLFIRE(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	Player *arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&O&i", cvt_p2c_arena, &arg1_in, cvt_p2c_player, &arg2_in, &arg3_in))
		return;
	DO_CBS(CB_BALLFIRE, arena, BallFireFunc, (arg1_in, arg2_in, arg3_in));
}

local void py_cb_CB_GOAL(Arena *arg1_in, Player *arg2_in, int arg3_in, int arg4_in, int arg5_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&iii)", cvt_c2p_arena, arg1_in, cvt_c2p_player, arg2_in, arg3_in, arg4_in, arg5_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_GOAL");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_GOAL, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_GOAL(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	Player *arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	if (!PyArg_ParseTuple(args, "O&O&iii", cvt_p2c_arena, &arg1_in, cvt_p2c_player, &arg2_in, &arg3_in, &arg4_in, &arg5_in))
		return;
	DO_CBS(CB_GOAL, arena, GoalFunc, (arg1_in, arg2_in, arg3_in, arg4_in, arg5_in));
}

local void py_cb_CB_SET_BANNER(Player *arg1_in, Banner *arg2_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&)", cvt_c2p_player, arg1_in, cvt_c2p_banner, arg2_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_SET_BANNER");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_SET_BANNER, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_SET_BANNER(Arena *arena, PyObject *args)
{
	Player *arg1_in;
	Banner *arg2_in;
	if (!PyArg_ParseTuple(args, "O&O&", cvt_p2c_player, &arg1_in, cvt_p2c_banner, &arg2_in))
		return;
	DO_CBS(CB_SET_BANNER, arena, SetBannerFunc, (arg1_in, arg2_in));
}

local void py_cb_CB_CHATMSG(Player *arg1_in, int arg2_in, int arg3_in, Player *arg4_in, int arg5_in, const char *arg6_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&iiO&is)", cvt_c2p_player, arg1_in, arg2_in, arg3_in, cvt_c2p_player, arg4_in, arg5_in, arg6_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_CHATMSG");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_CHATMSG, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_CHATMSG(Arena *arena, PyObject *args)
{
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	Player *arg4_in;
	int arg5_in;
	const char *arg6_in;
	if (!PyArg_ParseTuple(args, "O&iiO&is", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in, cvt_p2c_player, &arg4_in, &arg5_in, &arg6_in))
		return;
	DO_CBS(CB_CHATMSG, arena, ChatMsgFunc, (arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in));
}

local void py_cb_CB_GLOBALCONFIGCHANGED(void)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("()");
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_GLOBALCONFIGCHANGED");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_GLOBALCONFIGCHANGED, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_GLOBALCONFIGCHANGED(Arena *arena, PyObject *args)
{

	if (!PyArg_ParseTuple(args, ""))
		return;
	DO_CBS(CB_GLOBALCONFIGCHANGED, arena, GlobalConfigChangedFunc, ());
}

local void py_cb_CB_PLAYERACTION(Player *arg1_in, int arg2_in, Arena *arg3_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&iO&)", cvt_c2p_player, arg1_in, arg2_in, cvt_c2p_arena, arg3_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_PLAYERACTION");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_PLAYERACTION, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_PLAYERACTION(Arena *arena, PyObject *args)
{
	Player *arg1_in;
	int arg2_in;
	Arena *arg3_in;
	if (!PyArg_ParseTuple(args, "O&iO&", cvt_p2c_player, &arg1_in, &arg2_in, cvt_p2c_arena, &arg3_in))
		return;
	DO_CBS(CB_PLAYERACTION, arena, PlayerActionFunc, (arg1_in, arg2_in, arg3_in));
}

local void py_cb_CB_FLAGPICKUP(Arena *arg1_in, Player *arg2_in, int arg3_in, int arg4_in, int arg5_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&iii)", cvt_c2p_arena, arg1_in, cvt_c2p_player, arg2_in, arg3_in, arg4_in, arg5_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_FLAGPICKUP");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_FLAGPICKUP, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_FLAGPICKUP(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	Player *arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	if (!PyArg_ParseTuple(args, "O&O&iii", cvt_p2c_arena, &arg1_in, cvt_p2c_player, &arg2_in, &arg3_in, &arg4_in, &arg5_in))
		return;
	DO_CBS(CB_FLAGPICKUP, arena, FlagPickupFunc, (arg1_in, arg2_in, arg3_in, arg4_in, arg5_in));
}

local void py_cb_CB_FLAGDROP(Arena *arg1_in, Player *arg2_in, int arg3_in, int arg4_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&ii)", cvt_c2p_arena, arg1_in, cvt_c2p_player, arg2_in, arg3_in, arg4_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_FLAGDROP");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_FLAGDROP, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_FLAGDROP(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	Player *arg2_in;
	int arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "O&O&ii", cvt_p2c_arena, &arg1_in, cvt_p2c_player, &arg2_in, &arg3_in, &arg4_in))
		return;
	DO_CBS(CB_FLAGDROP, arena, FlagDropFunc, (arg1_in, arg2_in, arg3_in, arg4_in));
}

local void py_cb_CB_FLAGPOS(Arena *arg1_in, int arg2_in, int arg3_in, int arg4_in, int arg5_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&iiii)", cvt_c2p_arena, arg1_in, arg2_in, arg3_in, arg4_in, arg5_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_FLAGPOS");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_FLAGPOS, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_FLAGPOS(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	if (!PyArg_ParseTuple(args, "O&iiii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in))
		return;
	DO_CBS(CB_FLAGPOS, arena, FlagPosFunc, (arg1_in, arg2_in, arg3_in, arg4_in, arg5_in));
}

local void py_cb_CB_FLAGWIN(Arena *arg1_in, int arg2_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&i)", cvt_c2p_arena, arg1_in, arg2_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_FLAGWIN");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_FLAGWIN, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_FLAGWIN(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_arena, &arg1_in, &arg2_in))
		return;
	DO_CBS(CB_FLAGWIN, arena, FlagWinFunc, (arg1_in, arg2_in));
}

local void py_cb_CB_FREQCHANGE(Player *arg1_in, int arg2_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&i)", cvt_c2p_player, arg1_in, arg2_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_FREQCHANGE");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_FREQCHANGE, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_FREQCHANGE(Arena *arena, PyObject *args)
{
	Player *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_player, &arg1_in, &arg2_in))
		return;
	DO_CBS(CB_FREQCHANGE, arena, FreqChangeFunc, (arg1_in, arg2_in));
}

local void py_cb_CB_SHIPCHANGE(Player *arg1_in, int arg2_in, int arg3_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&ii)", cvt_c2p_player, arg1_in, arg2_in, arg3_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_SHIPCHANGE");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_SHIPCHANGE, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_SHIPCHANGE(Arena *arena, PyObject *args)
{
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in))
		return;
	DO_CBS(CB_SHIPCHANGE, arena, ShipChangeFunc, (arg1_in, arg2_in, arg3_in));
}

local void py_cb_CB_TIMESUP(Arena *arg1_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&)", cvt_c2p_arena, arg1_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_TIMESUP");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_TIMESUP, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_TIMESUP(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return;
	DO_CBS(CB_TIMESUP, arena, GameTimerFunc, (arg1_in));
}

local void py_cb_CB_SAFEZONE(Player *arg1_in, int arg2_in, int arg3_in, int arg4_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&iii)", cvt_c2p_player, arg1_in, arg2_in, arg3_in, arg4_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_SAFEZONE");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_SAFEZONE, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_SAFEZONE(Arena *arena, PyObject *args)
{
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "O&iii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in, &arg4_in))
		return;
	DO_CBS(CB_SAFEZONE, arena, SafeZoneFunc, (arg1_in, arg2_in, arg3_in, arg4_in));
}

local void py_cb_CB_GREEN(Player *arg1_in, int arg2_in, int arg3_in, int arg4_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&iii)", cvt_c2p_player, arg1_in, arg2_in, arg3_in, arg4_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_GREEN");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_GREEN, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_GREEN(Arena *arena, PyObject *args)
{
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "O&iii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in, &arg4_in))
		return;
	DO_CBS(CB_GREEN, arena, GreenFunc, (arg1_in, arg2_in, arg3_in, arg4_in));
}

local void py_cb_CB_LOGFUNC(const char *arg1_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(s)", arg1_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_LOGFUNC");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_LOGFUNC, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_LOGFUNC(Arena *arena, PyObject *args)
{
	const char *arg1_in;
	if (!PyArg_ParseTuple(args, "s", &arg1_in))
		return;
	DO_CBS(CB_LOGFUNC, arena, LogFunc, (arg1_in));
}

local void py_cb_CB_INTERVAL_ENDED(void)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("()");
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_INTERVAL_ENDED");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_INTERVAL_ENDED, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_INTERVAL_ENDED(Arena *arena, PyObject *args)
{

	if (!PyArg_ParseTuple(args, ""))
		return;
	DO_CBS(CB_INTERVAL_ENDED, arena, EndIntervalFunc, ());
}

local void py_cb_CB_TURFTAG(Arena *arg1_in, Player *arg2_in, int arg3_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&i)", cvt_c2p_arena, arg1_in, cvt_c2p_player, arg2_in, arg3_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_TURFTAG");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_TURFTAG, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_TURFTAG(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	Player *arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&O&i", cvt_p2c_arena, &arg1_in, cvt_p2c_player, &arg2_in, &arg3_in))
		return;
	DO_CBS(CB_TURFTAG, arena, TurfTagFunc, (arg1_in, arg2_in, arg3_in));
}

local void py_cb_CB_TURFSTEAL(Arena *arg1_in, Player *arg2_in, int arg3_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&O&i)", cvt_c2p_arena, arg1_in, cvt_c2p_player, arg2_in, arg3_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_TURFSTEAL");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_TURFSTEAL, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_TURFSTEAL(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	Player *arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&O&i", cvt_p2c_arena, &arg1_in, cvt_p2c_player, &arg2_in, &arg3_in))
		return;
	DO_CBS(CB_TURFSTEAL, arena, TurfStealFunc, (arg1_in, arg2_in, arg3_in));
}

local void py_cb_CB_TURFRECOVER(Arena *arg1_in, int arg2_in, int arg3_in, int arg4_in, int arg5_in, int arg6_in, int arg7_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&iiiiii)", cvt_c2p_arena, arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in, arg7_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_TURFRECOVER");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_TURFRECOVER, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_TURFRECOVER(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	int arg6_in;
	int arg7_in;
	if (!PyArg_ParseTuple(args, "O&iiiiii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in, &arg6_in, &arg7_in))
		return;
	DO_CBS(CB_TURFRECOVER, arena, TurfRecoverFunc, (arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in, arg7_in));
}

local void py_cb_CB_TURFLOST(Arena *arg1_in, int arg2_in, int arg3_in, int arg4_in, int arg5_in, int arg6_in, int arg7_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&iiiiii)", cvt_c2p_arena, arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in, arg7_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_TURFLOST");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_TURFLOST, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_TURFLOST(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	int arg6_in;
	int arg7_in;
	if (!PyArg_ParseTuple(args, "O&iiiiii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in, &arg6_in, &arg7_in))
		return;
	DO_CBS(CB_TURFLOST, arena, TurfLostFunc, (arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in, arg7_in));
}

local void py_cb_CB_TURFVICTORY(Arena *arg1_in)
{
	PyObject *args, *out = NULL;

	args = Py_BuildValue("(O&)", cvt_c2p_arena, arg1_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"callback CB_TURFVICTORY");
		return ;
	}

	call_gen_py_callbacks(PYCBPREFIX CB_TURFVICTORY, args);
	Py_DECREF(args);
	Py_XDECREF(out);
}

local void py_cb_call_CB_TURFVICTORY(Arena *arena, PyObject *args)
{
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return;
	DO_CBS(CB_TURFVICTORY, arena, TurfVictoryFunc, (arg1_in));
}

typedef void (*py_cb_caller)(Arena *arena, PyObject *args);
local HashTable *py_cb_callers;

local void init_py_callbacks(void)
{
	py_cb_callers = HashAlloc();
	{ BallPickupFunc typecheck = py_cb_CB_BALLPICKUP; (void)typecheck; }
	mm->RegCallback(CB_BALLPICKUP, py_cb_CB_BALLPICKUP, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_BALLPICKUP, py_cb_call_CB_BALLPICKUP);
	{ BallFireFunc typecheck = py_cb_CB_BALLFIRE; (void)typecheck; }
	mm->RegCallback(CB_BALLFIRE, py_cb_CB_BALLFIRE, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_BALLFIRE, py_cb_call_CB_BALLFIRE);
	{ GoalFunc typecheck = py_cb_CB_GOAL; (void)typecheck; }
	mm->RegCallback(CB_GOAL, py_cb_CB_GOAL, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_GOAL, py_cb_call_CB_GOAL);
	{ SetBannerFunc typecheck = py_cb_CB_SET_BANNER; (void)typecheck; }
	mm->RegCallback(CB_SET_BANNER, py_cb_CB_SET_BANNER, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_SET_BANNER, py_cb_call_CB_SET_BANNER);
	{ ChatMsgFunc typecheck = py_cb_CB_CHATMSG; (void)typecheck; }
	mm->RegCallback(CB_CHATMSG, py_cb_CB_CHATMSG, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_CHATMSG, py_cb_call_CB_CHATMSG);
	{ GlobalConfigChangedFunc typecheck = py_cb_CB_GLOBALCONFIGCHANGED; (void)typecheck; }
	mm->RegCallback(CB_GLOBALCONFIGCHANGED, py_cb_CB_GLOBALCONFIGCHANGED, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_GLOBALCONFIGCHANGED, py_cb_call_CB_GLOBALCONFIGCHANGED);
	{ PlayerActionFunc typecheck = py_cb_CB_PLAYERACTION; (void)typecheck; }
	mm->RegCallback(CB_PLAYERACTION, py_cb_CB_PLAYERACTION, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_PLAYERACTION, py_cb_call_CB_PLAYERACTION);
	{ FlagPickupFunc typecheck = py_cb_CB_FLAGPICKUP; (void)typecheck; }
	mm->RegCallback(CB_FLAGPICKUP, py_cb_CB_FLAGPICKUP, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_FLAGPICKUP, py_cb_call_CB_FLAGPICKUP);
	{ FlagDropFunc typecheck = py_cb_CB_FLAGDROP; (void)typecheck; }
	mm->RegCallback(CB_FLAGDROP, py_cb_CB_FLAGDROP, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_FLAGDROP, py_cb_call_CB_FLAGDROP);
	{ FlagPosFunc typecheck = py_cb_CB_FLAGPOS; (void)typecheck; }
	mm->RegCallback(CB_FLAGPOS, py_cb_CB_FLAGPOS, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_FLAGPOS, py_cb_call_CB_FLAGPOS);
	{ FlagWinFunc typecheck = py_cb_CB_FLAGWIN; (void)typecheck; }
	mm->RegCallback(CB_FLAGWIN, py_cb_CB_FLAGWIN, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_FLAGWIN, py_cb_call_CB_FLAGWIN);
	{ FreqChangeFunc typecheck = py_cb_CB_FREQCHANGE; (void)typecheck; }
	mm->RegCallback(CB_FREQCHANGE, py_cb_CB_FREQCHANGE, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_FREQCHANGE, py_cb_call_CB_FREQCHANGE);
	{ ShipChangeFunc typecheck = py_cb_CB_SHIPCHANGE; (void)typecheck; }
	mm->RegCallback(CB_SHIPCHANGE, py_cb_CB_SHIPCHANGE, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_SHIPCHANGE, py_cb_call_CB_SHIPCHANGE);
	{ GameTimerFunc typecheck = py_cb_CB_TIMESUP; (void)typecheck; }
	mm->RegCallback(CB_TIMESUP, py_cb_CB_TIMESUP, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_TIMESUP, py_cb_call_CB_TIMESUP);
	{ SafeZoneFunc typecheck = py_cb_CB_SAFEZONE; (void)typecheck; }
	mm->RegCallback(CB_SAFEZONE, py_cb_CB_SAFEZONE, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_SAFEZONE, py_cb_call_CB_SAFEZONE);
	{ GreenFunc typecheck = py_cb_CB_GREEN; (void)typecheck; }
	mm->RegCallback(CB_GREEN, py_cb_CB_GREEN, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_GREEN, py_cb_call_CB_GREEN);
	{ LogFunc typecheck = py_cb_CB_LOGFUNC; (void)typecheck; }
	mm->RegCallback(CB_LOGFUNC, py_cb_CB_LOGFUNC, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_LOGFUNC, py_cb_call_CB_LOGFUNC);
	{ EndIntervalFunc typecheck = py_cb_CB_INTERVAL_ENDED; (void)typecheck; }
	mm->RegCallback(CB_INTERVAL_ENDED, py_cb_CB_INTERVAL_ENDED, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_INTERVAL_ENDED, py_cb_call_CB_INTERVAL_ENDED);
	{ TurfTagFunc typecheck = py_cb_CB_TURFTAG; (void)typecheck; }
	mm->RegCallback(CB_TURFTAG, py_cb_CB_TURFTAG, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_TURFTAG, py_cb_call_CB_TURFTAG);
	{ TurfStealFunc typecheck = py_cb_CB_TURFSTEAL; (void)typecheck; }
	mm->RegCallback(CB_TURFSTEAL, py_cb_CB_TURFSTEAL, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_TURFSTEAL, py_cb_call_CB_TURFSTEAL);
	{ TurfRecoverFunc typecheck = py_cb_CB_TURFRECOVER; (void)typecheck; }
	mm->RegCallback(CB_TURFRECOVER, py_cb_CB_TURFRECOVER, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_TURFRECOVER, py_cb_call_CB_TURFRECOVER);
	{ TurfLostFunc typecheck = py_cb_CB_TURFLOST; (void)typecheck; }
	mm->RegCallback(CB_TURFLOST, py_cb_CB_TURFLOST, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_TURFLOST, py_cb_call_CB_TURFLOST);
	{ TurfVictoryFunc typecheck = py_cb_CB_TURFVICTORY; (void)typecheck; }
	mm->RegCallback(CB_TURFVICTORY, py_cb_CB_TURFVICTORY, ALLARENAS);
	HashReplace(py_cb_callers, PYCBPREFIX CB_TURFVICTORY, py_cb_call_CB_TURFVICTORY);
}

local void deinit_py_callbacks(void)
{
	mm->UnregCallback(CB_BALLPICKUP, py_cb_CB_BALLPICKUP, ALLARENAS);
	mm->UnregCallback(CB_BALLFIRE, py_cb_CB_BALLFIRE, ALLARENAS);
	mm->UnregCallback(CB_GOAL, py_cb_CB_GOAL, ALLARENAS);
	mm->UnregCallback(CB_SET_BANNER, py_cb_CB_SET_BANNER, ALLARENAS);
	mm->UnregCallback(CB_CHATMSG, py_cb_CB_CHATMSG, ALLARENAS);
	mm->UnregCallback(CB_GLOBALCONFIGCHANGED, py_cb_CB_GLOBALCONFIGCHANGED, ALLARENAS);
	mm->UnregCallback(CB_PLAYERACTION, py_cb_CB_PLAYERACTION, ALLARENAS);
	mm->UnregCallback(CB_FLAGPICKUP, py_cb_CB_FLAGPICKUP, ALLARENAS);
	mm->UnregCallback(CB_FLAGDROP, py_cb_CB_FLAGDROP, ALLARENAS);
	mm->UnregCallback(CB_FLAGPOS, py_cb_CB_FLAGPOS, ALLARENAS);
	mm->UnregCallback(CB_FLAGWIN, py_cb_CB_FLAGWIN, ALLARENAS);
	mm->UnregCallback(CB_FREQCHANGE, py_cb_CB_FREQCHANGE, ALLARENAS);
	mm->UnregCallback(CB_SHIPCHANGE, py_cb_CB_SHIPCHANGE, ALLARENAS);
	mm->UnregCallback(CB_TIMESUP, py_cb_CB_TIMESUP, ALLARENAS);
	mm->UnregCallback(CB_SAFEZONE, py_cb_CB_SAFEZONE, ALLARENAS);
	mm->UnregCallback(CB_GREEN, py_cb_CB_GREEN, ALLARENAS);
	mm->UnregCallback(CB_LOGFUNC, py_cb_CB_LOGFUNC, ALLARENAS);
	mm->UnregCallback(CB_INTERVAL_ENDED, py_cb_CB_INTERVAL_ENDED, ALLARENAS);
	mm->UnregCallback(CB_TURFTAG, py_cb_CB_TURFTAG, ALLARENAS);
	mm->UnregCallback(CB_TURFSTEAL, py_cb_CB_TURFSTEAL, ALLARENAS);
	mm->UnregCallback(CB_TURFRECOVER, py_cb_CB_TURFRECOVER, ALLARENAS);
	mm->UnregCallback(CB_TURFLOST, py_cb_CB_TURFLOST, ALLARENAS);
	mm->UnregCallback(CB_TURFVICTORY, py_cb_CB_TURFVICTORY, ALLARENAS);
	HashFree(py_cb_callers);
}
