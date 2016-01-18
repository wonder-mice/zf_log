#pragma once

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
