/* Controls Android log (android/log.h) support. When defined, must be 1
 * (enable) or 0 (disable). Disabled by default.
 */
#ifndef ZF_LOG_ANDROID_LOG
	#define ZF_LOG_ANDROID_LOG 0
#endif
/* Controls Apple log (asl.h) support. When defined, must be 1 (enable) or 0
 * (disable). Disabled by default. Doesn't use asl directly, but piggybacks on
 * non-public CFLog() function.
 */
#ifndef ZF_LOG_APPLE_LOG
	#define ZF_LOG_APPLE_LOG 0
#endif
/* Controls whether to add timestamp, pid, tid and level in the log message.
 * When defined, must be 1 (enable) or 0 (disable). If not defined, default
 * will be used.
 */
#ifndef ZF_LOG_PUT_CTX
	#if ZF_LOG_ANDROID_LOG || ZF_LOG_APPLE_LOG
		#define ZF_LOG_PUT_CTX 0
	#else
		#define ZF_LOG_PUT_CTX 1
	#endif
#endif
/* Size of the log line buffer. The buffer is allocated on the stack. It limits
 * the maximum length of the log line.
 */
#ifndef ZF_LOG_BUF_SZ
	#define ZF_LOG_BUF_SZ 256
#endif
/* String to put in the end of each log line when necessary (can be empty).
 */
#ifndef ZF_LOG_EOL
	#define ZF_LOG_EOL "\n"
#endif
/* Number of bytes to reserve for EOL in the log line buffer (must be >0).
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

#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "zf_log.h"

#if defined(__linux__)
	#include <sys/prctl.h>
	#include <sys/types.h>
	#include <sys/syscall.h>
	/* avoid defining _GNU_SOURCE */
	int syscall(int number, ...);
#endif
#if defined(__MACH__) && ZF_LOG_PUT_CTX
	#include <pthread.h>
#endif

#if ZF_LOG_ANDROID_LOG
	#include <android/log.h>
#endif
#if ZF_LOG_APPLE_LOG
	#include <CoreFoundation/CoreFoundation.h>
	CF_EXPORT void CFLog(int32_t level, CFStringRef format, ...);
#endif

#if ZF_LOG_INSTRUMENTED
	#define INSTRUMENTED_CONST
#else
	#define INSTRUMENTED_CONST const
#endif

#define STATIC_ASSERT(name, cond) \
	typedef char assert_##name[(cond)? 1: -1]

typedef void (*time_cb)(struct tm *const tm, unsigned *const usec);
typedef void (*pid_cb)(int *const pid, int *const tid);
typedef void (*buffer_cb)(zf_log_output_ctx *ctx, char *buf);

#if ZF_LOG_PUT_CTX
static void time_callback(struct tm *const tm, unsigned *const usec);
static void pid_callback(int *const pid, int *const tid);
#endif
static void buffer_callback(zf_log_output_ctx *ctx, char *buf);
static void output_callback(zf_log_output_ctx *const ctx);

STATIC_ASSERT(eol_fits_eol_sz, sizeof(ZF_LOG_EOL) <= ZF_LOG_EOL_SZ);
STATIC_ASSERT(eol_sz_greater_than_zero, 0 < ZF_LOG_EOL_SZ);
static const char c_hex[] = "0123456789abcdef";

static const char *g_tag_prefix = 0;
static size_t g_mem_width = ZF_LOG_MEM_WIDTH;
static INSTRUMENTED_CONST unsigned g_buf_sz = ZF_LOG_BUF_SZ - ZF_LOG_EOL_SZ;
#if ZF_LOG_PUT_CTX
static INSTRUMENTED_CONST time_cb g_time_cb = time_callback;
static INSTRUMENTED_CONST pid_cb g_pid_cb = pid_callback;
#endif
static INSTRUMENTED_CONST buffer_cb g_buffer_cb = buffer_callback;
static zf_log_output_cb g_output_cb = output_callback;

int _zf_log_output_lvl = 0;

#if ZF_LOG_ANDROID_LOG
static int android_lvl(const int lvl)
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
#endif

#if ZF_LOG_APPLE_LOG
static int apple_lvl(const int lvl)
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
#endif

#if ZF_LOG_PUT_CTX
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

static void time_callback(struct tm *const tm, unsigned *const usec)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	const time_t t = tv.tv_sec;
	*tm = *localtime(&t);
	*usec = tv.tv_usec;
}

static void pid_callback(int *const pid, int *const tid)
{
	*pid = getpid();
#if defined(__linux__)
	*tid = syscall(SYS_gettid);
#elif defined(__MACH__)
	*tid = pthread_mach_thread_np(pthread_self());
#else
	#define Platform not supported
#endif
}
#endif // ZF_LOG_PUT_CTX

static void buffer_callback(zf_log_output_ctx *ctx, char *buf)
{
	ctx->e = (ctx->p = ctx->buf = buf) + g_buf_sz;
}

static void output_callback(zf_log_output_ctx *const ctx)
{
#if ZF_LOG_ANDROID_LOG
	*ctx->p = 0;
	const char *tag = ctx->p;
	if (ctx->tag_e != ctx->tag_b)
	{
		tag = ctx->tag_b;
		*ctx->tag_e = 0;
	}
	__android_log_print(android_lvl(ctx->lvl), tag, "%s", ctx->msg_b);
#elif ZF_LOG_APPLE_LOG
	*ctx->p = 0;
	CFLog(apple_lvl(ctx->lvl), CFSTR("%s"), ctx->tag_b);
#else
	strcpy(ctx->p, ZF_LOG_EOL);
	fputs(ctx->buf, stderr);
#endif
	if (ZF_LOG_FATAL == ctx->lvl)
	{
		fflush(stderr);
		abort();
	}
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

static void put_ctx(zf_log_output_ctx *const ctx)
{
#if ZF_LOG_PUT_CTX
	int n;
	struct tm tm;
	unsigned usec;
	int pid, tid;
	g_time_cb(&tm, &usec);
	g_pid_cb(&pid, &tid);
	n = snprintf(ctx->p, nprintf_size(ctx),
				 "%02u-%02u %02u:%02u:%02u.%03u %5i %5i %c ",
				 (unsigned)tm.tm_mon, (unsigned)tm.tm_mday,
				 (unsigned)tm.tm_hour, (unsigned)tm.tm_min, (unsigned)tm.tm_sec,
				 (unsigned)(usec / 1000),
				 pid, tid, (char)lvl_char(ctx->lvl));
	put_nprintf(ctx, n);
#else
	(void)ctx;
#endif
}

static void put_tag(zf_log_output_ctx *const ctx, const char *const tag)
{
	const char *ch;
	ctx->tag_b = ctx->p;
	if (0 != (ch = g_tag_prefix))
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

static void put_src(zf_log_output_ctx *const ctx, const char *const func,
					const char *const file, const unsigned line)
{
	int n;
	n = snprintf(ctx->p, nprintf_size(ctx), "%s@%s:%u ",
				 func, filename(file), line);
	put_nprintf(ctx, n);
}

static void put_msg(zf_log_output_ctx *const ctx,
					const char *const fmt, va_list va)
{
	int n;
	ctx->msg_b = ctx->p;
	n = vsnprintf(ctx->p, nprintf_size(ctx), fmt, va);
	put_nprintf(ctx, n);
}

static void output_mem(zf_log_output_ctx *const ctx,
					   const void *const d, const unsigned d_sz)
{
	if (0 == d || 0 == d_sz)
	{
		return;
	}
	const unsigned char *mem_p = (const unsigned char *)d;
	const unsigned char *const mem_e = mem_p + d_sz;
	const unsigned char *mem_cut;
	const ptrdiff_t mem_width = g_mem_width;
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
		g_output_cb(ctx);
	}
}

void zf_log_set_tag_prefix(const char *const prefix)
{
	g_tag_prefix = prefix;
}

void zf_log_set_mem_width(const unsigned w)
{
	g_mem_width = w;
}

void zf_log_set_output_level(const int lvl)
{
	_zf_log_output_lvl = lvl;
}

void zf_log_set_output_callback(const zf_log_output_cb cb)
{
	g_output_cb = cb;
}

#define CTX(lvl_, tag_) \
	zf_log_output_ctx ctx; \
	char buf[ZF_LOG_BUF_SZ]; \
	ctx.lvl = (lvl_); \
	ctx.tag = (tag_); \
	g_buffer_cb(&ctx, buf); \
	(void)0


void _zf_log_write_d(const char *const func,
					 const char *const file, const unsigned line,
					 const int lvl, const char *const tag,
					 const char *const fmt, ...)
{
	CTX(lvl, tag);
	va_list va;
	va_start(va, fmt);
	put_ctx(&ctx);
	put_tag(&ctx, tag);
	put_src(&ctx, func, file, line);
	put_msg(&ctx, fmt, va);
	g_output_cb(&ctx);
	va_end(va);
}

void _zf_log_write(const int lvl, const char *const tag,
				   const char *const fmt, ...)
{
	CTX(lvl, tag);
	va_list va;
	va_start(va, fmt);
	put_ctx(&ctx);
	put_tag(&ctx, tag);
	put_msg(&ctx, fmt, va);
	g_output_cb(&ctx);
	va_end(va);
}

void _zf_log_write_mem_d(const char *const func,
						 const char *const file, const unsigned line,
						 const int lvl, const char *const tag,
						 const void *const d, const unsigned d_sz,
						 const char *const fmt, ...)
{
	CTX(lvl, tag);
	va_list va;
	va_start(va, fmt);
	put_ctx(&ctx);
	put_tag(&ctx, tag);
	put_src(&ctx, func, file, line);
	put_msg(&ctx, fmt, va);
	g_output_cb(&ctx);
	output_mem(&ctx, d, d_sz);
	va_end(va);
}

void _zf_log_write_mem(const int lvl, const char *const tag,
					   const void *const d, const unsigned d_sz,
					   const char *const fmt, ...)
{
	CTX(lvl, tag);
	va_list va;
	va_start(va, fmt);
	put_ctx(&ctx);
	put_tag(&ctx, tag);
	put_msg(&ctx, fmt, va);
	g_output_cb(&ctx);
	output_mem(&ctx, d, d_sz);
	va_end(va);
}
