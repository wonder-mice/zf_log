/*
 * Example demonstrating stripping out log messages which would otherwise
 * be unused.
 *
 * To test:
 * 1. Set the log level as desired
 * 2. Compile and run the application
 * 3. Observe application output for logs below the desired log level
 * 4. Use the `strings' application to search for the string at the desired log
 *  level
 * 4.1. example @ INFO log level:
 *     $ strings strip_log_messages | egrep "[a-z] dump string"
 *     i dump string
 *     w dump string
 *     e dump string
 *     f dump string
 *     $
 *
 * 5. Use the `strings' application to search for the string below the desired
 *   log level
 * 5.1. example @ INFO log level
 *     $ strings strip_log_messages | egrep "[a-z] dump string"
 *     i dump string
 *     w dump string
 *     e dump string
 *     f dump string
 *     $
 *
 * 6. Confirm that steps 4 & 5 also work for ZF_LOG*_MEM using the same
 * technique
 * 6.1. example @ INFO log level
 *     $ strings strip_log_messages | egrep "[a-z]m %s"
 *     im %s
 *     wm %s
 *     em %s
 *     fm %s
 *     $
 *
 */
#include <stdio.h>
#include <string.h>

#define ZF_LOG_LEVEL ZF_LOG_INFO
#include "zf_log.h"

int main()
{
    char* dumpable_string = "dump string\0";

    ZF_LOGV("v dump string");
    ZF_LOGD("d dump string");
    ZF_LOGI("i dump string");
    ZF_LOGW("w dump string");
    ZF_LOGE("e dump string");
    ZF_LOGF("f dump string");

    ZF_LOGV_MEM(dumpable_string, strlen(dumpable_string), "vm %s",
                dumpable_string);
    ZF_LOGD_MEM(dumpable_string, strlen(dumpable_string), "dm %s",
                dumpable_string);
    ZF_LOGI_MEM(dumpable_string, strlen(dumpable_string), "im %s",
                dumpable_string);
    ZF_LOGW_MEM(dumpable_string, strlen(dumpable_string), "wm %s",
                dumpable_string);
    ZF_LOGE_MEM(dumpable_string, strlen(dumpable_string), "em %s",
                dumpable_string);
    ZF_LOGF_MEM(dumpable_string, strlen(dumpable_string), "fm %s",
                dumpable_string);
}
