/*
 * cmd_help.c - Help command implementation
 */

#include <stdio.h>
#include <string.h>
#include "obmmctl.h"
#include "cli_parser.h"

/* cmd_help - Show help for commands */
int cmd_help(int argc, char **argv)
{
    /* If a specific command is requested */
    if (argc > 2) {
        cmd_type_t cmd = get_command_type(argv[2]);
        if (cmd != CMD_UNKNOWN && cmd != CMD_HELP) {
            print_command_help(cmd);
            return EXIT_SUCCESS_CODE;
        }
        /* Unknown command */
        printf("No help available for: %s\n\n", argv[2]);
        print_general_usage(argv[0]);
        return EXIT_USAGE_ERROR;
    }

    /* No specific command - show general help */
    print_general_usage(argv[0]);
    return EXIT_SUCCESS_CODE;
}