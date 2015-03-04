#pragma once

#include <sys/cdefs.h>

#if !defined(_ZF_LOG_STRINGIFY) && !defined(_ZF_LOG__STRINGIFY)
#define _ZF_LOG__STRINGIFY(x) #x
#define _ZF_LOG_STRINGIFY(x) _ZF_LOG__STRINGIFY(x)
#endif

#define ZF_LOG_VERBOSE 1
#define ZF_LOG_DEBUG   2
#define ZF_LOG_INFO    3
#define ZF_LOG_WARN    4
#define ZF_LOG_ERROR   5
#define ZF_LOG_FATAL   6
#define ZF_LOG_NONE	   64

/* Compile time log level:
 * - ZF_LOG_DEF_LEVEL (optional)
 * - ZF_LOG_LEVEL (optional, overrides ZF_LOG_DEF_LEVEL)
 * Common use is to have ZF_LOG_DEF_LEVEL defined in the build script
 * (e.g. Makefile) for the entire target. Then ZF_LOG_LEVEL can be used in
 * in .c/.cpp files to override default value when necessary.
 * Defining either ZF_LOG_DEF_LEVEL or ZF_LOG_LEVEL in header file is usually
 * undesired and produces weird results.
 * If both ZF_LOG_DEF_LEVEL and ZF_LOG_LEVEL are undefined, then ZF_LOG_INFO
 * will be used if NDEBUG is defined, ZF_LOG_DEBUG otherwise.
 */
#if defined(ZF_LOG_LEVEL)
	#define _ZF_LOG_LEVEL ZF_LOG_LEVEL
#elif defined(ZF_LOG_DEF_LEVEL)
	#define _ZF_LOG_LEVEL ZF_LOG_DEF_LEVEL
#else
	#ifdef NDEBUG
		#define _ZF_LOG_LEVEL ZF_LOG_INFO
	#else
		#define _ZF_LOG_LEVEL ZF_LOG_DEBUG
	#endif
#endif

/* Compile time log tag:
 * - ZF_LOG_DEF_TAG (optional)
 * - ZF_LOG_TAG (optional, overrides ZF_LOG_DEF_TAG)
 * When defined, value must be a string constant (i.e. contain double quotes).
 * Common use is to have ZF_LOG_DEF_TAG defined in the build script
 * (e.g. Makefile) for the entire target. Then ZF_LOG_TAG can be used in
 * in .c/.cpp files to override default value when necessary.
 * Defining either ZF_LOG_DEF_TAG or ZF_LOG_TAG in header files usually
 * undesired and produces weird results.
 * If both ZF_LOG_DEF_TAG and ZF_LOG_TAG are undefined process name will be used
 * as a tag. To disable tag completly - define ZF_LOG_DEF_TAG or ZF_LOG_DEF_TAG
 * as 0 (zero, no quotes).
 */
#if defined(ZF_LOG_TAG)
	#define _ZF_LOG_TAG ZF_LOG_TAG
#elif defined(ZF_LOG_DEF_TAG)
	#define _ZF_LOG_TAG ZF_LOG_DEF_TAG
#else
	#define _ZF_LOG_TAG 0
#endif

#ifdef __printflike
	#define _ZF_LOG_PRINTFLIKE(a, b) __printflike(a, b)
#else
	#define _ZF_LOG_PRINTFLIKE(a, b)
#endif

#define ZF_LOG_PROC_NAME 0

#ifdef __cplusplus
extern "C" {
#endif
/* Runtime log prefix. If prefix is ZF_LOG_PROC_NAME, current process name
 * will be used. Empty by default ("").
 */
int zf_log_set_prefix(const char *const prefix);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
void _zf_log_write_d(const char *const func, const char *const loc,
					 const int lvl, const char *const tag,
					 const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(5, 6);
void _zf_log_write(const int lvl, const char *const tag,
				   const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(3, 4);
#ifdef __cplusplus
}
#endif

#ifdef NDEBUG
	#define _ZF_LOG_IMP(lvl, tag, ...) \
			_zf_log_write(lvl, tag, __VA_ARGS__)
#else
	#define _ZF_LOG_IMP(lvl, tag, ...) \
			_zf_log_write_d(__FUNCTION__, \
							__FILE__ ":" _ZF_LOG_STRINGIFY(__LINE__), \
							lvl, tag, __VA_ARGS__)
#endif

static inline void _zf_log_unused(const int dummy, ...) {(void)dummy;}

#define _ZF_LOG_UNUSED(...) \
		do { if (false) _zf_log_unused(0, __VA_ARGS__); } while (false)

#define ZF_LOG_ALLOW(lvl) ((lvl) >= _ZF_LOG_LEVEL)
#define ZF_LOG_ALLOW_VERBOSE ZF_LOG_ALLOW(ZF_LOG_VERBOSE)
#define ZF_LOG_ALLOW_DEBUG ZF_LOG_ALLOW(ZF_LOG_DEBUG)
#define ZF_LOG_ALLOW_INFO ZF_LOG_ALLOW(ZF_LOG_INFO)
#define ZF_LOG_ALLOW_WARN ZF_LOG_ALLOW(ZF_LOG_WARN)
#define ZF_LOG_ALLOW_ERROR ZF_LOG_ALLOW(ZF_LOG_ERROR)
#define ZF_LOG_ALLOW_FATAL ZF_LOG_ALLOW(ZF_LOG_FATAL)

#if ZF_LOG_ALLOW_VERBOSE
	#define ZF_LOGV(...) \
			_ZF_LOG_IMP(ZF_LOG_VERBOSE, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGV(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_DEBUG
	#define ZF_LOGD(...) \
			_ZF_LOG_IMP(ZF_LOG_DEBUG, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGD(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_INFO
	#define ZF_LOGI(...) \
			_ZF_LOG_IMP(ZF_LOG_INFO, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGI(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_WARN
	#define ZF_LOGW(...) \
			_ZF_LOG_IMP(ZF_LOG_WARN, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGW(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_ERROR
	#define ZF_LOGE(...) \
			_ZF_LOG_IMP(ZF_LOG_ERROR, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGE(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_FATAL
	#define ZF_LOGF(...) \
			_ZF_LOG_IMP(ZF_LOG_FATAL, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGF(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif
