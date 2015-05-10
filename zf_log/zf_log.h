#pragma once

#ifndef _ZF_LOG_H_
#define _ZF_LOG_H_

/* To detect incompatible changes you can define ZF_LOG_VERSION_REQUIRED to
 * the current value of ZF_LOG_VERSION before including this file (or via
 * compiler command line):
 *
 *   #define ZF_LOG_VERSION_REQUIRED 1
 *   #include <zf_log.h>
 *
 * In that case compilation will fail when included file has incompatible
 * version.
 */
#define ZF_LOG_VERSION 1
#if defined(ZF_LOG_VERSION_REQUIRED)
	#if ZF_LOG_VERSION_REQUIRED != ZF_LOG_VERSION
		#error different zf_log version required
	#endif
#endif

/* Log level guideline:
 * - ZF_LOG_FATAL - happened something impossible and absolutely unexpected.
 *   Process can't continue and must be terminated. In other words, semantic is
 *   close to assert(). zf_log will call abort() after printing the log message.
 *   Example: division by zero, unexpected modifications from other thread.
 * - ZF_LOG_ERROR - happened something impossible and absolutely unexpected, but
 *   process is able to recover and continue execution.
 *   Example: out of memory (could also be FATAL if not handled properly).
 * - ZF_LOG_WARN - happened something that *usually* should not happen and
 *   significantly changes application behavior for some period of time.
 *   Example: configuration file not found, auth error.
 * - ZF_LOG_INFO - happened significant life cycle event or major state
 *   transition.
 *   Example: app started, user logged in.
 * - ZF_LOG_DEBUG - minimal set of events that could help to reconstruct the
 *   execution path.
 * - ZF_LOG_VERBOSE - all other events.
 *
 * Ideally, log file of debugged, well tested, production ready application
 * should be empty (no messages with level ZF_LOG_INFO or higher) or very small.
 */
#define ZF_LOG_VERBOSE 1
#define ZF_LOG_DEBUG   2
#define ZF_LOG_INFO    3
#define ZF_LOG_WARN    4
#define ZF_LOG_ERROR   5
#define ZF_LOG_FATAL   6
#define ZF_LOG_NONE    0xFFFF

/* Log level configuration:
 * - ZF_LOG_DEF_LEVEL - defines default log level. Only messages with that level
 *   and higher will be logged (if ZF_LOG_LEVEL is undefined).
 * - ZF_LOG_LEVEL - overrides default log level. Only messages with that level
 *   and higher will be logged.
 *
 * Common pattern is to define ZF_LOG_DEF_LEVEL in the build script (e.g.
 * Makefile, CMakeLists.txt) for the entire project/target:
 *
 *   CC_ARGS := -DZF_LOG_DEF_LEVEL=ZF_LOG_WARN
 *
 * And when necessary override it with ZF_LOG_LEVEL in .c/.cpp files (before
 * including zf_log.h):
 *
 *   #define ZF_LOG_LEVEL ZF_LOG_VERBOSE
 *   #include <zf_log.h>
 *
 * Defining either ZF_LOG_DEF_LEVEL or ZF_LOG_LEVEL in header file is usually
 * undesired and produces weird results.
 *
 * If both ZF_LOG_DEF_LEVEL and ZF_LOG_LEVEL are undefined, then ZF_LOG_INFO
 * will be used for release builds (NDEBUG is defined) and ZF_LOG_DEBUG
 * otherwise.
 *
 * When log message has level bellow current log level it will be compiled out
 * and its arguments will NOT be evaluated (no "unused variable" warning will be
 * generated for variables that are only used in compiled out log messages).
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

/* Log tag configuration:
 * - ZF_LOG_DEF_TAG - defines default log tag.
 * - ZF_LOG_TAG - overrides default log tag.
 *
 * When defined, value must be a string constant (in double quotes):
 *
 *   #define ZF_LOG_TAG "MAIN"
 *   #include <zf_log.h>
 *
 * Defining either ZF_LOG_DEF_TAG or ZF_LOG_TAG in header files usually
 * undesired and produces weird results.
 *
 * If both ZF_LOG_DEF_TAG and ZF_LOG_TAG are undefined no tag will be added to
 * the log message.
 */
#if defined(ZF_LOG_TAG)
	#define _ZF_LOG_TAG ZF_LOG_TAG
#elif defined(ZF_LOG_DEF_TAG)
	#define _ZF_LOG_TAG ZF_LOG_DEF_TAG
#else
	#define _ZF_LOG_TAG 0
#endif

/* Runtime configuration */
#ifdef __cplusplus
extern "C" {
#endif

/* Set tag prefix. Prefix will be separated from the tag with dot ('.').
 * Use 0 or empty string to disable (default). Common use is to set it to
 * the process (or target) name.
 */
void zf_log_set_tag_prefix(const char *const prefix);

/* Set number of bytes per log line in memory output.
 */
void zf_log_set_mem_width(const unsigned w);

/* Set output log level. When output log level is higher than the current log
 * level it overrides it. Otherwise output log level is ignored. This is because
 * messages below current log level are compiled out.
 */
void zf_log_set_output_level(const int lvl);

typedef struct zf_log_output_ctx
{
	int lvl;
	const char *tag;
	char *buf; /* Buffer start */
	char *e; /* Buffer end (last position where EOL with 0 could be written) */
	char *p; /* Buffer content end (append position) */
	char *tag_b; /* Prefixed tag start */
	char *tag_e; /* Prefixed tag end (if != tag_b, points to msg separator) */
	char *msg_b; /* Message start (expanded format string) */
}
zf_log_output_ctx;

typedef void (*zf_log_output_cb)(zf_log_output_ctx *ctx);

/* Set output callback function. It will be called for each log message allowed
 * by both current log level and output log level. Callback function is allowed
 * to modify contents of the buffer pointed by the ctx.
 */
void zf_log_set_output_callback(const zf_log_output_cb cb);

#ifdef __cplusplus
}
#endif

/* Checking current log level at compile time (ignoring output log level).
 * For example:
 *
 *   #if ZF_LOG_ALLOW_DEBUG
 *       const char *const g_enum_strings[] = {
 *           "enum_value_0", "enum_value_1", "enum_value_2"
 *       };
 *   #endif
 *   // ...
 *   #if ZF_LOG_ALLOW_DEBUG
 *       ZF_LOGD("enum value: %s", g_enum_strings[v]);
 *   #endif
 */
#define ZF_LOG_ALLOW(lvl) ((lvl) >= _ZF_LOG_LEVEL)
#define ZF_LOG_ALLOW_VERBOSE ZF_LOG_ALLOW(ZF_LOG_VERBOSE)
#define ZF_LOG_ALLOW_DEBUG ZF_LOG_ALLOW(ZF_LOG_DEBUG)
#define ZF_LOG_ALLOW_INFO ZF_LOG_ALLOW(ZF_LOG_INFO)
#define ZF_LOG_ALLOW_WARN ZF_LOG_ALLOW(ZF_LOG_WARN)
#define ZF_LOG_ALLOW_ERROR ZF_LOG_ALLOW(ZF_LOG_ERROR)
#define ZF_LOG_ALLOW_FATAL ZF_LOG_ALLOW(ZF_LOG_FATAL)

/* Checking output log level at run time (taking into account current log
 * level). For example:
 *
 *   if (ZF_LOG_OUTPUT_DEBUG)
 *   {
 *       char hash[65];
 *       sha256(data_ptr, data_sz, hash);
 *       ZF_LOGD("data: len=%u, sha256=%s", data_sz, hash);
 *   }
 */
#define ZF_LOG_OUTPUT(lvl) \
		(ZF_LOG_ALLOW((lvl)) && (lvl) >= _zf_log_output_lvl)
#define ZF_LOG_OUTPUT_VERBOSE ZF_LOG_OUTPUT(ZF_LOG_VERBOSE)
#define ZF_LOG_OUTPUT_DEBUG ZF_LOG_OUTPUT(ZF_LOG_DEBUG)
#define ZF_LOG_OUTPUT_INFO ZF_LOG_OUTPUT(ZF_LOG_INFO)
#define ZF_LOG_OUTPUT_WARN ZF_LOG_OUTPUT(ZF_LOG_WARN)
#define ZF_LOG_OUTPUT_ERROR ZF_LOG_OUTPUT(ZF_LOG_ERROR)
#define ZF_LOG_OUTPUT_FATAL ZF_LOG_OUTPUT(ZF_LOG_FATAL)

#ifdef __printflike
	#define _ZF_LOG_PRINTFLIKE(a, b) __printflike(a, b)
#else
	#define _ZF_LOG_PRINTFLIKE(a, b)
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int _zf_log_output_lvl;

void _zf_log_write_d(const char *const func,
					 const char *const file, const unsigned line,
					 const int lvl, const char *const tag,
					 const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(6, 7);
void _zf_log_write(const int lvl, const char *const tag,
				   const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(3, 4);

void _zf_log_write_mem_d(const char *const func,
						 const char *const file, const unsigned line,
						 const int lvl, const char *const tag,
						 const void *const d, const unsigned d_sz,
						 const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(8, 9);
void _zf_log_write_mem(const int lvl, const char *const tag,
					   const void *const d, const unsigned d_sz,
					   const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(5, 6);
#ifdef __cplusplus
}
#endif

/* Message logging macros:
 * - ZF_LOGV("format string", args, ...)
 * - ZF_LOGD("format string", args, ...)
 * - ZF_LOGI("format string", args, ...)
 * - ZF_LOGW("format string", args, ...)
 * - ZF_LOGF("format string", args, ...)
 * Memory logging macros:
 * - ZF_LOGV_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGD_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGI_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGW_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGF_MEM(data_ptr, data_sz, "format string", args, ...)
 */
#ifdef NDEBUG
	#define _ZF_LOG_IMP(lvl, tag, ...) \
			do { \
				if (ZF_LOG_OUTPUT(lvl)) \
					_zf_log_write(lvl, tag, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_MEM_IMP(lvl, tag, d, d_sz, ...) \
			do { \
				if (ZF_LOG_OUTPUT(lvl)) \
					_zf_log_write_mem(lvl, tag, d, d_sz, __VA_ARGS__); \
			} while (0)
#else
	#define _ZF_LOG_IMP(lvl, tag, ...) \
			do { \
				if (ZF_LOG_OUTPUT(lvl)) \
					_zf_log_write_d(__FUNCTION__, __FILE__, __LINE__, \
							lvl, tag, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_MEM_IMP(lvl, tag, d, d_sz, ...) \
			do { \
				if (ZF_LOG_OUTPUT(lvl)) \
					_zf_log_write_mem_d(__FUNCTION__, __FILE__, __LINE__, \
							lvl, tag, d, d_sz, __VA_ARGS__); \
			} while (0)
#endif

static inline void _zf_log_unused(const int dummy, ...) {(void)dummy;}

#define _ZF_LOG_UNUSED(...) \
		do { if (0) _zf_log_unused(0, __VA_ARGS__); } while (0)

#if ZF_LOG_ALLOW_VERBOSE
	#define ZF_LOGV(...) \
			_ZF_LOG_IMP(ZF_LOG_VERBOSE, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGV_MEM(...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_VERBOSE, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGV(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGV_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_DEBUG
	#define ZF_LOGD(...) \
			_ZF_LOG_IMP(ZF_LOG_DEBUG, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGD_MEM(...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_DEBUG, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGD(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGD_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_INFO
	#define ZF_LOGI(...) \
			_ZF_LOG_IMP(ZF_LOG_INFO, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGI_MEM(...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_INFO, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGI(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGI_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_WARN
	#define ZF_LOGW(...) \
			_ZF_LOG_IMP(ZF_LOG_WARN, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGW_MEM(...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_WARN, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGW(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGW_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_ERROR
	#define ZF_LOGE(...) \
			_ZF_LOG_IMP(ZF_LOG_ERROR, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGE_MEM(...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_ERROR, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGE(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGE_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ALLOW_FATAL
	#define ZF_LOGF(...) \
			_ZF_LOG_IMP(ZF_LOG_FATAL, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGF_MEM(...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_FATAL, _ZF_LOG_TAG, __VA_ARGS__)
#else
	#define ZF_LOGF(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGF_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#endif
