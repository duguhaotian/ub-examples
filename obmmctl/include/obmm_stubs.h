/*
 * obmm_stubs.h - OBMM API stubs for Syntax Check Mode
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

/* Export flags (from libobmm) */
#define OBMM_EXPORT_FLAG_ALLOW_MMAP    0x01

/* Import flags (from libobmm) */
#define OBMM_IMPORT_FLAG_ALLOW_MMAP    0x01

/* Memory descriptor structure - matches struct obmm_mem_desc from libobmm */
typedef struct {
    uint8_t deid[16];     /* Destination EID (for export) */
    uint8_t seid[16];     /* Source EID (for import) */
    uint64_t scna;        /* Source CNA (for import) */
    uint64_t addr;        /* Physical address */
    size_t size;          /* Size in bytes (2MB aligned) */
    unsigned long flags;  /* Operation flags */
} obmm_desc_t;

/* STUB API declarations */
mem_id obmm_export(obmm_desc_t *desc, size_t length, unsigned long flags);
int obmm_unexport(mem_id id, unsigned long flags);

mem_id obmm_import(obmm_desc_t *desc, size_t length, unsigned long flags);
int obmm_unimport(mem_id id, unsigned long flags);

int obmm_set_ownership(int fd, uint64_t start, uint64_t end, int prot);
int obmm_query_status(mem_id id, obmm_desc_t *desc);

#ifdef __cplusplus
}
#endif

#endif /* OBM_STUBS_H */