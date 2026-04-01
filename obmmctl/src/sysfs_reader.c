/*
 * sysfs_reader.c - Read hardware parameters from sysfs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "sysfs_reader.h"
#include "log.h"

#define MAX_PATH_LEN 512
#define MAX_LINE_LEN 128

void controller_info_init(controller_info_t *info)
{
    if (info) {
        memset(info->eid, 0, 16);
        info->primary_cna = 0;
        info->numa_id = -1;
        info->ummu_map = -1;
        info->valid = false;
    }
}

/* Read integer from file, supports hex (0x) and decimal */
static int read_int_from_file(const char *filepath, long *value)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        LOG_ERROR("cannot open %s: %s", filepath, strerror(errno));
        return -errno;
    }

    char line[MAX_LINE_LEN];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        LOG_ERROR("failed to read from %s", filepath);
        return -EIO;
    }
    fclose(fp);

    /* Parse as integer (handles hex 0x prefix) */
    char *endptr;
    errno = 0;
    *value = strtol(line, &endptr, 0);
    if (errno == ERANGE) {
        LOG_ERROR("value overflow in %s: %s", filepath, line);
        return -ERANGE;
    }
    if (endptr == line) {
        LOG_ERROR("invalid value in %s: %s", filepath, line);
        return -EINVAL;
    }

    return 0;
}

int read_eid(const char *base_path, uint8_t *eid)
{
    if (!base_path || !eid) {
        return -EINVAL;
    }

    char filepath[MAX_PATH_LEN];
    int len = snprintf(filepath, sizeof(filepath), "%s/eid", base_path);
    if (len < 0 || len >= (int)sizeof(filepath)) {
        return -ENAMETOOLONG;
    }

    long value;
    int ret = read_int_from_file(filepath, &value);
    if (ret != 0) {
        return ret;
    }

    /* Store as little-endian in first 4 bytes, rest zero */
    memset(eid, 0, 16);
    eid[0] = (uint8_t)(value & 0xFF);
    eid[1] = (uint8_t)((value >> 8) & 0xFF);
    eid[2] = (uint8_t)((value >> 16) & 0xFF);
    eid[3] = (uint8_t)((value >> 24) & 0xFF);

    LOG_DEBUG("read eid from %s: 0x%lx", filepath, value);
    return 0;
}

int read_primary_cna(const char *base_path, uint32_t *cna)
{
    if (!base_path || !cna) {
        return -EINVAL;
    }

    char filepath[MAX_PATH_LEN];
    int len = snprintf(filepath, sizeof(filepath), "%s/primary_cna", base_path);
    if (len < 0 || len >= (int)sizeof(filepath)) {
        return -ENAMETOOLONG;
    }

    long value;
    int ret = read_int_from_file(filepath, &value);
    if (ret != 0) {
        return ret;
    }

    *cna = (uint32_t)value;
    LOG_DEBUG("read primary_cna from %s: 0x%x", filepath, *cna);
    return 0;
}

int read_numa_id(const char *base_path, int *numa)
{
    if (!base_path || !numa) {
        return -EINVAL;
    }

    char filepath[MAX_PATH_LEN];
    int len = snprintf(filepath, sizeof(filepath), "%s/numa_id", base_path);
    if (len < 0 || len >= (int)sizeof(filepath)) {
        return -ENAMETOOLONG;
    }

    long value;
    int ret = read_int_from_file(filepath, &value);
    if (ret != 0) {
        /* numa_id may not exist, don't fail hard */
        *numa = -1;
        return 0;
    }

    *numa = (int)value;
    return 0;
}

int read_ummu_map(const char *base_path, int *ummu_map)
{
    if (!base_path || !ummu_map) {
        return -EINVAL;
    }

    char filepath[MAX_PATH_LEN];
    int len = snprintf(filepath, sizeof(filepath), "%s/ummu_map", base_path);
    if (len < 0 || len >= (int)sizeof(filepath)) {
        return -ENAMETOOLONG;
    }

    long value;
    int ret = read_int_from_file(filepath, &value);
    if (ret != 0) {
        /* ummu_map may not exist, don't fail hard */
        *ummu_map = -1;
        return 0;
    }

    *ummu_map = (int)value;
    return 0;
}

int read_controller_info(const char *controller_path, controller_info_t *info)
{
    if (!controller_path || !info) {
        return -EINVAL;
    }

    controller_info_init(info);

    /* Read eid (required) */
    int ret = read_eid(controller_path, info->eid);
    if (ret != 0) {
        LOG_ERROR("controller path not found: %s", controller_path);
        return ret;
    }

    /* Read primary_cna (required for borrow) */
    ret = read_primary_cna(controller_path, &info->primary_cna);
    if (ret != 0) {
        LOG_WARN("primary_cna not available for %s", controller_path);
        /* Don't fail, may not be needed for all operations */
    }

    /* Read optional fields */
    read_numa_id(controller_path, &info->numa_id);
    read_ummu_map(controller_path, &info->ummu_map);

    info->valid = true;
    LOG_INFO("controller info loaded from %s", controller_path);
    return 0;
}