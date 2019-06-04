from distutils.core import setup, Extension

shm_manager = \
Extension(
  "shm_manager",
  extra_compile_args=["-O3", "-Iext"],
  sources = ["shm_manager.cc"]
)

setup(
  name = "shm_manager",
  version = "1.0.0",
  description = "A shared memory segment manager for Py2 in linux",
  author = "Sergio Rodriguez Guasch",
  author_email = "sergio.r.guasch@gmail.com",
  url = "https://github.com/srgrr/shm_manager",
  ext_modules = [shm_manager]
)