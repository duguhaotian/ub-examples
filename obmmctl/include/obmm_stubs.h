/*
 * obmm_stubs.h - OBMM API stubs for Syntax Check Mode
 * Aligned with libobmm.h API definitions
 */

#ifndef OBM_STUBS_H
#define OBM_STUBS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Basic OBMM types */
typedef uint64_t mem_id;
#define OBMM_INVALID_MEMID ((mem_id)0)
#define OBMM_MAX_LOCAL_NUMA_NODES 16

/* Export flags (from libobmm) */
#define OBMM_EXPORT_FLAG_ALLOW_MMAP    0x01
#define OBMM_EXPORT_FLAG_FAST          0x02

/* Import flags (from libobmm) */
#define OBMM_IMPORT_FLAG_ALLOW_MMAP    0x01
#define OBMM_IMPORT_FLAG_NUMA_REMOTE   0x02
#define OBMM_IMPORT_FLAG_PREIMPORT     0x04

/*
 * Memory descriptor structure - matches struct obmm_mem_desc from libobmm.h
 * Note: priv[] flexible array member omitted for stubs simplicity
 */
struct obmm_mem_desc {
    uint64_t addr;        /* Physical address (UBA) */
    uint64_t length;      /* Memory size */
    uint8_t seid[16];     /* Source EID (128-bit, little-endian) */
    uint8_t deid[16];     /* Destination EID (128-bit, little-endian) */
    uint32_t tokenid;     /* Token ID */
    uint32_t scna;        /* Source CNA */
    uint32_t dcna;        /* Destination CNA */
    uint16_t priv_len;    /* Private data length */
};

/*
 * Preimport info structure - matches struct obmm_preimport_info from libobmm.h
 * Note: priv[] flexible array member omitted for stubs simplicity
 */
struct obmm_preimport_info {
    uint64_t pa;          /* Physical address base */
    uint64_t length;      /* Maximum memory size for preimport */
    int base_dist;        /* Base distance for NUMA */
    int numa_id;          /* Desired NUMA node ID (-1 for auto) */
    uint8_t seid[16];     /* Source EID */
    uint8_t deid[16];     /* Destination EID */
    uint32_t scna;        /* Source CNA */
    uint32_t dcna;        /* Destination CNA */
    uint16_t priv_len;    /* Private data length (ignored in preimport) */
};

/* STUB API declarations - aligned with libobmm.h */

/*
 * obmm_export - Export local memory
 * @length: Array of size per NUMA node (OBMM_MAX_LOCAL_NUMA_NODES elements)
 * @flags: Export flags
 * @desc: Memory descriptor (deid as input, addr/length/tokenid as output)
 * Returns: mem_id on success, OBMM_INVALID_MEMID on failure
 */
mem_id obmm_export(const size_t length[OBMM_MAX_LOCAL_NUMA_NODES],
                   unsigned long flags, struct obmm_mem_desc *desc);

/*
 * obmm_unexport - Release exported memory
 * @id: Memory ID to release
 * @flags: Unexport flags
 * Returns: 0 on success, -1 on failure
 */
int obmm_unexport(mem_id id, unsigned long flags);

/*
 * obmm_export_useraddr - Export user address space
 * @pid: Process ID (0 for current process)
 * @va: Virtual address
 * @length: Memory size
 * @flags: Export flags
 * @desc: Memory descriptor
 * Returns: mem_id on success, OBMM_INVALID_MEMID on failure
 */
mem_id obmm_export_useraddr(int pid, void *va, size_t length,
                            unsigned long flags, struct obmm_mem_desc *desc);

/*
 * obmm_import - Import remote memory
 * @desc: Memory descriptor with remote memory info
 * @flags: Import flags
 * @base_dist: Base distance for NUMA remote (0, 11-255)
 * @numa: NUMA node ID pointer (-1 for auto, NULL ignored)
 * Returns: mem_id on success, OBMM_INVALID_MEMID on failure
 */
mem_id obmm_import(const struct obmm_mem_desc *desc, unsigned long flags,
                   int base_dist, int *numa);

/*
 * obmm_unimport - Release imported memory
 * @id: Memory ID to release
 * @flags: Unimport flags
 * Returns: 0 on success, -1 on failure
 */
int obmm_unimport(mem_id id, unsigned long flags);

/*
 * obmm_preimport - Pre-import remote memory (accelerate NUMA creation)
 * @preimport_info: Preimport information
 * @flags: Preimport flags (must be 0)
 * Returns: 0 on success, -1 on failure
 */
int obmm_preimport(struct obmm_preimport_info *preimport_info, unsigned long flags);

/*
 * obmm_unpreimport - Release pre-imported memory
 * @preimport_info: Preimport information to release
 * @flags: Unpreimport flags
 * Returns: 0 on success, -1 on failure
 */
int obmm_unpreimport(const struct obmm_preimport_info *preimport_info, unsigned long flags);

/*
 * obmm_set_ownership - Change process ownership of OBMM memory
 * @fd: File descriptor of obmm_shmdev device
 * @start: Start virtual address
 * @end: End virtual address
 * @prot: Protection flags (PROT_NONE, PROT_READ, PROT_WRITE)
 * Returns: 0 on success, -1 on failure
 */
int obmm_set_ownership(int fd, void *start, void *end, int prot);

/*
 * obmm_query_memid_by_pa - Query mem_id by physical address
 * @pa: Physical address
 * @id: Output mem_id
 * @offset: Output offset in the memory region
 * Returns: 0 on success, -1 on failure
 */
int obmm_query_memid_by_pa(unsigned long pa, mem_id *id, unsigned long *offset);

/*
 * obmm_query_pa_by_memid - Query physical address by mem_id
 * @id: Memory ID
 * @offset: Offset in the memory region
 * @pa: Output physical address
 * Returns: 0 on success, -1 on failure
 */
int obmm_query_pa_by_memid(mem_id id, unsigned long offset, unsigned long *pa);

#ifdef __cplusplus
}
#endif

#endif /* OBM_STUBS_H */