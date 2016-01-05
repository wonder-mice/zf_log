#define ZF_LOG_LEVEL 0
#define ZF_LOG_OUTPUT_LEVEL g_output_level
#include <zf_log.c>
#include <zf_test.h>

static const int c_levels[] =
{
	ZF_LOG_VERBOSE,
	ZF_LOG_DEBUG,
	ZF_LOG_INFO,
	ZF_LOG_WARN,
	ZF_LOG_ERROR,
	ZF_LOG_FATAL,
	ZF_LOG_NONE,
};

static int g_output_level = 0;

static void test_level_checks()
{
	for (unsigned i = 0; _countof(c_levels) > i; ++i)
	{
		/* must not effect anything */
		zf_log_set_output_level(c_levels[i]);
		for (unsigned j = 0; _countof(c_levels) > j; ++j)
		{
			g_output_level = c_levels[j];
			TEST_VERIFY_EQUAL(!!ZF_LOG_ON_VERBOSE, g_output_level <= ZF_LOG_VERBOSE);
			TEST_VERIFY_EQUAL(!!ZF_LOG_ON_DEBUG, g_output_level <= ZF_LOG_DEBUG);
			TEST_VERIFY_EQUAL(!!ZF_LOG_ON_INFO, g_output_level <= ZF_LOG_INFO);
			TEST_VERIFY_EQUAL(!!ZF_LOG_ON_WARN, g_output_level <= ZF_LOG_WARN);
			TEST_VERIFY_EQUAL(!!ZF_LOG_ON_ERROR, g_output_level <= ZF_LOG_ERROR);
			TEST_VERIFY_EQUAL(!!ZF_LOG_ON_FATAL, g_output_level <= ZF_LOG_FATAL);
		}
	}
}

int main(int argc, char *argv[])
{
	TEST_RUNNER_CREATE(argc, argv);

	TEST_EXECUTE(test_level_checks());

	return TEST_RUNNER_EXIT_CODE();
}
