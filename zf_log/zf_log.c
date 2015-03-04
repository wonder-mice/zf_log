/* Compile time options:
 *
 * = ZF_LOG_CONF_PTHREADS =
 * Enable/disable pthreads support. 1 - enable, 0 - disable.
 * If not defined will default to 1 (enable).
 */

#ifndef ZF_LOG_CONF_PTHREADS
	#define ZF_LOG_CONF_PTHREADS 1
#endif

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#if ZF_LOG_CONF_PTHREADS
	#include <pthread.h>
#endif
#include <sys/time.h>
#include "zf_log.h"

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
#include <libproc.h>
#endif
#if defined(__linux__)
#include <sys/prctl.h>
#include <sys/types.h>
#endif

#ifdef ANDROID
	#include <android/log.h>
#endif

enum { c_tag_sz = 32 };

struct zf_log_ctx
{
	char prefix[c_tag_sz];
};

#if ZF_LOG_CONF_PTHREADS
static pthread_mutex_t g_init_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
static struct zf_log_ctx g_ctx;
static struct zf_log_ctx *g_ctx_ptr = 0;

static const char c_log_eol[] = "\n";

#ifndef ANDROID
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

#ifdef ANDROID
static int lvl_android(const int lvl)
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

static const char *short_loc(const char *loc)
{
	const char *f = loc;
	for (const char *p = loc; 0 != *p; ++p)
	{
		if ('/' == *p || '\\' == *p)
		{
			f = p + 1;
		}
	}
	return f;
}

static int put_tag(char *const buf, const size_t len,
				   const char *const prefix, const char *const tag)
{
	char *p = buf;
	const char *const e = buf + len;
	if (0 != prefix)
	{
		for (const char *ch = prefix; e != p && 0 != (*p = *ch); ++p, ++ch)
		{
		}
	}
	if (0 != tag && 0 != tag[0])
	{
		if (buf != p && e != p)
		{
			*p++ = '.';
		}
		for (const char *ch = tag; e != p && 0 != (*p = *ch); ++p, ++ch)
		{
		}
	}
	if (e == p && buf < p)
	{
		*--p = 0;
	}
	return p - buf;
}

int put_proc_name(char *const buf, const size_t len)
{
#if defined(__APPLE__) && defined(__MACH__)
	char temp[64];
	int n = proc_name(getpid(), temp, sizeof(temp));
	if (0 >= n)
	{
		buf[0] = 0;
		return 0;
	}
	if (len <= (size_t)n)
	{
		n = len - 1;
	}
	memcpy(buf, temp, n);
	buf[n] = 0;
	return n;
#elif defined(ANDROID)
	FILE *const f = fopen("/proc/self/comm", "r");
	if (0 == f)
	{
		buf[0] = 0;
		return 0;
	}
	size_t n = fread(buf, sizeof(char), len, f);
	fclose(f);
	if (0 < n)
	{
		--n;
	}
	buf[n] = 0;
	return n;
#elif defined(__linux__)
	char exe[256];
	size_t n = readlink("/proc/self/exe", exe, sizeof(exe));
	const char *p = exe + n;
	while (exe != p)
	{
		if ('/' == *--p)
		{
			++p;
			break;
		}
	}
	n -= p - exe;
	memcpy(buf, p, n);
	buf[n] = 0;
	return n;
#else
	#error Unsupported platform
#endif
}

#if !defined(ANDROID)
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

static struct zf_log_ctx *get_ctx()
{
	struct zf_log_ctx *ctx = __atomic_load_n(&g_ctx_ptr, __ATOMIC_RELAXED);
	if (0 != ctx)
	{
		return ctx;
	}
#if ZF_LOG_CONF_PTHREADS
	pthread_mutex_lock(&g_init_lock);
	if (0 == g_ctx_ptr)
	{
#endif
		g_ctx.prefix[0] = 0;
		__atomic_store_n(&g_ctx_ptr, &g_ctx, __ATOMIC_RELEASE);
#if ZF_LOG_CONF_PTHREADS
	}
	pthread_mutex_unlock(&g_init_lock);
#endif
	return g_ctx_ptr;
}

static void log_write(const char *const func, const char *const loc,
					  const int lvl, const char *const tag,
					  const char *const fmt, va_list va)
{
	char buf[256];
	char *p = buf;
	char *const e = buf + sizeof(buf) - sizeof(c_log_eol);
	int n;
	const struct zf_log_ctx *const ctx = get_ctx();

#if !defined(ANDROID)
	struct timeval tv;
	if (0 == gettimeofday(&tv, 0))
	{
		const time_t t = tv.tv_sec;
		struct tm *const tm = localtime(&t);
		if (0 < (n = strftime(p, e - p, "%m-%d %T", tm)))
		{
			p += n;
		}
		n = snprintf(p, e - p, ".%03u ", (unsigned)(tv.tv_usec / 1000));
		if (0 < n && e < (p += n))
		{
			p = e;
		}
	}
	n = snprintf(p, e - p, "%5i %5i %c ",
				 (int)getpid(), (int)thread_id(), (char)lvl_char(lvl));
	if (0 < n && e < (p += n))
	{
		p = e;
	}
	n = put_tag(p, e - p, ctx->prefix, tag);
	p += n;
	if (0 < n && e != p)
	{
		*p++ = ' ';
	}
#endif
	if (0 != func || 0 != loc)
	{
		n = snprintf(p, e - p, "%s@%s ",
					 0 != func? func: "", loc != 0? short_loc(loc): "");
		if (0 < n && e < (p += n))
		{
			p = e;
		}
	}
	n = vsnprintf(p, e - p, fmt, va);
	if (0 < n && e < (p += n))
	{
		p = e;
	}
	if (e == p && 0 == *(p - 1))
	{
		--p;
	}
#if defined(ANDROID)
	*p = 0;
	char tag_buf[c_tag_sz];
	put_tag(tag_buf, sizeof(tag_buf), ctx->prefix, tag);
	__android_log_print(lvl_android(lvl), tag_buf, "%s", buf);
#else
	strcpy(p, c_log_eol);
	fputs(buf, stderr);
#endif
	if (ZF_LOG_FATAL == lvl)
	{
		fflush(stderr);
		abort();
	}
}

int zf_log_set_prefix(const char *const prefix)
{
	struct zf_log_ctx *const ctx = get_ctx();
	char buf[sizeof(ctx->prefix)];
	const char *p = prefix;
	if (ZF_LOG_PROC_NAME == p)
	{
		put_proc_name(buf, sizeof(buf));
		p = buf;
	}
	const int len = strlen(p);
	// As a precaution, copy backward (a bit more safe if there are other
	// threads already reading ctx->prefix). However, that function should be
	// called from main as early as possible when there is only one thread yet.
	for (int i = len + 1; 0 < i--;)
	{
		ctx->prefix[i] = p[i];
	}
	return 0;
}

void _zf_log_write_d(const char *const func, const char *const loc,
					 const int lvl, const char *const tag,
					 const char *const fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	log_write(func, loc, lvl, tag, fmt, va);
	va_end(va);
}

void _zf_log_write(const int lvl, const char *const tag,
				   const char *const fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	log_write(0, 0, lvl, tag, fmt, va);
	va_end(va);
}
