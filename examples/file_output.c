#include <stdio.h>
#include <stdlib.h>
#include "zf_log.h"

FILE *g_log_file;

static void file_output_callback(zf_log_output_ctx *ctx)
{
    *ctx->p = '\n';
    fwrite(ctx->buf, ctx->p - ctx->buf + 1, 1, g_log_file);
    fflush(g_log_file);
}

static void file_output_close()
{
	fclose(g_log_file);
}

static void file_output_open(const char *const log_path)
{
    g_log_file = fopen(log_path, "a");
    if (!g_log_file)
	{
		ZF_LOGW("Failed to open log file %s", log_path);
		return;
	}
	atexit(file_output_close);
	zf_log_set_output_callback(file_output_callback);
}

int main(int argc, char *argv[])
{
	file_output_open("example.log");

    ZF_LOGI("Writing number of arguments to log file: %i", argc);
	ZF_LOGI_MEM(argv, argc * sizeof(*argv), "argv pointers:");

    return 0;
}
