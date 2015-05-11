Things to do
============

* Save one push by having different functions for each level
  Macro can dispatch based on the level argument and directly
  call the function for requested log level.
  But that will require level argument to be a compile time constant,
  but right now there is now public way to specify level as argument
  anyway.
  Probably will need to have an array of functions and dispatch using
  level as array index.
* Embeding into the library use case
  Add this to README
* Add library usage to REAMDE (embeding, cmake, ...)
* Update README that log messages are compiled out and args
  marked as unused (so no warning).
* Output some memory even when buffer is too small
  Probably not important, but will increase complexity.
