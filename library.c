/*
 * Author: Andrey Semenov, 2022
 */
#include <stddef.h>
#include "library.h"

#define PyBytesObject_SIZE (offsetof(PyBytesObject, ob_sval) + 1)
#define BIG_STRING_SORT_BUFSIZE 1 << 20 // 1 MiB

static size_t counters[256] = {0};

PyObject *BigStringSort_call(__attribute__((unused)) PyObject *self, PyObject *arg) {
    char *path = PyUnicode_AsUTF8(arg);  // uses Py_LIMITED_API function, though still works on CPython3.9
    FILE *fd;
    if ( !(fd = fopen(path, "r")) ) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    struct stat path_stat;
    stat(path, &path_stat);
    if ( ( size_t ) path_stat.st_size > ( size_t ) PY_SSIZE_T_MAX - PyBytesObject_SIZE ) {
        PyErr_SetString(PyExc_OverflowError,
                        "byte string is too large");
        return NULL;
    }

    PyBytesObject *retval = ( PyBytesObject * ) PyObject_Malloc(PyBytesObject_SIZE + path_stat.st_size);
    if ( retval == NULL ) {
        return PyErr_NoMemory();
    }

    size_t to_read = path_stat.st_size;
    uint8_t *buf = malloc(BIG_STRING_SORT_BUFSIZE);
    memset(counters, 0, sizeof(counters));
    while ( to_read ) {
        size_t chunk_size = fread(buf, 1, BIG_STRING_SORT_BUFSIZE, fd);
        to_read -= chunk_size;
        for ( uint32_t i = 0; i < chunk_size; i++ ) {
            counters[buf[i]]++;
        }
    }
    free(buf);
    if ( fclose(fd) ) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    PyObject_INIT_VAR(retval, &PyBytes_Type, path_stat.st_size);
    retval->ob_shash = -1;
    retval->ob_sval[path_stat.st_size] = '\0';
    size_t position = 0;
    for ( uint16_t char_val = 0; char_val < 256; char_val++ ) {
        if ( !counters[char_val] )
            continue;
        memset(retval->ob_sval + position, char_val, counters[char_val]);
        position += counters[char_val];
    }

    return ( PyObject * ) retval;
}

__attribute__((unused)) PyMODINIT_FUNC PyInit_big_string_sort(void) {
    PyObject *m = NULL;
    if ( PyType_Ready(&BigStringSortType) < 0 ) {
        goto fail;
    }

    m = PyModule_Create(&BigStringSort_module);
    if ( m == NULL ) {
        goto fail;
    }

    if ( PyModule_AddObject(m, "BigStringSort", ( PyObject * ) &BigStringSortType) < 0 ) {
        goto fail;
    }
    return m;
fail:
    Py_XDECREF(m);
    return NULL;
}
