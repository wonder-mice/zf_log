#include <zf_log.c>
#include <zf_test.h>

static unsigned g_logged = 0;

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

static void some_function_0()
{
	++g_logged;
}

static void some_function_1(const unsigned d)
{
	assert(0 < d);
	g_logged += d;
}

#if TEST_LOG_SECRETS
	#define EXPECTED_LINES(n) TEST_VERIFY_EQUAL(was_logged(), (n))
#else
	#define EXPECTED_LINES(n) TEST_VERIFY_EQUAL(was_logged(), 0)
#endif

static void test_censoring()
{
	const char name[] = "Orion";
	const char address[] = "Space";
	const char cipher[] = "Secret";
	const zf_log_spec spec = {ZF_LOG_GLOBAL_FORMAT, ZF_LOG_GLOBAL_OUTPUT};

	#if ZF_LOG_SECRETS
		ZF_LOGI("Customer name: %s", name);
		ZF_LOGI("Customer address: %s", address);
	#endif
	EXPECTED_LINES(2);

	ZF_LOG_SECRET(ZF_LOGI("Customer name: %s", name));
	EXPECTED_LINES(1);
	ZF_LOG_SECRET(ZF_LOGI_MEM(cipher, sizeof(cipher), "Customer cipher:"));
	EXPECTED_LINES(2);
	ZF_LOG_SECRET(ZF_LOGI_AUX(&spec, "Customer address: %s", address));
	EXPECTED_LINES(1);
	ZF_LOG_SECRET(ZF_LOGI_MEM_AUX(&spec, cipher, sizeof(cipher), "Customer cipher:"));
	EXPECTED_LINES(2);

	ZF_LOG_SECRET(some_function_0());
	EXPECTED_LINES(1);
	ZF_LOG_SECRET(some_function_1(42));
	EXPECTED_LINES(42);

	ZF_LOGI("Must always log this");
	TEST_VERIFY_EQUAL(was_logged(), 1);
}

int main(int argc, char *argv[])
{
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, mock_output_callback);

	TEST_RUNNER_CREATE(argc, argv);
	TEST_EXECUTE(test_censoring());
	return TEST_RUNNER_EXIT_CODE();
}
