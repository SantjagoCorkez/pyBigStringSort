/*
 * Author: Andrey Semenov, 2022
 */
#include <stddef.h>
#include "library.h"

#define PyBytesObject_SIZE (offsetof(PyBytesObject, ob_sval) + 1)
#define BIG_STRING_SORT_BUFSIZE 1 << 20 // 1 MiB

// 0.1 of a second
static const struct timespec err_recovery_time = {
    .tv_sec = 0,
    .tv_nsec = 100000000
};

static void BigStringSort__count_chars(size_t counters[],
                                       const size_t chunk_size,
                                       size_t *to_read,
                                       const uint8_t *const buf) {
    *to_read -= chunk_size;
    for ( uint32_t i = 0; i < chunk_size; i++ ) {
        counters[buf[i]]++;
    }
}

PyObject *BigStringSort_call(__attribute__((unused)) PyObject *self, PyObject *arg) {
    PyThreadState *_save = PyEval_SaveThread();
    size_t counters[256] = {0};

    char *path;
    // uses Py_LIMITED_API function, though still works on CPython3.9
    if ( (path = PyUnicode_AsUTF8(arg)) == NULL ) {
        PyEval_RestoreThread(_save);
        return NULL;
    }
    FILE *fd;
    if ( !(fd = fopen(path, "r")) ) {
        PyEval_RestoreThread(_save);
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }

    struct stat path_stat;
    if ( stat(path, &path_stat) ) {
        fclose(fd);
        PyEval_RestoreThread(_save);
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    if ( ( size_t ) path_stat.st_size > ( size_t ) PY_SSIZE_T_MAX - PyBytesObject_SIZE ) {
        fclose(fd);
        PyEval_RestoreThread(_save);
        PyErr_SetString(PyExc_OverflowError,
                        "byte string is too large");
        return NULL;
    }

    PyBytesObject *retval;
    if ( (retval = ( PyBytesObject * ) PyObject_Malloc(PyBytesObject_SIZE + path_stat.st_size)) == NULL ) {
        fclose(fd);
        PyEval_RestoreThread(_save);
        return PyErr_NoMemory();
    }

    size_t to_read = path_stat.st_size;
    uint8_t *buf = NULL;
    if ( (buf = malloc(BIG_STRING_SORT_BUFSIZE)) == NULL ) {
        PyEval_RestoreThread(_save);
        PyErr_SetFromErrno(PyExc_OSError);
        goto err_obj_cleanup;
    }
    uint8_t err_count = 0;
    while ( to_read ) {
        size_t chunk_size = fread(buf, 1, BIG_STRING_SORT_BUFSIZE, fd);
        if ( chunk_size != BIG_STRING_SORT_BUFSIZE && chunk_size != to_read ) {
            BigStringSort__count_chars(counters, chunk_size, &to_read, buf);
            if ( feof(fd) ) {
                PyEval_RestoreThread(_save);
                PyErr_SetString(PyExc_BrokenPipeError, "File was shrunk while being read");
                goto err_buf_cleanup;
            }
            if ( ferror(fd) ) {
                if ( err_count >= 10 ) {
                    char err_text[255] = {0};
                    sprintf(err_text,
                            "Could not read the contents of the file (10 failed attempts at region %lu)",
                            path_stat.st_size - to_read);
                    PyEval_RestoreThread(_save);
                    PyErr_SetString(PyExc_IOError, err_text);
                    goto err_buf_cleanup;
                }
                err_count++;
                struct timespec rem = {0};
                if ( clock_gettime(CLOCK_MONOTONIC, &rem) ) {
                    PyEval_RestoreThread(_save);
                    PyErr_SetFromErrno(PyExc_OSError);
                    goto err_buf_cleanup;
                }
                rem.tv_sec += 1 * ( int ) ((rem.tv_nsec + err_recovery_time.tv_nsec) >= 1000000000);
                rem.tv_nsec += err_recovery_time.tv_nsec;
                rem.tv_nsec -= (1000000000 * ( int ) (rem.tv_nsec >= 1000000000));
                if ( clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &rem, NULL) ) {
                    PyEval_RestoreThread(_save);
                    PyErr_SetFromErrno(PyExc_OSError);
                    goto err_buf_cleanup;
                }
                clearerr(fd);
                continue;
            }
        }
        err_count = 0;
        BigStringSort__count_chars(counters, chunk_size, &to_read, buf);
    }
    free(buf);

    // throws a ResourceWarning so a user could catch it and decide if there's a need to recover and how
    // no hard recovery is needed since the main function is done and results are only needed to be properly returned
    if ( fclose(fd) ) {
        PyEval_RestoreThread(_save);
        PyErr_WarnEx(PyExc_ResourceWarning, strerror(errno), 1);
        _save = PyEval_SaveThread();
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
    PyEval_RestoreThread(_save);

    return ( PyObject * ) retval;
err_buf_cleanup:
    free(buf);
err_obj_cleanup:
    fclose(fd);
    PyObject_Free(retval);
    return NULL;
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
