#if defined(_WIN32) || defined(_WIN64)
	#define ZF_LOG_USE_DEBUGSTRING
#endif
#if defined(__APPLE__) && defined(__MACH__)
	#define ZF_LOG_USE_NSLOG
#endif
#if defined(__ANDROID__)
	#define ZF_LOG_USE_ANDROID_LOG
#endif
#include <zf_log.c>

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	/* Testing compilation only for now. */
	return 0;
}
