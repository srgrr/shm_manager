#pragma once
#include <Python.h>
#include <structmember.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>
const int MAX_SHMGET_ATTEMPTS = 100;
// this definition is meant to 1) avoid compiler warnings
// 2) be conscious about the actual meaning of minus one in the size_t type
const size_t MINUS_ONE = (size_t) - 1;

/*
  This struct contains all the necessary information in order to work
  with a Shared Memory Segment
*/
typedef struct {
  PyObject_HEAD
  // it is preferrable to use unsigned instead of key_t for its
  // genericity
  unsigned key;
  int id;
  unsigned long long byte_amount;
  int flags;
  char *base_address;
  unsigned long long offset;
  int last_z;
} shm_manager;

static char* member_names[] = {
  (char*) "key",
  (char*) "id",
  (char*) "bytes",
  (char*) "flags",
  (char*) "base_address",
  (char*) "offset"
};

static char* member_descriptions[] = {
  (char*) "SHM key",
  (char*) "SHM identifier",
  (char*) "Byte length of the SHM",
  (char*) "Flag-set for this manager",
  (char*) "Base memory address of the mapped segment",
  (char*) "Current offset (aka pointer)"
};

PyMemberDef shm_manager_members[] = {
  {member_names[0], T_INT, offsetof(shm_manager, key), 0,
  member_descriptions[0]},
  {member_names[1], T_INT, offsetof(shm_manager, id), 0,
  member_descriptions[1]},
  {member_names[2], T_LONG, offsetof(shm_manager, byte_amount), 0,
  member_descriptions[2]},
  {member_names[3], T_INT, offsetof(shm_manager, flags), 0,
  member_descriptions[3]},
  {member_names[4], T_LONG, offsetof(shm_manager, base_address), 0,
  member_descriptions[4]},
  {member_names[5], T_LONG, offsetof(shm_manager, offset), 0,
  member_descriptions[5]},
  {NULL}  /* Sentinel */
};

/*
  Pre-constructor operations?
*/
int shm_manager_init(shm_manager *self, PyObject *args, PyObject *kwds);

/*
  Equivalent to the destructor method
*/
void shm_manager_dealloc(shm_manager* self);

/*
  Expected parameters (default value):
  - key (1337)
  - bytes (100)
  - flags(0600 | IPC_CREAT | IPC_EXCL)
  Gets (or creates) a shared memory segment with the specified key.
  If the specified key is not available, random keys will be attempted for
  a limited amount of times.
  It also attaches the SHM to the main process. Attaching means mapping the
  shm addresses to the process memory.

  If write_pyobject or read_pyobject are going to be used, it is MANDATORY
  to set permissions for any process using the SHM to, at least 0600

  Returns a shm_manager object if there was no problem.
  If some problem is found, a ShmException will be thrown.
*/
PyObject* shm_manager_new(PyTypeObject* type, PyObject* args, PyObject* kwds);

/*
  Memcpys the, literally, PyObject passed as argument to the SHM
*/
PyObject* shm_manager_write_pyobject(PyObject* self, PyObject* args);

/*
  Expected parameters (default value):
  - An shm object (implicit as self/this)
  - A bytearray
  Writes to the SHM the contents of the bytearray
*/
PyObject* shm_manager_write(PyObject* self, PyObject* args);

/*
  Returns a PyObject from the SHM.
  Since the process that creates the PyObject is not necessarily the
  same that reads this object, its ob_type must be changed in order to
  ensure that its type is correctly recovered. Otherwise, this object
  may be interpreted as None or, even worse, a segfault can occur.
*/
PyObject* shm_manager_read_pyobject(PyObject* self, PyObject* args);

/*
  Reads N bytes from the shm.
*/
PyObject* shm_manager_read(PyObject* self, PyObject* args);

/*
  Expected parameters (default value):
  - An shm object (implicit as self/this)
  Deletes the SHM (both as Linux SHM-Segment and python object)
*/
PyObject* shm_manager_delete(PyObject* self, PyObject* args);

/*
  Reads a line from the shm and returns it (\n char included)
*/
PyObject* shm_manager_readline(PyObject* self, PyObject* args);

/*
  Sets the pointer to the specified position
*/
PyObject* shm_manager_seek(PyObject* self, PyObject* args);

/*
  Returns the pointer current position
*/
PyObject* shm_manager_tell(PyObject* self, PyObject* args);

/*
    Module methods
*/
PyMethodDef module_methods[] = {
    {NULL, NULL, 0, NULL} /* Sentinel */
};

/*
    Class and instance methods
*/
PyMethodDef shm_manager_methods[] = {
    {"write_object", shm_manager_write_pyobject, METH_VARARGS,
    "Writes the, literally, content of a PyObject"},
    {"write", shm_manager_write, METH_VARARGS,
    "Writes N bytes to the shared memory segment"},
    {"read_object", shm_manager_read_pyobject, METH_VARARGS,
    "Reads the content of the SHM and \"creates\" a PObj with it"},
    {"read", shm_manager_read, METH_VARARGS,
    "Reads N bytes from the shared memory segment"},
    {"readline", shm_manager_readline, METH_VARARGS,
    "Reads a whole line from the shared memory segment"},
    {"seek", shm_manager_seek, METH_VARARGS,
    "Sets the pointer position"},
    {"tell", shm_manager_tell, METH_VARARGS,
    "Returns the current pointer poisition"},
    {"delete", shm_manager_delete, METH_VARARGS,
    "Deletes a Shared Memory Segment"},
    {NULL}  /* Sentinel */
};

char* shm_manager_doc = (char*)
"A System-V SHM interface for Python.\n\
Offers functionalities for shared memory segments\n\
creation and handling, file-like methods (i.e: read, readline, write)\n\
and the possibility to transfer PyStringObjects, saving time from\n\
memory copies";

/*
    Python type definition.
    Look at initshm_manager's implementation to see how
    this struct is used.
*/
PyTypeObject shm_managerType = {
    PyObject_HEAD_INIT(NULL)
    0,                                          /*ob_size*/
    "shm_manager.shm_manager",                  /*tp_name*/
    sizeof(shm_manager),                        /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)shm_manager_dealloc,            /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    0,                                          /*tp_compare*/
    0,                                          /*tp_repr*/
    0,                                          /*tp_as_number*/
    0,                                          /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash */
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    shm_manager_doc,                            /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    shm_manager_methods,                        /* tp_methods */
    shm_manager_members,                        /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    (initproc)shm_manager_init,                 /* tp_init */
    0,                                          /* tp_alloc */
    shm_manager_new,                            /* tp_new */
};


#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initshm_manager(void);
