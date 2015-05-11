#include <string.h>
#include <stdbool.h>
#include <zf_test.h>
#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.c>

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

static void mock_output_callback(zf_log_output_ctx *ctx)
{
	g_lvl = ctx->lvl;
	g_len = ctx->p - ctx->buf;
	strncpy(g_msg, ctx->buf, g_len);
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

static void test_level_checks()
{
	reset();
#if ZF_LOG_ALLOW_DEBUG
	#error No debug log
#endif
#if !ZF_LOG_ALLOW_INFO
	#error Info log
#endif
	TEST_VERIFY_FALSE(ZF_LOG_OUTPUT_DEBUG);
	TEST_VERIFY_TRUE(ZF_LOG_OUTPUT_INFO);

	reset();
	zf_log_set_output_level(ZF_LOG_WARN);
	TEST_VERIFY_FALSE(ZF_LOG_OUTPUT_INFO);
	TEST_VERIFY_TRUE(ZF_LOG_OUTPUT_WARN);
}

int main(int argc, char *argv[])
{
	zf_log_set_output_callback(mock_output_callback);
	TEST_RUNNER_CREATE(argc, argv);

	TEST_EXECUTE(test_current_level());
	TEST_EXECUTE(test_output_level());
	TEST_EXECUTE(test_args_evaluation());
	TEST_EXECUTE(test_level_checks());

	return TEST_RUNNER_EXIT_CODE();
}
