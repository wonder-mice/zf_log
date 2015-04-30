#include <string.h>
#include <stdbool.h>
#include <zf_test.h>
#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.h>

static bool g_output;
static int g_lvl;
static char g_msg[1024];
static unsigned g_len;

static void reset()
{
	g_output = false;
	zf_log_set_output_level(0);
}

static void output_callback(int lvl, char *s, unsigned len)
{
	g_lvl = lvl;
	strcpy(g_msg, s);
	g_len = len;
	g_output = true;
}

static void test_current_level()
{
	reset();
	ZF_LOGD("no debug log");
	TEST_VERIFY_FALSE(g_output);

	reset();
	ZF_LOGI("info log");
	TEST_VERIFY_TRUE(g_output);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);
}

static void test_output_level()
{
	reset();
	zf_log_set_output_level(ZF_LOG_WARN);
	ZF_LOGD("no debug log");
	TEST_VERIFY_FALSE(g_output);
	ZF_LOGI("no info log");
	TEST_VERIFY_FALSE(g_output);

	reset();
	zf_log_set_output_level(ZF_LOG_INFO);
	ZF_LOGD("no debug log");
	TEST_VERIFY_FALSE(g_output);
	ZF_LOGI("info log");
	TEST_VERIFY_TRUE(g_output);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);

	reset();
	zf_log_set_output_level(ZF_LOG_DEBUG);
	ZF_LOGD("no debug log");
	TEST_VERIFY_FALSE(g_output);
	ZF_LOGI("info log");
	TEST_VERIFY_TRUE(g_output);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);

	reset();
	zf_log_set_output_level(0);
	ZF_LOGD("no debug log");
	TEST_VERIFY_FALSE(g_output);
	ZF_LOGI("info log");
	TEST_VERIFY_TRUE(g_output);
	TEST_VERIFY_EQUAL(g_lvl, ZF_LOG_INFO);
}

int main(int argc, char *argv[])
{
	zf_log_set_output_callback(output_callback);
	TEST_RUNNER_CREATE(argc, argv);

	TEST_EXECUTE(test_current_level());
	TEST_EXECUTE(test_output_level());

	return TEST_RUNNER_EXIT_CODE();
}
