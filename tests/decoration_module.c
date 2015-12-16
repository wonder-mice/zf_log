#define ZF_LOG_LEVEL ZF_LOG_INFO
#define ZF_LOG_LIBRARY_PREFIX module
#include <zf_log.c>
#include <stdlib.h>
#include <stdio.h>

static int module_called;

static void module_output_callback(zf_log_output_ctx *ctx)
{
	if (strncmp("module", ctx->msg_b, (size_t)(ctx->p - ctx->msg_b)))
	{
		fprintf(stderr, "incorrect message in module\n");
		exit(1);
	}
	++module_called;
}

void test_module()
{
	zf_log_set_output_callback(module_output_callback);
	ZF_LOGI("module");
	if (!module_called)
	{
		fprintf(stderr, "module callback was not called\n");
		exit(1);
	}
}
