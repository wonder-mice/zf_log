/* synchronous file output sink
 *
 * (c) fenugrec 2017
 *
 * This is designed for single-process, multi-thread logging.
 * Multiple processes won't work due to the type of mutex used.
 *
 * OS implementations:
 * 	- Windows (winXP and up)
 *	- POSIX (unix / linux / etc)
 *
 * Log file is created if inexistant, otherwise appended to.
 * Currently works with a single, global output file, just
 * as zf_log has only one single global logging stream.
 */


#if defined(_WIN32)
	/* covers win32 / win64 targets */
	#include <windows.h>

#elif defined(__unix__)
	/* covers lots of stuff including linux, but not macs? */
	#include <stdio.h>
	#include <unistd.h>
	#include <pthread.h>

#else
	#error No sync file output implementation for your OS !
#endif

#include "zf_log.h"

/* private stuff, per-output file */
struct sf_out {
#if defined(_WIN32)
	HANDLE hfil;
	CRITICAL_SECTION mtx;	//lightweight mutex
#elif defined(__unix__)
	FILE *hfil;
	pthread_mutex_t mtx;
#endif
};

static struct sf_out global_sf;

static void sf_out_cb(const zf_log_message *msg, void *arg)
{
	struct sf_out *sf = arg;

	*msg->p = '\n';

#if defined(_WIN32)
	DWORD towrite, writ;

	towrite =  msg->p - msg->buf + 1;
	EnterCriticalSection(&sf->mtx);
	if (!WriteFile(sf->hfil, msg->buf, towrite, &writ, NULL)) {
		/* write error : silently ignore, where else would we report this ? */
		LeaveCriticalSection(&sf->mtx);
		return;
	}
	LeaveCriticalSection(&sf->mtx);

#elif defined(__unix__)
	pthread_mutex_lock(&sf->mtx);
	fwrite(msg->buf, 1, (msg->p - msg->buf + 1), sf->hfil);	// no point in checking for errors
	pthread_mutex_unlock(&sf->mtx);
#endif

	return;
}

/* Assume that caller has finished generating log outputs
 * when calling this
 */
void sf_out_close(void) {
	struct sf_out *sf = &global_sf;

#ifdef _WIN32
	DeleteCriticalSection(&sf->mtx);
	CloseHandle(sf->hfil);
#elif defined(__unix__)
	pthread_mutex_destroy(&sf->mtx);
	fclose(sf->hfil);
	sf->hfil = NULL;
#endif
	return;

}

int sf_out_open(const char *const fname) {
	struct sf_out *sf = &global_sf;

#ifdef _WIN32
	sf->hfil = CreateFile(fname, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (sf->hfil == INVALID_HANDLE_VALUE) {
		ZF_LOGW("Failed to create/open log file %s", fname);
		return -1;
	}
	InitializeCriticalSection(&sf->mtx);
#elif defined(__unix__)
	if (pthread_mutex_init(&sf->mtx, NULL)) {
		ZF_LOGW("pthread_mutex_init failed");
		return -1;
	}

	sf->hfil = fopen(fname, "ab");
	if (sf->hfil == NULL) {
		ZF_LOGW("Failed to create/open log file %s", fname);
		pthread_mutex_destroy(&sf->mtx);
		return -1;
	}
#endif

	zf_log_set_output_v(ZF_LOG_PUT_STD, sf, sf_out_cb);
	return 0;
}
