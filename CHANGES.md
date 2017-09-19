# Changes

## aspcud 1.9.4
  * refactor aspcud application to c++ to ease maintanance
  * sanitize aspcud options
## aspcud 1.9.3
  * switch to MIT license
  * switch to boost program options
  * update cmake files to require version 3.1
  * lemon and re2c files can be generated now
    (this make both optional for builds of releases)
  * move to github
## aspcud 1.9.2
  * removed obsolete unclasp related code and information
  * better checking of optimization criteria
## aspcud 1.9.1
  * adjusted encodings to be compatible with gringo-4.5
    (maintains backwards compatibility with <gringo-4.5)
  * manpage update
  * fixed problem with boost-1.56
## aspcud 1.9.0
  * added support for three new selectors (request,installrequest,upgraderequest)
  * added support for old-style criteria
  * added an install target (supports PREFIX/DESTDIR)
  * added old trendy criterion
  * added Debian manpage
  * switched to clasp-3's uncore algorithm
  * replaced bash/python script with a small C program
  * should build on MacOS X without problems now
  * compiles with mingw
  * support reading from streams
## aspcud 1.8.0
  * first official release
