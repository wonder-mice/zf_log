#define TEST_LIBRARY_ID_zf_log 1
#define TEST_LIBRARY_ID_spdlog 2

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#if TEST_LIBRARY_ID_zf_log == CONCAT(TEST_LIBRARY_ID_, TEST_LIBRARY)
	#define TEST_LIBRARY_ZF_LOG
#elif TEST_LIBRARY_ID_spdlog == CONCAT(TEST_LIBRARY_ID_, TEST_LIBRARY)
	#define TEST_LIBRARY_SPDLOG
#else
	#error Unknown test library name
#endif

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
