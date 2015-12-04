#include <stdio.h>
#include <string.h>
#include "zf_log.h"

#define LOG_FILENAME "test.log"

FILE *log_file;

static void log_file_write(zf_log_output_ctx *ctx) {
    *ctx->p = '\n';

    fwrite(ctx->buf, 1, (ctx->p + 1) - ctx->buf, log_file);

    fflush(log_file);
}

int main(int argc, char *argv[])
{
    log_file = fopen(LOG_FILENAME, "a+");
    if (NULL == log_file) {
        printf("Failed to open log file.\n");
    }

    zf_log_set_output_callback(log_file_write);

    ZF_LOGI("You will see the number of arguments: %i", argc);
    ZF_LOGI_MEM(argv, argc * sizeof(*argv), "argv pointers:");

    fclose(log_file);

    return 0;
}
