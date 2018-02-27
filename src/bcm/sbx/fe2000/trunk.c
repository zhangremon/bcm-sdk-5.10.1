/*
 * $Id: trunk.c 1.61.42.1 Broadcom SDK $
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
 * File:    trunk.c
 * Purpose: BCM level APIs for Link Aggregation (a.k.a Trunking)
 */

#define SBX_HASH_DEFINED 1
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/trunk.h>
#include <bcm_int/sbx/fe2000/trunk.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/pkt.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/fe2000/vlan.h>

#define _BCM_TRUNK_PSC_UNDEFINED  -1

#define _BCM_TRUNK_PSC_EXTENDED_FLAGS      \
     (BCM_TRUNK_PSC_MACDA       |       \
      BCM_TRUNK_PSC_MACSA       |       \
      BCM_TRUNK_PSC_IPSA        |       \
      BCM_TRUNK_PSC_IPDA        |       \
      BCM_TRUNK_PSC_L4SS        |       \
      BCM_TRUNK_PSC_L4DS        |       \
      BCM_TRUNK_PSC_VID)

typedef struct trunk_private_s {
    bcm_trunk_t     trunk_id;       /* trunk group ID */
    int             in_use;         /* whether particular trunk is in use */
    int             num_ports;        /* Number of ports in the trunk group */
    uint32_t        member_flags[BCM_TRUNK_MAX_PORTCNT];
    bcm_port_t      tp[BCM_TRUNK_MAX_PORTCNT];    /* Ports in trunk */
    bcm_module_t    tm[BCM_TRUNK_MAX_PORTCNT];    /* Modules per port */
} trunk_private_t;

typedef enum trunk_init_state_e {
    ts_none,
    ts_recovering, /* phase 1 recovered, second phase recovery required to
                    * complete init sequence */
    ts_init,       /* completely initialized */
} trunk_init_state_t;

typedef struct trunk_cntl_s {
    trunk_init_state_t  init_state; /* Initialized, recovering, or none */
    sal_mutex_t         lock;       /* Database lock */
    int                 ngroups;    /* max number of trunk groups */
    int                 nports;     /* max port count per trunk group */
    int                 psc;        /* port spec criterion (a.k.a. hash algorithm) */
    int                 invalid_oi;
    bcm_trunk_notify_cb callback_func[_BCM_TRUNK_MAX_CALLBACK]; /* callback function pointers. */
    void               *callback_user_data[_BCM_TRUNK_MAX_CALLBACK];  /* user_data pointers. */
    trunk_private_t    *t_info;
} trunk_cntl_t;

/*
 * One trunk control entry for each SOC device containing trunk book keeping
 * info for that device.
 */
static trunk_cntl_t bcm_trunk_control[BCM_MAX_NUM_UNITS];

#define TRUNK_MIN_MEMBERS   0   /* Minimum number of ports in a trunk */
#define TRUNK_NUM_VIDS      4095

#define TRUNK_CNTL(unit)        bcm_trunk_control[unit]
#define TRUNK_INFO(unit, tid)   bcm_trunk_control[unit].t_info[tid]

#define TRUNK_DB_LOCK(unit)                                                 \
        do {                                                                \
            if (NULL != TRUNK_CNTL(unit).lock)                              \
                sal_mutex_take(TRUNK_CNTL(unit).lock, sal_mutex_FOREVER);   \
        } while (0);

#define TRUNK_DB_UNLOCK(unit)                           \
        do {                                            \
            if (NULL != TRUNK_CNTL(unit).lock)          \
                sal_mutex_give(TRUNK_CNTL(unit).lock);  \
        } while (0);

/*
 * Cause a routine to return BCM_E_INIT if trunking subsystem is not
 * initialized to an acceptable initialization level (il_).
 */
#define TRUNK_CHECK_INIT(u_, il_)                                 \
    do {                                                          \
        if (!BCM_UNIT_VALID(u_)) return BCM_E_UNIT;               \
        if (u_ >= BCM_MAX_NUM_UNITS) return BCM_E_UNIT;           \
        if (TRUNK_CNTL(u_).init_state < (il_)) return BCM_E_INIT; \
    } while (0);

/*
 * Make sure TID is within valid range.
 */
#define TRUNK_CHECK_TID(unit, tid) \
    if (((tid) < 0) || ((tid) >= TRUNK_CNTL(unit).ngroups)) \
        return BCM_E_BADID;
/*
 * TID is in range, check to make sure it is actually in use.
 */
#define TRUNK_TID_VALID(unit, tid)                          \
    (TRUNK_INFO((unit), (tid)).trunk_id != BCM_TRUNK_INVALID)

#define TRUNK_PORTCNT_VALID(unit, port_cnt)                  \
    ((port_cnt >= TRUNK_MIN_MEMBERS) && (port_cnt <= TRUNK_CNTL(unit).nports))

#define G2_FE_HANDLER_GET(unit, fe)  \
    if ((fe = (sbG2Fe_t *)SOC_SBX_CONTROL(unit)->drv) == NULL) {  \
        return BCM_E_INIT;  \
    }
#define G2P3_FE_HANDLER_GET(unit, fe)  \
    if ((fe = (soc_sbx_g2p3_state_t*)SOC_SBX_CONTROL(unit)->drv) == NULL) {  \
        return BCM_E_INIT;  \
    }



/* Debug Macros */
#if 0 
#undef BCM_DEBUG
#define BCM_DEBUG(flags, stuff)   soc_cm_print stuff
#endif 
#define BCM_TRUNK_ERR(stuff)          BCM_DEBUG(BCM_DBG_ERR      | BCM_DBG_TRUNK, stuff)
#define BCM_TRUNK_WARN(stuff)         BCM_DEBUG(BCM_DBG_WARN     | BCM_DBG_TRUNK, stuff)
#define BCM_TRUNK_VERB(stuff)         BCM_DEBUG(BCM_DBG_TRUNK | BCM_DBG_VERBOSE, stuff)
#define BCM_TRUNK_VVERB(stuff)        BCM_DEBUG(BCM_DBG_TRUNK | BCM_DBG_VVERBOSE, stuff)
#define BCM_TRUNK_MSG(stuff)          BCM_DEBUG(BCM_DBG_NORMAL, stuff)

static int        _ngroups          = SBX_MAX_TRUNKS;
static int        _nports           = BCM_TRUNK_MAX_PORTCNT;

#define QE_SPI_SUBPORT_GET(funit, fport) ((fport) + SOC_PORT_MIN((funit), spi_subport))

/*
 * Fixed offsets
 */
/* total system ports including CPU */
#define TRUNK_SBX_HASH_SIZE             3
#define TRUNK_SBX_FIXED_PORTCNT         (1<<TRUNK_SBX_HASH_SIZE)

#define TRUNK_INDEX_SET(tid, offset)                \
    (TRUNK_SBX_FIXED_PORTCNT > (offset)) ?          \
     ((((tid))<<(TRUNK_SBX_HASH_SIZE)) | (offset)) :  \
     -1

/* forward declaration */
int bcm_fe2000_trunk_psc_set(int unit, bcm_trunk_t tid, int psc);



typedef struct recursive_mutex_s {
    sal_sem_t       sem;
    sal_thread_t    owner;
    int             recurse_count;
    char           *desc;
} recursive_mutex_t;


/*
 * Function:
 *    _bcm_fe2000_trunk_debug
 * Purpose:
 *      Displays trunk information maintained by software.
 * Parameters:
 *      unit - Device unit number
 * Returns:
 *      None
 */
#ifdef BCM_QE2000_SUPPORT
extern void _bcm_qe2000_trunk_debug(int unit);
#endif

void
_bcm_fe2000_trunk_debug(int unit)
{
#ifdef BROADCOM_DEBUG
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    recursive_mutex_t  *lock;
    int                 index, jindex;
    int                 invIdx = 0;
    bcm_trunk_t         invalidList[SBX_MAX_TRUNKS];
    int                 funit, fnode, fport;
    int                 mymod, fmod;

    if (!BCM_UNIT_VALID(unit)) {
        return;
    } else if (unit >= BCM_MAX_NUM_UNITS) {
        /* trunk_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX 
         */
        return;
    }
#ifdef BCM_QE2000_SUPPORT
    else if (unit == 0) {
        _bcm_qe2000_trunk_debug(unit);
        return;
    }
#endif

    tc = &TRUNK_CNTL(unit);
    BCM_TRUNK_MSG(("--- Debug ---\n"));
    BCM_TRUNK_MSG(("\nSW Information TRUNK - Unit %d\n", unit));
    BCM_TRUNK_MSG(("  Initialized         : %s\n", 
                   tc->init_state ? ((tc->init_state == ts_recovering) ?
                                     "Recovering" : "True")
                   : "False"));
    BCM_TRUNK_MSG(("  Lock                : 0x%08X\n", (uint32)tc->lock));
    if (tc->init_state != ts_none) {
        lock = (recursive_mutex_t *)((int)tc->lock + 12);
        BCM_TRUNK_MSG(("    Desc              : %s\n", lock->desc));
        BCM_TRUNK_MSG(("    Owner             : 0x%08X\n", (uint32)lock->owner));
        BCM_TRUNK_MSG(("    Count             : %d\n", lock->recurse_count));
    }
    BCM_TRUNK_MSG(("  Trunk groups        : %d\n", tc->ngroups));
    BCM_TRUNK_MSG(("  Trunk max ports     : %d\n", tc->nports));
    BCM_TRUNK_MSG(("  Port Select Criteria: 0x%x\n", tc->psc));

    for (index = 0; index < tc->ngroups; index++) {
        ti = &TRUNK_INFO(unit, index);
        if (ti->trunk_id == BCM_TRUNK_INVALID) {
            invalidList[invIdx++] = index;
        } else {
            BCM_TRUNK_MSG(("\n  Trunk %d\n", index));
            BCM_TRUNK_MSG(("      ID              : %d\n", ti->trunk_id));
            BCM_TRUNK_MSG(("      in use          : %d\n", ti->in_use));
            BCM_TRUNK_MSG(("      number of ports : %d\n", ti->num_ports));
            if (0 < ti->num_ports) {
                BCM_TRUNK_MSG(("         ports        : %d:%d  flags: %x", ti->tm[0], ti->tp[0], ti->member_flags[0]));
                for (jindex = 1; jindex < ti->num_ports; jindex++)
                    BCM_TRUNK_MSG((",  %d:%d  flags: %x", ti->tm[jindex], ti->tp[jindex], ti->member_flags[jindex]));
                BCM_TRUNK_MSG(("\n"));
            }
        }
    }
    BCM_TRUNK_MSG(("\nUnused Trunks: "));
    for (index = 0; index < invIdx; index++) {
        BCM_TRUNK_MSG(("%d ", invalidList[index]));
    }

    if (SOC_IS_SBX_FE(unit)) {
        (void)bcm_stk_my_modid_get(unit, &invIdx);
        (void)soc_sbx_node_port_get(unit, invIdx, 0, &funit, &fnode, &fport);
        (void)bcm_stk_modid_get(unit,  &mymod);
        (void)bcm_stk_modid_get(funit, &fmod);
        BCM_TRUNK_MSG(("\nModule Mapping, FE(unit=%d) => Module %d\n", unit, mymod));
        BCM_TRUNK_MSG(("                QE(unit=%d) => Module %d\n", funit, fmod));
        BCM_TRUNK_MSG(("\nPort Mapping, FE(unit:port) <=> QE(unit:port)  SPI-subport\n"));
        for (index = 0; index < 23; index++) {
            (void)soc_sbx_node_port_get(unit, invIdx, index, &funit, &fnode, &fport);
            if (SOC_IS_SBX_QE(funit)) {
                BCM_TRUNK_MSG(("\t%2d:%2d <=> %2d:%2d   %d\n", unit, index,
                               funit, fport, QE_SPI_SUBPORT_GET(funit, fport)));
            }
        }
    }
    BCM_TRUNK_MSG(("\n\n"));
#endif /* BROADCOM_DEBUG */

    return;
}


#ifdef BCM_FE2000_P3_SUPPORT
int
_bcm_fe2000_g2p3_trunk_outHdr_hw_set(int unit, uint32_t eIndex,
                                     uint32_t eteAddr)
{
    int                    rv = BCM_E_NONE;
    soc_sbx_g2p3_oi2e_t    sbxOutHdrIdx;

    soc_sbx_g2p3_oi2e_t_init(&sbxOutHdrIdx);
    sbxOutHdrIdx.eteptr = eteAddr;
    eIndex -= SBX_RAW_OHI_BASE;
    rv = soc_sbx_g2p3_oi2e_set(unit, eIndex, &sbxOutHdrIdx);

    return rv;
}
#endif /* BCM_FE2000_P3_SUPPORT */

/*
 * Function:
 *    _bcm_fe2000_trunk_outHdr_set
 * Purpose:
 *      Set an Out Header as specified.
 * Parameters:
 *      unit        - Device unit number.
 *      eIndex      - entry to modify
 *      eteAddr     - L2 ETE Entry address (index)
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
int
_bcm_fe2000_trunk_outHdr_set(int unit, uint32_t eIndex, uint32_t eteAddr)
{
    int rv = BCM_E_UNAVAIL;

    BCM_TRUNK_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit,
                     eIndex, eteAddr));
    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_trunk_outHdr_hw_set(unit, eIndex, eteAddr);
        break;
#endif
    default:
        rv = BCM_E_UNAVAIL;
    }

    BCM_TRUNK_VVERB(("\tProgrammed OutHdrIdx %d - ETE:%d (%s)\n",
                     eIndex, eteAddr, bcm_errmsg(rv)));
    if (BCM_FAILURE(rv)) {
        BCM_TRUNK_ERR(("Cannot set outHdrIdx2Etc[%d]: %s (%d)\n", eIndex,
                       bcm_errmsg(rv), rv));
    }

    BCM_TRUNK_VVERB(("%s(%d, %d, %d) - Exit(%s)\n", FUNCTION_NAME(), unit,
                     eIndex, eteAddr, bcm_errmsg(rv)));

    return rv;
}


#ifdef BCM_FE2000_P3_SUPPORT
int
_bcm_fe2000_g2p3_trunk_fte_hw_set(int unit, uint32_t eIndex, bcm_trunk_t tid,
                                  uint32_t outHdrIdx)
{
    soc_sbx_g2p3_ft_t    sbxFte;
    int                  rv;

    soc_sbx_g2p3_ft_t_init(&sbxFte);

    if (tid == BCM_TRUNK_INVALID) {
        rv = soc_sbx_g2p3_ft_set(unit, eIndex, &sbxFte);

    } else {
        rv = soc_sbx_g2p3_ft_get(unit, eIndex, &sbxFte);
        if (BCM_SUCCESS(rv)) {

            sbxFte.qid     = 0;
            sbxFte.lag     = 1;
            sbxFte.lagsize = 3;  /* 2^3  */
            sbxFte.lagbase = TRUNK_INDEX_SET(tid, 0);
            sbxFte.oi      = outHdrIdx;

            rv = soc_sbx_g2p3_ft_set(unit, eIndex, &sbxFte);
        }
    }

    return rv;
}
#endif /* BCM_FE2000_P3_SUPPORT */

/*
 * Function:
 *    _bcm_fe2000_trunk_fte_set
 * Purpose:
 *      Set an L2 FTE entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      tid       - trunk id
 *      outHdrIdx - OutHeader to program
 *
 * Notes:
 *   If tid == BCM_TRUNK_INVALID, the FT entry will be invalidated.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
int
_bcm_fe2000_trunk_fte_set(int unit, uint32_t eIndex, bcm_trunk_t tid,
                          uint32_t outHdrIdx)
{
    int rv = BCM_E_UNAVAIL;

    BCM_TRUNK_VVERB(("%s(%d, %d, %d, %d) - Enter\n", FUNCTION_NAME(), unit,
                     eIndex, tid, outHdrIdx));

    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_trunk_fte_hw_set(unit, eIndex, tid, outHdrIdx);
        break;
#endif
    default:
        rv = BCM_E_UNAVAIL;
    }

    if (BCM_FAILURE(rv)) {
        BCM_TRUNK_ERR(("Cannot set fte[%d]: %s (%d)\n", eIndex,
                       bcm_errmsg(rv), rv));
    }

    BCM_TRUNK_VVERB(("%s(%d, %d, %d, %d) - Exit(%s)\n", FUNCTION_NAME(), unit,
                     eIndex, tid, outHdrIdx, bcm_errmsg(rv)));
    return rv;
}




/*
 * Function:
 *      _bcm_fe2000_trunk_fte_inValidate
 * Purpose:
 *      Invalidate and clear an L2 FTE entry.
 * Parameters:
 *      unit    - Device unit number.
 *      eIndex  - entry to modify
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
int
_bcm_fe2000_trunk_fte_inValidate(int unit, uint32_t eIndex)
{
    int rv;

    BCM_TRUNK_VVERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, eIndex));

    rv = _bcm_fe2000_trunk_fte_set(unit, eIndex, BCM_TRUNK_INVALID, 0);

    BCM_TRUNK_VVERB(("%s(%d, %d) - Exit(%s)\n", FUNCTION_NAME(), unit, eIndex,
                     bcm_errmsg(rv)));
    return rv;
}


#ifdef BCM_FE2000_P3_SUPPORT
int
_bcm_fe2000_g2p3_trunk_ingLag_hw_set(int unit, uint32_t eIndex, uint32_t qid,
                                     uint32_t outHdrIdx)
{
    int                  rv;
    soc_sbx_g2p3_lag_t   sbxLag;

    soc_sbx_g2p3_lag_t_init(&sbxLag);
    sbxLag.qid = qid;
    sbxLag.oi  = outHdrIdx;
    rv = soc_sbx_g2p3_lag_set(unit, eIndex, &sbxLag);

    return rv;;
}
#endif /* BCM_FE2000_P3_SUPPORT */

/*
 * Function:
 *    _bcm_fe2000_trunk_ingLag_set
 * Purpose:
 *      Set an ingrLag entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      qid       - Queue for this entry to point to
 *      outHdrIdx - OutHeader to program
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
int
_bcm_fe2000_trunk_ingLag_set(int unit, uint32_t eIndex, uint32_t qid,
                             uint32_t outHdrIdx)
{
    int rv = BCM_E_UNAVAIL;

    BCM_TRUNK_VVERB(("%s(%d, %d, %d, %d) - Enter\n", FUNCTION_NAME(), unit,
                     eIndex, qid, outHdrIdx));

    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_trunk_ingLag_hw_set(unit, eIndex,
                                                  qid, outHdrIdx);
        break;
#endif
    default:
        rv = BCM_E_UNAVAIL;
    }

    BCM_TRUNK_VVERB(("\tProgrammed IngressLag %d - QID:%d  OHI:%d (%s)\n",
                     eIndex, qid, outHdrIdx, bcm_errmsg(rv)));

    BCM_TRUNK_VVERB(("%s(%d, %d, %d, %d) - Exit(%s)\n", FUNCTION_NAME(), unit,
                     eIndex, qid, outHdrIdx, bcm_errmsg(rv)));
    return rv;
}


#ifdef BCM_FE2000_P3_SUPPORT
int
_bcm_fe2000_g2p3_trunk_port2Etc_hw_set(int unit, uint32_t port, uint32_t sid)
{
    int                 rv;
    soc_sbx_g2p3_lp_t   sbxLogicalPort;

    rv = soc_sbx_g2p3_lp_get(unit, port, &sbxLogicalPort);

    if (BCM_SUCCESS(rv)) {
        sbxLogicalPort.pid = sid;

        rv = soc_sbx_g2p3_lp_set(unit, port, &sbxLogicalPort);
    }

    return rv;
}
#endif /* BCM_FE2000_P3_SUPPORT */

/*
 * Function:
 *    _bcm_fe2000_trunk_port2Etc_set
 * Purpose:
 *      Set an port2Etc entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      sid       - Source ID value of port2Etc value to program.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
int
_bcm_fe2000_trunk_port2Etc_set(int unit, uint32_t port, uint32_t sid)
{
    int rv = BCM_E_UNAVAIL;

    BCM_TRUNK_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit,
                     port, sid));

    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_trunk_port2Etc_hw_set(unit, port, sid);
        break;
#endif
    default:
        rv = BCM_E_UNAVAIL;
    }


    BCM_TRUNK_VVERB(("\tProgrammed port2Etc %d - SID:%d (%s)\n",
                     port, sid, bcm_errmsg(rv)));

    if (BCM_FAILURE(rv)) {
        BCM_TRUNK_ERR(("Cannot set port2Etc[%d]: %s (%d)\n", port,
                       bcm_errmsg(rv), rv));
    }

    BCM_TRUNK_VVERB(("%s(%d, %d, %d) - Exit(%s)\n", FUNCTION_NAME(), unit,
                     port, sid, bcm_errmsg(rv)));
    return rv;
}


#ifdef BCM_FE2000_P3_SUPPORT
int
_bcm_fe2000_g2p3_trunk_egrPort2Etc_hw_set(int unit, uint32_t eIndex,
                                          uint32_t sid)
{
    int                 rv;
    soc_sbx_g2p3_ep2e_t sbxEgrPort;

    rv = soc_sbx_g2p3_ep2e_get(unit, eIndex, &sbxEgrPort);

    if (BCM_SUCCESS(rv)) {
        sbxEgrPort.pid = sid;
        rv = soc_sbx_g2p3_ep2e_set(unit, eIndex, &sbxEgrPort);
    }

    return rv;
}
#endif /* BCM_FE2000_P3_SUPPORT */

/*
 * Function:
 *    _bcm_fe2000_trunk_egrPort2Etc_set
 * Purpose:
 *      Set an egrPort2Etc entry for Trunking.
 * Parameters:
 *      unit      - Device unit number.
 *      eIndex    - entry to modify
 *      sid       - Source ID value of egrPort2Etc value to program.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */
int
_bcm_fe2000_trunk_egrPort2Etc_set(int unit, uint32_t eIndex, uint32_t sid)
{
    int rv = BCM_E_UNAVAIL;

    BCM_TRUNK_VVERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit,
                     eIndex, sid));


    switch (SOC_SBX_CONTROL(unit)->ucodetype)
    {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_trunk_egrPort2Etc_hw_set(unit, eIndex, sid);
        break;
#endif
    default:
        rv = BCM_E_UNAVAIL;
    }

    if (BCM_FAILURE(rv)) {
        BCM_TRUNK_ERR(("Cannot set egrPort2Etc[%d]: %s (%d)\n", eIndex,
                       bcm_errmsg(rv), rv));
    }

    BCM_TRUNK_VVERB(("%s(%d, %d, %d) - Exit(%s)\n", FUNCTION_NAME(), unit,
                     eIndex, sid, bcm_errmsg(rv)));
    return rv;
}

/*
 * Function:
 *    _bcm_fe2000_trunk_set
 * Purpose:
 *      Add ports to a trunk group.
 * Parameters:
 *      unit       - Device unit number.
 *      tid        - The trunk ID to be affected.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Invalid ports specified.
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      the following fields of the bcm_trunk_add_info_t structure are ignored
 *      on SBX:
 *          flags
 *          dlf_index
 *          mc_index
 *          ipmc_index
 */

int
_bcm_fe2000_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_port_t          port;
    bcm_module_t        mymod, module;
    bcm_trunk_t         test_tid;
    trunk_private_t     removed, added;
    int                 index, jindex, tableIndex;
    int                 psc;
    uint32_t            qid;
    uint32_t            sid=0;
    uint32_t            eteAddr=0;
    uint32_t            outHdrIndex;
    uint32_t            member_flags;
    int                 result = BCM_E_NONE;
    int                 fab_unit, fab_node, fab_port;

    BCM_TRUNK_VVERB(("%s(%d, %d, [n=%d - ", FUNCTION_NAME(), unit, tid, add_info->num_ports));
    for (index = 0; index < add_info->num_ports; index++) {
        BCM_TRUNK_VVERB((" %d:%d flags:%x", add_info->tm[index], add_info->tp[index], add_info->member_flags[index]));
    }
    BCM_TRUNK_VVERB(("]) - Enter\n"));

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    result = bcm_stk_modid_get(unit, &mymod);
    if (BCM_E_NONE != result) {
        return result;
    }

    sal_memset(&added, 0, sizeof(trunk_private_t));
    sal_memset(&removed, 0, sizeof(trunk_private_t));

    /* assume all current port are to be removed */
    removed.num_ports = ti->num_ports;
    sal_memcpy(removed.tm, ti->tm, sizeof(bcm_module_t) * BCM_TRUNK_MAX_PORTCNT);
    sal_memcpy(removed.tp, ti->tp, sizeof(bcm_module_t) * BCM_TRUNK_MAX_PORTCNT);

    /*
     * Make sure the ports/nodes supplied are valid.
     * Make sure the ports don't belong to a different trunk.
     * Populate removed lists.
     */
    for (index = 0; index < add_info->num_ports; index++) {
        port = add_info->tp[index];
        if (FALSE == SOC_PORT_VALID(unit, port)) {
            return BCM_E_PARAM;
        }

        module = add_info->tm[index];
        if (!SOC_SBX_MODID_ADDRESSABLE(unit, module)) {
            return BCM_E_PARAM;
        }

        member_flags = add_info->member_flags[index];

        result = bcm_trunk_find(unit, module, port, &test_tid);
        if (BCM_E_NONE == result) {     /* mod:port was found */
            if (tid == test_tid) {      /* and it is already in this trunk */
                /* take this mod:port out of the removal list */
                for (tableIndex=0; tableIndex<removed.num_ports; tableIndex++) {
                    if ((port   == removed.tp[tableIndex]) &&
                        (module == removed.tm[tableIndex])) {
                        removed.num_ports -= 1;
                        removed.tp[tableIndex] = removed.tp[removed.num_ports];
                        removed.tm[tableIndex] = removed.tm[removed.num_ports];
                        break;
                    }
                }
            } else {                    /* what? port is in another trunk. */
                BCM_TRUNK_ERR(("Port %d:%d exists in trunk %d\n", module, port,
                               test_tid));
                return BCM_E_PARAM;
            }
        } else {
            added.member_flags[added.num_ports] = member_flags;
            added.tp[added.num_ports] = port;
            added.tm[added.num_ports] = module;
            added.num_ports++;
        }
    }

    if (add_info->psc <= 0) {
        if (tc->psc == _BCM_TRUNK_PSC_UNDEFINED) {
            psc = BCM_TRUNK_PSC_MACDA | BCM_TRUNK_PSC_MACSA | \
                  BCM_TRUNK_PSC_L4DS | BCM_TRUNK_PSC_L4SS | \
                  BCM_TRUNK_PSC_IPDA | BCM_TRUNK_PSC_IPSA;
        }else{
            psc = tc->psc;
        }
    } else {
        psc = add_info->psc;
    }

    result = bcm_fe2000_trunk_psc_set(unit, tid, psc);

    /* Remove ports that have been taken out of Trunk */
    if (BCM_E_NONE == result) {
        for (index = 0; index < removed.num_ports ; index++) {
            module = removed.tm[index];
            if (module == mymod) {
                /* only program port2Etc on this module */

                port = removed.tp[index];
                switch(SOC_SBX_CONTROL(unit)->ucodetype) {
                case SOC_SBX_UCODE_TYPE_G2P3:

                    result = soc_sbx_node_port_get(unit, mymod, port,
                                                   &fab_unit,
                                                   &fab_node,
                                                   &fab_port);
                    sid = SOC_SBX_PORT_SID(unit, fab_node, fab_port);
                    break;
                default:
                    BCM_TRUNK_ERR(("unknonw ucode type.\n"));
                    result = BCM_E_CONFIG;
                    break;
                }

                if (BCM_FAILURE(result)) {
                    BCM_TRUNK_ERR(("Failed to compute sid.\n"));
                    break;
                }

                /* port2Etc.SID */
                result = _bcm_fe2000_trunk_port2Etc_set(unit, port, sid);
                if (BCM_E_NONE != result) break;

                /* egrPort2Etc.SID */
                result = _bcm_fe2000_trunk_egrPort2Etc_set(unit, port, sid);
                if (BCM_E_NONE != result) break;
            }
        }
    }

    /* if there are no members in the lag, no point in setting these tables */
    if (BCM_E_NONE == result) {
        if (0 < add_info->num_ports) {
            for (jindex = 0; jindex < TRUNK_SBX_FIXED_PORTCNT; jindex++) {
                index   = jindex % add_info->num_ports;
                port    = add_info->tp[index];
                module  = add_info->tm[index];
                result  = soc_sbx_node_port_get(unit, module, port,
                                                &fab_unit, &fab_node, &fab_port);
                if (BCM_E_NONE != result) break;
                qid  = SOC_SBX_NODE_PORT_TO_QID(unit,fab_node, fab_port, 
                                                NUM_COS(unit));

                /* Member Flag: egress_disable */
                if (add_info->member_flags[index] & BCM_TRUNK_MEMBER_EGRESS_DISABLE) {
                   result = BCM_E_PARAM; 
                   BCM_TRUNK_VERB(("%s(BCM_TRUNK_MEMBER_EGRESS_DISABLE not available) \n", FUNCTION_NAME()));
                   break;
                }

                /* Member Flag: egress_drop */
                if (add_info->member_flags[index] & BCM_TRUNK_MEMBER_EGRESS_DROP) {
#ifdef BCM_QE2000_SUPPORT
                   qid  = (SBX_QID_END - 1) - NUM_COS(unit);
                   BCM_TRUNK_VERB(("%s(BCM_TRUNK_MEMBER_EGRESS_DROP on qid %x) \n", FUNCTION_NAME(),qid));
#endif /* BCM_QE2000_SUPPORT */
                }

                /* ingrLag table - outHdrIdx is ignored */

                tableIndex = TRUNK_INDEX_SET(tid, jindex);
                result = _bcm_fe2000_trunk_ingLag_set(unit, tableIndex, qid, 0);
                if (BCM_E_NONE != result) break;
            }
        } else if (0 == add_info->num_ports) {
            for (jindex = 0; jindex < TRUNK_SBX_FIXED_PORTCNT; jindex++) {
                tableIndex = TRUNK_INDEX_SET(tid, jindex);
                result = _bcm_fe2000_trunk_ingLag_set(unit, tableIndex, 0,
                                                      tc->invalid_oi);
                if (BCM_E_NONE != result) break;
            }
        }
    }

    if (BCM_E_NONE == result) {
        for (index = 0; index < added.num_ports ; index++) {
            module = added.tm[index];
            if (module == mymod) {
                /* only program port2Etc on this module */

                port = added.tp[index];
                member_flags = added.member_flags[index];
                /* port2Etc.SID */
                switch(SOC_SBX_CONTROL(unit)->ucodetype) {
                case SOC_SBX_UCODE_TYPE_G2P3:
                    sid = SOC_SBX_TRUNK_FTE(unit, tid);
                    break;
                default:
                    BCM_TRUNK_ERR(("unknonw ucode type.\n"));
                    result = BCM_E_CONFIG;
                    break;
                }
                if (BCM_E_NONE != result) break;

                /* Member Flag: ingress_disable */
                if (add_info->member_flags[index] & BCM_TRUNK_MEMBER_INGRESS_DISABLE) {
                   BCM_TRUNK_VERB(("%s(BCM_TRUNK_MEMBER_INGRESS_DISABLE set) \n", FUNCTION_NAME()));
                   } else {
                   result = _bcm_fe2000_trunk_port2Etc_set(unit, port, sid);
                   if (BCM_E_NONE != result) break;

                   /* egrPort2Etc.SID */
                   result = _bcm_fe2000_trunk_egrPort2Etc_set(unit, port, sid);
                   if (BCM_E_NONE != result) break;
               }
            }
        }
    }


    /*
     * Adjust outHdr mapping to one of member lag ports. Always pick the first
     * port in the list as it is always known to exist and be valid.
     */
#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_SBX_CONTROL(unit)->ucodetype ==  SOC_SBX_UCODE_TYPE_G2P3) {
        soc_sbx_g2p3_etel2_t sbxEteL2;

        if (BCM_SUCCESS(result)) {
            eteAddr = SOC_SBX_PORT_ETE(unit, add_info->tp[0]);
            result = soc_sbx_g2p3_etel2_get(unit, eteAddr, &sbxEteL2);
   
            if (BCM_SUCCESS(result)) {
                eteAddr = SOC_SBX_TRUNK_ETE(unit, add_info->tp[0]);
                result = soc_sbx_g2p3_etel2_get(unit, eteAddr, &sbxEteL2);
            }
        }

        /* VLAN based static LAG support */
        if (BCM_SUCCESS(result)) {
            uint32 static_lag;
            static_lag = (add_info->psc == BCM_TRUNK_PSC_VLANINDEX) ? 1 : 0;
            result = soc_sbx_g2p3_static_lag_set(unit, static_lag);                                                 
	}
    }
#endif /* BCM_FE2000_P3_SUPPORT */


    if (BCM_E_NONE == result) {
        outHdrIndex = SOC_SBX_TRUNK_TO_OHI(tid);
        result = _bcm_fe2000_trunk_outHdr_set(unit, outHdrIndex, eteAddr);
    }

    if (BCM_E_NONE == result) {
        ti->num_ports = add_info->num_ports;
        for (index = 0; index < add_info->num_ports; index++) {
            ti->tm[index] = add_info->tm[index];
            ti->tp[index] = add_info->tp[index];
            ti->member_flags[index] = add_info->member_flags[index];
        }
    }

    BCM_TRUNK_VVERB(("%s(%d, %d, *) - Exit(%s)\n",
                     FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));
    return result;
}


/*
 * Function:
 *    bcm_fe2000_trunk_vlan_remove_port
 * Purpose:
 *      When a port is removed from a VLAN, the vlan code calls this function
 *      to fix up the McPort2Etc table.
 * Parameters:
 *      unit - Device unit number.
 *      vid  - Vlan port is being removed from.
 *      port - The port being removed.
 * Returns:
 *      BCM_E_NONE      - Success or port not part of any lag.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      If the port is not a member of any lag, no action is taken
 */

int
bcm_fe2000_trunk_vlan_remove_port(int unit, bcm_vlan_t vid, bcm_port_t port)
{
    BCM_VERB(("%s(unit=%d, vid=%d, port=%d) - This API is deprecated.\n",
              FUNCTION_NAME(), unit, vid, port));

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_trunk_create
 * Purpose:
 *      Allocate an available Trunk ID from the pool
 *      bcm_trunk_create_id.
 * Parameters:
 *      unit - Device unit number.
 *      tid - (Out), The trunk ID.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_FULL      - Trunk table full, no more trunks available.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

int
bcm_fe2000_trunk_create(int unit, bcm_trunk_t *tid)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 result = BCM_E_FULL;
    int                 index;

    BCM_TRUNK_VERB(("%s(%d, *) - Enter\n", FUNCTION_NAME(), unit));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_DB_LOCK(unit);

    *tid = 0;
    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, *tid);

    for (index = 0; index < tc->ngroups; index++) {
        if (BCM_TRUNK_INVALID == ti->trunk_id) {
            result = bcm_trunk_create_id(unit, index);
            if (BCM_E_NONE == result)
                *tid = index;
            break;
        }
        ti++;
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, *tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_trunk_create_id
 * Purpose:
 *      Create the software data structure for this trunk ID and program the
 *      hardware for this TID. User must call bcm_trunk_set() to finish setting
 *      up this trunk.
 * Parameters:
 *      unit - Device unit number.
 *      tid - The trunk ID.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_EXISTS    - TID already used
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

int
bcm_fe2000_trunk_create_id(int unit, bcm_trunk_t tid)
{
    trunk_private_t        *ti;
    uint32_t                eIndex;
    uint32_t                outHdrIdx;
    int                     result = BCM_E_EXISTS;

    BCM_TRUNK_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    /* Go twiddle the chip */

    eIndex = SOC_SBX_TRUNK_FTE(unit, tid);
    outHdrIdx = 0;

    ti = &TRUNK_INFO(unit, tid);
    if (ti->trunk_id == BCM_TRUNK_INVALID) {
        result = _bcm_fe2000_trunk_fte_set(unit, eIndex, tid, outHdrIdx);

        if (BCM_E_NONE == result) {
            ti->trunk_id  = tid;
            ti->in_use    = FALSE;
            ti->num_ports = 0;
        }
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_psc_set
 * Purpose:
 *      Set the trunk selection criteria.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - The trunk ID to be affected.
 *      psc  - Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - TID out of range
 *      BCM_E_PARAM     - psc value specified is not supported
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      On this platform, port selection criteria is global and cannot be
 *      configured per trunk group. The rule is, last psc_set wins and affects
 *      EVERY trunk group!
 */

int
_bcm_fe2000_g2p3_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_cntl_t       *tc;
    int                 result = BCM_E_NOT_FOUND;
    uint32_t            flags = 0;
    soc_sbx_g2p3_state_t *pFe;
    uint32_t rc;
    uint32_t found = 1;

    BCM_TRUNK_VERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(),
                    unit, tid, psc));
    BCM_TRUNK_VVERB(("  Reminder, this is a global action, not "
                     "per Trunk Group.\n"));

    tc = &TRUNK_CNTL(unit);

    if (tc->psc == psc) {
        return BCM_E_NONE;
    }

    /* only allow update if PSC is not already set */
    if (tc->psc != _BCM_TRUNK_PSC_UNDEFINED){
        return BCM_E_RESOURCE;
    }
        
    result = BCM_E_NONE;
       
    /* check for 'standard' psc values */
    switch (psc){

    case BCM_TRUNK_PSC_SRCMAC:
        flags |= SB_G2P3_PSC_MAC_SA;
        break;
    case BCM_TRUNK_PSC_DSTMAC:
        flags |= SB_G2P3_PSC_MAC_DA;
        break;
    case BCM_TRUNK_PSC_SRCDSTMAC:
        flags |= (SB_G2P3_PSC_MAC_SA | SB_G2P3_PSC_MAC_DA);
        break;
    case BCM_TRUNK_PSC_SRCIP:
        flags |= SB_G2P3_PSC_IP_SA;
        break;
    case BCM_TRUNK_PSC_DSTIP:
        flags |= SB_G2P3_PSC_IP_DA;
        break;
    case BCM_TRUNK_PSC_SRCDSTIP:
        flags |= (SB_G2P3_PSC_IP_SA | SB_G2P3_PSC_IP_DA);
        break;
    case BCM_TRUNK_PSC_VLANINDEX:
        flags = 0;
        break;
    default:
        /* try extended flags below */
        found = 0;
    }

    if (!found){

        if (psc & ~_BCM_TRUNK_PSC_EXTENDED_FLAGS) {
            BCM_TRUNK_ERR(("unit %d: [%s] Invalid psc 0x%x\n",
                           unit, FUNCTION_NAME(), psc));
            return BCM_E_PARAM;
        }

        if (psc & BCM_TRUNK_PSC_MACSA){
            flags |= SB_G2P3_PSC_MAC_SA;
        }
        if (psc & BCM_TRUNK_PSC_MACDA){
            flags |= SB_G2P3_PSC_MAC_DA;
        }
        if (psc & BCM_TRUNK_PSC_IPSA){
            flags |= SB_G2P3_PSC_IP_SA;
        }
        if (psc & BCM_TRUNK_PSC_IPDA){
            flags |= SB_G2P3_PSC_IP_DA;
        }
        if (psc & BCM_TRUNK_PSC_L4SS){
            flags |= SB_G2P3_PSC_L4SS;
        }
        if (psc & BCM_TRUNK_PSC_L4DS){
            flags |= SB_G2P3_PSC_L4DS;
        }
        if (psc & BCM_TRUNK_PSC_VID){
            flags |= SB_G2P3_PSC_VID;
        }
    }

    G2P3_FE_HANDLER_GET(unit, pFe);
    rc = soc_sbx_g2p3_pp_hash_templates_set(pFe, flags);
    if (rc != SOC_E_NONE){
        result = translate_sbx_result(rc);
        BCM_TRUNK_ERR(("soc_sbx_g2p3_pp_hash_templates_set failed: flags 0x%x, %s (0x%x)\n", flags, bcm_errmsg(result), result));
        return BCM_E_INTERNAL;

    }

    if (psc != BCM_TRUNK_PSC_VLANINDEX) {
        rc = soc_sbx_g2p3_pp_psc_set(unit, flags);
        if (rc != SOC_E_NONE){
            result = translate_sbx_result(rc);
            BCM_TRUNK_ERR(("soc_sbx_g2p3_pp_psc_set failed: flags 0x%x, %s (0x%x)\n", flags, bcm_errmsg(result), result));
            return BCM_E_INTERNAL;
        }   
    }

    tc->psc = psc;

    BCM_TRUNK_VERB(("%s(%d, %d, %d) - Exit(%s)\n",
                    FUNCTION_NAME(), unit, tid, psc, bcm_errmsg(result)));

    return result;
}

int
bcm_fe2000_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_cntl_t       *tc;
    int                 result = BCM_E_NOT_FOUND;
    uint32_t            eType;

    BCM_TRUNK_VERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(),
                    unit, tid, psc));
    BCM_TRUNK_VVERB(("  Reminder, this is a global action, not "
                     "per Trunk Group.\n"));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    if (TRUNK_TID_VALID(unit, tid)) {

        TRUNK_DB_LOCK(unit);

        tc = &TRUNK_CNTL(unit);

#if SBX_HASH_DEFINED
#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_IS_SBX_G2P3(unit)) {
            int rv = _bcm_fe2000_g2p3_trunk_psc_set(unit, tid, psc);
            TRUNK_DB_UNLOCK(unit);
            return rv;
        }
#endif /* BCM_FE2000_P3_SUPPORT */

        result = BCM_E_NONE;
        if (psc != tc->psc) {
            switch (psc) {
            case BCM_TRUNK_PSC_SRCMAC:
            case BCM_TRUNK_PSC_DSTMAC:
            case BCM_TRUNK_PSC_SRCDSTMAC:
                eType = SB_FE_HASH_OVER_L2;
                break;
            case BCM_TRUNK_PSC_SRCIP:
            case BCM_TRUNK_PSC_DSTIP:
            case BCM_TRUNK_PSC_SRCDSTIP:
                eType = SB_FE_HASH_OVER_L3;
                break;
            case BCM_TRUNK_PSC_L4SS:
            case BCM_TRUNK_PSC_L4DS:
                eType = SB_FE_HASH_OVER_L4;
                break;
            default:
                result = BCM_E_PARAM;
            }

            if (BCM_E_NONE == result) {                
#else /* SBX_HASH_DEFINED */
                tc->psc = psc;
                BCM_TRUNK_VVERB(("This API currently performs no hardware action.\n"));
#endif /* SBX_HASH_DEFINED */
            }
        }
        TRUNK_DB_UNLOCK(unit);
    }

    BCM_TRUNK_VERB(("%s(%d, %d, %d) - Exit(%s)\n",
                    FUNCTION_NAME(), unit, tid, psc, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_psc_get
 * Purpose:
 *      Get the trunk selection criteria.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - The trunk ID to be used.
 *      psc  - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - TID out of range
 */

int
bcm_fe2000_trunk_psc_get(int unit, bcm_trunk_t tid, int *psc)
{
    int                 result = BCM_E_NOT_FOUND;

    BCM_TRUNK_VERB(("%s(%d, %d, *) - Enter\n", FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    if TRUNK_TID_VALID(unit, tid) {
        *psc = TRUNK_CNTL(unit).psc;
        result = BCM_E_NONE;
    }

    BCM_TRUNK_VERB(("%s(%d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, tid, *psc, bcm_errmsg(result)));

    return result;
}


/*
 * Function:
 *    _bcm_fe2000_g2p3_trunk_psc_recover
 * Purpose:
 *    recover the port selection criteria software state
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_*
 */

int
_bcm_fe2000_g2p3_trunk_psc_recover(int unit)
{
    int                   rv, sb_rv;
    trunk_cntl_t         *tc;
    uint32                flags;
    soc_sbx_g2p3_state_t *pFe;

    G2P3_FE_HANDLER_GET(unit, pFe);
    sb_rv = soc_sbx_g2p3_pp_hash_templates_get(pFe, &flags);
    if (sb_rv != SB_OK) {
        rv = translate_sbx_result(sb_rv);
        BCM_TRUNK_ERR(("soc_sbx_g2p3_pp_hash_templates_get failed: %s\n",
                       bcm_errmsg(rv)));
        return BCM_E_INTERNAL;
    }

    tc = &TRUNK_CNTL(unit);

    /* convert the sbx flags to a BCM PSC, integer based types are lost
     * The extended-flagged equivalent is recovered
     */
    tc->psc = 0;
    if (flags & SB_G2P3_PSC_MAC_SA) {
        tc->psc |= BCM_TRUNK_PSC_MACSA;
    }
    if (flags & SB_G2P3_PSC_MAC_DA) {
        tc->psc |= BCM_TRUNK_PSC_MACDA;
    }
    if (flags & SB_G2P3_PSC_IP_SA) {
        tc->psc |= BCM_TRUNK_PSC_IPSA;
    }
    if (flags & SB_G2P3_PSC_IP_DA) {
        tc->psc |= BCM_TRUNK_PSC_IPDA;
    }
    if (flags & SB_G2P3_PSC_L4SS) {
        tc->psc |= BCM_TRUNK_PSC_L4SS;
    }
    if (flags & SB_G2P3_PSC_L4DS) {
        tc->psc |= BCM_TRUNK_PSC_L4DS;
    }
    if (flags & SB_G2P3_PSC_VID) {
        tc->psc |= BCM_TRUNK_PSC_VID;
    }
    if (flags == 0) {
        tc->psc = BCM_TRUNK_PSC_VLANINDEX;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *    bcm_trunk_chip_info_get
 * Purpose:
 *      Get device specific trunking information.
 * Parameters:
 *      unit    - Device unit number.
 *      ta_info - (OUT) Chip specific Trunk information.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 */

int
bcm_fe2000_trunk_chip_info_get(int unit, bcm_trunk_chip_info_t *ta_info)
{
    trunk_cntl_t   *tc;
    int             result = BCM_E_NONE;

    BCM_TRUNK_VERB(("%s(%d, *) - Enter\n", FUNCTION_NAME(), unit));

    TRUNK_CHECK_INIT(unit, ts_init);

    tc = &TRUNK_CNTL(unit);

    ta_info->trunk_group_count = tc->ngroups;
    ta_info->trunk_id_min = 0;
    ta_info->trunk_id_max = tc->ngroups - 1;
    ta_info->trunk_ports_max = BCM_TRUNK_MAX_PORTCNT;
    ta_info->trunk_fabric_id_min = -1;
    ta_info->trunk_fabric_id_max = -1;
    ta_info->trunk_fabric_ports_max = -1;

    BCM_TRUNK_VERB(("%s(%d, *) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_set
 * Purpose:
 *      Add ports to a trunk group.
 * Parameters:
 *      unit       - Device unit number.
 *      tid        - The trunk ID the ports are added to.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - the specified TID was not found
 *      BCM_E_PARAM     - too or invalid many ports specified
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      Any existing ports in the trunk group will be replaced with new ones.
 */
int
bcm_fe2000_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 index;
    int                 result = BCM_E_NONE;

    BCM_TRUNK_VERB(("%s(%d, %d, [n=%d - ", FUNCTION_NAME(), unit, tid, add_info->num_ports));
    for (index = 0; index < add_info->num_ports; index++) {
        BCM_TRUNK_VERB((" %d:%d flags:%x", add_info->tm[index], add_info->tp[index], add_info->member_flags[index]));
    }
    BCM_TRUNK_VERB(("]) - Enter\n"));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    /* make sure trunk is in use */
    if (TRUNK_TID_VALID(unit, tid)) {

        /* validate trunk PSC */
        if ( (tc->psc != _BCM_TRUNK_PSC_UNDEFINED) &&
             (add_info->psc >= 0) )
        {
            if (add_info->psc != tc->psc){
                BCM_TRUNK_ERR(("Trunk PSC mismatch (0x%x != 0x%x)\n", tc->psc, add_info->psc));
                TRUNK_DB_UNLOCK(unit);
                return BCM_E_PARAM;
            }
        }

        /* Check number of ports in trunk group */
        if (TRUNK_PORTCNT_VALID(unit, add_info->num_ports)) {
            result = _bcm_fe2000_trunk_set(unit, tid, add_info);
            if (BCM_E_NONE == result) {
                ti->in_use  = TRUE;
            }
        } else {
            result = BCM_E_PARAM;
        }
    } else {
        result = BCM_E_NOT_FOUND;
    }

    /* Call the registered callbacks. */
    for (index = 0; index < _BCM_TRUNK_MAX_CALLBACK; index++) {
        if (tc->callback_func[index]) {
            (tc->callback_func[index])(unit, tid, add_info, tc->callback_user_data[index]);
#if 0
            if (!BCM_SUCCESS(rv)) {
	        result = rv;
                BCM_TRUNK_ERR(("Callback number %d failed with %d\n", index, rv));
	    }
#endif
        }
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("%s(%d, %d, *) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_get
 * Purpose:
 *      Return information of a given trunk ID.
 * Parameters:
 *      unit   - Device unit number.
 *      tid    - Trunk ID.
 *      t_data - (Out), data about this trunk.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - the specified TID was not found
 */

int
bcm_fe2000_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 index;
    int                 result = BCM_E_NOT_FOUND;

    BCM_TRUNK_VERB(("%s(%d, %d, *) - Enter\n", FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    sal_memset(t_data, 0, sizeof(bcm_trunk_add_info_t));
    if (ti->trunk_id != BCM_TRUNK_INVALID) {
        result = BCM_E_NONE;
        t_data->psc = tc->psc;
        t_data->dlf_index = t_data->mc_index = t_data->ipmc_index = -1;
        t_data->num_ports = ti->num_ports;

        for (index = 0; index < ti->num_ports; index++) {
            t_data->member_flags[index] = ti->member_flags[index];
            t_data->tm[index] = ti->tm[index];
            t_data->tp[index] = ti->tp[index];
        }
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VVERB(("%s(%d, %d, [n=%d - ", FUNCTION_NAME(), unit, tid, t_data->num_ports));
    for (index = 0; index < t_data->num_ports; index++) {
        BCM_TRUNK_VVERB((" %d:%d flags:%x", t_data->tm[index], t_data->tp[index], t_data->member_flags[index]));
    }
    BCM_TRUNK_VERB(("]) - Exit(%s)\n", bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_destroy
 * Purpose:
 *      Removes a trunk group. Performs hardware steps neccessary to tear
 *      down a create trunk.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - Trunk Id.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - Trunk does not exist
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      The return code of the trunk_set call is purposely ignored.
 */

int
bcm_fe2000_trunk_destroy(int unit, bcm_trunk_t tid)
{
    trunk_cntl_t           *tc;
    trunk_private_t        *ti;
    uint32_t                eIndex;
    bcm_trunk_add_info_t    add_info;
    int                     result = BCM_E_NOT_FOUND;
    uint32_t                tIndex;

    BCM_TRUNK_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit, ts_init);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    eIndex = SOC_SBX_TRUNK_FTE(unit, tid);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    if (ti->trunk_id != BCM_TRUNK_INVALID) {
        if (0 < ti->num_ports) {
            sal_memset((void *)&add_info, 0, sizeof(bcm_trunk_add_info_t));
            add_info.psc = tc->psc;
            (void)bcm_trunk_set(unit, tid, &add_info);
        }

        result = _bcm_fe2000_trunk_fte_inValidate(unit, eIndex);

        if (BCM_E_NONE == result) {
            ti->trunk_id  = BCM_TRUNK_INVALID;
            ti->in_use    = FALSE;
            ti->num_ports = 0;
        }
    }

    /* If the last trunk has been destroyed, reset the PSC criteria to UNDEFINED */
    for (tIndex=0; tIndex < SBX_MAX_TRUNKS; tIndex++){
        ti = &TRUNK_INFO(unit, tIndex);
        if (ti->in_use == TRUE) {
            break;
        }
    }
    if (tIndex == SBX_MAX_TRUNKS){
        tc->psc = _BCM_TRUNK_PSC_UNDEFINED;
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, tid, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_detach
 * Purpose:
 *      Shuts down the trunk module.
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_fe2000_trunk_detach(int unit)
{
    trunk_cntl_t       *tc;
    int                 result = BCM_E_NONE;

    BCM_TRUNK_VERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    /* Don't use TRUNK_CHECK_INIT macro here - If module is not initialized
     * just return OK.
     */
    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    } else if (unit >= BCM_MAX_NUM_UNITS) {
        /* trunk_control is sized by BCM_MAX_NUM_UNITS, not BCM_CONTROL_MAX,
         * or BCM_UNITS_MAX 
         */
        return BCM_E_UNIT;
    } else if (TRUNK_CNTL(unit).init_state == ts_none) {
        return BCM_E_NONE;
    }

    tc = &TRUNK_CNTL(unit);

    TRUNK_DB_LOCK(unit);

    tc->init_state = ts_none;

    /* Free trunk_private_t (t_info) structures */
    if (NULL != tc->t_info) {
        sal_free(tc->t_info);
        tc->t_info = NULL;
    }

    /* Set number of ports and groups to zero */
    tc->ngroups = 0;
    tc->nports  = 0;

    TRUNK_DB_UNLOCK(unit);

    /* Destroy LOCK (no more data to protect */
    if (NULL != tc->lock) {
        sal_mutex_destroy(tc->lock);
        tc->lock = NULL;
    }

    BCM_TRUNK_VERB(("%s(%d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *    bcm_trunk_bitmap_expand
 * Purpose:
 *      Given a port bitmap, if any of the ports are in a trunk,
 *      add all the trunk member ports to the bitmap.
 * Parameters:
 *      unit     - Device unit number.
 *      pbmp_ptr - Input/output port bitmap
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 * Notes:
 */

int
bcm_fe2000_trunk_bitmap_expand(int unit, bcm_pbmp_t *pbmp_ptr)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_trunk_t         tid;
    bcm_port_t          port;
    int                 index;
    bcm_pbmp_t          pbmp, t_pbmp;
    int                 result = BCM_E_NONE;

    BCM_TRUNK_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, SOC_PBMP_WORD_GET(*pbmp_ptr,0)));

    TRUNK_CHECK_INIT(unit, ts_init);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    for (tid = 0; tid < tc->ngroups; tid++) {
        ti = &TRUNK_INFO(unit, tid);
        if ((TRUE == ti->in_use) && (0 < ti->num_ports)) {
            BCM_PBMP_CLEAR(t_pbmp);
            BCM_PBMP_CLEAR(pbmp);
            for (index=0; index<ti->num_ports; index++) {
                /* create the port bitmap of this trunk */
                port = ti->tp[index];
                BCM_PBMP_PORT_ADD(t_pbmp, port);    /* construct temp bitmap */
            }
            BCM_PBMP_ASSIGN(pbmp, t_pbmp);      /* save a copy */
            BCM_PBMP_AND(t_pbmp, *pbmp_ptr);    /* find common ports */

            /* if lists have common member */
            if (TRUE == BCM_PBMP_NOT_NULL(t_pbmp)) {
                BCM_PBMP_OR(*pbmp_ptr, pbmp);   /* add saved member set */
            }
        }
    }
    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("%s(%d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, SOC_PBMP_WORD_GET(*pbmp_ptr,0), bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_mcast_join
 * Purpose:
 *    Add the trunk group to existing MAC multicast entry.
 * Parameters:
 *      unit - Device unit number.
 *      tid - Trunk Id.
 *      vid - Vlan ID.
 *      mac - MAC address.
 * Returns:
 *    BCM_E_XXX
 * Notes:
 *      Applications have to remove the MAC multicast entry and re-add in with
 *      new port bitmap to remove the trunk group from MAC multicast entry.
 */

int
bcm_fe2000_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid, sal_mac_addr_t mac)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcm_trunk_find
 * Description:
 *      Get trunk id that contains the given system port
 * Parameters:
 *      unit    - Device unit number
 *      modid   - Module ID
 *      port    - Port number
 *      tid     - (OUT) Trunk id
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_NOT_FOUND - The module:port combo was not found in a trunk.
 */
int
bcm_fe2000_trunk_find(int unit, bcm_module_t modid, bcm_port_t port,
                      bcm_trunk_t *tid)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_trunk_t         t;
    int                 index;
    int                 result = BCM_E_NOT_FOUND;

    BCM_TRUNK_VERB(("%s(%d, %d, %d, *) - Enter\n", FUNCTION_NAME(), unit, modid, port));

    TRUNK_CHECK_INIT(unit, ts_init);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    *tid = BCM_TRUNK_INVALID;
    for (t = 0; t < tc->ngroups; t++) {
        ti = &TRUNK_INFO(unit, t);
        if ((TRUE == ti->in_use) && (0 < ti->num_ports)) {
            for (index = 0; index < ti->num_ports; index++) {
                if ((ti->tm[index] == modid) && (ti->tp[index] == port)) {
                    *tid = ti->trunk_id;
                    result = BCM_E_NONE;
                    break;
                }
            }
        }
        if (BCM_E_NONE == result) {
            break;
        }
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("%s(%d, %d, %d, %d) - Exit(%s)\n",
                   FUNCTION_NAME(), unit, modid, port, *tid, bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *    bcm_trunk_egress_set
 * Description:
 *    Set switching only to indicated ports from given trunk.
 * Parameters:
 *    unit - Device unit number.
 *      tid - Trunk Id.  Negative trunk id means set all trunks.
 *    pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_fe2000_trunk_egress_set(int unit, bcm_trunk_t tid, bcm_pbmp_t pbmp)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));
#ifdef BROADCOM_DEBUG
    _bcm_fe2000_trunk_debug(unit);
#endif
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcm_trunk_egress_get
 * Description:
 *    Retrieve bitmap of ports for which switching is enabled for trunk.
 * Parameters:
 *    unit - Device unit number.
 *      tid - Trunk Id.  Negative trunk id means choose any trunk.
 *    pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_fe2000_trunk_egress_get(int unit, bcm_trunk_t tid, bcm_pbmp_t *pbmp)
{

    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                    FUNCTION_NAME(), unit, tid));
    
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ucast_set
 * Description:
 *      Set the trunk override over UC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_ucast_set(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int modid, int enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ucast_get
 * Description:
 *      Get the trunk override over UC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_ucast_get(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int modid, int *enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_mcast_set
 * Description:
 *      Set the trunk override over MC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_mcast_set(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int idx, int enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_mcast_get
 * Description:
 *      Get the trunk override over MC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_mcast_get(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int idx, int *enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ipmc_set
 * Description:
 *      Set the trunk override over IPMC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_ipmc_set(int unit, bcm_port_t port,
                            bcm_trunk_t tid, int idx, int enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ipmc_get
 * Description:
 *      Get the trunk override over IPMC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_ipmc_get(int unit, bcm_port_t port,
                            bcm_trunk_t tid, int idx, int *enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_vlan_set
 * Description:
 *      Set the trunk override over VLAN.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_vlan_set(int unit, bcm_port_t port,
                            bcm_trunk_t tid, bcm_vlan_t vid, int enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_vlan_get
 * Description:
 *      Get the trunk override over VLAN.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_override_vlan_get(int unit, bcm_port_t port,
                            bcm_trunk_t tid, bcm_vlan_t vid, int *enable)
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_pool_set
 * Description:
 *      Set trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - Device unit number.
 *      port    - Port number, -1 to all ports.
 *      tid     - Trunk id.
 *      size    - Trunk pool size.
 *      weights - Weights for each port, all 0 means weighted fair.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_pool_set(int unit, bcm_port_t port, bcm_trunk_t tid,
                   int size, const int weights[BCM_TRUNK_MAX_PORTCNT])
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_pool_get
 * Description:
 *      Get trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - Device unit number.
 *      port    - Port number.
 *      tid     - Trunk id.
 *      size    - (OUT) Trunk pool size.
 *      weights - (OUT) Weights (total count) for each port.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_fe2000_trunk_pool_get(int unit, bcm_port_t port, bcm_trunk_t tid,
                   int *size, int weights[BCM_TRUNK_MAX_PORTCNT])
{
    BCM_TRUNK_VERB(("%s(unit %d, tid %d) - This API is not available.\n",
                   FUNCTION_NAME(), unit, tid));

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *    _bcm_fe2000_trunk_recover
 * Purpose:
 *    recover the internal trunk software state
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_*
 */

static int
_bcm_fe2000_trunk_recover(int unit)
{
    trunk_cntl_t      *tc;
    trunk_private_t   *ti;
    int                rv;
    int                fab_node, fab_port, tid=0;
    bcm_gport_t        fab_gport, switch_gport;
    uint32             lagi, lagi_max;
    soc_sbx_g2p3_lag_t lag_ent;

    tc = &TRUNK_CNTL(unit);

    if (tc->init_state != ts_recovering) {
        return BCM_E_CONFIG;
    }

    lagi_max = SBX_MAX_TRUNKS * TRUNK_SBX_FIXED_PORTCNT;

    for (lagi = 0; lagi < lagi_max; lagi++) {
        rv = soc_sbx_g2p3_lag_get(unit, lagi, &lag_ent);
        if (BCM_FAILURE(rv)) {
            BCM_TRUNK_ERR((_SBX_D(unit, "Failed to read lag[0x%x] :%s\n"),
                           lagi, bcm_errmsg(rv)));
            return rv;
        }

        /* is this lag entry is disabled? */
        if (lag_ent.oi == tc->invalid_oi) {
            continue;
        }

        tid = lagi >> TRUNK_SBX_HASH_SIZE;
        ti = &TRUNK_INFO(unit, tid);
        
        if ((lagi & ((1<<TRUNK_SBX_HASH_SIZE) - 1)) == 0){
            BCM_TRUNK_VERB((_SBX_D(unit, "Recovering Trunk[%3d] lagi=0x%x\n"),
                            tid, lagi));
            ti->in_use = TRUE;
            ti->trunk_id = tid;
            ti->num_ports = 0;
        }

        
#ifdef BCM_QE2000_SUPPORT
        /* Cannot recover original port if found to be in 
         * BCM_TRUNK_MEMBER_EGRESS_DROP mode */
        if (lag_ent.qid == ((SBX_QID_END - 1) - NUM_COS(unit))) {
            BCM_TRUNK_ERR((_SBX_D(unit, "Found invaid qid in [0x%x] "
                                  "tid=0x%x\n"),
                           lagi, tid));
            continue;
        }
#endif /* BCM_QE2000_SUPPORT */
 
        rv = map_qid_to_np(unit, lag_ent.qid, &fab_node, &fab_port, NUM_COS(unit));
        fab_node += SBX_QE_BASE_MODID;
        
        if (BCM_FAILURE(rv)) {
            BCM_TRUNK_ERR((_SBX_D(unit, "Failed to map qid 0x%x "
                                  "to a node/port: %s\n"),
                           lag_ent.qid, bcm_errmsg(rv)));
            return rv;
        }

        /* Convert qid node port to switch port for local settings */
        BCM_GPORT_MODPORT_SET(fab_gport, fab_node, fab_port);
        rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, fab_gport,
                                                    &switch_gport);
        BCM_TRUNK_VERB((_SBX_D(unit, "Adding port_num=%d mod/port=%d/%d "
                               "from lag[0x%x]\n"),
                        ti->num_ports,
                        BCM_GPORT_MODPORT_MODID_GET(switch_gport),
                        BCM_GPORT_MODPORT_PORT_GET(switch_gport),
                        lagi));

        if (BCM_FAILURE(rv)) {
            BCM_TRUNK_ERR((_SBX_D(unit, "Failed to convert gport 0x%x "
                                  "to a switch gport: %s\n"),
                           fab_gport, bcm_errmsg(rv)));
            return rv;
        }

        if (ti->num_ports < BCM_TRUNK_MAX_PORTCNT) {
            ti->tp[ti->num_ports] = BCM_GPORT_MODPORT_PORT_GET(switch_gport);
            ti->tm[ti->num_ports] = BCM_GPORT_MODPORT_MODID_GET(switch_gport);
            ti->num_ports++;
        } else {
            BCM_TRUNK_ERR((_SBX_D(unit, "Invalid port count found"
                                  "tid 0x%x, lag[0x%x]\n"),
                           tid, lagi));
            return BCM_E_CONFIG;
        }
    }

    rv = _bcm_fe2000_g2p3_trunk_psc_recover(unit);
    if (BCM_FAILURE(rv)) {
        BCM_TRUNK_ERR((_SBX_D(unit, "Failed to recover PSC: %s\n"),
                       bcm_errmsg(rv)));
        return rv;
    }

    /* recovery complete; initialization complete */
    tc->init_state = ts_init;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_trunk_init
 * Purpose:
 *      Initializes the trunk module. The hardware and the software data
 *      structures are both set to their initial states with no trunks
 *      configured.
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_MEMORY    - Out of memory
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

int
bcm_fe2000_trunk_init(int unit)
{
    trunk_cntl_t               *tc;
    trunk_private_t            *ti;
    int                         alloc_size;
    bcm_trunk_t                 tid;
    uint32_t                    eIndex;
    uint32_t                    eteAddr;
    int                         index;
    int                         result = BCM_E_NONE;

    BCM_TRUNK_VERB(("%s(%d) - Enter\n", FUNCTION_NAME(), unit));

    if (!BCM_UNIT_VALID(unit) || (unit >= BCM_MAX_NUM_UNITS)) {
        return BCM_E_UNIT;
    }

    tc = &TRUNK_CNTL(unit);

    if (SOC_WARM_BOOT(unit) && (tc->init_state != ts_none)) {
        if (tc->init_state == ts_recovering) { 
            return _bcm_fe2000_trunk_recover(unit);
        }
        return BCM_E_CONFIG;
    }

    if (tc->t_info != NULL && tc->lock != NULL) {
        result = bcm_fe2000_trunk_detach(unit);
        if (result != BCM_E_NONE) {
            return result;
        }
        tc->lock = NULL;
        tc->t_info = NULL;
    }
 
    if (NULL == tc->lock) {
        if (NULL == (tc->lock = sal_mutex_create("fe2000_trunk_lock"))) {
            return BCM_E_MEMORY;
        }
    }
    tc->ngroups = _ngroups;
    tc->nports  = _nports;
    tc->psc     = _BCM_TRUNK_PSC_UNDEFINED;

    if (tc->t_info != NULL) {
        sal_free(tc->t_info);
        tc->t_info = NULL;
    }

    /* clear memory for the callback array */
    sal_memset(tc->callback_func, 0, sizeof(tc->callback_func));
    sal_memset(tc->callback_user_data, 0, sizeof(tc->callback_user_data));

    /* alloc memory and clear */
    if (tc->ngroups > 0) {
        alloc_size = tc->ngroups * sizeof(trunk_private_t);
        ti = sal_alloc(alloc_size, "Trunk-private");
        if (NULL == ti) {
            return BCM_E_MEMORY;
        }
        tc->t_info = ti;
        sal_memset(ti, 0, alloc_size);
    } else {
        return BCM_E_INTERNAL;
    }

    /* Init internal structures */
    ti = tc->t_info;
    for (tid = 0; tid < tc->ngroups; tid++) {
        /* disable all trunk group and clear all trunk bitmap */
        ti->trunk_id        = BCM_TRUNK_INVALID;
        ti->in_use          = FALSE;
        ti++;
    }

    tc->invalid_oi = soc_sbx_g2p3_oi2e_table_size_get(1) + 1;

    if (SOC_WARM_BOOT(unit)) {
        if (tc->init_state == ts_none) { 
            /* First stage of recovery, nothing more can be done at this time.
             * Must wait for application to re-establish the mod/port mappings
             * to recover the trunk port state from lag qids.  
             */
            tc->init_state = ts_recovering;
            return BCM_E_NONE;
        }
        return BCM_E_CONFIG;
    }
    
    /* now go twiddle the chip */
    for (tid = 0; tid < tc->ngroups; tid++) {
        /*
         * One outHdrIdx is consumed per trunk. Should be linked to a
         * unique L2 ETE entry. This allows the Trunk port attributes
         * (tagging options mostly) to be managed.
         */
        
        eteAddr = SOC_SBX_TRUNK_ETE(unit, tid);

#ifdef BCM_FE2000_P3_SUPPORT
        if (SOC_SBX_CONTROL(unit)->ucodetype ==  SOC_SBX_UCODE_TYPE_G2P3) {
            soc_sbx_g2p3_etel2_t sbxEteL2;
            soc_sbx_g2p3_etel2_t_init(&sbxEteL2);
            sbxEteL2.mtu = SBX_DEFAULT_MTU_SIZE;
            result = soc_sbx_g2p3_etel2_set(unit, eteAddr, &sbxEteL2);
        }
#endif /* BCM_FE2000_P3_SUPPORT */

        if (BCM_E_NONE != result) break;

        eIndex  = SOC_SBX_TRUNK_OHI(tid);
        result  = _bcm_fe2000_trunk_outHdr_set(unit, eIndex, eteAddr);
        if (BCM_E_NONE != result) break;

        for (index = 0; index < TRUNK_SBX_FIXED_PORTCNT; index++) {
            eIndex = TRUNK_INDEX_SET(tid, index);
            result = _bcm_fe2000_trunk_ingLag_set(unit, eIndex, 0,
                                                  tc->invalid_oi);
            if (BCM_E_NONE != result) break;
        }
        if (BCM_E_NONE != result) break;

        eIndex = SOC_SBX_TRUNK_FTE(unit, tid);
        result = _bcm_fe2000_trunk_fte_inValidate(unit, eIndex);
        if (BCM_E_NONE != result) break;
    }

    if (BCM_E_NONE == result) {
        tc->init_state = ts_init;
    }

    BCM_TRUNK_VERB(("bcm_trunk_init: unit=%d result=%d(%s)\n",
               unit, result, bcm_errmsg(result)));

    return result;
}

/* Returns the set difference of info_a and info_b.  
 * Specifically, <mod,port> pairs found in info_a not present info_b 
 */
void
bcm_fe2k_trunk_add_info_cmp(bcm_trunk_add_info_t *info_a,
                            bcm_trunk_add_info_t *info_b,
                            int                  *num_ports,
                            bcm_module_t          mods[BCM_TRUNK_MAX_PORTCNT],
                            bcm_port_t            ports[BCM_TRUNK_MAX_PORTCNT])
{
    int idxa, idxb, cnt;
    cnt = 0;

    /* Compare info_a to info_b, build a set of mod/ports that are present in
     * info_a, but not in info_b.
     */
    for (idxa = 0; idxa < info_a->num_ports; idxa++) {
            
        /* scan the last known membership for this port */
        for (idxb = 0; idxb < info_b->num_ports; idxb++) {
            if (info_b->tm[idxb] == info_a->tm[idxa] &&
                info_b->tp[idxb] == info_a->tp[idxa]) {
                break;
            }
        }
        
        /* mod,port not found in info_a; add it to the list */
        if (idxb >= info_b->num_ports) {
            mods[cnt]  = info_a->tm[idxa];
            ports[cnt] = info_a->tp[idxa];
            cnt++;
        }
    }

    *num_ports = cnt;
}


/*
 * Function:
 *      bcm_trunk_change_register
 * Purpose:
 *      Registers a callback routine, to be called whenever bcm_trunk_set
 *      is called.
 * Parameters:
 *      unit      - Device unit number.
 *      callback  - The callback function to call.
 *      user_data - An opaque cookie to pass to callback function.
 *                  whenever it is called.
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_PARAM - NULL function pointer or bad unit.
 *      BCM_E_FULL  - Cannot register more than _BCM_TRUNK_MAX_CALLBACK callbacks.
 *      BCM_E_EXISTS - Cannot register the same callback twice.
 *      BCM_E_XXXX  - As set by lower layers of software.
 */

int
bcm_fe2000_trunk_change_register(int unit,
                                 bcm_trunk_notify_cb callback,
                                 void *user_data)
{
    int                         rv = BCM_E_FULL;
    trunk_cntl_t               *tc;
    uint32_t                    i;

    BCM_TRUNK_VERB(("%s(%d, %d, %d) - Enter\n", FUNCTION_NAME(), unit, callback, user_data));

    TRUNK_CHECK_INIT(unit, ts_recovering);

    if (!callback) {
        return BCM_E_PARAM;
    }

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    /* find empty entry */
    for (i = 0; i < _BCM_TRUNK_MAX_CALLBACK; i++) {
        if (tc->callback_func[i] == callback) {
	    rv = BCM_E_EXISTS;
            break;
	}
    }

    if (rv != BCM_E_EXISTS) {
        for (i = 0; i < _BCM_TRUNK_MAX_CALLBACK; i++) {
            if (tc->callback_func[i] == NULL) {
                tc->callback_func[i] = callback;
                tc->callback_user_data[i] = user_data;
	        rv = BCM_E_NONE;
                break;
	    }
	}
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("register: unit=%d port=%d rv=%d\n",
                    unit, rv));

    return rv;
}

/*
 * Function:
 *      bcm_trunk_change_unregister
 * Purpose:
 *      Unregisters a callback.
 * Parameters:
 *      unit      - Device unit number.
 *      callback  - The callback function to be unregistered.
 * Returns:
 *      BCM_E_NONE  - Success
 *      BCM_E_PARAM - NULL function pointer or bad unit.
 *      BCM_E_XXXX  - As set by lower layers of software.
 */

int
bcm_fe2000_trunk_change_unregister(int unit, bcm_trunk_notify_cb callback)
{
    int                         rv = BCM_E_NOT_FOUND;
    trunk_cntl_t               *tc;
    uint32_t                    i;

    BCM_TRUNK_VERB(("%s(%d, %d) - Enter\n", FUNCTION_NAME(), unit, callback));

    TRUNK_CHECK_INIT(unit, ts_recovering);

    if (!callback) {
        return BCM_E_PARAM;
    }

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    /* check if exists, and remove */
    for (i = 0; i < _BCM_TRUNK_MAX_CALLBACK; i++) {
        if (tc->callback_func[i] == callback) {
            tc->callback_func[i] = NULL;
            tc->callback_user_data[i] = NULL;
	    rv = BCM_E_NONE;
            break;
	}
    }

    TRUNK_DB_UNLOCK(unit);

    BCM_TRUNK_VERB(("unregister: unit=%d port=%d rv=%d\n",
                    unit, rv));

    return rv;
}
