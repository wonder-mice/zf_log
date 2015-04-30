#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
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

#ifdef ANDROID
	#include <android/log.h>
#endif

static void stderr_output_callback(int lvl, char *s, unsigned len)
{
	(void)lvl; (void)len;
	fputs(s, stderr);
}

static const char c_log_eol[] = "\n";

static zf_log_output_cb g_output_cb = stderr_output_callback;
static const char *g_tag_prefix = 0;

int _zf_log_output_lvl = 0;

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

static void log_write(const char *const func, const char *const loc,
					  const int lvl, const char *const tag,
					  const char *const fmt, va_list va)
{
	char buf[256];
	char *p = buf;
	char *const e = buf + sizeof(buf) - sizeof(c_log_eol);
	int n;

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
	n = put_tag(p, e - p, g_tag_prefix, tag);
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
	char tag_buf[32];
	put_tag(tag_buf, sizeof(tag_buf), g_tag_prefix, tag);
	__android_log_print(lvl_android(lvl), tag_buf, "%s", buf);
#else
	strcpy(p, c_log_eol);
	g_output_cb(lvl, buf, p - buf);
#endif
	if (ZF_LOG_FATAL == lvl)
	{
		fflush(stderr);
		abort();
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
