#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(_ZF_TEST_STRINGIFY) && !defined(_ZF_TEST__STRINGIFY)
#define _ZF_TEST__STRINGIFY(x) #x
#define _ZF_TEST_STRINGIFY(x) _ZF_TEST__STRINGIFY(x)
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
	#define _ZF_TEST_INLINE __inline
#else
	#define _ZF_TEST_INLINE inline
#endif

/* Workaround for MSVC warning 4127 about constant expression in if condition.
 */
static _ZF_TEST_INLINE bool _zf_test_bool(const bool v)
{
	return v;
}

#define TEST_RUNNER_CREATE(argc, argv) \
	(void)argc; (void)argv;

#define TEST_SUIT_ARGUMENTS

#define TEST_VERIFY_TRUE(a) \
	if (!_zf_test_bool(a)) { \
		fprintf(stderr, "%s:%u: %s:\n", __FILE__, __LINE__, "not true"); \
		fprintf(stderr, "    false: %s\n", _ZF_TEST_STRINGIFY(a)); \
		exit(1); \
	}

#define TEST_VERIFY_TRUE_MSG(a, ...) \
	if (!_zf_test_bool(a)) { \
		fprintf(stderr, "%s:%u: %s:\n", __FILE__, __LINE__, "not true"); \
		fprintf(stderr, "    false: %s\n", _ZF_TEST_STRINGIFY(a)); \
		fprintf(stderr, "    about: "); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(1); \
	}

#define TEST_VERIFY_FALSE(a) \
	if (_zf_test_bool(a)) { \
		fprintf(stderr, "%s:%u: %s:\n", __FILE__, __LINE__, "not false"); \
		fprintf(stderr, "    true: %s\n", _ZF_TEST_STRINGIFY(a)); \
		exit(1); \
	}

#define TEST_VERIFY_EQUAL(a, b) \
	if (!_zf_test_bool((a) == (b))) { \
		fprintf(stderr, "%s:%u: %s:\n", __FILE__, __LINE__, "not equal"); \
		fprintf(stderr, "    left:   %s\n", _ZF_TEST_STRINGIFY(a)); \
		fprintf(stderr, "    reight: %s\n", _ZF_TEST_STRINGIFY(b)); \
		exit(1); \
	}

#define TEST_VERIFY_NOT_EQUAL(a, b) \
	if (!_zf_test_bool((a) != (b))) { \
		fprintf(stderr, "%s:%u: %s:\n", __FILE__, __LINE__, "equal"); \
		fprintf(stderr, "    left:   %s\n", _ZF_TEST_STRINGIFY(a)); \
		fprintf(stderr, "    reight: %s\n", _ZF_TEST_STRINGIFY(b)); \
		exit(1); \
	}

#define TEST_VERIFY_GREATER_OR_EQUAL(a, b) \
	if (!_zf_test_bool((a) >= (b))) { \
		fprintf(stderr, "%s:%u: %s:\n", __FILE__, __LINE__, "less"); \
		fprintf(stderr, "    left:   %s\n", _ZF_TEST_STRINGIFY(a)); \
		fprintf(stderr, "    reight: %s\n", _ZF_TEST_STRINGIFY(b)); \
		exit(1); \
	}

#define TEST_EXECUTE(f) \
	f

#define TEST_EXECUTE_SUITE(s) \
	s()

#define TEST_RUNNER_EXIT_CODE() \
	0
