/*
 * libobmm_stubs.c - OBMM API stub implementations
 * Aligned with libobmm.h API definitions
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "obmm_stubs.h"

static mem_id next_export_id = 1000;
static mem_id next_import_id = 2000;

mem_id obmm_export(const size_t length[OBMM_MAX_LOCAL_NUMA_NODES],
                   unsigned long flags, struct obmm_mem_desc *desc)
{
    size_t total_size = 0;
    int i;

    if (desc == NULL || length == NULL) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    /* Calculate total size from all NUMA nodes */
    for (i = 0; i < OBMM_MAX_LOCAL_NUMA_NODES; i++) {
        if (length[i] > 0) {
            /* Check 2MB alignment */
            if (length[i] % (2 * 1024 * 1024) != 0) {
                errno = EINVAL;
                return OBMM_INVALID_MEMID;
            }
            total_size += length[i];
        }
    }

    if (total_size == 0) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    /* STUB: simulate kernel allocating memory */
    desc->length = total_size;
    (void)flags;  /* flags not stored in desc for libobmm API */
    desc->addr = 0x100000000ULL + (next_export_id * total_size);  /* Fake physical addr */
    desc->tokenid = (uint32_t)next_export_id;
    desc->scna = 0;
    desc->dcna = 0;

    return next_export_id++;
}

mem_id obmm_export_useraddr(int pid, void *va, size_t length,
                            unsigned long flags, struct obmm_mem_desc *desc)
{
    (void)pid;
    (void)flags;

    if (desc == NULL || va == NULL) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    if (length % (2 * 1024 * 1024) != 0) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    /* STUB: simulate kernel pinning and exporting user address */
    desc->length = length;
    desc->addr = (uint64_t)va;  /* Fake: use VA as UBA */
    desc->tokenid = (uint32_t)next_export_id;

    return next_export_id++;
}

int obmm_unexport(mem_id id, unsigned long flags)
{
    (void)flags;
    if (id == OBMM_INVALID_MEMID) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

mem_id obmm_import(const struct obmm_mem_desc *desc, unsigned long flags,
                   int base_dist, int *numa)
{
    (void)base_dist;
    (void)flags;

    if (desc == NULL) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    if (desc->length == 0) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    /* Check 2MB alignment */
    if (desc->length % (2 * 1024 * 1024) != 0) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    /* STUB: simulate import */
    if (numa != NULL) {
        *numa = -1;  /* No NUMA assignment in stub mode */
    }

    return next_import_id++;
}

int obmm_unimport(mem_id id, unsigned long flags)
{
    (void)flags;
    if (id == OBMM_INVALID_MEMID) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int obmm_preimport(struct obmm_preimport_info *preimport_info, unsigned long flags)
{
    (void)flags;

    if (preimport_info == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* STUB: simulate preimport success */
    if (preimport_info->numa_id == -1) {
        preimport_info->numa_id = 100;  /* Fake NUMA ID */
    }

    return 0;
}

int obmm_unpreimport(const struct obmm_preimport_info *preimport_info, unsigned long flags)
{
    (void)flags;

    if (preimport_info == NULL) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int obmm_set_ownership(int fd, void *start, void *end, int prot)
{
    (void)fd; (void)start; (void)end; (void)prot;
    return 0;
}

int obmm_query_memid_by_pa(unsigned long pa, mem_id *id, unsigned long *offset)
{
    /* STUB: return fake data for valid PA range */
    if (pa >= 0x100000000ULL && pa < 0x200000000ULL) {
        if (id) *id = 1000;
        if (offset) *offset = pa - 0x100000000ULL;
        return 0;
    }

    errno = ENOENT;
    return -1;
}

int obmm_query_pa_by_memid(mem_id id, unsigned long offset, unsigned long *pa)
{
    if (id == OBMM_INVALID_MEMID) {
        errno = ENOENT;
        return -1;
    }

    /* STUB: return fake PA for valid mem_id */
    if (pa) {
        *pa = 0x100000000ULL + (id * 2 * 1024 * 1024) + offset;
    }

    return 0;
}