#include <signal.h>
#include <stdlib.h>
#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.h>

static int call_exit()
{
	exit(1);
}

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;

	/* Current log level is set to ZF_LOG_INFO by defining ZF_LOG_LEVEL
	 * before zf_log.h include. All log messages below INFO level will be
	 * compiled out.
	 */
	ZF_LOGV("Argument of this VERBOSE message will not be evaluated: %i",
			call_exit());
	ZF_LOGI("So you will see that INFO message");

	/* Output log level is set to WARN and then to INFO. Argument of INFO log
	 * statement will be evaluated only once (after setting output log level to
	 * INFO).
	 */
	zf_log_set_output_level(ZF_LOG_WARN);
	int count = 0;
	for (int i = 2; 0 < i--;)
	{
		ZF_LOGI("Argument of this INFO message will be evaluated only once: %i",
				++count);
		zf_log_set_output_level(ZF_LOG_INFO);
	}
	if (1 != count)
	{
		abort();
	}
	ZF_LOGI("And you will see that INFO message");
	return 0;
}
