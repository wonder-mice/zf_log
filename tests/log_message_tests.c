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
STATIC_ASSERT(mock_msg_fits_buff, sizeof(c_mock_msg) < ZF_LOG_BUF_SZ);

static char g_msg[ZF_LOG_BUF_SZ + 1];
static size_t g_len;
static size_t g_null_pos;

static size_t memchk(const void *const b, const int c, const size_t sz)
{
	const unsigned char v = c;
	const unsigned char *const s = (const unsigned char *)b;
	const unsigned char *const e = s + sz;
	const unsigned char *p = s;
	for (;p != e && v == *p; ++p) {}
	return (size_t)(p - s);
}

static size_t common_prefix(const char *const s1, const size_t s1_len,
							const char *const s2, const size_t s2_len)
{
	const char *const e1 = s1 + s1_len;
	const char *const e2 = s2 + s2_len;
	const char *c1 = s1, *c2 = s2;
	while (e1 != c1 && e2 != c2 && *c1 == *c2)
	{
		++c1;
		++c2;
	}
	return (size_t)(c1 - s1);
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

static void mock_buffer_callback(zf_log_output_ctx *ctx, char *buf)
{
	memset(buf, -1, ZF_LOG_BUF_SZ);
	buffer_callback(ctx, buf);
}

static void mock_output_callback(zf_log_output_ctx *ctx)
{
	g_len = (int)(ctx->p - ctx->buf);
	memcpy(g_msg, ctx->buf, ZF_LOG_BUF_SZ);
	for	(g_null_pos = 0; g_len > g_null_pos && 0 != *(g_msg + g_null_pos);
		 ++g_null_pos)
	{
	}
}

static void test_buffer_size()
{
	for (size_t buf_sz = 0; sizeof(c_mock_msg) >= buf_sz; ++buf_sz)
	{
		reset();
		const size_t modifiable = buf_sz + 1;
		const size_t unmodifiable = ZF_LOG_BUF_SZ - modifiable;
		g_buf_sz = buf_sz;
		_zf_log_write_d("function", "file", 42, ZF_LOG_INFO, ZF_LOG_TAG,
						c_mock_fmt);
		const size_t untouched = memchk(g_msg + modifiable, -1, unmodifiable);
		const size_t match = common_prefix(c_mock_msg, sizeof(c_mock_msg),
										   g_msg, g_len);
		TEST_VERIFY_EQUAL(untouched, unmodifiable);
		TEST_VERIFY_EQUAL(g_null_pos, g_len);
		TEST_VERIFY_GREATER_OR_EQUAL(match, g_len);
	}
}

int main(int argc, char *argv[])
{
	zf_log_set_tag_prefix("prefix");
	zf_log_set_output_callback(mock_output_callback);
	g_time_cb = mock_time_callback;
	g_pid_cb = mock_pid_callback;
	g_buffer_cb = mock_buffer_callback;
	TEST_RUNNER_CREATE(argc, argv);

	TEST_EXECUTE(test_buffer_size());

	return TEST_RUNNER_EXIT_CODE();
}
