zf_log
========

### Tiny logging library for C (and C++)

This is just a thin wrapper around sprintf() function. It provides less than 20%
of functionality found in more sophisticated libraries, but covers more than 80%
of common use cases. It could be particularly useful in mobile/embedded
applications, early stages of development or when prototyping. Focus is made on
simplicity, ease of use and performance (to be more precise - low overhead).

Features:

* Debug logging is reduced to no-op in release builds
* Arguments are not evaluated when the message is not logged
* No "unused" warning for variables used in log statements only
* Log a memory region as HEX and ASCII
* Optional built-in support for Android log and Apple system log (iOS, OS X)
* Custom output functions

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

Why zf?
--------

It stands for Ze Foundation. "Ze" is like "The" but with french or german accent.
Mostly because zf_anything looks better than tf_anything.
