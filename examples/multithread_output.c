/*
 * fenugrec 2018
 * Test for multiple threads generating logging messages.
 * Speed, # of threads, and # of messages can be adjusted.
 *
 * This uses the synchronous file output sink, with either
 * the first arg as output file, or a hardcoded file name
 *
 */

#define MT_MCOUNT	2000UL	// # of log messages per thread
#define MT_THREADS	4	//concurrent threads
#define MT_DELAY	0	//delay between each log message call
#define DEFAULT_LOGFILE	"mt.log"

#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
	/* covers win32 / win64 targets */
	#include <windows.h>

#elif defined(__unix__)
	#define _XOPEN_SOURCE 700

	/* covers most other OSes */

	#include <errno.h>
	#include <unistd.h>
	#include <pthread.h>
	#include <time.h>
#else
	#error weird OS
#endif


#include "zf_log.h"
#include "zf_log_sinks.h"

struct tdata {
	unsigned thread_no;
	const char *msg_header;
};

#if defined(_WIN32)
HANDLE ttable[MT_MCOUNT];

DWORD WINAPI logger_func(LPVOID priv) {
	struct tdata *td = priv;
	unsigned mcnt;

	for (mcnt = 0; mcnt < MT_MCOUNT; mcnt++) {
		ZF_LOGI(td->msg_header, td->thread_no, mcnt);
		if (MT_DELAY) {
			Sleep(MT_DELAY);
		}
	}
	return 0;
}

#elif defined(__unix__)
pthread_t ttable[MT_THREADS];

static void *logger_func(void *priv) {
	struct tdata *td = priv;
	unsigned mcnt;

	for (mcnt = 0; mcnt < MT_MCOUNT; mcnt++) {
		ZF_LOGI(td->msg_header, td->thread_no, mcnt);
		if (MT_DELAY) {

			struct timespec rqst, resp;
			int rv;

			rqst.tv_sec = MT_DELAY / 1000;
			rqst.tv_nsec = (MT_DELAY % 1000) * 1000*1000;

			errno = 0;
			//clock_nanosleep is interruptible, hence this loop
			while ((rv=nanosleep(&rqst, &resp)) != 0) {
				if (rv == EINTR) {
					rqst = resp;
					errno = 0;
				} else {
					//unlikely
					break;
				}
			}
		}	//MT_DELAY
	}	//for
	return NULL;
}
#endif


static void start_test(void) {
	unsigned n;
	struct tdata td[MT_THREADS];
	const char header_base[] = "\t\t\t\t%02u mcnt=%u";

	for (n=0; n < MT_THREADS; n++) {
		td[n].msg_header = &header_base[(n & 0x03)];	//just an offset to help visualizing
		td[n].thread_no = n;
		printf("starting thread %u/%u\n", n, MT_THREADS);
#if defined(_WIN32)
		ttable[n] = CreateThread(NULL, 0, logger_func, &td[n], 0, NULL);
		if (ttable[n] == NULL) {
			printf("CreateThread creation problem. good luck\n");
		}

#elif defined(__unix__)
		if (pthread_create(&ttable[n], NULL, logger_func, &td[n])) {
			printf("pthread creation problem. good luck\n");
		}
#endif
	}


	for (n=0; n < MT_THREADS; n++) {
#if defined(_WIN32)
		WaitForSingleObject(ttable[n], 0);
#elif defined(__unix__)
		pthread_join(ttable[n], NULL);
#endif
		printf("thread %u/%u finished.\n", n, MT_THREADS);
	}
}

int main(int argc, char **argv) {
	const char dname[]=DEFAULT_LOGFILE;
	const char *fname;

	if (argc > 1) {
		fname = argv[1];
	} else {
		fname = dname;
	}

	if (sf_out_open(fname)) {
		exit(1);
	}

	start_test();

	sf_out_close();
	return 0;
}
