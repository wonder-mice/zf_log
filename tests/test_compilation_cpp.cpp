#include <zf_log.c>

namespace
{
	void mock_output_callback(const zf_log_message *, void *)
	{
	}
}

int main(int argc, char *argv[])
{
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, mock_output_callback);
	ZF_LOGI("log from cpp, argc=%i", argc);
	ZF_LOGI_MEM(argv, argc * sizeof(*argv), "log from cpp, argv pointers:");
	return 0;
}
