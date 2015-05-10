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
* Output some memory even when buffer is too small
* Add library usage to REAMDE (embeding, cmake, ...)
* Update README that log messages are compiled out and args
  marked as unused (so no warning).
