#define ZF_LOG_LEVEL ZF_LOG_INFO
#define ZF_LOG_LIBRARY_PREFIX module
#include <zf_log.c>
#include <stdlib.h>
#include <stdio.h>

static int module_called;

static void module_output_callback(zf_log_message *msg)
{
	if (strncmp("module", msg->msg_b, (size_t)(msg->p - msg->msg_b)))
	{
		fprintf(stderr, "incorrect message in module\n");
		exit(1);
	}
	++module_called;
}

void test_module()
{
	zf_log_set_output_callback(ZF_LOG_PUT_STD, module_output_callback);
	ZF_LOGI("module");
	if (!module_called)
	{
		fprintf(stderr, "module callback was not called\n");
		exit(1);
	}
}
