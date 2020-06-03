#include <Python.h>

#include <stdbool.h>

#include "dict-common.h" // my

PyDoc_STRVAR(chainmap__doc__,
"A (hopefully) faster collections.ChainMap implementation.");

typedef struct {
    PyObject_HEAD
    PyListObject *maps;
} PyChainMap;

static Py_ssize_t chainmap_length(PyChainMap *self) {
    // TODO
    return 0;
}

// rewrite of dict_subscript() that handles the list of dicts.
static PyObject *chainmap_subscript(PyChainMap *self, PyObject *key) {
    Py_hash_t hash;
    if (!PyUnicode_CheckExact(key) ||
        (hash = ((PyASCIIObject *) key)->hash) == -1) {
        hash = PyObject_Hash(key);
        if (hash == -1) {
            return NULL;
        }
    }

    Py_ssize_t ix;
    PyObject *value;
    for (Py_ssize_t i = 0; i < PyList_GET_SIZE(self->maps); i++) {
        PyObject *o = PyList_GET_ITEM(self->maps, i);
        // TODO handle other types of maps
        if (!PyDict_CheckExact(o)) {
            PyErr_Format(PyExc_TypeError, "'%.200s' object is not subscriptable", Py_TYPE(o)->tp_name);
            return NULL;
        }

        PyDictObject *mp = (PyDictObject*)o;

        ix = mp->ma_keys->dk_lookup(mp, key, hash, &value);
        if (ix == DKIX_ERROR) {
            return NULL;
        }
        if (ix == DKIX_EMPTY || value == NULL) {
            continue;
        }

        Py_INCREF(value);
        return value;
    }

    _PyErr_SetKeyError(key);
    return NULL;
}

static int chainmap_ass_sub(PyChainMap *self, PyObject *v, PyObject *w) {
    assert(PyList_GET_SIZE(self->maps) > 0);
    return PyDict_Type.tp_as_mapping->mp_ass_subscript(PyList_GET_ITEM(self->maps, 0), v, w);
}

static void chainmap_dealloc(PyChainMap *self) {
    PyObject_GC_UnTrack(self);
    Py_DECREF(self->maps);
    PyObject_GC_Del((PyObject*)self);
}

static PyMappingMethods chainmap_as_mapping = {
    (lenfunc)chainmap_length,
    (binaryfunc)chainmap_subscript,
    (objobjargproc)chainmap_ass_sub,
};

static PyMethodDef chainmap_methods[] = {
    {NULL, NULL},
};

static PyTypeObject ChainMap_type;

static int chainmap_init(PyChainMap *self, PyObject *args, PyObject *kwds) {
    PyObject *dict = NULL;
    assert(Py_IS_TYPE(self, &ChainMap_type));

    if (!_PyArg_NoKeywords("ChainMap", kwds)) {
        goto out;
    }

    self->maps = (PyListObject*)PyType_GenericAlloc(&PyList_Type, 0);
    if (0 > PyList_Type.tp_init((PyObject*)self->maps, args, kwds)) {
        goto out_maps;
    }

    if (PyList_GET_SIZE(self->maps) == 0) {
        PyObject *dict = PyDict_New();
        if (!dict) {
            goto out_maps;
        }

        if (0 > PyList_Append((PyObject*)self->maps, dict)) {
            goto out_dict;
        }

        Py_DECREF(dict);
    }

    return 0;

out_dict:
    Py_DECREF(dict);
out_maps:
    Py_DECREF(self->maps);
out:
    return -1;
}

static PyObject *chainmap_maps_get(PyChainMap *self, void *Py_UNUSED(ignored))
{
    Py_INCREF(self->maps);
    return (PyObject*)self->maps;
}

static int chainmap_maps_set(PyChainMap *self, PyObject *o, void *Py_UNUSED(ignored))
{
    if (o) {
        if (!PyList_CheckExact(o)) {
            PyErr_SetString(PyExc_TypeError, "maps must be a list");
            return -1;
        }

        Py_INCREF(o);
        Py_SETREF(self->maps, (PyListObject*)o);
        return 0;
    } else {
        PyErr_SetString(PyExc_TypeError, "maps may not be deleted");
        return -1;
    }
}

static PyGetSetDef chainmap_getset[] = {
    { "maps", (getter)chainmap_maps_get, (setter)chainmap_maps_set, "maps list", NULL },
    { 0 },
};

static int chainamp_traverse(PyChainMap *self, visitproc visit, void *arg)
{
    Py_VISIT(self->maps);
    return 0;
}

static PyTypeObject ChainMap_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "chainmap.ChainMap",
    sizeof(PyChainMap),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)chainmap_dealloc,
    .tp_as_mapping = &chainmap_as_mapping,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse = (traverseproc)chainamp_traverse,
    .tp_methods = chainmap_methods,
    .tp_getset = chainmap_getset,
    .tp_init = (initproc)chainmap_init,
    .tp_new = PyType_GenericNew,
};

static int chainmap_exec(PyObject* m) {
    if (PyType_Ready(&ChainMap_type) < 0) {
        return -1;
    }

    Py_INCREF(&ChainMap_type);
    if (PyModule_AddObject(m, "ChainMap", (PyObject*)&ChainMap_type) < 0) {
        return -1;
    }

    return 0;
}

static struct PyModuleDef_Slot chainmap_slots[] = {
    {Py_mod_exec, chainmap_exec},
    {0, NULL},
};

static struct PyModuleDef chainmapmodule = {
    PyModuleDef_HEAD_INIT,
    "chainmap",
    chainmap__doc__,
    0,
    NULL,
    chainmap_slots,
    NULL,
    NULL,
    NULL,
};

PyMODINIT_FUNC PyInit_chainmap(void) {
    return PyModuleDef_Init(&chainmapmodule);
}
