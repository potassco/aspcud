1 Prerequisites
---------------

1.1 Build Dependencies
----------------------

- Boost (>= v1.43) (http://www.boost.org/)
- cmake (>= v2.6)  (http://www.cmake.org/)
- re2c  (>= v0.13) (http://www.re2c.org/)

And a C++ 14 conforming compiler like:
- gcc (>= 4.9) (http://gcc.gnu.org/)
- clang (http://clang.llvm.org/)

1.2 Runtime Dependencies
------------------------

- clasp  (>= v3.0.0) (https://github.com/potassco/clasp/)
- gringo (>= v4.2.1) (https://github.com/potassco/clingo/)

2 Compilation and Installation
------------------------------

To compile a release build, create a release folder and execute cmake
accordingly:

    mkdir -p build/release
    cd build/release
    cmake -DCMAKE_BUILD_TYPE=Release ../..
    make
    make install

For information on how to run aspcud, please refer to the
[README](README.md) file.

2.1 Configuring Paths to Binaries and Encodings
-----------------------------------------------

By default aspcud uses some hard-coded paths for binaries/encodings.  These can
be overwritten using the following cmake variables (this are *not* environment
variables and thus have to be set using cmakes `-D` option):

- `ASPCUD_CUDF2LP_PATH`: The path to the cudf2lp binary.
  (Default: `CMAKE_INSTALL_PREFIX/bin/cudf2lp`)
- `ASPCUD_GRINGO_PATH`: The path to the gringo binary.
  (Default: `CMAKE_INSTALL_PREFIX/bin/gringo`)
- `ASPCUD_CLASP_PATH`: The path to the clasp binary.
  (Default: `CMAKE_INSTALL_PREFIX/bin/clasp`)
- `ASPCUD_ENCODING_PATH`: The path to the default encoding.
  (Default: `CMAKE_INSTALL_PREFIX/share/aspcud/misc2012.lp`)

Furthermore, you can use the string "`<module_path>`" as prefix of any of the
above paths.  This prefix is then replaced by the path the aspcud executable is
in. This is meant for distributing relocatable binaries of aspcud.

