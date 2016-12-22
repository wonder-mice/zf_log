[![Build Status](https://travis-ci.org/wonder-mice/zf_log.svg?branch=master)](https://travis-ci.org/wonder-mice/zf_log)
[![Build Status](https://ci.appveyor.com/api/projects/status/u9rmuaw147q578w0/branch/master?svg=true)](https://ci.appveyor.com/project/wonder-mice/zf-log/branch/master)
[![Gitter Chat](https://badges.gitter.im/wonder-mice/zf_log.svg)](https://gitter.im/wonder-mice/zf_log?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

zf_log
========

### Core logging library for C, Objective-C and C++

Following the [Unix way](https://en.wikipedia.org/wiki/Unix_philosophy), this
library provides the logging core which can be used directly or extended. In
essence, it's a thin wrapper around snprintf() function. By implementing less
than 20% of functionality found in more sophisticated and feature reach
libraries, it covers more than 80% of common use cases. Found to be
particularly useful in cross-platform applications and on mobile/embedded
platforms. Focus is made on simplicity, ease of use and performance (to be
more precise - low overhead).

Features:

* Debug logging is reduced to no-op in release builds:

  ```c
  /* no runtime overhead whatsoever if verbose log is disabled */
  ZF_LOGV("entering foobar(), args: %i, %s", arg0, arg1);
  ```

* No "unused" warning for variables used in log statements only:

  ```c
  /* no warning about err being unused even if verbose log is disabled */
  int err = close(fd);
  ZF_LOGV("close status %i", err);
  ```

* Arguments are not evaluated when the message is not logged:

  ```c
  /* to_utf8() will not be called if debug log is turned off or disabled */
  ZF_LOGD("Login: %s", to_utf8(loginUtf16));
  ```

* Log a memory region as HEX and ASCII:

  ```c
  /* will print HEX and ASCII view of received network packet */
  ZF_LOGD_MEM(pkg_ptr, pkg_sz, "Received network packet (%u bytes):", pkg_sz);
  ```

* Compiler warnings when format string and arguments don't match:

  ```c
  /* warning: format specifies type 'char *' but the argument has type 'int' */
  ZF_LOGI("This is int %s", 42);
  ```

* Conditional logging of sensitive information (also known as Personally
  Identifiable Information or PII):

  ```c
  /* will be logged only when logging of sensitive information is enabled */
  ZF_LOG_SECRET(ZF_LOGI("Credit card number: %s", credit_card));
  ```

* Custom output functions
* Custom log message formats
* Compile time configuration of logging level
* Run time configuration of logging level
* Optional built-in support for Android log and Apple system log (iOS, OS X)
* Reasonably cross-platform (OS X, iOS, Linux, Android, other Unix flavors,
  POSIX platforms and Windows)
* No external dependencies
* Compact call site (smaller executables)
* Thread safe
* Library size is under 10Kb (when compiled for x86_64)
* Can be used privatly in libraries

Examples
--------

Library provides a set of `ZF_LOGX` macros where `X` is an abbreviated log
level (e.g. `I` for `INFO`). This code will log an `INFO` message:

```c
ZF_LOGI("Number of arguments: %i", argc);
```

And will produce the following log line if `NDEBUG` is defined (aka release
build):

```
+- month           +- process id
|  +- day          |      +- thread id      +- message
|  |               |      |                 |
04-29 22:43:20.244 40059  1299 I hello.MAIN Number of arguments: 1
      |                        | |     |
      +- time                  | |     +- tag (optional)
                               | +- tag prefix (optional)
                               +- level
```

And if `NDEBUG` is NOT defined (aka debug build):

```
04-29 22:43:20.244 40059  1299 I hello.MAIN main@hello.c:9 Number of arguments: 1
                                            |    |       |
                                            |    |       +- line number
                                            |    +- source file name
                                            +- function name
```

It's also possible to dump a memory region. For example:

```c
const char data[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
ZF_LOGI_MEM(data, sizeof(data), "Lorem ipsum at %p", data);
```

Will produce the following output:

```
05-06 00:54:33.825 35864  1299 I hello.MAIN Lorem ipsum at 0x10fbc0f20:
05-06 00:54:33.825 35864  1299 I hello.MAIN 4c6f72656d20697073756d20646f6c6f  Lorem ipsum dolo
05-06 00:54:33.825 35864  1299 I hello.MAIN 722073697420616d65742c20636f6e73  r sit amet, cons
05-06 00:54:33.825 35864  1299 I hello.MAIN 65637465747572206164697069736369  ectetur adipisci
05-06 00:54:33.825 35864  1299 I hello.MAIN 6e6720656c69742e00                ng elit.?
```

More examples available in [examples](examples) folder. For more details see
comments in [zf_log/zf_log.h](zf_log/zf_log.h) file.

Usage
--------

### Embedding

The simplest way of using this library is to embed its sources into existing
project. For that, copy the following files to your source tree:

* [zf_log.h](zf_log/zf_log.h)
* [zf_log.c](zf_log/zf_log.c)

See comments in those files for configuration macros. One particularly useful
option when embedding into a library project is `ZF_LOG_LIBRARY_PREFIX`. It
could be used to decorate zf_log exported symbols to avoid linker conflicts
(when that library project is used in other project that is also uses zf_log).

### Embedding with CMake

Another options is avaibale for projects that are using CMake. Copy
[zf_log](zf_log) folder to you source tree and add it with `add_subdirectory()`
call in one of your CMakeLists.txt files. Also see
[zf_log/CMakeLists.txt](zf_log/CMakeLists.txt) for available `ZF_LOG_`
configuration options. For example:

```
set(ZF_LOG_ANDROID_LOG ON)
add_subdirectory(zf_log)
```

This will add `zf_log` library target. For each target that uses zf_log in
corresponding CMakeLists.txt file add:

```cmake
target_link_libraries(my_target zf_log)
```

### Installation

Another option is to build and install the library:

```bash
git clone https://github.com/wonder-mice/zf_queue.git zf_queue.git
mkdir zf_queue.build && cd zf_queue.build
cmake ../zf_queue.git -DCMAKE_INSTALL_PREFIX=/usr/local
make
sudo make install
```

This will also install
`${CMAKE_INSTALL_PREFIX}/lib/cmake/zf_log/zf_log.cmake`
and
`${CMAKE_INSTALL_PREFIX}/lib/cmake/zf_log/zf_log-config.cmake`.
The first one is for direct `include` from CMakeLists.txt files.
The second can be located by CMake with:

```cmake
find_package(zf_log)
```

Both will add `zf_log` imported library target.
For each target that uses zf_log in corresponding CMakeLists.txt file add:

```cmake
target_link_libraries(my_target zf_log)
```

To build as a shared library set CMake variable `BUILD_SHARED_LIBS`:

```bash
cmake ../zf_queue.git -DBUILD_SHARED_LIBS:BOOL=ON
```

Performance
--------

Log statements that are below *current log level* (compile time check) have
**no overhead** - they are compiled out and their log arguments will **not** be
evaluated. Consider:

```c
#include <signal.h>
#include <unistd.h>
#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.h>

int main(int argc, char *argv[])
{
	ZF_LOGV("Argument of this VERBOSE message will not be evaluated: %i",
			kill(getpid(), SIGKILL));
	ZF_LOGI("So you will see that INFO message");
	return 0;
}
```

Log statements that are below *output log level* (run time check)
have a **small overhead** of compare operation and conditional jump. Arguments
will **not** be evaluated and no function call will be performed. Consider:

```c
#include <stdlib.h>
#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.h>

int main(int argc, char *argv[])
{
	zf_log_set_output_level(ZF_LOG_WARN);
	int count = 0;
	for (int i = 2; 0 < i--;)
	{
		ZF_LOGI("Argument of this INFO message will be evaluated only once: %i",
				++count);
		zf_log_set_output_level(ZF_LOG_INFO);
	}
	if (1 != count)
	{
		abort();
	}
	ZF_LOGI("And you will see that INFO message");
	return 0;
}
```

Log statements that are on or above current log level and output log level will
go into log output (and arguments will be evaluated). In that case it's hard to
talk about performance because string formatting routines will be called and IO
will be performed.

To conclude, it is OK to have log statements for debug and development purposes,
even in performance critical parts. But make sure to set correct current log
level (to compile them out) or output log level (to suppress them) in release
builds.

That said, in some rare cases it could be useful to provide a custom output
function that will use memory buffer for the log output.

Output
--------

By default log messages are written to the `stderr`, but it is also possible to
set custom output function. Library has an optional built-in support for the
following output facilities (see [zf_log/zf_log.c] for details):

* Android Log (via android/log.h)
* Apple System Log (iOS, OS X via asl.h)

See [examples/custom_output.c] for an example of custom output function.

[zf_log/zf_log.c]: zf_log/zf_log.c
[examples/custom_output.c]: examples/custom_output.c

Comparison
--------

This table is work in progress. See [tests/perf](tests/perf) folder for how
this table was generated (fully automated).

|                                        |  Easylogging++  |     g3log     |     glog    |     spdlog    |      zf_log     |
| -------------------------------------- | ---------------:| -------------:| -----------:| -------------:| ---------------:|
|  Call site size: string                |          304 B  |        360 B  |      160 B  |        352 B  |         **48 B**|
|  Call site size: 3 integers            |          856 B  |        384 B  |      320 B  |        312 B  |         **72 B**|
|  Executable size: 1 module             |      208.84 KB  |    183.73 KB  |  137.37 KB  |    133.69 KB  |     **18.33 KB**|
|  Module compile time                   |      3.182 sec  |    0.511 sec  |  0.374 sec  |    2.163 sec  |    **0.024 sec**|
|  Executable link time                  |      0.025 sec  |    0.022 sec  |  0.026 sec  |  **0.020 sec**|    **0.017 sec**|
|  Speed: 1 thread, string               |        378,722  |    1,257,168  |    385,098  |    1,909,253  |    **5,296,201**|
|  Speed: 1 thread, 3 integers           |        311,690  |    1,023,108  |    321,507  |    1,470,271  |    **3,173,097**|
|  Speed: 1 thread, string, off          |      4,199,431  |   55,297,247  |    411,960  |   78,675,820  |  **482,713,041**|
|  Speed: 1 thread, slow function, off   |            743  |   54,877,221  |        732  |          750  |  **423,658,146**|
|  Speed: 4 threads, string              |        132,836  |    2,353,169  |    196,950  |      196,930  |    **9,690,216**|
|  Speed: 4 threads, 3 integers          |        116,696  |    1,896,178  |    235,141  |      172,880  |    **5,545,075**|
|  Speed: 4 threads, string, off         |        244,448  |  106,600,866  |    186,233  |  164,582,184  |**1,229,398,467**|
|  Speed: 4 threads, slow function, off  |            733  |  111,685,033  |      2,887  |        2,976  |**1,106,166,551**|
|  Speed: 8 threads, string              |        131,691  |    2,447,058  |    172,824  |      183,277  |    **9,731,434**|
|  Speed: 8 threads, 3 integers          |        115,991  |    1,936,270  |    176,594  |      177,950  |    **5,596,045**|
|  Speed: 8 threads, string, off         |        240,538  |  104,424,749  |    182,465  |  164,632,729  |**1,228,058,986**|
|  Speed: 8 threads, slow function, off  |            774  |  116,354,224  |      5,815  |        5,976  |**1,127,173,552**|

Details:

* Call site size - amount of code generated for each LOG() statement when
  logging a string, format string with 4 integers or format string with 4
  integers where the function is used to get an integer.
* Compile time - time to compile a source file that includes public API
  header from the logging library.
* Speed - log lines per second. Thread(s) call LOG() N times in a
  tight loop for T seconds. N/T is log lines per second.
* When it says "off", it means that log level was turned off. Expecting
  very low overhead in this mode.

Why zf?
--------

It stands for Ze Foundation. "Ze" is like "The" but with french or german accent.
Mostly because zf_anything looks better than tf_anything.
