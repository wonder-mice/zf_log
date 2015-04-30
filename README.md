zf_log
========

### Tiny logging library for C (and C++)

This is just a thin wrapper around sprintf() function. It provides less than 20%
of functionality found in more sophisticated libraries, but covers more than 80%
of use cases in mobile/embedded applications, early stages of development or
when prototyping. Focus is made on simplicity, ease of use, performance and
small library size.


Usage
--------

### Example

This code:

```c
ZF_LOGD("Number of arguments: %i", argc);
```

Will produce following log line if `NDEBUG` is defined (release build):

```
+- month           +- process id
|  +- day          |      +- thread id      +- message
|  |               |      |                 |
04-29 22:43:20.244 40059  1299 D hello.MAIN Number of arguments: 1
      |  |  |  |               | |     |
      |  |  |  +- millisecond  | |     +- tag (optional)
      |  |  +- second          | +- tag prefix (optional)
      |  +- minute             +- level
      +- hour
```

And if `NDEBUG` is NOT defined (debug build):

```
04-29 22:43:20.244 40059  1299 D hello.MAIN main@hello.c:9 Number of arguments: 1
                                            |    |       |
                                            |    |       +- line number
                                            |    +- source file name
                                            +- function name
```

### Building

To build shared library:
```bash
cmake -DBUILD_SHARED_LIBS:BOOL=ON
```

Why zf?
--------

It stands for Ze Foundation. "Ze" is like "The" but with french or german accent.
Mostly because zf_anything looks better than tf_anything.
