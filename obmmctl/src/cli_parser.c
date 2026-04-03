/*
 * cli_parser.c - Command-line argument parser implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include "cli_parser.h"
#include "obmmctl.h"
#include "log.h"

/* Command definitions */
static const char *cmd_names[] = {
    "lend",
    "unlend",
    "borrow",
    "unborrow",
    "set-owner",
    "status",
    "help"
};

/* Initialize options structure */
void cmd_options_init(cmd_options_t *opts)
{
    if (opts != NULL) {
        opts->controller_path = NULL;
        opts->remote_addr = 0;
        opts->size = 0;
        opts->flags = 0;
        opts->mem_id = 0;
        opts->node_id = 0;
        opts->show_help = false;
        opts->verbose_count = 0;
    }
}

/* Parse size string with suffix (K, M, G) */
int parse_size(const char *str, size_t *size)
{
    if (str == NULL || size == NULL) {
        return -1;
    }

    /* Check for negative sign - size shouldn't be negative */
    if (*str == '-') {
        return -1;
    }

    char *endptr;
    errno = 0;
    unsigned long val = strtoul(str, &endptr, 0);

    /* Check for overflow */
    if (errno == ERANGE) {
        return -1;
    }

    if (endptr == str) {
        return -1;  /* No conversion performed */
    }

    /* Check for suffix */
    if (*endptr != '\0') {
        switch (toupper(*endptr)) {
            case 'K':
                val *= 1024;
                break;
            case 'M':
                val *= 1024 * 1024;
                break;
            case 'G':
                val *= 1024 * 1024 * 1024;
                break;
            default:
                return -1;  /* Invalid suffix */
        }

        /* Ensure no extra characters after suffix */
        if (*(endptr + 1) != '\0') {
            return -1;
        }
    }

    *size = (size_t)val;
    return 0;
}

/* Parse address string as uint64_t (hex or decimal) */
static int parse_uint64(const char *str, uint64_t *val)
{
    if (str == NULL || val == NULL) {
        return -1;
    }

    char *endptr;
    *val = strtoull(str, &endptr, 0);

    if (endptr == str || *endptr != '\0') {
        return -1;
    }

    return 0;
}

/* Get command name from type */
const char* get_command_name(cmd_type_t cmd)
{
    if (cmd >= 0 && cmd < CMD_COUNT) {
        return cmd_names[cmd];
    }
    return "unknown";
}

/* Get command type from name */
cmd_type_t get_command_type(const char *name)
{
    if (name == NULL) {
        return CMD_UNKNOWN;
    }

    for (int i = 0; i < CMD_COUNT; i++) {
        if (strcmp(name, cmd_names[i]) == 0) {
            return (cmd_type_t)i;
        }
    }

    return CMD_UNKNOWN;
}

/* Print general usage */
void print_general_usage(const char *program)
{
    printf("Usage: %s <command> [options]\n\n", program ? program : OBMCTL_NAME);
    printf("OBMM Control Tool - Memory borrowing/lending management\n\n");
    printf("Commands:\n");
    printf("  lend        Lend out memory to another node\n");
    printf("  unlend      Cancel lend out operation\n");
    printf("  borrow      Borrow memory from another node\n");
    printf("  unborrow    Cancel borrow operation\n");
    printf("  set-owner   Set memory ownership\n");
    printf("  status      Show memory region status\n");
    printf("  help        Show help for a command\n");
    printf("\nUse '%s help <command>' for command-specific help\n", program ? program : OBMCTL_NAME);
    printf("\nOptions:\n");
    printf("  -h, --help      Show this help message\n");
    printf("  -v, --verbose   Verbose output (repeat for more: -v=INFO, -vv=DEBUG)\n");
}

/* Print command-specific help */
void print_command_help(cmd_type_t cmd)
{
    switch (cmd) {
        case CMD_LEND:
            printf("Usage: obmmctl lend --eid <controller_path> [--size <size>] [--flags <flags>]\n\n");
            printf("Lend out memory to another node.\n\n");
            printf("Options:\n");
            printf("  -e, --eid <path>       Controller path (e.g. /sys/devices/ub_bus_controller0/0)\n");
            printf("  -s, --size <bytes>     Size in bytes (default: 128M, supports K, M, G suffixes)\n");
            printf("  -f, --flags <flags>    Export flags (default: 0x01 = ALLOW_MMAP)\n");
            printf("  -h, --help             Show this help\n");
            printf("\nOutput:\n");
            printf("  mem_id: Memory ID for unlend\n");
            printf("  addr:   Physical address (provide this to remote for borrow)\n");
            printf("\nExample:\n");
            printf("  obmmctl lend --eid /sys/devices/ub_bus_controller0/0 --size 128M\n");
            break;

        case CMD_UNLEND:
            printf("Usage: obmmctl unlend --id <mem_id>\n\n");
            printf("Cancel lend out operation.\n\n");
            printf("Options:\n");
            printf("  -i, --id <mem_id>      Memory ID to unexport\n");
            printf("  -h, --help             Show this help\n");
            break;

        case CMD_BORROW:
            printf("Usage: obmmctl borrow --eid <controller_path> --addr <remote_addr> --size <size> [--flags <flags>]\n\n");
            printf("Borrow memory from another node.\n\n");
            printf("Options:\n");
            printf("  -e, --eid <path>       Local controller path\n");
            printf("  -a, --addr <address>   Remote physical address (from remote lend output)\n");
            printf("  -s, --size <bytes>     Size in bytes (must match remote lend size)\n");
            printf("  -f, --flags <flags>    Import flags (default: 0x01 = ALLOW_MMAP)\n");
            printf("  -h, --help             Show this help\n");
            printf("\nExample:\n");
            printf("  obmmctl borrow --eid /sys/devices/ub_bus_controller0/0 --addr 0x100000000 --size 128M\n");
            break;

        case CMD_UNBORROW:
            printf("Usage: obmmctl unborrow --id <mem_id>\n\n");
            printf("Cancel borrow operation.\n\n");
            printf("Options:\n");
            printf("  -i, --id <mem_id>      Memory ID to unimport\n");
            printf("  -h, --help             Show this help\n");
            break;

        case CMD_SET_OWNER:
            printf("Usage: obmmctl set-owner --id <mem_id> --node <node_id>\n\n");
            printf("Set memory ownership.\n\n");
            printf("Options:\n");
            printf("  -i, --id <mem_id>      Memory ID to modify\n");
            printf("  -n, --node <node_id>   Target node ID for ownership\n");
            printf("  -h, --help             Show this help\n");
            break;

        case CMD_STATUS:
            printf("Usage: obmmctl status [--id <mem_id>]\n\n");
            printf("Show memory region status.\n\n");
            printf("Options:\n");
            printf("  -i, --id <mem_id>      Show status for specific memory ID\n");
            printf("  -h, --help             Show this help\n");
            break;

        case CMD_HELP:
            printf("Usage: obmmctl help [command]\n\n");
            printf("Show help for commands.\n");
            break;

        default:
            printf("Unknown command\n");
            break;
    }
}

/* Parse command-line arguments */
int parse_args(int argc, char **argv, cmd_type_t *cmd, cmd_options_t *opts)
{
    if (argc < 2 || cmd == NULL || opts == NULL) {
        return -1;
    }

    cmd_options_init(opts);

    /* First argument is command */
    *cmd = get_command_type(argv[1]);

    if (*cmd == CMD_UNKNOWN) {
        LOG_ERROR("Unknown command: %s", argv[1]);
        return -1;
    }

    /* Setup long options based on command */
    struct option long_options[] = {
        {"eid", required_argument, 0, 'e'},
        {"addr", required_argument, 0, 'a'},
        {"size", required_argument, 0, 's'},
        {"flags", required_argument, 0, 'f'},
        {"id", required_argument, 0, 'i'},
        {"node", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    /* Parse options */
    int opt;
    int option_index = 0;

    /* Skip command name, start from options */
    optind = 2;

    while ((opt = getopt_long(argc, argv, "e:a:s:f:i:n:hv",
                               long_options, &option_index)) != -1) {
        switch (opt) {
            case 'e':
                opts->controller_path = optarg;
                break;

            case 'a':
                if (parse_uint64(optarg, &opts->remote_addr) != 0) {
                    LOG_ERROR("Invalid address: %s", optarg);
                    return -1;
                }
                break;

            case 's':
                if (parse_size(optarg, &opts->size) != 0) {
                    LOG_ERROR("Invalid size: %s", optarg);
                    return -1;
                }
                break;

            case 'f':
                {
                    char *endptr;
                    errno = 0;
                    unsigned long flags_val = strtoul(optarg, &endptr, 0);
                    if (errno == ERANGE || endptr == optarg || *endptr != '\0') {
                        LOG_ERROR("Invalid flags: %s", optarg);
                        return -1;
                    }
                    opts->flags = (uint32_t)flags_val;
                }
                break;

            case 'i':
                {
                    char *endptr;
                    errno = 0;
                    unsigned long long mem_id_val = strtoull(optarg, &endptr, 0);
                    if (errno == ERANGE || endptr == optarg || *endptr != '\0') {
                        LOG_ERROR("Invalid mem_id: %s", optarg);
                        return -1;
                    }
                    opts->mem_id = mem_id_val;
                }
                break;

            case 'n':
                {
                    char *endptr;
                    errno = 0;
                    long node_val = strtol(optarg, &endptr, 0);
                    if (errno == ERANGE || endptr == optarg || *endptr != '\0') {
                        LOG_ERROR("Invalid node_id: %s", optarg);
                        return -1;
                    }
                    opts->node_id = (int)node_val;
                }
                break;

            case 'h':
                opts->show_help = true;
                break;

            case 'v':
                opts->verbose_count++;
                break;

            case '?':
                /* getopt_long already printed an error message */
                return -1;

            default:
                return -1;
        }
    }

    return 0;
}