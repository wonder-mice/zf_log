#include <stddef.h>
#include <string.h>
#include <zf_test.h>
#define ZF_LOG_ANDROID_LOG 0
#define ZF_LOG_PUT_CTX 1
#define ZF_LOG_BUF_SZ 128
#define ZF_LOG_MEM_WIDTH 16
#define ZF_LOG_INSTRUMENTED 1
#define ZF_LOG_LEVEL ZF_LOG_INFO
#define ZF_LOG_TAG "TAG"
#include <zf_log.c>

static const char c_mock_fmt[] =
	"Lorem ipsum dolor sit amet.";
static const char c_mock_msg[] =
	"12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
	"Lorem ipsum dolor sit amet.";

static char g_msg[ZF_LOG_BUF_SZ + 1];
static ptrdiff_t g_len;

static unsigned common_prefix(const char *const s1, const unsigned s1_len,
							  const char *const s2, const unsigned s2_len)
{
	const char *const e1 = s1 + s1_len;
	const char *const e2 = s2 + s2_len;
	const char *c1 = s1, *c2 = s2;
	while (e1 != c1 && e2 != c2 && *c1 == *c2)
	{
		++c1;
		++c2;
	}
	return (unsigned)(c1 - s1);
}

static void reset()
{
	g_buf_sz = ZF_LOG_BUF_SZ;
	memset(g_msg, -1, sizeof(g_msg));
	g_len = -1;
}

static void mock_time_callback(struct tm *const tm, unsigned *const usec)
{
	tm->tm_sec = 56;
	tm->tm_min = 34;
	tm->tm_hour = 12;
	tm->tm_mday = 23;
	tm->tm_mon = 12;
	tm->tm_year = 0;
	tm->tm_wday = 0;
	tm->tm_yday = 0;
	tm->tm_isdst = 0;
	tm->tm_gmtoff = 0;
	tm->tm_zone = 0;
	*usec = 789000;
}

static void mock_pid_callback(int *const pid, int *const tid)
{
	*pid = 9876;
	*tid = 5432;
}

static void mock_output_callback(zf_log_output_ctx *ctx)
{
	g_len = (int)(ctx->p - ctx->buf);
	memcpy(g_msg, ctx->buf, ZF_LOG_BUF_SZ);
}

static void test_buffer_size()
{
	for (unsigned buf_sz = 0;
		 ZF_LOG_BUF_SZ >= buf_sz && sizeof(c_mock_msg) >= buf_sz; ++buf_sz)
	{
		reset();
		g_buf_sz = buf_sz;
		_zf_log_write_d("function", "file", 42,
						ZF_LOG_INFO, ZF_LOG_TAG, c_mock_fmt);
		const unsigned match = common_prefix(c_mock_msg, sizeof(c_mock_msg),
											 g_msg, g_len);
		// *nprintf() family always puts 0 in the end, while put_tag() don't.
		// when put_tag() goes after any *nprintf() it will overwrite 0 in the
		// end with the tag or tag prefix first character. +1 here is to
		// workaround that inconsistency.
		TEST_VERIFY_GREATER_OR_EQUAL(match + 1, g_len);
	}
}

int main(int argc, char *argv[])
{
	zf_log_set_tag_prefix("prefix");
	zf_log_set_output_callback(mock_output_callback);
	g_time_cb = mock_time_callback;
	g_pid_cb = mock_pid_callback;
	TEST_RUNNER_CREATE(argc, argv);
	TEST_EXECUTE(test_buffer_size());

	return TEST_RUNNER_EXIT_CODE();
}
