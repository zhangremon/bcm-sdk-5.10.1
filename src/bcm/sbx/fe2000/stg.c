/*
 * $Id: stg.c 1.41 Broadcom SDK $
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
 * File:        stg.c
 * Purpose:     Spanning tree group support
 *
 * Multiple spanning trees (MST) is supported on this chipset
 */

#include <shared/bitop.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbStatus.h>

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stg.h>
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/recovery.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <bcm_int/sbx/fe2000/g2p3.h>
#endif


/*   Version 1.0 of the scache layout for oam level-2 recovery
 *  The version is stored in state[u]->wb_cache->version
 *  This memory map is applied to state[u]->wb_cache->cache;
 *  This memory map is used to define the layout and amount of memory 
 *    allocated for the purposes of warmboot - if its not here - 
 *    it's not stored.
 */
typedef struct fe2k_stg_wb_mem_layout_s {

    uint16   stg_map[BCM_VLAN_COUNT];  /* Map VLAN->STG */

    struct {
        uint8 e[(SBX_MAX_PORTS + 7) / 8];  /* port enable per STG            */
        uint8 hl[(SBX_MAX_PORTS + 7) / 8]; /* stp state; listen(1), or 
                                            * recover from hardware(0)       */
    } e_state[BCM_STG_MAX];             
        
} fe2k_stg_wb_mem_layout_t;

#define STG_WB_VERSION_1_0        SOC_SCACHE_VERSION(1,0)
#define STG_WB_CURRENT_VERSION    STG_WB_VERSION_1_0


#ifdef BCM_WARM_BOOT_SUPPORT

#define WB_E_STATE(lo__, stg__)  ((lo__)->e_state[stg__])


#define WB_E_STATE_VAR_CLEAR(lo__, var, stg__, port__)  \
  WB_E_STATE(lo__, stg__).var[(port__) / 8] &= ~(1 << ((port__) & 7))
#define WB_E_STATE_VAR_SET(lo__, var, stg__, port__)  \
  WB_E_STATE(lo__, stg__).var[(port__) / 8] |= (1 << ((port__) & 7))
#define WB_E_STATE_VAR_GET(lo__, var, stg__, port__)  \
  (!!  (WB_E_STATE(lo__, stg__).var[(port__) / 8] & (1 << ((port__) & 7))))


#define WB_STATE_ENABLE_CLEAR(lo__, stg__, port__)       \
    WB_E_STATE_VAR_CLEAR(lo__, e, stg__, port__)
#define WB_STATE_ENABLE_SET(lo__, stg__, port__)         \
    WB_E_STATE_VAR_SET(lo__, e, stg__, port__)
#define WB_STATE_ENABLE_GET(lo__, stg__, port__)         \
    WB_E_STATE_VAR_GET(lo__, e, stg__, port__)


#define WB_STATE_READ_FROM_HW      0
#define WB_STATE_LISTEN            1

#define WB_STATE_READ_HW_SET(lo__, stg__, port__)         \
    WB_E_STATE_VAR_CLEAR(lo__, hl, stg__, port__)
#define WB_STATE_LISTEN_SET(lo__, stg__, port__)         \
    WB_E_STATE_VAR_SET(lo__, hl, stg__, port__)
#define WB_STATE_READ_HW_GET(lo__, stg__, port__)         \
    (WB_E_STATE_VAR_GET(lo__, hl, stg__, port__) == WB_STATE_READ_FROM_HW)
#define WB_STATE_LISTEN_GET(lo__, stg__, port__)         \
    (WB_E_STATE_VAR_GET(lo__, hl, stg__, port__) == WB_STATE_LISTEN)

#else  /* BCM_WARM_BOOT_SUPPORT */

#define WB_STATE_ENABLE_CLEAR(lo__, stg__, port__)
#define WB_STATE_ENABLE_SET(lo__, stg__, port__)
#define WB_STATE_ENABLE_GET(lo__, stg__, port__)

#endif /* BCM_WARM_BOOT_SUPPORT */


/*
 * The STG info structure is protected by STG_DB_LOCK. The hardware PTABLE and
 * hardware STG table are protected by memory locks in the lower level.
 */
typedef struct bcm_stg_info_s {
    int         init;       /* TRUE if STG module has been initialized */
    sal_mutex_t lock;       /* Database lock */
    bcm_stg_t   stg_min;    /* STG table min index */
    bcm_stg_t   stg_max;    /* STG table max index */
    bcm_stg_t   stg_defl;   /* Default STG */
    SHR_BITDCL *stg_bitmap; /* Bitmap of allocated STGs */
    bcm_pbmp_t *stg_enable; /* array of port bitmaps indicating whether the
                               port+stg has STP enabled */
    bcm_pbmp_t *stg_state_h;/* array of port bitmaps indicating STP state for the */
    bcm_pbmp_t *stg_state_l;/* port+stg combo. Only valid if stg_enable = TRUE */
    int         stg_count;  /* Number STGs allocated */
    /* STG reverse map - keep a linked list of VLANs in each STG */
    bcm_vlan_t *vlan_first; /* Indexed by STG (also links free list) */
    bcm_vlan_t *vlan_next;  /* Indexed by VLAN ID */
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32      wb_hdl;
    uint32      wb_size;
#endif /* BCM_WARM_BOOT_SUPPORT */

} bcm_stg_info_t;

static  bcm_stg_info_t  stg_info[BCM_MAX_NUM_UNITS];

int bcm_fe2000_stg_destroy(int unit, bcm_stg_t stg);
int bcm_fe2000_stg_init(int unit);

#define STG_LEVEL_ONE  1

#define STG_DB_LOCK(unit) \
        sal_mutex_take(stg_info[(unit)].lock, sal_mutex_FOREVER)

#define STG_DB_UNLOCK(unit) sal_mutex_give(stg_info[(unit)].lock)

#define STG_CNTL(unit)  stg_info[(unit)]

#define STG_CHECK_INIT(unit)                                           \
    do {                                                                \
        if (!BCM_UNIT_VALID(unit)) return BCM_E_UNIT;                  \
        if (unit >= BCM_MAX_NUM_UNITS) return BCM_E_UNIT;              \
        if (STG_CNTL(unit).init == FALSE) return BCM_E_INIT;           \
        if (STG_CNTL(unit).init != TRUE) return STG_CNTL(unit).init;  \
    } while (0);

#define STG_CHECK_STG(si, stg)                            \
    if ((stg) < (si)->stg_min || (stg) > (si)->stg_max) return BCM_E_BADID;

#define STG_CHECK_PORT(unit, port)                 \
    if (((port < 0) || (port >= SBX_MAX_PORTS))    \
        || (!IS_E_PORT((unit), (port))             \
            && !IS_HG_PORT((unit), (port))         \
            && !IS_SPI_SUBPORT_PORT((unit), (port))\
            && !IS_CPU_PORT((unit), (port)))) {    \
        return BCM_E_PORT;                         \
    }

#define G2_FE_HANDLER_GET(unit, fe)  \
    if ((fe = (sbG2Fe_t *)SOC_SBX_CONTROL(unit)->drv) == NULL) {  \
        return BCM_E_INIT;  \
    }

/*
 * Allocation bitmap macros
 */
#define STG_BITMAP_TST(si, stg)     SHR_BITGET(si->stg_bitmap, stg)
#define STG_BITMAP_SET(si, stg)     SHR_BITSET(si->stg_bitmap, stg)
#define STG_BITMAP_CLR(si, stg)     SHR_BITCLR(si->stg_bitmap, stg)

/* Debug Macros */
#define BCM_STG_ERR(stuff)          BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_ERR, stuff)
#define BCM_STG_WARN(stuff)         BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_WARN, stuff)
#define BCM_STG_VERB(stuff)         BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_VERBOSE, stuff)
#define BCM_STG_VVERB(stuff)        BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_VVERBOSE, stuff)
#define BCM_STG_MSG(stuff)          BCM_DEBUG(BCM_DBG_NORMAL, stuff)

#define NUM_EGR_PVID2ETC_WORDS BCM_VLAN_COUNT/16

static bcm_vlan_t _stg_vlan_min     = BCM_VLAN_MIN + 1;
static bcm_vlan_t _stg_vlan_max     = BCM_VLAN_MAX - 1;
static bcm_stg_t  _stg_default      = BCM_STG_DEFAULT;
static bcm_stg_t  _stg_min          = 1;
static bcm_stg_t  _stg_max          = BCM_STG_MAX;


void
_bcm_fe2000_stg_sw_dump(int unit)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg;
    bcm_vlan_t      vid;
    int             cnt, num_display_vids;

    num_display_vids = 8;

    si = &STG_CNTL(unit);
    if (FALSE == si->init) {
        soc_cm_print("Unit %d STG not initialized\n", unit);
        return;
    }

    soc_cm_print("stg_min=%d stg_max=%d stg_default=%d allocated STGs=%d\n",
                 si->stg_min, si->stg_max, si->stg_defl, si->stg_count);
    soc_cm_print("STG list:\nSTG :  VID list\n");

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {

        if (STG_BITMAP_TST(si, stg)) {
            soc_cm_print("%4d: ", stg);

            cnt = 0;
            for (vid = si->vlan_first[stg];
                 vid != BCM_VLAN_NONE;
                 vid = si->vlan_next[vid]) {
                
                if (cnt < num_display_vids) {
                    if (cnt==0) {
                        soc_cm_print("%d", vid);
                    } else {
                        soc_cm_print(", %d", vid);
                    }
                }

                cnt++;
            }

            if (cnt > num_display_vids) {
                soc_cm_print(", ... %d more", cnt - num_display_vids);
            }

            soc_cm_print("\n");
        }
    }
    soc_cm_print("\n");
    
}

#ifdef BCM_WARM_BOOT_SUPPORT
static int
_bcm_fe2k_stg_wb_layout_get(int unit, fe2k_stg_wb_mem_layout_t **layout)
{
    int rv = BCM_E_NONE;
    uint32 size;
    soc_wb_cache_t *wbc;
    bcm_stg_info_t *si;

    si = &STG_CNTL(unit);
    *layout = NULL;
    rv = soc_scache_ptr_get(unit, si->wb_hdl, (uint8**)&wbc, &size);
    if (BCM_FAILURE(rv)) {
        return rv; 
    }

    *layout = (fe2k_stg_wb_mem_layout_t*)wbc->cache;
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

static int
_bcm_fe2000_stg_map_store(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
#ifdef BCM_WARM_BOOT_SUPPORT
    bcm_stg_info_t            *si;
    fe2k_stg_wb_mem_layout_t  *layout;

    si = &STG_CNTL(unit);
    if (si->wb_size == 0) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_fe2k_stg_wb_layout_get(unit, &layout));

    soc_scache_handle_lock(unit, si->wb_hdl);
    layout->stg_map[vid] = stg;
    soc_scache_handle_unlock(unit, si->wb_hdl);

#endif /* BCM_WARM_BOOT_SUPPORT */

    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_fe2000_stg_map_add
 * Purpose:
 *      Add VLAN to STG linked list
 */
static void
_bcm_fe2000_stg_map_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;

    BCM_STG_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, vid));

    assert(BCM_VLAN_NONE != vid);

    si = &STG_CNTL(unit);

    si->vlan_next[vid] = si->vlan_first[stg];
    si->vlan_first[stg] = vid;

    _bcm_fe2000_stg_map_store(unit, stg, vid);

    BCM_STG_VVERB(("%s(%d, %d, %d) - Exit\n", FUNCTION_NAME(), unit, stg, vid));
}

/*
 * Function:
 *      _bcm_fe2000_stg_map_delete
 * Purpose:
 *      Remove VLAN from STG linked list. No action if VLAN is not in list.
 */
static void
_bcm_fe2000_stg_map_delete(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    bcm_vlan_t     *vp;

    BCM_STG_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, vid));

    assert(BCM_VLAN_NONE != vid);

    si = &STG_CNTL(unit);

    vp = &si->vlan_first[stg];

    while (BCM_VLAN_NONE != *vp) {
        if (*vp == vid) {
            *vp = si->vlan_next[*vp];
        } else {
            vp = &si->vlan_next[*vp];
        }
    }
    
    _bcm_fe2000_stg_map_store(unit, stg, vid);

    BCM_STG_VVERB(("%s(%d, %d, %d) - Exit\n", FUNCTION_NAME(), unit, stg, vid));
}

/*
 * Function:
 *      _bcm_fe2000_stg_map_get
 * Purpose:
 *      Get STG that a VLAN is mapped to.
 * Parameters:
 *      unit - device unit number.
 *      vid  - VLAN id to search for
 *      *stg - Spanning tree group id if found
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_fe2000_stg_map_get(int unit, bcm_vlan_t vid, bcm_stg_t *stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NOT_FOUND;
    int             index;
    bcm_vlan_t      vlan;

    BCM_STG_VVERB(("%s(%d, %d, *) - Enter\n", FUNCTION_NAME(), unit, vid));

    /* this "internal" function is public so check parms */
    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    assert(BCM_VLAN_NONE != vid);
    *stg = 0;

    for (index = si->stg_min; index < si->stg_max; index++) {
        vlan = si->vlan_first[index];

        while (BCM_VLAN_NONE != vlan) {
            if (vlan  == vid) {
                /* since a vlan may exist in only one STG, safe to exit */
                *stg = index;
                result = BCM_E_NONE;
                break;
            }

            vlan = si->vlan_next[vlan];
        }

        if (BCM_E_NONE == result) {
            break;
        }
    }

    BCM_STG_VVERB(("%s(%d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, vid, *stg, bcm_errmsg(result)));

    return result;
}



/*
 * Function:
 *      _bcm_fe2000_stg_vid_compar
 * Purpose:
 *      Compare routine for sorting on VLAN ID.
 */
static int
_bcm_fe2000_stg_vid_compare(void *a, void *b)
{
    uint16 a16, b16;

    a16 = *(uint16 *)a;
    b16 = *(uint16 *)b;

    return a16 - b16;
}

#ifdef BCM_WARM_BOOT_SUPPORT
static int
_bcm_fe2000_stg_scache_recover(int unit)
{
    int              rv = BCM_E_NONE;
    bcm_stg_info_t  *si;
    soc_wb_cache_t  *wbc;

    si = &STG_CNTL(unit);

    si->wb_size = (sizeof(fe2k_stg_wb_mem_layout_t) +
                   SOC_WB_SCACHE_CONTROL_SIZE);

    /* Allocate a new chunk of the scache during a cold boot */
    if (!SOC_WARM_BOOT(unit)) {
        rv = soc_scache_alloc(unit, si->wb_hdl, si->wb_size);
        if (BCM_FAILURE(rv)) {
            BCM_STG_ERR((_SBX_D(unit, "Failed to allocate scache "
                                "for warm boot: %s\n"),
                         bcm_errmsg(rv)));
            return rv;
        }
    }

    /* Get the pointer for the Level 2 cache */
    wbc = NULL;
    rv = soc_scache_ptr_get(unit, si->wb_hdl, (uint8**)&wbc, &si->wb_size);
    if (BCM_FAILURE(rv)) {
        return rv; 
    }

    if (wbc) {
        if (!SOC_WARM_BOOT(unit)) {
            wbc->version = STG_WB_CURRENT_VERSION;
        }
        
        BCM_STG_VERB((_SBX_D(unit, "Obtained scache pointer=0x%08x, %d bytes, "
                             "version=%d.%d\n"),
                      (int)wbc->cache, si->wb_size,
                      SOC_SCACHE_VERSION_MAJOR(wbc->version),
                      SOC_SCACHE_VERSION_MINOR(wbc->version)));
        
        if (wbc->version > STG_WB_CURRENT_VERSION) {
            BCM_STG_ERR((_SBX_D(unit, "Upgrade scenario not supported.  "
                                "Current version=%d.%d  found %d.%d\n"),
                         SOC_SCACHE_VERSION_MAJOR(STG_WB_CURRENT_VERSION),
                         SOC_SCACHE_VERSION_MINOR(STG_WB_CURRENT_VERSION),
                         SOC_SCACHE_VERSION_MAJOR(wbc->version),
                         SOC_SCACHE_VERSION_MINOR(wbc->version)));
            rv = BCM_E_CONFIG;

        } else if (wbc->version < STG_WB_CURRENT_VERSION) {
            BCM_STG_ERR((_SBX_D(unit, "Downgrade scenario not supported.  "
                                "Current version=%d.%d  found %d.%d\n"),
                         SOC_SCACHE_VERSION_MAJOR(STG_WB_CURRENT_VERSION),
                         SOC_SCACHE_VERSION_MINOR(STG_WB_CURRENT_VERSION),
                         SOC_SCACHE_VERSION_MAJOR(wbc->version),
                         SOC_SCACHE_VERSION_MINOR(wbc->version)));
            rv = BCM_E_CONFIG;
        }
        
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    return rv;
}
#endif /*  BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
static int
_bcm_fe2000_stg_stp_state_recover(int unit, bcm_stg_t stg,
                                  uint32 *fast_stpstate)
{
    int                port, fast_idx, bcm_stp_state;
    bcm_vlan_t         vid;
    bcm_stg_info_t    *si;
    fe2k_stg_wb_mem_layout_t *layout = NULL;

    si = &STG_CNTL(unit);

    _bcm_fe2k_stg_wb_layout_get(unit, &layout);
    if (layout == NULL) {
        return BCM_E_INTERNAL;
    }

    /* A vid may exist in exactly one STG, and all vids in an STG are 
     * programmed to the same state, for a given port.  Given this,
     * recovery only needs one vid from the STG to recover the stp
     * state for the port.
     */
    vid = si->vlan_first[stg];
    
    PBMP_ALL_ITER(unit, port) {

        if (WB_STATE_ENABLE_GET(layout, stg, port) == TRUE) {

            BCM_PBMP_PORT_ADD(si->stg_enable[stg], port);
            if (WB_STATE_LISTEN_GET(layout, stg, port) == TRUE) {

                /* BCM_STG_STP_LISTEN */
                BCM_PBMP_PORT_ADD(si->stg_state_l[stg], port); 

            } else if (WB_STATE_READ_HW_GET(layout, stg, port) == TRUE) {

                fast_idx = (port * BCM_VLAN_COUNT) + vid;
                BCM_PBMP_PORT_ADD(si->stg_enable[stg], port);

                bcm_stp_state = 
                    _bcm_fe2000_g2p3_stg_stp_translate_to_bcm(unit,
                                                              fast_stpstate[fast_idx]);
                        
                switch (bcm_stp_state) {                        
                case BCM_STG_STP_FORWARD: 
                    BCM_PBMP_PORT_ADD(si->stg_state_l[stg], port); 
                    /* fall thru */
                case BCM_STG_STP_LEARN:
                    BCM_PBMP_PORT_ADD(si->stg_state_h[stg], port); 
                    /* fall thru */
                case BCM_STG_STP_BLOCK: break;
                default:
                    return BCM_E_INTERNAL;
                }
                        
            } else {
                return BCM_E_INTERNAL;
            }
        }
    }
    return BCM_E_NONE;
}
#endif /*  BCM_WARM_BOOT_SUPPORT */


#ifdef BCM_WARM_BOOT_SUPPORT
static int
_bcm_fe2000_stg_recover(int unit)
{
    int                        rv = BCM_E_NONE;
    int                        stg, vid, size, count;
    uint32                    *fast_stpstate;
    bcm_stg_info_t            *si;
    fe2k_stg_wb_mem_layout_t  *layout;

    si = &STG_CNTL(unit);

    SOC_SCACHE_HANDLE_SET(si->wb_hdl, unit, BCM_MODULE_STG, 0);

    /* Is Level 2 recovery even available? */
    rv = soc_stable_size_get(unit, (int*)&size);
    if (BCM_FAILURE(rv) || (size == 0)) {
        BCM_STG_WARN((_SBX_D(unit, "Level 2 not enabled.  Application "
                             "must restate STGs\n")));
        return BCM_E_NONE;
    }

    /* Get our scache from persistent storage, 
     * or allocate one for a cold boot 
     */
    rv = _bcm_fe2000_stg_scache_recover(unit);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    _bcm_fe2k_stg_wb_layout_get(unit, &layout);
    soc_scache_handle_lock(unit, si->wb_hdl);

    /* Cold boot? - Initialize the scache & done. */
    if (SOC_WARM_BOOT(unit) == FALSE) {
        sal_memset(layout, 0, sizeof(*layout));

        for (vid=0; vid < BCM_VLAN_COUNT; vid++) {
            layout->stg_map[vid] = BCM_STG_MAX;
        }

        soc_scache_handle_unlock(unit, si->wb_hdl);
        return BCM_E_NONE;
    }
    
    /* Recover the STG VID membership */
    for (vid=0; vid < BCM_VLAN_COUNT; vid++) {        
        stg = layout->stg_map[vid];
        if (stg >= BCM_STG_MAX) {
            continue;
        }

        if (STG_BITMAP_TST(si, stg) == FALSE) {
            STG_BITMAP_SET(si, stg);
            si->stg_count++;
        }

        si->vlan_next[vid]  = si->vlan_first[stg];
        si->vlan_first[stg] = vid;
    }

    soc_scache_handle_unlock(unit, si->wb_hdl);

    /* Since a VID exists in exactly one STG, and all VIDs of an STG
     * are configured the same way - scan the enabled STGs, and read only
     * the first VID for the STP state. 
     */
    count = BCM_VLAN_COUNT * MAX_PORT(unit);
    size  = sizeof(uint32_t) * count;
    fast_stpstate = sal_alloc(size, "fast stpstate");
    if (fast_stpstate == NULL) {
        return BCM_E_MEMORY;
    }

    rv = soc_sbx_g2p3_pv2e_stpstate_fast_get(unit, 0, 0, BCM_VLAN_MAX, 
                                             MAX_PORT(unit) - 1,
                                             fast_stpstate, count);
    if (BCM_FAILURE(rv)) {
        sal_free(fast_stpstate);
        return rv;
    }

    /* Recover the STP state for each port in all known STGs */
    for (stg = si->stg_min; stg <= si->stg_max; stg++) {

        if (STG_BITMAP_TST(si, stg) == FALSE) {
            continue;
        }
        
        rv = _bcm_fe2000_stg_stp_state_recover(unit, stg, fast_stpstate);

        if (BCM_FAILURE(rv)) {
            BCM_STG_ERR(("Failed to recover stp state: %s\n", 
                         bcm_errmsg(rv)));
        }
    }
    
    sal_free(fast_stpstate);

    return rv;
}
#endif   /* BCM_WARM_BOOT_SUPPORT */



/*
 * Function:
 *      _bcm_fe2000_stg_vid_stp_set
 * Purpose:
 *      Set the spanning tree state for a port in specified VLAN.
 * Parameters:
 *      unit      - device unit number.
 *      vid       - VLAN id.
 *      port      - device port number.
 *      stp_state - Port STP state.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_fe2000_stg_vid_stp_set(int unit, bcm_vlan_t vid, bcm_port_t port, int stp_state)
{
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_stg_vid_stp_set(unit, vid, port, stp_state);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}

/*
 * Function:
 *      _bcm_fe2000_stg_stp_get
 * Purpose:
 *      Retrieve the spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - Spanning tree group id.
 *      port      - device port number.
 *      stp_state - (OUT)Port STP state int the group.
 * Returns:
 *      BCM_E_XXX
 */
static int
_bcm_fe2000_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state)
{
    bcm_stg_info_t *si;
    int             state = 0;
    int             result = BCM_E_NONE; /* local result code */

    BCM_STG_VVERB(("%s(%d, %d, %d, *) - Enter\n", FUNCTION_NAME(), unit, stg, port));

    /* Input parameters check. */
    STG_CHECK_PORT(unit, port);
    si = &STG_CNTL(unit);

    if (BCM_PBMP_MEMBER(si->stg_enable[stg], port)) {
        if (BCM_PBMP_MEMBER(si->stg_state_h[stg], port)) {
            state |= 0x2;
        }
        if (BCM_PBMP_MEMBER(si->stg_state_l[stg], port)) {
            state |= 0x1;
        }
        switch (state) {
        case 0:  *stp_state = BCM_STG_STP_BLOCK;
            break;
        case 1:  *stp_state = BCM_STG_STP_LISTEN;
            break;
        case 2:  *stp_state = BCM_STG_STP_LEARN;
            break;
        case 3:  *stp_state = BCM_STG_STP_FORWARD;
            break;
        default: *stp_state = BCM_STG_STP_DISABLE;
        }
    } else {
        *stp_state = BCM_STG_STP_DISABLE;
    }

    BCM_STG_VVERB(("%s(%d, %d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, port, *stp_state, bcm_errmsg(result)));

    return result;
}

#ifdef BCM_FE2000_P3_SUPPORT
int _bcm_fe2000_g2p3_stg_fast_stp_set(int unit, bcm_stg_t stg,
                                      bcm_port_t port, int stp_state)
{
    int rv;
    bcm_vlan_t vid;
    bcm_stg_info_t *si; 
    int *fastSets;

    si = &STG_CNTL(unit);
    
    fastSets = sal_alloc(BCM_VLAN_COUNT * sizeof(int), "STG-fast sets");
    if (fastSets == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(fastSets, 0, BCM_VLAN_COUNT * sizeof(int));

    vid = si->vlan_first[stg];
    for (; BCM_VLAN_NONE != vid; vid = si->vlan_next[vid]) {
        fastSets[vid] = 1;
    }

    rv = _bcm_fe2000_g2p3_stp_fast_set(unit, port, stp_state, fastSets);

    sal_free(fastSets);

    return rv;
}
#endif

/*
 * Function:
 *      _bcm_fe2000_stg_stp_set
 * Purpose:
 *      Set the spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - Spanning tree group id.
 *      port      - device port number.
 *      stp_state - (OUT)Port STP state int the group.
 * Returns:
 *      BCM_E_XXX
 */


#define _STG_VLAN_COUNT BCM_VLAN_COUNT

static int
_bcm_fe2000_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state)
{
    bcm_stg_info_t *si;
    bcm_vlan_t      vid;
    int             result = BCM_E_NOT_FOUND; /* local result code */

    SOC_SBX_WARM_BOOT_DECLARE(fe2k_stg_wb_mem_layout_t *layout = NULL);

    BCM_STG_VVERB(("%s(%d, %d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, port, stp_state));

    /* Input parameters check. */
    STG_CHECK_PORT(unit, port);

    si = &STG_CNTL(unit);

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        result = _bcm_fe2000_g2p3_stg_fast_stp_set(unit, stg, port, stp_state);
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        /* get first vlan */
        vid = si->vlan_first[stg];

        while (BCM_VLAN_NONE != vid) {
            result = _bcm_fe2000_stg_vid_stp_set(unit, vid, port, stp_state);
            if (BCM_E_NONE != result) {
                break;
            }
            vid = si->vlan_next[vid];
        }
    }

    if (BCM_FAILURE(result)) {
        return result;
    }


#ifdef BCM_WARM_BOOT_SUPPORT
    BCM_IF_ERROR_RETURN(
        _bcm_fe2k_stg_wb_layout_get(unit, &layout));
    if (layout) {
        soc_scache_handle_lock(unit, si->wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    if (stp_state == BCM_STG_STP_DISABLE) {
        BCM_PBMP_PORT_REMOVE(si->stg_enable[stg], port);

#ifdef BCM_WARM_BOOT_SUPPORT
        if (layout) {
            WB_STATE_ENABLE_CLEAR(layout, stg, port);
        }
#endif /* BCM_WARM_BOOT_SUPPORT */

    } else {
        BCM_PBMP_PORT_ADD(si->stg_enable[stg], port);

#ifdef BCM_WARM_BOOT_SUPPORT
        if (layout) {
            WB_STATE_ENABLE_SET(layout, stg, port);
            if (stp_state == BCM_STG_STP_LISTEN) {
                WB_STATE_LISTEN_SET(layout, stg, port);
            } else {
                WB_STATE_READ_HW_SET(layout, stg, port);
            }
        }
#endif /* BCM_WARM_BOOT_SUPPORT */

        if ((stp_state == BCM_STG_STP_LEARN) ||
            (stp_state == BCM_STG_STP_FORWARD)) {
            BCM_PBMP_PORT_ADD(si->stg_state_h[stg], port);
        } else {
            BCM_PBMP_PORT_REMOVE(si->stg_state_h[stg], port);
        }
        if ((stp_state == BCM_STG_STP_LISTEN) ||
            (stp_state == BCM_STG_STP_FORWARD)) {
            BCM_PBMP_PORT_ADD(si->stg_state_l[stg], port);
        } else {
            BCM_PBMP_PORT_REMOVE(si->stg_state_l[stg], port);
        }
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    if (layout) {
        soc_scache_handle_lock(unit, si->wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */


    BCM_STG_VVERB(("%s(%d, %d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, port, stp_state, bcm_errmsg(result)));


    return result;
}


/*
 * Function:
 *      _bcm_fe2000_stg_vlan_add
 * Purpose:
 *      Main part of bcm_stg_vlan_add; assumes locks already acquired.
 * Parameters:
 *      unit    - device unit number.
 *      stg     - spanning tree group id.
 *      vid     - vlan to add
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The vlan is removed from it's current STG if necessary.
 */
static int
_bcm_fe2000_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg_cur;
    int             result = BCM_E_NOT_FOUND; /* local result code */

    BCM_STG_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, vid));

    si = &STG_CNTL(unit);

    if ((_stg_vlan_min > vid) || (_stg_vlan_max < vid)) {
        result = BCM_E_PARAM;

    } else if (STG_BITMAP_TST(si, stg)) { /* STG must already exist */

        /* Get the STG the VLAN is currently associated to */
        result = _bcm_fe2000_stg_map_get(unit, vid, &stg_cur);
        if ((BCM_E_NONE == result) || (BCM_E_NOT_FOUND == result)) {

            /* iff found, delete it */
            if (BCM_E_NONE == result) {
                _bcm_fe2000_stg_map_delete(unit, stg_cur, vid);
            }
        }

        /* Set the new STG */
        _bcm_fe2000_stg_map_add(unit, stg, vid);

        result = BCM_E_NONE;
    }

    BCM_STG_VVERB(("%s(%d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, vid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      _bcm_fe2000_stg_vlan_remove
 * Purpose:
 *      Main part of bcm_stg_vlan_remove; assumes lock already acquired.
 * Parameters:
 *      unit    - device unit number.
 *      stg     - spanning tree group id.
 *      vid     - vlan id to remove
 *      destroy - boolean flag indicating the VLAN is being destroyed and is
 *                not to be added to the default STG. Also used internally
 *                to supress default STG assignment during transition.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
_bcm_fe2000_stg_vlan_remove(int unit, bcm_stg_t stg, bcm_vlan_t vid, int destroy)
{
    bcm_stg_info_t *si;
    int             stg_cur;
    int             result = BCM_E_NOT_FOUND; /* local result code */

    BCM_STG_VVERB(("%s(%d, %d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, vid, destroy));

    si = &STG_CNTL(unit);

    /* STG must already exist */
    if (STG_BITMAP_TST(si, stg)) {

        /* Get the STG the VLAN is currently associated to */
        result = _bcm_fe2000_stg_map_get(unit, vid, &stg_cur);
        if ((BCM_E_NONE == result) && ((stg == stg_cur))) {
            _bcm_fe2000_stg_map_delete(unit, stg, vid);

            /* If the VLAN is not being destroyed, set the VLAN to the default STG */
            if (FALSE == destroy) {
                _bcm_fe2000_stg_map_add(unit, si->stg_defl, vid);
            }
        }
    }

    BCM_STG_VVERB(("%s(%d, %d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, vid, destroy, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      _bcm_fe2000_stg_vlan_port_add
 * Purpose:
 *      Callout for vlan code to get a port that has been added to a vlan into
 *      the proper STP state
 * Parameters:
 *      unit      - device unit number.
 *      vid       - VLAN id.
 *      port      - device port number.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_fe2000_stg_vlan_port_add(int unit, bcm_vlan_t vid, bcm_port_t port)
{
    int         result = BCM_E_FAIL; /* local result code */
    bcm_stg_t   stg;
    int         state;

    BCM_STG_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, vid, port));

    result = _bcm_fe2000_stg_map_get(unit, vid, &stg);

    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_stg_stp_get(unit, stg, port, &state);
    }

    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_stg_vid_stp_set(unit, vid, port, state);
    }

    BCM_STG_VVERB(("%s(%d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, vid, port, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      _bcm_fe2000_stg_vlan_port_remove
 * Purpose:
 *      Callout for vlan code to get a port that has been removed from a vlan
 *      into the proper STP state
 * Parameters:
 *      unit      - device unit number.
 *      vid       - VLAN id.
 *      port      - device port number.
 * Returns:
 *      BCM_E_NONE - This function is best effort
 */
int
_bcm_fe2000_stg_vlan_port_remove(int unit, bcm_vlan_t vid, bcm_port_t port)
{
    BCM_STG_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, vid, port));

    (void)_bcm_fe2000_stg_vid_stp_set(unit, vid, port, BCM_STG_STP_BLOCK);

    BCM_STG_VVERB(("%s(%d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, vid, port, bcm_errmsg(BCM_E_NONE)));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_stg_stp_init
 * Purpose:
 *      Initialize spanning tree group on a single device.
 * Parameters:
 *      unit - SOC unit number.
 *      stg  - Spanning tree group id.
 * Returns:
 *      BCM_E_XXX
 */
static int
_bcm_fe2000_stg_stp_init(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    bcm_port_t      port;
    int             result = BCM_E_NONE;
    SOC_SBX_WARM_BOOT_DECLARE(fe2k_stg_wb_mem_layout_t *layout = NULL);

    BCM_STG_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, stg));

    si = &STG_CNTL(unit);


#ifdef BCM_WARM_BOOT_SUPPORT
    _bcm_fe2k_stg_wb_layout_get(unit, &layout);
    if (layout) {
        soc_scache_handle_lock(unit, si->wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    PBMP_ALL_ITER(unit, port) {
        /* place port in blocked */
        BCM_PBMP_PORT_ADD(si->stg_enable[stg], port);

#ifdef BCM_WARM_BOOT_SUPPORT
        if (layout) {
            WB_STATE_ENABLE_SET(layout, stg, port);
        }
#endif /*  BCM_WARM_BOOT_SUPPORT */

        BCM_PBMP_PORT_REMOVE(si->stg_state_h[stg], port);
        BCM_PBMP_PORT_REMOVE(si->stg_state_l[stg], port);
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    if (layout) {
        soc_scache_handle_unlock(unit, si->wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */
    
    BCM_STG_VVERB(("%s(%d, %d) - Exit(%s)\n",
                  FUNCTION_NAME(), unit, stg, bcm_errmsg(BCM_E_NONE)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_vlan_list
 * Purpose:
 *      Return a list of VLANs in a specified Spanning Tree Group (STG).
 * Parameters:
 *      unit  - device unit number
 *      stg   - STG ID to list
 *      list  - Place where pointer to return array will be stored.
 *              Will be NULL if there are zero VLANs returned.
 *      count - Place where number of entries in array will be stored.
 *              Will be 0 if there are zero VLANs associated to the STG.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If the Spanning Tree Group (STG) requested is not defined in the
 *      system, call returns NULL list and count of zero (0). The caller is
 *      responsible for freeing the memory that is returned, using
 *      bcm_stg_vlan_list_destroy().
 */
int
bcm_fe2000_stg_vlan_list(int unit, bcm_stg_t stg, bcm_vlan_t **list,
                         int *count)
{
    bcm_stg_info_t *si;
    bcm_vlan_t      vlan;
    int             index;
    int             result = BCM_E_NOT_FOUND;

    BCM_STG_VERB(("%s(%d, %d, *, *) - Enter\n", FUNCTION_NAME(), unit, stg));
    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    *list = NULL;
    *count = 0;

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        /* Traverse list once just to get an allocation count */
        vlan = si->vlan_first[stg];

        while (BCM_VLAN_NONE != vlan) {
            (*count)++;
            vlan = si->vlan_next[vlan];
        }

        if (0 == *count) {
            result = BCM_E_NONE;
        }
        else {
            *list = sal_alloc(*count * sizeof (bcm_vlan_t), "bcm_fe2000_stg_vlan_list");

            if (NULL == *list) {
                result = BCM_E_MEMORY;
            }
            else {
                /* Traverse list a second time to record the VLANs */
                vlan = si->vlan_first[stg];
                index = 0;

                while (BCM_VLAN_NONE != vlan) {
                    (*list)[index++] = vlan;
                    vlan = si->vlan_next[vlan];
                }

                /* Sort the vlan list */
                _shr_sort(*list, *count, sizeof (bcm_vlan_t), _bcm_fe2000_stg_vid_compare);

                result = BCM_E_NONE;
            }
        }

    }
    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d, *, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, *count, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_vlan_list_destroy
 * Purpose:
 *      Destroy a list returned by bcm_fe2000_stg_vlan_list.
 * Parameters:
 *      unit  - device unit number
 *      list  - Pointer to VLAN array to be destroyed.
 *      count - Number of entries in array.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_vlan_list_destroy(int unit, bcm_vlan_t *list, int count)
{
    int     result = BCM_E_NONE;
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(count);

    BCM_STG_VERB(("%s(%d, *, %d) - Enter\n", FUNCTION_NAME(), unit, count));

    if (NULL != list) {
        sal_free(list);
    }

    BCM_STG_VERB(("%s(%d, *, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, count, bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_default_get
 * Purpose:
 *      Returns the default STG for the device.
 * Parameters:
 *      unit    - device unit number.
 *      stg_ptr - STG ID for default.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_default_get(int unit, bcm_stg_t *stg_ptr)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NONE;

    BCM_STG_VERB(("%s(%d, *) - Enter\n", FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    *stg_ptr = si->stg_defl;

    STG_CHECK_STG(si, *stg_ptr);

    BCM_STG_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, *stg_ptr, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_default_set
 * Purpose:
 *      Changes the default STG for the device.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      stg  - STG ID to become default.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The specified STG must already exist.
 */
int
bcm_fe2000_stg_default_set(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NOT_FOUND;

    BCM_STG_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        si->stg_defl = stg;

#ifdef BCM_WARM_BOOT_SUPPORT
        result = 
            _bcm_fe2000_recovery_var_set(unit, fe2k_rcvr_default_stg, stg);
        if (BCM_FAILURE(result)) {
            BCM_STG_ERR(("Failed to set recovery data\n"));
        }
#endif /* BCM_WARM_BOOT_SUPPORT */
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_vlan_add
 * Purpose:
 *      Add a VLAN to a spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to use
 *      vid  - VLAN id to be added to STG
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Spanning tree group ID must have already been created. The
 *      VLAN is removed from the STG it is currently in.
 */
int
bcm_fe2000_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_BADID;

    BCM_STG_VERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, vid));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (BCM_VLAN_VALID(vid)) {
        result = _bcm_fe2000_stg_vlan_add(unit, stg, vid);
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, vid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_vlan_remove
 * Purpose:
 *      Remove a VLAN from a spanning tree group. The VLAN is placed in the
 *      default spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to use
 *      vid  - VLAN id to be removed from STG
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_vlan_remove(int unit, bcm_stg_t stg, bcm_vlan_t vid)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_BADID;

    BCM_STG_VERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, vid));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (BCM_VLAN_VALID(vid)) {
        result = _bcm_fe2000_stg_vlan_remove(unit, stg, vid, FALSE);
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, vid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_vlan_remove_all
 * Purpose:
 *      Remove all VLANs from a spanning tree group. The VLANs are placed in
 *      the default spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to clear
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_vlan_remove_all(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NONE;
    bcm_vlan_t      vid;

    BCM_STG_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (stg != si->stg_defl) {

        if (!STG_BITMAP_TST(si, stg)) {   /* STG must already exist */
            result = BCM_E_NOT_FOUND;
        } else {
            vid = si->vlan_first[stg];
            while (BCM_VLAN_NONE != vid) {
                result = _bcm_fe2000_stg_vlan_remove(unit, stg, vid, FALSE);
                if (BCM_E_NONE != result) {
                    break;
                }

                result = _bcm_fe2000_stg_vlan_add(unit, si->stg_defl, vid);
                if (BCM_E_NONE != result) {
                    break;
                }

                /*
                 * Iterate through list. The vid just removed was popped from
                 * the list. Use the new first.
                 */
                vid = si->vlan_first[stg];
            }
        }
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_clear
 * Description:
 *      Destroy all STGs
 * Parameters:
 *      unit - device unit number.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_clear(int unit)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg;
    int             result = BCM_E_NONE;

    BCM_STG_VERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit);

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
        if (STG_BITMAP_TST(si, stg)) {
            STG_DB_UNLOCK(unit);
            /* ignore error code as unit will be 'init'ed later. */
            result = bcm_fe2000_stg_destroy(unit, stg);
            if (BCM_FAILURE(result)) {
                return result;
            }
            STG_DB_LOCK(unit);
        }
    }

    STG_DB_UNLOCK(unit);

    result = bcm_fe2000_stg_init(unit);

    BCM_STG_VERB(("%s(%d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_create_id
 * Description:
 *      Create a STG, using a specified ID.
 * Parameters:
 *      unit - Device unit number
 *      stg -  STG to create
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      In the new STG, all ports are in the DISABLED state.
 */
int
bcm_fe2000_stg_create_id(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_EXISTS;

    BCM_STG_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (!STG_BITMAP_TST(si, stg)) {
        /* No device action needed */
        result = _bcm_fe2000_stg_stp_init(unit, stg);
        if (BCM_E_NONE == result) {
            STG_BITMAP_SET(si, stg);
            si->stg_count++;
        }
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_create
 * Description:
 *      Create a STG, picking an unused ID and returning it.
 * Parameters:
 *      unit - device unit number
 *      stg_ptr - (OUT) the STG ID.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_create(int unit, bcm_stg_t *stg_ptr)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg = BCM_STG_INVALID;
    int             result = BCM_E_FULL;

    BCM_STG_VERB(("%s(%d, *) - Enter\n", FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit);

    for (stg = si->stg_min; stg <= si->stg_max; stg++) {
        if (!STG_BITMAP_TST(si, stg)) {
            break;          /* free id found */
        }
    }

    if (si->stg_max >= stg) {
        result = bcm_fe2000_stg_create_id(unit, stg);
    }

    STG_DB_UNLOCK(unit);

    *stg_ptr = stg;

    BCM_STG_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, *stg_ptr, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_destroy
 * Description:
 *      Destroy an STG.
 * Parameters:
 *      unit - device unit number.
 *      stg  - The STG ID to be destroyed.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The default STG may not be destroyed.
 */
int
bcm_fe2000_stg_destroy(int unit, bcm_stg_t stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_PARAM;

    BCM_STG_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, stg));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);

    STG_DB_LOCK(unit);

    if (si->stg_defl != stg) {
        /* The next call checks if STG exists as well as removing all VLANs */
        result = bcm_fe2000_stg_vlan_remove_all(unit, stg);

        if (BCM_E_NONE == result) {
            STG_BITMAP_CLR(si, stg);
            si->stg_count--;
        }
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_list
 * Purpose:
 *      Return a list of defined Spanning Tree Groups
 * Parameters:
 *      unit  - device unit number
 *      list  - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero STGs returned.
 *      count - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero STGs returned.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The caller is responsible for freeing the memory that is returned,
 *      using bcm_stg_list_destroy().
 */
int
bcm_fe2000_stg_list(int unit, bcm_stg_t **list, int *count)
{
    bcm_stg_info_t *si;
    bcm_stg_t       stg;
    int             index = 0;
    int             result = BCM_E_NONE;

    BCM_STG_VERB(("%s(%d, *, *) - Enter\n", FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit);

    if (0 == si->stg_count) {
        *count = 0;
        *list = NULL;
    } else {
        *count = si->stg_count;
        *list = sal_alloc(si->stg_count * sizeof (bcm_stg_t), "bcm_stg_list");

        if (NULL == *list) {
            result = BCM_E_MEMORY;
        } else {
            for (stg = si->stg_min; stg <= si->stg_max; stg++) {
                if (STG_BITMAP_TST(si, stg)) {
                    assert(index < *count);
                    (*list)[index++] = stg;
                }
            }
        }
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, *, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, *count, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_stg_list_destroy
 * Purpose:
 *      Destroy a list returned by bcm_stg_list.
 * Parameters:
 *      unit  - device unit number
 *      list  - Place where pointer to return array will be stored,
 *              which will be NULL if there are zero STGs returned.
 *      count - Place where number of entries in array will be stored,
 *              which will be 0 if there are zero STGs returned.
 * Returns:
 *      BCM_E_NONE
 */
int
bcm_fe2000_stg_list_destroy(int unit, bcm_stg_t *list, int count)
{
    int     result = BCM_E_NONE;

    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(count);

    BCM_STG_VERB(("%s(%d, *, %d) - Enter\n", FUNCTION_NAME(), unit, count));

    if (NULL != list) {
        sal_free(list);
    }

    BCM_STG_VERB(("%s(%d, *, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, count, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_stp_set
 * Purpose:
 *      Set the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - Spanning Tree State of port.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NOT_FOUND;

    BCM_STG_VERB(("%s(%d, %d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, stg, port, stp_state));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);
    STG_CHECK_PORT(unit, port);
    if ((stp_state < BCM_STG_STP_DISABLE) || (stp_state > BCM_STG_STP_FORWARD))
        return BCM_E_PARAM;

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        result = _bcm_fe2000_stg_stp_set(unit, stg, port, stp_state);
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, port, stp_state, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_fe2000_stg_stp_get
 * Purpose:
 *      Get the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - (Out) Pointer to where Spanning Tree State is stored.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
int
bcm_fe2000_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NOT_FOUND;

    BCM_STG_VERB(("%s(%d, %d, %d, *) - Enter\n", FUNCTION_NAME(), unit, stg, port));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);
    STG_CHECK_STG(si, stg);
    STG_CHECK_PORT(unit, port);

    STG_DB_LOCK(unit);

    if (STG_BITMAP_TST(si, stg)) {
        result = _bcm_fe2000_stg_stp_get(unit, stg, port, stp_state);
    }

    STG_DB_UNLOCK(unit);

    BCM_STG_VERB(("%s(%d, %d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, stg, port, *stp_state, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *     bcm_stg_count_get
 * Purpose:
 *     Get the maximum number of STG groups the device supports
 * Parameters:
 *     unit    - device unit number.
 *     max_stg - max number of STG groups supported by this unit
 * Returns:
 *     BCM_E_xxx
 */
int
bcm_fe2000_stg_count_get(int unit, int *max_stg)
{
    bcm_stg_info_t *si;
    int             result = BCM_E_NONE;

    BCM_STG_VERB(("%s(%d, *) - Enter\n", FUNCTION_NAME(), unit));

    STG_CHECK_INIT(unit);
    si = &STG_CNTL(unit);

    *max_stg = si->stg_max - si->stg_min + 1;

    BCM_STG_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, *max_stg, bcm_errmsg(result)));

    return result;
}

int
bcm_fe2000_stg_detach(int unit)
{
    bcm_stg_info_t     *si;

    BCM_STG_VERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || (unit >= BCM_MAX_NUM_UNITS)) {
        /* stg_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX
         */
        return BCM_E_UNIT;
    }

    si = &STG_CNTL(unit);

    STG_DB_LOCK(unit); 
    if (si->stg_bitmap) {
        sal_free(si->stg_bitmap);
        si->stg_bitmap = NULL;
    }

    if (si->stg_enable) {
        sal_free(si->stg_enable);
        si->stg_enable = NULL;
    } 

    if (si->stg_state_h) {
        sal_free(si->stg_state_h);
        si->stg_state_h = NULL;
    }

    if (si->stg_state_l) {
        sal_free(si->stg_state_l);
        si->stg_state_l = NULL;
    }

    if (si->vlan_first) {
        sal_free(si->vlan_first); 
        si->vlan_first = NULL;
    }

    if (si->vlan_next) {
        sal_free(si->vlan_next);
        si->vlan_next = NULL;
    }
    STG_DB_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_fe2000_stg_init
 * Description:
 *      Initialize the STG module according to Initial Configuration.
 * Parameters:
 *      unit - Device unit number (driver internal).
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_stg_init(int unit)
{
    bcm_stg_info_t     *si;
    int                 sz_bmp, sz_e, sz_sh, sz_sl, sz_vfirst, sz_vnext;
    int                 index;
    bcm_port_t          port;
    int                 result = BCM_E_NONE;

    BCM_STG_VERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || (unit >= BCM_MAX_NUM_UNITS)) {
        /* stg_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX 
         */
        return BCM_E_UNIT;
    }

    si = &STG_CNTL(unit);

    if (NULL == si->lock) {
        if (NULL == (si->lock = sal_mutex_create("fe2000_stg_lock"))) {
            return BCM_E_MEMORY;
        }
    }

    /* Set the device properties */
    si->stg_count = 0;                  /* no STGs currently defined */
    si->stg_min   = _stg_min;
    si->stg_max   = _stg_max;
    si->stg_defl  = _stg_default;       /* The default STG is always 1 */

    assert(si->stg_defl >= si->stg_min && si->stg_defl <= si->stg_max &&
           si->stg_min <= si->stg_max);

    /* alloc memory and/or clear */
    sz_bmp = SHR_BITALLOCSIZE(si->stg_max+1);
    if (NULL == si->stg_bitmap) {
        si->stg_bitmap = sal_alloc(sz_bmp, "STG-bitmap");
    }

    /* array of port bitmaps indicating whether the port+stg has STP enabled,
     * 0 = BCM_STG_STP_DISABLE, 1= Enabled.
     */
    sz_sh = sz_sl = sz_e = sizeof(bcm_pbmp_t) * (si->stg_max + 1);
    if (NULL == si->stg_enable) {
        si->stg_enable = sal_alloc(sz_e, "STG-enable");
    }

    /* array of port bitmaps indicating STP state for the port+stg combo.
     * Only valid if stg_enable = TRUE. Use high (_h) and low (_l) for
     * each port to represent one of four states in bcm_stg_stp_t, i.e
     * BLOCK(_h,_l = 0,0), LISTEN(0,1), LEARN(1,0), FORWARD(1,1).
     */
    if (NULL == si->stg_state_h) {
        si->stg_state_h = sal_alloc(sz_sh, "STG-state_h");
    }

    if (NULL == si->stg_state_l) {
        si->stg_state_l = sal_alloc(sz_sl, "STG-state_l");
    }

    sz_vfirst = (si->stg_max + 1) * sizeof (bcm_vlan_t);
    if (NULL == si->vlan_first) {
        si->vlan_first = sal_alloc(sz_vfirst, "STG-vfirst");
    }

    sz_vnext = BCM_VLAN_COUNT * sizeof (bcm_vlan_t);
    if (NULL == si->vlan_next) {
        si->vlan_next = sal_alloc(sz_vnext, "STG-vnext");
    }

    if (si->stg_bitmap == NULL || si->stg_enable == NULL || 
        si->stg_state_h == NULL || si->stg_state_l == NULL ||
        si->vlan_first == NULL || si->vlan_next == NULL) {

        result = bcm_fe2000_stg_detach(unit);
        if (BCM_FAILURE(result)) {
            BCM_STG_ERR(("Failed to clean up data\n"));
        }
        return BCM_E_MEMORY;
    }

    sal_memset(si->stg_bitmap, 0, sz_bmp);
    sal_memset(si->stg_enable, 0, sz_e);
    sal_memset(si->stg_state_h, 0, sz_sh);
    sal_memset(si->stg_state_l, 0, sz_sl);
    sal_memset(si->vlan_first, 0, sz_vfirst);
    sal_memset(si->vlan_next, 0, sz_vnext);

    /*
     * Create default STG and add all VLANs to it.  Use private calls.
     * This creates a slight maintenance issue but allows delayed setting
     * of the init flag. This will prevent any public API functions
     * from executing.
     */
    result = _bcm_fe2000_stg_stp_init(unit, si->stg_defl);
    BCM_IF_ERROR_RETURN(result);

    STG_BITMAP_SET(si, si->stg_defl);
    si->stg_count++;

    BCM_PBMP_ITER(PBMP_CMIC(unit), port) {
        for (index = _stg_vlan_min; index <= _stg_vlan_max; index++) {
            (void)_bcm_fe2000_stg_vid_stp_set(unit, (bcm_vlan_t)index, port, BCM_STG_STP_DISABLE);
        }
    }

    si->init = TRUE;

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        uint32 tmp;
        result = _bcm_fe2000_recovery_var_get(unit, 
                                              fe2k_rcvr_default_stg, 
                                              &tmp);
        if (BCM_FAILURE(result)) {
            BCM_STG_ERR(("Failed to access recovery data\n"));
            return result;
        }

        if (si->stg_defl != tmp) {
            si->stg_defl = tmp;
        }
        
    } else {
        result = _bcm_fe2000_recovery_var_set(unit, 
                                              fe2k_rcvr_default_stg, 
                                              si->stg_defl);
    }

    if (BCM_FAILURE(result)) {
        BCM_STG_ERR(("Failed to access recovery data\n"));
        return result;
    }

    result = _bcm_fe2000_stg_recover(unit);
    if (BCM_FAILURE(result)) {
        BCM_STG_ERR(("Failed recovery\n"));
        return result;
    }

#endif /* BCM_WARM_BOOT_SUPPORT */


#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_SBX_CONTROL(unit)->ucodetype ==  SOC_SBX_UCODE_TYPE_G2P3) {
      result = _bcm_fe2000_g2p3_stg_init(unit);
      BCM_IF_ERROR_RETURN(result);
    }
#endif

    BCM_STG_VERB(("bcm_stg_init: unit=%d rv=%d(%s)\n",
               unit, BCM_E_NONE, bcm_errmsg(BCM_E_NONE)));

    return BCM_E_NONE;
}
