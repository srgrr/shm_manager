#include "shm_manager.h"

int shm_manager_init(shm_manager *self, PyObject *args, PyObject *kwds) {
  return 0;
}

void shm_manager_dealloc(shm_manager *self) {
  // maybe it is already marked for deletion, but since we cannot know
  // without explicit IPC queries, then re-set it
  shmctl(self -> id, IPC_RMID, NULL);
  shm_manager* _self = (shm_manager*) self;
  shmdt(_self -> base_address);
}

PyObject* shm_manager_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  srand(time(NULL) + getpid());
  shm_manager *self;
  self = (shm_manager*) type -> tp_alloc(type, 0);
  // these are the default arg values, note that the pipe specifier is set at
  // PyArg_ParseTuple (all values after pipe are optional but, if given,
  // format must be respected)
  self -> key = 1337;
  self -> byte_amount = 100;
  // IPC_CREAT | IPC_EXCL means "i want to CREATe an SHM and i want to ensure
  // that my key is EXCLusive"
  self -> flags = 0600 | IPC_CREAT | IPC_EXCL | SHM_NORESERVE | SHM_LOCKED;
  //| SHM_LOCKED | SHM_NORESERVE;
  self -> offset = 0;

  if(!PyArg_ParseTuple(
        //read |lli as optional args (but in this order): two longs and an int
  args, "|lli", &self -> key, &self -> byte_amount, &self -> flags)) {
    std::cout << "shm_manager.__init__: ERROR, wrong argspec!" << std::endl;
    return NULL;
  }

  // if we are creating a segment then the key does not really matter, so we
  // can try a few random keys before deciding that there is an error
  // if we are getting a segment then the key must be the one that is specified
  int attempt_count = self -> flags & (IPC_CREAT | IPC_EXCL) ?
                        0 : MAX_SHMGET_ATTEMPTS - 1;
  while(
  attempt_count++ < MAX_SHMGET_ATTEMPTS &&
  (self -> id = shmget(self -> key, self -> byte_amount, self -> flags)) == -1) {
    self -> key = (key_t) rand();
  }
  // we tried to create or get an shm with no success? throw an error
  if(self -> id == -1) {
    std::cout << "shm_manager.__init__: ERROR,";
    if(self -> flags & (IPC_CREAT | IPC_EXCL)) {
      std::cout << " could not create SHM" << std::endl;
    }
    else {
      std::cout << " could not get SHM with key " << self -> key << std::endl <<
      "are you sure that exists?" << std::endl;
    }
    return NULL;
  }
  // attach the shared segment to our process
  // int attach_flags = self->flags&(IPC_CREAT | IPC_EXCL) ? 0 : SHM_RDONLY |
  // SHM_RND;
  int attach_flags = 0;
  if( (self -> base_address = (char*)shmat(self -> id, NULL, attach_flags)) == (char*) -1) {
    std::cout << "shm_manager.__init__: ERROR, could not attach the SHM to the"
    << " process memory!" << std::endl;
    return NULL;
  }
  return (PyObject*) self;
}

void _touch_next_page(shm_manager *self) {
  unsigned long long page_size = 1ll << 23ll;
  unsigned long long next_page = (1ll + (self -> offset) / page_size) * page_size;
  unsigned long long dir = std::min(self -> byte_amount - 1ll, next_page + 1ll);
  self -> last_z = self -> base_address[dir];
}

PyObject *shm_manager_write_pyobject(PyObject *self, PyObject *args) {
  shm_manager *_self = (shm_manager*)self;
  PyObject *content_object;
  if(!PyArg_ParseTuple(args, "O", &content_object)) {
    std::cout << "shm_manager.write: ERROR, wrong argspec!" << std::endl;
    return NULL;
  }
  memcpy(_self -> base_address, content_object, _self -> byte_amount);
  Py_RETURN_NONE;
}

PyObject *shm_manager_write(PyObject *self, PyObject *args) {
  shm_manager *_self = (shm_manager*) self;
  const char *to_write;
  int byte_amount;
  if(!PyArg_ParseTuple(args, "s#", &to_write, &byte_amount)) {
    std::cout << "shm_manager.write: ERROR, wrong argspec!" << std::endl;
    return NULL;
  }
  // are we going to write in a legal location?
  if((size_t)byte_amount + _self -> offset > _self -> byte_amount) {
    std::cout << "shm_manager.write: ERROR, OOB write operation!" <<
    std::endl;
    return NULL;
  }
  // this is the actual write to the manager
  memcpy(_self -> base_address + _self -> offset, to_write, byte_amount);
  _self -> offset += byte_amount;
  Py_RETURN_NONE;
}

PyObject *shm_manager_read(PyObject *self, PyObject *args) {
  shm_manager *_self = (shm_manager*) self;
  unsigned long long bytes_to_read = _self -> byte_amount - _self -> offset;
  if(!PyArg_ParseTuple(args, "|l", &bytes_to_read)) {
    std::cout << "shm_manager.read: ERROR, wrong argspec!" << std::endl;
    return NULL;
  }
  _self -> offset += bytes_to_read;
  return PyString_FromStringAndSize(
    _self -> base_address + _self -> offset - bytes_to_read,
    bytes_to_read);
}


PyObject *shm_manager_read_pyobject(PyObject *self, PyObject *args) {
  shm_manager *_self = (shm_manager*) self;
  PyObject *ret = (PyObject*)_self->base_address;
  ret -> ob_type = &PyString_Type;
  Py_INCREF(ret);
  return (PyObject*) ret;
}

PyObject *shm_manager_readline(PyObject* self, PyObject* args) {
  shm_manager* _self = (shm_manager*) self;
  unsigned long long old_offset = _self -> offset;
  bool done = false;
  while(!done && _self->offset < _self -> byte_amount) {
      done |= _self->base_address[_self -> offset] == '\n';
      ++_self -> offset;
  }
  auto ret = PyString_FromStringAndSize(_self->base_address + old_offset,
  _self -> offset - old_offset);
  return ret;
}

PyObject *shm_manager_seek(PyObject *self, PyObject *args) {
  shm_manager *_self = (shm_manager*) self;
  unsigned long long new_offset = 0;
  if(!PyArg_ParseTuple(args, "|l", &new_offset)) {
    std::cout << "shm_manager.seek: ERROR, wrong argspec!" << std::endl;
  }
  _self -> offset = new_offset;
  Py_RETURN_NONE;
}

PyObject* shm_manager_tell(PyObject *self, PyObject *args) {
  shm_manager *_self = (shm_manager*) self;
  return Py_BuildValue("l", _self -> offset);
}

PyObject *shm_manager_delete(PyObject *self, PyObject *args) {
  Py_RETURN_NONE;
}

PyMODINIT_FUNC initshm_manager(void) {
    PyObject *m;
    if (PyType_Ready(&shm_managerType) < 0)
        return;
    m = Py_InitModule("shm_manager", module_methods);
    Py_INCREF(&shm_managerType);
    PyModule_AddObject(m, "shm_manager", (PyObject *)&shm_managerType);
}
