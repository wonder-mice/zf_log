#include <zf_log.c>
#include <zf_test.h>
#include <stdio.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && !defined(snprintf)
	#define snprintf(buf, len, ...) _snprintf_s(buf, len, _TRUNCATE, __VA_ARGS__)
#endif

const char *const c_filename = "test_source_location.c";
static char g_srcloc_buf[ZF_LOG_BUF_SZ];
static const char *g_srcloc;

static char *trim(char *s)
{
	char *sb;
	while (0 != *(sb = s) && ' ' == *s) ++s;
	char *se;
	while (0 != *(se = s) && ' ' != *s) ++s;
	*se = 0;
	return sb;
}

static void mock_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	const size_t len = msg->msg_b - msg->tag_e;
	memcpy(g_srcloc_buf, msg->tag_e, len);
	g_srcloc_buf[len] = 0;
	g_srcloc = trim(g_srcloc_buf);
}

static void test_function()
{
	const unsigned line = __LINE__ + 1;
	ZF_LOGI("test message");

	char expected[64];
#if ZF_LOG_SRCLOC_NONE==TEST_SRCLOC
	(void)line;
	*expected = 0;
#endif
#if ZF_LOG_SRCLOC_SHORT==TEST_SRCLOC
	snprintf(expected, sizeof(expected), "@%s:%u",
			 c_filename, line);
#endif
#if ZF_LOG_SRCLOC_LONG==TEST_SRCLOC
	snprintf(expected, sizeof(expected), "%s@%s:%u",
			 _ZF_LOG_FUNCTION, c_filename, line);
#endif
	TEST_VERIFY_EQUAL(strcmp(expected, g_srcloc), 0);
}

int main(int argc, char *argv[])
{
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, mock_output_callback);

	TEST_RUNNER_CREATE(argc, argv);
	TEST_EXECUTE(test_function());
	return TEST_RUNNER_EXIT_CODE();
}
