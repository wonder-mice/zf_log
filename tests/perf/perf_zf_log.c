#include <zf_log.c>

static void null_output_callback(const zf_log_message *msg, void *arg)
{
	(void)msg; (void)arg;
}

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	zf_log_set_output_v(ZF_LOG_PUT_STD, null_output_callback, 0);

	int howmany = 10000000;
    for(int i  = 0 ; i < howmany; ++i)
	{
		ZF_LOGI("zf_log message #%i: This is some text for your pleasure", i);
	}
	return 0;
}
