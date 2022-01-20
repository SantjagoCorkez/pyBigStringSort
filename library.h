/*
 * Author: Andrey Semenov, 2022
 */
#ifndef PYBIGSTRINGSORT_LIBRARY_H
#define PYBIGSTRINGSORT_LIBRARY_H

#include <Python.h>

PyObject *BigStringSort_call(PyObject *self, PyObject *arg);

#define BigStringSort_call__doc \
        "BigStringSort.sorted(path: PathLike) -> bytes\n" \
        "Reads the file pointed to by the `path` parameter and returns it sorted byte-wise.\n" \
        "Possible exceptions being raised:\n" \
        " - OSError - a file-related syscall operation failed (fopen()/fclose()); errno is then provided within the exception raised\n" \
        " - OverflowError - the file pointed to is larger that the largest memory region the runtime could handle\n" \
        " - MemoryError - the runtime was unable to allocate enough memory to store the result\n" \
        "\n" \
        ":param path: path to a file; is used in system `fopen()` as is.\n" \
        " If it's not an absolute path it's caller's responsibility to ensure the file is available to the runtime by its relative path\n" \
        ":return: the bytes sequence already sorted (according to characters' integer values)\n"

static PyMethodDef BigStringSortMethodDef[] = {
    {
        .ml_name = "sorted",
        .ml_flags = METH_STATIC | METH_O,
        .ml_doc = BigStringSort_call__doc,
        .ml_meth = &BigStringSort_call
    },
    {0}
};

PyTypeObject BigStringSortType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "BigStringSort",
    .tp_basicsize = 0,
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "C implementation of a large string sorter for Python runtime",
    .tp_methods = BigStringSortMethodDef,
    .tp_new = NULL,
    .tp_dealloc = NULL,
};

static PyModuleDef BigStringSort_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "big_string_sort",
    .m_doc = "C implementation for sorting large strings in python",
    .m_size = -1,
};

#endif //PYBIGSTRINGSORT_LIBRARY_H
