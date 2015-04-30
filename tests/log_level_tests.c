#include <string.h>
#include <stdbool.h>
#include <zf_test.h>
#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.h>

static unsigned g_output;
static int g_lvl;
static char g_msg[1024];
static unsigned g_len;
static unsigned g_arg;

static void reset()
{
	g_output = 0;
	zf_log_set_output_level(0);
	g_arg = 0;
}

static void output_callback(int lvl, char *s, unsigned len)
{
	g_lvl = lvl;
	strcpy(g_msg, s);
	g_len = len;
	++g_output;
}

static int get_arg()
{
	++g_arg;
	return g_arg;
}

static void test_current_level()
{
	reset();
	ZF_LOGD("no debug log");
	TEST_VERIFY_EQUAL(g_output, 0);

	reset();
	ZF_LOGI("info log");
	TEST_VERIFY_EQUAL(g_output, 1);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);
}

static void test_output_level()
{
	reset();
	zf_log_set_output_level(ZF_LOG_WARN);
	ZF_LOGD("no debug log");
	TEST_VERIFY_EQUAL(g_output, 0);
	ZF_LOGI("no info log");
	TEST_VERIFY_EQUAL(g_output, 0);

	reset();
	zf_log_set_output_level(ZF_LOG_INFO);
	ZF_LOGD("no debug log");
	TEST_VERIFY_EQUAL(g_output, 0);
	ZF_LOGI("info log");
	TEST_VERIFY_EQUAL(g_output, 1);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);

	reset();
	zf_log_set_output_level(ZF_LOG_DEBUG);
	ZF_LOGD("no debug log");
	TEST_VERIFY_EQUAL(g_output, 0);
	ZF_LOGI("info log");
	TEST_VERIFY_EQUAL(g_output, 1);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);

	reset();
	zf_log_set_output_level(0);
	ZF_LOGD("no debug log");
	TEST_VERIFY_EQUAL(g_output, 0);
	ZF_LOGI("info log");
	TEST_VERIFY_EQUAL(g_output, 1);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);
}

static void test_args_evaluation()
{
	reset();
	ZF_LOGD("debug log arg not evaluated: %i", get_arg());
	TEST_VERIFY_EQUAL(g_arg, 0);

	reset();
	ZF_LOGI("info log arg evaluated: %i", get_arg());
	TEST_VERIFY_EQUAL(g_arg, 1);

	reset();
	zf_log_set_output_level(ZF_LOG_WARN);
	ZF_LOGI("info log arg not evaluated: %i", get_arg());
	TEST_VERIFY_EQUAL(g_arg, 0);
}

int main(int argc, char *argv[])
{
	zf_log_set_output_callback(output_callback);
	TEST_RUNNER_CREATE(argc, argv);

	TEST_EXECUTE(test_current_level());
	TEST_EXECUTE(test_output_level());
	TEST_EXECUTE(test_args_evaluation());

	return TEST_RUNNER_EXIT_CODE();
}
