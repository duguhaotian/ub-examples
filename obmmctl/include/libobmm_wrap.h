/*
 * libobmm_wrap.h - Wrapper for libobmm API
 */

#ifndef LIBOBMM_WRAP_H
#define LIBOBMM_WRAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "obmm_stubs.h"
#include "sysfs_reader.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Handle for OBMM operations */
typedef struct {
    mem_id id;                   /* Memory ID */
    struct obmm_mem_desc desc;   /* Memory descriptor */
    bool is_export;              /* true: lend/export, false: borrow/import */
    bool initialized;            /* Resource initialized flag */
} obmm_handle_t;

/* Initialize handle */
void obmm_handle_init(obmm_handle_t *handle);

/* Lend operation - export memory
 * ctrl: Controller info (deid from ctrl->eid, numa_id determines allocation)
 * size: Memory size (must be 2MB aligned)
 * flags: Default OBMM_EXPORT_FLAG_ALLOW_MMAP
 * Returns: 0 success, negative on failure
 */
int obmm_do_lend(const controller_info_t *ctrl, size_t size,
                 unsigned long flags, obmm_handle_t *handle);

/* Borrow operation - import memory
 * ctrl: Controller info (seid from ctrl->eid, scna from ctrl->primary_cna)
 * remote_addr: Remote physical address (from remote lend)
 * size: Memory size
 * flags: Default OBMM_IMPORT_FLAG_ALLOW_MMAP
 * base_dist: Base distance for NUMA remote (default 0)
 * numa: NUMA node ID pointer (NULL for auto)
 * Returns: 0 success, negative on failure
 */
int obmm_do_borrow(const controller_info_t *ctrl, uint64_t remote_addr, size_t size,
                   unsigned long flags, int base_dist, int *numa, obmm_handle_t *handle);

/* Unlend operation */
int obmm_do_unlend(mem_id id, unsigned long flags);

/* Unborrow operation */
int obmm_do_unborrow(mem_id id, unsigned long flags);

/* Query physical address by mem_id
 * id: Memory ID
 * offset: Offset in the memory region
 * pa: Output physical address
 * Returns: 0 success, negative on failure
 */
int obmm_query_pa(mem_id id, unsigned long offset, unsigned long *pa);

/* Cleanup handle */
void obmm_handle_cleanup(obmm_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* LIBOBMM_WRAP_H */