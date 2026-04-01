/*
 * main.c - OBMM Control Tool entry point
 */

#include <stdio.h>
#include <string.h>
#include "obmmctl.h"
#include "cli_parser.h"
#include "log.h"

/* Forward declarations for command handlers */
extern int cmd_lend(int argc, char **argv);
extern int cmd_unlend(int argc, char **argv);
extern int cmd_borrow(int argc, char **argv);
extern int cmd_unborrow(int argc, char **argv);
extern int cmd_set_owner(int argc, char **argv);
extern int cmd_status(int argc, char **argv);
extern int cmd_help(int argc, char **argv);

/* Command dispatch table */
static const cmd_entry_t commands[] = {
    { "lend",      cmd_lend,      "obmmctl lend [options]",      "Lend out memory" },
    { "unlend",    cmd_unlend,    "obmmctl unlend [options]",    "Cancel lend out" },
    { "borrow",    cmd_borrow,    "obmmctl borrow [options]",    "Borrow memory" },
    { "unborrow",  cmd_unborrow,  "obmmctl unborrow [options]",  "Cancel borrow" },
    { "set-owner", cmd_set_owner, "obmmctl set-owner [options]", "Set ownership" },
    { "status",    cmd_status,    "obmmctl status [options]",    "Show status" },
    { "help",      cmd_help,      "obmmctl help [command]",      "Show help" },
};

static const size_t num_commands = sizeof(commands) / sizeof(commands[0]);

/* Get version string */
const char* get_version(void)
{
    return OBMCTL_VERSION;
}

/* Main entry point */
int main(int argc, char **argv)
{
    /* Check for minimum arguments */
    if (argc < 2) {
        print_general_usage(argv[0]);
        return EXIT_USAGE_ERROR;
    }

    /* Check for global options */
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        printf("%s version %s\n", OBMCTL_NAME, get_version());
#ifdef USE_REAL_LIBOBMM
        printf("Execution Mode: Production (libobmm linked)\n");
#else
        printf("Execution Mode: Test (stubs enabled)\n");
#endif
        return EXIT_SUCCESS_CODE;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_general_usage(argv[0]);
        return EXIT_SUCCESS_CODE;
    }

    /* Find and dispatch command */
    cmd_type_t cmd_type = get_command_type(argv[1]);

    if (cmd_type == CMD_UNKNOWN) {
        ERR("Unknown command: %s", argv[1]);
        fprintf(stderr, "\n");
        print_general_usage(argv[0]);
        return EXIT_COMMAND_NOT_FOUND;
    }

    /* Dispatch to command handler */
    for (size_t i = 0; i < num_commands; i++) {
        if (cmd_type == (cmd_type_t)i) {
            return commands[i].handler(argc, argv);
        }
    }

    /* Should never reach here */
    ERR("Internal error: command handler not found");
    return EXIT_FAILURE_CODE;
}
