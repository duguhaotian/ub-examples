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

    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }

    /* If mem_id specified, show specific status */
    if (opts.mem_id != 0) {
        obmm_desc_t desc;
        memset(&desc, 0, sizeof(desc));

        int ret = obmm_query_mem_status((mem_id)opts.mem_id, &desc);
        if (ret != 0) {
            LOG_ERROR("Failed to query status for mem_id %lu",
                      (unsigned long)opts.mem_id);
            return EXIT_OBMM_ERROR;
        }

        printf("Memory Region Status:\n");
        printf("  mem_id:  %lu\n", (unsigned long)opts.mem_id);
        printf("  addr:    0x%lx\n", (unsigned long)desc.addr);
        printf("  size:    %zu bytes (%.2f MB)\n", desc.size, desc.size / (1024.0 * 1024.0));
        printf("  flags:   0x%lx\n", desc.flags);
    } else {
        /* Show general status */
        printf("OBMM Status:\n");
        printf("  Mode: NON-NUMA\n");
        printf("  Active regions: (use --id <mem_id> for specific status)\n");
    }

    return EXIT_SUCCESS_CODE;
}