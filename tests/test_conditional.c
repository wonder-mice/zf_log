#include <zf_log.c>
#include <zf_test.h>

static unsigned g_logged = 0;
/* Keep it extern (non-static), so compiler has less chances to optimize access
 * to this variable.
 */
unsigned g_forty_two = 42;

static void mock_output_callback(const zf_log_message *msg, void *arg)
{
	(void)msg; (void)arg;
	++g_logged;
}

static unsigned was_logged()
{
	const unsigned logged = g_logged;
	g_logged = 0;
	return logged;
}

static unsigned forty_two()
{
	return g_forty_two;
}

#define EXPECTED_LINES(n) TEST_VERIFY_EQUAL(was_logged(), (n))

static void test_conditional()
{
	ZF_LOG_IF(1 < 2, ZF_LOGI("True"));
	EXPECTED_LINES(1);
	ZF_LOG_IF(2 < 1, ZF_LOGI("False"));
	EXPECTED_LINES(0);

	ZF_LOG_IF(g_forty_two == 42, ZF_LOGI("True"));
	EXPECTED_LINES(1);
	ZF_LOG_IF(g_forty_two != 42, ZF_LOGI("False"));
	EXPECTED_LINES(0);

	ZF_LOG_IF(g_forty_two == 42, ZF_LOGI("True"));
	EXPECTED_LINES(1);
	ZF_LOG_IF(g_forty_two != 42, ZF_LOGI("False"));
	EXPECTED_LINES(0);

	ZF_LOG_IF(forty_two() == 42, ZF_LOGI("True"));
	EXPECTED_LINES(1);
	ZF_LOG_IF(forty_two() != 42, ZF_LOGI("False"));
	EXPECTED_LINES(0);
}

int main(int argc, char *argv[])
{
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, mock_output_callback);

	TEST_RUNNER_CREATE(argc, argv);
	TEST_EXECUTE(test_conditional());
	return TEST_RUNNER_EXIT_CODE();
}
