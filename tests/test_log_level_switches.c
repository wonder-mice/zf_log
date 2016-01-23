#ifndef ZF_LOG_LEVEL
	#error ZF_LOG_LEVEL must be defined for this test
#endif
#include <zf_log.c>
#include <zf_test.h>
#include <string.h>
#include <stdbool.h>

static int g_output_lvl_used;
static unsigned g_output_called;
static unsigned g_arg_called;
static char g_msg[1024];
static unsigned g_msg_len;

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

static void reset()
{
	g_output_called = 0;
	g_arg_called = 0;
	zf_log_set_output_level(0);
}

static void mock_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	g_output_lvl_used = msg->lvl;
	g_msg_len = (unsigned)(msg->p - msg->buf);
	memcpy(g_msg, msg->buf, g_msg_len);
	++g_output_called;
}

static int get_arg()
{
	++g_arg_called;
	return 0;
}

static void test_current_level()
{
	reset();
	ZF_LOGV("verbose log");
	TEST_VERIFY_EQUAL(1 == g_output_called, ZF_LOG_LEVEL <= ZF_LOG_VERBOSE);
	TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_VERBOSE == g_output_lvl_used);
	reset();
	ZF_LOGD("debug log");
	TEST_VERIFY_EQUAL(1 == g_output_called, ZF_LOG_LEVEL <= ZF_LOG_DEBUG);
	TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_DEBUG == g_output_lvl_used);
	reset();
	ZF_LOGI("info log");
	TEST_VERIFY_EQUAL(1 == g_output_called, ZF_LOG_LEVEL <= ZF_LOG_INFO);
	TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_INFO == g_output_lvl_used);
	reset();
	ZF_LOGW("warning log");
	TEST_VERIFY_EQUAL(1 == g_output_called, ZF_LOG_LEVEL <= ZF_LOG_WARN);
	TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_WARN == g_output_lvl_used);
	reset();
	ZF_LOGE("error log");
	TEST_VERIFY_EQUAL(1 == g_output_called, ZF_LOG_LEVEL <= ZF_LOG_ERROR);
	TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_ERROR == g_output_lvl_used);
	reset();
	ZF_LOGF("fatal log");
	TEST_VERIFY_EQUAL(1 == g_output_called, ZF_LOG_LEVEL <= ZF_LOG_FATAL);
	TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_FATAL == g_output_lvl_used);
}

static void test_output_level()
{
	for (unsigned i = 0; _countof(c_levels) > i; ++i)
	{
		const int lvl = c_levels[i];
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGV("verbose log");
		TEST_VERIFY_EQUAL(1 == g_output_called,
						  ZF_LOG_LEVEL <= ZF_LOG_VERBOSE && lvl <= ZF_LOG_VERBOSE);
		TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_VERBOSE == g_output_lvl_used);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGD("debug log");
		TEST_VERIFY_EQUAL(1 == g_output_called,
						  ZF_LOG_LEVEL <= ZF_LOG_DEBUG && lvl <= ZF_LOG_DEBUG);
		TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_DEBUG == g_output_lvl_used);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGI("info log");
		TEST_VERIFY_EQUAL(1 == g_output_called,
						  ZF_LOG_LEVEL <= ZF_LOG_INFO && lvl <= ZF_LOG_INFO);
		TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_INFO == g_output_lvl_used);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGW("warn log");
		TEST_VERIFY_EQUAL(1 == g_output_called,
						  ZF_LOG_LEVEL <= ZF_LOG_WARN && lvl <= ZF_LOG_WARN);
		TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_WARN == g_output_lvl_used);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGE("error log");
		TEST_VERIFY_EQUAL(1 == g_output_called,
						  ZF_LOG_LEVEL <= ZF_LOG_ERROR && lvl <= ZF_LOG_ERROR);
		TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_ERROR == g_output_lvl_used);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGF("fatal log");
		TEST_VERIFY_EQUAL(1 == g_output_called,
						  ZF_LOG_LEVEL <= ZF_LOG_FATAL && lvl <= ZF_LOG_FATAL);
		TEST_VERIFY_TRUE(0 == g_output_called || ZF_LOG_FATAL == g_output_lvl_used);
	}
}

static void test_args_evaluation()
{
	reset();
	ZF_LOGV("verbose log: %i", get_arg());
	TEST_VERIFY_EQUAL(1 == g_arg_called, ZF_LOG_LEVEL <= ZF_LOG_VERBOSE);
	reset();
	ZF_LOGD("debug log: %i", get_arg());
	TEST_VERIFY_EQUAL(1 == g_arg_called, ZF_LOG_LEVEL <= ZF_LOG_DEBUG);
	reset();
	ZF_LOGI("info log: %i", get_arg());
	TEST_VERIFY_EQUAL(1 == g_arg_called, ZF_LOG_LEVEL <= ZF_LOG_INFO);
	reset();
	ZF_LOGW("warning log: %i", get_arg());
	TEST_VERIFY_EQUAL(1 == g_arg_called, ZF_LOG_LEVEL <= ZF_LOG_WARN);
	reset();
	ZF_LOGE("error log: %i", get_arg());
	TEST_VERIFY_EQUAL(1 == g_arg_called, ZF_LOG_LEVEL <= ZF_LOG_ERROR);
	reset();
	ZF_LOGF("fatal log: %i", get_arg());
	TEST_VERIFY_EQUAL(1 == g_arg_called, ZF_LOG_LEVEL <= ZF_LOG_FATAL);

	for (unsigned i = 0; _countof(c_levels) > i; ++i)
	{
		const int lvl = c_levels[i];
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGV("verbose log: %i", get_arg());
		TEST_VERIFY_EQUAL(1 == g_arg_called,
						  ZF_LOG_LEVEL <= ZF_LOG_VERBOSE && lvl <= ZF_LOG_VERBOSE);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGD("debug log: %i", get_arg());
		TEST_VERIFY_EQUAL(1 == g_arg_called,
						  ZF_LOG_LEVEL <= ZF_LOG_DEBUG && lvl <= ZF_LOG_DEBUG);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGI("info log: %i", get_arg());
		TEST_VERIFY_EQUAL(1 == g_arg_called,
						  ZF_LOG_LEVEL <= ZF_LOG_INFO && lvl <= ZF_LOG_INFO);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGW("warn log: %i", get_arg());
		TEST_VERIFY_EQUAL(1 == g_arg_called,
						  ZF_LOG_LEVEL <= ZF_LOG_WARN && lvl <= ZF_LOG_WARN);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGE("error log: %i", get_arg());
		TEST_VERIFY_EQUAL(1 == g_arg_called,
						  ZF_LOG_LEVEL <= ZF_LOG_ERROR && lvl <= ZF_LOG_ERROR);
		reset();
		zf_log_set_output_level(lvl);
		ZF_LOGF("fatal log: %i", get_arg());
		TEST_VERIFY_EQUAL(1 == g_arg_called,
						  ZF_LOG_LEVEL <= ZF_LOG_FATAL && lvl <= ZF_LOG_FATAL);
	}
}

static void test_level_checks()
{
	reset();
	TEST_VERIFY_EQUAL(!!ZF_LOG_ENABLED_VERBOSE, ZF_LOG_LEVEL <= ZF_LOG_VERBOSE);
	TEST_VERIFY_EQUAL(!!ZF_LOG_ENABLED_DEBUG, ZF_LOG_LEVEL <= ZF_LOG_DEBUG);
	TEST_VERIFY_EQUAL(!!ZF_LOG_ENABLED_INFO, ZF_LOG_LEVEL <= ZF_LOG_INFO);
	TEST_VERIFY_EQUAL(!!ZF_LOG_ENABLED_WARN, ZF_LOG_LEVEL <= ZF_LOG_WARN);
	TEST_VERIFY_EQUAL(!!ZF_LOG_ENABLED_ERROR, ZF_LOG_LEVEL <= ZF_LOG_ERROR);
	TEST_VERIFY_EQUAL(!!ZF_LOG_ENABLED_FATAL, ZF_LOG_LEVEL <= ZF_LOG_FATAL);

	for (unsigned i = 0; _countof(c_levels) > i; ++i)
	{
		const int lvl = c_levels[i];
		reset();
		zf_log_set_output_level(lvl);
		TEST_VERIFY_EQUAL(!!ZF_LOG_ON_VERBOSE,
						  ZF_LOG_LEVEL <= ZF_LOG_VERBOSE && lvl <= ZF_LOG_VERBOSE);
		TEST_VERIFY_EQUAL(!!ZF_LOG_ON_DEBUG,
						  ZF_LOG_LEVEL <= ZF_LOG_DEBUG && lvl <= ZF_LOG_DEBUG);
		TEST_VERIFY_EQUAL(!!ZF_LOG_ON_INFO,
						  ZF_LOG_LEVEL <= ZF_LOG_INFO && lvl <= ZF_LOG_INFO);
		TEST_VERIFY_EQUAL(!!ZF_LOG_ON_WARN,
						  ZF_LOG_LEVEL <= ZF_LOG_WARN && lvl <= ZF_LOG_WARN);
		TEST_VERIFY_EQUAL(!!ZF_LOG_ON_ERROR,
						  ZF_LOG_LEVEL <= ZF_LOG_ERROR && lvl <= ZF_LOG_ERROR);
		TEST_VERIFY_EQUAL(!!ZF_LOG_ON_FATAL,
						  ZF_LOG_LEVEL <= ZF_LOG_FATAL && lvl <= ZF_LOG_FATAL);
	}
}

int main(int argc, char *argv[])
{
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, mock_output_callback);
	TEST_RUNNER_CREATE(argc, argv);

	TEST_EXECUTE(test_current_level());
	TEST_EXECUTE(test_output_level());
	TEST_EXECUTE(test_args_evaluation());
	TEST_EXECUTE(test_level_checks());

	return TEST_RUNNER_EXIT_CODE();
}
