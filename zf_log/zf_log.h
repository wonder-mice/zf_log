#pragma once

#ifndef _ZF_LOG_H_
#define _ZF_LOG_H_

/* To detect incompatible changes you can define ZF_LOG_VERSION_REQUIRED to
 * the current value of ZF_LOG_VERSION before including this file (or via
 * compiler command line):
 *
 *   #define ZF_LOG_VERSION_REQUIRED 2
 *   #include <zf_log.h>
 *
 * Compilation will fail when included file has different version. Provided for
 * complitness and not intended for common use.
 */
#define ZF_LOG_VERSION 2
#if defined(ZF_LOG_VERSION_REQUIRED)
	#if ZF_LOG_VERSION_REQUIRED != ZF_LOG_VERSION
		#error different zf_log version required
	#endif
#endif

/* Log level guideline:
 * - ZF_LOG_FATAL - happened something impossible and absolutely unexpected.
 *   Process can't continue and must be terminated.
 *   Example: division by zero, unexpected modifications from other thread.
 * - ZF_LOG_ERROR - happened something possible, but highly unexpected. The
 *   process is able to recover and continue execution.
 *   Example: out of memory (could also be FATAL if not handled properly).
 * - ZF_LOG_WARN - happened something that *usually* should not happen and
 *   significantly changes application behavior for some period of time.
 *   Example: configuration file not found, auth error.
 * - ZF_LOG_INFO - happened significant life cycle event or major state
 *   transition.
 *   Example: app started, user logged in.
 * - ZF_LOG_DEBUG - minimal set of events that could help to reconstruct the
 *   execution path. Usually disabled in release builds.
 * - ZF_LOG_VERBOSE - all other events. Usually disabled in release builds.
 *
 * *Ideally*, log file of debugged, well tested, production ready application
 * should be empty or very small. Choosing a right log level is as important as
 * providing short and self descriptive log message.
 */
#define ZF_LOG_VERBOSE 1
#define ZF_LOG_DEBUG   2
#define ZF_LOG_INFO    3
#define ZF_LOG_WARN    4
#define ZF_LOG_ERROR   5
#define ZF_LOG_FATAL   6
#define ZF_LOG_NONE    0xFF

/* Current log level is a compile time check and has no runtime overhead. When
 * log level is below current log level it said to be "disabled". Otherwise,
 * it's "enabled". Log messages that are disabled has no runtime overhead - they
 * are converted to no-op by preprocessor and then eliminated by compiler.
 *
 * Log level configuration:
 * - ZF_LOG_DEF_LEVEL - defines current log level. Only messages with that level
 *   and higher will be logged (if ZF_LOG_LEVEL is undefined).
 * - ZF_LOG_LEVEL - overrides current log level. Only messages with that level
 *   and higher will be logged.
 *
 * Common practice is to define ZF_LOG_DEF_LEVEL in the build script (e.g.
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

/* Output log level override. For more details about output log level, see
 * zf_log_set_output_level() function. When defined, must evaluate to integral
 * value that corresponds to desired output log level. That doesn't have to be a
 * constant value. On the contrary, the intended use case is to allow different
 * output log levels between modules. Use it only when module is required to
 * have different output log level configurable in runtime. For other cases,
 * consider defining ZF_LOG_LEVEL or using zf_log_set_output_level() function.
 * Output log level override example:
 *
 *   #define ZF_LOG_OUTPUT_LEVEL g_module_log_level
 *   #include <zf_log.h>
 *   static int g_module_log_level = ZF_LOG_INFO;
 *   static void foo() {
 *       ZF_LOGI("Will check g_module_log_level for output log level");
 *   }
 *   void debug_log(bool on) {
 *       g_module_log_level = on? ZF_LOG_DEBUG: ZF_LOG_INFO;
 *   }
 *
 * Note on performance. This expression will be evaluated each time message is
 * logged (except when message log level is disabled - see ZF_LOG_LEVEL for
 * details). Keep this expression as simple as possible, otherwise it will not
 * only add runtime overhead, but also will increase size of call site
 * significantly (which will result in larger executable). The prefered way is
 * to use integeral variable (as in example above). If structure must be used,
 * log_level field must be the first field in this structure:
 *
 *   #define ZF_LOG_OUTPUT_LEVEL (g_config.log_level)
 *   #include <zf_log.h>
 *   struct config {
 *       int log_level;
 *       unsigned other_field;
 *       [...]
 *   };
 *   static config g_config = {ZF_LOG_INFO, 0, ...};
 *
 * This allows compiler to generate more compact load instruction (no need to
 * specify offset since it's zero). Calling a function to get output log level
 * is generaly a bad idea, since it will increase call site size and runtime
 * overhead even further.
 */
#if defined(ZF_LOG_OUTPUT_LEVEL)
	#define _ZF_LOG_OUTPUT_LEVEL ZF_LOG_OUTPUT_LEVEL
#else
	#define _ZF_LOG_OUTPUT_LEVEL _zf_log_global_output_lvl
#endif

/* Log tag configuration:
 * - ZF_LOG_DEF_TAG - defines default log tag.
 * - ZF_LOG_TAG - overrides default log tag.
 *
 * Tag usually identifies component or module. Tag prefix identifies context in
 * which component or module is running (e.g. process name). For more details
 * about tag prefix see zf_log_set_tag_prefix() function. Output example:
 *
 *   04-29 22:43:20.244 40059  1299 I hello.MAIN Number of arguments: 1
 *                                    |     |
 *                                    |     +- tag
 *                                    +- tag prefix
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
 * the log message (tag prefix still could be added though).
 */
#if defined(ZF_LOG_TAG)
	#define _ZF_LOG_TAG ZF_LOG_TAG
#elif defined(ZF_LOG_DEF_TAG)
	#define _ZF_LOG_TAG ZF_LOG_DEF_TAG
#else
	#define _ZF_LOG_TAG 0
#endif

/* Static (compile-time) initialization support. Set of macros below allows to
 * define and initialize zf_log variables outside of the library. Such
 * initialization will be performed at compile-time and will not have run-time
 * overhead. Also it allows to avoid initialization code inside main() function.
 * That means that logging could be used even before entering main() function,
 * for example in global object constructors or in global variable
 * initialization functions (C++ features).
 * To use that, library must be compiled with following macros defined:
 * - ZF_LOG_EXTERN_TAG_PREFIX for ZF_LOG_DEFINE_TAG_PREFIX
 * - ZF_LOG_EXTERN_GLOBAL_FORMAT for ZF_LOG_DEFINE_GLOBAL_FORMAT
 * - ZF_LOG_EXTERN_GLOBAL_OUTPUT for ZF_LOG_DEFINE_GLOBAL_OUTPUT
 * - ZF_LOG_EXTERN_GLOBAL_OUTPUT_LEVEL for ZF_LOG_DEFINE_GLOBAL_OUTPUT_LEVEL
 * Example:
 *
 *   ZF_LOG_DEFINE_TAG_PREFIX = "TagPrefix";
 *   ZF_LOG_DEFINE_GLOBAL_FORMAT = {CUSTOM_MEM_WIDTH};
 *   ZF_LOG_DEFINE_GLOBAL_OUTPUT = {ZF_LOG_PUT_STD, custom_output_callback};
 *   ZF_LOG_DEFINE_GLOBAL_OUTPUT_LEVEL = ZF_LOG_INFO;
 *
 * When zf_log library compiled with one of ZF_LOG_EXTERN_XXX macros defined,
 * corresponding ZF_LOG_DEFINE_XXX macro MUST be used exactly once somewhere.
 * Otherwise it will result in link error (undefined symbol).
 */
#define ZF_LOG_DEFINE_TAG_PREFIX const char *_zf_log_tag_prefix
#define ZF_LOG_DEFINE_GLOBAL_FORMAT zf_log_format _zf_log_global_format
#define ZF_LOG_DEFINE_GLOBAL_OUTPUT zf_log_output _zf_log_global_output
#define ZF_LOG_DEFINE_GLOBAL_OUTPUT_LEVEL int _zf_log_global_output_lvl

/* Pointer to global format variable. Direct modification is not allowed. Use
 * zf_log_set_mem_width() instead.
 */
#define ZF_LOG_GLOBAL_FORMAT ((const zf_log_format *)&_zf_log_global_format)

/* Pointer to global output variable. Direct modification is not allowed. Use
 * zf_log_set_output_callback() instead.
 */
#define ZF_LOG_GLOBAL_OUTPUT ((const zf_log_output *)&_zf_log_global_output)

/* When defined, all symbols produced by linker will be prefixed with its value.
 * That allows to use zf_log library privately in another library without
 * exposing zf_log symbols in their original form (to avoid possible conflicts
 * with other libraries / components that also use zf_log for logging).
 * Value must be without quotes:
 *
 *   CC_ARGS := -DZF_LOG_LIBRARY_PREFIX=my_lib
 */
#ifdef ZF_LOG_LIBRARY_PREFIX
	#define _ZF_LOG_DECOR__(prefix, name) prefix ## name
	#define _ZF_LOG_DECOR_(prefix, name) _ZF_LOG_DECOR__(prefix, name)
	#define _ZF_LOG_DECOR(name) _ZF_LOG_DECOR_(ZF_LOG_LIBRARY_PREFIX, name)

	#define zf_log_set_tag_prefix _ZF_LOG_DECOR(zf_log_set_tag_prefix)
	#define zf_log_set_mem_width _ZF_LOG_DECOR(zf_log_set_mem_width)
	#define zf_log_set_output_level _ZF_LOG_DECOR(zf_log_set_output_level)
	#define zf_log_set_output_callback _ZF_LOG_DECOR(zf_log_set_output_callback)
	#define zf_log_out_android_callback _ZF_LOG_DECOR(zf_log_out_android_callback)
	#define zf_log_out_nslog_callback _ZF_LOG_DECOR(zf_log_out_nslog_callback)
	#define zf_log_out_debugstring_callback _ZF_LOG_DECOR(zf_log_out_debugstring_callback)
	#define zf_log_out_stderr_callback _ZF_LOG_DECOR(zf_log_out_stderr_callback)
	#define _zf_log_tag_prefix _ZF_LOG_DECOR(_zf_log_tag_prefix)
	#define _zf_log_global_format _ZF_LOG_DECOR(_zf_log_global_format)
	#define _zf_log_global_output _ZF_LOG_DECOR(_zf_log_global_output)
	#define _zf_log_global_output_lvl _ZF_LOG_DECOR(_zf_log_global_output_lvl)
	#define _zf_log_write_d _ZF_LOG_DECOR(_zf_log_write_d)
	#define _zf_log_write_aux_d _ZF_LOG_DECOR(_zf_log_write_aux_d)
	#define _zf_log_write _ZF_LOG_DECOR(_zf_log_write)
	#define _zf_log_write_aux _ZF_LOG_DECOR(_zf_log_write_aux)
	#define _zf_log_write_mem_d _ZF_LOG_DECOR(_zf_log_write_mem_d)
	#define _zf_log_write_mem_aux_d _ZF_LOG_DECOR(_zf_log_write_mem_aux_d)
	#define _zf_log_write_mem _ZF_LOG_DECOR(_zf_log_write_mem)
	#define _zf_log_write_mem_aux _ZF_LOG_DECOR(_zf_log_write_mem_aux)
#endif

/* Runtime configuration */
#ifdef __cplusplus
extern "C" {
#endif

/* Set tag prefix. Prefix will be separated from the tag with dot ('.').
 * Use 0 or empty string to disable (default). Common use is to set it to
 * the process (or build target) name (e.g. to separate client and server
 * processes). Function will NOT copy provided prefix string, but will store the
 * pointer. Hence specified prefix string must remain valid until process exit.
 */
void zf_log_set_tag_prefix(const char *const prefix);

/* Set number of bytes per log line in memory (ASCII-HEX) dump. Example:
 *
 *   I hello.MAIN 4c6f72656d20697073756d20646f6c6f  Lorem ipsum dolo
 *                |<-          w bytes         ->|  |<-  w chars ->|
 */
void zf_log_set_mem_width(const unsigned w);

/* Set output log level. Output log level is a run time check. When log level is
 * below output log level it said to be "turned off" (or just "off" for short).
 * Otherwise it's "turned on" (or just "on").
 *
 * Log messages that are turned off has low overhead of compare operation and
 * conditional jump. Format arguments are not evaluated. For log messages that
 * are turned on output callback function will be invoked.
 *
 * Since all log messages that are disabled (below current log level) will be
 * compiled out, only log messages that are enabled will be affected by the
 * output log level.
 *
 * Output log level can be changed at any time during program execution.
 */
void zf_log_set_output_level(const int lvl);

/* Flags that control what information to include in each log line. Default
 * value is ZF_LOG_PUT_STD and other flags could be used to alter its behavior.
 */
enum
{
	ZF_LOG_PUT_CTX = 1 << 0, /* put context (time, pid, tid) */
	ZF_LOG_PUT_TAG = 1 << 1, /* put tag (including tag prefix) */
	ZF_LOG_PUT_SRC = 1 << 2, /* put source location (file, line, function) */
	ZF_LOG_PUT_MSG = 1 << 3, /* put message text */
	ZF_LOG_PUT_STD = 0xffff, /* put everything by default */
};

//TODO: rename to zf_log_line or zf_log_message
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

/* Set output callback function. It will be called for each log line allowed
 * by both current log level and output log level (enabled and turned on).
 * Callback function is allowed to modify content of the buffers pointed by the
 * ctx, but it's not allowed to modify buffer pointers and other fields.
 *
 * Mask allows to control what information will be added to the log line buffer
 * before callback function is invoked. Default mask value is ZF_LOG_PUT_STD.
 *
 * String inside buffer is UTF-8 encoded (no BOM mark).
 * FIXME: rename to zf_log_set_output()
 */
void zf_log_set_output_callback(const unsigned mask, const zf_log_output_cb cb);

typedef struct zf_log_format
{
	unsigned mem_width; /* Bytes per line in memory (ASCII-HEX) dump */
}
zf_log_format;

typedef struct zf_log_output
{
	unsigned put_mask; /* What to put into log line buffer */
	zf_log_output_cb output_cb; /* Output callback */
}
zf_log_output;

//FIXME: rename to zf_log_spec
typedef struct zf_log_instance
{
	const zf_log_format *format; /* Bytes per line in memory (ASCII-HEX) dump */
	const zf_log_output *output;
}
zf_log_instance;

#ifdef __cplusplus
}
#endif

/* Check current log level at compile time (ignoring output log level).
 * Evaluates to true when specified log level is enabled. For example:
 *
 *   #if ZF_LOG_ENABLED_DEBUG
 *       const char *const g_enum_strings[] = {
 *           "enum_value_0", "enum_value_1", "enum_value_2"
 *       };
 *   #endif
 *   // ...
 *   #if ZF_LOG_ENABLED_DEBUG
 *       ZF_LOGD("enum value: %s", g_enum_strings[v]);
 *   #endif
 */
#define ZF_LOG_ENABLED(lvl)     ((lvl) >= _ZF_LOG_LEVEL)
#define ZF_LOG_ENABLED_VERBOSE  ZF_LOG_ENABLED(ZF_LOG_VERBOSE)
#define ZF_LOG_ENABLED_DEBUG    ZF_LOG_ENABLED(ZF_LOG_DEBUG)
#define ZF_LOG_ENABLED_INFO     ZF_LOG_ENABLED(ZF_LOG_INFO)
#define ZF_LOG_ENABLED_WARN     ZF_LOG_ENABLED(ZF_LOG_WARN)
#define ZF_LOG_ENABLED_ERROR    ZF_LOG_ENABLED(ZF_LOG_ERROR)
#define ZF_LOG_ENABLED_FATAL    ZF_LOG_ENABLED(ZF_LOG_FATAL)

/* Check output log level at run time (taking into account current log
 * level as well). Evaluares to true when specified log level is turned on AND
 * enabled. For example:
 *
 *   if (ZF_LOG_ON_DEBUG)
 *   {
 *       char hash[65];
 *       sha256(data_ptr, data_sz, hash);
 *       ZF_LOGD("data: len=%u, sha256=%s", data_sz, hash);
 *   }
 */
#define ZF_LOG_ON(lvl) \
		(ZF_LOG_ENABLED((lvl)) && (lvl) >= _ZF_LOG_OUTPUT_LEVEL)
#define ZF_LOG_ON_VERBOSE   ZF_LOG_ON(ZF_LOG_VERBOSE)
#define ZF_LOG_ON_DEBUG     ZF_LOG_ON(ZF_LOG_DEBUG)
#define ZF_LOG_ON_INFO      ZF_LOG_ON(ZF_LOG_INFO)
#define ZF_LOG_ON_WARN      ZF_LOG_ON(ZF_LOG_WARN)
#define ZF_LOG_ON_ERROR     ZF_LOG_ON(ZF_LOG_ERROR)
#define ZF_LOG_ON_FATAL     ZF_LOG_ON(ZF_LOG_FATAL)

#ifdef __printflike
	#define _ZF_LOG_PRINTFLIKE(a, b) __printflike(a, b)
#else
	#define _ZF_LOG_PRINTFLIKE(a, b)
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const char *_zf_log_tag_prefix;
extern zf_log_format _zf_log_global_format;
extern zf_log_output _zf_log_global_output;
extern int _zf_log_global_output_lvl;

void _zf_log_write_d(
		const char *const func, const char *const file, const unsigned line,
		const int lvl, const char *const tag,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(6, 7);
void _zf_log_write_aux_d(
		const char *const func, const char *const file, const unsigned line,
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(7, 8);
void _zf_log_write(
		const int lvl, const char *const tag,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(3, 4);
void _zf_log_write_aux(
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(4, 5);
void _zf_log_write_mem_d(
		const char *const func, const char *const file, const unsigned line,
		const int lvl, const char *const tag,
		const void *const d, const unsigned d_sz,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(8, 9);
void _zf_log_write_mem_aux_d(
		const char *const func, const char *const file, const unsigned line,
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const void *const d, const unsigned d_sz,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(9, 10);
void _zf_log_write_mem(
		const int lvl, const char *const tag,
		const void *const d, const unsigned d_sz,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(5, 6);
void _zf_log_write_mem_aux(
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const void *const d, const unsigned d_sz,
		const char *const fmt, ...) _ZF_LOG_PRINTFLIKE(6, 7);

#ifdef __cplusplus
}
#endif

/* Message logging macros:
 * - ZF_LOGV("format string", args, ...)
 * - ZF_LOGD("format string", args, ...)
 * - ZF_LOGI("format string", args, ...)
 * - ZF_LOGW("format string", args, ...)
 * - ZF_LOGE("format string", args, ...)
 * - ZF_LOGF("format string", args, ...)
 *
 * Memory logging macros:
 * - ZF_LOGV_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGD_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGI_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGW_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGE_MEM(data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGF_MEM(data_ptr, data_sz, "format string", args, ...)
 *
 * Auxiliary logging macros:
 * - ZF_LOGV_AUX(&log_instance, "format string", args, ...)
 * - ZF_LOGD_AUX(&log_instance, "format string", args, ...)
 * - ZF_LOGI_AUX(&log_instance, "format string", args, ...)
 * - ZF_LOGW_AUX(&log_instance, "format string", args, ...)
 * - ZF_LOGE_AUX(&log_instance, "format string", args, ...)
 * - ZF_LOGF_AUX(&log_instance, "format string", args, ...)
 *
 * Auxiliary memory logging macros:
 * - ZF_LOGV_MEM_AUX(&log_instance, data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGD_MEM_AUX(&log_instance, data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGI_MEM_AUX(&log_instance, data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGW_MEM_AUX(&log_instance, data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGE_MEM_AUX(&log_instance, data_ptr, data_sz, "format string", args, ...)
 * - ZF_LOGF_MEM_AUX(&log_instance, data_ptr, data_sz, "format string", args, ...)
 *
 * Preformatted string logging macros:
 * - ZF_LOGV_STR("preformatted string");
 * - ZF_LOGD_STR("preformatted string");
 * - ZF_LOGI_STR("preformatted string");
 * - ZF_LOGW_STR("preformatted string");
 * - ZF_LOGE_STR("preformatted string");
 * - ZF_LOGF_STR("preformatted string");
 *
 * Format string follows printf() conventions. Both data_ptr and data_sz could
 * be 0. Most compilers will verify that type of arguments match format
 * specifiers in format string.
 *
 * Library assuming UTF-8 encoding for all strings (char *), including format
 * string itself.
 */
#ifdef NDEBUG
	#define _ZF_LOG_IMP(lvl, tag, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write(lvl, tag, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_MEM_IMP(lvl, tag, d, d_sz, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write_mem(lvl, tag, d, d_sz, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_AUX_IMP(log, lvl, tag, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write_aux(log, lvl, tag, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_MEM_AUX_IMP(log, lvl, tag, d, d_sz, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write_mem_aux(log, lvl, tag, d, d_sz, __VA_ARGS__); \
			} while (0)
#else
	#define _ZF_LOG_IMP(lvl, tag, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write_d(__FUNCTION__, __FILE__, __LINE__, \
							lvl, tag, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_MEM_IMP(lvl, tag, d, d_sz, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write_mem_d(__FUNCTION__, __FILE__, __LINE__, \
							lvl, tag, d, d_sz, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_AUX_IMP(log, lvl, tag, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write_d(__FUNCTION__, __FILE__, __LINE__, \
							log, lvl, tag, __VA_ARGS__); \
			} while (0)
	#define _ZF_LOG_MEM_AUX_IMP(log, lvl, tag, d, d_sz, ...) \
			do { \
				if (ZF_LOG_ON(lvl)) \
					_zf_log_write_mem_aux_d(__FUNCTION__, __FILE__, __LINE__, \
							log, lvl, tag, d, d_sz, __VA_ARGS__); \
			} while (0)
#endif

static inline void _zf_log_unused(const int dummy, ...) {(void)dummy;}

#define _ZF_LOG_UNUSED(...) \
		do { if (0) _zf_log_unused(0, __VA_ARGS__); } while (0)

#if ZF_LOG_ENABLED_VERBOSE
	#define ZF_LOGV(...) \
			_ZF_LOG_IMP(ZF_LOG_VERBOSE, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGV_AUX(log, ...) \
			_ZF_LOG_AUX_IMP(log, ZF_LOG_VERBOSE, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGV_MEM(d, d_sz, ...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_VERBOSE, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
	#define ZF_LOGV_MEM_AUX(log, d, d_sz, ...) \
			_ZF_LOG_MEM_IMP(log, ZF_LOG_VERBOSE, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
#else
	#define ZF_LOGV(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGV_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGV_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGV_MEM_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ENABLED_DEBUG
	#define ZF_LOGD(...) \
			_ZF_LOG_IMP(ZF_LOG_DEBUG, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGD_AUX(log, ...) \
			_ZF_LOG_AUX_IMP(log, ZF_LOG_DEBUG, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGD_MEM(d, d_sz, ...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_DEBUG, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
	#define ZF_LOGD_MEM_AUX(log, d, d_sz, ...) \
			_ZF_LOG_MEM_AUX_IMP(log, ZF_LOG_DEBUG, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
#else
	#define ZF_LOGD(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGD_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGD_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGD_MEM_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ENABLED_INFO
	#define ZF_LOGI(...) \
			_ZF_LOG_IMP(ZF_LOG_INFO, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGI_AUX(log, ...) \
			_ZF_LOG_AUX_IMP(log, ZF_LOG_INFO, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGI_MEM(d, d_sz, ...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_INFO, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
	#define ZF_LOGI_MEM_AUX(log, d, d_sz, ...) \
			_ZF_LOG_MEM_AUX_IMP(log, ZF_LOG_INFO, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
#else
	#define ZF_LOGI(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGI_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGI_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGI_MEM_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ENABLED_WARN
	#define ZF_LOGW(...) \
			_ZF_LOG_IMP(ZF_LOG_WARN, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGW_AUX(log, ...) \
			_ZF_LOG_AUX_IMP(log, ZF_LOG_WARN, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGW_MEM(d, d_sz, ...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_WARN, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
	#define ZF_LOGW_MEM_AUX(log, d, d_sz, ...) \
			_ZF_LOG_MEM_AUX_IMP(log, ZF_LOG_WARN, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
#else
	#define ZF_LOGW(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGW_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGW_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGW_MEM_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ENABLED_ERROR
	#define ZF_LOGE(...) \
			_ZF_LOG_IMP(ZF_LOG_ERROR, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGE_AUX(log, ...) \
			_ZF_LOG_AUX_IMP(log, ZF_LOG_ERROR, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGE_MEM(d, d_sz, ...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_ERROR, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
	#define ZF_LOGE_MEM_AUX(log, d, d_sz, ...) \
			_ZF_LOG_MEM_AUX_IMP(log, ZF_LOG_ERROR, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
#else
	#define ZF_LOGE(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGE_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGE_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGE_MEM_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#if ZF_LOG_ENABLED_FATAL
	#define ZF_LOGF(...) \
			_ZF_LOG_IMP(ZF_LOG_FATAL, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGF_AUX(log, ...) \
			_ZF_LOG_AUX_IMP(log, ZF_LOG_FATAL, _ZF_LOG_TAG, __VA_ARGS__)
	#define ZF_LOGF_MEM(d, d_sz, ...) \
			_ZF_LOG_MEM_IMP(ZF_LOG_FATAL, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
	#define ZF_LOGF_MEM_AUX(log, d, d_sz, ...) \
			_ZF_LOG_MEM_AUX_IMP(log, ZF_LOG_FATAL, _ZF_LOG_TAG, d, d_sz, __VA_ARGS__)
#else
	#define ZF_LOGF(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGF_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGF_MEM(...) _ZF_LOG_UNUSED(__VA_ARGS__)
	#define ZF_LOGF_MEM_AUX(...) _ZF_LOG_UNUSED(__VA_ARGS__)
#endif

#define ZF_LOGV_STR(s) ZF_LOGV("%s", (s))
#define ZF_LOGD_STR(s) ZF_LOGD("%s", (s))
#define ZF_LOGI_STR(s) ZF_LOGI("%s", (s))
#define ZF_LOGW_STR(s) ZF_LOGW("%s", (s))
#define ZF_LOGE_STR(s) ZF_LOGE("%s", (s))
#define ZF_LOGF_STR(s) ZF_LOGF("%s", (s))

/* Output to standard error stream. Library uses it by default, though in few
 * cases it could be necessary to specify it explicitly. For example, when
 * zf_log library is compiled with ZF_LOG_EXTERN_GLOBAL_OUTPUT, application must
 * define and initialize global output variable:
 *
 *   ZF_LOG_DEFINE_GLOBAL_OUTPUT = ZF_LOG_OUT_STDERR;
 *
 * When using custom output, stderr could be used as a fallback when custom
 * output having problems:
 *
 *   zf_log_set_output_callback(zf_log_out_stderr_callback,
 *                              ZF_LOG_OUT_STDERR_MASK);
 */
enum { ZF_LOG_OUT_STDERR_MASK = ZF_LOG_PUT_STD };
void zf_log_out_stderr_callback(zf_log_output_ctx *const ctx);
#define ZF_LOG_OUT_STDERR {ZF_LOG_OUT_STDERR_MASK, zf_log_out_stderr_callback}

#endif
