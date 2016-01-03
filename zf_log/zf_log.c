/* If defined, Android log (android/log.h) will be used by default instead of
 * stderr (when available).
 */
#ifdef ZF_LOG_USE_ANDROID_LOG
	#undef ZF_LOG_USE_ANDROID_LOG
	#define ZF_LOG_USE_ANDROID_LOG 1
#else
	#define ZF_LOG_USE_ANDROID_LOG 0
#endif
/* When defined, NSLog will be used instead of stderr when available. Doesn't
 * use NSLog() directly, but piggybacks on non-public CFLog() function.
 */
#ifdef ZF_LOG_USE_NSLOG
	#undef ZF_LOG_USE_NSLOG
	#define ZF_LOG_USE_NSLOG 1
#else
	#define ZF_LOG_USE_NSLOG 0
#endif
/* When defined, OutputDebugString() will be used instead of stderr when
 * available. Uses OutputDebugStringA() variant and feeds it with UTF-8 data.
 */
#ifdef ZF_LOG_USE_DEBUGSTRING
	#undef ZF_LOG_USE_DEBUGSTRING
	#define ZF_LOG_USE_DEBUGSTRING 1
#else
	#define ZF_LOG_USE_DEBUGSTRING 0
#endif
/* When defined, zf_log library will not contain definition of tag prefix
 * variable. In that case it must be defined elsewhere using
 * ZF_LOG_DEFINE_TAG_PREFIX macro, for example:
 *
 *   ZF_LOG_DEFINE_TAG_PREFIX = "ProcessName";
 *
 * This allows to specify custom value for static initialization and avoid
 * overhead of setting this value in runtime.
 */
#ifdef ZF_LOG_EXTERN_TAG_PREFIX
	#undef ZF_LOG_EXTERN_TAG_PREFIX
	#define ZF_LOG_EXTERN_TAG_PREFIX 1
#else
	#define ZF_LOG_EXTERN_TAG_PREFIX 0
#endif
/* When defined, zf_log library will not contain definition of global format
 * variable. In that case it must be defined elsewhere using
 * ZF_LOG_DEFINE_GLOBAL_FORMAT macro, for example:
 *
 *   ZF_LOG_DEFINE_GLOBAL_FORMAT = {MEM_WIDTH};
 *
 * This allows to specify custom value for static initialization and avoid
 * overhead of setting this value in runtime.
 */
#ifdef ZF_LOG_EXTERN_GLOBAL_FORMAT
	#undef ZF_LOG_EXTERN_GLOBAL_FORMAT
	#define ZF_LOG_EXTERN_GLOBAL_FORMAT 1
#else
	#define ZF_LOG_EXTERN_GLOBAL_FORMAT 0
#endif
/* When defined, zf_log library will not contain definition of global output
 * variable. In that case it must be defined elsewhere using
 * ZF_LOG_DEFINE_GLOBAL_OUTPUT macro, for example:
 *
 *   ZF_LOG_DEFINE_GLOBAL_OUTPUT = {ZF_LOG_PUT_STD, custom_output_callback};
 *
 * This allows to specify custom value for static initialization and avoid
 * overhead of setting this value in runtime.
 */
#ifdef ZF_LOG_EXTERN_GLOBAL_OUTPUT
	#undef ZF_LOG_EXTERN_GLOBAL_OUTPUT
	#define ZF_LOG_EXTERN_GLOBAL_OUTPUT 1
#else
	#define ZF_LOG_EXTERN_GLOBAL_OUTPUT 0
#endif
/* When defined, zf_log library will not contain definition of global output
 * level variable. In that case it must be defined elsewhere using
 * ZF_LOG_DEFINE_GLOBAL_OUTPUT_LEVEL macro, for example:
 *
 *   ZF_LOG_DEFINE_GLOBAL_OUTPUT_LEVEL = ZF_LOG_WARN;
 *
 * This allows to specify custom value for static initialization and avoid
 * overhead of setting this value in runtime.
 */
#ifdef ZF_LOG_EXTERN_GLOBAL_OUTPUT_LEVEL
	#undef ZF_LOG_EXTERN_GLOBAL_OUTPUT_LEVEL
	#define ZF_LOG_EXTERN_GLOBAL_OUTPUT_LEVEL 1
#else
	#define ZF_LOG_EXTERN_GLOBAL_OUTPUT_LEVEL 0
#endif
/* When defined, implementation will prefer smaller code size over speed.
 * Disabled by default.
 */
#ifdef ZF_LOG_OPTIMIZE_SIZE
	#undef ZF_LOG_OPTIMIZE_SIZE
	#define ZF_LOG_OPTIMIZE_SIZE 1
#else
	#define ZF_LOG_OPTIMIZE_SIZE 0
#endif
/* Size of the log line buffer. The buffer is allocated on the stack. It limits
 * the maximum length of the log line.
 */
#ifndef ZF_LOG_BUF_SZ
	#define ZF_LOG_BUF_SZ 512
#endif
/* String to put in the end of each log line when necessary (can be empty).
 */
#ifndef ZF_LOG_EOL
	#define ZF_LOG_EOL "\n"
#endif
/* Number of bytes to reserve for EOL in the log line buffer (must be >0).
 * Must be larger than or equal to length of ZF_LOG_EOL with terminating null.
 */
#ifndef ZF_LOG_EOL_SZ
	#define ZF_LOG_EOL_SZ sizeof(ZF_LOG_EOL)
#endif
/* Default number of bytes in one line of memory output. For large values
 * ZF_LOG_BUF_SZ also must be increased.
 */
#ifndef ZF_LOG_MEM_WIDTH
	#define ZF_LOG_MEM_WIDTH 32
#endif
/* Compile instrumented version of the library to facilitate unit testing.
 */
#ifndef ZF_LOG_INSTRUMENTED
	#define ZF_LOG_INSTRUMENTED 0
#endif

#if defined(__linux__)
	#define _POSIX_SOURCE
#endif
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "zf_log.h"

#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/time.h>
	#if defined(__linux__)
		#include <linux/limits.h>
	#else
		#include <sys/syslimits.h>
	#endif
#endif

#if defined(__linux__)
	#include <sys/prctl.h>
	#include <sys/types.h>
#endif
#if defined(__linux__) && !defined(__ANDROID__)
	#include <sys/syscall.h>
	/* avoid defining _GNU_SOURCE */
	int syscall(int number, ...);
#endif
#if defined(__MACH__)
	#include <pthread.h>
#endif

#if ZF_LOG_INSTRUMENTED
	#define INSTRUMENTED_CONST
#else
	#define INSTRUMENTED_CONST const
#endif

#define RETVAL_UNUSED(expr) do { while(expr) break; } while(0)
#define STATIC_ASSERT(name, cond) \
	typedef char assert_##name[(cond)? 1: -1]
#ifndef _countof
	#define _countof(xs) (sizeof(xs) / sizeof((xs)[0]))
#endif

typedef void (*time_cb)(struct tm *const tm, unsigned *const usec);
typedef void (*pid_cb)(int *const pid, int *const tid);
typedef void (*buffer_cb)(zf_log_output_ctx *ctx, char *buf);

typedef struct src_location
{
	const char *const func;
	const char *const file;
	const unsigned line;
}
src_location;

typedef struct mem_block
{
	const void *const d;
	const unsigned d_sz;
}
mem_block;

static void time_callback(struct tm *const tm, unsigned *const usec);
static void pid_callback(int *const pid, int *const tid);
static void buffer_callback(zf_log_output_ctx *ctx, char *buf);

STATIC_ASSERT(eol_fits_eol_sz, sizeof(ZF_LOG_EOL) <= ZF_LOG_EOL_SZ);
STATIC_ASSERT(eol_sz_greater_than_zero, 0 < ZF_LOG_EOL_SZ);
STATIC_ASSERT(eol_sz_less_than_buf_sz, ZF_LOG_EOL_SZ < ZF_LOG_BUF_SZ);
#if !defined(_WIN32) && !defined(_WIN64)
	STATIC_ASSERT(buf_sz_less_than_pipe_buf, ZF_LOG_BUF_SZ <= PIPE_BUF);
#endif
static const char c_hex[] = "0123456789abcdef";

static INSTRUMENTED_CONST unsigned g_buf_sz = ZF_LOG_BUF_SZ - ZF_LOG_EOL_SZ;
static INSTRUMENTED_CONST time_cb g_time_cb = time_callback;
static INSTRUMENTED_CONST pid_cb g_pid_cb = pid_callback;
static INSTRUMENTED_CONST buffer_cb g_buffer_cb = buffer_callback;

#if ZF_LOG_USE_ANDROID_LOG && defined(ZF_LOG_OUT_ANDROID)
	#include <android/log.h>

	static inline int android_lvl(const int lvl)
	{
		switch (lvl)
		{
		case ZF_LOG_VERBOSE:
			return ANDROID_LOG_VERBOSE;
		case ZF_LOG_DEBUG:
			return ANDROID_LOG_DEBUG;
		case ZF_LOG_INFO:
			return ANDROID_LOG_INFO;
		case ZF_LOG_WARN:
			return ANDROID_LOG_WARN;
		case ZF_LOG_ERROR:
			return ANDROID_LOG_ERROR;
		case ZF_LOG_FATAL:
			return ANDROID_LOG_FATAL;
		default:
			assert(!"Bad log level");
			return ANDROID_LOG_UNKNOWN;
		}
	}

	void zf_log_out_android_callback(zf_log_output_ctx *const ctx)
	{
		*ctx->p = 0;
		const char *tag = ctx->p;
		if (ctx->tag_e != ctx->tag_b)
		{
			tag = ctx->tag_b;
			*ctx->tag_e = 0;
		}
		__android_log_print(android_lvl(ctx->lvl), tag, "%s", ctx->msg_b);
	}
#endif

#if ZF_LOG_USE_NSLOG && defined(ZF_LOG_OUT_NSLOG)
	#include <CoreFoundation/CoreFoundation.h>
	CF_EXPORT void CFLog(int32_t level, CFStringRef format, ...);

	static inline int apple_lvl(const int lvl)
	{
		switch (lvl)
		{
		case ZF_LOG_VERBOSE:
			return 7; /* ASL_LEVEL_DEBUG / kCFLogLevelDebug */;
		case ZF_LOG_DEBUG:
			return 7; /* ASL_LEVEL_DEBUG / kCFLogLevelDebug */;
		case ZF_LOG_INFO:
			return 6; /* ASL_LEVEL_INFO / kCFLogLevelInfo */;
		case ZF_LOG_WARN:
			return 4; /* ASL_LEVEL_WARNING / kCFLogLevelWarning */;
		case ZF_LOG_ERROR:
			return 3; /* ASL_LEVEL_ERR / kCFLogLevelError */;
		case ZF_LOG_FATAL:
			return 0; /* ASL_LEVEL_EMERG / kCFLogLevelEmergency */;
		default:
			assert(!"Bad log level");
			return 0; /* ASL_LEVEL_EMERG / kCFLogLevelEmergency */;
		}
	}

	void zf_log_out_nslog_callback(zf_log_output_ctx *const ctx)
	{
		*ctx->p = 0;
		CFLog(apple_lvl(ctx->lvl), CFSTR("%s"), ctx->tag_b);
	}
#endif

#if ZF_LOG_USE_DEBUGSTRING && defined(ZF_LOG_OUT_DEBUGSTRING)
	#include <windows.h>

	void zf_log_out_debugstring_callback(zf_log_output_ctx *const ctx)
	{
		*ctx->p = 0;
		OutputDebugStringA(ctx->p);
	}
#endif

void zf_log_out_stderr_callback(zf_log_output_ctx *const ctx)
{
#if defined(_WIN32) || defined(_WIN64)
	const unsigned eol_len = sizeof(ZF_LOG_EOL) - 1;
	memcpy(ctx->p, ZF_LOG_EOL, eol_len);
	/* WriteFile() is atomic for local files opened with FILE_APPEND_DATA and
	   without FILE_WRITE_DATA */
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), ctx->buf,
			  (DWORD)(ctx->p - ctx->buf + eol_len), 0, 0);
#else
	const unsigned eol_len = sizeof(ZF_LOG_EOL) - 1;
	memcpy(ctx->p, ZF_LOG_EOL, eol_len);
	/* write() is atomic for buffers less than or equal to PIPE_BUF. */
	RETVAL_UNUSED(write(STDERR_FILENO, ctx->buf, ctx->p - ctx->buf + eol_len));
#endif
}

#if !ZF_LOG_EXTERN_TAG_PREFIX
	ZF_LOG_DEFINE_TAG_PREFIX = 0;
#endif

#if !ZF_LOG_EXTERN_GLOBAL_FORMAT
	ZF_LOG_DEFINE_GLOBAL_FORMAT = {ZF_LOG_MEM_WIDTH};
#endif

#if !ZF_LOG_EXTERN_GLOBAL_OUTPUT
	#if ZF_LOG_USE_ANDROID_LOG && defined(ZF_LOG_OUT_ANDROID)
		ZF_LOG_DEFINE_GLOBAL_OUTPUT = ZF_LOG_OUT_ANDROID;
	#elif ZF_LOG_USE_NSLOG && defined(ZF_LOG_OUT_NSLOG)
		ZF_LOG_DEFINE_GLOBAL_OUTPUT = ZF_LOG_OUT_NSLOG;
	#elif ZF_LOG_USE_DEBUGSTRING && defined(ZF_LOG_OUT_DEBUGSTRING)
		ZF_LOG_DEFINE_GLOBAL_OUTPUT = ZF_LOG_OUT_DEBUGSTRING;
	#else
		ZF_LOG_DEFINE_GLOBAL_OUTPUT = ZF_LOG_OUT_STDERR;
	#endif
#endif

#if !ZF_LOG_EXTERN_GLOBAL_OUTPUT_LEVEL
	ZF_LOG_DEFINE_GLOBAL_OUTPUT_LEVEL = 0;
#endif

static const zf_log_instance global_spec =
{
	ZF_LOG_GLOBAL_FORMAT,
	ZF_LOG_GLOBAL_OUTPUT,
};

static char lvl_char(const int lvl)
{
	switch (lvl)
	{
	case ZF_LOG_VERBOSE:
		return 'V';
	case ZF_LOG_DEBUG:
		return 'D';
	case ZF_LOG_INFO:
		return 'I';
	case ZF_LOG_WARN:
		return 'W';
	case ZF_LOG_ERROR:
		return 'E';
	case ZF_LOG_FATAL:
		return 'F';
	default:
		assert(!"Bad log level");
		return '?';
	}
}

#if !ZF_LOG_OPTIMIZE_SIZE && !defined(__WIN32) && !defined(_WIN64)
static pthread_rwlock_t g_tcache_lock = PTHREAD_RWLOCK_INITIALIZER;
static struct timeval g_tcache_tv = {0, 0};
static struct tm g_tcache_tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

static void time_callback(struct tm *const tm, unsigned *const usec)
{
#if defined(__WIN32) || defined(_WIN64)
	SYSTEMTIME st;
	GetLocalTime(&st);
	tm->tm_year = st.wYear;
	tm->tm_mon = st.wMonth;
	tm->tm_mday = st.wDay;
	tm->tm_wday = st.wDayOfWeek;
	tm->tm_hour = st.wHour;
	tm->tm_min = st.wMinute;
	tm->tm_sec = st.wSecond;
	*usec = 1000 * st.wMilliseconds;
#else
	struct timeval tv;
	gettimeofday(&tv, 0);
	#if !ZF_LOG_OPTIMIZE_SIZE
		pthread_rwlock_rdlock(&g_tcache_lock);
		if (g_tcache_tv.tv_sec == tv.tv_sec)
		{
			*tm = g_tcache_tm;
		}
		else
		{
			pthread_rwlock_unlock(&g_tcache_lock);
	#endif
			localtime_r(&tv.tv_sec, tm);
	#if !ZF_LOG_OPTIMIZE_SIZE
			pthread_rwlock_wrlock(&g_tcache_lock);
			g_tcache_tv = tv;
			g_tcache_tm = *tm;
		}
		pthread_rwlock_unlock(&g_tcache_lock);
	#endif
	*usec = tv.tv_usec;
#endif
}

static void pid_callback(int *const pid, int *const tid)
{
#if defined(_WIN32) || defined(_WIN64)
	*pid = GetCurrentProcessId();
#else
	*pid = getpid();
#endif

#if defined(_WIN32) || defined(_WIN64)
	*tid = GetCurrentThreadId();
#elif defined(__ANDROID__)
	*tid = gettid();
#elif defined(__linux__)
	*tid = syscall(SYS_gettid);
#elif defined(__MACH__)
	*tid = pthread_mach_thread_np(pthread_self());
#else
	#define Platform not supported
#endif
}

static void buffer_callback(zf_log_output_ctx *ctx, char *buf)
{
	ctx->e = (ctx->p = ctx->buf = buf) + g_buf_sz;
}

static const char *filename(const char *file)
{
	const char *f = file;
	for (const char *p = file; 0 != *p; ++p)
	{
		if ('/' == *p || '\\' == *p)
		{
			f = p + 1;
		}
	}
	return f;
}

static inline size_t nprintf_size(zf_log_output_ctx *const ctx)
{
	// *nprintf() always puts 0 in the end when input buffer is not empty. This
	// 0 is not desired because its presence sets (ctx->p) to (ctx->e - 1) which
	// leaves space for one more character. Some put_xxx() functions don't use
	// *nprintf() and could use that last character. In that case log line will
	// have multiple (two) half-written parts which is confusing. To workaround
	// that we allow *nprintf() to write its 0 in the eol area (which is always
	// not empty).
	return ctx->e - ctx->p + 1;
}

static inline void put_nprintf(zf_log_output_ctx *const ctx, const int n)
{
	if (0 < n && ctx->e < (ctx->p += n))
	{
		ctx->p = ctx->e;
	}
}

#if !ZF_LOG_OPTIMIZE_SIZE
static inline char *put_padding_r(const unsigned w, const char wc,
								  char *p, char *e)
{
	for (char *const b = e - w; b < p; *--p = wc) {}
	return p;
}

static char *put_integer_r(unsigned v, const int sign,
						   const unsigned w, const char wc, char *const e)
{
	static const char _signs[] = {'-', '0', '+'};
	static const char *const signs = _signs + 1;
	char *p = e;
	do { *--p = '0' + v % 10; } while (0 != (v /= 10));
	if (0 == sign) return put_padding_r(w, wc, p, e);
	if ('0' != wc)
	{
		*--p = signs[sign];
		return put_padding_r(w, wc, p, e);
	}
	p = put_padding_r(w, wc, p, e + 1);
	*--p = signs[sign];
	return p;
}

static inline char *put_uint_r(const unsigned v, const unsigned w, const char wc,
							   char *const e)
{
	return put_integer_r(v, 0, w, wc, e);
}

static inline char *put_int_r(const int v, const unsigned w, const char wc,
							  char *const e)
{
	return 0 <= v? put_integer_r(v, 0, w, wc, e): put_integer_r(-v, -1, w, wc, e);
}

static inline char *put_string(const char *s, char *p, const char *const e)
{
	while (*s && p < e) { *p++ = *s++; }
	return p;
}

static inline char *put_stringn(const char *const s_p, const char *const s_e,
								char *const p, const char *const e)
{
	const ptrdiff_t m = e - p;
	ptrdiff_t n = s_e - s_p;
	if (n > m)
	{
		n = m;
	}
	memcpy(p, s_p, n);
	return p + n;
}

static inline char *put_uint(unsigned v, const unsigned w, const char wc,
							 char *const p, const char *const e)
{
	char buf[16];
	char *const se = buf + _countof(buf);
	char *sp = put_uint_r(v, w, wc, se);
	return put_stringn(sp, se, p, e);
}
#endif

static void put_ctx(zf_log_output_ctx *const ctx)
{
	struct tm tm;
	unsigned usec;
	int pid, tid;
	g_time_cb(&tm, &usec);
	g_pid_cb(&pid, &tid);

#if ZF_LOG_OPTIMIZE_SIZE
	int n;
	n = snprintf(ctx->p, nprintf_size(ctx),
				 "%02u-%02u %02u:%02u:%02u.%03u %5i %5i %c ",
				 (unsigned)(tm.tm_mon + 1), (unsigned)tm.tm_mday,
				 (unsigned)tm.tm_hour, (unsigned)tm.tm_min, (unsigned)tm.tm_sec,
				 (unsigned)(usec / 1000),
				 pid, tid, (char)lvl_char(ctx->lvl));
	put_nprintf(ctx, n);
#else
	char buf[64];
	char *const e = buf + sizeof(buf);
	char *p = e;
	*--p = ' ';
	*--p = lvl_char(ctx->lvl);
	*--p = ' ';
	p = put_int_r(tid, 5, ' ', p);
	*--p = ' ';
	p = put_int_r(pid, 5, ' ', p);
	*--p = ' ';
	p = put_uint_r(usec / 1000, 3, '0', p);
	*--p = '.';
	p = put_uint_r(tm.tm_sec, 2, '0', p);
	*--p = ':';
	p = put_uint_r(tm.tm_min, 2, '0', p);
	*--p = ':';
	p = put_uint_r(tm.tm_hour, 2, '0', p);
	*--p = ' ';
	p = put_uint_r(tm.tm_mday, 2, '0', p);
	*--p = '-';
	p = put_uint_r(tm.tm_mon + 1, 2, '0', p);
	ctx->p = put_stringn(p, e, ctx->p, ctx->e);
#endif
}

static void put_tag(zf_log_output_ctx *const ctx, const char *const tag)
{
	const char *ch;
	ctx->tag_b = ctx->p;
	if (0 != (ch = _zf_log_tag_prefix))
	{
		for (;ctx->e != ctx->p && 0 != (*ctx->p = *ch); ++ctx->p, ++ch)
		{
		}
	}
	if (0 != (ch = tag) && 0 != tag[0])
	{
		if (ctx->tag_b != ctx->p && ctx->e != ctx->p)
		{
			*ctx->p++ = '.';
		}
		for (;ctx->e != ctx->p && 0 != (*ctx->p = *ch); ++ctx->p, ++ch)
		{
		}
	}
	ctx->tag_e = ctx->p;
	if (ctx->tag_b != ctx->p && ctx->e != ctx->p)
	{
		*ctx->p++ = ' ';
	}
}

static void put_src(zf_log_output_ctx *const ctx, const src_location *const src)
{
#if ZF_LOG_OPTIMIZE_SIZE
	int n;
	n = snprintf(ctx->p, nprintf_size(ctx), "%s@%s:%u ",
				 src->func, filename(src->file), src->line);
	put_nprintf(ctx, n);
#else
	ctx->p = put_string(src->func, ctx->p, ctx->e);
	if (ctx->p < ctx->e) *ctx->p++ = '@';
	ctx->p = put_string(filename(src->file), ctx->p, ctx->e);
	if (ctx->p < ctx->e) *ctx->p++ = ':';
	ctx->p = put_uint(src->line, 0, '\0', ctx->p, ctx->e);
	if (ctx->p < ctx->e) *ctx->p++ = ' ';
#endif
}

static void put_msg(zf_log_output_ctx *const ctx,
					const char *const fmt, va_list va)
{
	int n;
	ctx->msg_b = ctx->p;
	n = vsnprintf(ctx->p, nprintf_size(ctx), fmt, va);
	put_nprintf(ctx, n);
}

static void output_mem(const zf_log_instance *log, zf_log_output_ctx *const ctx,
					   const mem_block *const mem)
{
	if (0 == mem->d || 0 == mem->d_sz)
	{
		return;
	}
	const unsigned char *mem_p = (const unsigned char *)mem->d;
	const unsigned char *const mem_e = mem_p + mem->d_sz;
	const unsigned char *mem_cut;
	const ptrdiff_t mem_width = log->format->mem_width;
	char *const hex_b = ctx->msg_b;
	char *const ascii_b = hex_b + 2 * mem_width + 2;
	char *const ascii_e = ascii_b + mem_width;
	if (ctx->e < ascii_e)
	{
		return;
	}
	while (mem_p != mem_e)
	{
		char *hex = hex_b;
		char *ascii = ascii_b;
		for (mem_cut = mem_width < mem_e - mem_p? mem_p + mem_width: mem_e;
			 mem_cut != mem_p; ++mem_p)
		{
			const char ch = *mem_p;
			*hex++ = c_hex[(0xf0 & ch) >> 4];
			*hex++ = c_hex[(0x0f & ch)];
			*ascii++ = isprint(ch)? ch: '?';
		}
		while (hex != ascii_b)
		{
			*hex++ = ' ';
		}
		ctx->p = ascii;
		log->output->output_cb(ctx);
	}
}

void zf_log_set_tag_prefix(const char *const prefix)
{
	_zf_log_tag_prefix = prefix;
}

void zf_log_set_mem_width(const unsigned w)
{
	_zf_log_global_format.mem_width = w;
}

void zf_log_set_output_level(const int lvl)
{
	_zf_log_global_output_lvl = lvl;
}

void zf_log_set_output_callback(const unsigned mask, const zf_log_output_cb cb)
{
	_zf_log_global_output.put_mask = mask;
	_zf_log_global_output.output_cb = cb;
}

static void _zf_log_write_imp(
		const zf_log_instance *log,
		const src_location *const src, const mem_block *const mem,
		const int lvl, const char *const tag, const char *const fmt, va_list va)
{
	zf_log_output_ctx ctx;
	char buf[ZF_LOG_BUF_SZ];
	const unsigned put_mask = log->output->put_mask;
	ctx.lvl = lvl;
	ctx.tag = tag;
	g_buffer_cb(&ctx, buf);
	if (ZF_LOG_PUT_CTX & put_mask)
	{
		put_ctx(&ctx);
	}
	if (ZF_LOG_PUT_TAG & put_mask)
	{
		put_tag(&ctx, tag);
	}
	if (0 != src && ZF_LOG_PUT_SRC & put_mask)
	{
		put_src(&ctx, src);
	}
	if (ZF_LOG_PUT_MSG & put_mask)
	{
		put_msg(&ctx, fmt, va);
	}
	log->output->output_cb(&ctx);
	if (0 != mem && ZF_LOG_PUT_MSG & put_mask)
	{
		output_mem(log, &ctx, mem);
	}
}

void _zf_log_write_d(
		const char *const func, const char *const file, const unsigned line,
		const int lvl, const char *const tag,
		const char *const fmt, ...)
{
	const src_location src = {func, file, line};
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(&global_spec, &src, 0, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write_aux_d(
		const char *const func, const char *const file, const unsigned line,
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const char *const fmt, ...)
{
	const src_location src = {func, file, line};
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(log, &src, 0, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write(const int lvl, const char *const tag,
				   const char *const fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(&global_spec, 0, 0, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write_aux(
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const char *const fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(log, 0, 0, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write_mem_d(
		const char *const func, const char *const file, const unsigned line,
		const int lvl, const char *const tag,
		const void *const d, const unsigned d_sz,
		const char *const fmt, ...)
{
	const src_location src = {func, file, line};
	const mem_block mem = {d, d_sz};
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(&global_spec, &src, &mem, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write_mem_aux_d(
		const char *const func, const char *const file, const unsigned line,
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const void *const d, const unsigned d_sz,
		const char *const fmt, ...)
{
	const src_location src = {func, file, line};
	const mem_block mem = {d, d_sz};
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(log, &src, &mem, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write_mem(const int lvl, const char *const tag,
					   const void *const d, const unsigned d_sz,
					   const char *const fmt, ...)
{
	const mem_block mem = {d, d_sz};
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(&global_spec, 0, &mem, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write_mem_aux(
		const zf_log_instance *const log, const int lvl, const char *const tag,
		const void *const d, const unsigned d_sz,
		const char *const fmt, ...)
{
	const mem_block mem = {d, d_sz};
	va_list va;
	va_start(va, fmt);
	_zf_log_write_imp(log, 0, &mem, lvl, tag, fmt, va);
	va_end(va);
}
