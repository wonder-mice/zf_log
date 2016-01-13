#define ZF_LOG_ANDROID_LOG 0
#define ZF_LOG_PUT_CTX_DEPRECATED 1
#define ZF_LOG_BUF_SZ 128
#define ZF_LOG_MEM_WIDTH 16
#define ZF_LOG_INSTRUMENTED 1
#define ZF_LOG_LEVEL ZF_LOG_INFO
#define ZF_LOG_TAG "TAG"
#include <zf_log.c>
#include <stddef.h>
#include <string.h>

#define ASSERT_FITS(s) \
	((sizeof((s)) < ZF_LOG_BUF_SZ)? (s): ((char **)0)[0])

static const char c_test_fmt[] =
	"Lorem ipsum dolor sit amet.";
static const char c_test_mem[] =
	"Here's to the crazy ones.";

static const char *const c_msg_expected_lines[] = {
	ASSERT_FITS("12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
				"Lorem ipsum dolor sit amet.")
};
static const char *const c_mem_expected_lines[] = {
	ASSERT_FITS("12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
				"Lorem ipsum dolor sit amet."),
	ASSERT_FITS("12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
				"48657265277320746f20746865206372  Here's to the cr"),
	ASSERT_FITS("12-23 12:34:56.789  9876  5432 I prefix.TAG function@file:42 "
				"617a79206f6e65732e00              azy ones.?")
};

#define MAX_LINES 4
static char g_lines[MAX_LINES][ZF_LOG_BUF_SZ];
static size_t g_len[MAX_LINES];
static size_t g_null_pos[MAX_LINES];
static size_t g_line;

static size_t memchk(const void *const b, const int c, const size_t sz)
{
	const unsigned char v = (unsigned char)c;
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
	for (;e1 != c1 && e2 != c2 && *c1 == *c2; ++c1, ++c2) {}
	return (size_t)(c1 - s1);
}

static void reset()
{
	g_buf_sz = ZF_LOG_BUF_SZ;
	for (size_t i = 0; MAX_LINES > i; ++i)
	{
		memset(g_lines[i], -1, ZF_LOG_BUF_SZ);
		g_len[i] = 0;
		g_null_pos[i] = 0;
	}
	g_line = 0;
}

static void mock_time_callback(struct tm *const tm, unsigned *const msec)
{
	memset(tm, 0, sizeof(*tm));
	tm->tm_sec = 56;
	tm->tm_min = 34;
	tm->tm_hour = 12;
	tm->tm_mday = 23;
	tm->tm_mon = 11;
	*msec = 789;
}

static void mock_pid_callback(int *const pid, int *const tid)
{
	*pid = 9876;
	*tid = 5432;
}

static void mock_buffer_callback(zf_log_message *msg, char *buf)
{
	memset(buf, -1, ZF_LOG_BUF_SZ);
	buffer_callback(msg, buf);
}

static void mock_output_callback(const zf_log_message *msg)
{
	const size_t i = g_line++;
	if (MAX_LINES <= i)
	{
		fprintf(stderr, "too many lines produced\n");
		exit(1);
	}
	char *const line = g_lines[i];
	memcpy(line, msg->buf, ZF_LOG_BUF_SZ);
	const size_t len = (size_t)(msg->p - msg->buf);
	size_t null_pos;
	for	(null_pos = 0; len > null_pos && 0 != line[null_pos]; ++null_pos) {}
	g_len[i] = len;
	g_null_pos[i] = null_pos;
}

static void verify_log_output(const size_t buf_sz,
							  const char *const expected[],
							  const size_t expected_n)
{
	const size_t modifiable = buf_sz + 1;
	const size_t unmodifiable = ZF_LOG_BUF_SZ - modifiable;
	if (g_line > expected_n)
	{
		fprintf(stderr, "Lines produced: actual=%u, expected=<%u\n",
				(unsigned)g_line, (unsigned)expected_n);
		exit(1);
	}
	size_t complete_lines = 0;
	for (size_t i = 0; g_line > i; ++i)
	{
		const char *const line = g_lines[i];
		const size_t line_len = strlen(expected[i]);
		const size_t untouched = memchk(line + modifiable, -1, unmodifiable);
		const size_t match = common_prefix(expected[i], line_len,
										   line, g_len[i]);
		if (untouched != unmodifiable)
		{
			fprintf(stderr, "Untouched bytes: actual=%u, expected=%u\n",
					(unsigned)untouched, (unsigned)unmodifiable);
			exit(1);
		}
		if (g_null_pos[i] != g_len[i])
		{
			fprintf(stderr, "Null position: actual=%u, expected=%u\n",
					(unsigned)g_null_pos[i], (unsigned)g_len[i]);
			exit(1);
		}
		if (match < g_len[i])
		{
			fprintf(stderr, "Line partial match: actual=%u, expected=>%u\n",
					(unsigned)match, (unsigned)g_len[i]);
			exit(1);
		}
		if (line_len <= buf_sz)
		{
			++complete_lines;
			if (line_len <= buf_sz && match != g_len[i])
			{
				fprintf(stderr, "Line complete match: actual=%u, expected=%u\n",
						(unsigned)match, (unsigned)g_len[i]);
				exit(1);
			}
		}
	}
	if (expected_n == complete_lines && g_line != expected_n)
	{
		fprintf(stderr, "Complete lines produced: actual=%u, expected=<%u\n",
				(unsigned)g_line, (unsigned)expected_n);
		exit(1);
	}
}

static void test_msg_output()
{
	for (unsigned buf_sz = 0; ZF_LOG_BUF_SZ - ZF_LOG_EOL_SZ >= buf_sz; ++buf_sz)
	{
		reset();
		g_buf_sz = buf_sz;
		_zf_log_write_d("function", "file", 42, ZF_LOG_INFO, ZF_LOG_TAG,
						c_test_fmt);
		verify_log_output(buf_sz,
						  c_msg_expected_lines, _countof(c_msg_expected_lines));
	}
}

static void test_mem_output()
{
	for (unsigned buf_sz = 0; ZF_LOG_BUF_SZ - ZF_LOG_EOL_SZ >= buf_sz; ++buf_sz)
	{
		reset();
		g_buf_sz = buf_sz;
		_zf_log_write_mem_d("function", "file", 42, ZF_LOG_INFO, ZF_LOG_TAG,
							c_test_mem, sizeof(c_test_mem),
							c_test_fmt);
		verify_log_output(buf_sz,
						  c_mem_expected_lines, _countof(c_mem_expected_lines));
	}
}

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	g_time_cb = mock_time_callback;
	g_pid_cb = mock_pid_callback;
	g_buffer_cb = mock_buffer_callback;
	zf_log_set_output_v(ZF_LOG_PUT_STD, mock_output_callback, 0);
	zf_log_set_tag_prefix("prefix");

	test_msg_output();
	test_mem_output();

	return 0;
}
