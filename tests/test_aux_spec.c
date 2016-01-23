#include <zf_log.c>
#include <zf_test.h>

static const zf_log_spec g_spec =
{
	ZF_LOG_GLOBAL_FORMAT,
	ZF_LOG_GLOBAL_OUTPUT
};

static void mock_output_callback(const zf_log_message *msg, void *arg)
{
	(void)msg; (void)arg;
}

int main(int argc, char *argv[])
{
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, mock_output_callback);
	ZF_LOGI_AUX(&g_spec, "aux log, argc=%i", argc);
	ZF_LOGI_MEM_AUX(&g_spec, argv, argc * sizeof(*argv), "aux log, argv pointers:");
	return 0;
}
