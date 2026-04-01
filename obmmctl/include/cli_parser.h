/*
 * cli_parser.h - Command-line argument parser for obmmctl
 */

#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Command types */
typedef enum {
    CMD_UNKNOWN = -1,
    CMD_LEND = 0,
    CMD_UNLEND,
    CMD_BORROW,
    CMD_UNBORROW,
    CMD_SET_OWNER,
    CMD_STATUS,
    CMD_HELP,
    CMD_COUNT
} cmd_type_t;

/* Command options structure */
typedef struct {
    char *controller_path;  /* Controller sysfs path (e.g. /sys/devices/ub_bus_controller0/0) */
    uint64_t remote_addr;   /* Remote physical address for borrow */
    size_t size;            /* Size in bytes */
    uint32_t flags;         /* Operation flags */
    uint64_t mem_id;        /* Memory ID */
    int node_id;            /* Target node ID */
    bool show_help;         /* Show help flag */
    bool verbose;           /* Verbose output */
} cmd_options_t;

/* Function declarations */
void cmd_options_init(cmd_options_t *opts);
int parse_args(int argc, char **argv, cmd_type_t *cmd, cmd_options_t *opts);
void print_general_usage(const char *program);
void print_command_help(cmd_type_t cmd);
int parse_size(const char *str, size_t *size);
const char* get_command_name(cmd_type_t cmd);
cmd_type_t get_command_type(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* CLI_PARSER_H */