#include <zf_log.c>

namespace
{
	void mock_output_callback(zf_log_message *)
	{
	}
}

int main(int argc, char *argv[])
{
	zf_log_set_output_callback(ZF_LOG_PUT_STD, mock_output_callback);
	ZF_LOGI("log from cpp, argc=%i", argc);
	ZF_LOGI_MEM(argv, argc * sizeof(*argv), "log from cpp, argv pointers:");
	return 0;
}
