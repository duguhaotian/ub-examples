/*
 * cmd_lend.c - Lend/Unlend command implementations
 */

#include <stdio.h>
#include "obmmctl.h"
#include "cli_parser.h"
#include "sysfs_reader.h"
#include "libobmm_wrap.h"
#include "log.h"

#define DEFAULT_SIZE (128 * 1024 * 1024)  /* 128MB */
#define DEFAULT_FLAGS OBMM_EXPORT_FLAG_ALLOW_MMAP

int cmd_lend(int argc, char **argv)
{
    cmd_type_t cmd;
    cmd_options_t opts;

    if (parse_args(argc, argv, &cmd, &opts) != 0) {
        print_command_help(CMD_LEND);
        return EXIT_USAGE_ERROR;
    }

    if (opts.show_help) {
        print_command_help(CMD_LEND);
        return EXIT_SUCCESS_CODE;
    }

    /* Validate required --eid (controller path) */
    if (opts.controller_path == NULL) {
        LOG_ERROR("Missing required argument: --eid");
        print_command_help(CMD_LEND);
        return EXIT_USAGE_ERROR;
    }

    /* Set verbose mode */
    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }

    /* Use default size if not specified */
    size_t size = opts.size > 0 ? opts.size : DEFAULT_SIZE;
    unsigned long flags = opts.flags > 0 ? opts.flags : DEFAULT_FLAGS;

    LOG_INFO("Reading controller info from %s", opts.controller_path);

    /* Read hardware parameters from sysfs */
    controller_info_t ctrl;
    int ret = read_controller_info(opts.controller_path, &ctrl);
    if (ret != 0) {
        LOG_ERROR("Failed to read controller info from %s", opts.controller_path);
        return EXIT_OBMM_ERROR;
    }

    /* Perform lend operation */
    obmm_handle_t handle;
    ret = obmm_do_lend(&ctrl, size, flags, &handle);
    if (ret != 0) {
        LOG_ERROR("Lend operation failed");
        return EXIT_OBMM_ERROR;
    }

    /* Output key information */
    printf("mem_id: %lu\n", (unsigned long)handle.id);
    printf("addr: 0x%lx\n", (unsigned long)handle.desc.addr);
    printf("size: %zu\n", handle.desc.length);

    /* Prevent auto cleanup - caller responsible for unlend */
    handle.initialized = false;

    return EXIT_SUCCESS_CODE;
}

int cmd_unlend(int argc, char **argv)
{
    cmd_type_t cmd;
    cmd_options_t opts;

    if (parse_args(argc, argv, &cmd, &opts) != 0) {
        print_command_help(CMD_UNLEND);
        return EXIT_USAGE_ERROR;
    }

    if (opts.show_help) {
        print_command_help(CMD_UNLEND);
        return EXIT_SUCCESS_CODE;
    }

    if (opts.mem_id == 0) {
        LOG_ERROR("Missing required argument: --id");
        print_command_help(CMD_UNLEND);
        return EXIT_USAGE_ERROR;
    }

    if (opts.verbose_count >= 2) {
        log_set_level(LOG_LEVEL_DEBUG);
    } else if (opts.verbose_count >= 1) {
        log_set_level(LOG_LEVEL_INFO);
    }

    int ret = obmm_do_unlend((mem_id)opts.mem_id, 0);
    if (ret != 0) {
        LOG_ERROR("Unlend operation failed for mem_id=%lu", (unsigned long)opts.mem_id);
        return EXIT_OBMM_ERROR;
    }

    LOG_INFO("Successfully unlent memory");
    return EXIT_SUCCESS_CODE;
}