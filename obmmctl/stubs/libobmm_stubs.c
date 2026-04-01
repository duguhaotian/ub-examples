/*
 * libobmm_stubs.c - OBMM API stub implementations
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "obmm_stubs.h"

static mem_id next_export_id = 1000;
static mem_id next_import_id = 2000;

mem_id obmm_export(obmm_desc_t *desc, size_t length, unsigned long flags)
{
    if (desc == NULL) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    if (length % (2 * 1024 * 1024) != 0) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    /* STUB: simulate kernel allocating memory */
    desc->size = length;
    desc->flags = flags;
    desc->addr = 0x100000000ULL + (next_export_id * length);  /* Fake physical addr */

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

mem_id obmm_import(obmm_desc_t *desc, size_t length, unsigned long flags)
{
    if (desc == NULL) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    if (length % (2 * 1024 * 1024) != 0) {
        errno = EINVAL;
        return OBMM_INVALID_MEMID;
    }

    desc->size = length;
    desc->flags = flags;

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

int obmm_set_ownership(int fd, uint64_t start, uint64_t end, int prot)
{
    (void)fd; (void)start; (void)end; (void)prot;
    return 0;
}

int obmm_query_status(mem_id id, obmm_desc_t *desc)
{
    if (desc == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (id == OBMM_INVALID_MEMID) {
        errno = EINVAL;
        return -1;
    }

    memset(desc, 0, sizeof(obmm_desc_t));
    desc->size = 2 * 1024 * 1024;

    return 0;
}