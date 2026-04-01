/*
 * sysfs_reader.h - Read hardware parameters from sysfs
 */

#ifndef SYSFS_READER_H
#define SYSFS_READER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t eid[16];       /* 128-bit EID (low 4 bytes from file, rest zero) */
    uint32_t primary_cna;  /* SCNA value from primary_cna file */
    int numa_id;           /* NUMA node ID */
    int ummu_map;          /* UMMU mapping */
    bool valid;            /* Read success flag */
} controller_info_t;

/* Initialize controller_info */
void controller_info_init(controller_info_t *info);

/* Read all info from controller path
 * path: e.g. "/sys/devices/ub_bus_controller0/0"
 * Returns: 0 success, negative on failure (errno set)
 */
int read_controller_info(const char *controller_path, controller_info_t *info);

/* Read individual attributes */
int read_eid(const char *path, uint8_t *eid);
int read_primary_cna(const char *path, uint32_t *cna);
int read_numa_id(const char *path, int *numa);
int read_ummu_map(const char *path, int *ummu_map);

#ifdef __cplusplus
}
#endif

#endif /* SYSFS_READER_H */