/*
 * $Id: shr_resmgr.c 1.9 Broadcom SDK $
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
 * Global resource allocator
 */

#include <sal/core/sync.h>
#include <soc/cm.h>
#include <shared/shr_resmgr.h>

#include <bcm/error.h>
#include <bcm/debug.h>

/*****************************************************************************/
/*
 *  Internal implementation
 */

#define RES_MESSAGE_ENABLE TRUE
#define RES_EXCESS_VERBOSITY TRUE
#if 0 
#undef BCM_DEBUG
#define BCM_DEBUG(flags, stuff)   soc_cm_print stuff
#endif 
#if RES_MESSAGE_ENABLE
#define RES_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_API, stuff)
#else /* RES_MESSAGE_ENABLE */
#define RES_DEBUG(flags, stuff)
#endif /* RES_MESSAGE_ENABLE */
#define RES_OUT(stuff)           RES_DEBUG(BCM_DBG_API, stuff)
#define RES_WARN(stuff)          RES_DEBUG(BCM_DBG_WARN, stuff)
#define RES_ERR(stuff)           RES_DEBUG(BCM_DBG_ERR, stuff)
#define RES_VERB(stuff)          RES_DEBUG(BCM_DBG_VERBOSE, stuff)
#define RES_VVERB(stuff)         RES_DEBUG(BCM_DBG_VVERBOSE, stuff)
#if RES_EXCESS_VERBOSITY
#define RES_EVERB(stuff)         RES_DEBUG(BCM_DBG_VVERBOSE, stuff)
#else /* RES_EXCESS_VERBOSITY */
#define RES_EVERB(stuff)
#endif /* RES_EXCESS_VERBOSITY */
/* Please do not change the following seven lines */
#if defined(__GNUC__) && !defined(__PEDANTIC__)
#define RES_MSG(string) "%s[%d]%s" string, __FILE__, __LINE__, __FUNCTION__
#define RES_MSG1(string) "%s[%d]%s: " string, __FILE__, __LINE__, __FUNCTION__
#else /* defined(__GNUC__) && !defined(__PEDANTIC__) */
#define RES_MSG(string) "%s[%d]" string, __FILE__, __LINE__
#define RES_MSG1(string) "%s[%d]: " string, __FILE__, __LINE__
#endif /* defined(__GNUC__) && !defined(__PEDANTIC__) */

/*
 *  This describes how a single resource maps to an underlying pool.
 *
 *  resPoolId is the ID of the resource pool on this unit from which this
 *  resource is drawn.
 *
 *  resElemSize is the number of elements in the specified resource pool that
 *  must be taken to make a single element of this resource.  Basically, any
 *  alloc/free of this resource will multiply the number of elements by this
 *  value to determine how many to alloc/free of the underlying pool.
 *
 *  name is a string that names this resource.  It is used only for diagnostic
 *  purposes.  Internally, the provided name will be copied to the same cell as
 *  the structure, so the name array is really variable length.
 */
typedef struct _shr_res_type_desc_s {
    int resPoolId;              /* unit specific resource pool ID */
    int resElemSize;            /* how many elems of this pool per this res */
    int refCount;               /* number of elements allocated currently */
    char name[1];               /* descriptive name (for diagnostics) */
} _shr_res_type_desc_t;

/*
 *  This describes a single resource pool on a unit.
 *
 *  resManagerType is the ID of the reousrce manager that will be used to
 *  manage this resource pool on this unit.
 *
 *  low is the minimum valid element of this resource.
 *
 *  count is the number of valid elements of this resource.
 *
 *  extras points to a struct (which will be appended to this during setup of
 *  this pool) that provides additional arguments to how the pool needs to be
 *  managed.  This is provided because some of the supported allocation
 *  managers require more information than just the range of valid IDs.
 *
 *  name is a string that names this resource.  It is used only for diagnostic
 *  purposes.  Internally, the provided name will be copied to the same cell as
 *  the structure, so the name array is really variable length.
 *
 *  Note that the extras struct will be appended to the same memory cell as
 *  this struct, after the name, and so the pointer will not need to be freed
 *  when this is destroyed.
 */
typedef struct _shr_res_pool_desc_s {
    shr_res_allocator_t resManagerType; /* which resoource manager to use */
    int low;                            /* minimum available element */
    int count;                          /* number of available elements */
    int refCount;                       /* number of types using this pool */
    void *resHandle;                    /* handle for this resource */
    void *extras;                       /* additional config per resmgr type */
    char name[1];                       /* descriptive name for diagnostics */
} _shr_res_pool_desc_t;

/*
 *  This structure describes attributes about the unit, and includes lists
 *  that are applicable to the unit.  Note that while resource IDs and resource
 *  types are unit-specific, the resource managers themselves are globally
 *  available to all units.
 *
 *  resTypeCount indicates how many different resources on this unit are being
 *  managed through this system.  It is possible for more than one resource to
 *  use the same resource pool, but it is not possible for one resource to use
 *  more than one resource pool.
 *
 *  resPoolCount indicates how many different resource pools are being managed
 *  on this unit.
 *
 *  res points to an array of resource descriptors,  Each descriptor will map a
 *  single resource on the unit to its underlying resource pool.
 *
 *  pool points to an array of resource pool descriptors.  Each of these will
 *  describe a single resource pool on the unit and map to the manager that
 *  will be used for that particular pool.
 */
typedef struct _shr_res_unit_desc_s {
    uint16 resTypeCount;          /* maximum presented resource ID */
    uint16 resPoolCount;          /* maximum resource pool ID */
    _shr_res_type_desc_t **res;   /* array of type -> pool map pointers */
    _shr_res_pool_desc_t **pool;  /* array of pool description pointers */
} _shr_res_unit_desc_t;

/*
 *  This structure is global, and points to the information about all units.
 *
 *  For each unit, it's just a pointer here, since this reduces the overall
 *  memory footprint for the case of units that do not use this mechanism, plus
 *  it allows each unit to use only as much memory as needed to describe its
 *  resources and pools and how they map.
 */
static volatile sal_mutex_t _g_resLock = NULL;
static _shr_res_unit_desc_t *_g_unitResDesc[BCM_LOCAL_UNITS_MAX];

/*
 *  Various function prototypes per method for the alloc managers.
 */
typedef int (*_shr_res_alloc_create)(_shr_res_pool_desc_t **desc,
                                     int low_id,
                                     int count,
                                     const void *extras,
                                     const char *name);
typedef int (*_shr_res_alloc_destroy)(_shr_res_pool_desc_t *desc);
typedef int (*_shr_res_alloc_alloc)(_shr_res_pool_desc_t *desc,
                                    uint32 flags,
                                    int count,
                                    int *elem);
typedef int (*_shr_res_alloc_align)(_shr_res_pool_desc_t *desc,
                                    uint32 flags,
                                    int align,
                                    int offset,
                                    int count,
                                    int *elem);
typedef int (*_shr_res_alloc_free)(_shr_res_pool_desc_t *desc,
                                   int count,
                                   int elem);
typedef int (*_shr_res_alloc_check)(_shr_res_pool_desc_t *desc,
                                    int count,
                                    int elem);

/*
 *  This structure describes a single allocator mechanism, specifically by
 *  providing a set pointers to functions that are used to manipulate it.
 */
typedef struct _shr_res_alloc_mgr_s {
    _shr_res_alloc_create create;
    _shr_res_alloc_destroy destroy;
    _shr_res_alloc_alloc alloc;
    _shr_res_alloc_align align;
    _shr_res_alloc_free free;
    _shr_res_alloc_check check;
    char *name;
} _shr_res_alloc_mgr_t;

/*
 *  These prototypes are for the global const structure below that points to
 *  all of the various implementations.
 */
static int _shr_res_bitmap_create(_shr_res_pool_desc_t **desc,
                                  int low_id,
                                  int count,
                                  const void* extras,
                                  const char* name);
static int _shr_res_bitmap_destroy(_shr_res_pool_desc_t *desc);
static int _shr_res_bitmap_alloc(_shr_res_pool_desc_t *desc,
                                 uint32 flags,
                                 int count,
                                 int *elem);
static int _shr_res_bitmap_alloc_align(_shr_res_pool_desc_t *desc,
                                       uint32 flags,
                                       int align,
                                       int offs,
                                       int count,
                                       int *elem);
static int _shr_res_bitmap_free(_shr_res_pool_desc_t *desc,
                                int count,
                                int elem);
static int _shr_res_bitmap_check(_shr_res_pool_desc_t *desc,
                                 int count,
                                 int elem);
static int _shr_res_idxres_create(_shr_res_pool_desc_t **desc,
                                  int low_id,
                                  int count,
                                  const void* extras,
                                  const char* name);
static int _shr_res_idxres_destroy(_shr_res_pool_desc_t *desc);
static int _shr_res_idxres_alloc(_shr_res_pool_desc_t *desc,
                                 uint32 flags,
                                 int count,
                                 int *elem);
static int _shr_res_idxres_free(_shr_res_pool_desc_t *desc,
                                int count,
                                int elem);
static int _shr_res_idxres_check(_shr_res_pool_desc_t *desc,
                                 int count,
                                 int elem);
static int _shr_res_aidxres_create(_shr_res_pool_desc_t **desc,
                                   int low_id,
                                   int count,
                                   const void* extras,
                                   const char* name);
static int _shr_res_aidxres_destroy(_shr_res_pool_desc_t *desc);
static int _shr_res_aidxres_alloc(_shr_res_pool_desc_t *desc,
                                  uint32 flags,
                                  int count,
                                  int *elem);
static int _shr_res_aidxres_free(_shr_res_pool_desc_t *desc,
                                 int count,
                                 int elem);
static int _shr_res_aidxres_check(_shr_res_pool_desc_t *desc,
                                  int count,
                                  int elem);
static int _shr_res_mdb_create(_shr_res_pool_desc_t **desc,
                               int low_id,
                               int count,
                               const void* extras,
                               const char* name);
static int _shr_res_mdb_destroy(_shr_res_pool_desc_t *desc);
static int _shr_res_mdb_alloc(_shr_res_pool_desc_t *desc,
                              uint32 flags,
                              int count,
                              int *elem);
static int _shr_res_mdb_free(_shr_res_pool_desc_t *desc,
                             int count,
                             int elem);
static int _shr_res_mdb_check(_shr_res_pool_desc_t *desc,
                              int count,
                              int elem);

/*
 *  Global const structure describing the various allocator mechanisms.
 */
static const _shr_res_alloc_mgr_t _shr_res_alloc_mgrs[SHR_RES_ALLOCATOR_COUNT] =
    {
        {
            _shr_res_bitmap_create,
            _shr_res_bitmap_destroy,
            _shr_res_bitmap_alloc,
            _shr_res_bitmap_alloc_align,
            _shr_res_bitmap_free,
            _shr_res_bitmap_check,
            "SHR_RES_ALLOCATOR_BITMAP"
        } /* bitmap */,
        {
            _shr_res_idxres_create,
            _shr_res_idxres_destroy,
            _shr_res_idxres_alloc,
            NULL,
            _shr_res_idxres_free,
            _shr_res_idxres_check,
            "SHR_RES_ALLOCATOR_IDXRES"
        } /* idxres */,
        {
            _shr_res_aidxres_create,
            _shr_res_aidxres_destroy,
            _shr_res_aidxres_alloc,
            NULL,
            _shr_res_aidxres_free,
            _shr_res_aidxres_check,
            "SHR_RES_ALLOCATOR_AIDXRES"
        } /* aidxres */,
        {
            _shr_res_mdb_create,
            _shr_res_mdb_destroy,
            _shr_res_mdb_alloc,
            NULL,
            _shr_res_mdb_free,
            _shr_res_mdb_check,
            "SHR_RES_ALLOCATOR_MDB"
        } /* mdb */
    };

/*
 *  Note that this takes the global lock, not some unit-specific lock.  This is
 *  because the units are expected to be protected by their own locks,
 *  preventing parallel resource manipulation on the same unit.  It should,
 *  however be safe to manipulate resources on different units in parallel, so
 *  we only use this when init or detach of units, to prevent the unit from
 *  being set up or torn down twice (or, rather more amusingly, having both
 *  happen at the same time).
 *
 *  Since both of these macros issue return in error case, THEY MUST NOT BE
 *  USED WITH TEMPORARY OR NONCOMMITTED RESOURCES ARE IN PLAY OR WHEN AN
 *  OPERATION IS ONLY PARTIALLY COMMITTED.
 */
#define RES_LOCK_TAKE \
    if (!_g_resLock) { \
        RES_ERR((RES_MSG1("no unit has been initialised yet\n"))); \
        return BCM_E_INIT; \
    } \
    if (sal_mutex_take(_g_resLock, sal_sem_FOREVER)) { \
        RES_ERR((RES_MSG1("unable to takee global lock\n"))); \
        return BCM_E_INTERNAL; \
    }
#define RES_LOCK_GIVE \
    if (sal_mutex_give(_g_resLock)) { \
        RES_ERR((RES_MSG1("unable to release global lock\n"))); \
        return BCM_E_INTERNAL; \
    }

/*
 *  Basic checks performed for many functions
 */
#define RES_UNIT_CHECK(_unit, _unitInfo) \
    if ((0 > (_unit)) || (BCM_LOCAL_UNITS_MAX <= (_unit))) { \
        RES_ERR((RES_MSG1("invalid unit number %d\n"), _unit)); \
        return BCM_E_PARAM; \
    } \
    if (!(_g_unitResDesc[_unit])) { \
        RES_ERR((RES_MSG1("unit %d is not initialised\n"), _unit)); \
        return BCM_E_INIT; \
    } \
    (_unitInfo) = _g_unitResDesc[_unit]
#define RES_HANDLE_VALID_CHECK(_handle) \
    if (!(_handle)) { \
        RES_ERR((RES_MSG1("NULL handle is not valid\n"))); \
        return BCM_E_PARAM; \
    }
#define RES_POOL_VALID_CHECK(_handle, _pool) \
    if ((0 > (_pool)) || ((_handle)->resPoolCount <= (_pool))) { \
        RES_ERR((RES_MSG1("%p pool %d does not exist\n"), ((void*)(_handle)), _pool)); \
        return BCM_E_PARAM; \
    }
#define RES_POOL_EXIST_CHECK(_handle, _pool) \
    if (!((_handle)->pool[_pool])) { \
        RES_ERR((RES_MSG1("%p pool %d is not configured\n"), ((void*)(_handle)), _pool)); \
        return BCM_E_CONFIG; \
    }
#define RES_TYPE_VALID_CHECK(_handle, _type) \
    if ((0 > (_type)) || ((_handle)->resTypeCount <= (_type))) { \
        RES_ERR((RES_MSG1("%p resource %d does not exist\n"), ((void*)(_handle)), _type)); \
        return BCM_E_PARAM; \
    }
#define RES_TYPE_EXIST_CHECK(_handle, _type) \
    if (!((_handle)->res[_type])) { \
        RES_ERR((RES_MSG1("%p resource %d is not configured\n"), ((void*)(_handle)), _type)); \
        return BCM_E_CONFIG; \
    }

/*
 *  Destroys all of the resources and then pools for a unit.
 */
static int
_shr_mres_destroy_data(_shr_res_unit_desc_t *unitData)
{
    int i;
    int result = BCM_E_NONE;
    _shr_res_type_desc_t *type;
    _shr_res_pool_desc_t *pool;

    /* destroy resources */
    for (i = 0; i < unitData->resTypeCount; i++) {
        if (unitData->res[i]) {
            
            type = unitData->res[i];
            unitData->res[i] = NULL;
            if (type->refCount) {
                RES_WARN((RES_MSG1("%p type %d (%s): still in use (%d)\n"),
                          (void*)unitData,
                          i,
                          &(type->name[0]),
                          type->refCount));
            }
            unitData->pool[type->resPoolId]->refCount--;
            sal_free(type);
        }
    } /* for (all resources this unit) */

    /* destroy pools */
    for (i = 0;
         (i < unitData->resPoolCount) && (BCM_E_NONE == result);
         i++) {
        if (unitData->pool[i]) {
            
            pool = unitData->pool[i];
            unitData->pool[i] = NULL;
            if (pool->refCount) {
                RES_WARN((RES_MSG1("%p pool %d (%s): unexpectedly still"
                                   " in use (%d) - invalid condition???\n"),
                          (void*)unitData,
                          i,
                          &(pool->name[0]),
                          pool->refCount));
            }
            result = _shr_res_alloc_mgrs[pool->resManagerType].destroy(pool);
            if (BCM_E_NONE != result) {
                RES_ERR((RES_MSG1("%p pool %d (%s): unable to destroy:"
                                  " %d (%s)\n"),
                         (void*)unitData,
                         i,
                         &(pool->name[0]),
                         result,
                         _SHR_ERRMSG(result)));
                unitData->pool[i] = pool;
            } /* if (BCM_E_NONE != result) */
        } /* if (unitData->pool[i]) */
    } /* for (all pools as long as no errors) */
    return result;
}

/*****************************************************************************/
/*
 *  Exposed API implementation (handle based)
 */

/*
 *  Initialise unit
 */
int
shr_mres_create(shr_mres_handle_t *handle,
                int num_res_types,
                int num_res_pools)
{
    _shr_res_unit_desc_t *tempHandle;
    int result = BCM_E_NONE;

    RES_VVERB((RES_MSG("(%p, %d, %d) enter\n"),
              (void*)handle,
              num_res_types,
              num_res_pools));

    /* a little parameter checking */
    if (!handle) {
        RES_ERR((RES_MSG1("obligatory OUT argument must not be NULL\n")));
        result = BCM_E_PARAM;
    }
    if (1 > num_res_pools) {
        RES_ERR((RES_MSG1("resource pools %d; must be > 0\n"), num_res_pools));
        result =  BCM_E_PARAM;
    }
    if (1 > num_res_types) {
        RES_ERR((RES_MSG1("resource types %d; must be > 0\n"), num_res_types));
        result =  BCM_E_PARAM;
    }
    if (BCM_E_NONE != result) {
        /* displayed diagnostics above */
        return result;
    }
    /* set things up */
    tempHandle = sal_alloc(sizeof(_shr_res_unit_desc_t) +
                         (sizeof(_shr_res_pool_desc_t) * num_res_pools) +
                         (sizeof(_shr_res_type_desc_t) * num_res_types),
                         "resource descriptor");
    if (!tempHandle) {
        /* alloc failed */
        RES_ERR((RES_MSG1("unable to allocate %u bytes for info\n"),
                 (unsigned int)(sizeof(_shr_res_unit_desc_t) +
                                (sizeof(_shr_res_pool_desc_t) * num_res_pools) +
                                (sizeof(_shr_res_type_desc_t) * num_res_types))));
        result = BCM_E_MEMORY;
    } else { /* if (!tempUnit) */
        /* got the unit information heap cell, set it up */
        sal_memset(tempHandle,
                   0x00,
                   sizeof(_shr_res_unit_desc_t) +
                   (sizeof(_shr_res_pool_desc_t*) * num_res_pools) +
                   (sizeof(_shr_res_type_desc_t*) * num_res_types));
        tempHandle->pool = (_shr_res_pool_desc_t**)(&(tempHandle[1]));
        tempHandle->res = (_shr_res_type_desc_t**)(&(tempHandle->pool[num_res_pools]));
        tempHandle->resTypeCount = num_res_types;
        tempHandle->resPoolCount = num_res_pools;
        *handle = tempHandle;
    } /* if (!tempUnit) */

    RES_VVERB((RES_MSG("(&(%p), %d, %d) return %d (%s)\n"),
              (void*)(*handle),
              num_res_types,
              num_res_pools,
              result,
              _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Get unit resource information (top level)
 */
int
shr_mres_get(shr_mres_handle_t handle,
             int *num_res_types,
             int *num_res_pools)
{
    RES_VVERB((RES_MSG("(%p, %p, %p) enter\n"),
               (void*)handle,
               num_res_types,
               num_res_pools));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    /* return the requested information */
    if (num_res_pools) {
        *num_res_pools = handle->resPoolCount;
    }
    if (num_res_types) {
        *num_res_types = handle->resTypeCount;
    }

    RES_VVERB((RES_MSG("(%p, &(%d), &(%d)) return %d (%s)\n"),
               (void*)handle,
               num_res_types?*num_res_types:-1,
               num_res_pools?*num_res_pools:-1,
               BCM_E_NONE,
               _SHR_ERRMSG(BCM_E_NONE)));
    return BCM_E_NONE;
}

/*
 *  Deinitialise unit
 */
int
shr_mres_destroy(shr_mres_handle_t handle)
{
    int result = BCM_E_NONE;

    RES_VVERB((RES_MSG("(%p) enter\n"), (void*)handle));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    /* tear things down */
    result = _shr_mres_destroy_data(handle);
    if (BCM_E_NONE == result) {
        sal_free(handle);
    }

    RES_VVERB((RES_MSG("(%p) return %d (%s)\n"),
               (void*)handle,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Configure a resource pool on a unit
 */
int
shr_mres_pool_set(shr_mres_handle_t handle,
                  int pool_id,
                  shr_res_allocator_t manager,
                  int low_id,
                  int count,
                  const void *extras,
                  const char *name)
{
    _shr_res_pool_desc_t *tempPool;
    _shr_res_pool_desc_t *oldPool;
    int result = BCM_E_NONE;
    int xresult;
    const char *noname = "???";

    RES_VVERB((RES_MSG("(%p, %d, %s, %d, %d, %p, \"%s\") enter\n"),
               (void*)handle,
               pool_id,
               ((0 <= manager) && (SHR_RES_ALLOCATOR_COUNT > manager))?_shr_res_alloc_mgrs[manager].name:"INVALID",
               low_id,
               count,
               extras,
               name?name:noname));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_POOL_VALID_CHECK(handle, pool_id);
    if ((0 > manager) || (SHR_RES_ALLOCATOR_COUNT <= manager)) {
        RES_ERR((RES_MSG1("allocation manager type %d not supported\n"),
                 manager));
        return BCM_E_PARAM;
    }
    if (0 > count) {
        RES_ERR((RES_MSG1("negative counts are not permitted\n")));
        return BCM_E_PARAM;
    }
    if ((handle->pool[pool_id]) && (handle->pool[pool_id]->refCount)) {
        RES_ERR((RES_MSG1("%p pool %d (%s) can not be changed because it"
                          " has %d types that use it\n"),
                 (void*)handle,
                 pool_id,
                 handle->pool[pool_id]->name,
                 handle->pool[pool_id]->refCount));
        return BCM_E_CONFIG;
    }
    
    oldPool = handle->pool[pool_id];
    handle->pool[pool_id] = NULL;
    /* create the new pool */
    result = _shr_res_alloc_mgrs[manager].create(&tempPool,
                                                 low_id,
                                                 count,
                                                 extras,
                                                 name?name:noname);
    if (BCM_E_NONE == result) {
        /* new one created successfully */
        tempPool->resManagerType = manager;
        tempPool->refCount = 0;
        if (oldPool) {
            /* old one exists; get rid of it */
            result = _shr_res_alloc_mgrs[oldPool->resManagerType].destroy(oldPool);
            if (BCM_E_NONE != result) {
                handle->pool[pool_id] = oldPool;
                RES_ERR((RES_MSG1("unable to destroy %p old pool %d (%s):"
                                  " %d (%s)\n"),
                         (void*)handle,
                         pool_id,
                         oldPool->name,
                         result,
                         _SHR_ERRMSG(result)));
                xresult = _shr_res_alloc_mgrs[tempPool->resManagerType].destroy(tempPool);
                if (BCM_E_NONE != xresult) {
                    RES_ERR((RES_MSG1("unable to destroy new pool for %p pool"
                                      " %d after replace error: %d (%s)\n"),
                             (void*)handle,
                             pool_id,
                             xresult,
                             _SHR_ERRMSG(xresult)));
                }
            } /* if (BCM_E_NONE != result) */
        } /* if (oldPool) */
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        handle->pool[pool_id] = tempPool;
    }

    RES_VVERB((RES_MSG("(%p, %d, %s, %d, %d, %p, \"%s\") return %d (%s)\n"),
               (void*)handle,
               pool_id,
               _shr_res_alloc_mgrs[manager].name,
               low_id,
               count,
               extras,
               name?name:noname,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Get information about a resource pool on a unit
 */
int
shr_mres_pool_get(shr_mres_handle_t handle,
                  int pool_id,
                  shr_res_allocator_t *manager,
                  int *low_id,
                  int *count,
                  const void **extras,
                  const char **name)
{
    _shr_res_pool_desc_t *thisPool;

    RES_VVERB((RES_MSG("(%p, %d, %p, %p, %p, %p, %p) enter\n"),
               (void*)handle,
               pool_id,
               manager,
               low_id,
               count,
               extras,
               name));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_POOL_VALID_CHECK(handle, pool_id);
    RES_POOL_EXIST_CHECK(handle, pool_id);
    /* fill in the caller's request */
    thisPool = handle->pool[pool_id];
    if (manager) {
        *manager = thisPool->resManagerType;
    }
    if (low_id) {
        *low_id = thisPool->low;
    }
    if (count) {
        *count = thisPool->count;
    }
    if (extras) {
        *extras = thisPool->extras;
    }
    if (name) {
        *name = thisPool->name;
    }

    RES_VVERB((RES_MSG("(%p, %d, &(%s), &(%d), &(%d), &(%p), &(\"%s\")) return %d (%s)\n"),
               (void*)handle,
               pool_id,
               manager?_shr_res_alloc_mgrs[*manager].name:"NULL",
               low_id?*low_id:0,
               count?*count:0,
               extras?*extras:NULL,
               name?*name:"NULL",
               BCM_E_NONE,
               _SHR_ERRMSG(BCM_E_NONE)));
    return BCM_E_NONE;
}

/*
 *  Configure a resource type on a unit
 */
int
shr_mres_type_set(shr_mres_handle_t handle,
                  int res_id,
                  int pool_id,
                  int elem_size,
                  const char *name)
{
    _shr_res_type_desc_t *tempType;
    _shr_res_type_desc_t *oldType;
    int result = BCM_E_NONE;
    const char *noname = "???";

    RES_VVERB((RES_MSG("(%p, %d, %d, %d, \"%s\") enter\n"),
               (void*)handle,
               res_id,
               pool_id,
               elem_size,
               name?name:noname));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_POOL_VALID_CHECK(handle, pool_id);
    RES_POOL_EXIST_CHECK(handle, pool_id);
    RES_TYPE_VALID_CHECK(handle, res_id);
    if (1 > elem_size) {
        RES_ERR((RES_MSG1("element size %d too small; must be >= 1\n"),
                 elem_size));
        return BCM_E_PARAM;
    }
    if ((handle->res[res_id]) && (handle->res[res_id]->refCount)) {
        RES_ERR((RES_MSG1("%p resource %d (%s) can not be changed"
                          " because it has %d elements in use\n"),
                 (void*)handle,
                 res_id,
                 handle->res[res_id]->name,
                 handle->res[res_id]->refCount));
        return BCM_E_CONFIG;
    }
    if (!name) {
        /* force a non-NULL name pointer */
        name = noname;
    }
    
    oldType = handle->res[res_id];
    handle->res[res_id] = NULL;
    /* allocate new type descriptor */
    /* note base type includes one character, so don't need to add NUL here */
    tempType = sal_alloc(sizeof(*tempType) + sal_strlen(name),
                         "resource type descriptor");
    if (tempType) {
        /* got the needed memory; set it up */
        sal_memset(tempType,
                   0x00,
                   sizeof(*tempType) + sal_strlen(name));
        tempType->resElemSize = elem_size;
        tempType->resPoolId = pool_id;
        sal_strcpy(&(tempType->name[0]), name);
        if (oldType) {
            /* there was an old one; get rid of it and adjust references */
            handle->pool[oldType->resPoolId]->refCount--;
            sal_free(oldType);
        }
        /* adjust references and put this type in place */
        handle->pool[pool_id]->refCount++;
        handle->res[res_id] = tempType;
    } else {
        RES_ERR((RES_MSG1("unable to allocate %u bytes for %p resource"
                          " type %d\n"),
                 (unsigned int)(sizeof(*tempType) + sal_strlen(name)),
                 (void*)handle,
                 res_id));
        result = BCM_E_MEMORY;
        /* restore the old type */
        handle->res[res_id] = oldType;
    }

    RES_VVERB((RES_MSG("(%p, %d, %d, %d, \"%s\") return %d (%s)\n"),
               (void*)handle,
               res_id,
               pool_id,
               elem_size,
               name?name:noname,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Get information about a resource type
 */
int
shr_mres_type_get(shr_mres_handle_t handle,
                  int res_id,
                  int *pool_id,
                  int *elem_size,
                  const char **name)
{
    _shr_res_type_desc_t *thisType;

    RES_VVERB((RES_MSG("(%p, %d, %p, %p, %p) enter\n"),
               (void*)handle,
               res_id,
               pool_id,
               elem_size,
               name));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    /* fill in the caller's request */
    thisType = handle->res[res_id];
    if (pool_id) {
        *pool_id = thisType->resPoolId;
    }
    if (elem_size) {
        *elem_size = thisType->resElemSize;
    }
    if (name) {
        *name = thisType->name;
    }

    RES_VVERB((RES_MSG("(%p, %d, &(%d), &(%d), &(\"%s\")) return %d (%s)\n"),
               (void*)handle,
               res_id,
               pool_id?*pool_id:0,
               elem_size?*elem_size:0,
               name?*name:"NULL",
               BCM_E_NONE,
               _SHR_ERRMSG(BCM_E_NONE)));
    return BCM_E_NONE;
}

/*
 *  Allocate elements of a resource type
 */
int
shr_mres_alloc(shr_mres_handle_t handle,
               int res_id,
               uint32 flags,
               int count,
               int *elem)
{
    _shr_res_pool_desc_t *thisPool;
    int result = BCM_E_NONE;
    int scaled;

    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, %p) enter\n"),
               (void*)handle,
               res_id,
               flags,
               count,
               elem));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (0 >= count) {
        RES_ERR((RES_MSG1("element count %d must be > 0\n"), count));
        return BCM_E_PARAM;
    }
    if (flags & (~SHR_RES_ALLOC_SINGLE_FLAGS)) {
        RES_ERR((RES_MSG1("invalid flags %08X\n"),
                 flags & (~SHR_RES_ALLOC_SINGLE_FLAGS)));
        return BCM_E_PARAM;
    }
    if (!elem) {
        RES_ERR((RES_MSG1("obligatory argument is NULL\n")));
        return BCM_E_PARAM;
    }
    /* adjust element count per scaling factor */
    scaled = count * handle->res[res_id]->resElemSize;
    
    /* get the pool information */
    thisPool = handle->pool[handle->res[res_id]->resPoolId];
    /* make the call */
    result = _shr_res_alloc_mgrs[thisPool->resManagerType].alloc(thisPool,
                                                                 flags,
                                                                 scaled,
                                                                 elem);
    if (BCM_E_NONE == result) {
        /* account for successful allocation */
        handle->res[res_id]->refCount += count;
    }

    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, &(%d)) return %d (%s)\n"),
               (void*)handle,
               res_id,
               flags,
               count,
               *elem,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Allocate a bunch of elements or blocks of a resource type
 */
int
shr_mres_alloc_group(shr_mres_handle_t handle,
                     int res_id,
                     uint32 grp_flags,
                     int grp_size,
                     int *grp_done,
                     const uint32 *flags,
                     const int *count,
                     int *elem)
{
    _shr_res_pool_desc_t *thisPool;
    _shr_res_type_desc_t *thisRes;
    int result = BCM_E_NONE;
    int xresult;
    int scaled;
    int index;
    uint32 blkFlags;

    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, %p, %p, %p, %p) enter\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               grp_done,
               flags,
               count,
               elem));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (!grp_done) {
        RES_ERR((RES_MSG1("obligatory out argument grp_done is NULL\n")));
        return BCM_E_PARAM;
    }
    *grp_done = 0;
    if (0 > grp_size) {
        RES_ERR((RES_MSG1("group member count %d must be >= 0\n"), grp_size));
        return BCM_E_PARAM;
    }
    if ((0 < grp_size) && (!flags || !count || !elem)) {
        RES_ERR((RES_MSG1("an obligatory array pointer is NULL\n")));
        return BCM_E_PARAM;
    }
    if (grp_flags & (~(SHR_RES_ALLOC_SINGLE_FLAGS | SHR_RES_ALLOC_GROUP_FLAGS))) {
        RES_ERR((RES_MSG1("invalid group flags %08X\n"),
                 grp_flags & (~(SHR_RES_ALLOC_SINGLE_FLAGS |
                                SHR_RES_ALLOC_GROUP_FLAGS))));
        return BCM_E_PARAM;
    }
    /* get the resource information */
    thisRes = handle->res[res_id];
    /* get the pool information */
    thisPool = handle->pool[thisRes->resPoolId];
    /* try to collect the requested blocks */
    for (index = 0;
         (BCM_E_NONE == result) && (index < grp_size);
         index++) {
        /* check parameters for this block */
        blkFlags = flags[index] | (grp_flags & SHR_RES_ALLOC_SINGLE_FLAGS);
        if (blkFlags & (~SHR_RES_ALLOC_SINGLE_FLAGS)) {
            RES_ERR((RES_MSG1("invalid flags %08X for block %d\n"),
                     blkFlags & (~SHR_RES_ALLOC_SINGLE_FLAGS),
                     index));
            result = BCM_E_PARAM;
        }
        if (0 >= count[index]) {
            RES_ERR((RES_MSG1("element count %d must be > 0\n"), count[index]));
            result = BCM_E_PARAM;
        }
        if (BCM_E_NONE == result) {
            /* adjust element count per scaling factor */
            scaled = count[index] * thisRes->resElemSize;
            
            /* make the call */
            result = _shr_res_alloc_mgrs[thisPool->resManagerType].alloc(thisPool,
                                                                         blkFlags,
                                                                         scaled,
                                                                         &(elem[index]));
        }
        if (BCM_E_NONE == result) {
            /* account for successful allocation */
            thisRes->refCount += count[index];
        } else {
            /* we'll hope the allocation manager displayed an error */
            /* don't want postincrement if an error occurred */
            break;
        }
    } /* for (all blocks in the caller's group) */
    if ((BCM_E_NONE != result) && (grp_flags & SHR_RES_ALLOC_GROUP_ATOMIC)) {
        /* atomic mode and it failed; back out everything that we have */
        /* index is at the first failure */
        while (index > 0) {
            /* look at previous index (must have been successful) */
            index--;
            /* adjust element count per scaling factor */
            scaled = count[index] * thisRes->resElemSize;
            /* free this element or block */
            xresult = _shr_res_alloc_mgrs[thisPool->resManagerType].free(thisPool,
                                                                         scaled,
                                                                         elem[index]);
            if (BCM_E_NONE != xresult) {
                RES_ERR((RES_MSG1("unable to back out %p resource %d"
                                  " index %d base %d count %d: %d (%s)\n"),
                         (void*)handle,
                         res_id,
                         index,
                         elem[index],
                         count[index],
                         result,
                         _SHR_ERRMSG(result)));
            } else {
                thisRes->refCount -= count[index];
            }
        } /* while (index > 0) */
    } /* if (error && SHR_RES_ALLOC_GROUP_ATOMIC was set) */
    /* update number of allocations that succeeded */
    *grp_done = index;

    /* return the result */
    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, &(%d), %p, %p, %p) enter\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               *grp_done,
               flags,
               count,
               elem));
#if RES_EXCESS_VERBOSITY
    for (index = 0; index < grp_size; index++) {
        RES_EVERB((RES_MSG1("  block %12d: %08X %12d %12d\n"),
                   index,
                   flags[index],
                   count[index],
                   elem[index]));
    }
#endif /* RES_EXCESS_VERBOSITY */
    return result;
}

/*
 *  Allocate a block of elements with the requested alignment and offset.
 */
int
shr_mres_alloc_align(shr_mres_handle_t handle,
                     int res_id,
                     uint32 flags,
                     int align,
                     int offset,
                     int count,
                     int *elem)
{
    _shr_res_pool_desc_t *thisPool;
    _shr_res_type_desc_t *thisType;
    int result = BCM_E_NONE;
    int base;
    int scaled;
    int scaledAlign;
    int scaledOffset;

    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, %d, %d, %p) enter\n"),
               (void*)handle,
               res_id,
               flags,
               align,
               offset,
               count,
               elem));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (0 >= count) {
        RES_ERR((RES_MSG1("element count %d must be > 0\n"), count));
        return BCM_E_PARAM;
    }
    if (1 > align) {
        RES_WARN((RES_MSG1("align <= 0 invalid, using align = 1 instead\n")));
        align = 1;
    }
    if ((offset >= align) || (0 > offset)) {
        RES_ERR((RES_MSG1("offset %d must be >= 0 and < align %d\n"),
                 offset,
                 align));
        return BCM_E_PARAM;
    }
    if (flags & (~SHR_RES_ALLOC_SINGLE_FLAGS)) {
        RES_ERR((RES_MSG1("invalid flags %08X\n"),
                 flags & (~SHR_RES_ALLOC_SINGLE_FLAGS)));
        return BCM_E_PARAM;
    }
    if (!elem) {
        RES_ERR((RES_MSG1("obligatory argument is NULL\n")));
        return BCM_E_PARAM;
    }
    /* get the pool information */
    thisType = handle->res[res_id];
    thisPool = handle->pool[thisType->resPoolId];
    /* adjust element count per scaling factor */
    scaled = count * thisType->resElemSize;
    scaledAlign = align * thisType->resElemSize;
    scaledOffset = offset * thisType->resElemSize;
    
    /* check alignment for WITH_ID case */
    if (flags & SHR_RES_ALLOC_WITH_ID) {
        if (flags & SHR_RES_ALLOC_ALIGN_ZERO) {
            base = *elem;
        } else {
            base = *elem - thisPool->low;
        }
        if (((((base) / scaledAlign) * scaledAlign) + scaledOffset) != base) {
            RES_ERR((RES_MSG1("WITH_ID requested element %d does not comply"
                              " with alignment specifications\n"),
                     *elem));
            return BCM_E_PARAM;
        }
    }
    if (_shr_res_alloc_mgrs[thisPool->resManagerType].align) {
        /* allocator supports it; make the call */
        result = _shr_res_alloc_mgrs[thisPool->resManagerType].align(thisPool,
                                                                     flags,
                                                                     scaledAlign,
                                                                     scaledOffset,
                                                                     scaled,
                                                                     elem);
    } else {
        /* not supported by this allocator */
        RES_ERR((RES_MSG1("allocator type %s does not support aligned alloc\n"),
                 _shr_res_alloc_mgrs[thisPool->resManagerType].name));
        result = BCM_E_UNAVAIL;
    }
    if (BCM_E_NONE == result) {
        /* account for successful allocation */
        thisType->refCount += count;
    }

    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, %d, %d, &(%d)) return %d (%s)\n"),
               (void*)handle,
               res_id,
               flags,
               align,
               offset,
               count,
               *elem,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Allocate a bunch of aligned elements or blocks of a resource type
 */
int
shr_mres_alloc_align_group(shr_mres_handle_t handle,
                           int res_id,
                           uint32 grp_flags,
                           int grp_size,
                           int *grp_done,
                           const uint32 *flags,
                           const int *align,
                           const int *offset,
                           const int *count,
                           int *elem)
{
    _shr_res_pool_desc_t *thisPool;
    _shr_res_type_desc_t *thisRes;
    int result = BCM_E_NONE;
    int xresult;
    int scaled;
    int scaledAlign;
    int scaledOffset;
    int index;
    int xalign;
    int base;
    uint32 blkFlags;

    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, %p, %p, %p, %p, %p, %p) enter\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               grp_done,
               flags,
               align,
               offset,
               count,
               elem));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (!grp_done) {
        RES_ERR((RES_MSG1("obligatory out argument grp_done is NULL\n")));
        return BCM_E_PARAM;
    }
    *grp_done = 0;
    if (0 > grp_size) {
        RES_ERR((RES_MSG1("group member count %d must be >= 0\n"), grp_size));
        return BCM_E_PARAM;
    }
    if ((0 < grp_size) && (!flags || !count || !elem || !align || !offset)) {
        RES_ERR((RES_MSG1("an obligatory array pointer is NULL\n")));
        return BCM_E_PARAM;
    }
    if (grp_flags & (~(SHR_RES_ALLOC_SINGLE_FLAGS | SHR_RES_ALLOC_GROUP_FLAGS))) {
        RES_ERR((RES_MSG1("invalid group flags %08X\n"),
                 grp_flags & (~(SHR_RES_ALLOC_SINGLE_FLAGS |
                                SHR_RES_ALLOC_GROUP_FLAGS))));
        return BCM_E_PARAM;
    }
    /* get the resource information */
    thisRes = handle->res[res_id];
    /* get the pool information */
    thisPool = handle->pool[thisRes->resPoolId];
    /* try to collect the requested blocks */
    if (_shr_res_alloc_mgrs[thisPool->resManagerType].align) {
        /* allocator does not support this feature */
        RES_ERR((RES_MSG1("allocator type %s does not support aligned alloc\n"),
                 _shr_res_alloc_mgrs[thisPool->resManagerType].name));
        result = BCM_E_UNAVAIL;
    }
    for (index = 0;
         (BCM_E_NONE == result) && (index < grp_size);
         index++) {
        /* check parameters for this block */
        blkFlags = flags[index] | (grp_flags & SHR_RES_ALLOC_SINGLE_FLAGS);
        if (blkFlags & (~SHR_RES_ALLOC_SINGLE_FLAGS)) {
            RES_ERR((RES_MSG1("invalid flags %08X for block %d\n"),
                     blkFlags & (~SHR_RES_ALLOC_SINGLE_FLAGS),
                     index));
            result = BCM_E_PARAM;
        }
        if (0 >= count[index]) {
            RES_ERR((RES_MSG1("element count %d must be > 0\n"), count[index]));
            result = BCM_E_PARAM;
        }
        if (1 > align[index]) {
            RES_WARN((RES_MSG1("align <= 0 invalid, using align = 1 instead\n")));
            xalign = 1;
        } else {
            xalign = align[index];
        }
        if ((offset[index] >= xalign) || (0 > offset[index])) {
            RES_ERR((RES_MSG1("offset %d must be >= 0 and < align %d\n"),
                     offset[index],
                     xalign));
            result = BCM_E_PARAM;
        }
        if (BCM_E_NONE == result) {
            /* adjust element count per scaling factor */
            scaled = count[index] * thisRes->resElemSize;
            scaledAlign = xalign * thisRes->resElemSize;
            scaledOffset = offset[index] * thisRes->resElemSize;
            
            /* check alignment for WITH_ID case */
            if (blkFlags & SHR_RES_ALLOC_WITH_ID) {
                if (blkFlags & SHR_RES_ALLOC_ALIGN_ZERO) {
                    base = elem[index];
                } else {
                    base = elem[index] - thisPool->low;
                }
                if (((((base) / scaledAlign) * scaledAlign) + scaledOffset) != base) {
                    RES_ERR((RES_MSG1("WITH_ID requested element %d does not"
                                      " comply with alignment specifications\n"),
                             elem[index]));
                    result = BCM_E_PARAM;
                }
            }
            if (BCM_E_NONE == result) {
                result = _shr_res_alloc_mgrs[thisPool->resManagerType].align(thisPool,
                                                                             blkFlags,
                                                                             scaled,
                                                                             scaledAlign,
                                                                             scaledOffset,
                                                                             &(elem[index]));
            }
        }
        if (BCM_E_NONE == result) {
            /* account for successful allocation */
            thisRes->refCount += count[index];
        } else {
            /* we'll hope the allocation manager displayed an error */
            /* don't want postincrement if an error occurred */
            break;
        }
    } /* for (all blocks in the caller's group) */
    if ((BCM_E_NONE != result) && (grp_flags & SHR_RES_ALLOC_GROUP_ATOMIC)) {
        /* atomic mode and it failed; back out everything that we have */
        /* index is at the first failure */
        while (index > 0) {
            /* look at previous index (must have been successful) */
            index--;
            /* adjust element count per scaling factor */
            scaled = count[index] * thisRes->resElemSize;
            /* free this element or block */
            xresult = _shr_res_alloc_mgrs[thisPool->resManagerType].free(thisPool,
                                                                         scaled,
                                                                         elem[index]);
            if (BCM_E_NONE != xresult) {
                RES_ERR((RES_MSG1("unable to back out %p resource %d"
                                  " index %d base %d count %d: %d (%s)\n"),
                         (void*)handle,
                         res_id,
                         index,
                         elem[index],
                         count[index],
                         result,
                         _SHR_ERRMSG(result)));
            } else {
                thisRes->refCount -= count[index];
            }
        } /* while (index > 0) */
    } /* if (error && SHR_RES_ALLOC_GROUP_ATOMIC was set) */
    /* update number of allocations that succeeded */
    *grp_done = index;

    /* return the result */
    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, &(%d), %p, %p, %p, %p, %p) enter\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               *grp_done,
               flags,
               align,
               offset,
               count,
               elem));
#if RES_EXCESS_VERBOSITY
    for (index = 0; index < grp_size; index++) {
        RES_EVERB((RES_MSG1("  block %12d: %08X %12d %12d %12d %12d\n"),
                   index,
                   flags[index],
                   align[index],
                   offset[index],
                   count[index],
                   elem[index]));
    }
#endif /* RES_EXCESS_VERBOSITY */
    return result;
}

/*
 *  Free elements of a resource type
 */
int
shr_mres_free(shr_mres_handle_t handle,
              int res_id,
              int count,
              int elem)
{
    _shr_res_pool_desc_t *thisPool;
    int result = BCM_E_NONE;
    int scaled;

    RES_VVERB((RES_MSG1("(%p, %d, %d, %d) enter\n"),
               (void*)handle,
               res_id,
               count,
               elem));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (0 >= count) {
        RES_ERR((RES_MSG1("element count %d must be > 0\n"), count));
        return BCM_E_PARAM;
    }
    /* adjust element count per scaling factor */
    scaled = count * handle->res[res_id]->resElemSize;
    
    /* get the pool information */
    thisPool = handle->pool[handle->res[res_id]->resPoolId];
    /* make the call */
    result = _shr_res_alloc_mgrs[thisPool->resManagerType].free(thisPool,
                                                                scaled,
                                                                elem);
    if (BCM_E_NONE == result) {
        /* account for successful deallocation */
        handle->res[res_id]->refCount -= count;
    }

    RES_VVERB((RES_MSG1("(%p, %d, %d, %d) return %d (%s)\n"),
               (void*)handle,
               res_id,
               count,
               elem,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Free a bunch of elements/blocks of a resource type
 */
int
shr_mres_free_group(shr_mres_handle_t handle,
                    int res_id,
                    uint32 grp_flags,
                    int grp_size,
                    int *grp_done,
                    const int *count,
                    const int *elem)
{
    _shr_res_pool_desc_t *thisPool;
    _shr_res_type_desc_t *thisRes;
    int result = BCM_E_NONE;
    int index;
    int scaled;

    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, %p, %p, %p) enter\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               grp_done,
               count,
               elem));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (!grp_done) {
        RES_ERR((RES_MSG1("obligatory out argument grp_done is NULL\n")));
        return BCM_E_PARAM;
    }
    *grp_done = 0;
    if (0 > grp_size) {
        RES_ERR((RES_MSG1("group member count %d must be >= 0\n"), grp_size));
        return BCM_E_PARAM;
    }
    if ((0 < grp_size) && (!count || !elem)) {
        RES_ERR((RES_MSG1("an obligatory array pointer is NULL\n")));
        return BCM_E_PARAM;
    }
    if (grp_flags & (~SHR_RES_ALLOC_GROUP_FLAGS)) {
        RES_ERR((RES_MSG1("invalid group flags %08X\n"),
                 grp_flags & (~SHR_RES_ALLOC_GROUP_FLAGS)));
        return BCM_E_PARAM;
    }
    /* get the resource information */
    thisRes = handle->res[res_id];
    /* get the pool information */
    thisPool = handle->pool[thisRes->resPoolId];
    /* try to release the requested blocks */
    for (index = 0;
         (BCM_E_NONE == result) && (index < grp_size);
         index++) {
        /* adjust element count per scaling factor */
        scaled = count[index] * thisRes->resElemSize;
        /* free this element or block */
        result = _shr_res_alloc_mgrs[thisPool->resManagerType].free(thisPool,
                                                                    scaled,
                                                                    elem[index]);
        if (BCM_E_NONE == result) {
            /* account for successful deallocation */
            handle->res[res_id]->refCount -= count[index];
        } else {
            /* we'll hope the allocation manager displayed an error */
            /* don't want postincrement if an error occurred */
            break;
        }
    } /* for (all elements/blocks as long as no errors) */
    /* update number of frees that succeeded */
    *grp_done = index;

    /* return the result */
    RES_VVERB((RES_MSG1("(%p, %d, %08X, %d, &(%d), %p, %p) enter\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               *grp_done,
               count,
               elem));
#if RES_EXCESS_VERBOSITY
    for (index = 0; index < grp_size; index++) {
        RES_EVERB((RES_MSG1("  block %12d: %12d, %12d\n"),
                   index,
                   count[index],
                   elem[index]));
    }
#endif /* RES_EXCESS_VERBOSITY */
    return result;
}

/*
 *  Check whether elements are free or not
 */
int
shr_mres_check(shr_mres_handle_t handle,
               int res_id,
               int count,
               int elem)
{
    _shr_res_pool_desc_t *thisPool;
    int scaled;
    int result;

    RES_VVERB((RES_MSG("(%p, %d, %d, %d) enter\n"),
               (void*)handle,
               res_id,
               count,
               elem));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (1 > count) {
        RES_ERR((RES_MSG1("element count %d must be > 0\n"), count));
        return BCM_E_PARAM;
    }
    /* adjust element count per scaling factor */
    scaled = count * handle->res[res_id]->resElemSize;
    
    /* get the pool information */
    thisPool = handle->pool[handle->res[res_id]->resPoolId];
    /* make the call */
    result = _shr_res_alloc_mgrs[thisPool->resManagerType].check(thisPool,
                                                                 scaled,
                                                                 elem);

    RES_VVERB((RES_MSG("(%p, %d, %d, %d) return %d (%s)\n"),
               (void*)handle,
               res_id,
               count,
               elem,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Check a bunch of elements/blocks of a resource type
 */
int
shr_mres_check_group(shr_mres_handle_t handle,
                     int res_id,
                     uint32 grp_flags,
                     int grp_size,
                     int *grp_done,
                     const int *count,
                     const int *elem,
                     int *status)
{
    _shr_res_pool_desc_t *thisPool;
    _shr_res_type_desc_t *thisRes;
    int result = BCM_E_NONE;
    int xresult;
    int index;
    int scaled;

    RES_VVERB((RES_MSG("(%p, %d, %08X, %d, %p, %p, %p, %p) enter\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               grp_done,
               count,
               elem,
               status));

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);
    RES_TYPE_VALID_CHECK(handle, res_id);
    RES_TYPE_EXIST_CHECK(handle, res_id);
    if (!grp_done) {
        RES_ERR((RES_MSG1("obligatory out argument grp_done is NULL\n")));
        return BCM_E_PARAM;
    }
    *grp_done = 0;
    if (0 > grp_size) {
        RES_ERR((RES_MSG1("group member count %d must be >= 0\n"), grp_size));
        return BCM_E_PARAM;
    }
    if ((0 < grp_size) && (!count || !elem || !status)) {
        RES_ERR((RES_MSG1("an obligatory array pointer is NULL\n")));
        return BCM_E_PARAM;
    }
    if (grp_flags & (~SHR_RES_ALLOC_GROUP_FLAGS)) {
        RES_ERR((RES_MSG1("invalid group flags %08X\n"),
                 grp_flags & (~SHR_RES_ALLOC_GROUP_FLAGS)));
        return BCM_E_PARAM;
    }
    /* get the resource information */
    thisRes = handle->res[res_id];
    /* get the pool information */
    thisPool = handle->pool[thisRes->resPoolId];
    /* try to check the requested blocks */
    for (index = 0;
         (BCM_E_NONE == result) && (index < grp_size);
         index++) {
        /* adjust element count per scaling factor */
        scaled = count[index] * thisRes->resElemSize;
        /* check this element or block */
        xresult = _shr_res_alloc_mgrs[thisPool->resManagerType].check(thisPool,
                                                                      scaled,
                                                                      elem[index]);
        status[index] = xresult;
        if ((BCM_E_NOT_FOUND != xresult) &&
            (BCM_E_EXISTS != xresult)) {
            RES_ERR((RES_MSG1("unexpected result checking %p resource %d"
                              " index %d elem %d count %d: %d (%s)\n"),
                     (void*)handle,
                     res_id,
                     index,
                     elem[index],
                     count[index],
                     result,
                     _SHR_ERRMSG(result)));
            if (!(grp_flags & SHR_RES_ALLOC_GROUP_ATOMIC)) {
                /* if not atomic mode, abort on first unexpected result */
                result = BCM_E_FAIL;
                break;
            }
        } /* if (result is neither NOT_FOUND nor EXISTS) */
    } /* for (all elements/blocks as long as no errors) */
    /* update number of frees that succeeded */
    *grp_done = index;

    /* return the result */
    RES_VVERB((RES_MSG("(%p, %d, %08X, %d, &(%d), %p, %p, %p) return %d (%s)\n"),
               (void*)handle,
               res_id,
               grp_flags,
               grp_size,
               *grp_done,
               count,
               elem,
               status,
               result,
               _SHR_ERRMSG(result)));
#if RES_EXCESS_VERBOSITY
    for (index = 0; index < grp_size; index++) {
        RES_EVERB((RES_MSG1("  block %12d: %d, %d, %d (%s)\n"),
                   index,
                   count[index],
                   elem[index],
                   status[index],
                   _SHR_ERRMSG(status[index])));
    }
#endif /* RES_EXCESS_VERBOSITY */
    return result;
}

/*
 *  Diagnostic dump
 */
int
shr_mres_dump(shr_mres_handle_t handle)
{
    _shr_res_pool_desc_t *thisPool;
    _shr_res_type_desc_t *thisRes;
    int index;
    int result = BCM_E_NONE;

    /* a little parameter checking */
    RES_HANDLE_VALID_CHECK(handle);

    /* dump information about the handle */
    soc_cm_print("%p resource allocation manager\n", (void*)handle);
    soc_cm_print("  resource type count = %d\n", handle->resTypeCount);
    soc_cm_print("  resource pool count = %d\n", handle->resPoolCount);
    for (index = 0; index < handle->resPoolCount; index++) {
        thisPool = handle->pool[index];
        if (thisPool) {
            soc_cm_print("  resource pool %d (%s):\n",
                         index,
                         &(thisPool->name[0]));
            soc_cm_print("    method = %d (%s)\n",
                         thisPool->resManagerType,
                         _shr_res_alloc_mgrs[thisPool->resManagerType].name);
            soc_cm_print("    range  = %d..%d (%d elements)\n",
                         thisPool->low,
                         thisPool->low + thisPool->count - 1,
                         thisPool->count);
            soc_cm_print("    refcnt = %d\n", thisPool->refCount);
        } else {
            soc_cm_print("  resource pool %d is not in use\n", index);
        }
    }
    for (index = 0; index < handle->resTypeCount; index++) {
        thisRes = handle->res[index];
        if (thisRes) {
            thisPool = handle->pool[thisRes->resPoolId];
            soc_cm_print("  resource type %d (%s):\n",
                         index,
                         &(thisRes->name[0]));
            soc_cm_print("    resource pool   = %d (%s)\n",
                         thisRes->resPoolId,
                         &(thisPool->name[0]));
            soc_cm_print("    pool elem each  = %d\n",
                         thisRes->resElemSize);
            soc_cm_print("    active elements = %d\n",
                         thisRes->refCount);
        } else {
            soc_cm_print("  resource type %d is not in use\n", index);
        }
    }
    return result;
}

/*****************************************************************************/
/*
 *  Exposed API implementation (unit based, local handle cache)
 */

/*
 *  Initialise unit
 */
int
shr_res_init(int unit,
             int num_res_types,
             int num_res_pools)
{
    sal_mutex_t tempLock = NULL;
    _shr_res_unit_desc_t *tempUnit;
    int result = BCM_E_NONE;

    RES_VVERB((RES_MSG("(%d, %d, %d) enter\n"),
               unit,
               num_res_types,
               num_res_pools));

    /* a little parameter checking */
    if ((0 > unit) || (BCM_LOCAL_UNITS_MAX <= unit)) {
        RES_ERR((RES_MSG1("invalid unit number %d\n"),
                 unit));
        return BCM_E_PARAM;
    }

    /* first, create global lock if none exists yet, and init unit pointers */
    if (!_g_resLock) {
        RES_VVERB((RES_MSG1("create global resource lock\n")));
        tempLock = sal_mutex_create("global resource mutex");
        if (!tempLock) {
            RES_ERR((RES_MSG1("unable to create global resource lock\n")));
            return BCM_E_RESOURCE;
        }
        /* claim the newly created lock */
        if (sal_mutex_take(tempLock, sal_sem_FOREVER)) {
            RES_ERR((RES_MSG1("unable to claim global resource lock\n")));
            if (tempLock) {
                sal_mutex_destroy(tempLock);
                return BCM_E_INTERNAL;
            }
        }
        /* put claimed lock as the global one */
        _g_resLock = tempLock;
        RES_VVERB((RES_MSG1("init unit resource pointers\n")));
        /* clear the unit pointers */
        sal_memset(&(_g_unitResDesc[0]),
                   0x00,
                   sizeof(_g_unitResDesc[0]) * BCM_LOCAL_UNITS_MAX);
        /* deal with contention */
        if (_g_resLock != tempLock) {
            /* somebody else tried init at the same time */
            /*
             *  More than one thread tried to do the base initialisation at the
             *  same time.  Since all this amounts to is the creation of the
             *  global lock and the zeroing of the unit pointers, there should
             *  be no loss to this point.  What we want to do is clean up the
             *  'lost' lock(s) by freeing and destroying them.  The thread who
             *  owns the winning (last!) global lock will release its copy but
             *  not destroy it, and all threads will compete to see who gets to
             *  initialise each specific unit.
             *
             *  Unhappily, this does not solve the contention problem, but
             *  merely tries to deal with some part of it.  It is possible that
             *  two or more threads will reach this point at sightly different
             *  times with just the right phase offset to miss it.  In that
             *  case, we have to hope that leaking a few semaphores is not too
             *  big of a problem.
             */
            RES_WARN((RES_MSG1("contention detected; free disused lock\n")));
            if (sal_mutex_give(tempLock)) {
                RES_ERR((RES_MSG1("unable to release temporary lock\n")));
                result = BCM_E_INTERNAL;
            }
            sal_mutex_destroy(tempLock);
        } else { /* if (_g_resLock != tempLock) */
            /*
             *  Looks like I'm the winning thread (defined here rather
             *  counterintuitively as the *last* thread to complete this code
             *  fragment!), so don't free the lock, but do release it.
             */
            if (sal_mutex_give(_g_resLock)) {
                RES_ERR((RES_MSG1("unable to release global lock\n")));
                result = BCM_E_INTERNAL;
            }
        } /* if (_g_resLock != tempLock) */
    } /* if (!_g_resLock) */
    if (BCM_E_NONE != result) {
        /*
         *  Something went wrong above, and a cleanup was attempted.  Even if
         *  the cleanup was successful, return the error now.
         */
        return result;
    }

    /* at this point, everybody needs to be in sync */
    RES_LOCK_TAKE;

    /* get this unit's current information, mark it as destroyed */
    
    tempUnit = _g_unitResDesc[unit];
    _g_unitResDesc[unit] = NULL;
    if (tempUnit) {
        /* this unit has already been initialised; tear it down */
        result = _shr_mres_destroy_data(tempUnit);
        if (BCM_E_NONE != result) {
            /* something went wrong with the teardown, put what's left back */
            _g_unitResDesc[unit] = tempUnit;
        }
        tempUnit = NULL;
    }

    if (BCM_E_NONE == result) {
        result = shr_mres_create(&tempUnit,
                                 num_res_types,
                                 num_res_pools);
        if (BCM_E_NONE == result) {
            _g_unitResDesc[unit] = tempUnit;
        }
    } /* if (BCM_E_NONE == result) */

    /* release the lock and return the result */
    RES_LOCK_GIVE;
    RES_VVERB((RES_MSG("(%d, %d, %d) return %d (%s)\n"),
               unit,
               num_res_types,
               num_res_pools,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Get unit resource information (top level)
 */
int
shr_res_get(int unit,
            int *num_res_types,
            int *num_res_pools)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the fetch */
    return shr_mres_get(thisUnit, num_res_types, num_res_pools);
}

/*
 *  Deinitialise unit
 */
int
shr_res_detach(int unit)
{
    _shr_res_unit_desc_t *tempUnit;
    int result = BCM_E_NONE;

    RES_VVERB((RES_MSG("(%d) enter\n"), unit));

    if (!_g_resLock) {
        /* not inited at all, detach is easy! */
        return BCM_E_NONE;
    }

    /* at this point, everybody needs to be in sync */
    RES_LOCK_TAKE;

    /* get this unit's current information, mark it as destroyed */
    
    tempUnit = _g_unitResDesc[unit];
    _g_unitResDesc[unit] = NULL;
    if (tempUnit) {
        /* this unit has already been initialised; tear it down */
        result = _shr_mres_destroy_data(tempUnit);
        if (BCM_E_NONE != result) {
            /* something went wrong with the teardown, put what's left back */
            _g_unitResDesc[unit] = tempUnit;
        }
        tempUnit = NULL;
    } /* if (tempUnit) */
    /* else would be not inited, again, easy to detach in that case - NOP */

    /* release the lock and return the result */
    RES_LOCK_GIVE;
    RES_VVERB((RES_MSG("(%d) return %d (%s)\n"),
               unit,
               result,
               _SHR_ERRMSG(result)));
    return result;
}

/*
 *  Configure a resource pool on a unit
 */
int
shr_res_pool_set(int unit,
                 int pool_id,
                 shr_res_allocator_t manager,
                 int low_id,
                 int count,
                 const void *extras,
                 const char *name)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_pool_set(thisUnit,
                             pool_id,
                             manager,
                             low_id,
                             count,
                             extras,
                             name);
}

/*
 *  Get information about a resource pool on a unit
 */
int
shr_res_pool_get(int unit,
                 int pool_id,
                 shr_res_allocator_t *manager,
                 int *low_id,
                 int *count,
                 const void **extras,
                 const char **name)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_pool_get(thisUnit,
                             pool_id,
                             manager,
                             low_id,
                             count,
                             extras,
                             name);
}

/*
 *  Configure a resource type on a unit
 */
int
shr_res_type_set(int unit,
                 int res_id,
                 int pool_id,
                 int elem_size,
                 const char *name)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_type_set(thisUnit, res_id, pool_id, elem_size, name);
}

/*
 *  Get information about a resource type
 */
int
shr_res_type_get(int unit,
                 int res_id,
                 int *pool_id,
                 int *elem_size,
                 const char **name)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_type_get(thisUnit, res_id, pool_id, elem_size, name);
}

/*
 *  Allocate elements of a resource type
 */
int
shr_res_alloc(int unit,
              int res_id,
              uint32 flags,
              int count,
              int *elem)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_alloc(thisUnit, res_id, flags, count, elem);
}

/*
 *  Allocate a bunch of elements or blocks of a resource type
 */
int
shr_res_alloc_group(int unit,
                    int res_id,
                    uint32 grp_flags,
                    int grp_size,
                    int *grp_done,
                    const uint32 *flags,
                    const int *count,
                    int *elem)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_alloc_group(thisUnit,
                                res_id,
                                grp_flags,
                                grp_size,
                                grp_done,
                                flags,
                                count,
                                elem);
}

/*
 *  Allocate a block of elements with the requested alignment and offset.
 */
int
shr_res_alloc_align(int unit,
                    int res_id,
                    uint32 flags,
                    int align,
                    int offset,
                    int count,
                    int *elem)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_alloc_align(thisUnit,
                                res_id,
                                flags,
                                align,
                                offset,
                                count,
                                elem);
}

/*
 *  Allocate a bunch of aligned elements or blocks of a resource type
 */
int
shr_res_alloc_align_group(int unit,
                          int res_id,
                          uint32 grp_flags,
                          int grp_size,
                          int *grp_done,
                          const uint32 *flags,
                          const int *align,
                          const int *offset,
                          const int *count,
                          int *elem)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_alloc_align_group(thisUnit,
                                      res_id,
                                      grp_flags,
                                      grp_size,
                                      grp_done,
                                      flags,
                                      align,
                                      offset,
                                      count,
                                      elem);
}

/*
 *  Free elements of a resource type
 */
int
shr_res_free(int unit,
             int res_id,
             int count,
             int elem)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_free(thisUnit, res_id, count, elem);
}

/*
 *  Free a bunch of elements/blocks of a resource type
 */
int
shr_res_free_group(int unit,
                   int res_id,
                   uint32 grp_flags,
                   int grp_size,
                   int *grp_done,
                   const int *count,
                   const int *elem)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_free_group(thisUnit,
                               res_id,
                               grp_flags,
                               grp_size,
                               grp_done,
                               count,
                               elem);
}

/*
 *  Check whether elements are free or not
 */
int
shr_res_check(int unit,
              int res_id,
              int count,
              int elem)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_check(thisUnit, res_id, count, elem);
}

/*
 *  Check a bunch of elements/blocks of a resource type
 */
int
shr_res_check_group(int unit,
                    int res_id,
                    uint32 grp_flags,
                    int grp_size,
                    int *grp_done,
                    const int *count,
                    const int *elem,
                    int *status)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_check_group(thisUnit,
                                res_id,
                                grp_flags,
                                grp_size,
                                grp_done,
                                count,
                                elem,
                                status);
}

/*
 *  Diagnostic dump
 */
int
shr_res_dump(int unit)
{
    _shr_res_unit_desc_t *thisUnit;

    /* a little parameter checking */
    RES_UNIT_CHECK(unit, thisUnit);

    /* perform the action */
    return shr_mres_dump(thisUnit);
}

/*****************************************************************************/
/*
 *  Implementation of bitmap (implemented here rather than externally)
 */



typedef struct _shr_res_bitmap_data_s {
    int lastFree;
    int nextAlloc;
    SHR_BITDCL data[1];
} _shr_res_bitmap_data_t;

static int
_shr_res_bitmap_create(_shr_res_pool_desc_t **desc,
                       int low_id,
                       int count,
                       const void* extras,
                       const char* name)
{
    /* need the base descriptor */
    *desc = sal_alloc(sizeof(_shr_res_pool_desc_t) + sal_strlen(name),
                      "bitmap resource descriptor");
    if (!(*desc)) {
        /* alloc failed */
        RES_ERR((RES_MSG1("unable to allocate %u bytes for descriptor\n"),
                 (unsigned int)(sizeof(_shr_res_pool_desc_t) +
                                sal_strlen(name))));
        return BCM_E_MEMORY;
    }
    sal_memset(*desc, 0x00, sizeof(_shr_res_pool_desc_t) + sal_strlen(name));
    (*desc)->count = count;
    (*desc)->low = low_id;
    (*desc)->extras = NULL; /* don't need this here */
    sal_strcpy(&((*desc)->name[0]), name);
    /* now allocate the bitmap space */
    (*desc)->resHandle = sal_alloc(sizeof(_shr_res_bitmap_data_t) +
                                   SHR_BITALLOCSIZE(count) -
                                   sizeof(SHR_BITDCL),
                                   "bitmap resource data");
    if (!((*desc)->resHandle)) {
        /* alloc failed */
        RES_ERR((RES_MSG1("unable to allocate %u bytes for data\n"),
                 (unsigned int)(sizeof(_shr_res_bitmap_data_t) +
                                SHR_BITALLOCSIZE(count) -
                                sizeof(SHR_BITDCL))));
        sal_free(*desc);
        *desc = NULL;
        return BCM_E_MEMORY;
    }
    sal_memset((*desc)->resHandle,
               0x00,
               sizeof(_shr_res_bitmap_data_t) +
               SHR_BITALLOCSIZE(count) -
               sizeof(SHR_BITDCL));
    /* all's well if we got here */
    return BCM_E_NONE;
}

static int
_shr_res_bitmap_destroy(_shr_res_pool_desc_t *desc)
{
    /* free the data */
    sal_free(desc->resHandle);
    /* free the descriptor */
    sal_free(desc);
    /* that usually crashes if something is wrong */
    return BCM_E_NONE;
}

static int
_shr_res_bitmap_alloc(_shr_res_pool_desc_t *desc,
                      uint32 flags,
                      int count,
                      int *elem)
{
    int index;
    int offset;
    int result = BCM_E_NONE;
    _shr_res_bitmap_data_t *data = desc->resHandle;

    if (flags & SHR_RES_ALLOC_WITH_ID) {
        /* WITH_ID, so only try the specifically requested block */
        if (*elem < desc->low) {
            /* not valid ID */
            result = BCM_E_PARAM;
        }
        index = *elem - desc->low;
        if (index + count > desc->count) {
            /* not valid ID */
            result = BCM_E_PARAM;
        }
        if (BCM_E_NONE == result) {
            /* check whether the block is free */
            for (offset = 0; offset < count; offset++) {
                if (SHR_BITGET(data->data, index + offset)) {
                    /* in use; can't do WITH_ID alloc of the block */
                    result = BCM_E_RESOURCE;
                    break;
                }
            } /* for (offset = 0; offset < count; offset++) */
        }
        /* don't adjust last free or next alloc for WITH_ID */
    } else { /* if (flags & SHR_RES_ALLOC_WITH_ID) */
        /* see if there are enough elements after last free */
        index = data->lastFree;
        if (index + count < desc->count) {
            /* it might fit */
            for (offset = 0; offset < count; offset++) {
                if (SHR_BITGET(data->data, index + offset)) {
                    result = BCM_E_EXISTS;
                    break;
                }
            }
        } else {
            result = BCM_E_EXISTS;
        }
        if (BCM_E_NONE == result) {
            /* looks good; adjust last free to miss this block */
            data->lastFree = index + count;
        } else { /* if (BCM_E_NONE == result) */
            /* start searching after last successful alloc */
            index = data->nextAlloc;
            while (index <= desc->count - count) {
                while (SHR_BITGET(data->data, index) &&
                       (index > desc->count - count)) {
                    index++;
                }
                if (index <= desc->count - count) {
                    /* have a candidate; see if block is big enough */
                    result = BCM_E_NONE;
                    for (offset = 0; offset < count; offset++) {
                        if (SHR_BITGET(data->data, index + offset)) {
                            /* not big enough; skip this block */
                            result = BCM_E_EXISTS;
                            index += offset + 1;
                            break;
                        }
                    } /* for (offset = 0; offset < count; offset++) */
                } /* if (index <= desc->count - count) */
                if (BCM_E_NONE == result) {
                    /* found a sufficient block */
                    break;
                }
            } /* while (index <= desc->count - count) */
            if (BCM_E_NONE != result) {
                /* no space, so try space before last successful alloc */
                index = 0;
                while (index < data->nextAlloc) {
                    while (SHR_BITGET(data->data, index) &&
                           (index < data->nextAlloc)) {
                        index++;
                    }
                    if (index < data->nextAlloc) {
                        /* have a candidate; see if block is big enough */
                        result = BCM_E_NONE;
                        for (offset = 0; offset < count; offset++) {
                            if (SHR_BITGET(data->data, index + offset)) {
                                /* not big enough; skip this block */
                                result = BCM_E_EXISTS;
                                index += offset + 1;
                                break;
                            }
                        } /* for (offset = 0; offset < count; offset++) */
                    } /* if (index < data->next_alloc) */
                    if (BCM_E_NONE == result) {
                        /* found a sufficient block */
                        break;
                    }
                } /* while (index < data->next_alloc) */
            } /* if (BCM_E_NONE != result) */
            if (BCM_E_NONE == result) {
                /* got some space; update next alloc  */
                data->nextAlloc = index + count;
            } else {
                /* still no space; give up */
                result = BCM_E_RESOURCE;
            }
        } /* if (BCM_E_NONE == result) */
    } /* if (flags & SHR_RES_ALLOC_WITH_ID) */
    if (BCM_E_NONE == result) {
        /* return the beginning element */
        *elem = index + desc->low;
        /* mark the block as in use */
        for (offset = 0; offset < count; offset++) {
            SHR_BITSET(data->data, index + offset);
        }
    } /* if (BCM_E_NONE == result) */
    return result;
}

static int
_shr_res_bitmap_alloc_align(_shr_res_pool_desc_t *desc,
                            uint32 flags,
                            int align,
                            int offs,
                            int count,
                            int *elem)
{
    int index;
    int offset;
    int result = BCM_E_NONE;
    _shr_res_bitmap_data_t *data = desc->resHandle;

    if (flags & SHR_RES_ALLOC_WITH_ID) {
        /* WITH_ID, so only try the specifically requested block */
        if (*elem < desc->low) {
            /* not valid ID */
            result = BCM_E_PARAM;
        }
        index = *elem - desc->low;
        if (index + count > desc->count) {
            /* not valid ID */
            result = BCM_E_PARAM;
        }
        if (BCM_E_NONE == result) {
            /* check whether the block is free */
            for (offset = 0; offset < count; offset++) {
                if (SHR_BITGET(data->data, index + offset)) {
                    /* in use; can't do WITH_ID alloc of the block */
                    result = BCM_E_RESOURCE;
                    break;
                }
            } /* for (offset = 0; offset < count; offset++) */
        }
        /* don't adjust last free or next alloc for WITH_ID */
    } else { /* if (flags & SHR_RES_ALLOC_WITH_ID) */
        if (flags & SHR_RES_ALLOC_ALIGN_ZERO) {
            /* alignment is against zero, not start of pool */
            offs += align - (desc->low % align);
        }
        /* see if there are enough elements after last free */
        index = (((data->lastFree + align - 1) / align) * align) + offs;
        if (index + count < desc->count) {
            /* it might fit */
            for (offset = 0; offset < count; offset++) {
                if (SHR_BITGET(data->data, index + offset)) {
                    result = BCM_E_EXISTS;
                    break;
                }
            }
        } else {
            result = BCM_E_EXISTS;
        }
        if (BCM_E_NONE == result) {
            /* looks good; adjust last free to miss this block */
            if (0 == offs) {
                data->lastFree = index + count;
            }
        } else { /* if (BCM_E_NONE == result) */
            /* start searching after last successful alloc */
            index = (((data->nextAlloc + align - 1) / align) * align) + offs;
            while (index <= desc->count - count) {
                while (SHR_BITGET(data->data, index) &&
                       (index > desc->count - count)) {
                    index += align;
                }
                if (index <= desc->count - count) {
                    /* have a candidate; see if block is big enough */
                    result = BCM_E_NONE;
                    for (offset = 0; offset < count; offset++) {
                        if (SHR_BITGET(data->data, index + offset)) {
                            /* not big enough; skip this block */
                            result = BCM_E_EXISTS;
                            index = (((index + offset + align) / align) * align) + offs;
                            break;
                        }
                    } /* for (offset = 0; offset < count; offset++) */
                } /* if (index <= desc->count - count) */
                if (BCM_E_NONE == result) {
                    /* found a sufficient block */
                    break;
                }
            } /* while (index <= desc->count - count) */
            if (BCM_E_NONE != result) {
                /* no space, so try space before last successful alloc */
                index = offs;
                while (index < data->nextAlloc) {
                    while (SHR_BITGET(data->data, index) &&
                           (index < data->nextAlloc)) {
                        index += align;
                    }
                    if (index < data->nextAlloc) {
                        /* have a candidate; see if block is big enough */
                        result = BCM_E_NONE;
                        for (offset = 0; offset < count; offset++) {
                            if (SHR_BITGET(data->data, index + offset)) {
                                /* not big enough; skip this block */
                                result = BCM_E_EXISTS;
                                index = (((index + offset + align) / align) * align) + offs;
                                break;
                            }
                        } /* for (offset = 0; offset < count; offset++) */
                    } /* if (index < data->next_alloc) */
                    if (BCM_E_NONE == result) {
                        /* found a sufficient block */
                        break;
                    }
                } /* while (index < data->next_alloc) */
            } /* if (BCM_E_NONE != result) */
            if (BCM_E_NONE == result) {
                /* got some space; update next alloc  */
                data->nextAlloc = index + count;
            } else {
                /* still no space; give up */
                result = BCM_E_RESOURCE;
            }
        } /* if (BCM_E_NONE == result) */
    } /* if (flags & SHR_RES_ALLOC_WITH_ID) */
    if (BCM_E_NONE == result) {
        /* return the beginning element */
        *elem = index + desc->low;
        /* mark the block as in use */
        for (offset = 0; offset < count; offset++) {
            SHR_BITSET(data->data, index + offset);
        }
    } /* if (BCM_E_NONE == result) */
    return result;
}

static int
_shr_res_bitmap_free(_shr_res_pool_desc_t *desc,
                     int count,
                     int elem)
{
    int index;
    int offset;
    int result = BCM_E_NONE;
    _shr_res_bitmap_data_t *data = desc->resHandle;

    /* check arguments */
    if (elem < desc->low) {
        /* not valid ID */
        result = BCM_E_PARAM;
    }
    index = elem - desc->low;
    if (index + count > desc->count) {
        /* not valid ID */
        result = BCM_E_PARAM;
    }
    if (BCM_E_NONE == result) {
        /* check whether the block is in use */
        for (offset = 0; offset < count; offset++) {
            if (!SHR_BITGET(data->data, index + offset)) {
                /* not entirely in use */
                result = BCM_E_NOT_FOUND;
                break;
            }
        } /* for (offset = 0; offset < count; offset++) */
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE == result) {
        /* looks fine, so mark the block as free */
        for (offset = 0; offset < count; offset++) {
            SHR_BITCLR(data->data, index + offset);
        }
        data->lastFree = index;
    } /* if (BCM_E_NONE == result) */
    /* return the result */
    return result;
}

static int
_shr_res_bitmap_check(_shr_res_pool_desc_t *desc,
                      int count,
                      int elem)
{
    int index;
    int offset;
    int result = BCM_E_NONE;
    _shr_res_bitmap_data_t *data = desc->resHandle;

    /* check arguments */
    if (elem < desc->low) {
        /* not valid ID */
        result = BCM_E_PARAM;
    }
    index = elem - desc->low;
    if (index + count > desc->count) {
        /* not valid ID */
        result = BCM_E_PARAM;
    }
    if (BCM_E_NONE == result) {
        /* check whether the block is in use */
        result = BCM_E_NOT_FOUND;
        for (offset = 0; offset < count; offset++) {
            if (SHR_BITGET(data->data, index + offset)) {
                /* not entirely free */
                result = BCM_E_EXISTS;
                break;
            }
        } /* for (offset = 0; offset < count; offset++) */
    } /* if (BCM_E_NONE == result) */
    /* return the result */
    return result;
}

/*****************************************************************************/
/*
 *  Interface to idxres (external implementation)
 */

/*
 *  idxres_fl is a folded freelist implementation that has roughly O(1) alloc,
 *  free, and check times, at the cost of about 8.1 bits per element.  It does
 *  not deal well with blocks of elements (unless everything works by a
 *  specific number of elements, and so can scale the list), though, and should
 *  not be asked to allocate blocks except WITH_ID, which unhappily has more
 *  like O(log128(n)) operation time where n is elements in the list.
 *
 *  When WITH_ID is provided, we can use the reserve function to specify the
 *  block requested, but in other cases, count must equal the scale else we
 *  will return BCM_E_PARAM.
 *
 *  Since idxres checks arguments, fewer explicit argument checks are performed
 *  by these function than, for example, by the bitmap implementation above.
 */

static int
_shr_res_idxres_create(_shr_res_pool_desc_t **desc,
                       int low_id,
                       int count,
                       const void* extras,
                       const char* name)
{
    int result;
    const shr_res_idxres_extras_t *info = extras;
    int namesize = ((sal_strlen(name) + 3) & (~3)); /* align to quadbyte */
    int nodesize = namesize + sizeof(**desc) + sizeof(*info);

    /* need the base descriptor */
    *desc = sal_alloc(nodesize, "idxres resource descriptor");
    if (!(*desc)) {
        /* alloc failed */
        RES_ERR((RES_MSG1("unable to allocate %d bytes for descriptor\n"),
                 nodesize));
        return BCM_E_MEMORY;
    }
    sal_memset(*desc, 0x00, nodesize);
    (*desc)->count = count;
    (*desc)->low = low_id;
    (*desc)->extras = &((*desc)->name[namesize]);
    sal_strcpy(&((*desc)->name[0]), name);
    if (info) {
        if ((1 > info->scaling_factor)) {
            /* negative or zero scaling factors are ignored */
            RES_WARN((RES_MSG1("invalid scaling factor %d; using 1 instead\n"),
                      info->scaling_factor));
            ((shr_res_idxres_extras_t*)((*desc)->extras))->scaling_factor = 1;
        } else {
            ((shr_res_idxres_extras_t*)((*desc)->extras))->scaling_factor = info->scaling_factor;
        }
    } else {
        /* no scaling factor provided */
        RES_WARN((RES_MSG1("missing scaling factor; using 1\n")));
        ((shr_res_idxres_extras_t*)((*desc)->extras))->scaling_factor = 1;
    }
    if (1 == ((shr_res_idxres_extras_t*)((*desc)->extras))->scaling_factor) {
        result = shr_idxres_list_create((shr_idxres_list_handle_t*)&((*desc)->resHandle),
                                        low_id,
                                        low_id + count - 1,
                                        low_id,
                                        low_id + count - 1,
                                        "managed idxres");
    } else {
        result = shr_idxres_list_create_scaled((shr_idxres_list_handle_t*)&((*desc)->resHandle),
                                               low_id,
                                               low_id + count - 1,
                                               low_id,
                                               low_id + count - 1,
                                               ((shr_res_idxres_extras_t*)((*desc)->extras))->scaling_factor,
                                               "managed idxres (scaled)");
    }
    if (BCM_E_NONE != result) {
        /* creation failed */
        RES_ERR((RES_MSG1("unable to create idxres(%d,%d,%d,%d,%d): %d (%s)\n"),
                 low_id,
                 low_id + count - 1,
                 low_id,
                 low_id + count - 1,
                 ((shr_res_idxres_extras_t*)((*desc)->extras))->scaling_factor,
                 result,
                 _SHR_ERRMSG(result)));
        sal_free(*desc);
        *desc = NULL;
    }
    /* return the result */
    return result;
}

static int
_shr_res_idxres_destroy(_shr_res_pool_desc_t *desc)
{
    int result;

    result = shr_idxres_list_destroy((shr_idxres_list_handle_t)(desc->resHandle));
    if (BCM_E_NONE == result) {
        sal_free(desc);
    }
    return result;
}

static int
_shr_res_idxres_alloc(_shr_res_pool_desc_t *desc,
                      uint32 flags,
                      int count,
                      int *elem)
{
    int result;
    const shr_res_idxres_extras_t *info = desc->extras;
    shr_idxres_list_handle_t handle = desc->resHandle;
    shr_idxres_element_t item;

    if (SHR_RES_ALLOC_WITH_ID & flags) {
        /* allocate WITH_ID */
        result = shr_idxres_list_reserve(handle,
                                         *elem,
                                         (*elem) + count - 1);
    } else {
        /* allocate next available */
        if (count > info->scaling_factor) {
            RES_ERR((RES_MSG1("tried to allocate %d elements from idxres list"
                              " of scaling_factor %d\n"),
                     count,
                     info->scaling_factor));
            return BCM_E_PARAM;
        }
        result = shr_idxres_list_alloc(handle, &item);
        if (BCM_E_NONE == result) {
            *elem = item;
        }
    }
    return result;
}

static int
_shr_res_idxres_free(_shr_res_pool_desc_t *desc,
                     int count,
                     int elem)
{
    int result = BCM_E_NONE;
    const shr_res_idxres_extras_t *info = desc->extras;
    shr_idxres_list_handle_t handle = desc->resHandle;

    while ((BCM_E_NONE == result) && (count > 0)) {
        result = shr_idxres_list_free(handle, elem);
        elem += info->scaling_factor;
        count -= info->scaling_factor;
    }
    if (BCM_E_RESOURCE == result) {
        /* return NOT_FOUND instead */
        result = BCM_E_NOT_FOUND;
    }
    return result;
}

static int
_shr_res_idxres_check(_shr_res_pool_desc_t *desc,
                      int count,
                      int elem)
{
    int result = BCM_E_NOT_FOUND;
    shr_idxres_list_handle_t handle = desc->resHandle;

    while ((BCM_E_NOT_FOUND == result) && (0 < count)) {
        result = shr_idxres_list_elem_state(handle, elem);
        elem++;
        count--;
    }
    return result;
}

/*****************************************************************************/
/*
 *  Interface to aidxres (external implementation)
 */

/*
 *  aidxres_fl is a folded freelist implementation that has roughly O(1) alloc,
 *  free, and check times, at the cost of about 16.2 bits per element.  It
 *  deals reasonably well with blocks of elements but ends up aligning them to
 *  the next higher power of two (unless they are exactly a power of two
 *  already, then that size is also the alignment).  It suffers similar issues
 *  to idxres when used WITH_ID, but not quite at such a performance loss.
 *  Blocks will be aligned unless they are allocated WITH_ID, then it places
 *  the block as requested.
 *
 *  Note that the blocking factor has some effect on the performance; larger
 *  blocking factor means also larger memory footprint (more sublists).
 *  Despite these limitations, the default blocking factor is 7 if it is not
 *  provided by the caller, as this is the largest value supported in all of
 *  the normal operation modes (4b is *not* normal operation), and provides the
 *  ability to manage blocks up to 128 elements in size.
 *
 *  Since aidxres checks arguments, fewer argument checks are performed by
 *  these function than, for example, by the bitmap implementation above.
 */

static int
_shr_res_aidxres_create(_shr_res_pool_desc_t **desc,
                        int low_id,
                        int count,
                        const void* extras,
                        const char* name)
{
    int result;
    const shr_res_aidxres_extras_t *info = extras;
    int namesize = ((sal_strlen(name) + 3) & (~3)); /* align to quadbyte */
    int nodesize = namesize + sizeof(**desc) + sizeof(*info);

    /* need the base descriptor */
    *desc = sal_alloc(nodesize, "aidxres resource descriptor");
    if (!(*desc)) {
        /* alloc failed */
        RES_ERR((RES_MSG1("unable to allocate %d bytes for descriptor\n"),
                 nodesize));
        return BCM_E_MEMORY;
    }
    sal_memset(*desc, 0x00, nodesize);
    (*desc)->count = count;
    (*desc)->low = low_id;
    (*desc)->extras = &((*desc)->name[namesize]);
    sal_strcpy(&((*desc)->name[0]), name);
    if (info) {
        if ((1 >= info->blocking_factor)) {
            /* negative or zero scaling factors are ignored */
            RES_WARN((RES_MSG1("invalid blocking factor %d; using 7 instead\n"),
                      info->blocking_factor));
            ((shr_res_aidxres_extras_t*)((*desc)->extras))->blocking_factor = 7;
        } else {
            ((shr_res_aidxres_extras_t*)((*desc)->extras))->blocking_factor = info->blocking_factor;
        }
    } else {
        /* no scaling factor provided */
        RES_WARN((RES_MSG1("missing blocking factor; using 7\n")));
        ((shr_res_aidxres_extras_t*)((*desc)->extras))->blocking_factor = 7;
    }
    result = shr_aidxres_list_create((shr_aidxres_list_handle_t*)&((*desc)->resHandle),
                                     low_id,
                                     low_id + count - 1,
                                     low_id,
                                     low_id + count - 1,
                                     ((shr_res_aidxres_extras_t*)((*desc)->extras))->blocking_factor,
                                     "managed aidxres");
    if (BCM_E_NONE != result) {
        /* creation failed */
        RES_ERR((RES_MSG1("unable to create aidxres(%d,%d,%d,%d,%d): %d (%s)\n"),
                 low_id,
                 low_id + count - 1,
                 low_id,
                 low_id + count - 1,
                 ((shr_res_aidxres_extras_t*)((*desc)->extras))->blocking_factor,
                 result,
                 _SHR_ERRMSG(result)));
        sal_free(*desc);
        *desc = NULL;
    }
    /* return the result */
    return result;
}

static int
_shr_res_aidxres_destroy(_shr_res_pool_desc_t *desc)
{
    int result;

    result = shr_aidxres_list_destroy((shr_aidxres_list_handle_t)(desc->resHandle));
    if (BCM_E_NONE == result) {
        sal_free(desc);
    }
    return result;
}

static int
_shr_res_aidxres_alloc(_shr_res_pool_desc_t *desc,
                       uint32 flags,
                       int count,
                       int *elem)
{
    int result;
    const shr_res_aidxres_extras_t *info = desc->extras;
    shr_aidxres_list_handle_t handle = desc->resHandle;
    shr_aidxres_element_t item;

    if (SHR_RES_ALLOC_WITH_ID & flags) {
        /* allocate WITH_ID */
        result = shr_aidxres_list_reserve(handle,
                                          *elem,
                                          (*elem) + count - 1);
    } else {
        /* allocate next available */
        if (count > (2 << info->blocking_factor)) {
            RES_ERR((RES_MSG1("tried to allocate %d elements from idxres list"
                              " with blocking_factor %d\n"),
                     count,
                     info->blocking_factor));
            return BCM_E_PARAM;
        }
        if (count > 1) {
            result = shr_aidxres_list_alloc_block(handle, count, &item);
        } else {
            result = shr_aidxres_list_alloc(handle, &item);
        }
        if (BCM_E_NONE == result) {
            *elem = item;
        }
    }
    return result;
}

static int
_shr_res_aidxres_free(_shr_res_pool_desc_t *desc,
                      int count,
                      int elem)
{
    int result = BCM_E_NONE;
    int xresult;
    shr_aidxres_list_handle_t handle = desc->resHandle;

    result = shr_aidxres_list_free(handle, elem);
    /*
     *  If it was a single block that was allocated, then it is all free, but
     *  if it was more than one block, or if it was marked by 'reserve' (so
     *  WITH_ID), then we need to traverse it.  Since we're freeing blocks as
     *  we come to them, any members of those blocks will already be free after
     *  the first element, but there isn't a way to know how many elements were
     *  in the block.  Basically, we need to skip any elements after the first
     *  that are marked NOT_FOUND (not in use) as if nothing was wrong.
     */
    if (BCM_E_NONE == result) {
        count--;
        elem++;
        xresult = BCM_E_NONE;
        while (((BCM_E_NONE == xresult) || (BCM_E_RESOURCE == xresult)) &&
               (count > 0)) {
            xresult = shr_aidxres_list_free(handle, elem);
            if ((BCM_E_NONE != xresult) && (BCM_E_RESOURCE != xresult)) {
                RES_ERR((RES_MSG1("element %d unable to free: %d (%s)\n"),
                         elem,
                         xresult,
                         _SHR_ERRMSG(xresult)));
                result = xresult;
            }
            elem++;
            count--;
        }
    }
    if (BCM_E_RESOURCE == result) {
        /* return NOT_FOUND instead */
        result = BCM_E_NOT_FOUND;
    }
    return result;
}

static int
_shr_res_aidxres_check(_shr_res_pool_desc_t *desc,
                       int count,
                       int elem)
{
    int result = BCM_E_NOT_FOUND;
    shr_aidxres_list_handle_t handle = desc->resHandle;

    while ((BCM_E_NOT_FOUND == result) && (0 < count)) {
        result = shr_aidxres_list_elem_state(handle, elem);
        elem++;
        count--;
    }
    return result;
}

/*****************************************************************************/
/*
 *  Interface to mdb (external implementation)
 */

/*
 *  MDB has a lot of features that are not exposed through this interface, but
 *  it does improve upon aidxres for allowing greater versatility in terms of
 *  not requiring alignment of multiple element blocks, and it can track
 *  variable (non-power-of-two) size free block lists, in order to optimise
 *  allocation of certain sizes of blocks.  Its performance will be slightly
 *  worse than aidxres because mdb likes to combine neighbouring free blocks
 *  when allocating and freeing, and this causes roughly O(logm(b)) where m is
 *  the number of free lists additional overhead after the primary one and b is
 *  the size of the banks.  Note that poor choice of free list sizes (such as
 *  only using the mandatory single-element free list) will significantly
 *  imparir performance, degrading it as far as O(c*b) where c is the number of
 *  elements in a block.
 *
 *  The overhead for mdb is fairly close to 32.5 bits per element.  This may
 *  sound like rather a lot, but if you consider it against a traditional
 *  linked list, which requires an alloc cell per block (so 16 bytes of memory
 *  per block) and can't do query in O(1) time or best-fit as described above,
 *  it's still worthwhile in case where there are large resources to manage.
 *
 *  The mdb interface here also offers a lot more sanity checking than most of
 *  the other mechanisms.  This is because mdb allows information to be
 *  retrieved about a block of elements, and so free() and check() call
 *  parameters can be validated very specifically.
 *
 *  In order to take advantage of the other features of mdb, such as user
 *  lists, live adjustments to allocation mechanism, and so on, the mdb code
 *  should be used directly.
 */

static int
_shr_res_mdb_create(_shr_res_pool_desc_t **desc,
                    int low_id,
                    int count,
                    const void* extras,
                    const char* name)
{
    int result = BCM_E_NONE;
    const shr_res_mdb_extras_t *info = extras;
    shr_res_mdb_extras_t *intInfo;
    int namesize = ((sal_strlen(name) + 3) & (~3)); /* align to quadbyte */
    int nodesize = namesize + sizeof(**desc) + sizeof(*info);
    int index;

    /* need the base descriptor */
    *desc = sal_alloc(nodesize, "mdb resource descriptor");
    if (!(*desc)) {
        /* alloc failed */
        RES_ERR((RES_MSG1("unable to allocate %d bytes for descriptor\n"),
                 nodesize));
        return BCM_E_MEMORY;
    }
    sal_memset(*desc, 0x00, nodesize);
    (*desc)->count = count;
    (*desc)->low = low_id;
    (*desc)->extras = &((*desc)->name[namesize]);
    intInfo = (shr_res_mdb_extras_t*)((*desc)->extras);
    sal_strcpy(&((*desc)->name[0]), name);
    if (info) {
        /* Just copy the settings; the mdb manager will check things */
        intInfo->bank_size = info->bank_size;
        intInfo->free_lists = info->free_lists;
        for (index = 0; index < info->free_lists; index++) {
            intInfo->free_counts[index] = info->free_counts[index];
        }
    } else {
        /* no settings provided; pick a reasonable(?) default set */
        RES_WARN((RES_MSG1("missing extra information; using defaults\n")));
        intInfo->bank_size = 4096;
        intInfo->free_lists = 12;
        intInfo->free_counts[0] = 2;
        intInfo->free_counts[1] = 4;
        intInfo->free_counts[2] = 8;
        intInfo->free_counts[3] = 16;
        intInfo->free_counts[4] = 32;
        intInfo->free_counts[5] = 64;
        intInfo->free_counts[6] = 128;
        intInfo->free_counts[7] = 256;
        intInfo->free_counts[8] = 512;
        intInfo->free_counts[9] = 1024;
        intInfo->free_counts[10] = 2048;
        intInfo->free_counts[11] = 4096;
    }
    result = shr_mdb_create((shr_mdb_list_handle_t*)&((*desc)->resHandle),
                            intInfo->bank_size,
                            intInfo->free_lists,
                            &(intInfo->free_counts[0]),
                            0 /* user lists */,
                            low_id,
                            low_id + count - 1,
                            FALSE /* no implied locking */);
    if (BCM_E_NONE != result) {
        /* creation failed */
        RES_ERR((RES_MSG1("unable to create mdb(%d,%d,%d,%d,...): %d (%s)\n"),
                 low_id,
                 low_id + count - 1,
                 intInfo->bank_size,
                 intInfo->free_lists,
                 result,
                 _SHR_ERRMSG(result)));
        sal_free(*desc);
        *desc = NULL;
    } else {
        /* set the thing to use a reasonable alloc mode */
        result = shr_mdb_allocmode_set((shr_mdb_list_handle_t)((*desc)->resHandle),
                                       shr_mdb_alloc_bank_first |
                                       shr_mdb_alloc_block_low |
                                       shr_mdb_free_block_low |
                                       shr_mdb_join_alloc_and_free |
                                       shr_mdb_join_high_and_low);
    }
    /* return the result */
    return result;
}

static int
_shr_res_mdb_destroy(_shr_res_pool_desc_t *desc)
{
    int result;

    result = shr_mdb_destroy((shr_mdb_list_handle_t)(desc->resHandle));
    if (BCM_E_NONE == result) {
        sal_free(desc);
    }
    return result;
}

static int
_shr_res_mdb_alloc(_shr_res_pool_desc_t *desc,
                   uint32 flags,
                   int count,
                   int *elem)
{
    shr_mdb_list_handle_t handle = desc->resHandle;
    shr_mdb_elem_index_t item;
    int result;

    if (SHR_RES_ALLOC_WITH_ID & flags) {
        /* allocate WITH_ID */
        /*
         *  More complicated than idxres, aidxress.  For starters, mdb honours
         *  BCM tendency to declare BCM_E_NOT_FOUND when it would be more
         *  helpful for BCM_E_PARAM -- trying to go beyond the end of a list.
         *  Therefore we need to check and provide the results documented for
         *  this API.
         */
        if (((*elem) < desc->low) || ((*elem) + count > desc->low + desc->count)) {
            return BCM_E_PARAM;
        }
        result = shr_mdb_alloc_id(handle, *elem, count);
    } else {
        /* allocate next available */
        result = shr_mdb_alloc(handle, &item, count);
        if (BCM_E_NONE == result) {
            *elem = item;
        }
    }
    return result;
}

static int
_shr_res_mdb_free(_shr_res_pool_desc_t *desc,
                  int count,
                  int elem)
{
    int result;
    shr_mdb_list_handle_t handle = desc->resHandle;
    shr_mdb_block_info_t info;

    /*
     *  More complicated than idxres, aidxress.  For starters, mdb honours BCM
     *  tendency to declare BCM_E_NOT_FOUND when it would be more helpful for
     *  BCM_E_PARAM -- trying to go beyond the end of a list.  Therefore we
     *  need to check and provide the results documented for this API.
     */
    if ((elem < desc->low) || (elem + count > desc->low + desc->count)) {
        return BCM_E_PARAM;
    }
    /*
     *  Life gets interesting now.  In idxres and aidxres and even bitmap
     *  models, we had no way of getting the block size back from the starting
     *  element.  Here we do have that ability.  Ensure the block is the
     *  claimed size before freeing it.
     */
    result = shr_mdb_block_info(handle, elem, &info);
    if (BCM_E_NONE == result) {
        if (info.size != count) {
            RES_ERR((RES_MSG1("freeing block size %d but claimed %d\n"),
                     info.size,
                     count));
            result = BCM_E_FAIL;
        }
        if (info.head != (uint32)elem) {
            RES_ERR((RES_MSG1("freeing block with head %d by non-head"
                              " element %d\n"),
                     info.head,
                     elem));
            result = BCM_E_FAIL;
        }
        if (BCM_E_NONE == result) {
            result = shr_mdb_free(handle, elem);
        }
    }
    return result;
}

static int
_shr_res_mdb_check(_shr_res_pool_desc_t *desc,
                   int count,
                   int elem)
{
    int result = BCM_E_NOT_FOUND;
    shr_mdb_list_handle_t handle = desc->resHandle;
    shr_mdb_block_info_t info;

    /*
     *  More complicated than idxres, aidxress.  For starters, mdb honours BCM
     *  tendency to declare BCM_E_NOT_FOUND when it would be more helpful for
     *  BCM_E_PARAM -- trying to go beyond the end of a list.  Therefore we
     *  need to check and provide the results documented for this API.
     */
    if ((elem < desc->low) || (elem + count > desc->low + desc->count)) {
        return BCM_E_PARAM;
    }
    /*
     *  Since this is meant to be a probe for elements in use, we don't want to
     *  simply bail out if the caller requests a block that is different in
     *  size than one that already exists, or if the caller's block is a member
     *  of an existing block that started elsewhere.  Just scan for elements
     *  that are in use and bail for those, otherwise keep going.
     */
    while ((count > 0) && (BCM_E_NOT_FOUND == result)) {
        result = shr_mdb_block_info(handle, elem, &info);
        count--;
        elem++;
    }
    if (BCM_E_NONE == result) {
        /* shr_mdb_block_info returns BCM_E_NONE if it finds a block */
        result = BCM_E_EXISTS;
    }
    return result;
}
