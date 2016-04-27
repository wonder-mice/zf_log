#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.h>
#include <time.h>

#define LOG_SOME_ONCE \
	ZF_LOG_SECRET(ZF_LOGI("Lorem ipsum dolor sit amet")); \
	time(0); \

static void log_some()
{
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
	LOG_SOME_ONCE
}

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	log_some();
	return 0;
}
