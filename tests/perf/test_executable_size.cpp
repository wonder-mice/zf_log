#include "test_switch.h"

#define A_MESSAGE "A log message"

#ifdef TEST_LIBRARY_ZF_LOG
	#include <zf_log.h>
	#define ONE_STATEMENT() ZF_LOGI(A_MESSAGE)
#endif
#ifdef TEST_LIBRARY_SPDLOG
	#include <spdlog/spdlog.h>
	auto logger = spdlog::stderr_logger_st("stderr");
	#define ONE_STATEMENT() logger->info(A_MESSAGE)
#endif
#ifdef TEST_LIBRARY_EASYLOG
	#include <easylogging++.h>
	INITIALIZE_EASYLOGGINGPP
	#define ONE_STATEMENT() LOG(INFO) << A_MESSAGE
#endif
#ifdef TEST_LIBRARY_G3LOG
	#include <g3log/g3log.hpp>
	#include <g3log/logworker.hpp>
	auto worker = g3::LogWorker::createLogWorker();
	auto handle= worker->addDefaultLogger("test_call_site_size.g3log",
										  "test_call_site_size.g3log.log");
	auto dummy = []() { g3::initializeLogging(worker.get()); return 0; }();
	#define ONE_STATEMENT() LOGF(INFO, A_MESSAGE)
#endif
#ifdef TEST_LIBRARY_GLOG
	#include <glog/logging.h>
	auto dummy = []() { google::InitGoogleLogging("test_call_site_size.glog"); return 0; }();
	#define ONE_STATEMENT() LOG(INFO) << A_MESSAGE
#endif

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	ONE_STATEMENT();
#ifdef EXTRA_STATEMENT
	ONE_STATEMENT();
#endif
	return 0;
}
