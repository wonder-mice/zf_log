Things to do
============

* Save one push by having different functions for each level
  Macro can dispatch based on the level argument and directly
  call the function for requested log level.
* Embeding into the library use case
  Allow to prefix all public symbols with custom string. That
  way zf_log can be embeded inside the library without linking
  problems when the library is used with other code that also
  uses zf_log.
* Add unit test for memory output (value + small buffer)
* Output some memory even when buffer is too small
* put_xxx() functions can change e, need to restore it
  There are some flaws in that strategy. If output_mem will
  do the same thing, then each next line will have smaller
  size. Need either restor is or do not do it in the first
  place.
