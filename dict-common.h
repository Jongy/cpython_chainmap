// not exported (?) so meanwhile I took relevant parts manually :p
// it's nicer to call dk_lookup directly to skip the creation of KeyErrors
// etc during lookups.

#ifndef Py_DICT_COMMON_H
#define Py_DICT_COMMON_H

typedef Py_ssize_t (*dict_lookup_func)
    (PyDictObject *mp, PyObject *key, Py_hash_t hash, PyObject **value_addr);

#define DKIX_EMPTY (-1)
#define DKIX_ERROR (-3)

struct _dictkeysobject {
    Py_ssize_t dk_refcnt;
    Py_ssize_t dk_size;
    dict_lookup_func dk_lookup;
    // ...
};

#endif
