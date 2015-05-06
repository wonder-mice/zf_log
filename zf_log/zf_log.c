/* Library configuration options:
 * - ZF_LOG_ANDROID_LOG - enable android/log.h support
 * - ZF_LOG_PUT_CTX - put timestamp, pid, tid, level, tag in the log line
 * - ZF_LOG_LINE_MAX - maximum log line length
 * - ZF_LOG_EOL - string to put in the end of each log line
 * - ZF_LOG_EOL_SZ - number of bytes to reserve for EOL
 * - ZF_LOG_HEX_WIDTH - number of bytes in one line of hex output
 */
#ifndef ZF_LOG_ANDROID_LOG
	#if defined(ANDROID)
		#define ZF_LOG_ANDROID_LOG 1
	#else
		#define ZF_LOG_ANDROID_LOG 0
	#endif
#endif
#ifndef ZF_LOG_PUT_CTX
	#if ZF_LOG_ANDROID_LOG
		#define ZF_LOG_PUT_CTX 0
	#else
		#define ZF_LOG_PUT_CTX 1
	#endif
#endif
#ifndef ZF_LOG_LINE_MAX
	#define ZF_LOG_LINE_MAX 256
#endif
#ifndef ZF_LOG_EOL
	#define ZF_LOG_EOL "\n"
#endif
#ifndef ZF_LOG_EOL_SZ
	#define ZF_LOG_EOL_SZ 4
#endif
#ifndef ZF_LOG_HEX_WIDTH
	#define ZF_LOG_HEX_WIDTH 32
#endif

#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include "zf_log.h"

#if defined(__linux__)
	#include <sys/prctl.h>
	#include <sys/types.h>
#endif
#if defined(__MACH__)
	#include <pthread.h>
#endif

#if ZF_LOG_ANDROID_LOG
	#include <android/log.h>
#endif

typedef char c_eol_sz_check[sizeof(ZF_LOG_EOL) < ZF_LOG_EOL_SZ? 1: -1];
static const size_t c_eol_sz = ZF_LOG_EOL_SZ;
static const char c_hex[] = "0123456789abcdef";
static const ptrdiff_t c_hex_width = ZF_LOG_HEX_WIDTH;

static void output_callback(zf_log_output_ctx *const ctx);

static zf_log_output_cb g_output_cb = output_callback;
static const char *g_tag_prefix = 0;

int _zf_log_output_lvl = 0;

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
#endif

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

#if ZF_LOG_PUT_CTX
static int thread_id()
{
#if defined(__linux__)
	return gettid();
#elif defined(__MACH__)
	return pthread_mach_thread_np(pthread_self());
#else
	#define Unsupported platform
#endif
}
#endif

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

static void put_ctx(zf_log_output_ctx *const ctx)
{
#if ZF_LOG_PUT_CTX
	int n;
	struct timeval tv;
	if (0 == gettimeofday(&tv, 0))
	{
		const time_t t = tv.tv_sec;
		struct tm *const tm = localtime(&t);
		if (0 < (n = strftime(ctx->p, ctx->e - ctx->p, "%m-%d %T", tm)))
		{
			ctx->p += n;
		}
		n = snprintf(ctx->p, ctx->e - ctx->p, ".%03u ",
					 (unsigned)(tv.tv_usec / 1000));
		if (0 < n && ctx->e < (ctx->p += n))
		{
			ctx->p = ctx->e;
		}
	}
	n = snprintf(ctx->p, ctx->e - ctx->p, "%5i %5i %c ",
				 (int)getpid(), (int)thread_id(), (char)lvl_char(ctx->lvl));
	if (0 < n && ctx->e < (ctx->p += n))
	{
		ctx->p = ctx->e;
	}
#else
	(void)ctx;
#endif
}

static void put_tag(zf_log_output_ctx *const ctx, const char *const tag)
{
	ctx->tag_b = ctx->p;
	if (0 != g_tag_prefix)
	{
		for (const char *ch = g_tag_prefix;
			 ctx->e != ctx->p && 0 != (*ctx->p = *ch); ++ctx->p, ++ch)
		{
		}
	}
	if (0 != tag && 0 != tag[0])
	{
		if (ctx->tag_b != ctx->p && ctx->e != ctx->p)
		{
			*ctx->p++ = '.';
		}
		for (const char *ch = tag;
			 ctx->e != ctx->p && 0 != (*ctx->p = *ch); ++ctx->p, ++ch)
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
	n = snprintf(ctx->p, ctx->e - ctx->p, "%s@%s:%u ",
				 func, filename(file), line);
	if (0 < n && ctx->e < (ctx->p += n))
	{
		ctx->p = ctx->e;
	}
}

static void put_msg(zf_log_output_ctx *const ctx,
					const char *const fmt, va_list va)
{
	int n;
	ctx->msg_b = ctx->p;
	n = vsnprintf(ctx->p, ctx->e - ctx->p, fmt, va);
	if (0 < n && ctx->e < (ctx->p += n))
	{
		ctx->p = ctx->e;
	}
	if (ctx->e == ctx->p && 0 == *(ctx->p - 1))
	{
		--ctx->p;
	}
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

static void output_mem(zf_log_output_ctx *const ctx,
					   const void *const d, const unsigned d_sz)
{
	const unsigned char *mem_p = (const unsigned char *)d;
	const unsigned char *const mem_e = mem_p + d_sz;
	const unsigned char *mem_cut;
	char *const hex_b = ctx->msg_b;
	char *const ascii_b = hex_b + 2 * c_hex_width + 2;
	char *const ascii_e = ascii_b + c_hex_width;
	if (ctx->e < ascii_e)
	{
		return;
	}
	while (mem_p != mem_e)
	{
		char *hex = hex_b;
		char *ascii = ascii_b;
		for (mem_cut = c_hex_width < mem_e - mem_p? mem_p + c_hex_width: mem_e;
			 mem_cut != mem_p; ++mem_p)
		{
			*hex++ = c_hex[(0xf0 & *mem_p) >> 4];
			*hex++ = c_hex[(0x0f & *mem_p)];
			*ascii++ = isprint(*mem_p)? *mem_p: '?';
		}
		while (hex != ascii_b)
		{
			*hex++ = ' ';
		}
		ctx->p = ascii;
		g_output_cb(ctx);
	}
}

void zf_log_set_output_level(const int lvl)
{
	_zf_log_output_lvl = lvl;
}

void zf_log_set_tag_prefix(const char *const prefix)
{
	g_tag_prefix = prefix;
}

void zf_log_set_output_callback(const zf_log_output_cb cb)
{
	g_output_cb = cb;
}

#define CTX(lvl_, tag_) \
	zf_log_output_ctx ctx; \
	char buf[ZF_LOG_LINE_MAX]; \
	ctx.lvl = (lvl_); \
	ctx.tag = (tag_); \
	ctx.buf = buf; \
	ctx.p = buf; \
	ctx.e = buf + sizeof(buf) - c_eol_sz; \
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
