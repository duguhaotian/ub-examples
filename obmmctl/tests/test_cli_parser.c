/*
 * test_cli_parser.c - CLI parser unit tests
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "cli_parser.h"
#include "obmmctl.h"

static int test_count = 0;
static int fail_count = 0;

#define TEST_ASSERT(cond, msg) do { \
    test_count++; \
    if (!(cond)) { \
        printf("\n    FAIL: %s (line %d)", msg, __LINE__); \
        fail_count++; \
        return 1; \
    } \
} while(0)

int test_cli_parser(void)
{
    test_count = 0;
    fail_count = 0;

    /* Test 1: cmd_options_init */
    {
        cmd_options_t opts;
        memset(&opts, 0xFF, sizeof(opts));  /* Fill with garbage */
        cmd_options_init(&opts);
        TEST_ASSERT(opts.controller_path == NULL, "controller_path should be NULL after init");
        TEST_ASSERT(opts.remote_addr == 0, "remote_addr should be 0 after init");
        TEST_ASSERT(opts.size == 0, "size should be 0 after init");
        TEST_ASSERT(opts.flags == 0, "flags should be 0 after init");
        TEST_ASSERT(opts.mem_id == 0, "mem_id should be 0 after init");
        TEST_ASSERT(opts.node_id == 0, "node_id should be 0 after init");
        TEST_ASSERT(opts.show_help == false, "show_help should be false after init");
        TEST_ASSERT(opts.verbose_count == 0, "verbose_count should be 0 after init");
    }

    /* Test 2: parse_size - basic values */
    {
        size_t size;
        TEST_ASSERT(parse_size("1024", &size) == 0, "parse_size should succeed for 1024");
        TEST_ASSERT(size == 1024, "size should be 1024");

        TEST_ASSERT(parse_size("2K", &size) == 0, "parse_size should succeed for 2K");
        TEST_ASSERT(size == 2048, "size should be 2048 for 2K");

        TEST_ASSERT(parse_size("1M", &size) == 0, "parse_size should succeed for 1M");
        TEST_ASSERT(size == 1024 * 1024, "size should be 1048576 for 1M");

        TEST_ASSERT(parse_size("2G", &size) == 0, "parse_size should succeed for 2G");
        TEST_ASSERT(size == 2ULL * 1024 * 1024 * 1024, "size should be correct for 2G");
    }

    /* Test 3: parse_size - invalid values */
    {
        size_t size;
        TEST_ASSERT(parse_size(NULL, &size) != 0, "parse_size should fail for NULL");
        TEST_ASSERT(parse_size("abc", &size) != 0, "parse_size should fail for non-numeric");
        TEST_ASSERT(parse_size("1X", &size) != 0, "parse_size should fail for invalid suffix");
    }

    /* Test 4: get_command_name */
    {
        TEST_ASSERT(strcmp(get_command_name(CMD_LEND), "lend") == 0, "CMD_LEND should be 'lend'");
        TEST_ASSERT(strcmp(get_command_name(CMD_UNLEND), "unlend") == 0, "CMD_UNLEND should be 'unlend'");
        TEST_ASSERT(strcmp(get_command_name(CMD_BORROW), "borrow") == 0, "CMD_BORROW should be 'borrow'");
        TEST_ASSERT(strcmp(get_command_name(CMD_UNBORROW), "unborrow") == 0, "CMD_UNBORROW should be 'unborrow'");
        TEST_ASSERT(strcmp(get_command_name(CMD_SET_OWNER), "set-owner") == 0, "CMD_SET_OWNER should be 'set-owner'");
        TEST_ASSERT(strcmp(get_command_name(CMD_STATUS), "status") == 0, "CMD_STATUS should be 'status'");
        TEST_ASSERT(strcmp(get_command_name(CMD_HELP), "help") == 0, "CMD_HELP should be 'help'");
    }

    /* Test 5: get_command_type */
    {
        TEST_ASSERT(get_command_type("lend") == CMD_LEND, "'lend' should be CMD_LEND");
        TEST_ASSERT(get_command_type("unlend") == CMD_UNLEND, "'unlend' should be CMD_UNLEND");
        TEST_ASSERT(get_command_type("borrow") == CMD_BORROW, "'borrow' should be CMD_BORROW");
        TEST_ASSERT(get_command_type("unborrow") == CMD_UNBORROW, "'unborrow' should be CMD_UNBORROW");
        TEST_ASSERT(get_command_type("set-owner") == CMD_SET_OWNER, "'set-owner' should be CMD_SET_OWNER");
        TEST_ASSERT(get_command_type("status") == CMD_STATUS, "'status' should be CMD_STATUS");
        TEST_ASSERT(get_command_type("help") == CMD_HELP, "'help' should be CMD_HELP");
        TEST_ASSERT(get_command_type("unknown") == CMD_UNKNOWN, "'unknown' should be CMD_UNKNOWN");
        TEST_ASSERT(get_command_type(NULL) == CMD_UNKNOWN, "NULL should be CMD_UNKNOWN");
    }

    /* Test 6: parse_args with --eid and --addr */
    {
        char *args[] = { "obmmctl", "borrow", "--eid", "/sys/devices/ub_bus_controller0/0", "--addr", "0x100000000", "--size", "2M" };
        cmd_type_t cmd;
        cmd_options_t opts;

        TEST_ASSERT(parse_args(8, args, &cmd, &opts) == 0, "parse_args should succeed");
        TEST_ASSERT(cmd == CMD_BORROW, "cmd should be CMD_BORROW");
        TEST_ASSERT(strcmp(opts.controller_path, "/sys/devices/ub_bus_controller0/0") == 0, "controller_path should be set");
        TEST_ASSERT(opts.remote_addr == 0x100000000ULL, "remote_addr should be 0x100000000");
        TEST_ASSERT(opts.size == 2 * 1024 * 1024, "size should be 2M");
    }

    /* Test 7: parse_args with mem_id */
    {
        char *args[] = { "obmmctl", "unlend", "--id", "12345" };
        cmd_type_t cmd;
        cmd_options_t opts;

        TEST_ASSERT(parse_args(4, args, &cmd, &opts) == 0, "parse_args should succeed");
        TEST_ASSERT(cmd == CMD_UNLEND, "cmd should be CMD_UNLEND");
        TEST_ASSERT(opts.mem_id == 12345, "mem_id should be 12345");
    }

    /* Test 8: parse_args with node_id */
    {
        char *args[] = { "obmmctl", "set-owner", "--id", "12345", "--node", "1" };
        cmd_type_t cmd;
        cmd_options_t opts;

        TEST_ASSERT(parse_args(6, args, &cmd, &opts) == 0, "parse_args should succeed");
        TEST_ASSERT(cmd == CMD_SET_OWNER, "cmd should be CMD_SET_OWNER");
        TEST_ASSERT(opts.mem_id == 12345, "mem_id should be 12345");
        TEST_ASSERT(opts.node_id == 1, "node_id should be 1");
    }

    printf("(%d tests)", test_count);
    return fail_count > 0 ? 1 : 0;
}

int test_verbose_counting(void)
{
    test_count = 0;
    fail_count = 0;

    cmd_type_t cmd;
    cmd_options_t opts;

    // Test single -v
    char *args1[] = { "obmmctl", "lend", "-v", "--eid", "/sys/devices/ub_bus_controller0/0" };
    int ret = parse_args(5, args1, &cmd, &opts);
    TEST_ASSERT(ret == 0, "parse_args with -v should succeed");
    TEST_ASSERT(opts.verbose_count == 1, "single -v should set verbose_count to 1");

    // Test double -vv
    cmd_options_init(&opts);
    char *args2[] = { "obmmctl", "lend", "-v", "-v", "--eid", "/sys/devices/ub_bus_controller0/0" };
    ret = parse_args(6, args2, &cmd, &opts);
    TEST_ASSERT(ret == 0, "parse_args with -vv should succeed");
    TEST_ASSERT(opts.verbose_count == 2, "-vv should set verbose_count to 2");

    // Test --verbose --verbose
    cmd_options_init(&opts);
    char *args3[] = { "obmmctl", "lend", "--verbose", "--verbose", "--eid", "/sys/devices/ub_bus_controller0/0" };
    ret = parse_args(6, args3, &cmd, &opts);
    TEST_ASSERT(ret == 0, "parse_args with --verbose --verbose should succeed");
    TEST_ASSERT(opts.verbose_count == 2, "double --verbose should set verbose_count to 2");

    printf("(%d tests)", test_count);
    return fail_count > 0 ? 1 : 0;
}