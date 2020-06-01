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
        PyDictObject *mp = (PyDictObject*)PyList_GET_ITEM(self->maps, i);

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
    Py_DECREF(self->maps);
    PyObject_Del((PyObject*)self);
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
    assert(Py_IS_TYPE(self, &ChainMap_type));

    if (!_PyArg_NoKeywords("ChainMap", kwds)) {
        return -1;
    }

    self->maps = (PyListObject*)PyType_GenericAlloc(&PyList_Type, 0);
    if (0 > PyList_Type.tp_init((PyObject*)self->maps, args, kwds)) {
        Py_DECREF(self->maps);
        return -1;
    }

    // TODO make sure they are all dicts / mappings whatever
    // TODO insert empty dict if no maps were given

    return 0;
}

static PyTypeObject ChainMap_type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "chainmap.ChainMap",
    sizeof(PyChainMap),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)chainmap_dealloc,
    .tp_as_mapping = &chainmap_as_mapping,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = chainmap_methods,
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