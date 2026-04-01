/*
 * libobmm_wrap.c - Wrapper for libobmm API
 */

#include <string.h>
#include <errno.h>
#include "libobmm_wrap.h"
#include "log.h"

void obmm_handle_init(obmm_handle_t *handle)
{
    if (handle) {
        handle->id = OBMM_INVALID_MEMID;
        memset(&handle->desc, 0, sizeof(obmm_desc_t));
        handle->is_export = false;
        handle->initialized = false;
    }
}

int obmm_do_lend(const controller_info_t *ctrl, size_t size,
                 unsigned long flags, obmm_handle_t *handle)
{
    if (!ctrl || !handle || !ctrl->valid) {
        LOG_ERROR("invalid controller info or handle");
        return -EINVAL;
    }

    /* Validate 2MB alignment */
    const size_t alignment = 2 * 1024 * 1024;
    if (size % alignment != 0) {
        LOG_ERROR("size %zu not 2MB aligned", size);
        return -EINVAL;
    }

    obmm_handle_init(handle);

    /* Setup descriptor - copy deid from controller */
    obmm_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    memcpy(desc.deid, ctrl->eid, 16);
    desc.size = size;
    desc.flags = flags;

    LOG_DEBUG("calling obmm_export with deid=" EID_FMT64 ", size=%zu",
              EID_ARGS64(desc.deid), size);

    /* Call libobmm API - kernel allocates memory */
    mem_id id = obmm_export(&desc, size, flags);
    if (id == OBMM_INVALID_MEMID) {
        LOG_ERROR("obmm_export failed");
        return -errno;
    }

    handle->id = id;
    handle->desc = desc;
    handle->is_export = true;
    handle->initialized = true;

    LOG_INFO("lend success, mem_id=%lu, addr=0x%lx",
             (unsigned long)id, (unsigned long)desc.addr);

    return 0;
}

int obmm_do_borrow(const controller_info_t *ctrl, uint64_t remote_addr, size_t size,
                   unsigned long flags, obmm_handle_t *handle)
{
    if (!ctrl || !handle || !ctrl->valid) {
        LOG_ERROR("invalid controller info or handle");
        return -EINVAL;
    }

    /* Validate 2MB alignment */
    const size_t alignment = 2 * 1024 * 1024;
    if (size % alignment != 0) {
        LOG_ERROR("size %zu not 2MB aligned", size);
        return -EINVAL;
    }

    obmm_handle_init(handle);

    /* Setup descriptor - copy seid and scna from controller */
    obmm_desc_t desc;
    memset(&desc, 0, sizeof(desc));
    memcpy(desc.seid, ctrl->eid, 16);
    desc.scna = ctrl->primary_cna;
    desc.addr = remote_addr;  /* Remote physical address */
    desc.size = size;
    desc.flags = flags;

    LOG_DEBUG("calling obmm_import with seid=" EID_FMT64 ", scna=0x%x, addr=0x%lx",
              EID_ARGS64(desc.seid), desc.scna, (unsigned long)desc.addr);

    /* Call libobmm API */
    mem_id id = obmm_import(&desc, size, flags);
    if (id == OBMM_INVALID_MEMID) {
        LOG_ERROR("obmm_import failed");
        return -errno;
    }

    handle->id = id;
    handle->desc = desc;
    handle->is_export = false;
    handle->initialized = true;

    LOG_INFO("borrow success, mem_id=%lu", (unsigned long)id);

    return 0;
}

int obmm_do_unlend(mem_id id, unsigned long flags)
{
    if (id == OBMM_INVALID_MEMID) {
        LOG_ERROR("invalid mem_id");
        return -EINVAL;
    }

    int ret = obmm_unexport(id, flags);
    if (ret != 0) {
        LOG_ERROR("obmm_unexport failed for mem_id=%lu", (unsigned long)id);
        return -errno;
    }

    LOG_INFO("unlend success, mem_id=%lu", (unsigned long)id);
    return 0;
}

int obmm_do_unborrow(mem_id id, unsigned long flags)
{
    if (id == OBMM_INVALID_MEMID) {
        LOG_ERROR("invalid mem_id");
        return -EINVAL;
    }

    int ret = obmm_unimport(id, flags);
    if (ret != 0) {
        LOG_ERROR("obmm_unimport failed for mem_id=%lu", (unsigned long)id);
        return -errno;
    }

    LOG_INFO("unborrow success, mem_id=%lu", (unsigned long)id);
    return 0;
}

int obmm_query_mem_status(mem_id id, obmm_desc_t *desc)
{
    if (id == OBMM_INVALID_MEMID || !desc) {
        return -EINVAL;
    }

    return obmm_query_status(id, desc);
}

void obmm_handle_cleanup(obmm_handle_t *handle)
{
    if (!handle || !handle->initialized) {
        return;
    }

    if (handle->is_export) {
        obmm_do_unlend(handle->id, 0);
    } else {
        obmm_do_unborrow(handle->id, 0);
    }

    handle->initialized = false;
    handle->id = OBMM_INVALID_MEMID;
}