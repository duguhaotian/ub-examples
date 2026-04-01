/*
 * test_sysfs_reader.c - Unit tests for sysfs_reader
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "sysfs_reader.h"
#include "log.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name(void)
#define ASSERT(cond) do { \
    if (cond) { tests_passed++; } \
    else { tests_failed++; fprintf(stderr, "FAIL: %s\n", __func__); } \
} while(0)

/* Create temporary mock sysfs directory */
static char *create_mock_sysfs(const char *eid_val, const char *cna_val)
{
    static char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/test_sysfs_%d", getpid());

    mkdir(tmpdir, 0755);

    if (eid_val) {
        char path[512];
        snprintf(path, sizeof(path), "%s/eid", tmpdir);
        FILE *fp = fopen(path, "w");
        if (fp) { fprintf(fp, "%s\n", eid_val); fclose(fp); }
    }

    if (cna_val) {
        char path[512];
        snprintf(path, sizeof(path), "%s/primary_cna", tmpdir);
        FILE *fp = fopen(path, "w");
        if (fp) { fprintf(fp, "%s\n", cna_val); fclose(fp); }
    }

    return tmpdir;
}

static void cleanup_mock_sysfs(const char *path)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

TEST(init_controller_info)
{
    controller_info_t info;
    controller_info_init(&info);

    ASSERT(info.eid[0] == 0);
    ASSERT(info.eid[15] == 0);
    ASSERT(info.primary_cna == 0);
    ASSERT(info.numa_id == -1);
    ASSERT(info.valid == false);
}

TEST(read_eid_decimal)
{
    char *mock_path = create_mock_sysfs("100", NULL);

    uint8_t eid[16];
    int ret = read_eid(mock_path, eid);

    ASSERT(ret == 0);
    ASSERT(eid[0] == 100);  /* 100 in little-endian */
    ASSERT(eid[1] == 0);
    ASSERT(eid[2] == 0);
    ASSERT(eid[3] == 0);

    cleanup_mock_sysfs(mock_path);
}

TEST(read_eid_hex)
{
    char *mock_path = create_mock_sysfs("0x64", NULL);

    uint8_t eid[16];
    int ret = read_eid(mock_path, eid);

    ASSERT(ret == 0);
    ASSERT(eid[0] == 0x64);  /* 100 decimal */

    cleanup_mock_sysfs(mock_path);
}

TEST(read_primary_cna)
{
    char *mock_path = create_mock_sysfs("100", "0x401");

    uint32_t cna;
    int ret = read_primary_cna(mock_path, &cna);

    ASSERT(ret == 0);
    ASSERT(cna == 0x401);

    cleanup_mock_sysfs(mock_path);
}

TEST(read_controller_info_success)
{
    char *mock_path = create_mock_sysfs("0x64", "0x401");

    controller_info_t info;
    int ret = read_controller_info(mock_path, &info);

    ASSERT(ret == 0);
    ASSERT(info.valid == true);
    ASSERT(info.eid[0] == 0x64);
    ASSERT(info.primary_cna == 0x401);

    cleanup_mock_sysfs(mock_path);
}

TEST(read_controller_info_missing_eid)
{
    char *mock_path = create_mock_sysfs(NULL, "0x401");

    controller_info_t info;
    int ret = read_controller_info(mock_path, &info);

    ASSERT(ret != 0);  /* Should fail */
    ASSERT(info.valid == false);

    cleanup_mock_sysfs(mock_path);
}

TEST(read_eid_missing_file)
{
    char *mock_path = create_mock_sysfs(NULL, NULL);

    uint8_t eid[16];
    int ret = read_eid(mock_path, eid);

    ASSERT(ret != 0);  /* Should fail */

    cleanup_mock_sysfs(mock_path);
}

int run_sysfs_reader_tests(void)
{
    log_set_level(LOG_LEVEL_ERROR);  /* Quiet for tests */

    test_init_controller_info();
    test_read_eid_decimal();
    test_read_eid_hex();
    test_read_primary_cna();
    test_read_controller_info_success();
    test_read_controller_info_missing_eid();
    test_read_eid_missing_file();

    printf("sysfs_reader tests: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}