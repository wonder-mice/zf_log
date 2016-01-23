#if defined(_WIN32) || defined(_WIN64)
	#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include "zf_log.h"

FILE *g_log_file;

static void file_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	*msg->p = '\n';
	fwrite(msg->buf, msg->p - msg->buf + 1, 1, g_log_file);
	fflush(g_log_file);
}

static void file_output_close(void)
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
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, file_output_callback);
}

int main(int argc, char *argv[])
{
	file_output_open("example.log");

	ZF_LOGI("Writing number of arguments to log file: %i", argc);
	ZF_LOGI_MEM(argv, argc * sizeof(*argv), "argv pointers:");

	return 0;
}
