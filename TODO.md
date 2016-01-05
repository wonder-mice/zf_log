Things to do
------------

* Add callback arg to output callback
* Update README, it sucks now
* Print start address for each line of memory output?
* Introduce private format_callback which probably will replace put_msg
  This will provide more modular structure and will point out where to
  insert custom code when different format function must be used.
  As an exercise, add optional support for CFStringCreateWithFormat().
* Use Cmake GNUInstallDirs for install locations.

Things probably not to do
-------------------------

* Number of parameters in _zf_log_write_xxx() functions could be reduced
  by having a separate function for each log level. One way to implement
  that is to have a static array of logging functions. Log level will be
  an index in that array. This technique reduces the size of code
  generated per log statement (e.g. 5 bytes less on x64). But it also
  increases the size of the library itself, because all those new
  28 functions (7 levels x 2 mem/msg x 2 debug/ndebug) must be defined.
  And it also makes implementation more complex. Currently considered
  not worth the effort.

* Output some memory even when buffer is too small. Will significantly
  increase complexity of output_mem() function, while providing not
  much benefits. Memory output line is pretty much limited in length,
  so problem could be solved easily by choosing right ZF_LOG_BUF_SZ.

* Debug functions (_zf_log_write.*_d) could also receive the length of
  string passed in as a "file" parameter. That will allow to search for
  a slash from the end of the string. While it will be up to 30 times
  faster, overall performance gain will be too small to notice, because
  the biggest offenders right now are localtime() and file io. Also
  this will require to push additional argument which will increase the
  size of call site which could be very noticable - size of the binary
  will go up.
