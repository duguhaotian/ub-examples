/*
 * cmd_status.c - Status command implementation
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "obmmctl.h"
#include "cli_parser.h"
#include "libobmm_wrap.h"
#include "log.h"

/* cmd_status - Show memory status */
int cmd_status(int argc, char **argv)
{
    cmd_type_t cmd;
    cmd_options_t opts;

    if (parse_args(argc, argv, &cmd, &opts) != 0) {
        print_command_help(CMD_STATUS);
        return EXIT_USAGE_ERROR;
    }

    if (opts.show_help) {
        print_command_help(CMD_STATUS);
        return EXIT_SUCCESS_CODE;
    }

    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }

    /* If mem_id specified, show specific status */
    if (opts.mem_id != 0) {
        unsigned long pa;
        int ret = obmm_query_pa((mem_id)opts.mem_id, 0, &pa);

        if (ret != 0) {
            LOG_ERROR("Failed to query status for mem_id %lu",
                      (unsigned long)opts.mem_id);
            return EXIT_OBMM_ERROR;
        }

        printf("Memory Region Status:\n");
        printf("  mem_id:  %lu\n", (unsigned long)opts.mem_id);
        printf("  pa:      0x%lx (at offset 0)\n", pa);
    } else {
        /* Show general status */
        printf("OBMM Status:\n");
        printf("  Mode: NON-NUMA\n");
        printf("  Active regions: (use --id <mem_id> for specific status)\n");
    }

    return EXIT_SUCCESS_CODE;
}