#ifndef ZF_LOG_SINKS_H
#define ZF_LOG_SINKS_H

/* Collection of additional output sinks.
 * All are optional and selectable at compile-time.
 */

#ifdef __cplusplus
extern "C" {
#endif


/***********
 * Synchronous (blocking) file output sink.
 * Caller must ensure no more logging calls will be made
 * by any of its threads before calling sf_out_close().
 */

/** Ret 0 if ok */
int sf_out_open(const char *const fname);

void sf_out_close(void);


#ifdef __cplusplus
}
#endif

#endif // ZF_LOG_SINKS_H
