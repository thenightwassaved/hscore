
/* THIS IS AN AUTOMATICALLY GENERATED FILE */

/* pyint declarations */

typedef char charbuf[1024];

local void pyint_generic_dealloc(pyint_generic_interface_object *self)
{
	mm->ReleaseInterface(self->i);
	PyObject_Del(self);
}


/* using interface I_ARENAMAN from python {{{ */

typedef struct {
	PyObject_HEAD
	Iarenaman *i;
} pyint_obj_I_ARENAMAN;

local PyObject *
pyint_method_I_ARENAMAN_RecycleArena(pyint_obj_I_ARENAMAN *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return NULL;

	ret = me->i->RecycleArena(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_ARENAMAN_SendToArena(pyint_obj_I_ARENAMAN *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	const char *arg2_in;
	int arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "O&sii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in, &arg4_in))
		return NULL;

	me->i->SendToArena(arg1_in, arg2_in, arg3_in, arg4_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_ARENAMAN_FindArena(pyint_obj_I_ARENAMAN *me, PyObject *args)
{
	PyObject *out;
	Arena *ret;
	const char *arg1_in;
	int  arg2_out = 0;
	int  arg3_out = 0;
	if (!PyArg_ParseTuple(args, "s", &arg1_in))
		return NULL;

	ret = me->i->FindArena(arg1_in, &arg2_out, &arg3_out);

	out = Py_BuildValue("O&ii", cvt_c2p_arena, ret, arg2_out, arg3_out);

	return out;
}

local PyObject *
pyint_method_I_ARENAMAN_GetPopulationSummary(pyint_obj_I_ARENAMAN *me, PyObject *args)
{
	PyObject *out;
	int  arg1_out = 0;
	int  arg2_out = 0;
	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	me->i->GetPopulationSummary(&arg1_out, &arg2_out);

	out = Py_BuildValue("ii", arg1_out, arg2_out);

	return out;
}

local PyObject *
pyint_method_I_ARENAMAN_Lock(pyint_obj_I_ARENAMAN *me, PyObject *args)
{
	PyObject *out;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	me->i->Lock();

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_ARENAMAN_Unlock(pyint_obj_I_ARENAMAN *me, PyObject *args)
{
	PyObject *out;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	me->i->Unlock();

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_ARENAMAN_methods[] = {
	{"RecycleArena", (PyCFunction)pyint_method_I_ARENAMAN_RecycleArena, METH_VARARGS, NULL },
	{"SendToArena", (PyCFunction)pyint_method_I_ARENAMAN_SendToArena, METH_VARARGS, NULL },
	{"FindArena", (PyCFunction)pyint_method_I_ARENAMAN_FindArena, METH_VARARGS, NULL },
	{"GetPopulationSummary", (PyCFunction)pyint_method_I_ARENAMAN_GetPopulationSummary, METH_VARARGS, NULL },
	{"Lock", (PyCFunction)pyint_method_I_ARENAMAN_Lock, METH_VARARGS, NULL },
	{"Unlock", (PyCFunction)pyint_method_I_ARENAMAN_Unlock, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_ARENAMAN_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_ARENAMAN_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iarenaman",       /*tp_name*/
	sizeof(pyint_obj_I_ARENAMAN), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_ARENAMAN_methods,        /*tp_methods*/
	pyint_I_ARENAMAN_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_ARENAPLACE from python {{{ */

typedef struct {
	PyObject_HEAD
	Iarenaplace *i;
} pyint_obj_I_ARENAPLACE;

local PyObject *
pyint_method_I_ARENAPLACE_Place(pyint_obj_I_ARENAPLACE *me, PyObject *args)
{
	PyObject *out;
	int ret;
	charbuf arg1_out = {0};
	int  arg3_out = 0;
	int  arg4_out = 0;
	Player *arg5_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_player, &arg5_in))
		return NULL;

	ret = me->i->Place(arg1_out, 1024, &arg3_out, &arg4_out, arg5_in);

	out = Py_BuildValue("isii", ret, arg1_out, arg3_out, arg4_out);

	return out;
}

local PyMethodDef pyint_I_ARENAPLACE_methods[] = {
	{"Place", (PyCFunction)pyint_method_I_ARENAPLACE_Place, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_ARENAPLACE_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_ARENAPLACE_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iarenaplace",       /*tp_name*/
	sizeof(pyint_obj_I_ARENAPLACE), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_ARENAPLACE_methods,        /*tp_methods*/
	pyint_I_ARENAPLACE_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* implementing interface I_ARENAPLACE in python {{{ */

local int pyint_func_I_ARENAPLACE_Place(char *arg1_out, int arg2, int *arg3_out, int *arg4_out, Player *arg5_in)
{
	PyObject *args, *out = NULL;
	int ret;
	const char *arg1_outv;
	int arg3_outv;
	int arg4_outv;
	args = Py_BuildValue("(O&)", cvt_c2p_player, arg5_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function Place in interface I_ARENAPLACE");
		return 0;
	}

	out = call_gen_py_interface(PYINTPREFIX I_ARENAPLACE, 0, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function Place in interface I_ARENAPLACE");
		return 0;
	}

	if (!PyArg_ParseTuple(out, "isii", &ret, &arg1_outv, &arg3_outv, &arg4_outv))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function Place in interface I_ARENAPLACE");
		return 0;
	}
	astrncpy(arg1_out, arg1_outv, arg2);
	*arg3_out = arg3_outv;
	*arg4_out = arg4_outv;
	Py_XDECREF(out);
	return ret;
}

local struct Iarenaplace pyint_int_I_ARENAPLACE = {
	INTERFACE_HEAD_INIT(I_ARENAPLACE, "pyint-I_ARENAPLACE")
	pyint_func_I_ARENAPLACE_Place
};


/* }}} */

/* using interface I_BALLS from python {{{ */

typedef struct {
	PyObject_HEAD
	Iballs *i;
} pyint_obj_I_BALLS;

local PyObject *
pyint_method_I_BALLS_SetBallCount(pyint_obj_I_BALLS *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_arena, &arg1_in, &arg2_in))
		return NULL;

	me->i->SetBallCount(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_BALLS_EndGame(pyint_obj_I_BALLS *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return NULL;

	me->i->EndGame(arg1_in);

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_BALLS_methods[] = {
	{"SetBallCount", (PyCFunction)pyint_method_I_BALLS_SetBallCount, METH_VARARGS, NULL },
	{"EndGame", (PyCFunction)pyint_method_I_BALLS_EndGame, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_BALLS_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_BALLS_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iballs",       /*tp_name*/
	sizeof(pyint_obj_I_BALLS), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_BALLS_methods,        /*tp_methods*/
	pyint_I_BALLS_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_BANNERS from python {{{ */

typedef struct {
	PyObject_HEAD
	Ibanners *i;
} pyint_obj_I_BANNERS;

local PyObject *
pyint_method_I_BANNERS_SetBanner(pyint_obj_I_BANNERS *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	Banner *arg2_in;
	int arg3_in = 0;
	if (!PyArg_ParseTuple(args, "O&O&", cvt_p2c_player, &arg1_in, cvt_p2c_banner, &arg2_in))
		return NULL;

	me->i->SetBanner(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_BANNERS_methods[] = {
	{"SetBanner", (PyCFunction)pyint_method_I_BANNERS_SetBanner, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_BANNERS_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_BANNERS_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ibanners",       /*tp_name*/
	sizeof(pyint_obj_I_BANNERS), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_BANNERS_methods,        /*tp_methods*/
	pyint_I_BANNERS_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_BRICKS from python {{{ */

typedef struct {
	PyObject_HEAD
	Ibricks *i;
} pyint_obj_I_BRICKS;

local PyObject *
pyint_method_I_BRICKS_DropBrick(pyint_obj_I_BRICKS *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	int arg6_in;
	if (!PyArg_ParseTuple(args, "O&iiiii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in, &arg6_in))
		return NULL;

	me->i->DropBrick(arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in);

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_BRICKS_methods[] = {
	{"DropBrick", (PyCFunction)pyint_method_I_BRICKS_DropBrick, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_BRICKS_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_BRICKS_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ibricks",       /*tp_name*/
	sizeof(pyint_obj_I_BRICKS), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_BRICKS_methods,        /*tp_methods*/
	pyint_I_BRICKS_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_CAPMAN from python {{{ */

typedef struct {
	PyObject_HEAD
	Icapman *i;
} pyint_obj_I_CAPMAN;

local PyObject *
pyint_method_I_CAPMAN_HasCapability(pyint_obj_I_CAPMAN *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Player *arg1_in;
	const char *arg2_in;
	if (!PyArg_ParseTuple(args, "O&s", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	ret = me->i->HasCapability(arg1_in, arg2_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_CAPMAN_HasCapabilityByName(pyint_obj_I_CAPMAN *me, PyObject *args)
{
	PyObject *out;
	int ret;
	const char *arg1_in;
	const char *arg2_in;
	if (!PyArg_ParseTuple(args, "ss", &arg1_in, &arg2_in))
		return NULL;

	ret = me->i->HasCapabilityByName(arg1_in, arg2_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_CAPMAN_HigherThan(pyint_obj_I_CAPMAN *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Player *arg1_in;
	Player *arg2_in;
	if (!PyArg_ParseTuple(args, "O&O&", cvt_p2c_player, &arg1_in, cvt_p2c_player, &arg2_in))
		return NULL;

	ret = me->i->HigherThan(arg1_in, arg2_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_CAPMAN_methods[] = {
	{"HasCapability", (PyCFunction)pyint_method_I_CAPMAN_HasCapability, METH_VARARGS, NULL },
	{"HasCapabilityByName", (PyCFunction)pyint_method_I_CAPMAN_HasCapabilityByName, METH_VARARGS, NULL },
	{"HigherThan", (PyCFunction)pyint_method_I_CAPMAN_HigherThan, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_CAPMAN_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_CAPMAN_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Icapman",       /*tp_name*/
	sizeof(pyint_obj_I_CAPMAN), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_CAPMAN_methods,        /*tp_methods*/
	pyint_I_CAPMAN_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* implementing interface I_CAPMAN in python {{{ */

local int pyint_func_I_CAPMAN_HasCapability(Player *arg1_in, const char *arg2_in)
{
	PyObject *args, *out = NULL;
	int ret;
	args = Py_BuildValue("(O&s)", cvt_c2p_player, arg1_in, arg2_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function HasCapability in interface I_CAPMAN");
		return 0;
	}

	out = call_gen_py_interface(PYINTPREFIX I_CAPMAN, 0, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function HasCapability in interface I_CAPMAN");
		return 0;
	}

	if (!PyArg_ParseTuple(out, "i", &ret))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function HasCapability in interface I_CAPMAN");
		return 0;
	}

	Py_XDECREF(out);
	return ret;
}

local int pyint_func_I_CAPMAN_HasCapabilityByName(const char *arg1_in, const char *arg2_in)
{
	PyObject *args, *out = NULL;
	int ret;
	args = Py_BuildValue("(ss)", arg1_in, arg2_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function HasCapabilityByName in interface I_CAPMAN");
		return 0;
	}

	out = call_gen_py_interface(PYINTPREFIX I_CAPMAN, 1, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function HasCapabilityByName in interface I_CAPMAN");
		return 0;
	}

	if (!PyArg_ParseTuple(out, "i", &ret))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function HasCapabilityByName in interface I_CAPMAN");
		return 0;
	}

	Py_XDECREF(out);
	return ret;
}

local int pyint_func_I_CAPMAN_HigherThan(Player *arg1_in, Player *arg2_in)
{
	PyObject *args, *out = NULL;
	int ret;
	args = Py_BuildValue("(O&O&)", cvt_c2p_player, arg1_in, cvt_c2p_player, arg2_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function HigherThan in interface I_CAPMAN");
		return 0;
	}

	out = call_gen_py_interface(PYINTPREFIX I_CAPMAN, 2, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function HigherThan in interface I_CAPMAN");
		return 0;
	}

	if (!PyArg_ParseTuple(out, "i", &ret))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function HigherThan in interface I_CAPMAN");
		return 0;
	}

	Py_XDECREF(out);
	return ret;
}

local struct Icapman pyint_int_I_CAPMAN = {
	INTERFACE_HEAD_INIT(I_CAPMAN, "pyint-I_CAPMAN")
	pyint_func_I_CAPMAN_HasCapability,
	pyint_func_I_CAPMAN_HasCapabilityByName,
	pyint_func_I_CAPMAN_HigherThan
};


/* }}} */

/* using interface I_GROUPMAN from python {{{ */

typedef struct {
	PyObject_HEAD
	Igroupman *i;
} pyint_obj_I_GROUPMAN;

local PyObject *
pyint_method_I_GROUPMAN_GetGroup(pyint_obj_I_GROUPMAN *me, PyObject *args)
{
	PyObject *out;
	const char *ret;
	Player *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_player, &arg1_in))
		return NULL;

	ret = me->i->GetGroup(arg1_in);

	out = Py_BuildValue("s", ret);

	return out;
}

local PyObject *
pyint_method_I_GROUPMAN_SetPermGroup(pyint_obj_I_GROUPMAN *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	const char *arg2_in;
	int arg3_in;
	const char *arg4_in;
	if (!PyArg_ParseTuple(args, "O&siz", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in, &arg4_in))
		return NULL;

	me->i->SetPermGroup(arg1_in, arg2_in, arg3_in, arg4_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GROUPMAN_SetTempGroup(pyint_obj_I_GROUPMAN *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	const char *arg2_in;
	if (!PyArg_ParseTuple(args, "O&s", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	me->i->SetTempGroup(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GROUPMAN_RemoveGroup(pyint_obj_I_GROUPMAN *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	const char *arg2_in;
	if (!PyArg_ParseTuple(args, "O&z", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	me->i->RemoveGroup(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GROUPMAN_CheckGroupPassword(pyint_obj_I_GROUPMAN *me, PyObject *args)
{
	PyObject *out;
	int ret;
	const char *arg1_in;
	const char *arg2_in;
	if (!PyArg_ParseTuple(args, "ss", &arg1_in, &arg2_in))
		return NULL;

	ret = me->i->CheckGroupPassword(arg1_in, arg2_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_GROUPMAN_methods[] = {
	{"GetGroup", (PyCFunction)pyint_method_I_GROUPMAN_GetGroup, METH_VARARGS, NULL },
	{"SetPermGroup", (PyCFunction)pyint_method_I_GROUPMAN_SetPermGroup, METH_VARARGS, NULL },
	{"SetTempGroup", (PyCFunction)pyint_method_I_GROUPMAN_SetTempGroup, METH_VARARGS, NULL },
	{"RemoveGroup", (PyCFunction)pyint_method_I_GROUPMAN_RemoveGroup, METH_VARARGS, NULL },
	{"CheckGroupPassword", (PyCFunction)pyint_method_I_GROUPMAN_CheckGroupPassword, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_GROUPMAN_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_GROUPMAN_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Igroupman",       /*tp_name*/
	sizeof(pyint_obj_I_GROUPMAN), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_GROUPMAN_methods,        /*tp_methods*/
	pyint_I_GROUPMAN_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_CHAT from python {{{ */

typedef struct {
	PyObject_HEAD
	Ichat *i;
} pyint_obj_I_CHAT;

local PyObject *
pyint_method_I_CHAT_SendMessage(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	const char *arg2_infmt;
	if (!PyArg_ParseTuple(args, "O&s", cvt_p2c_player, &arg1_in, &arg2_infmt))
		return NULL;

	me->i->SendMessage(arg1_in, "%s", arg2_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CHAT_SendCmdMessage(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	const char *arg2_infmt;
	if (!PyArg_ParseTuple(args, "O&s", cvt_p2c_player, &arg1_in, &arg2_infmt))
		return NULL;

	me->i->SendCmdMessage(arg1_in, "%s", arg2_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CHAT_SendSoundMessage(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	const char *arg3_infmt;
	if (!PyArg_ParseTuple(args, "O&is", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_infmt))
		return NULL;

	me->i->SendSoundMessage(arg1_in, arg2_in, "%s", arg3_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CHAT_SendArenaMessage(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	const char *arg2_infmt;
	if (!PyArg_ParseTuple(args, "O&s", cvt_p2c_arena, &arg1_in, &arg2_infmt))
		return NULL;

	me->i->SendArenaMessage(arg1_in, "%s", arg2_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CHAT_SendArenaSoundMessage(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	const char *arg3_infmt;
	if (!PyArg_ParseTuple(args, "O&is", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_infmt))
		return NULL;

	me->i->SendArenaSoundMessage(arg1_in, arg2_in, "%s", arg3_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CHAT_SendModMessage(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	const char *arg1_infmt;
	if (!PyArg_ParseTuple(args, "s", &arg1_infmt))
		return NULL;

	me->i->SendModMessage("%s", arg1_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CHAT_GetArenaChatMask(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return NULL;

	ret = me->i->GetArenaChatMask(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_CHAT_SetArenaChatMask(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_arena, &arg1_in, &arg2_in))
		return NULL;

	me->i->SetArenaChatMask(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CHAT_GetPlayerChatMask(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Player *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_player, &arg1_in))
		return NULL;

	ret = me->i->GetPlayerChatMask(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_CHAT_SetPlayerChatMask(pyint_obj_I_CHAT *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	me->i->SetPlayerChatMask(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_CHAT_methods[] = {
	{"SendMessage", (PyCFunction)pyint_method_I_CHAT_SendMessage, METH_VARARGS, NULL },
	{"SendCmdMessage", (PyCFunction)pyint_method_I_CHAT_SendCmdMessage, METH_VARARGS, NULL },
	{"SendSoundMessage", (PyCFunction)pyint_method_I_CHAT_SendSoundMessage, METH_VARARGS, NULL },
	{"SendArenaMessage", (PyCFunction)pyint_method_I_CHAT_SendArenaMessage, METH_VARARGS, NULL },
	{"SendArenaSoundMessage", (PyCFunction)pyint_method_I_CHAT_SendArenaSoundMessage, METH_VARARGS, NULL },
	{"SendModMessage", (PyCFunction)pyint_method_I_CHAT_SendModMessage, METH_VARARGS, NULL },
	{"GetArenaChatMask", (PyCFunction)pyint_method_I_CHAT_GetArenaChatMask, METH_VARARGS, NULL },
	{"SetArenaChatMask", (PyCFunction)pyint_method_I_CHAT_SetArenaChatMask, METH_VARARGS, NULL },
	{"GetPlayerChatMask", (PyCFunction)pyint_method_I_CHAT_GetPlayerChatMask, METH_VARARGS, NULL },
	{"SetPlayerChatMask", (PyCFunction)pyint_method_I_CHAT_SetPlayerChatMask, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_CHAT_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_CHAT_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ichat",       /*tp_name*/
	sizeof(pyint_obj_I_CHAT), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_CHAT_methods,        /*tp_methods*/
	pyint_I_CHAT_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_CONFIG from python {{{ */

typedef struct {
	PyObject_HEAD
	Iconfig *i;
} pyint_obj_I_CONFIG;

local PyObject *
pyint_method_I_CONFIG_GetStr(pyint_obj_I_CONFIG *me, PyObject *args)
{
	PyObject *out;
	const char *ret;
	ConfigHandle arg1_in;
	const char *arg2_in;
	const char *arg3_in;
	if (!PyArg_ParseTuple(args, "O&ss", cvt_p2c_config, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	ret = me->i->GetStr(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("s", ret);

	return out;
}

local PyObject *
pyint_method_I_CONFIG_GetInt(pyint_obj_I_CONFIG *me, PyObject *args)
{
	PyObject *out;
	int ret;
	ConfigHandle arg1_in;
	const char *arg2_in;
	const char *arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "O&ssi", cvt_p2c_config, &arg1_in, &arg2_in, &arg3_in, &arg4_in))
		return NULL;

	ret = me->i->GetInt(arg1_in, arg2_in, arg3_in, arg4_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_CONFIG_SetStr(pyint_obj_I_CONFIG *me, PyObject *args)
{
	PyObject *out;
	ConfigHandle arg1_in;
	const char *arg2_in;
	const char *arg3_in;
	const char *arg4_in;
	const char *arg5_in;
	int arg6_in;
	if (!PyArg_ParseTuple(args, "O&ssszi", cvt_p2c_config, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in, &arg6_in))
		return NULL;

	me->i->SetStr(arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CONFIG_SetInt(pyint_obj_I_CONFIG *me, PyObject *args)
{
	PyObject *out;
	ConfigHandle arg1_in;
	const char *arg2_in;
	const char *arg3_in;
	int arg4_in;
	const char *arg5_in;
	int arg6_in;
	if (!PyArg_ParseTuple(args, "O&ssizi", cvt_p2c_config, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in, &arg6_in))
		return NULL;

	me->i->SetInt(arg1_in, arg2_in, arg3_in, arg4_in, arg5_in, arg6_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CONFIG_ReloadConfigFile(pyint_obj_I_CONFIG *me, PyObject *args)
{
	PyObject *out;
	ConfigHandle arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_config, &arg1_in))
		return NULL;

	me->i->ReloadConfigFile(arg1_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CONFIG_FlushDirtyValues(pyint_obj_I_CONFIG *me, PyObject *args)
{
	PyObject *out;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	me->i->FlushDirtyValues();

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_CONFIG_CheckModifiedFiles(pyint_obj_I_CONFIG *me, PyObject *args)
{
	PyObject *out;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	me->i->CheckModifiedFiles();

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_CONFIG_methods[] = {
	{"GetStr", (PyCFunction)pyint_method_I_CONFIG_GetStr, METH_VARARGS, NULL },
	{"GetInt", (PyCFunction)pyint_method_I_CONFIG_GetInt, METH_VARARGS, NULL },
	{"SetStr", (PyCFunction)pyint_method_I_CONFIG_SetStr, METH_VARARGS, NULL },
	{"SetInt", (PyCFunction)pyint_method_I_CONFIG_SetInt, METH_VARARGS, NULL },
	{"ReloadConfigFile", (PyCFunction)pyint_method_I_CONFIG_ReloadConfigFile, METH_VARARGS, NULL },
	{"FlushDirtyValues", (PyCFunction)pyint_method_I_CONFIG_FlushDirtyValues, METH_VARARGS, NULL },
	{"CheckModifiedFiles", (PyCFunction)pyint_method_I_CONFIG_CheckModifiedFiles, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_CONFIG_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_CONFIG_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iconfig",       /*tp_name*/
	sizeof(pyint_obj_I_CONFIG), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_CONFIG_methods,        /*tp_methods*/
	pyint_I_CONFIG_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_FREQMAN from python {{{ */

typedef struct {
	PyObject_HEAD
	Ifreqman *i;
} pyint_obj_I_FREQMAN;

local PyObject *
pyint_method_I_FREQMAN_InitialFreq(pyint_obj_I_FREQMAN *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int  arg2_inout = 0;
	int  arg3_inout = 0;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_inout, &arg3_inout))
		return NULL;

	me->i->InitialFreq(arg1_in, &arg2_inout, &arg3_inout);

	out = Py_BuildValue("ii", arg2_inout, arg3_inout);

	return out;
}

local PyObject *
pyint_method_I_FREQMAN_ShipChange(pyint_obj_I_FREQMAN *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int  arg2_inout = 0;
	int  arg3_inout = 0;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_inout, &arg3_inout))
		return NULL;

	me->i->ShipChange(arg1_in, &arg2_inout, &arg3_inout);

	out = Py_BuildValue("ii", arg2_inout, arg3_inout);

	return out;
}

local PyObject *
pyint_method_I_FREQMAN_FreqChange(pyint_obj_I_FREQMAN *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int  arg2_inout = 0;
	int  arg3_inout = 0;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_inout, &arg3_inout))
		return NULL;

	me->i->FreqChange(arg1_in, &arg2_inout, &arg3_inout);

	out = Py_BuildValue("ii", arg2_inout, arg3_inout);

	return out;
}

local PyMethodDef pyint_I_FREQMAN_methods[] = {
	{"InitialFreq", (PyCFunction)pyint_method_I_FREQMAN_InitialFreq, METH_VARARGS, NULL },
	{"ShipChange", (PyCFunction)pyint_method_I_FREQMAN_ShipChange, METH_VARARGS, NULL },
	{"FreqChange", (PyCFunction)pyint_method_I_FREQMAN_FreqChange, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_FREQMAN_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_FREQMAN_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ifreqman",       /*tp_name*/
	sizeof(pyint_obj_I_FREQMAN), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_FREQMAN_methods,        /*tp_methods*/
	pyint_I_FREQMAN_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* implementing interface I_FREQMAN in python {{{ */

local void pyint_func_I_FREQMAN_InitialFreq(Player *arg1_in, int *arg2_inout, int *arg3_inout)
{
	PyObject *args, *out = NULL;
	int arg2_inoutv;
	int arg3_inoutv;
	args = Py_BuildValue("(O&ii)", cvt_c2p_player, arg1_in, *arg2_inout, *arg3_inout);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function InitialFreq in interface I_FREQMAN");
		return ;
	}

	out = call_gen_py_interface(PYINTPREFIX I_FREQMAN, 0, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function InitialFreq in interface I_FREQMAN");
		return ;
	}

	if (!PyArg_ParseTuple(out, "ii", &arg2_inoutv, &arg3_inoutv))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function InitialFreq in interface I_FREQMAN");
		return ;
	}
	*arg2_inout = arg2_inoutv;
	*arg3_inout = arg3_inoutv;
	Py_XDECREF(out);
	return ;
}

local void pyint_func_I_FREQMAN_ShipChange(Player *arg1_in, int *arg2_inout, int *arg3_inout)
{
	PyObject *args, *out = NULL;
	int arg2_inoutv;
	int arg3_inoutv;
	args = Py_BuildValue("(O&ii)", cvt_c2p_player, arg1_in, *arg2_inout, *arg3_inout);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function ShipChange in interface I_FREQMAN");
		return ;
	}

	out = call_gen_py_interface(PYINTPREFIX I_FREQMAN, 1, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function ShipChange in interface I_FREQMAN");
		return ;
	}

	if (!PyArg_ParseTuple(out, "ii", &arg2_inoutv, &arg3_inoutv))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function ShipChange in interface I_FREQMAN");
		return ;
	}
	*arg2_inout = arg2_inoutv;
	*arg3_inout = arg3_inoutv;
	Py_XDECREF(out);
	return ;
}

local void pyint_func_I_FREQMAN_FreqChange(Player *arg1_in, int *arg2_inout, int *arg3_inout)
{
	PyObject *args, *out = NULL;
	int arg2_inoutv;
	int arg3_inoutv;
	args = Py_BuildValue("(O&ii)", cvt_c2p_player, arg1_in, *arg2_inout, *arg3_inout);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function FreqChange in interface I_FREQMAN");
		return ;
	}

	out = call_gen_py_interface(PYINTPREFIX I_FREQMAN, 2, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function FreqChange in interface I_FREQMAN");
		return ;
	}

	if (!PyArg_ParseTuple(out, "ii", &arg2_inoutv, &arg3_inoutv))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function FreqChange in interface I_FREQMAN");
		return ;
	}
	*arg2_inout = arg2_inoutv;
	*arg3_inout = arg3_inoutv;
	Py_XDECREF(out);
	return ;
}

local struct Ifreqman pyint_int_I_FREQMAN = {
	INTERFACE_HEAD_INIT(I_FREQMAN, "pyint-I_FREQMAN")
	pyint_func_I_FREQMAN_InitialFreq,
	pyint_func_I_FREQMAN_ShipChange,
	pyint_func_I_FREQMAN_FreqChange
};


/* }}} */

/* using interface I_FAKE from python {{{ */

typedef struct {
	PyObject_HEAD
	Ifake *i;
} pyint_obj_I_FAKE;

local PyObject *
pyint_method_I_FAKE_CreateFakePlayer(pyint_obj_I_FAKE *me, PyObject *args)
{
	PyObject *out;
	Player *ret;
	const char *arg1_in;
	Arena *arg2_in;
	int arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "sO&ii", &arg1_in, cvt_p2c_arena, &arg2_in, &arg3_in, &arg4_in))
		return NULL;

	ret = me->i->CreateFakePlayer(arg1_in, arg2_in, arg3_in, arg4_in);

	out = Py_BuildValue("O&", cvt_c2p_player, ret);

	return out;
}

local PyObject *
pyint_method_I_FAKE_EndFaked(pyint_obj_I_FAKE *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Player *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_player, &arg1_in))
		return NULL;

	ret = me->i->EndFaked(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_FAKE_methods[] = {
	{"CreateFakePlayer", (PyCFunction)pyint_method_I_FAKE_CreateFakePlayer, METH_VARARGS, NULL },
	{"EndFaked", (PyCFunction)pyint_method_I_FAKE_EndFaked, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_FAKE_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_FAKE_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ifake",       /*tp_name*/
	sizeof(pyint_obj_I_FAKE), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_FAKE_methods,        /*tp_methods*/
	pyint_I_FAKE_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_FLAGS from python {{{ */

typedef struct {
	PyObject_HEAD
	Iflags *i;
} pyint_obj_I_FLAGS;

local PyObject *
pyint_method_I_FLAGS_MoveFlag(pyint_obj_I_FLAGS *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	if (!PyArg_ParseTuple(args, "O&iiii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in))
		return NULL;

	me->i->MoveFlag(arg1_in, arg2_in, arg3_in, arg4_in, arg5_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_FLAGS_FlagVictory(pyint_obj_I_FLAGS *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	me->i->FlagVictory(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_FLAGS_GetCarriedFlags(pyint_obj_I_FLAGS *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Player *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_player, &arg1_in))
		return NULL;

	ret = me->i->GetCarriedFlags(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_FLAGS_GetFreqFlags(pyint_obj_I_FLAGS *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_arena, &arg1_in, &arg2_in))
		return NULL;

	ret = me->i->GetFreqFlags(arg1_in, arg2_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_FLAGS_methods[] = {
	{"MoveFlag", (PyCFunction)pyint_method_I_FLAGS_MoveFlag, METH_VARARGS, NULL },
	{"FlagVictory", (PyCFunction)pyint_method_I_FLAGS_FlagVictory, METH_VARARGS, NULL },
	{"GetCarriedFlags", (PyCFunction)pyint_method_I_FLAGS_GetCarriedFlags, METH_VARARGS, NULL },
	{"GetFreqFlags", (PyCFunction)pyint_method_I_FLAGS_GetFreqFlags, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_FLAGS_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_FLAGS_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iflags",       /*tp_name*/
	sizeof(pyint_obj_I_FLAGS), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_FLAGS_methods,        /*tp_methods*/
	pyint_I_FLAGS_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_GAME from python {{{ */

typedef struct {
	PyObject_HEAD
	Igame *i;
} pyint_obj_I_GAME;

local PyObject *
pyint_method_I_GAME_SetFreq(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	me->i->SetFreq(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_SetShip(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	me->i->SetShip(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_SetFreqAndShip(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	me->i->SetFreqAndShip(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_WarpTo(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	const Target *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_target, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	me->i->WarpTo(arg1_in, arg2_in, arg3_in);
	afree(arg1_in);
	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_GivePrize(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	const Target *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_target, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	me->i->GivePrize(arg1_in, arg2_in, arg3_in);
	afree(arg1_in);
	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_Lock(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	const Target *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "O&iii", cvt_p2c_target, &arg1_in, &arg2_in, &arg3_in, &arg4_in))
		return NULL;

	me->i->Lock(arg1_in, arg2_in, arg3_in, arg4_in);
	afree(arg1_in);
	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_Unlock(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	const Target *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_target, &arg1_in, &arg2_in))
		return NULL;

	me->i->Unlock(arg1_in, arg2_in);
	afree(arg1_in);
	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_LockArena(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	if (!PyArg_ParseTuple(args, "O&iiii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in))
		return NULL;

	me->i->LockArena(arg1_in, arg2_in, arg3_in, arg4_in, arg5_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_GAME_UnlockArena(pyint_obj_I_GAME *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	me->i->UnlockArena(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_GAME_methods[] = {
	{"SetFreq", (PyCFunction)pyint_method_I_GAME_SetFreq, METH_VARARGS, NULL },
	{"SetShip", (PyCFunction)pyint_method_I_GAME_SetShip, METH_VARARGS, NULL },
	{"SetFreqAndShip", (PyCFunction)pyint_method_I_GAME_SetFreqAndShip, METH_VARARGS, NULL },
	{"WarpTo", (PyCFunction)pyint_method_I_GAME_WarpTo, METH_VARARGS, NULL },
	{"GivePrize", (PyCFunction)pyint_method_I_GAME_GivePrize, METH_VARARGS, NULL },
	{"Lock", (PyCFunction)pyint_method_I_GAME_Lock, METH_VARARGS, NULL },
	{"Unlock", (PyCFunction)pyint_method_I_GAME_Unlock, METH_VARARGS, NULL },
	{"LockArena", (PyCFunction)pyint_method_I_GAME_LockArena, METH_VARARGS, NULL },
	{"UnlockArena", (PyCFunction)pyint_method_I_GAME_UnlockArena, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_GAME_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_GAME_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Igame",       /*tp_name*/
	sizeof(pyint_obj_I_GAME), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_GAME_methods,        /*tp_methods*/
	pyint_I_GAME_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_JACKPOT from python {{{ */

typedef struct {
	PyObject_HEAD
	Ijackpot *i;
} pyint_obj_I_JACKPOT;

local PyObject *
pyint_method_I_JACKPOT_ResetJP(pyint_obj_I_JACKPOT *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return NULL;

	me->i->ResetJP(arg1_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_JACKPOT_AddJP(pyint_obj_I_JACKPOT *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_arena, &arg1_in, &arg2_in))
		return NULL;

	me->i->AddJP(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_JACKPOT_GetJP(pyint_obj_I_JACKPOT *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return NULL;

	ret = me->i->GetJP(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_JACKPOT_SetJP(pyint_obj_I_JACKPOT *me, PyObject *args)
{
	PyObject *out;
	Arena *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_arena, &arg1_in, &arg2_in))
		return NULL;

	me->i->SetJP(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_JACKPOT_methods[] = {
	{"ResetJP", (PyCFunction)pyint_method_I_JACKPOT_ResetJP, METH_VARARGS, NULL },
	{"AddJP", (PyCFunction)pyint_method_I_JACKPOT_AddJP, METH_VARARGS, NULL },
	{"GetJP", (PyCFunction)pyint_method_I_JACKPOT_GetJP, METH_VARARGS, NULL },
	{"SetJP", (PyCFunction)pyint_method_I_JACKPOT_SetJP, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_JACKPOT_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_JACKPOT_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ijackpot",       /*tp_name*/
	sizeof(pyint_obj_I_JACKPOT), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_JACKPOT_methods,        /*tp_methods*/
	pyint_I_JACKPOT_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_POINTS_KOTH from python {{{ */

typedef struct {
	PyObject_HEAD
	Ipoints_koth *i;
} pyint_obj_I_POINTS_KOTH;

local PyObject *
pyint_method_I_POINTS_KOTH_GetPoints(pyint_obj_I_POINTS_KOTH *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	ret = me->i->GetPoints(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_POINTS_KOTH_methods[] = {
	{"GetPoints", (PyCFunction)pyint_method_I_POINTS_KOTH_GetPoints, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_POINTS_KOTH_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_POINTS_KOTH_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ipoints_koth",       /*tp_name*/
	sizeof(pyint_obj_I_POINTS_KOTH), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_POINTS_KOTH_methods,        /*tp_methods*/
	pyint_I_POINTS_KOTH_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* implementing interface I_POINTS_KOTH in python {{{ */

local int pyint_func_I_POINTS_KOTH_GetPoints(Arena *arg1_in, int arg2_in, int arg3_in)
{
	PyObject *args, *out = NULL;
	int ret;
	args = Py_BuildValue("(O&ii)", cvt_c2p_arena, arg1_in, arg2_in, arg3_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function GetPoints in interface I_POINTS_KOTH");
		return 0;
	}

	out = call_gen_py_interface(PYINTPREFIX I_POINTS_KOTH, 0, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function GetPoints in interface I_POINTS_KOTH");
		return 0;
	}

	if (!PyArg_ParseTuple(out, "i", &ret))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function GetPoints in interface I_POINTS_KOTH");
		return 0;
	}

	Py_XDECREF(out);
	return ret;
}

local struct Ipoints_koth pyint_int_I_POINTS_KOTH = {
	INTERFACE_HEAD_INIT(I_POINTS_KOTH, "pyint-I_POINTS_KOTH")
	pyint_func_I_POINTS_KOTH_GetPoints
};


/* }}} */

/* using interface I_LOGMAN from python {{{ */

typedef struct {
	PyObject_HEAD
	Ilogman *i;
} pyint_obj_I_LOGMAN;

local PyObject *
pyint_method_I_LOGMAN_Log(pyint_obj_I_LOGMAN *me, PyObject *args)
{
	PyObject *out;
	int arg1_in;
	const char *arg2_infmt;
	if (!PyArg_ParseTuple(args, "is", &arg1_in, &arg2_infmt))
		return NULL;

	me->i->Log(arg1_in, "%s", arg2_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_LOGMAN_LogA(pyint_obj_I_LOGMAN *me, PyObject *args)
{
	PyObject *out;
	int arg1_in;
	const char *arg2_in;
	Arena *arg3_in;
	const char *arg4_infmt;
	if (!PyArg_ParseTuple(args, "isO&s", &arg1_in, &arg2_in, cvt_p2c_arena, &arg3_in, &arg4_infmt))
		return NULL;

	me->i->LogA(arg1_in, arg2_in, arg3_in, "%s", arg4_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_LOGMAN_LogP(pyint_obj_I_LOGMAN *me, PyObject *args)
{
	PyObject *out;
	int arg1_in;
	const char *arg2_in;
	Player *arg3_in;
	const char *arg4_infmt;
	if (!PyArg_ParseTuple(args, "isO&s", &arg1_in, &arg2_in, cvt_p2c_player, &arg3_in, &arg4_infmt))
		return NULL;

	me->i->LogP(arg1_in, arg2_in, arg3_in, "%s", arg4_infmt);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_LOGMAN_FilterLog(pyint_obj_I_LOGMAN *me, PyObject *args)
{
	PyObject *out;
	int ret;
	const char *arg1_in;
	const char *arg2_in;
	if (!PyArg_ParseTuple(args, "ss", &arg1_in, &arg2_in))
		return NULL;

	ret = me->i->FilterLog(arg1_in, arg2_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_LOGMAN_methods[] = {
	{"Log", (PyCFunction)pyint_method_I_LOGMAN_Log, METH_VARARGS, NULL },
	{"LogA", (PyCFunction)pyint_method_I_LOGMAN_LogA, METH_VARARGS, NULL },
	{"LogP", (PyCFunction)pyint_method_I_LOGMAN_LogP, METH_VARARGS, NULL },
	{"FilterLog", (PyCFunction)pyint_method_I_LOGMAN_FilterLog, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_LOGMAN_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_LOGMAN_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Ilogman",       /*tp_name*/
	sizeof(pyint_obj_I_LOGMAN), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_LOGMAN_methods,        /*tp_methods*/
	pyint_I_LOGMAN_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_MAPDATA from python {{{ */

typedef struct {
	PyObject_HEAD
	Imapdata *i;
} pyint_obj_I_MAPDATA;

local PyObject *
pyint_method_I_MAPDATA_GetMapFilename(pyint_obj_I_MAPDATA *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	charbuf arg2_out = {0};
	const char *arg4_in;
	if (!PyArg_ParseTuple(args, "O&z", cvt_p2c_arena, &arg1_in, &arg4_in))
		return NULL;

	ret = me->i->GetMapFilename(arg1_in, arg2_out, 1024, arg4_in);

	out = Py_BuildValue("is", ret, arg2_out);

	return out;
}

local PyObject *
pyint_method_I_MAPDATA_GetFlagCount(pyint_obj_I_MAPDATA *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_arena, &arg1_in))
		return NULL;

	ret = me->i->GetFlagCount(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_MAPDATA_GetTile(pyint_obj_I_MAPDATA *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	ret = me->i->GetTile(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_MAPDATA_methods[] = {
	{"GetMapFilename", (PyCFunction)pyint_method_I_MAPDATA_GetMapFilename, METH_VARARGS, NULL },
	{"GetFlagCount", (PyCFunction)pyint_method_I_MAPDATA_GetFlagCount, METH_VARARGS, NULL },
	{"GetTile", (PyCFunction)pyint_method_I_MAPDATA_GetTile, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_MAPDATA_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_MAPDATA_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Imapdata",       /*tp_name*/
	sizeof(pyint_obj_I_MAPDATA), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_MAPDATA_methods,        /*tp_methods*/
	pyint_I_MAPDATA_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_MAPNEWSDL from python {{{ */

typedef struct {
	PyObject_HEAD
	Imodman *i;
} pyint_obj_I_MAPNEWSDL;

local PyObject *
pyint_method_I_MAPNEWSDL_LoadModule(pyint_obj_I_MAPNEWSDL *me, PyObject *args)
{
	PyObject *out;
	int ret;
	const char *arg1_in;
	if (!PyArg_ParseTuple(args, "s", &arg1_in))
		return NULL;

	ret = me->i->LoadModule(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_MAPNEWSDL_UnloadModule(pyint_obj_I_MAPNEWSDL *me, PyObject *args)
{
	PyObject *out;
	int ret;
	const char *arg1_in;
	if (!PyArg_ParseTuple(args, "s", &arg1_in))
		return NULL;

	ret = me->i->UnloadModule(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_MAPNEWSDL_methods[] = {
	{"LoadModule", (PyCFunction)pyint_method_I_MAPNEWSDL_LoadModule, METH_VARARGS, NULL },
	{"UnloadModule", (PyCFunction)pyint_method_I_MAPNEWSDL_UnloadModule, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_MAPNEWSDL_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_MAPNEWSDL_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Imodman",       /*tp_name*/
	sizeof(pyint_obj_I_MAPNEWSDL), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_MAPNEWSDL_methods,        /*tp_methods*/
	pyint_I_MAPNEWSDL_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_NET from python {{{ */

typedef struct {
	PyObject_HEAD
	Inet *i;
} pyint_obj_I_NET;

local PyObject *
pyint_method_I_NET_GetListenData(pyint_obj_I_NET *me, PyObject *args)
{
	PyObject *out;
	int ret;
	int arg1_in;
	int  arg2_out = 0;
	charbuf arg3_out = {0};
	if (!PyArg_ParseTuple(args, "i", &arg1_in))
		return NULL;

	ret = me->i->GetListenData(arg1_in, &arg2_out, arg3_out, 1024);

	out = Py_BuildValue("iis", ret, arg2_out, arg3_out);

	return out;
}

local PyObject *
pyint_method_I_NET_GetLDPopulation(pyint_obj_I_NET *me, PyObject *args)
{
	PyObject *out;
	int ret;
	const char *arg1_in;
	if (!PyArg_ParseTuple(args, "s", &arg1_in))
		return NULL;

	ret = me->i->GetLDPopulation(arg1_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_NET_methods[] = {
	{"GetListenData", (PyCFunction)pyint_method_I_NET_GetListenData, METH_VARARGS, NULL },
	{"GetLDPopulation", (PyCFunction)pyint_method_I_NET_GetLDPopulation, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_NET_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_NET_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Inet",       /*tp_name*/
	sizeof(pyint_obj_I_NET), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_NET_methods,        /*tp_methods*/
	pyint_I_NET_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_PERIODIC_POINTS from python {{{ */

typedef struct {
	PyObject_HEAD
	Iperiodicpoints *i;
} pyint_obj_I_PERIODIC_POINTS;

local PyObject *
pyint_method_I_PERIODIC_POINTS_GetPoints(pyint_obj_I_PERIODIC_POINTS *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Arena *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	int arg5_in;
	if (!PyArg_ParseTuple(args, "O&iiii", cvt_p2c_arena, &arg1_in, &arg2_in, &arg3_in, &arg4_in, &arg5_in))
		return NULL;

	ret = me->i->GetPoints(arg1_in, arg2_in, arg3_in, arg4_in, arg5_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyMethodDef pyint_I_PERIODIC_POINTS_methods[] = {
	{"GetPoints", (PyCFunction)pyint_method_I_PERIODIC_POINTS_GetPoints, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_PERIODIC_POINTS_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_PERIODIC_POINTS_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iperiodicpoints",       /*tp_name*/
	sizeof(pyint_obj_I_PERIODIC_POINTS), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_PERIODIC_POINTS_methods,        /*tp_methods*/
	pyint_I_PERIODIC_POINTS_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* implementing interface I_PERIODIC_POINTS in python {{{ */

local int pyint_func_I_PERIODIC_POINTS_GetPoints(Arena *arg1_in, int arg2_in, int arg3_in, int arg4_in, int arg5_in)
{
	PyObject *args, *out = NULL;
	int ret;
	args = Py_BuildValue("(O&iiii)", cvt_c2p_arena, arg1_in, arg2_in, arg3_in, arg4_in, arg5_in);
	if (!args)
	{
		log_py_exception(L_ERROR, "python error building args for "
			"function GetPoints in interface I_PERIODIC_POINTS");
		return 0;
	}

	out = call_gen_py_interface(PYINTPREFIX I_PERIODIC_POINTS, 0, args);
	Py_DECREF(args);
	if (!out)
	{
		log_py_exception(L_ERROR, "python error calling "
			"function GetPoints in interface I_PERIODIC_POINTS");
		return 0;
	}

	if (!PyArg_ParseTuple(out, "i", &ret))
	{
		Py_XDECREF(out);
		log_py_exception(L_ERROR, "python error unpacking results of "
			"function GetPoints in interface I_PERIODIC_POINTS");
		return 0;
	}

	Py_XDECREF(out);
	return ret;
}

local struct Iperiodicpoints pyint_int_I_PERIODIC_POINTS = {
	INTERFACE_HEAD_INIT(I_PERIODIC_POINTS, "pyint-I_PERIODIC_POINTS")
	pyint_func_I_PERIODIC_POINTS_GetPoints
};


/* }}} */

/* using interface I_PLAYERDATA from python {{{ */

typedef struct {
	PyObject_HEAD
	Iplayerdata *i;
} pyint_obj_I_PLAYERDATA;

local PyObject *
pyint_method_I_PLAYERDATA_KickPlayer(pyint_obj_I_PLAYERDATA *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	if (!PyArg_ParseTuple(args, "O&", cvt_p2c_player, &arg1_in))
		return NULL;

	me->i->KickPlayer(arg1_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_PLAYERDATA_PidToPlayer(pyint_obj_I_PLAYERDATA *me, PyObject *args)
{
	PyObject *out;
	Player *ret;
	int arg1_in;
	if (!PyArg_ParseTuple(args, "i", &arg1_in))
		return NULL;

	ret = me->i->PidToPlayer(arg1_in);

	out = Py_BuildValue("O&", cvt_c2p_player, ret);

	return out;
}

local PyObject *
pyint_method_I_PLAYERDATA_FindPlayer(pyint_obj_I_PLAYERDATA *me, PyObject *args)
{
	PyObject *out;
	Player *ret;
	const char *arg1_in;
	if (!PyArg_ParseTuple(args, "s", &arg1_in))
		return NULL;

	ret = me->i->FindPlayer(arg1_in);

	out = Py_BuildValue("O&", cvt_c2p_player, ret);

	return out;
}

local PyMethodDef pyint_I_PLAYERDATA_methods[] = {
	{"KickPlayer", (PyCFunction)pyint_method_I_PLAYERDATA_KickPlayer, METH_VARARGS, NULL },
	{"PidToPlayer", (PyCFunction)pyint_method_I_PLAYERDATA_PidToPlayer, METH_VARARGS, NULL },
	{"FindPlayer", (PyCFunction)pyint_method_I_PLAYERDATA_FindPlayer, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_PLAYERDATA_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_PLAYERDATA_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iplayerdata",       /*tp_name*/
	sizeof(pyint_obj_I_PLAYERDATA), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_PLAYERDATA_methods,        /*tp_methods*/
	pyint_I_PLAYERDATA_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_PRNG from python {{{ */

typedef struct {
	PyObject_HEAD
	Iprng *i;
} pyint_obj_I_PRNG;

local PyObject *
pyint_method_I_PRNG_Number(pyint_obj_I_PRNG *me, PyObject *args)
{
	PyObject *out;
	int ret;
	int arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "ii", &arg1_in, &arg2_in))
		return NULL;

	ret = me->i->Number(arg1_in, arg2_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_PRNG_Rand(pyint_obj_I_PRNG *me, PyObject *args)
{
	PyObject *out;
	int ret;
	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	ret = me->i->Rand();

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_PRNG_Uniform(pyint_obj_I_PRNG *me, PyObject *args)
{
	PyObject *out;
	double ret;
	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	ret = me->i->Uniform();

	out = Py_BuildValue("d", ret);

	return out;
}

local PyMethodDef pyint_I_PRNG_methods[] = {
	{"Number", (PyCFunction)pyint_method_I_PRNG_Number, METH_VARARGS, NULL },
	{"Rand", (PyCFunction)pyint_method_I_PRNG_Rand, METH_VARARGS, NULL },
	{"Uniform", (PyCFunction)pyint_method_I_PRNG_Uniform, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_PRNG_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_PRNG_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Iprng",       /*tp_name*/
	sizeof(pyint_obj_I_PRNG), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_PRNG_methods,        /*tp_methods*/
	pyint_I_PRNG_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

/* using interface I_STATS from python {{{ */

typedef struct {
	PyObject_HEAD
	Istats *i;
} pyint_obj_I_STATS;

local PyObject *
pyint_method_I_STATS_IncrementStat(pyint_obj_I_STATS *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	me->i->IncrementStat(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_STATS_StartTimer(pyint_obj_I_STATS *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	me->i->StartTimer(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_STATS_StopTimer(pyint_obj_I_STATS *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	me->i->StopTimer(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_STATS_SetStat(pyint_obj_I_STATS *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	int arg4_in;
	if (!PyArg_ParseTuple(args, "O&iii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in, &arg4_in))
		return NULL;

	me->i->SetStat(arg1_in, arg2_in, arg3_in, arg4_in);

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_STATS_GetStat(pyint_obj_I_STATS *me, PyObject *args)
{
	PyObject *out;
	int ret;
	Player *arg1_in;
	int arg2_in;
	int arg3_in;
	if (!PyArg_ParseTuple(args, "O&ii", cvt_p2c_player, &arg1_in, &arg2_in, &arg3_in))
		return NULL;

	ret = me->i->GetStat(arg1_in, arg2_in, arg3_in);

	out = Py_BuildValue("i", ret);

	return out;
}

local PyObject *
pyint_method_I_STATS_SendUpdates(pyint_obj_I_STATS *me, PyObject *args)
{
	PyObject *out;

	if (!PyArg_ParseTuple(args, ""))
		return NULL;

	me->i->SendUpdates();

	out = Py_BuildValue("");

	return out;
}

local PyObject *
pyint_method_I_STATS_ScoreReset(pyint_obj_I_STATS *me, PyObject *args)
{
	PyObject *out;
	Player *arg1_in;
	int arg2_in;
	if (!PyArg_ParseTuple(args, "O&i", cvt_p2c_player, &arg1_in, &arg2_in))
		return NULL;

	me->i->ScoreReset(arg1_in, arg2_in);

	out = Py_BuildValue("");

	return out;
}

local PyMethodDef pyint_I_STATS_methods[] = {
	{"IncrementStat", (PyCFunction)pyint_method_I_STATS_IncrementStat, METH_VARARGS, NULL },
	{"StartTimer", (PyCFunction)pyint_method_I_STATS_StartTimer, METH_VARARGS, NULL },
	{"StopTimer", (PyCFunction)pyint_method_I_STATS_StopTimer, METH_VARARGS, NULL },
	{"SetStat", (PyCFunction)pyint_method_I_STATS_SetStat, METH_VARARGS, NULL },
	{"GetStat", (PyCFunction)pyint_method_I_STATS_GetStat, METH_VARARGS, NULL },
	{"SendUpdates", (PyCFunction)pyint_method_I_STATS_SendUpdates, METH_VARARGS, NULL },
	{"ScoreReset", (PyCFunction)pyint_method_I_STATS_ScoreReset, METH_VARARGS, NULL },
	{NULL}
};

local PyMemberDef pyint_I_STATS_members[] = {
	{NULL}
};

local PyTypeObject pyint_I_STATS_type = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Istats",       /*tp_name*/
	sizeof(pyint_obj_I_STATS), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)pyint_generic_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,                         /*tp_doc */
	0,                         /*tp_traverse */
	0,                         /*tp_clear */
	0,                         /*tp_richcompare*/
	0,                         /*tp_weaklistoffset*/
	0,                         /*tp_iter*/
	0,                         /*tp_iternext*/
	pyint_I_STATS_methods,        /*tp_methods*/
	pyint_I_STATS_members,        /*tp_members*/
	/* rest are null */
};

/* }}} */

local HashTable *pyint_ints;
local HashTable *pyint_impl_ints;

local void init_py_interfaces(void)
{
	pyint_ints = HashAlloc();
	pyint_impl_ints = HashAlloc();
	if (PyType_Ready(&pyint_I_ARENAMAN_type) < 0) return;
	HashReplace(pyint_ints, I_ARENAMAN, &pyint_I_ARENAMAN_type);
	if (PyType_Ready(&pyint_I_ARENAPLACE_type) < 0) return;
	HashReplace(pyint_ints, I_ARENAPLACE, &pyint_I_ARENAPLACE_type);
	HashReplace(pyint_impl_ints, PYINTPREFIX I_ARENAPLACE, &pyint_int_I_ARENAPLACE);
	if (PyType_Ready(&pyint_I_BALLS_type) < 0) return;
	HashReplace(pyint_ints, I_BALLS, &pyint_I_BALLS_type);
	if (PyType_Ready(&pyint_I_BANNERS_type) < 0) return;
	HashReplace(pyint_ints, I_BANNERS, &pyint_I_BANNERS_type);
	if (PyType_Ready(&pyint_I_BRICKS_type) < 0) return;
	HashReplace(pyint_ints, I_BRICKS, &pyint_I_BRICKS_type);
	if (PyType_Ready(&pyint_I_CAPMAN_type) < 0) return;
	HashReplace(pyint_ints, I_CAPMAN, &pyint_I_CAPMAN_type);
	HashReplace(pyint_impl_ints, PYINTPREFIX I_CAPMAN, &pyint_int_I_CAPMAN);
	if (PyType_Ready(&pyint_I_GROUPMAN_type) < 0) return;
	HashReplace(pyint_ints, I_GROUPMAN, &pyint_I_GROUPMAN_type);
	if (PyType_Ready(&pyint_I_CHAT_type) < 0) return;
	HashReplace(pyint_ints, I_CHAT, &pyint_I_CHAT_type);
	if (PyType_Ready(&pyint_I_CONFIG_type) < 0) return;
	HashReplace(pyint_ints, I_CONFIG, &pyint_I_CONFIG_type);
	if (PyType_Ready(&pyint_I_FREQMAN_type) < 0) return;
	HashReplace(pyint_ints, I_FREQMAN, &pyint_I_FREQMAN_type);
	HashReplace(pyint_impl_ints, PYINTPREFIX I_FREQMAN, &pyint_int_I_FREQMAN);
	if (PyType_Ready(&pyint_I_FAKE_type) < 0) return;
	HashReplace(pyint_ints, I_FAKE, &pyint_I_FAKE_type);
	if (PyType_Ready(&pyint_I_FLAGS_type) < 0) return;
	HashReplace(pyint_ints, I_FLAGS, &pyint_I_FLAGS_type);
	if (PyType_Ready(&pyint_I_GAME_type) < 0) return;
	HashReplace(pyint_ints, I_GAME, &pyint_I_GAME_type);
	if (PyType_Ready(&pyint_I_JACKPOT_type) < 0) return;
	HashReplace(pyint_ints, I_JACKPOT, &pyint_I_JACKPOT_type);
	if (PyType_Ready(&pyint_I_POINTS_KOTH_type) < 0) return;
	HashReplace(pyint_ints, I_POINTS_KOTH, &pyint_I_POINTS_KOTH_type);
	HashReplace(pyint_impl_ints, PYINTPREFIX I_POINTS_KOTH, &pyint_int_I_POINTS_KOTH);
	if (PyType_Ready(&pyint_I_LOGMAN_type) < 0) return;
	HashReplace(pyint_ints, I_LOGMAN, &pyint_I_LOGMAN_type);
	if (PyType_Ready(&pyint_I_MAPDATA_type) < 0) return;
	HashReplace(pyint_ints, I_MAPDATA, &pyint_I_MAPDATA_type);
	if (PyType_Ready(&pyint_I_MAPNEWSDL_type) < 0) return;
	HashReplace(pyint_ints, I_MAPNEWSDL, &pyint_I_MAPNEWSDL_type);
	if (PyType_Ready(&pyint_I_NET_type) < 0) return;
	HashReplace(pyint_ints, I_NET, &pyint_I_NET_type);
	if (PyType_Ready(&pyint_I_PERIODIC_POINTS_type) < 0) return;
	HashReplace(pyint_ints, I_PERIODIC_POINTS, &pyint_I_PERIODIC_POINTS_type);
	HashReplace(pyint_impl_ints, PYINTPREFIX I_PERIODIC_POINTS, &pyint_int_I_PERIODIC_POINTS);
	if (PyType_Ready(&pyint_I_PLAYERDATA_type) < 0) return;
	HashReplace(pyint_ints, I_PLAYERDATA, &pyint_I_PLAYERDATA_type);
	if (PyType_Ready(&pyint_I_PRNG_type) < 0) return;
	HashReplace(pyint_ints, I_PRNG, &pyint_I_PRNG_type);
	if (PyType_Ready(&pyint_I_STATS_type) < 0) return;
	HashReplace(pyint_ints, I_STATS, &pyint_I_STATS_type);
}

local void deinit_py_interfaces(void)
{
	HashFree(pyint_ints);
	HashFree(pyint_impl_ints);
}
