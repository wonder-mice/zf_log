#include <zf_log.c>
#include <zf_test.h>

static char *strcopy_r(const char *s, char *e)
{
	e -= strlen(s) + 1;
	for (char *p = e; 0 != (*p++ = *s++);) {}
	return e;
}

typedef struct put_padding_r_testcase
{
	const char *const s;
	const unsigned w;
	const char *const p;
}
put_padding_r_testcase;

static const put_padding_r_testcase g_put_padding_r_testcases[] =
{
	{"", 0, ""},
	{"1", 0, "1"},
	{"123", 0, "123"},
	{"", 3, "---"},
	{"1", 3, "--1"},
	{"123", 3, "123"},
	{"1234", 3, "1234"},
};

static void test_put_padding_r()
{
	char buf[16];
	char *const e = buf + _countof(buf) - 1;
	char *p;
	for (unsigned i = 0; _countof(g_put_padding_r_testcases) > i; ++i)
	{
		const put_padding_r_testcase *const tc = g_put_padding_r_testcases + i;
		p = strcopy_r(tc->s, e + 1);
		p = put_padding_r(tc->w, '-', p, e);
		TEST_VERIFY_TRUE_MSG(0 == strcmp(p, tc->p), "i=%u", i);
	}
}

int main(int argc, char *argv[])
{
	TEST_RUNNER_CREATE(argc, argv);

	TEST_EXECUTE(test_put_padding_r());

	return TEST_RUNNER_EXIT_CODE();
}
