
/* dist: public */

#include <signal.h>
#include <pthread.h>

#include "Python.h"
#include "structmember.h"

#include "asss.h"

#include "persist.h"
#include "log_file.h"
#include "net-client.h"
#include "filetrans.h"
#include "fake.h"
#include "clientset.h"
#include "cfghelp.h"
#include "banners.h"
#include "billing.h"
#include "jackpot.h"
#include "koth.h"
#include "watchdamage.h"
#include "objects.h"
#include "reldb.h"

#include "contrib/turf_reward.h"


#define PYCBPREFIX "PY-"
#define PYINTPREFIX "PY-"

#ifndef WIN32
#define CHECK_SIGS
#endif


/* forward decls */

typedef struct PlayerObject
{
	PyObject_HEAD
	Player *p;
	PyObject *dict;
} PlayerObject;

typedef struct ArenaObject
{
	PyObject_HEAD
	Arena *a;
	PyObject *dict;
} ArenaObject;

local PyTypeObject PlayerType;
local PyTypeObject ArenaType;


typedef struct pdata
{
	PlayerObject *obj;
} pdata;

typedef struct adata
{
	ArenaObject *obj;
} adata;


/* our locals */
local Imodman *mm;
local Iplayerdata *pd;
local Iarenaman *aman;
local Ilogman *lm;
local Iconfig *cfg;
local Icmdman *cmd;
local Ipersist *persist;
local Imainloop *mainloop;

local int pdkey, adkey;
local int mods_loaded;

local PyObject *cPickle;
local PyObject *sysdict;

#ifdef not_yet
local pthread_mutex_t pymtx = PTHREAD_MUTEX_INITIALIZER;
#define LOCK() pthread_mutex_lock(&pymtx)
#define UNLOCK() pthread_mutex_unlock(&pymtx)
#endif


/* utility functions */

local void log_py_exception(char lev, const char *msg)
{
	if (PyErr_Occurred())
	{
		if (msg)
			lm->Log(lev, "<pymod> %s", msg);
		/* FIXME: this only goes to stderr */
		PyErr_Print();

		/* this is pretty disgusting. sorry, but it seems to be
		 * necessary when using PyErr_Print to avoid leaking objects. */
		PyDict_DelItemString(sysdict, "last_traceback");
	}
}


/* converters */

local PyObject * cvt_c2p_player(Player *p)
{
	PyObject *o;
	if (p)
		o = (PyObject*)((pdata*)PPDATA(p, pdkey))->obj;
	else
		o = Py_None;
	Py_XINCREF(o);
	return o;
}

local int cvt_p2c_player(PyObject *o, Player **pp)
{
	if (o == Py_None)
	{
		*pp = NULL;
		return TRUE;
	}
	else if (o->ob_type == &PlayerType)
	{
		*pp = ((PlayerObject*)o)->p;
		return TRUE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "arg isn't a player object");
		return FALSE;
	}
}


local PyObject * cvt_c2p_arena(Arena *a)
{
	PyObject *o;
	if (a)
		o = (PyObject*)((adata*)P_ARENA_DATA(a, adkey))->obj;
	else
		o = Py_None;
	Py_XINCREF(o);
	return o;
}

local int cvt_p2c_arena(PyObject *o, Arena **ap)
{
	if (o == Py_None)
	{
		*ap = NULL;
		return TRUE;
	}
	else if (o->ob_type == &ArenaType)
	{
		*ap = ((ArenaObject*)o)->a;
		return TRUE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "arg isn't a arena object");
		return FALSE;
	}
}


local PyObject * cvt_c2p_banner(Banner *b)
{
	return PyString_FromStringAndSize((const char *)b, sizeof(*b));
}

local int cvt_p2c_banner(PyObject *o, Banner **bp)
{
	char *buf;
	int len = -1;
	PyString_AsStringAndSize(o, &buf, &len);
	if (len == sizeof(Banner))
	{
		*bp = (Banner*)buf;
		return TRUE;
	}
	else
		return FALSE;
}


local PyObject * cvt_c2p_target(Target *t)
{
	/* the representation of a target in python depends on which sort of
	 * target it is, and is generally lax. none is none. players and
	 * arenas are represented as themselves. freqs are (arena, freq)
	 * tuples. the zone is a special string. a list is a python list of
	 * players. */
	/* NOTE: lists aren't supported yet. */
	switch (t->type)
	{
		case T_NONE:
			Py_INCREF(Py_None);
			return Py_None;

		case T_PLAYER:
			return cvt_c2p_player(t->u.p);

		case T_ARENA:
			return cvt_c2p_arena(t->u.arena);

		case T_FREQ:
		{
			PyObject *obj = PyTuple_New(2);
			PyTuple_SET_ITEM(obj, 0, cvt_c2p_arena(t->u.freq.arena));
			PyTuple_SET_ITEM(obj, 1, PyInt_FromLong(t->u.freq.freq));
			return obj;
		}

		case T_ZONE:
			return PyString_FromString("zone");

		case T_LIST:
			return NULL;

		default:
			return NULL;
	}
}

local int cvt_p2c_target(PyObject *o, Target **tp)
{
	Target *t;
	if (o->ob_type == &PlayerType)
	{
		t = amalloc(sizeof(*t));
		t->type = T_PLAYER;
		t->u.p = ((PlayerObject*)o)->p;
		*tp = t;
		return TRUE;
	}
	else if (o->ob_type == &ArenaType)
	{
		t = amalloc(sizeof(*t));
		t->type = T_ARENA;
		t->u.arena = ((ArenaObject*)o)->a;
		*tp = t;
		return TRUE;
	}
	else if (PyTuple_Check(o))
	{
		t = amalloc(sizeof(*t));
		t->type = T_FREQ;
		t->u.arena = ((ArenaObject*)o)->a;
		if (!PyArg_ParseTuple(o, "O&i",
					cvt_p2c_arena, &t->u.freq.arena,
					&t->u.freq.freq))
		{
			afree(t);
			return FALSE;
		}
		*tp = t;
		return TRUE;
	}
	else if (PyString_Check(o))
	{
		const char *c = PyString_AsString(o);
		if (strcmp(c, "zone") == 0)
		{
			t = amalloc(sizeof(*t));
			t->type = T_ZONE;
			return TRUE;
		}
		else
			return FALSE;
	}
#if 0
	else if (PyList_Check(o))
	{
	}
#endif
	else if (o == Py_None)
	{
		t = amalloc(sizeof(*t));
		t->type = T_NONE;
		return TRUE;
	}
	else
		return FALSE;
}


local void close_config_file(void *v)
{
	cfg->CloseConfigFile(v);
}

local PyObject * cvt_c2p_config(ConfigHandle ch)
{
	cfg->AddRef(ch);
	return PyCObject_FromVoidPtr(ch, close_config_file);
}

local int cvt_p2c_config(PyObject *o, ConfigHandle *chp)
{
	if (o == Py_None)
	{
		*chp = GLOBAL;
		return TRUE;
	}
	else if (PyCObject_Check(o))
	{
		*chp = PyCObject_AsVoidPtr(o);
		return TRUE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "arg isn't a config handle object");
		return FALSE;
	}
}


#if 0
local PyObject * cvt_c2p_cb_string_to_void(void (*func)(const char *))
{
}

local void helper_string_to_void(const char *s)
{
	/* convert s to a python string and put it in a tuple
	 * get the function this is supposed to represent
	 * from where?
	 * thread-local storage
	 * invoke the function on that tuple */
}

local int cvt_p2c_cb_string_to_void(PyObject *o, void (**func)(const char *))
{
}
#endif

/* defining the asss module */

/* players */

local void Player_dealloc(PlayerObject *obj)
{
	Py_XDECREF(obj->dict);
	PyObject_Del(obj);
}


local PyObject *Player_get_pid(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyInt_FromLong(p->pid);
}

local PyObject *Player_get_status(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyInt_FromLong(p->status);
}

local PyObject *Player_get_type(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyInt_FromLong(p->type);
}

local PyObject *Player_get_arena(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	PyObject *ao = p->arena ?
		(PyObject*)(((adata*)P_ARENA_DATA(p->arena, adkey))->obj) :
		Py_None;
	Py_XINCREF(ao);
	return ao;
}

local PyObject *Player_get_name(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyString_FromString(p->name);
}

local PyObject *Player_get_squad(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyString_FromString(p->squad);
}

local PyObject *Player_get_ship(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyInt_FromLong(p->p_ship);
}

local PyObject *Player_get_freq(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyInt_FromLong(p->p_freq);
}

local PyObject *Player_get_res(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return Py_BuildValue("ii", p->xres, p->yres);
}

local PyObject *Player_get_onfor(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	int tm = TICK_DIFF(current_ticks(), p->connecttime);
	return PyLong_FromLong(tm/100);
}

local PyObject *Player_get_position(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return Py_BuildValue("iiiiiii",
			p->position.x,
			p->position.y,
			p->position.xspeed,
			p->position.yspeed,
			p->position.rotation,
			p->position.bounty,
			p->position.status);
}

local PyObject *Player_get_macid(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyLong_FromLong(p->macid);
}

local PyObject *Player_get_ipaddr(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyString_FromString(p->ipaddr);
}

local PyObject *Player_get_connectas(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	if (p->connectas)
		return PyString_FromString(p->connectas);
	else
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
}

local PyObject *Player_get_authenticated(PyObject *obj, void *v)
{
	Player *p = ((PlayerObject*)obj)->p;
	return PyInt_FromLong(p->flags.authenticated);
}


local PyGetSetDef Player_getseters[] =
{
#define SIMPLE_GETTER(n, doc) { #n, Player_get_ ## n, NULL, doc, NULL },
	SIMPLE_GETTER(pid, "player id")
	SIMPLE_GETTER(status, "current status")
	SIMPLE_GETTER(type, "client type (e.g., cont, 1.34, chat)")
	SIMPLE_GETTER(arena, "current arena")
	SIMPLE_GETTER(name, "player name")
	SIMPLE_GETTER(squad, "player squad")
	SIMPLE_GETTER(ship, "player ship")
	SIMPLE_GETTER(freq, "player freq")
	SIMPLE_GETTER(res, "screen resolution, returns tuple (x,y)")
	SIMPLE_GETTER(onfor, "seconds since login")
	SIMPLE_GETTER(position, "current position, returns tuple (x, y, v_x, v_y, rotation, bounty, status)")
	SIMPLE_GETTER(macid, "machine id")
	SIMPLE_GETTER(ipaddr, "client ip address")
	SIMPLE_GETTER(connectas, "which virtual server the client connected to")
	SIMPLE_GETTER(authenticated, "whether the client has been authenticated")
#undef SIMPLE_GETTER
	{NULL}
};

local PyTypeObject PlayerType =
{
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Player",             /*tp_name*/
	sizeof(PlayerObject),      /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Player_dealloc,/*tp_dealloc*/
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
	"Player object",           /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	0,                         /* tp_methods */
	0,                         /* tp_members */
	Player_getseters,          /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	offsetof(PlayerObject, dict),  /* tp_dictoffset */
	0,                         /* tp_init */
	0,                         /* tp_alloc */
	0,                         /* tp_new */
};


/* arenas */

local void Arena_dealloc(ArenaObject *obj)
{
	Py_XDECREF(obj->dict);
	PyObject_Del(obj);
}


local PyObject *Arena_get_status(PyObject *obj, void *v)
{
	Arena *a = ((ArenaObject*)obj)->a;
	return PyInt_FromLong(a->status);
}

local PyObject *Arena_get_name(PyObject *obj, void *v)
{
	Arena *a = ((ArenaObject*)obj)->a;
	return PyString_FromString(a->name);
}

local PyObject *Arena_get_basename(PyObject *obj, void *v)
{
	Arena *a = ((ArenaObject*)obj)->a;
	return PyString_FromString(a->basename);
}

local PyObject *Arena_get_cfg(PyObject *obj, void *v)
{
	Arena *a = ((ArenaObject*)obj)->a;
	return cvt_c2p_config(a->cfg);
}


local PyGetSetDef Arena_getseters[] =
{
#define SIMPLE_GETTER(n, doc) { #n, Arena_get_ ## n, NULL, doc, NULL },
	SIMPLE_GETTER(status, "current status")
	SIMPLE_GETTER(name, "arena name")
	SIMPLE_GETTER(basename, "arena basename (without a number at the end)")
	SIMPLE_GETTER(cfg, "arena config file handle")
#undef SIMPLE_GETTER
	{NULL}
};

local PyTypeObject ArenaType =
{
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"asss.Arena",              /*tp_name*/
	sizeof(ArenaObject),       /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Arena_dealloc, /*tp_dealloc*/
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
	"Arena object",            /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	0,                         /* tp_methods */
	0,                         /* tp_members */
	Arena_getseters,           /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	offsetof(ArenaObject, dict),  /* tp_dictoffset */
	0,                         /* tp_init */
	0,                         /* tp_alloc */
	0,                         /* tp_new */
};




/* other types */




/* associating asss objects with python objects */


local void call_gen_py_callbacks(const char *cbid, PyObject *args)
{
	LinkedList cbs = LL_INITIALIZER;
	Link *l;
	PyObject *ret;

	mm->LookupCallback(cbid, mm->GetArenaOfCurrentCallback(), &cbs);

	for (l = LLGetHead(&cbs); l; l = l->next)
	{
		ret = PyObject_Call(l->data, args, NULL);
		if (ret)
			Py_DECREF(ret);
		else
		{
			lm->Log(L_ERROR, "<pymod> python error in callback for '%s'", cbid+3);
			log_py_exception(L_ERROR, NULL);
		}
	}
	LLEmpty(&cbs);
}


typedef struct {
	PyObject_HEAD
	void *i;
} pyint_generic_interface_object;

typedef struct {
	INTERFACE_HEAD_DECL
#define PY_INT_MAGIC 0x13806503
	unsigned py_int_magic;
	void *real_interface;
	Arena *arena;
	PyObject *funcs;
} pyint_generic_interface;


local PyObject * call_gen_py_interface(const char *iid, int idx, PyObject *args)
{
	pyint_generic_interface *i;
	PyObject *ret = NULL;

	/* this method of getting the arena isn't very good. it should work
	 * as long as people use interfaces in normal ways, but it's
	 * somewhat fragile. the alternatives are adding a closure parameter
	 * to all or most interface functions, or copying and modifying code
	 * in memory. */
	i = mm->GetInterface(iid, mm->GetArenaOfLastInterfaceRequest());
	if (i && i->funcs && i->py_int_magic == PY_INT_MAGIC)
	{
		PyObject *func = PyTuple_GetItem(i->funcs, idx);
		if (func)
			ret = PyObject_Call(func, args, NULL);
	}
	/* the refcount on a python interface struct is meaningless, so
	 * don't bother releasing it. */
	return ret;
}

/* this is where most of the generated code gets inserted */
#include "py_callbacks.inc"
#include "py_interfaces.inc"


local void py_newplayer(Player *p, int isnew)
{
	pdata *d = PPDATA(p, pdkey);
	if (isnew)
	{
		d->obj = PyObject_New(PlayerObject, &PlayerType);
		d->obj->p = p;
		d->obj->dict = PyDict_New();
	}
	else
	{
		if (d->obj->ob_refcnt != 1)
			lm->Log(L_ERROR, "<pymod> there are %d remaining references to a player object!",
					d->obj->ob_refcnt);

		/* this stuff would usually be done in dealloc, but I want to
		 * make sure player objects for players who are gone are
		 * useless. specifically, all data assigned to misc. attributes
		 * of the player object should be deallocated and the object
		 * shouldn't be able to reference the asss player struct anymore
		 * (because it doesn't exist). */
		d->obj->p = NULL;
		Py_DECREF(d->obj->dict);
		d->obj->dict = NULL;

		/* this releases our reference */
		Py_DECREF(d->obj);
		d->obj = NULL;
	}
}


local void py_aaction(Arena *a, int action)
{
	adata *d = P_ARENA_DATA(a, adkey);
	PyObject *args;

	if (action == AA_CREATE)
	{
		d->obj = PyObject_New(ArenaObject, &ArenaType);
		d->obj->a = a;
		d->obj->dict = PyDict_New();
	}

	args = Py_BuildValue("Oi", d->obj, action);
	if (args)
	{
		call_gen_py_callbacks(PYCBPREFIX CB_ARENAACTION, args);
		Py_DECREF(args);
	}

	if (action == AA_DESTROY && d->obj)
	{
		if (d->obj->ob_refcnt != 1)
			lm->Log(L_ERROR, "<pymod> there are %d remaining references to an arena object!",
					d->obj->ob_refcnt);

		/* see notes for py_newplayer as to why this is done here. */
		d->obj->a = NULL;
		Py_DECREF(d->obj->dict);
		d->obj->dict = NULL;

		/* this releases our reference */
		Py_DECREF(d->obj);
		d->obj = NULL;
	}
}



struct callback_ticket
{
	PyObject *func;
	Arena *arena;
	char cbid[1];
};

local void destroy_callback_ticket(void *v)
{
	struct callback_ticket *t = v;
	Py_DECREF(t->func);
	mm->UnregCallback(t->cbid, t->func, t->arena);
	afree(t);
}

local PyObject *mthd_reg_callback(PyObject *self, PyObject *args)
{
	char *cbid;
	PyObject *func;
	Arena *arena = ALLARENAS;
	struct callback_ticket *tkt;

	if (!PyArg_ParseTuple(args, "sO|O&", &cbid, &func, cvt_p2c_arena, &arena))
		return NULL;

	if (!PyCallable_Check(func))
	{
		PyErr_SetString(PyExc_TypeError, "func isn't callable");
		return NULL;
	}

	Py_INCREF(func);

	mm->RegCallback(cbid, func, arena);

	tkt = amalloc(sizeof(*tkt) + strlen(cbid));
	tkt->func = func;
	tkt->arena = arena;
	strcpy(tkt->cbid, cbid);

	return PyCObject_FromVoidPtr(tkt, destroy_callback_ticket);
}


local PyObject *mthd_call_callback(PyObject *self, PyObject *args)
{
	char *cbid;
	PyObject *cbargs;
	Arena *arena = ALLARENAS;
	py_cb_caller func;

	if (!PyArg_ParseTuple(args, "sO|O&", &cbid, &cbargs, cvt_p2c_arena, &arena))
		return NULL;

	if (!PyTuple_Check(cbargs))
	{
		PyErr_SetString(PyExc_TypeError, "args isn't a tuple");
		return NULL;
	}

	func = HashGetOne(py_cb_callers, cbid);
	if (func)
		func(arena, cbargs);

	Py_INCREF(Py_None);
	return Py_None;
}


local PyObject *mthd_get_interface(PyObject *self, PyObject *args)
{
	char *iid;
	Arena *arena = ALLARENAS;
	pyint_generic_interface_object *o;
	PyTypeObject *typeo;

	if (!PyArg_ParseTuple(args, "s|O&", &iid, cvt_p2c_arena, &arena))
		return NULL;

	iid += 3;
	typeo = HashGetOne(pyint_ints, iid);
	if (typeo)
	{
		void *i = mm->GetInterface(iid, arena);
		if (i)
		{
			o = PyObject_New(pyint_generic_interface_object, typeo);
			o->i = i;
			return (PyObject*)o;
		}
		else
			return PyErr_Format(PyExc_Exception, "interface %s wasn't found", iid);
	}
	else
		return PyErr_Format(PyExc_Exception, "interface %s isn't available to python", iid);
}


local void destroy_interface(void *v)
{
	pyint_generic_interface *i = v;
	int r;

	Py_DECREF(i->funcs);
	i->funcs = NULL;

	i->head.refcount = 0;
	mm->UnregInterface(i, i->arena);

	if ((r = mm->UnregInterface(i->real_interface, i->arena)))
	{
		lm->Log(L_WARN, "<pymod> there are %d remaining references"
				" to an interface implemented in python!", r);
	}
	else
	{
		afree(i->head.iid);
		afree(i);
	}
}

local PyObject *mthd_reg_interface(PyObject *self, PyObject *args)
{
	char *iid;
	PyObject *funcs;
	Arena *arena = ALLARENAS;
	pyint_generic_interface *newint;
	void *realint;

	if (!PyArg_ParseTuple(args, "sO|O&", &iid, &funcs, cvt_p2c_arena, &arena))
		return NULL;

	if (!PyTuple_Check(funcs))
	{
		PyErr_SetString(PyExc_TypeError, "funcs isn't a tuple");
		return NULL;
	}

	realint = HashGetOne(pyint_impl_ints, iid);
	if (!realint)
	{
		PyErr_SetString(PyExc_TypeError, "you can't implement that interface in Python");
		return NULL;
	}

	Py_INCREF(funcs);

	newint = amalloc(sizeof(*newint));
	newint->head.magic = MODMAN_MAGIC;
	newint->head.iid = astrdup(iid);
	newint->head.name = "__unused__";
	newint->head.reserved1 = 0;
	newint->head.refcount = 0;
	newint->py_int_magic = PY_INT_MAGIC;
	newint->real_interface = realint;
	newint->arena = arena;
	newint->funcs = funcs;

	/* this is the "real" one that other modules will make calls on */
	mm->RegInterface(realint, arena);
	/* this is the dummy one registered with an iid that's only known
	 * internally to pymod. */
	mm->RegInterface(newint, arena);

	return PyCObject_FromVoidPtr(newint, destroy_interface);
}


/* commands */

local HashTable *pycmd_cmds;

local void init_py_commands(void)
{
	pycmd_cmds = HashAlloc();
}

local void deinit_py_commands(void)
{
	HashFree(pycmd_cmds);
}

local void pycmd_command(const char *tc, const char *params, Player *p, const Target *t)
{
	PyObject *args;
	LinkedList cmds = LL_INITIALIZER;
	Link *l;

	args = Py_BuildValue("ssO&O&",
			tc,
			params,
			cvt_c2p_player,
			p,
			cvt_c2p_target,
			t);
	if (args)
	{
		HashGetAppend(pycmd_cmds, tc, &cmds);
		for (l = LLGetHead(&cmds); l; l = l->next)
			PyObject_Call(l->data, args, NULL);
		Py_DECREF(args);
	}
}

struct pycmd_ticket
{
	PyObject *func, *htobj;
	Arena *arena;
	char cmd[1];
};

local void remove_command(void *v)
{
	struct pycmd_ticket *t = v;

	cmd->RemoveCommand(t->cmd, pycmd_command, t->arena);
	HashRemove(pycmd_cmds, t->cmd, t->func);
	Py_DECREF(t->func);
	Py_XDECREF(t->htobj);
	afree(t);
}

local PyObject *mthd_add_command(PyObject *self, PyObject *args)
{
	char *cmdname, *helptext = NULL;
	PyObject *func, *helptextobj;
	Arena *arena = ALLARENAS;
	struct pycmd_ticket *t;

	if (!PyArg_ParseTuple(args, "sO|O&", &cmdname, &func, cvt_p2c_arena, &arena))
		return NULL;

	if (!PyCallable_Check(func))
	{
		PyErr_SetString(PyExc_TypeError, "func isn't callable");
		return NULL;
	}

	Py_INCREF(func);

	t = amalloc(sizeof(*t) + strlen(cmdname));
	t->func = func;
	t->arena = arena;
	strcpy(t->cmd, cmdname);

	helptextobj = PyObject_GetAttrString(func, "__doc__");
	if (helptextobj && helptextobj != Py_None && PyString_Check(helptextobj))
	{
		t->htobj = helptextobj;
		helptext = PyString_AsString(helptextobj);
	}
	else
		Py_XDECREF(helptextobj);

	cmd->AddCommand(cmdname, pycmd_command, arena, helptext);
	HashAdd(pycmd_cmds, cmdname, func);

	return PyCObject_FromVoidPtr(t, remove_command);
}


/* timers */

local int pytmr_timer(void *v)
{
	PyObject *func = v, *ret;
	int goagain;

	ret = PyObject_CallFunction(func, NULL);
	if (!ret)
	{
		log_py_exception(L_ERROR, "error in python timer function");
		return FALSE;
	}

	goagain = ret == Py_None || PyObject_IsTrue(ret);
	Py_DECREF(ret);

	return goagain;
}

local void clear_timer(void *v)
{
	PyObject *func = v;
	mainloop->ClearTimer(pytmr_timer, func);
	Py_DECREF(func);
}

local PyObject *mthd_set_timer(PyObject *self, PyObject *args)
{
	PyObject *func;
	int initial, interval = -1;

	if (!PyArg_ParseTuple(args, "Oi|i", &func, &initial, &interval))
		return NULL;

	if (interval == -1)
		interval = initial;

	if (!PyCallable_Check(func))
	{
		PyErr_SetString(PyExc_TypeError, "func isn't callable");
		return NULL;
	}

	Py_INCREF(func);

	mainloop->SetTimer(pytmr_timer, initial, interval, func, func);

	return PyCObject_FromVoidPtr(func, clear_timer);
}


/* player persistent data */

struct pypersist_ppd
{
	PlayerPersistentData ppd;
	PyObject *get, *set, *clear;
};


local int get_player_data(Player *p, void *data, int len, void *v)
{
	struct pypersist_ppd *pyppd = v;
	PyObject *val, *pkl;
	const void *pkldata;
	int pkllen;

	val = PyEval_CallFunction(pyppd->get, "(O&)", cvt_c2p_player, p);
	if (!val)
	{
		log_py_exception(L_ERROR, "error in persistent data getter");
		return 0;
	}

	pkl = PyObject_CallMethod(cPickle, "dumps", "(Oi)", val, 1);
	Py_DECREF(val);
	if (!pkl)
	{
		log_py_exception(L_ERROR, "error pickling persistent data");
		return 0;
	}

	if (PyObject_AsReadBuffer(pkl, &pkldata, &pkllen))
	{
		Py_DECREF(pkl);
		lm->Log(L_ERROR, "<pymod> pickle result isn't buffer");
		return 0;
	}

	if (pkllen > len)
	{
		Py_DECREF(pkl);
		lm->Log(L_WARN, "<pymod> persistent data getter returned more "
				"than %d bytes of data (%d allowed)",
				pkllen, len);
		return 0;
	}

	memcpy(data, pkldata, pkllen);

	Py_DECREF(pkl);

	return pkllen;
}

local void set_player_data(Player *p, void *data, int len, void *v)
{
	struct pypersist_ppd *pyppd = v;
	PyObject *buf, *val, *ret;

	buf = PyString_FromStringAndSize(data, len);
	val = PyObject_CallMethod(cPickle, "loads", "(O)", buf);
	Py_XDECREF(buf);
	if (!val)
	{
		log_py_exception(L_ERROR, "can't unpickle persistent data");
		return;
	}

	ret = PyEval_CallFunction(pyppd->set, "(O&O)", cvt_c2p_player, p, val);
	Py_XDECREF(ret);
	if (!ret)
		log_py_exception(L_ERROR, "error in persistent data setter");
}

local void clear_player_data(Player *p, void *v)
{
	struct pypersist_ppd *pyppd = v;
	PyObject *ret;

	ret = PyEval_CallFunction(pyppd->clear, "(O&)", cvt_c2p_player, p);
	Py_XDECREF(ret);
	if (!ret)
		log_py_exception(L_ERROR, "error in persistent data clearer");
}


local void unreg_ppd(void *v)
{
	struct pypersist_ppd *pyppd = v;
	persist->UnregPlayerPD(&pyppd->ppd);
	Py_XDECREF(pyppd->get);
	Py_XDECREF(pyppd->set);
	Py_XDECREF(pyppd->clear);
	afree(pyppd);
}

local PyObject * mthd_reg_ppd(PyObject *self, PyObject *args)
{
	struct pypersist_ppd *pyppd;
	PyObject *get, *set, *clear;
	int key, interval, scope;

	if (!persist)
	{
		PyErr_SetString(PyExc_Exception, "the persist module isn't loaded");
		return NULL;
	}

	if (!cPickle)
	{
		PyErr_SetString(PyExc_Exception, "cPickle isn't loaded");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "iiiOOO", &key, &interval, &scope,
				&get, &set, &clear))
		return NULL;

	if (!PyCallable_Check(get) || !PyCallable_Check(set) || !PyCallable_Check(clear))
	{
		PyErr_SetString(PyExc_TypeError, "one of the functions isn't callable");
		return NULL;
	}

	if (scope != PERSIST_ALLARENAS && scope != PERSIST_GLOBAL)
	{
		PyErr_SetString(PyExc_ValueError, "scope must be PERSIST_ALLARENAS or PERSIST_GLOBAL");
		return NULL;
	}

	if (interval < 0 || interval > MAX_INTERVAL)
	{
		PyErr_SetString(PyExc_ValueError, "interval out of range");
		return NULL;
	}

	Py_INCREF(get);
	Py_INCREF(set);
	Py_INCREF(clear);

	pyppd = amalloc(sizeof(*pyppd));
	pyppd->ppd.key = key;
	pyppd->ppd.interval = interval;
	pyppd->ppd.scope = scope;
	pyppd->ppd.GetData = get_player_data;
	pyppd->ppd.SetData = set_player_data;
	pyppd->ppd.ClearData = clear_player_data;
	pyppd->ppd.clos = pyppd;
	pyppd->get = get;
	pyppd->set = set;
	pyppd->clear = clear;

	persist->RegPlayerPD(&pyppd->ppd);

	return PyCObject_FromVoidPtr(pyppd, unreg_ppd);
}


/* arena persistent data */

struct pypersist_apd
{
	ArenaPersistentData apd;
	PyObject *get, *set, *clear;
};


local int get_arena_data(Arena *a, void *data, int len, void *v)
{
	struct pypersist_apd *pyapd = v;
	PyObject *val, *pkl;
	const void *pkldata;
	int pkllen;

	val = PyEval_CallFunction(pyapd->get, "(O&)", cvt_c2p_arena, a);
	if (!val)
	{
		log_py_exception(L_ERROR, "error in persistent data getter");
		return 0;
	}

	pkl = PyObject_CallMethod(cPickle, "dumps", "(Oi)", val, 1);
	Py_DECREF(val);
	if (!pkl)
	{
		log_py_exception(L_ERROR, "error pickling persistent data");
		return 0;
	}

	if (PyObject_AsReadBuffer(pkl, &pkldata, &pkllen))
	{
		Py_DECREF(pkl);
		lm->Log(L_ERROR, "<pymod> pickle result isn't buffer");
		return 0;
	}

	if (pkllen > len)
	{
		Py_DECREF(pkl);
		lm->Log(L_WARN, "<pymod> persistent data getter returned more "
				"than %d bytes of data (%d allowed)",
				pkllen, len);
		return 0;
	}

	memcpy(data, pkldata, pkllen);

	Py_DECREF(pkl);

	return pkllen;
}

local void set_arena_data(Arena *a, void *data, int len, void *v)
{
	struct pypersist_apd *pyapd = v;
	PyObject *buf, *val, *ret;

	buf = PyString_FromStringAndSize(data, len);
	val = PyObject_CallMethod(cPickle, "loads", "(O)", buf);
	Py_XDECREF(buf);
	if (!val)
	{
		log_py_exception(L_ERROR, "can't unpickle persistent data");
		return;
	}

	ret = PyEval_CallFunction(pyapd->set, "(O&O)", cvt_c2p_arena, a, val);
	Py_XDECREF(ret);
	if (!ret)
		log_py_exception(L_ERROR, "error in persistent data setter");
}

local void clear_arena_data(Arena *a, void *v)
{
	struct pypersist_apd *pyapd = v;
	PyObject *ret;

	ret = PyEval_CallFunction(pyapd->clear, "(O&)", cvt_c2p_arena, a);
	Py_XDECREF(ret);
	if (!ret)
		log_py_exception(L_ERROR, "error in persistent data clearer");
}


local void unreg_apd(void *v)
{
	struct pypersist_apd *pyapd = v;
	persist->UnregArenaPD(&pyapd->apd);
	Py_XDECREF(pyapd->get);
	Py_XDECREF(pyapd->set);
	Py_XDECREF(pyapd->clear);
	afree(pyapd);
}

local PyObject * mthd_reg_apd(PyObject *self, PyObject *args)
{
	struct pypersist_apd *pyapd;
	PyObject *get, *set, *clear;
	int key, interval, scope;

	if (!persist)
	{
		PyErr_SetString(PyExc_Exception, "the persist module isn't loaded");
		return NULL;
	}

	if (!cPickle)
	{
		PyErr_SetString(PyExc_Exception, "cPickle isn't loaded");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "iiiOOO", &key, &interval, &scope,
				&get, &set, &clear))
		return NULL;

	if (!PyCallable_Check(get) || !PyCallable_Check(set) || !PyCallable_Check(clear))
	{
		PyErr_SetString(PyExc_TypeError, "one of the functions isn't callable");
		return NULL;
	}

	if (scope != PERSIST_ALLARENAS && scope != PERSIST_GLOBAL)
	{
		PyErr_SetString(PyExc_ValueError, "scope must be PERSIST_ALLARENAS or PERSIST_GLOBAL");
		return NULL;
	}

	if (interval < 0 || interval > MAX_INTERVAL)
	{
		PyErr_SetString(PyExc_ValueError, "interval out of range");
		return NULL;
	}

	Py_INCREF(get);
	Py_INCREF(set);
	Py_INCREF(clear);

	pyapd = amalloc(sizeof(*pyapd));
	pyapd->apd.key = key;
	pyapd->apd.interval = interval;
	pyapd->apd.scope = scope;
	pyapd->apd.GetData = get_arena_data;
	pyapd->apd.SetData = set_arena_data;
	pyapd->apd.ClearData = clear_arena_data;
	pyapd->apd.clos = pyapd;
	pyapd->get = get;
	pyapd->set = set;
	pyapd->clear = clear;

	persist->RegArenaPD(&pyapd->apd);

	return PyCObject_FromVoidPtr(pyapd, unreg_apd);
}


local PyMethodDef asss_module_methods[] =
{
	{"reg_callback", mthd_reg_callback, METH_VARARGS,
		"registers a callback"},
	{"call_callback", mthd_call_callback, METH_VARARGS,
		"calls some callback functions"},
	{"get_interface", mthd_get_interface, METH_VARARGS,
		"gets an interface object"},
	{"reg_interface", mthd_reg_interface, METH_VARARGS,
		"registers an interface implemented in python"},
	{"add_command", mthd_add_command, METH_VARARGS,
		"registers a command implemented in python. the docstring of the "
		"command function will be used for command helptext."},
	{"reg_player_persistent", mthd_reg_ppd, METH_VARARGS,
		"registers a per-player persistent data handler"},
	{"reg_arena_persistent", mthd_reg_apd, METH_VARARGS,
		"registers a per-arena persistent data handler"},
	{"set_timer", mthd_set_timer, METH_VARARGS,
		"registers a timer"},
	{NULL}
};

local void init_asss_module(void)
{
	PyObject *m;

	if (PyType_Ready(&PlayerType) < 0)
		return;
	if (PyType_Ready(&ArenaType) < 0)
		return;

	m = Py_InitModule3("asss", asss_module_methods, "the asss interface module");
	if (m == NULL)
		return;

	/* handle constants */
#define STRING(x) PyModule_AddStringConstant(m, #x, x);
#define PYCALLBACK(x) PyModule_AddStringConstant(m, #x, PYCBPREFIX x);
#define PYINTERFACE(x) PyModule_AddStringConstant(m, #x, PYINTPREFIX x);
#define INT(x) PyModule_AddIntConstant(m, #x, x);
#define ONE(x) PyModule_AddIntConstant(m, #x, 1);
#include "py_constants.inc"
#undef STRING
#undef CALLBACK
#undef INTERFACE
#undef INT
#undef ONE
}



/* stuff relating to loading and unloading */

local int load_py_module(mod_args_t *args, const char *line)
{
	PyObject *mod;
	lm->Log(L_INFO, "<pymod> loading python module '%s'", line);
	mod = PyImport_ImportModule((char*)line);
	if (mod)
	{
		args->privdata = mod;
		astrncpy(args->name, line, sizeof(args->name));
		args->info = NULL;
		mods_loaded++;
		return MM_OK;
	}
	else
	{
		lm->Log(L_ERROR, "<pymod> error loading python module '%s'", line);
		log_py_exception(L_ERROR, NULL);
		return MM_FAIL;
	}
}


local int unload_py_module(mod_args_t *args)
{
	PyObject *mdict = PyImport_GetModuleDict();
	PyObject *mod = args->privdata;
	char *mname = PyModule_GetName(mod);

	if (mod->ob_refcnt != 2)
	{
		lm->Log(L_WARN, "<pymod> there are %d remaining references to module %s",
				mod->ob_refcnt, mname);
		return MM_FAIL;
	}

	/* the modules dictionary has a reference */
	PyDict_DelItemString(mdict, mname);
	/* and then get rid of our reference too */
	Py_DECREF(mod);

	mods_loaded--;

	return MM_OK;
}


local int pyloader(int action, mod_args_t *args, const char *line, Arena *arena)
{
	switch (action)
	{
		case MM_LOAD:
			return load_py_module(args, line);

		case MM_UNLOAD:
			return unload_py_module(args);

		case MM_ATTACH:
		case MM_DETACH:
		case MM_POSTLOAD:
		case MM_PREUNLOAD:
			/* FIXME: implement these */
			return MM_FAIL;

		default:
			return MM_FAIL;
	}
}


EXPORT int MM_pymod(int action, Imodman *mm_, Arena *arena)
{
#ifdef CHECK_SIGS
	struct sigaction sa_before;
#endif
	if (action == MM_LOAD)
	{
		/* grab some modules */
		mm = mm_;
		pd = mm->GetInterface(I_PLAYERDATA, ALLARENAS);
		aman = mm->GetInterface(I_ARENAMAN, ALLARENAS);
		lm = mm->GetInterface(I_LOGMAN, ALLARENAS);
		cfg = mm->GetInterface(I_CONFIG, ALLARENAS);
		cmd = mm->GetInterface(I_CMDMAN, ALLARENAS);
		persist = mm->GetInterface(I_PERSIST, ALLARENAS);
		mainloop = mm->GetInterface(I_MAINLOOP, ALLARENAS);
		if (!pd || !aman || !lm || !cfg || !cmd || !mainloop)
			return MM_FAIL;
		pdkey = pd->AllocatePlayerData(sizeof(pdata));
		adkey = aman->AllocateArenaData(sizeof(adata));
		if (pdkey < 0 || adkey < 0)
			return MM_FAIL;

#ifdef CHECK_SIGS
		sigaction(SIGINT, NULL, &sa_before);
#endif
		/* start up python */
		Py_Initialize();
#ifdef CHECK_SIGS
		/* if python changed this, reset it */
		sigaction(SIGINT, &sa_before, NULL);
#endif
		/* set up our search path */
		PyRun_SimpleString(
				"import sys, os\n"
				"sys.path[:0] = ["
				"  os.getcwd() + '/python',"
				"  os.getcwd() + '/bin']\n");
		/* add our module */
		init_asss_module();
		init_py_callbacks();
		init_py_interfaces();
		init_py_commands();
		/* extra init stuff */
		{
			PyObject *sys = PyImport_ImportModule("sys");
			sysdict = PyModule_GetDict(sys);
			Py_INCREF(sysdict);
			Py_DECREF(sys);
			if (persist)
			{
				cPickle = PyImport_ImportModule("cPickle");
				if (!cPickle)
					log_py_exception(L_ERROR, "can't import cPickle");
			}
		}

		mm->RegCallback(CB_NEWPLAYER, py_newplayer, ALLARENAS);
		mm->RegCallback(CB_ARENAACTION, py_aaction, ALLARENAS);

		mm->RegModuleLoader("py", pyloader);

		mods_loaded = 0;

		return MM_OK;
	}
	else if (action == MM_UNLOAD)
	{
		/* check that there are no more python modules loaded */
		if (mods_loaded)
			return MM_FAIL;

		mm->UnregModuleLoader("py", pyloader);

		mm->UnregCallback(CB_NEWPLAYER, py_newplayer, ALLARENAS);
		mm->UnregCallback(CB_ARENAACTION, py_aaction, ALLARENAS);

		/* close down python */
		Py_XDECREF(sysdict);
		Py_XDECREF(cPickle);
		mainloop->ClearTimer(pytmr_timer, NULL);
		deinit_py_commands();
		deinit_py_interfaces();
		deinit_py_callbacks();
		Py_Finalize();

		/* release our modules */
		pd->FreePlayerData(pdkey);
		aman->FreeArenaData(adkey);
		mm->ReleaseInterface(pd);
		mm->ReleaseInterface(aman);
		mm->ReleaseInterface(lm);
		mm->ReleaseInterface(cfg);
		mm->ReleaseInterface(cmd);
		mm->ReleaseInterface(persist);
		mm->ReleaseInterface(mainloop);

		return MM_OK;
	}
	return MM_FAIL;
}

