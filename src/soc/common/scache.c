/*
 * $Id: scache.c 1.31.14.8 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * Purpose: Level 2 Warm Boot cache management    
 *
 *
 */

/* upgrade:  should separate out from scache/stable */
#include <shared/switch.h> /* for _SHR_SWITCH_STABLE_*  */

#ifdef BCM_ESW_SUPPORT
/* upgrade: remove for phase1*/
#include <soc/mem.h> /* soc_mem_entry_bytes */
#endif

#include <shared/alloc.h>

#include <soc/scache.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/cm.h>
#include <soc/types.h>


/* Next phase work:  (search for upgrade)
 *  1. remove knowledge of specific stable types/locations from scache.
 *  1.1. remove stable.location, index_min, index_max
 *  1.2. move stable into scache state entirely
 *  1.3. remove shared/switch.h include..
 *  1.4. remove SHR_SWITCH_STABLE_APPLICATION, SHR_SWITCH_STABLE_*?
 *  1.5. move SOC_STABLE_BASIC, SOC_STABLE_* to specific impls
 *  2. enable dirty bit support
 *  
 */


#ifdef BCM_WARM_BOOT_SUPPORT

typedef struct soc_stable_s {
    VOL uint32  location;    /* Storage location (Level 2 WB) */
    VOL uint32  size;        /* Storage size (Level 2 WB)     */
    VOL uint32  used;        /* Storage usage (Level 2 WB)    */
    VOL uint32  index_min;   /* Min index if using chip memory (Level 2 WB) */
    VOL uint32  index_max;   /* Max index if using chip memory (Level 2 WB) */
    VOL uint32  flags;       /* Flags for storage attributes (Level 2 WB) */
    soc_read_func_t rf;      /* Read function (Level 2 WB)    */
    soc_write_func_t wf;     /* Write function (Level 2 WB)   */
    soc_alloc_func_t alloc_f;/* To allocate per scache handle,
                              * buffer must be persistent for life
                              * of process */
    soc_free_func_t  free_f; /* To free per scache handle     */
} soc_stable_t;


/* soc_stable: per-unit storage table structure for level 2 warm boot */
/* not externally visible state; use accessors */
/* upgrade:  move into scache state */
static soc_stable_t    soc_stable[SOC_MAX_NUM_DEVICES];

#define SOC_STABLE(unit)                (&soc_stable[unit])


/* Read/Write stable accessors must be configured for scache to function */
#define STABLE_CONFIGURED(stable_) \
  (!(((stable_)->rf == NULL) || ((stable_)->wf == NULL)))

#define RETURN_IF_STABLE_NOT_CONFIGURED(unit_) \
  if (!STABLE_CONFIGURED(SOC_STABLE(unit_))) { return SOC_E_CONFIG; }


/*
 *  In the persistent storage, the data will be stored in the following format:
 *  Each entry will be preceded by an scache_descriptor, the last discriptor 
 *  will contain null information.
 *   --------------------------------------------------------------------------
 *  |  Cache guard  1 (32 bit) <--- Starting offset zero                       |
 *  |  Cache handle 1 (32 bit)                                                 |
 *  |  Cache size   1 (32 bit)                                                 |
 *  |  Module data  1 (size = Cache size 1)                                    |
 *  |  Cache guard  2 (32 bit) <--- Starting offset = sum(cache sizes + 2 * 32)|
 *  |  Cache handle 2 (32 bit)                                                 |
 *  |  Cache size   2 (32 bit)                                                 |
 *  |  Module data  2 (size = Cache size 2)                                    |
 *  |  .                                                                       |
 *  |  .                                                                       |
 *  |  Cache guard  N (32 bit) <--- Starting offset = sum(cache sizes + 2 * 32)|
 *  |  Cache handle N (32 bit)                                                 |
 *  |  Cache size   N (32 bit)                                                 |
 *  |  Module data  N (size = Cache size)                                      |
 *  |  Cache guard  N+1                                                        |
 *  |  Cache handle N+1 ~0    - End of list                                    |
 *  |  Cache size   N+1 0     - End of list                                    |
 *  |  Module data  N+1 Empty - end of list                                    |
 *   --------------------------------------------------------------------------
 *  The format for each individual module data is module-specific.
 *  The functions here are oblivious of the module-specific formats.
 *  The size of the cache equals the size of the stable, which must be selected
 *  before using the functions below.
 *  On Warm Boot initialization, the entire blob will be DMA'ed.
 */
typedef struct scache_descriptor_s {
    uint32  guard;
    uint32  handle;
    uint32  size;
} scache_descriptor_t;

#define DESCR_GUARD  0xba5eba11
#define DESCRIPTOR_IS_VALID(sd_)   ((sd_)->guard == DESCR_GUARD)
#define DESCRIPTOR_IS_NULL(sd_)    ((sd_)->size == 0)


/* compute the size to allocated from stable/persistent storage */
#define SCACHE_HANDLE_ALLOC_SIZE(sz)  ((sz) + sizeof(scache_descriptor_t))

/* Compute the size requested by the caller from the size in
 * stable/persistent storage */
#define SCACHE_HANDLE_CALLER_SIZE(sz) ((sz) - sizeof(scache_descriptor_t))

/* Compute pointer to the caller data from the start of an scache entry */
#define SCACHE_HANDLE_CALLER_PTR(ptr) ((uint8 *)(ptr) + sizeof(scache_descriptor_t))

static void
scache_descriptor_t_init(scache_descriptor_t *sd, uint32 size, uint32 handle) 
{
    sal_memset(sd, 0, sizeof(*sd));
    sd->guard  = DESCR_GUARD;
    sd->handle = handle;
    sd->size   = size;
}

#define SCACHE_HANDLE_ID_INVALID      ~0
#define SCACHE_HANDLE_STATE_DIRTY      0x1

typedef struct scache_handle_state_s {
    soc_scache_handle_t handle; /* BCM API module, e.g., BCM_MODULE_VLAN */
    sal_mutex_t         lock;   /* lock access to buffer */
    void  *buf;                 /* Starting pointer for this cache's desc */
    uint32 size;                /* Total cache handle size =
                                 *  (descriptor_size + requested_size) */
    uint32 flags;
    uint32 offset;              /* Position of this handle in persistent
                                 * storage */
    struct scache_handle_state_s *next; /* Next pointer */
} scache_handle_state_t;

typedef struct scache_state_s {
    scache_handle_state_t *head;
    scache_descriptor_t    null_descriptor;
    uint32                 flags;
    uint32                 last_offset;
} scache_state_t;

static scache_state_t scache_state[SOC_MAX_NUM_DEVICES];


/* clean up scache_handle_state resources */
/* unit agnostic */
static void
scache_handle_state_destroy(scache_handle_state_t *hs,
                            soc_stable_t *stable)
{
    if (hs->lock) {
        sal_mutex_destroy(hs->lock);
    }
    if (stable && hs->buf) {
        stable->free_f(hs->buf);
    }
}

/* scache_handle_state factory.  
 * Allocates associated stable buffer, if available and initializes state */
/* unit agnostic */
static int
scache_handle_state_create(soc_stable_t *stable,
                           soc_scache_handle_t handle_id,
                           uint32 alloc_size,
                           scache_handle_state_t **handle_state)
{
    scache_handle_state_t *hs;
    scache_descriptor_t *descriptor;

    hs = sal_alloc(sizeof(*hs), "WB cache info");

    if (hs == NULL) {
        return SOC_E_MEMORY;
    }

    hs->lock = sal_mutex_create("wb handle lock");

    if (stable) {
        hs->buf = stable->alloc_f(alloc_size);
    } else {
        hs->buf = NULL;
    }

    if (hs->lock == NULL || (stable && (hs->buf == NULL))) {
        scache_handle_state_destroy(hs, stable);
        sal_free(hs);
        return SOC_E_MEMORY;
    }

    hs->flags  = 0;
    hs->offset = 0;
    hs->size   = alloc_size;
    hs->handle = handle_id;

    /* initalize the descriptor in scache */
    if (hs->buf) {
        descriptor = (scache_descriptor_t*)hs->buf;
        scache_descriptor_t_init(descriptor, 
                                 SCACHE_HANDLE_CALLER_SIZE(alloc_size), 
                                 handle_id);
    }

    if (handle_state) {
        *handle_state = hs;
    }

    /*    coverity[leaked_storage : FALSE]    */
    return SOC_E_NONE;
}

/* Find an scache_handle_state object with the given handle */
/* unit agnostic */
static int
scache_handle_state_find(scache_handle_state_t *head, 
                         soc_scache_handle_t handle,
                         scache_handle_state_t **hs)
{
    while (head && (head->handle != handle)) {
        head = head->next;
    }
    
    if (head == NULL) {
        return SOC_E_NOT_FOUND;
    }
    if (hs) { *hs = head; }

    return SOC_E_NONE;
}



/* recursively dump the scache info list from any given node */
#define SCACHE_HANDLE_P_HEADER "%4s %10s %10s %10s %10s %10s %6s\n", \
           "idx", "handle", "offset", "size", "cache", "lock", "flags"
#define SCACHE_HANDLE_P_FORMAT "%4d 0x%08x 0x%08x 0x%08x %p %p 0x%04x"

static void 
soc_scache_dump_handle_state(int idx, scache_handle_state_t *sc)
{
    scache_descriptor_t *desc;
    if (sc == NULL)   return;
    
    desc = (scache_descriptor_t*)sc->buf;

    soc_cm_print(SCACHE_HANDLE_P_FORMAT, 
                 idx, sc->handle, sc->offset, sc->size, sc->buf,
                 (void *)sc->lock, sc->flags);

    /* validate the soft state matches stored state */
    if (desc->handle != sc->handle) {
        soc_cm_print("* HANDLE MISMATCH: 0x%08x * ", desc->handle);
    }
    if (desc->size != sc->size) {
        soc_cm_print("* SIZE MISMATCH: 0x%08x * ", desc->size);
    }
    soc_cm_print("\n");

    soc_scache_dump_handle_state(idx+1, sc->next);

    return;
}

void 
soc_scache_dump_state(int unit)
{
    if (!SOC_UNIT_NUM_VALID(unit)) {
        soc_cm_print("invalid unit: %d\n", unit);
        return;
    }

    soc_cm_print("scache info: flags=0x%04x last_offset=0x%08x\n",
                 scache_state[unit].flags, scache_state[unit].last_offset);
    soc_cm_print("scache handles:\n");
    soc_cm_print(SCACHE_HANDLE_P_HEADER);
    soc_scache_dump_handle_state(0, scache_state[unit].head);

    return;
}

/* insert an scache_handle_state into the scache list for this unit */
static int
scache_handle_list_insert(int unit, scache_handle_state_t *hs)
{
    hs->offset = scache_state[unit].last_offset;
    
    /* head insert */
    hs->next = scache_state[unit].head;
    scache_state[unit].head = hs;

    scache_state[unit].last_offset += hs->size;
    return SOC_E_NONE;
}

/* Clear the stable info structure for a newly attached unit. */
void
soc_stable_attach(int unit)
{
    sal_memset(SOC_STABLE(unit), 0, sizeof(*(SOC_STABLE(unit))));
}

/* Allocate the global block of memory for persistent storage synchronization */
/* This is called by soc_stable_size_set() during cold boot */
int
soc_scache_init(int unit, uint32 size, uint32 flags)
{
    if (flags) {
        /* flags no longer needed */
        return SOC_E_PARAM;
    }
    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    if (size == 0) {
        return SOC_E_NONE;
    }

    scache_state[unit].head = NULL;
    scache_state[unit].flags = flags;
    scache_state[unit].last_offset = 0;

    /* The null descriptor determines the end of list in persistent storage.
     * Its offset is updated for each handle allocation,
     * cleared on init & detach
     */
    scache_descriptor_t_init(&scache_state[unit].null_descriptor,
                             0, SCACHE_HANDLE_ID_INVALID);

    return SOC_E_NONE;
}

/* Free the scache and the meta-data */
int
soc_scache_detach(int unit)
{
    scache_handle_state_t *current = NULL, *temp = NULL;
    soc_stable_t *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    RETURN_IF_STABLE_NOT_CONFIGURED(unit);

    stable = SOC_STABLE(unit);

    current = scache_state[unit].head;
    while (current) {
        temp = current;
        current = current->next;

        scache_handle_state_destroy(temp, stable);
        sal_free(temp);
    }
    scache_state[unit].head = NULL;
    scache_state[unit].last_offset = 0;

    scache_descriptor_t_init(&scache_state[unit].null_descriptor,
                             0, SCACHE_HANDLE_ID_INVALID);

    return SOC_E_NONE;
}

/* Carve out a block of memory from the global block */ 
/* This is called by each module that requires persistent storage during 
   cold boot */
int
soc_scache_alloc(int unit, soc_scache_handle_t handle_id, uint32 size)
{
    int rv;
    int stable_size, alloc_size;
    scache_handle_state_t *hs;
    soc_stable_t *stable;

    /* Don't allow the invalid ID as a handle.  This will be used to
     * determine future versioning of the stable format, if needed.
     */
    if (handle_id == SCACHE_HANDLE_ID_INVALID) {
        return SOC_E_PARAM;
    }

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    RETURN_IF_STABLE_NOT_CONFIGURED(unit);

    /* make sure this handle doesn't already exist */
    rv = scache_handle_state_find(scache_state[unit].head, handle_id, NULL);
    if (SOC_SUCCESS(rv)) {
        return SOC_E_EXISTS;
    }
    if (rv != SOC_E_NOT_FOUND) {
        return rv;
    }
    rv = SOC_E_NONE;

    stable = SOC_STABLE(unit);

    SOC_SCACHE_ALIGN_SIZE(size);

    /* allocate the requested size, plus a descriptor 
     * from persistent storage */
    alloc_size = SCACHE_HANDLE_ALLOC_SIZE(size);

    /* exclude the null-descriptor from the total available size */
    SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
    stable_size -= sizeof(scache_state[unit].null_descriptor);

    if ((alloc_size + scache_state[unit].last_offset) > stable_size) {
        soc_scache_dump_state(unit);
        soc_cm_print("Scache on unit %d exhausted.\n", unit);        
        return SOC_E_RESOURCE;
    }

    SOC_IF_ERROR_RETURN(
        scache_handle_state_create(stable, handle_id, alloc_size, &hs));
    
    SOC_IF_ERROR_RETURN(
        scache_handle_list_insert(unit, hs));

    
    stable->used += alloc_size;

#if BROADCOM_DEBUG
    if (SOC_DEBUG_CHECK(SOC_DBG_VVERBOSE)) {
        soc_cm_print("%s: allocated handle=0x%x size=0x%08x\n", 
                     FUNCTION_NAME(), handle_id, size);
        soc_scache_dump_state(unit);
    }
#endif
    return SOC_E_NONE;
}

/* ReCarve out a block of memory from the global block */ 
/* This is called by each module that requires persistent storage to be increased/decreased during 
   warm boot due to scache version change */
int
soc_scache_realloc(int unit, soc_scache_handle_t handle_id, int32 incr_size)
{
    int rv;
    scache_handle_state_t *hs;

    /* Don't allow the invalid ID as a handle.  This will be used to
     * determine future versioning of the stable format, if needed.
     */
    if (handle_id == SCACHE_HANDLE_ID_INVALID) {
        return SOC_E_PARAM;
    }

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    RETURN_IF_STABLE_NOT_CONFIGURED(unit);

    /* make sure this handle doesn't already exist */
    rv = scache_handle_state_find(scache_state[unit].head, handle_id, &hs);
    if (SOC_SUCCESS(rv)) {
        soc_stable_t *stable;
         /* Realloc*/
        stable = SOC_STABLE(unit);
 
        if (stable != NULL) {
            incr_size += hs->size;
            SOC_SCACHE_ALIGN_SIZE(incr_size);
        
            stable->free_f(hs->buf);
            hs->buf = stable->alloc_f(incr_size);

            if (hs->buf == NULL) {
                return SOC_E_MEMORY;
            } else {
                scache_handle_state_t *hs_ptr;
                scache_descriptor_t *descriptor;

                hs_ptr = scache_state[unit].head;

                stable->used -= hs->size;
                stable->used += incr_size;
                while(hs_ptr) {
                    if (hs_ptr->offset > hs->offset) {
                        hs_ptr->offset += (incr_size - hs->size);
                    }
                    hs_ptr = hs_ptr->next;
                }
                scache_state[unit].last_offset += (incr_size - hs->size);
                hs->size = incr_size;
                descriptor = (scache_descriptor_t*)hs->buf;
                scache_descriptor_t_init(descriptor, 
                                      SCACHE_HANDLE_CALLER_SIZE(hs->size), 
                                      handle_id);
            }
        }
    } else {
        return rv;
    }


#if BROADCOM_DEBUG
    if (SOC_DEBUG_CHECK(SOC_DBG_VVERBOSE)) {
        soc_cm_print("%s: allocated handle=0x%x incr_size=0x%08x\n", 
                     FUNCTION_NAME(), handle_id, incr_size);
        soc_scache_dump_state(unit);
    }
#endif
    return SOC_E_NONE;
}

/* Recover the global blob of memory from persistent storage - called by the
   SOC layer during warm boot */
int
soc_scache_recover(int unit)
{
    int size;
    uint32 offset;
    soc_stable_t *stable;
    scache_handle_state_t *handle_state;
    scache_descriptor_t desc;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }

    SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &size));
    if (size == 0) {
        return SOC_E_NONE;
    }

    stable = SOC_STABLE(unit);
    RETURN_IF_STABLE_NOT_CONFIGURED(unit);

    /* Remove any latent soft-scache descriptors.  This should only
     * happen during a simulated warmboot
     */
    soc_scache_detach(unit);

    /* Read each handle from persistent storage into scache until 
     * the null descriptor is found
     */
    offset = 0;
    SOC_IF_ERROR_RETURN(stable->rf(unit,(uint8*)&desc, offset, sizeof(desc)));
    while (!DESCRIPTOR_IS_NULL(&desc)) {

        /* Create a handle_state entry representing this scache handle */
        SOC_IF_ERROR_RETURN(
            scache_handle_state_create(stable, desc.handle, 
                                       SCACHE_HANDLE_ALLOC_SIZE(desc.size),
                                       &handle_state));

        handle_state->flags |= SCACHE_HANDLE_STATE_DIRTY;
        SOC_IF_ERROR_RETURN(
            scache_handle_list_insert(unit, handle_state));

        /* Read the entire entry into the newly allocated handle */
        SOC_IF_ERROR_RETURN(
            stable->rf(unit, handle_state->buf, offset, handle_state->size));
        offset += handle_state->size;

        
        stable->used += handle_state->size;

        /* get the next descriptor */
        SOC_IF_ERROR_RETURN(stable->rf(unit, (uint8*)&desc, offset, sizeof(desc)));
    }

#if BROADCOM_DEBUG
    if (SOC_DEBUG_CHECK(SOC_DBG_VERBOSE)) {
        soc_cm_print("%s: Recovered scache:\n", FUNCTION_NAME());
        soc_scache_dump_state(unit);
    }
#endif
    return SOC_E_NONE;
}
 
/* Get the pointer to the module's area - called by each BCM module during
   initialization */
int
soc_scache_ptr_get(int unit, soc_scache_handle_t handle, uint8 **ptr, 
                   uint32 *size)
{
    scache_handle_state_t *hs;
    
    if (ptr == NULL) {
        return SOC_E_PARAM;
    }

    /* Obtain the cache information */
    SOC_IF_ERROR_RETURN(
        scache_handle_state_find(scache_state[unit].head, handle, &hs));

    *ptr  = SCACHE_HANDLE_CALLER_PTR(hs->buf);
    *size = SCACHE_HANDLE_CALLER_SIZE(hs->size);
    return SOC_E_NONE;
}

/* Write function called by bcmSwitchControlSync after all modules update
*  their scache info. */
int
soc_scache_commit(int unit)
{
    int size, dirty = 0;
    scache_handle_state_t *current;
    soc_stable_t *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    RETURN_IF_STABLE_NOT_CONFIGURED(unit);
    SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &size));

    if (size == 0) {
        return SOC_E_NONE;
    }

    stable = SOC_STABLE(unit);

    current = scache_state[unit].head;
    while (current) {

        sal_mutex_take(current->lock, sal_mutex_FOREVER);
/* upgrade: enable */
#ifdef USE_SCACHE_DIRTY_BIT
        if (current->flags & SCACHE_HANDLE_STATE_DIRTY)
#endif
        {
            /* Copy to persistent storage from cache */
            stable->wf(unit, (uint8*)current->buf, 
                       (int)current->offset,
                       (int)(current->size));
            current->flags &= ~SCACHE_HANDLE_STATE_DIRTY;
            dirty = 1;
        }
        sal_mutex_give(current->lock);
        
        current = current->next;
    }

    if (dirty) {
        stable->wf(unit, (uint8*)&scache_state[unit].null_descriptor,
                   scache_state[unit].last_offset,
                   sizeof(scache_state[unit].null_descriptor));
    }

    return SOC_E_NONE;
}


int
soc_scache_handle_lock(int unit, soc_scache_handle_t handle)
{
    scache_handle_state_t *hs;

    SOC_IF_ERROR_RETURN(
        scache_handle_state_find(scache_state[unit].head, handle, &hs));

    sal_mutex_take(hs->lock, sal_mutex_FOREVER);
    hs->flags |= SCACHE_HANDLE_STATE_DIRTY;

    return SOC_E_NONE;
}

int
soc_scache_handle_unlock(int unit, soc_scache_handle_t handle)
{
    scache_handle_state_t *hs;

    SOC_IF_ERROR_RETURN(
        scache_handle_state_find(scache_state[unit].head, handle, &hs));

    sal_mutex_give(hs->lock);
    return SOC_E_NONE;
}


int
soc_stable_set(int unit, int arg, uint32 flags)
{
    soc_stable_t    *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    stable = SOC_STABLE(unit);

    if (flags) {
        uint32 valid;
        /*
         * Only SOC_STABLE_BASIC and SOC_STABLE_DIFFSERV are valid.
         * SOC_STABLE_DIFFSERV can only be set for non-basic.
         */
        valid = SOC_STABLE_BASIC | SOC_STABLE_DIFFSERV;
        if ((flags & ~valid) ||
            ((flags & SOC_STABLE_BASIC) && (flags & SOC_STABLE_DIFFSERV))) {
            return SOC_E_PARAM;
        }
    }
    stable->flags = flags;

    /* upgrade: move to specific impls */
    switch (arg) {
    case _SHR_SWITCH_STABLE_DEVICE_NEXT_HOP:
#ifdef BCM_XGS3_SWITCH_SUPPORT
        stable->wf = soc_scache_esw_nh_mem_write;
        stable->rf = soc_scache_esw_nh_mem_read;
#else
        return SOC_E_PARAM;
#endif /* BCM_ESW_SUPPORT */
        /* Fall through */
    case _SHR_SWITCH_STABLE_NONE:
    case _SHR_SWITCH_STABLE_DEVICE_EXT_MEM:
    case _SHR_SWITCH_STABLE_APPLICATION:
        stable->location = arg;
        break;
    default:
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

int
soc_stable_get(int unit, int *arg, uint32 *flags)
{
    soc_stable_t    *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    stable = SOC_STABLE(unit);

    *arg = stable->location;
    *flags = stable->flags;
    return SOC_E_NONE;
}

int
soc_stable_size_set(int unit, int arg)
{
    soc_stable_t    *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }

    if (arg < 0) {
        return SOC_E_PARAM;
    }
    stable = SOC_STABLE(unit);

    stable->size = arg;

    if (0 == stable->size) {
        /* Deallocate existing stable storage */
        soc_scache_detach(unit);
        return SOC_E_NONE;
    }

    if (stable->location == _SHR_SWITCH_STABLE_DEVICE_NEXT_HOP) {
        /* upgrade: move to specific stable impl */
#ifdef BCM_XGS3_SWITCH_SUPPORT
        int num_entries, bytes0, bytes1, bytes2;
        void* soc_scache_defl_alloc(uint32 sz);
        soc_persist_t *sop = SOC_PERSIST(unit);
        stable->wf = soc_scache_esw_nh_mem_write;
        stable->rf = soc_scache_esw_nh_mem_read;
        stable->alloc_f = soc_scache_defl_alloc;
        stable->free_f = sal_free; 

        bytes0 = soc_mem_entry_bytes(unit, INITIAL_ING_L3_NEXT_HOPm);
        bytes1 = soc_mem_entry_bytes(unit, ING_L3_NEXT_HOPm);
        bytes2 = soc_mem_entry_bytes(unit, EGR_L3_NEXT_HOPm);
        num_entries = stable->size / (bytes2 - 1) + 1;
        stable->index_max = soc_mem_index_max(unit, ING_L3_NEXT_HOPm);
        stable->index_min = stable->index_max - num_entries + 1;

        sop->memState[INITIAL_ING_L3_NEXT_HOPm].index_max = 
            stable->index_min - 1;
        sop->memState[ING_L3_NEXT_HOPm].index_max = stable->index_min - 1;
        sop->memState[EGR_L3_NEXT_HOPm].index_max = stable->index_min - 1;
#else
        return SOC_E_PARAM;
#endif /* BCM_ESW_SUPPORT */
    }

    SOC_IF_ERROR_RETURN(soc_scache_init(unit, stable->size, 0));

    return SOC_E_NONE;
}

int
soc_stable_size_get(int unit, int *arg)
{
    soc_stable_t    *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    stable = SOC_STABLE(unit);

    *arg = stable->size;

    return SOC_E_NONE;
}

int
soc_stable_used_get(int unit, int *arg)
{
    soc_stable_t    *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }
    stable = SOC_STABLE(unit);

    *arg = stable->used;
    return SOC_E_NONE;
}


/* upgrade: temporary  - shd be removed when scache upgrade is complete */
int
soc_stable_tmp_access(int unit, soc_stable_field_t field, 
                      uint32 *val, int get)
{
#ifdef SCACHE_PHASE1_UPGRADE_COMPLETE
    return SOC_E_UNAVAIL;
#else
    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }

    switch (field) {
    case sf_index_min:
        if (get) { *val = SOC_STABLE(unit)->index_min; }
        else     { SOC_STABLE(unit)->index_min = *val; }
        break;
    case sf_index_max:
        if (get) { *val = SOC_STABLE(unit)->index_max; }
        else     { SOC_STABLE(unit)->index_max = *val; }
        break;
    default:
        return SOC_E_PARAM;
    }
#endif /* SCACHE_PHASE1_UPGRADE_COMPLETE */

    return SOC_E_NONE;
}

/* upgrade: temporary  - shd be removed when scache upgrade is complete */
int
soc_stable_tmp_flags_get(int unit)
{
    if (!SOC_UNIT_NUM_VALID(unit)) {
        return 0;
    }
    return SOC_STABLE(unit)->flags;
}

/* default allocator */
void*
soc_scache_defl_alloc(uint32 sz)
{
    return sal_alloc(sz, "wb handle");
}

int 
soc_switch_stable_register(int unit, soc_read_func_t rf, 
                           soc_write_func_t wf,
                           soc_alloc_func_t alloc_f,
                           soc_free_func_t free_f)
{

    soc_stable_t    *stable;

    if (!SOC_UNIT_NUM_VALID(unit)) {
        return SOC_E_UNIT;
    }

    if ((NULL == rf) || (NULL == wf)) {
        return SOC_E_PARAM;
    }

    stable = SOC_STABLE(unit);
    stable->rf = rf;
    stable->wf = wf;
    stable->alloc_f = alloc_f ? alloc_f : soc_scache_defl_alloc;
    stable->free_f  = free_f ? free_f : sal_free;

    return SOC_E_NONE;
}

#else /* BCM_WARM_BOOT_SUPPORT */
int _scache_not_empty;
#endif
