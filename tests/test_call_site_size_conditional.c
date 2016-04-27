#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.h>
#include <time.h>

#if TEST_CONDITION
	#define CONDITION 1 < 2
#else
	#define CONDITION 1 > 2
#endif

#define LOG_SOME_ONCE \
	ZF_LOG_IF(CONDITION, ZF_LOGI("Lorem ipsum dolor sit amet")); \
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
