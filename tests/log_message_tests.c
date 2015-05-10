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

#ifndef _countof
	#define _countof(a) (sizeof(a) / sizeof(*a))
#endif

static const char c_mock_fmt[] =
	"Lorem ipsum dolor sit amet.";
static const char c_mock_mem[] =
	"Here's to the crazy ones.";
static const char c_mock_msg[] =
	"12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
	"Lorem ipsum dolor sit amet.";
static const char *const c_mock_mem_msg[] = {
	"12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
	"Lorem ipsum dolor sit amet.",
	"12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
	"48657265277320746f20746865206372  Here's to the cr",
	"12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
	"617a79206f6e65732e00              azy ones.?"
};
STATIC_ASSERT(mock_msg_fits_buff, sizeof(c_mock_msg) < ZF_LOG_BUF_SZ);

#define MAX_LINES 4
static char g_lines[MAX_LINES][ZF_LOG_BUF_SZ];
static size_t g_len[MAX_LINES];
static size_t g_null_pos[MAX_LINES];
static size_t g_line;

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
	for (size_t i = MAX_LINES; 0 < i--;)
	{
		memset(g_lines[i], -1, ZF_LOG_BUF_SZ);
		g_len[i] = 0;
		g_null_pos[i] = 0;
	}
	g_line = 0;
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
	const int i = g_line++;
	char *const line = g_lines[i];
	const size_t len = (size_t)(ctx->p - ctx->buf);
	memcpy(line, ctx->buf, ZF_LOG_BUF_SZ);
	size_t null_pos;
	for	(null_pos = 0; len > null_pos && 0 != line[null_pos]; ++null_pos) {}
	g_len[i] = len;
	g_null_pos[i] = null_pos;

	//line[len] = 0;
	//fprintf(stderr, "msg=\"%s\"\n", line);
}

static void verify_log_output(const size_t buf_sz,
							  const char *const expected[],
							  const size_t expected_n)
{
	g_buf_sz = buf_sz;
	const size_t modifiable = buf_sz + 1;
	const size_t unmodifiable = ZF_LOG_BUF_SZ - modifiable;
	_zf_log_write_mem_d("function", "file", 42, ZF_LOG_INFO, ZF_LOG_TAG,
						c_mock_mem, sizeof(c_mock_mem),
						c_mock_fmt);
	for (size_t i = 0; _countof(c_mock_mem_msg) > i; ++i)
	{
		const char *const line = g_lines[i];
		const size_t untouched = memchk(line + modifiable, -1, unmodifiable);
		const size_t match = common_prefix(c_mock_mem_msg[i], strlen(c_mock_mem_msg[i]),
										   line, g_len[i]);
		fprintf(stderr, "i=%i, match=%i, g_len[i]=%i\n",
				(int)i, (int)match, (int)g_len[i]);
		TEST_VERIFY_EQUAL(untouched, unmodifiable);
		TEST_VERIFY_EQUAL(g_null_pos[i], g_len[i]);
		TEST_VERIFY_GREATER_OR_EQUAL(match, g_len[i]);
	}
}

static void test_buffer_size()
{
	for (size_t buf_sz = 0; sizeof(c_mock_msg) >= buf_sz; ++buf_sz)
	{
		reset();
		g_buf_sz = buf_sz;
		const size_t modifiable = buf_sz + 1;
		const size_t unmodifiable = ZF_LOG_BUF_SZ - modifiable;
		_zf_log_write_d("function", "file", 42, ZF_LOG_INFO, ZF_LOG_TAG,
						c_mock_fmt);
		const size_t untouched = memchk(g_msg[0] + modifiable, -1, unmodifiable);
		const size_t match = common_prefix(c_mock_msg, sizeof(c_mock_msg),
										   g_msg[0], g_len[0]);
		TEST_VERIFY_EQUAL(untouched, unmodifiable);
		TEST_VERIFY_EQUAL(g_null_pos[0], g_len[0]);
		TEST_VERIFY_GREATER_OR_EQUAL(match, g_len[0]);
	}
}

static void test_buffer_overflow_mem()
{
	for (size_t buf_sz = 0; ZF_LOG_BUF_SZ - ZF_LOG_EOL_SZ >= buf_sz; ++buf_sz)
	{
		reset();
		verify_log_output(buf_sz, c_mock_mem_msg, _countof(c_mock_mem_msg));
		g_buf_sz = buf_sz;
		const size_t modifiable = buf_sz + 1;
		const size_t unmodifiable = ZF_LOG_BUF_SZ - modifiable;
		_zf_log_write_mem_d("function", "file", 42, ZF_LOG_INFO, ZF_LOG_TAG,
							c_mock_mem, sizeof(c_mock_mem),
							c_mock_fmt);
		for (size_t i = 0; _countof(c_mock_mem_msg) > i; ++i)
		{
			const char *const line = g_msg[i];
			const size_t untouched = memchk(line + modifiable, -1, unmodifiable);
			const size_t match = common_prefix(c_mock_mem_msg[i], strlen(c_mock_mem_msg[i]),
											   line, g_len[i]);
			fprintf(stderr, "i=%i, match=%i, g_len[i]=%i\n",
					(int)i, (int)match, (int)g_len[i]);
			TEST_VERIFY_EQUAL(untouched, unmodifiable);
			TEST_VERIFY_EQUAL(g_null_pos[i], g_len[i]);
			TEST_VERIFY_GREATER_OR_EQUAL(match, g_len[i]);
		}
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
	TEST_EXECUTE(test_buffer_overflow_mem());

	return TEST_RUNNER_EXIT_CODE();
}
