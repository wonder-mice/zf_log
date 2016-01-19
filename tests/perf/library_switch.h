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
