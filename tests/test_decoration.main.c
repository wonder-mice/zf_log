#define ZF_LOG_LEVEL ZF_LOG_INFO
#include <zf_log.c>
#include <stdlib.h>
#include <stdio.h>

static int main_called;

static void main_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	if (strncmp("main", msg->msg_b, (size_t)(msg->p - msg->msg_b)))
	{
		fprintf(stderr, "incorrect message in main\n");
		exit(1);
	}
	++main_called;
}

void test_module();

void test_main()
{
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, main_output_callback);
	ZF_LOGI("main");
	if (!main_called)
	{
		fprintf(stderr, "main callback was not called\n");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, main_output_callback);

	test_module();
	test_main();

	return 0;
}
