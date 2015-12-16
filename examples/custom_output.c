#include <assert.h>
#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	#define OUTPUT_DEBUG_STRING
#else
	#include <syslog.h>
	#define OUTPUT_SYSLOG
#endif
#include <zf_log.h>

#ifdef OUTPUT_SYSLOG
static int syslog_level(const int lvl)
{
	switch (lvl)
	{
	case ZF_LOG_VERBOSE:
		return LOG_DEBUG;
	case ZF_LOG_DEBUG:
		return LOG_DEBUG;
	case ZF_LOG_INFO:
		return LOG_INFO;
	case ZF_LOG_WARN:
		return LOG_WARNING;
	case ZF_LOG_ERROR:
		return LOG_ERR;
	case ZF_LOG_FATAL:
		return LOG_EMERG;
	default:
		assert(!"can't be");
		return LOG_EMERG;
	}
}
#endif

static void custom_output_callback(zf_log_output_ctx *ctx)
{
	/* p points to the log message end. By default, message is not terminated
	 * with 0, but it has some space allocated for EOL area, so there is always
	 * some place for terminating zero in the end (see ZF_LOG_EOL_SZ define in
	 * zf_log.c).
	 */
	*ctx->p = 0;
#if defined(OUTPUT_DEBUG_STRING)
	OutputDebugStringA(ctx->buf);
#elif defined(OUTPUT_SYSLOG)
	syslog(syslog_level(ctx->lvl), "%s", ctx->tag_b);
#else
	#error Unsupported platform
#endif
}

int main(int argc, char *argv[])
{
#if defined(OUTPUT_SYSLOG)
	openlog("custom_output", LOG_CONS|LOG_PERROR|LOG_PID, LOG_USER);
#endif

	zf_log_set_output_callback(custom_output_callback);

	ZF_LOGI("Number of arguments goes into custom output: %i", argc);
	ZF_LOGI_MEM(argv, argc * sizeof(*argv), "and argv pointers as well:");

#if defined(OUTPUT_SYSLOG)
	closelog();
#endif
	return 0;
}
