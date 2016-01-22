#include <time.h>
#include "test_switch.h"

#define A_MESSAGE "A log message"
int an_int;
int a_func() { return (int)time(nullptr); }

#ifdef TEST_LIBRARY_ZF_LOG
	#include <zf_log.h>
	#if defined(TEST_FORMAT_INT)
		#define ONE_STATEMENT() ZF_LOGI("vA: %i, vB: %i, vC: %i, vD: %i", \
				an_int, an_int, an_int, an_int)
	#elif defined(TEST_FORMAT_FUNC)
		#define ONE_STATEMENT() ZF_LOGI("vA: %i, vB: %i, vC: %i, vD: %i", \
				a_func(), a_func(), a_func(), a_func())
	#else
		#define ONE_STATEMENT() ZF_LOGI(A_MESSAGE)
	#endif
#endif
#ifdef TEST_LIBRARY_SPDLOG
	#include <spdlog/spdlog.h>
	auto logger = spdlog::stderr_logger_st("stderr");
	#if defined(TEST_FORMAT_INT)
		#define ONE_STATEMENT() logger->info("vA: %i, vB: %i, vC: %i, vD: %i", \
				an_int, an_int, an_int, an_int)
	#elif defined(TEST_FORMAT_FUNC)
		#define ONE_STATEMENT() logger->info("vA: %i, vB: %i, vC: %i, vD: %i", \
				a_func(), a_func(), a_func(), a_func())
	#else
		#define ONE_STATEMENT() logger->info(A_MESSAGE)
	#endif
#endif
#ifdef TEST_LIBRARY_EASYLOG
	#include <easylogging++.h>
	INITIALIZE_EASYLOGGINGPP
	#if defined(TEST_FORMAT_INT)
		#define ONE_STATEMENT() LOG(INFO) << "vA: " << an_int << \
				", vB: " << an_int << ", vC: " << an_int << ", vD: " << an_int
	#elif defined(TEST_FORMAT_FUNC)
		#define ONE_STATEMENT() LOG(INFO) << "vA: " << a_func() << \
				", vB: " << a_func() << ", vC: " << a_func() << ", vD: " << a_func()
	#else
		#define ONE_STATEMENT() LOG(INFO) << A_MESSAGE
	#endif
#endif
#ifdef TEST_LIBRARY_G3LOG
	#include <g3log/g3log.hpp>
	#include <g3log/logworker.hpp>
	auto worker = g3::LogWorker::createLogWorker();
	auto handle= worker->addDefaultLogger("test_executable_size.g3log",
										  "test_executable_size.g3log.log");
	auto dummy = []() {
		g3::initializeLogging(worker.get()); return 0;
	}();
	#if defined(TEST_FORMAT_INT)
		#define ONE_STATEMENT() LOGF(INFO, "vA: %i, vB: %i, vC: %i, vD: %i", \
				an_int, an_int, an_int, an_int)
	#elif defined(TEST_FORMAT_FUNC)
		#define ONE_STATEMENT() LOGF(INFO, "vA: %i, vB: %i, vC: %i, vD: %i", \
				a_func(), a_func(), a_func(), a_func())
	#else
		#define ONE_STATEMENT() LOGF(INFO, A_MESSAGE)
	#endif
#endif
#ifdef TEST_LIBRARY_GLOG
	#include <glog/logging.h>
	auto dummy = []() {
		google::InitGoogleLogging("test_executable_size.glog"); return 0;
	}();
	#if defined(TEST_FORMAT_INT)
		#define ONE_STATEMENT() LOG(INFO) << "vA: " << an_int << \
				", vB: " << an_int << ", vC: " << an_int << ", vD: " << an_int
	#elif defined(TEST_FORMAT_FUNC)
		#define ONE_STATEMENT() LOG(INFO) << "vA: " << a_func() << \
				", vB: " << a_func() << ", vC: " << a_func() << ", vD: " << a_func()
	#else
		#define ONE_STATEMENT() LOG(INFO) << A_MESSAGE
	#endif
#endif

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	ONE_STATEMENT();
#ifdef TEST_EXTRA_STATEMENT
	ONE_STATEMENT();
#endif
	return 0;
}
