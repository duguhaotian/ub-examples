/*
 * cmd_borrow.c - Borrow/Unborrow command implementations
 */

#include <stdio.h>
#include "obmmctl.h"
#include "cli_parser.h"
#include "sysfs_reader.h"
#include "libobmm_wrap.h"
#include "log.h"

#define DEFAULT_FLAGS OBMM_IMPORT_FLAG_ALLOW_MMAP
#define DEFAULT_BASE_DIST 0

int cmd_borrow(int argc, char **argv)
{
    cmd_type_t cmd;
    cmd_options_t opts;

    if (parse_args(argc, argv, &cmd, &opts) != 0) {
        print_command_help(CMD_BORROW);
        return EXIT_USAGE_ERROR;
    }

    if (opts.show_help) {
        print_command_help(CMD_BORROW);
        return EXIT_SUCCESS_CODE;
    }

    /* Validate required arguments */
    if (opts.controller_path == NULL) {
        LOG_ERROR("Missing required argument: --eid");
        print_command_help(CMD_BORROW);
        return EXIT_USAGE_ERROR;
    }

    if (opts.remote_addr == 0) {
        LOG_ERROR("Missing required argument: --addr");
        print_command_help(CMD_BORROW);
        return EXIT_USAGE_ERROR;
    }

    if (opts.size == 0) {
        LOG_ERROR("Missing required argument: --size");
        print_command_help(CMD_BORROW);
        return EXIT_USAGE_ERROR;
    }

    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }

    unsigned long flags = opts.flags > 0 ? opts.flags : DEFAULT_FLAGS;
    int base_dist = DEFAULT_BASE_DIST;
    int *numa_ptr = NULL;  /* NULL means we don't care about numa assignment */

    LOG_INFO("Reading controller info from %s", opts.controller_path);

    /* Read hardware parameters from sysfs */
    controller_info_t ctrl;
    int ret = read_controller_info(opts.controller_path, &ctrl);
    if (ret != 0) {
        LOG_ERROR("Failed to read controller info from %s", opts.controller_path);
        return EXIT_OBMM_ERROR;
    }

    /* Perform borrow operation */
    obmm_handle_t handle;
    ret = obmm_do_borrow(&ctrl, opts.remote_addr, opts.size, flags, base_dist, numa_ptr, &handle);
    if (ret != 0) {
        LOG_ERROR("Borrow operation failed");
        return EXIT_OBMM_ERROR;
    }

    /* Output key information */
    printf("mem_id: %lu\n", (unsigned long)handle.id);
    printf("size: %zu\n", handle.desc.length);

    /* Prevent auto cleanup */
    handle.initialized = false;

    return EXIT_SUCCESS_CODE;
}

int cmd_unborrow(int argc, char **argv)
{
    cmd_type_t cmd;
    cmd_options_t opts;

    if (parse_args(argc, argv, &cmd, &opts) != 0) {
        print_command_help(CMD_UNBORROW);
        return EXIT_USAGE_ERROR;
    }

    if (opts.show_help) {
        print_command_help(CMD_UNBORROW);
        return EXIT_SUCCESS_CODE;
    }

    if (opts.mem_id == 0) {
        LOG_ERROR("Missing required argument: --id");
        print_command_help(CMD_UNBORROW);
        return EXIT_USAGE_ERROR;
    }

    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }

    int ret = obmm_do_unborrow((mem_id)opts.mem_id, 0);
    if (ret != 0) {
        LOG_ERROR("Unborrow operation failed for mem_id=%lu", (unsigned long)opts.mem_id);
        return EXIT_OBMM_ERROR;
    }

    LOG_INFO("Successfully unborrowed memory");
    return EXIT_SUCCESS_CODE;
}