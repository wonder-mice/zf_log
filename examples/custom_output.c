#include <assert.h>
#include <syslog.h>
#include <zf_log.h>

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

static void syslog_output_callback(zf_log_output_ctx *ctx)
{
	/* p points to the log message end. By default, message is not terminated
	 * with 0, but it has some space allocated for EOL area, so there is always
	 * some place for terminating zero in the end (see ZF_LOG_EOL_SZ define in
	 * zf_log.c).
	 */
	*ctx->p = 0;
	syslog(syslog_level(ctx->lvl), "%s", ctx->tag_b);
}

int main(int argc, char *argv[])
{
	openlog("custom_output", LOG_CONS|LOG_PERROR|LOG_PID, LOG_USER);

	zf_log_set_output_callback(syslog_output_callback);

	ZF_LOGI("You will see the number of arguments: %i", argc);
	ZF_LOGI_MEM(argv, argc * sizeof(*argv), "argv pointers:");

	closelog();
	return 0;
}
