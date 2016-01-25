#pragma once

#define TEST_LIBRARY_ID_zf_log 1
#define TEST_LIBRARY_ID_spdlog 2
#define TEST_LIBRARY_ID_easylog 3
#define TEST_LIBRARY_ID_g3log 4
#define TEST_LIBRARY_ID_glog 5

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)

#if TEST_LIBRARY_ID_zf_log == CONCAT(TEST_LIBRARY_ID_, TEST_LIBRARY)
	#define TEST_LIBRARY_ZF_LOG
#elif TEST_LIBRARY_ID_spdlog == CONCAT(TEST_LIBRARY_ID_, TEST_LIBRARY)
	#define TEST_LIBRARY_SPDLOG
#elif TEST_LIBRARY_ID_easylog == CONCAT(TEST_LIBRARY_ID_, TEST_LIBRARY)
	#define TEST_LIBRARY_EASYLOG
#elif TEST_LIBRARY_ID_g3log == CONCAT(TEST_LIBRARY_ID_, TEST_LIBRARY)
	#define TEST_LIBRARY_G3LOG
#elif TEST_LIBRARY_ID_glog == CONCAT(TEST_LIBRARY_ID_, TEST_LIBRARY)
	#define TEST_LIBRARY_GLOG
#else
	#error Unknown test library name
#endif

#define XLOG_STRING_LITERAL "A random string"
#define XLOG_INT_LITERAL 42
/* It's important that values are not const. Otherwise compilers will be able
 * to optimize out things that we care about.
 */
extern const char *XLOG_STRING_VALUE;
extern int XLOG_INT_VALUE;
#ifndef TEST_SWITCH_MODULE
	const char *XLOG_STRING_VALUE = XLOG_STRING_LITERAL;
	int XLOG_INT_VALUE = XLOG_INT_LITERAL;
#endif
#ifdef TEST_FORMAT_SLOW_FUNC
	#include <chrono>
	#include <thread>
	static int XLOG_SLOW_FUNC()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		return (int)std::hash<std::thread::id>()(std::this_thread::get_id());
	}
#endif

#define XLOG_MESSAGE_STR_LITERAL_PRINTF XLOG_STRING_LITERAL
#define XLOG_MESSAGE_STR_LITERAL_CPPFMT XLOG_STRING_LITERAL
#define XLOG_MESSAGE_STR_LITERAL_STREAM XLOG_STRING_LITERAL

#define XLOG_MESSAGE_3INT_VALUES_PRINTF \
		"vA: %i, vB: %i, vC: %i", \
		XLOG_INT_VALUE, XLOG_INT_VALUE, XLOG_INT_VALUE
#define XLOG_MESSAGE_3INT_VALUES_CPPFMT \
		"vA: {}, vB: {}, vC: {}", \
		XLOG_INT_VALUE, XLOG_INT_VALUE, XLOG_INT_VALUE
#define XLOG_MESSAGE_3INT_VALUES_STREAM \
		"vA: " << XLOG_INT_VALUE << ", vB: " << XLOG_INT_VALUE << \
		", vC: " << XLOG_INT_VALUE

#define XLOG_MESSAGE_SLOW_FUNC_PRINTF "%i", XLOG_SLOW_FUNC()
#define XLOG_MESSAGE_SLOW_FUNC_CPPFMT "{}", XLOG_SLOW_FUNC()
#define XLOG_MESSAGE_SLOW_FUNC_STREAM XLOG_SLOW_FUNC()

#ifdef TEST_LIBRARY_ZF_LOG
	#include <zf_log.h>
	#ifdef TEST_NULL_SINK
		#define _XLOG_INIT_SINK() \
			zf_log_set_output_v(ZF_LOG_PUT_STD, 0, \
								[](const zf_log_message *, void *){})
	#else
		#define _XLOG_INIT_SINK()
	#endif
	#ifdef TEST_LOG_OFF
		#define _XLOG_INIT_LEVEL() \
			zf_log_set_output_level(ZF_LOG_ERROR)
	#else
		#define _XLOG_INIT_LEVEL()
	#endif
	static void XLOG_INIT()
	{
		_XLOG_INIT_SINK();
		_XLOG_INIT_LEVEL();
	}

	#if defined(TEST_FORMAT_INTS)
		#define XLOG_STATEMENT() ZF_LOGI(XLOG_MESSAGE_3INT_VALUES_PRINTF)
	#elif defined(TEST_FORMAT_SLOW_FUNC)
		#define XLOG_STATEMENT() ZF_LOGI(XLOG_MESSAGE_SLOW_FUNC_PRINTF)
	#else
		#define XLOG_STATEMENT() ZF_LOGI(XLOG_MESSAGE_STR_LITERAL_PRINTF)
	#endif
#endif

#ifdef TEST_LIBRARY_SPDLOG
	#include <spdlog/spdlog.h>
	extern const std::shared_ptr<spdlog::logger> g_logger;
	#ifdef TEST_NULL_SINK
		class null_sink: public spdlog::sinks::sink
		{
		public:
			void log(const spdlog::details::log_msg &) override {}
			void flush() override {}
		};
		#ifndef TEST_SWITCH_MODULE
			const std::shared_ptr<spdlog::logger> g_logger = spdlog::create<null_sink>("null");
		#endif
	#else
		#ifndef TEST_SWITCH_MODULE
			const std::shared_ptr<spdlog::logger> g_logger = spdlog::stderr_logger_st("stderr");
		#endif
	#endif
	#ifdef TEST_LOG_OFF
		#define _XLOG_INIT_LEVEL() spdlog::set_level(spdlog::level::err)
	#else
		#define _XLOG_INIT_LEVEL()
	#endif
	static void XLOG_INIT()
	{
		_XLOG_INIT_LEVEL();
	}

	#if defined(TEST_FORMAT_INTS)
		#define XLOG_STATEMENT() g_logger->info(XLOG_MESSAGE_3INT_VALUES_CPPFMT)
	#elif defined(TEST_FORMAT_SLOW_FUNC)
		#define XLOG_STATEMENT() g_logger->info(XLOG_MESSAGE_SLOW_FUNC_CPPFMT)
	#else
		#define XLOG_STATEMENT() g_logger->info(XLOG_MESSAGE_STR_LITERAL_CPPFMT)
	#endif
#endif

#ifdef TEST_LIBRARY_EASYLOG
	#ifdef TEST_NULL_SINK
		class null_stream {
		public:
			template<typename T>
			null_stream &operator<<(T) { return *this; }
			null_stream &operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
		};
		extern null_stream g_null;
		#ifndef TEST_SWITCH_MODULE
			null_stream g_null;
		#endif
		#define ELPP_CUSTOM_COUT g_null
		#define _XLOG_INIT_SINK() \
			el::Loggers::reconfigureAllLoggers(el::ConfigurationType::ToFile, "false")
	#else
		#define _XLOG_INIT_SINK()
	#endif
	#ifdef TEST_LOG_OFF
		#define _XLOG_INIT_LEVEL() \
			el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Enabled, "false")
	#else
		#define _XLOG_INIT_LEVEL()
	#endif
	#define ELPP_THREAD_SAFE
	#include <easylogging++.h>
	#ifndef TEST_SWITCH_MODULE
		INITIALIZE_EASYLOGGINGPP
	#endif
	static void XLOG_INIT()
	{
		_XLOG_INIT_SINK();
		_XLOG_INIT_LEVEL();
	}


	#if defined(TEST_FORMAT_INTS)
		#define XLOG_STATEMENT() LOG(INFO) << XLOG_MESSAGE_3INT_VALUES_STREAM
	#elif defined(TEST_FORMAT_SLOW_FUNC)
		#define XLOG_STATEMENT() LOG(INFO) << XLOG_MESSAGE_SLOW_FUNC_STREAM
	#else
		#define XLOG_STATEMENT() LOG(INFO) << XLOG_MESSAGE_STR_LITERAL_STREAM
	#endif
#endif

#ifdef TEST_LIBRARY_G3LOG
	#include <g3log/g3log.hpp>
	#include <g3log/logworker.hpp>
	#ifdef TEST_NULL_SINK
		class null_sink
		{
		public:
			void log(const std::string) {}
		};
		#define _XLOG_INIT_SINK() \
			auto worker = g3::LogWorker::createLogWorker(); \
			g3::initializeLogging(worker.get()); \
			worker->addSink(std::unique_ptr<null_sink>(new null_sink), &null_sink::log);
	#else
		#define _XLOG_INIT_SINK() \
			auto worker = g3::LogWorker::createLogWorker(); \
			g3::initializeLogging(worker.get()); \
			worker->addDefaultLogger("g3log", "g3log.log");
	#endif
	#ifdef TEST_LOG_OFF
		#ifndef G3_DYNAMIC_LOGGING
			#error g3log must be built with G3_DYNAMIC_LOGGING defined
		#endif
		#define _XLOG_INIT_LEVEL() \
			g3::only_change_at_initialization::setLogLevel(INFO, false)
	#else
		#define _XLOG_INIT_LEVEL()
	#endif
	static void XLOG_INIT()
	{
		_XLOG_INIT_SINK();
		_XLOG_INIT_LEVEL();
	}

	#if defined(TEST_FORMAT_INTS)
		#define XLOG_STATEMENT() LOGF(INFO, XLOG_MESSAGE_3INT_VALUES_PRINTF)
	#elif defined(TEST_FORMAT_SLOW_FUNC)
		#define XLOG_STATEMENT() LOGF(INFO, XLOG_MESSAGE_SLOW_FUNC_PRINTF)
	#else
		#define XLOG_STATEMENT() LOGF(INFO, XLOG_MESSAGE_STR_LITERAL_PRINTF)
	#endif
#endif

#ifdef TEST_LIBRARY_GLOG
	#include <glog/logging.h>
	#ifdef TEST_NULL_SINK
		class null_sink: public google::LogSink
		{
		public:
			void send(google::LogSeverity, const char *, const char *, int,
					  const struct ::tm *, const char *, size_t) override {}
			void WaitTillSent() override {}
		};
		extern null_sink g_sink;
		#ifndef TEST_SWITCH_MODULE
			null_sink g_sink;
		#endif
		#define _XLOG_LOG(lvl) LOG_TO_SINK_BUT_NOT_TO_LOGFILE(&g_sink, lvl)
	#else
		#define _XLOG_LOG(lvl) LOG(lvl)
	#endif
	#ifdef TEST_LOG_OFF
		#define _XLOG_INIT_LEVEL() FLAGS_minloglevel = google::ERROR
	#else
		#define _XLOG_INIT_LEVEL()
	#endif
	static void XLOG_INIT()
	{
		google::InitGoogleLogging("glog");
		_XLOG_INIT_LEVEL();
	}

	#if defined(TEST_FORMAT_INTS)
		#define XLOG_STATEMENT() _XLOG_LOG(INFO) << XLOG_MESSAGE_3INT_VALUES_STREAM
	#elif defined(TEST_FORMAT_SLOW_FUNC)
		#define XLOG_STATEMENT() _XLOG_LOG(INFO) << XLOG_MESSAGE_SLOW_FUNC_STREAM
	#else
		#define XLOG_STATEMENT() _XLOG_LOG(INFO) << XLOG_MESSAGE_STR_LITERAL_STREAM
	#endif
#endif
