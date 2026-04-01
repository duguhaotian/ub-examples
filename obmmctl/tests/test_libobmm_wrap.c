/*
 * test_libobmm_wrap.c - Unit tests for libobmm_wrap
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libobmm_wrap.h"
#include "sysfs_reader.h"
#include "log.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name(void)
#define ASSERT(cond) do { \
    if (cond) { tests_passed++; } \
    else { tests_failed++; fprintf(stderr, "FAIL: %s\n", __func__); } \
} while(0)

static char *create_mock_sysfs(const char *eid_val, const char *cna_val)
{
    static char tmpdir[256];
    snprintf(tmpdir, sizeof(tmpdir), "/tmp/test_wrap_%d", getpid());
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

TEST(handle_init)
{
    obmm_handle_t handle;
    obmm_handle_init(&handle);

    ASSERT(handle.id == OBMM_INVALID_MEMID);
    ASSERT(handle.initialized == false);
    ASSERT(handle.is_export == false);
}

TEST(do_lend_success)
{
    char *mock_path = create_mock_sysfs("0x64", "0x401");

    controller_info_t ctrl;
    read_controller_info(mock_path, &ctrl);

    obmm_handle_t handle;
    int ret = obmm_do_lend(&ctrl, 2 * 1024 * 1024, OBMM_EXPORT_FLAG_ALLOW_MMAP, &handle);

    ASSERT(ret == 0);
    ASSERT(handle.initialized == true);
    ASSERT(handle.is_export == true);
    ASSERT(handle.id != OBMM_INVALID_MEMID);
    ASSERT(handle.desc.addr != 0);  /* Kernel allocated address */

    cleanup_mock_sysfs(mock_path);
}

TEST(do_lend_invalid_size)
{
    char *mock_path = create_mock_sysfs("0x64", "0x401");

    controller_info_t ctrl;
    read_controller_info(mock_path, &ctrl);

    obmm_handle_t handle;
    int ret = obmm_do_lend(&ctrl, 1024 * 1024, OBMM_EXPORT_FLAG_ALLOW_MMAP, &handle);  /* 1MB - not aligned */

    ASSERT(ret != 0);  /* Should fail */

    cleanup_mock_sysfs(mock_path);
}

TEST(do_borrow_success)
{
    char *mock_path = create_mock_sysfs("0x64", "0x401");

    controller_info_t ctrl;
    read_controller_info(mock_path, &ctrl);

    obmm_handle_t handle;
    uint64_t remote_addr = 0x100000000ULL;
    int ret = obmm_do_borrow(&ctrl, remote_addr, 2 * 1024 * 1024,
                              OBMM_IMPORT_FLAG_ALLOW_MMAP, &handle);

    ASSERT(ret == 0);
    ASSERT(handle.initialized == true);
    ASSERT(handle.is_export == false);
    ASSERT(handle.id != OBMM_INVALID_MEMID);

    cleanup_mock_sysfs(mock_path);
}

TEST(do_unlend_success)
{
    char *mock_path = create_mock_sysfs("0x64", "0x401");

    controller_info_t ctrl;
    read_controller_info(mock_path, &ctrl);

    obmm_handle_t handle;
    obmm_do_lend(&ctrl, 2 * 1024 * 1024, OBMM_EXPORT_FLAG_ALLOW_MMAP, &handle);

    int ret = obmm_do_unlend(handle.id, 0);
    ASSERT(ret == 0);

    cleanup_mock_sysfs(mock_path);
}

TEST(do_unborrow_success)
{
    char *mock_path = create_mock_sysfs("0x64", "0x401");

    controller_info_t ctrl;
    read_controller_info(mock_path, &ctrl);

    obmm_handle_t handle;
    obmm_do_borrow(&ctrl, 0x100000000ULL, 2 * 1024 * 1024,
                   OBMM_IMPORT_FLAG_ALLOW_MMAP, &handle);

    int ret = obmm_do_unborrow(handle.id, 0);
    ASSERT(ret == 0);

    cleanup_mock_sysfs(mock_path);
}

int run_libobmm_wrap_tests(void)
{
    log_set_level(LOG_LEVEL_ERROR);

    test_handle_init();
    test_do_lend_success();
    test_do_lend_invalid_size();
    test_do_borrow_success();
    test_do_unlend_success();
    test_do_unborrow_success();

    printf("libobmm_wrap tests: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}