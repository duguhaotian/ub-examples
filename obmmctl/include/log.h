/*
 * log.h - Logging system for obmmctl
 */

#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_ERROR = 0,   /* Errors, always output */
    LOG_LEVEL_WARN  = 1,   /* Warnings */
    LOG_LEVEL_INFO  = 2,   /* Info (verbose mode) */
    LOG_LEVEL_DEBUG = 3,   /* Debug (debug mode) */
} log_level_t;

/* Set/get log level */
void log_set_level(log_level_t level);
log_level_t log_get_level(void);

/* Core output function */
void log_output(log_level_t level, const char *fmt, ...);

/* Convenience macros */
#define LOG_ERROR(fmt, ...)  log_output(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   log_output(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)   log_output(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)  log_output(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

/* EID formatting macros */
#define EID_FMT16 "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define EID_ARGS16(e) (e)[0], (e)[1], (e)[2], (e)[3], (e)[4], (e)[5], (e)[6], (e)[7], \
                      (e)[8], (e)[9], (e)[10], (e)[11], (e)[12], (e)[13], (e)[14], (e)[15]

#define EID_FMT64 "%02x%02x%02x%02x%02x%02x%02x%02x"
#define EID_ARGS64(e) (e)[0], (e)[1], (e)[2], (e)[3], (e)[4], (e)[5], (e)[6], (e)[7]

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */