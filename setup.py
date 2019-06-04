from distutils.core import setup, Extension

thread_affinity = \
Extension(
  "shm_manager",
  extra_compile_args=["-O3", "-Iext"],
  sources = ["shm_manager.cc"]
)

setup(
  name = "shm_manager",
  version = "1.0.0",
  description = "A shared memory segment manager for Py2 in linux",
  author = "The COMPSs Team",
  author_email = "support-compss@bsc.es",
  url = "https://github.com/bsc-wdc/thread_affinity",
  ext_modules = [thread_affinity]
)