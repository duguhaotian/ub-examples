/*
 * obmmctl.h - OBMM Control Tool main header
 *
 * This is a command-line debugging tool for OBMM (Ownership-Based Memory Management)
 * library supporting memory lending/borrowing operations in NON-NUMA mode.
 *
 * Execution Mode: Syntax Check Mode (uses API stubs)
 */

#ifndef OBMCTL_H
#define OBMCTL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version information */
#define OBMCTL_VERSION "1.0.0"
#define OBMCTL_NAME "obmmctl"

/* Exit codes */
typedef enum {
    EXIT_SUCCESS_CODE = 0,
    EXIT_FAILURE_CODE = 1,
    EXIT_USAGE_ERROR = 2,
    EXIT_OBMM_ERROR = 3,
    EXIT_PERMISSION_DENIED = 126,
    EXIT_COMMAND_NOT_FOUND = 127
} exit_code_t;

/* Command handler function type */
typedef int (*cmd_handler_t)(int argc, char **argv);

/* Command entry structure */
typedef struct {
    const char *name;
    cmd_handler_t handler;
    const char *usage;
    const char *description;
} cmd_entry_t;

/* Function declarations */
const char* get_version(void);
void print_usage(const char *program);

#ifdef __cplusplus
}
#endif

#endif /* OBMCTL_H */
