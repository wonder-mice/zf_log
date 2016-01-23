#define ZF_LOG_EXTERN_TAG_PREFIX
#define ZF_LOG_EXTERN_GLOBAL_FORMAT
#define ZF_LOG_EXTERN_GLOBAL_OUTPUT
#define ZF_LOG_EXTERN_GLOBAL_OUTPUT_LEVEL
#include <zf_log.c>

ZF_LOG_DEFINE_TAG_PREFIX = "MAIN";
ZF_LOG_DEFINE_GLOBAL_FORMAT = {32};
ZF_LOG_DEFINE_GLOBAL_OUTPUT = {ZF_LOG_OUT_STDERR};
ZF_LOG_DEFINE_GLOBAL_OUTPUT_LEVEL = ZF_LOG_INFO;

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
