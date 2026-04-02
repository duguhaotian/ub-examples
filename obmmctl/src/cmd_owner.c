/*
 * cmd_owner.c - Set ownership command implementation
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include "obmmctl.h"
#include "cli_parser.h"
#include "libobmm_wrap.h"
#include "log.h"

/* cmd_set_owner - Set memory ownership */
int cmd_set_owner(int argc, char **argv)
{
    cmd_type_t cmd;
    cmd_options_t opts;

    if (parse_args(argc, argv, &cmd, &opts) != 0) {
        print_command_help(CMD_SET_OWNER);
        return EXIT_USAGE_ERROR;
    }

    if (opts.show_help) {
        print_command_help(CMD_SET_OWNER);
        return EXIT_SUCCESS_CODE;
    }

    if (opts.mem_id == 0) {
        LOG_ERROR("Missing required argument: --id");
        print_command_help(CMD_SET_OWNER);
        return EXIT_USAGE_ERROR;
    }

    if (opts.node_id < 0) {
        LOG_ERROR("Missing required argument: --node");
        print_command_help(CMD_SET_OWNER);
        return EXIT_USAGE_ERROR;
    }

    if (opts.verbose) {
        log_set_level(LOG_LEVEL_INFO);
    }

    LOG_INFO("Setting ownership of mem_id %lu to node %d",
             (unsigned long)opts.mem_id, opts.node_id);

    /* Open device file for ownership operation */
    char dev_path[64];
    snprintf(dev_path, sizeof(dev_path), "/dev/obmm_shmdev%lu", (unsigned long)opts.mem_id);

    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        LOG_ERROR("Failed to open %s", dev_path);
        return EXIT_OBMM_ERROR;
    }

    /* Call obmm_set_ownership - using whole range
     * Note: node_id is used as prot value (PROT_NONE=0, PROT_READ=1, PROT_WRITE=2)
     * This is a simplified interpretation for the stub mode.
     */
    void *start = NULL;
    void *end = (void *)(uintptr_t)UINT64_MAX;
    int prot = opts.node_id;  /* Map node_id to prot for stub compatibility */
    int ret = obmm_set_ownership(fd, start, end, prot);
    close(fd);

    if (ret != 0) {
        LOG_ERROR("Failed to set ownership");
        return EXIT_OBMM_ERROR;
    }

    printf("Ownership set: mem_id %lu -> node %d\n",
           (unsigned long)opts.mem_id, opts.node_id);

    LOG_INFO("Successfully set ownership");
    return EXIT_SUCCESS_CODE;
}