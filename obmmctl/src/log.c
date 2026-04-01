/*
 * log.c - Logging system implementation
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "log.h"

static log_level_t current_level = LOG_LEVEL_ERROR;
static const char *level_names[] = {
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

void log_set_level(log_level_t level)
{
    if (level >= LOG_LEVEL_ERROR && level <= LOG_LEVEL_DEBUG) {
        current_level = level;
    }
}

log_level_t log_get_level(void)
{
    return current_level;
}

void log_output(log_level_t level, const char *fmt, ...)
{
    if (level > current_level) {
        return;
    }

    const char *level_name = level_names[level];
    fprintf(stderr, "[%s] obmmctl: ", level_name);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}