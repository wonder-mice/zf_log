#include "library_switch.h"

#ifdef TEST_LIBRARY_ZF_LOG
	#include <zf_log.c>
	#define LOG_STATEMENT() ZF_LOGI("A log message")
#endif
#ifdef TEST_LIBRARY_SPDLOG
	#include <spdlog/spdlog.h>
	auto logger = spdlog::stderr_logger_st("stderr");
	#define LOG_STATEMENT() logger->info("A log message")
#endif

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	LOG_STATEMENT();
#ifdef EXTRA_STATEMENT
	LOG_STATEMENT();
#endif
	return 0;
}
