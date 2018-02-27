/*
 * $Id: policer.c 1.58 Broadcom SDK $
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
 * Module: Policer management
 */

#include <soc/drv.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2k_common/sbFe2000Pm.h>
#include <soc/sbx/fe2k/sbFe2000Util.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#endif

#include <shared/idxres_fl.h>
#include <shared/idxres_afl.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/cosq.h>
#include <bcm/policer.h>

#include <bcm_int/sbx/fe2000/policer.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/error.h>


#define POLICER_OUT(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_POLICER, stuff)
#define POLICER_WARN(stuff)        BCM_DEBUG(BCM_DBG_WARN, stuff)
#define POLICER_ERR(stuff)         BCM_DEBUG(BCM_DBG_ERR, stuff)
#define POLICER_VERB(stuff)        POLICER_OUT(BCM_DBG_VERBOSE, stuff)
#define POLICER_VVERB(stuff)       POLICER_OUT(BCM_DBG_VVERBOSE, stuff)

#define POLICER_ID_INVALID(unit, id) \
  (((id) < 0) || ((id) > _bcm_policer_max_id_get((unit))))


#define POLICER_UNIT_LOCK_nr(unit, rv) \
do { \
    if (sal_mutex_take(_policer[unit].lock, sal_mutex_FOREVER)) { \
        POLICER_ERR(("%s: sal_mutex_take failed for unit %d. \n", \
                    FUNCTION_NAME(), unit)); \
        (rv) = BCM_E_INTERNAL; \
    } \
} while (0)

#define POLICER_UNIT_LOCK(unit) \
do { \
    int pol__rv__ = BCM_E_NONE; \
    POLICER_UNIT_LOCK_nr(unit, pol__rv__);  \
    if (BCM_FAILURE(pol__rv__)) { return pol__rv__; } \
} while (0)

#define POLICER_UNIT_UNLOCK_nr(unit, rv) \
do { \
    if (sal_mutex_give(_policer[unit].lock)) { \
        POLICER_ERR(("%s: sal_mutex_give failed for unit %d. \n", \
                    FUNCTION_NAME(), unit)); \
        (rv) = BCM_E_INTERNAL; \
    } \
} while (0)

#define POLICER_UNIT_UNLOCK(unit) \
do { \
    int pol__rv__ = BCM_E_NONE; \
    POLICER_UNIT_UNLOCK_nr(unit, pol__rv__);  \
    if (BCM_FAILURE(pol__rv__)) { return pol__rv__; } \
} while (0)

#define POLICER_UNIT_CHECK(unit) \
do { \
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) { \
        POLICER_ERR(("%s: Invalid unit \n", FUNCTION_NAME())); \
        return BCM_E_UNIT; \
    } \
} while (0)

#define POLICER_INIT_CHECK(unit) \
do { \
    POLICER_UNIT_CHECK(unit); \
    if (!_policer[unit].lock) { \
        POLICER_ERR(("%s: Policers unitialized on unit:%d \n", \
                     FUNCTION_NAME(), unit)); \
        return BCM_E_INIT; \
    } \
} while (0)

#define POLICER_PM_GET_ERROR_RETURN(rv, pPm) \
do { \
         rv = _bcm_fe2000_policer_pm_get(unit, &pPm); \
         if (rv != BCM_E_NONE) { \
             return rv; \
         } \
} while (0)

#define POLICER_NULL_PARAM_CHECK(_param) \
    if ((_param) == NULL) { return BCM_E_PARAM; }

/* convert from fe2000 rate to bcm rate */
#define POLICER_CONFIG_RATE_UNITS(_r, _b) \
  (((_r) <= 1) ? 0 : ( ((_r) - 1) * (_b) + 1))

/* Convert from bcm rate to fe2000 rate */
#define POLICER_RATE_UNITS(rate, base) (((rate) <= 0) ? \
                                            0 : (1 + ((rate)-1)/(base)))
#define KBITS_TO_BYTES(kbits) ((kbits)*125)
#define BYTES_TO_KBITS(bytes) (((bytes) <= 0) ? 0 : (1 + (((bytes)-1)*8)/1000))

/* Macros to overload info of whether this policer is the base policer or not
   on group_mode member of _bcm_policer_node_t */
#define BASE_POLICER_ID_SIGNATURE 0xBA000000
#define IS_BASE_POLICER(mode) (((mode) & BASE_POLICER_ID_SIGNATURE) \
                               == (BASE_POLICER_ID_SIGNATURE))
#define BASE_POLICER_SET(mode) ((mode) | BASE_POLICER_ID_SIGNATURE)
#define GROUP_MODE_GET(mode) ((mode) & (~BASE_POLICER_ID_SIGNATURE))

#define POLICER_GROUP_MODE_GET(unit, pol_id) \
        ((GROUP_MODE_GET(_policer[(unit)].policers[(pol_id)].group_mode)))


#ifdef BROADCOM_DEBUG
#define POLICER_TIMESTAMP(_t) (_t) = sal_time_usecs();
#else
#define POLICER_TIMESTAMP(_t) 
#endif


typedef struct _bcm_policer_node_s {
    bcm_policer_t               id;         /* policer id */
    bcm_policer_group_mode_t    group_mode; /* group mode policer belongs */
    uint32                      cookie;     /* store info in this node */
    struct _bcm_policer_node_s  *prev;      /* prev in the created list */
    struct _bcm_policer_node_s  *next;      /* next in the created list */
    bcm_policer_config_t        *pol_cfg;   /* policer config */
} _bcm_policer_node_t;

typedef struct _bcm_policer_glob_s {
    sal_mutex_t                 lock;      /* per unit lock */
    int                         pol_count; /* number of created policers */
    _bcm_policer_node_t         *pol_head; /* list of created policers */
    _bcm_policer_node_t         *policers; /* ptr to array of policer nodes */
    shr_idxres_list_handle_t    res_idlist;/* usage list of reserved policer ids */
    shr_aidxres_list_handle_t   idlist;    /* usage list of policers ids */
    bcm_policer_t               mon_use_map[BCM_FE2000_NUM_MONITORS]; /* Monitor 
                                                                         usage map */
    uint32_t                    numExceptionPolicers;
} _bcm_policer_glob_t;

typedef struct _bcm_policer_node_g2p3_cookie_s {
    uint32      base_cntr;
} _bcm_policer_node_g2p3_cookie_t;

/* internal SW state is required for traverse & config get API */
static _bcm_policer_glob_t _policer[BCM_MAX_NUM_UNITS]; /* glob policer data */
static sal_mutex_t         _policer_glob_lock;          /* glob policer lock */


static int _g2p3_policer_group_num_policers[] = {
                        1,  /* bcmPolicerGroupModeSingle */
                        -1, /* bcmPolicerGroupModeTrafficType - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfAll - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfIntPri - Unsupported*/
                        4,  /* bcmPolicerGroupModeTyped */
                        5,  /* bcmPolicerGroupModeTypedAll */
                        -1,  /* bcmPolicerGroupModeTypedIntPri 
                               based on configured cos levels */
                        2,  /* bcmPolicerGroupModeSingleWithControl */
                        -1, /* bcmPolicerGroupModeTrafficTypeWithControl - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfAllWithControl - Unsupported */
                        -1, /* bcmPolicerGroupModeDlfIntPriWithControl - Unsupported*/
                        5,  /* bcmPolicerGroupModeTypedWithControl */
                        6,  /* bcmPolicerGroupModeTypedAllWithControl */
                        -1  /* bcmPolicerGroupModeTypedIntPriWithControl 
                               based on configured cos levels */
                       };



/*
 * Function
 *      _bcm_fe2000_policer_pm_get
 * Purpose
 *      Gets the PMU state handle
 * Parameters
 *      (in) unit   = unit number
 *      (out) ppPm  = PMU state handle holder
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not 
 */
int
_bcm_fe2000_policer_pm_get(int unit, sbFe2000Pm_t **ppPm)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_state_t *g2p3_st;
#endif
    
    POLICER_UNIT_CHECK(unit);

    if (!ppPm) {
        return BCM_E_PARAM;
    }
    *ppPm = NULL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        g2p3_st = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
        if (!g2p3_st) {
            return BCM_E_INTERNAL;
        }
        *ppPm = g2p3_st->pm;
        return BCM_E_NONE;
#endif  /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}



/*
 * Function
 *      _bcm_policer_base_rate
 * Purpose
 *      Return the configured base rate for policers
 * Parameters
 *      (in) unit   = unit number
 *      (in) polId  = policerId used to find base rate
 *      (out) rate  = base rate in kbps
 *      (out) enum  = base rate in GU2 enum
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_policer_base_rate(int unit, int polId, uint32 *rate_kpbs,
                       sbFe2000PmPolicerRate_t *rate_enum)
{
    int                     bcm_status = BCM_E_NONE;
    sbFe2000Pm_t            *pPm;
    sbStatus_t              sb_status;
    sbFe2000PmGroupConfig_t config;

    POLICER_PM_GET_ERROR_RETURN(bcm_status, pPm);

    if ((pPm == NULL) || (rate_kpbs == NULL) || (rate_enum == NULL)) {
        return BCM_E_PARAM;
    }

    if (polId < _policer[unit].numExceptionPolicers) {
        sb_status = sbFe2000PmPolicerGroupRead(pPm,
                                               BCM_FE2000_EXC_POLICER_GROUPID,
                                               &config);
    } else {
        sb_status = sbFe2000PmPolicerGroupRead(pPm, BCM_FE2000_POLICER_GROUPID,
                                               &config);
    }

    if (sb_status == SB_OK) {
        *rate_enum = config.uRate;

        switch (config.uRate) {
        case SB_FE_2000_PM_PRATE_1KBPS:
            *rate_kpbs = 1;
            break;
        case SB_FE_2000_PM_PRATE_10KBPS:
            *rate_kpbs = 10;
            break;
        case SB_FE_2000_PM_PRATE_100KBPS:
            *rate_kpbs = 100;
            break;
        case SB_FE_2000_PM_PRATE_250KBPS:
            *rate_kpbs = 250;
            break;
        case SB_FE_2000_PM_PRATE_500KBPS:
            *rate_kpbs = 500;
            break;
        case SB_FE_2000_PM_PRATE_1MBPS:
            *rate_kpbs = 1000;
            break;
        case SB_FE_2000_PM_PRATE_10MBPS:
            *rate_kpbs = 1000*10;
            break;
        case SB_FE_2000_PM_PRATE_100MBPS:
            *rate_kpbs = 1000*100;
            break;
        case SB_FE_2000_PM_PRATE_1GBPS:
            *rate_kpbs = 1000*1000;
            break;
        case SB_FE_2000_PM_PRATE_10GBPS:
            *rate_kpbs = 1000*1000*10;
            break;
        default:
            bcm_status = BCM_E_INTERNAL;
            break;
        }
    } else {
        bcm_status = BCM_E_INTERNAL;
    }

    return bcm_status;
}


/*
 * Function
 *      _bcm_fe2000_policer_unit_lock
 * Purpose
 *      Lock the top level policer state for this unit 
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not  
 */
int
_bcm_fe2000_policer_unit_lock(unit)
{
    POLICER_UNIT_CHECK(unit);
    POLICER_UNIT_LOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_fe2000_policer_unit_unlock
 * Purpose
 *      Unlock the top level policer state for this unit 
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not  
 */
int
_bcm_fe2000_policer_unit_unlock(unit)
{
    POLICER_UNIT_CHECK(unit);
    POLICER_UNIT_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_fe2000_policer_group_num_get
 * Purpose
 *      Get the number of policers in the group mode
 * Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode
 *      (out) npolicers = number of policers
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not 
 */
int
_bcm_fe2000_policer_group_num_get(int unit, bcm_policer_group_mode_t mode, 
                                  uint32_t *npolicers)
{
    int rv = BCM_E_NONE;
    int num;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        num = _g2p3_policer_group_num_policers[mode];

        if (mode == bcmPolicerGroupModeTypedIntPri) {
            /* based on current cos configuration */
            num = (NUM_COS(unit) + 4);
        } else if (mode == bcmPolicerGroupModeTypedIntPriWithControl) {
            /* based on current cos configuration */
            num = (NUM_COS(unit) + 5);
        }
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }

    if (num < 0) {
        /* Unsupported mode specified */
        rv = BCM_E_PARAM;
    } else {
        *npolicers = (uint32_t) num;
    }

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_policer_get_hw
 * Purpose
 *      Retrieve the policer configuration from hardware
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id
 *      (out) pcfg      = policer configuration
 * Returns
 *   BCM_E_*
 */
int
_bcm_fe2000_policer_get_hw(int unit, int policer_id, 
                           bcm_policer_config_t *pcfg,
                           uint32_t *profile_id)
{
    sbStatus_t                  sb_rv;
    sbFe2000Pm_t                *pPm;
    sbZfFe2000PmPolicerConfig_t pm_config;
    int                         rv = BCM_E_NONE;
    uint32                      base_rate;

    rv = _bcm_fe2000_policer_pm_get(unit, &pPm);
    if (BCM_FAILURE(rv)) {
        POLICER_ERR(("Failed to get pm: %s\n", bcm_errmsg(rv)));
        return rv;
    }

    sbZfFe2000PmPolicerConfig_InitInstance(&pm_config);

    rv = _bcm_policer_base_rate(unit, policer_id, 
                                &base_rate, &pm_config.uRate);
    if (BCM_FAILURE(rv)) {
        POLICER_ERR(("Failed to get base rate for policer 0x%04x: %s\n",
                     policer_id, bcm_errmsg(rv)));
        return rv;
    }

    sb_rv = sbFe2000PmPolicerReadExt(pPm, policer_id, &pm_config, profile_id);
    if (sb_rv != SB_OK) {
        POLICER_ERR(("Failed to read Policer: 0x%08x\n", sb_rv));
        return translate_sbx_result(sb_rv);
    }

    /* common conversions first */
    bcm_policer_config_t_init(pcfg);
    pcfg->ckbits_sec   = POLICER_CONFIG_RATE_UNITS(pm_config.uCIR, base_rate);
    pcfg->ckbits_burst = BYTES_TO_KBITS(pm_config.uCBS);
    pcfg->pkbits_sec   = POLICER_CONFIG_RATE_UNITS(pm_config.uEIR, base_rate);

    switch (pm_config.uRfcMode) 
    {
    case SB_FE_2000_PM_PMODE_RFC_2697:
        pcfg->mode = bcmPolicerModeSrTcm;
        break;
    case SB_FE_2000_PM_PMODE_RFC_2698:
        if (pm_config.bCBSNoDecrement && pm_config.bEBSNoDecrement) {
            pcfg->mode = bcmPolicerModePassThrough;
        } else {
            pcfg->mode = bcmPolicerModeTrTcm;
        }
        break;
    case SB_FE_2000_PM_PMODE_RFC_4115:
        if (pm_config.bCoupling) {
            pcfg->mode = bcmPolicerModeCoupledTrTcmDs;
        } else {
            pcfg->mode = bcmPolicerModeTrTcmDs;
        }
        break;
    default:
        POLICER_ERR(("Invalid policer mode: %d \n", pm_config.uRfcMode));
        return BCM_E_CONFIG;
    }

    pcfg->flags |= pm_config.bBlindMode ? BCM_POLICER_COLOR_BLIND : 0;
    pcfg->flags |= pm_config.bDropOnRed ? BCM_POLICER_DROP_RED : 0;

    
    if (BCM_DEBUG_CHECK(BCM_DBG_POLICER | BCM_DBG_VERBOSE)) {
        POLICER_OUT(0, ("Read PmConfig 0x%04x:\n", policer_id));
        POLICER_OUT(0, ("rfcMode=%d rate=%d cbs=%d cir=%d ebs=%d eir=%d\n", 
                        pm_config.uRfcMode, pm_config.uRate, pm_config.uCBS,
                        pm_config.uEBS, pm_config.uEIR));
        POLICER_OUT(0, ("blind=%d dor=%d coupling=%d cbsNoDecr=%d ebsNoDecr=%d\n",
                        pm_config.bBlindMode, pm_config.bDropOnRed,
                        pm_config.bCoupling, pm_config.bCBSNoDecrement,
                        pm_config.bEBSNoDecrement));
        POLICER_OUT(0, ("Converts to bcm_policer_confit_t:\n"));
        POLICER_OUT(0, ("ckbits_sec=%d ckbits_burst=%d pkbits_sec=%d\n",
                        pcfg->ckbits_sec, pcfg->ckbits_burst, 
                        pcfg->pkbits_sec));
        POLICER_OUT(0, (""));
    }

    return BCM_E_NONE;
}


/*
 * Function
 *      _bcm_fe2000_g2p3_policer_lp_decode
 * Purpose
 *      decode policer information from the given logical port
 * Parameters
 *      (in) unit       = unit number
 *      (in) lp         = g2p3 logical port
 *      (out) group_mode= configured policer group mode
 * Returns
 *   BCM_E_*
 */
int
_bcm_fe2000_g2p3_policer_lp_decode(int unit, soc_sbx_g2p3_lp_t *lp, 
                                   bcm_policer_group_mode_t *group_mode)
{
    int rv = BCM_E_NONE;

    if (lp->typedpolice) {
        *group_mode = bcmPolicerGroupModeTyped;
    } else if (lp->mef) {
        if (lp->mefcos) {
            if (lp->xtpolreplace) {
                *group_mode = bcmPolicerGroupModeTypedIntPriWithControl;
            } else {
                *group_mode = bcmPolicerGroupModeTypedIntPri;
            }
        } else {
            if (lp->xtpolreplace) {
                *group_mode = bcmPolicerGroupModeTypedAllWithControl;
	    } else {
                *group_mode = bcmPolicerGroupModeTypedAll;
	    }
        }
    } else {
        if (lp->xtpolreplace) {
            *group_mode = bcmPolicerGroupModeSingleWithControl;
	} else {
            *group_mode = bcmPolicerGroupModeSingle;
	}
    } 

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_g2p3_policer_lp_program
 * Purpose
 *      Sets the lp struct with the relevant policer info.
 *      The counter information is also programmed.
 * Parameters
 *      (in) unit       = unit number
 *      (in) pol_id     = policer id
 *      (out) lp        = logical port struct which is programmed
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not 
 * Note:
 *      This does not program the HW
 */
#ifdef BCM_FE2000_P3_SUPPORT
int
_bcm_fe2000_g2p3_policer_lp_program(int unit, bcm_policer_t pol_id,
                                    soc_sbx_g2p3_lp_t *lp)
{
    int                             rv = BCM_E_NONE;
    bcm_policer_group_mode_t        mode;
    uint32_t                        typedpolice=0, mef=0, mefcos=0, xtpolreplace=0;
    uint32                          typedcount=0, base_cntr=0;
    _bcm_policer_node_g2p3_cookie_t *cookie;

    if (!lp) {
        return BCM_E_PARAM;
    }

    if (pol_id) {
        /* non-zero policer id. check group mode */
        rv = _bcm_fe2000_policer_group_mode_get(unit, pol_id, &mode);

        if (rv == BCM_E_NONE) {
            switch (mode) {
            case bcmPolicerGroupModeSingleWithControl:
                xtpolreplace = 1;
		break;
            case bcmPolicerGroupModeTypedWithControl:
                typedpolice = 1;
                xtpolreplace = 1;
		break;
            case bcmPolicerGroupModeTypedIntPriWithControl:
                mefcos = 1; /* intentional fall thru */
            case bcmPolicerGroupModeTypedAllWithControl:
                mef = 1;
                xtpolreplace = 1;
                break;
            case bcmPolicerGroupModeSingle:
                break;
            case bcmPolicerGroupModeTyped:
                typedpolice = 1;
                break;
            case bcmPolicerGroupModeTypedIntPri:
                mefcos = 1; /* intentional fall thru */
            case bcmPolicerGroupModeTypedAll:
                mef = 1;
                break;
            default:
                rv = BCM_E_CONFIG;
            }
        }
    }

    POLICER_UNIT_LOCK(unit);
    cookie = (_bcm_policer_node_g2p3_cookie_t *)
              _policer[unit].policers[pol_id].cookie;
    if (cookie) {
        base_cntr = cookie->base_cntr;
        typedcount = typedpolice;
    }
    POLICER_UNIT_UNLOCK(unit);

    if (rv == BCM_E_NONE) {
        lp->policer = pol_id;
        lp->typedpolice = typedpolice;
        lp->mef = mef;
        lp->mefcos = mefcos;
        lp->counter = base_cntr;
        lp->typedcount = typedcount;
        lp->xtpolreplace = xtpolreplace;

        /* lp->updaterdp controls if a policer dp is picked up */
        lp->updaterdp = !!pol_id && mef;
        lp->updatefdp = lp->updaterdp;
    }

    return rv;
}
#endif

/*
 * Function
 *      _bcm_policer_num_policers_get
 * Purpose
 *      Returns the max policers possible
 * Parameters
 *      (in) unit   = unit number
 * Returns
 *      int - max number of policers
 */
int
_bcm_policer_num_policers_get(int unit) {
    sbFe2000Pm_t    *pPm;
    int             rv = BCM_E_NONE;

    rv = _bcm_fe2000_policer_pm_get(unit, &pPm);
    if (rv != BCM_E_NONE) {
        return 0;
    }

    return pPm->PolicerConfig.uNumberOfPolicersPerGroup[BCM_FE2000_POLICER_GROUPID];
}

/*
 * Function
 *      _bcm_fe2000_g2p3_policer_stat_mem_get
 * Purpose
 *      Returns the offset at which the counter for a particular stat
 *      type exists. Also indicates if this a pkt or byte counter.
 * Parameters
 *      (in) unit       = unit number
 *      (in) grp_mode   = group mode to which the policer belongs
 *      (in) stat       = stat type which to look at
 *      (in) cos        = cos level. Only applicable for TypedIntPri mode
 *      (out)ctr_offset = offset at which the counter sits
 *      (out)pkt        = whether pkt or not
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_fe2000_g2p3_policer_stat_mem_get(int unit, 
                                      bcm_policer_group_mode_t grp_mode, 
                                      bcm_policer_stat_t stat,
                                      int cos,
                                      int *ctr_offset,
                                      int *pkt)
{
    int rv = BCM_E_NONE;
    
    
    switch (grp_mode) {
    /* There is no support for counting control, policers under WithControl are treated at par */
    case bcmPolicerGroupModeSingle:
    case bcmPolicerGroupModeSingleWithControl:
        switch (stat) {
        /* intentional fall thrus for bcmPolicerStat*Packets */
        case bcmPolicerStatPackets:     *pkt = 1;
        case bcmPolicerStatBytes:       *ctr_offset = 0; break;
        case bcmPolicerStatDropPackets: *pkt = 1;
        case bcmPolicerStatDropBytes:   *ctr_offset = 1; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    case bcmPolicerGroupModeTyped:
    case bcmPolicerGroupModeTypedWithControl:
        switch (stat) {
        case bcmPolicerStatUnknownUnicastPackets:       *pkt = 1;
        case bcmPolicerStatUnknownUnicastBytes:         *ctr_offset = 0; break;
        case bcmPolicerStatDropUnknownUnicastPackets:   *pkt = 1;
        case bcmPolicerStatDropUnknownUnicastBytes:     *ctr_offset = 1; break;
        case bcmPolicerStatUnicastPackets:              *pkt = 1;
        case bcmPolicerStatUnicastBytes:                *ctr_offset = 2; break;
        case bcmPolicerStatDropUnicastPackets:          *pkt = 1;
        case bcmPolicerStatDropUnicastBytes:            *ctr_offset = 3; break;
        case bcmPolicerStatMulticastPackets:            *pkt = 1;
        case bcmPolicerStatMulticastBytes:              *ctr_offset = 4; break;
        case bcmPolicerStatDropMulticastPackets:        *pkt = 1;
        case bcmPolicerStatDropMulticastBytes:          *ctr_offset = 5; break;
        case bcmPolicerStatBroadcastPackets:            *pkt = 1;
        case bcmPolicerStatBroadcastBytes:              *ctr_offset = 6; break;
        case bcmPolicerStatDropBroadcastPackets:        *pkt = 1;
        case bcmPolicerStatDropBroadcastBytes:          *ctr_offset = 7; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    case bcmPolicerGroupModeTypedAll:
    case bcmPolicerGroupModeTypedAllWithControl:
        switch (stat) {
        case bcmPolicerStatDropUnknownUnicastPackets:   *pkt = 1;
        case bcmPolicerStatDropUnknownUnicastBytes:     *ctr_offset = 0; break;
        case bcmPolicerStatDropUnicastPackets:          *pkt = 1;
        case bcmPolicerStatDropUnicastBytes:            *ctr_offset = 1; break;
        case bcmPolicerStatDropMulticastPackets:        *pkt = 1;
        case bcmPolicerStatDropMulticastBytes:          *ctr_offset = 2; break;
        case bcmPolicerStatDropBroadcastPackets:        *pkt = 1;
        case bcmPolicerStatDropBroadcastBytes:          *ctr_offset = 3; break;
        case bcmPolicerStatGreenPackets:                *pkt = 1;
        case bcmPolicerStatGreenBytes:                  *ctr_offset = 4; break;
        case bcmPolicerStatYellowPackets:               *pkt = 1;
        case bcmPolicerStatYellowBytes:                 *ctr_offset = 5; break;
        case bcmPolicerStatRedPackets:                  *pkt = 1;
        case bcmPolicerStatRedBytes:                    *ctr_offset = 6; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    case bcmPolicerGroupModeTypedIntPri:
    case bcmPolicerGroupModeTypedIntPriWithControl:
        switch (stat) {
        case bcmPolicerStatDropUnknownUnicastPackets:   *pkt = 1;
        case bcmPolicerStatDropUnknownUnicastBytes:     *ctr_offset = 0; break;
        case bcmPolicerStatDropUnicastPackets:          *pkt = 1;
        case bcmPolicerStatDropUnicastBytes:            *ctr_offset = 1; break;
        case bcmPolicerStatDropMulticastPackets:        *pkt = 1;
        case bcmPolicerStatDropMulticastBytes:          *ctr_offset = 2; break;
        case bcmPolicerStatDropBroadcastPackets:        *pkt = 1;
        case bcmPolicerStatDropBroadcastBytes:          *ctr_offset = 3; break;
        case bcmPolicerStatGreenPackets:        *pkt = 1;
        case bcmPolicerStatGreenBytes:          *ctr_offset = 4 + cos*3; break;
        case bcmPolicerStatYellowPackets:       *pkt = 1;
        case bcmPolicerStatYellowBytes:         *ctr_offset = 5 + cos*3; break;
        case bcmPolicerStatRedPackets:          *pkt = 1;
        case bcmPolicerStatRedBytes:            *ctr_offset = 6 + cos*3; break;
        default:
            rv = BCM_E_PARAM;
        }
        break;
    default:
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_g2p3_policer_stat_get
 * Purpose
 *      read the ucode counter for the stat type of  policer/cos
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id
 *      (in) stat       = stat type which to look at
 *      (in) cos        = cos level. Only applicable for TypedIntPri mode
 *      (in) use_cookie = whether to use counter from cookie
 *      (in) counter    = base counter when use_cookie = 0
 *      (in)clear       = whether to clear the value or not
 *      (out)val        = stat value
 *
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_fe2000_g2p3_policer_stat_get(int unit, bcm_policer_t policer_id, 
                                  bcm_policer_stat_t stat, int cos, 
                                  int use_cookie, uint32 counter,
                                  int clear, uint64 *val)
{
    int                             rv = BCM_E_NONE;
    bcm_policer_group_mode_t        grp_mode;
    _bcm_policer_node_g2p3_cookie_t *cookie;
    int                             ctr_offset, base_ctr = 0, pkts;
    soc_sbx_g2p3_counter_value_t    soc_val;
    
    grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);

    pkts = 0;
    rv = _bcm_fe2000_g2p3_policer_stat_mem_get(unit, grp_mode, stat, cos, 
                                               &ctr_offset, &pkts); 

    if (rv == BCM_E_NONE) {
        if (use_cookie) {
            cookie = (_bcm_policer_node_g2p3_cookie_t *)
                     _policer[unit].policers[policer_id].cookie;
            if (!cookie) {
                rv = BCM_E_INTERNAL;
            } else {
                base_ctr = cookie->base_cntr;
            }
        } else {
            if (counter) {
                rv = BCM_E_INTERNAL;
            }
            base_ctr = counter;
        }
        rv = soc_sbx_g2p3_ingctr_get(unit, clear, 
                                    (base_ctr + ctr_offset), &soc_val);
    }

    if (rv == BCM_E_NONE) {
        if (pkts) {
            COMPILER_64_SET(*val, COMPILER_64_HI(soc_val.packets),
                            COMPILER_64_LO(soc_val.packets));
        } else {
            COMPILER_64_SET(*val, COMPILER_64_HI(soc_val.bytes),
                            COMPILER_64_LO(soc_val.bytes));
        }
    }

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_g2p3_policer_stat_set
 * Purpose
 *      set the ucode counter for the stat type of  policer/cos
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id
 *      (in) stat       = stat type which to look at
 *      (in) cos        = cos level. Only applicable for TypedIntPri mode
 *      (in) use_cookie = whether to use counter from cookie
 *      (in) counter    = base counter when use_cookie = 0
 *      (in)val        = stat value to set to
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 * NOTE
 *      Set to value of only zero is supported.
 */
int
_bcm_fe2000_g2p3_policer_stat_set(int unit, bcm_policer_t policer_id, 
                                  bcm_policer_stat_t stat, int cos,
                                  int use_cookie, uint32 counter,
                                  uint64 val)
{
    int     rv = BCM_E_NONE;
    uint64  temp;
    int     clear=1;

    /* only set to zero is supported */
    if (COMPILER_64_HI(val) || COMPILER_64_LO(val)) {
        return BCM_E_PARAM;
    }

    /* call the get with clear set to 1 */
    rv = _bcm_fe2000_g2p3_policer_stat_get(unit, policer_id, stat, cos,
                                           use_cookie, counter, clear,
                                           &temp);

    return rv;
}

/*
 * Function
 *      _bcm_policer_max_id_get
 * Purpose
 *      Return max valid id
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *      int - max possible policer id
 */
int
_bcm_policer_max_id_get(int unit) {
    return _bcm_policer_num_policers_get(unit) - 1;
}

/*
 * Function
 *      _bcm_fe2000_policer_group_mode_get
 * Purpose
 *      Get the policer group mode for the given policer ID
 * Parameters
 *      (in) unit     = unit number
 *      (in) id       = id of policer mode to get
 *     (out) grp_mode = group mode
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_fe2000_policer_group_mode_get(int unit, bcm_policer_t id,
                                   bcm_policer_group_mode_t *grp_mode)
{
    _bcm_policer_node_t     *cur;
    int                      rv = BCM_E_NONE;

    POLICER_INIT_CHECK(unit);
    if (POLICER_ID_INVALID(unit, id)) {
        POLICER_ERR(("%s: invalid policer id %d. \n", FUNCTION_NAME(), id));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);
    cur = &_policer[unit].policers[id];
    if (cur->pol_cfg == NULL) {
        rv = BCM_E_NOT_FOUND;
    }
    *grp_mode = GROUP_MODE_GET(cur->group_mode);
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/* Function
 *      _bcm_policer_id_check
 * Purpose
 *      To check if the specified policer id is valid
 * Parameters
 *      (in) unit   = unit number
 *      (in) id     = policer id to check
 * Returns
 *      bcm_error_t = BCM_E_NONE if its valid
 *                    BCM_E_RESOURCE if its invalid or already in use
 */
int
_bcm_policer_id_check(int unit, bcm_policer_t id)
{
    int rv = BCM_E_RESOURCE;

    if (POLICER_ID_INVALID(unit, id)) {
        return rv;
    }

    if (id <= BCM_FE2000_RESERVED_POLICERS) {
        /* reserved policer id space */
        rv = shr_idxres_list_elem_state(_policer[unit].res_idlist, id);
    } else {
        rv = shr_aidxres_list_elem_state(_policer[unit].idlist, id);
    }

    return ((rv == BCM_E_NOT_FOUND) ? BCM_E_NONE: BCM_E_RESOURCE);
}

/* Function
 *      _bcm_policer_id_assign
 * Purpose
 *      Assign a policer id
 * Parameters
 *      (in) unit   = unit number
 *      (out) id    = policer id
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_policer_id_assign(int unit, bcm_policer_t *id)
{
    if (!id) {
        return BCM_E_PARAM;
    }
    /* always assign from non-reserved id space */
    return shr_aidxres_list_alloc(_policer[unit].idlist, (uint32 *) id);
}

/* Function
 *      _bcm_policer_id_free
 * Purpose
 *      Return a policer id to available pool
 * Parameters
 *      (in) unit   = unit number
 *      (out) id    = policer id
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_policer_id_free(int unit, bcm_policer_t id)
{
    if (id <= BCM_FE2000_RESERVED_POLICERS) {
        return shr_idxres_list_free(_policer[unit].res_idlist, id);
    } else {
        return shr_aidxres_list_free(_policer[unit].idlist, id);
    }
}

/* Function
 *      _bcm_policer_id_reserve
 * Purpose
 *      Mark the specified id as used
 * Parameters
 *      (in) unit   = unit number
 *      (out) id    = policer id
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 */
int
_bcm_policer_id_reserve(int unit, bcm_policer_t id)
{
    if (id <= BCM_FE2000_RESERVED_POLICERS) {
        return shr_idxres_list_reserve(_policer[unit].res_idlist, id, id);
    }

    return shr_aidxres_list_reserve(_policer[unit].idlist, id, id);
}


/*  Function
 *      _bcm_policer_state_alloc
 *  Purpose
 *      Allocate internal state storage
 *  Parameters
 *      (in) unit     = unit number
 *      (in) id       = policer id of policer
 *      (in) grp_mode = policer group mode of policer
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
_bcm_policer_state_alloc(int unit, bcm_policer_t id,
                         bcm_policer_group_mode_t grp_mode, int base_policer)
{
    int                     status = BCM_E_NONE;
    _bcm_policer_node_t     *cur;

    cur = &_policer[unit].policers[id];

    if (cur->pol_cfg) {
        POLICER_ERR(("%s: Policer with id (%d) already exists.\n",
                     FUNCTION_NAME(), id));
        return BCM_E_PARAM;
    }
    /* insert at top of list */
    cur->prev = NULL;
    cur->next = _policer[unit].pol_head;
    cur->id = id;
    cur->group_mode = (base_policer ? BASE_POLICER_SET(grp_mode): grp_mode);
    cur->pol_cfg = (bcm_policer_config_t *)
                    sal_alloc(sizeof(bcm_policer_config_t), "pol cfg");

    if (cur->pol_cfg) {
        if (_policer[unit].pol_head) {
            _policer[unit].pol_head->prev = cur;
        }
        _policer[unit].pol_head = cur;
        _policer[unit].pol_count++;
    } else {
        POLICER_ERR(("%s: sal_alloc failed \n", FUNCTION_NAME()));
        status = BCM_E_MEMORY;
    }

    return status;
}

/*  Function
 *      _bcm_policer_state_set
 *  Purpose
 *      update the given policer id in internal software state
 *  Parameters
 *      (in) unit   = unit number
 *      (in) id     = policer id to update
 *      (in) pol_cfg= update policer with this config
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
_bcm_policer_state_set(int unit, bcm_policer_t id,
                       bcm_policer_config_t *pol_cfg)
{
    int                     status = BCM_E_NONE;
    _bcm_policer_node_t     *cur;

    if (!pol_cfg) {
        POLICER_ERR(("%s: pol_cfg NULL.\n", FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    cur = &_policer[unit].policers[id];

    if (cur->pol_cfg == NULL) {
        POLICER_ERR(("%s: Policer with id (%d) not found.\n",
                     FUNCTION_NAME(), id));
        return BCM_E_PARAM;
    }

    sal_memcpy(cur->pol_cfg, pol_cfg, sizeof(bcm_policer_config_t));

    return status;

}

/*  Function
 *      _bcm_policer_state_remove
 *  Purpose
 *      Remove the given policer id in internal software state
 *  Parameters
 *      (in) unit   = unit number
 *      (in) id     = policer id to remove
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 * Notes:
 *   assumes - lock is taken, id & unit are valid
 */
int
_bcm_policer_state_remove(int unit, bcm_policer_t id)
{
    int                     status = BCM_E_NONE;
    int                     idx;
    int                     success = 0;
    _bcm_policer_node_t     *cur;
    _bcm_policer_node_t     *prev;
    _bcm_policer_node_t     *next;

    cur = _policer[unit].pol_head;
    for (idx = 0; ((idx < _policer[unit].pol_count) && (cur)); idx++) {
        if (cur->id == id) {
            /* found the node to delete */
            next = cur->next;
            prev = cur->prev;
            if (prev) {
                prev->next = next; /* update the next of prev node */
            } else {
                _policer[unit].pol_head = next;
            }

            if (next) {
                next->prev = prev; /* update the prev of next node */
            }

            if (cur->pol_cfg) {
                sal_free(cur->pol_cfg);
            }
            sal_memset(cur, 0, sizeof(_bcm_policer_node_t));
            _policer[unit].pol_count--;
            _bcm_policer_id_free(unit, id);
            success = 1;
            break;
        }
        cur = cur->next;
    }

    if (!success) {
        POLICER_ERR(("%s: failed for policer id %d. \n", FUNCTION_NAME(), id));
        status = BCM_E_PARAM;
    }

    return status;
}


/*
 * Function
 *      _bcm_policer_hw_create
 * Purpose
 *      Map bcm policer config to SB policer config, commit to hardware
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = location to commit policer config
 *      (in) pol_cfg    = policer config to commit
 * Returns
 *      bcm_error_t = BCM_E_NONE if no error
 *                    BCM_E_* appropriately if not
 * Note:
 *      Assumes unit, policer_id and policer config are valid
 */
int
_bcm_policer_hw_create(int unit, uint32_t policer_id,
                       bcm_policer_config_t *pol_cfg)
{
    sbZfFe2000PmPolicerConfig_t config;
    sbFe2000Pm_t                *pPm;
    sbStatus_t                  sbRv;
    int                         rv;
    uint32                      base_rate = 0;
    uint32                      id_out;

    POLICER_PM_GET_ERROR_RETURN(rv, pPm);

    sbZfFe2000PmPolicerConfig_InitInstance(&config);

    if (pol_cfg->flags & BCM_POLICER_REPLACE) {
        POLICER_ERR(("Un-supported flags BCM_POLICER_REPLACE\n"));
        return BCM_E_PARAM;
    }

    rv = _bcm_policer_base_rate(unit, policer_id, 
                                &base_rate, &config.uRate);

    if (BCM_FAILURE(rv) || (base_rate == 0)) {
        POLICER_ERR(("%s: Invalid policer base rate", FUNCTION_NAME()));
        return BCM_E_INIT;
    }

    /* for PassThrough mode there is no relation between burst sizes & IRs */
    if (pol_cfg->mode != bcmPolicerModePassThrough) {
        /* Check if burst sizes are big enough. *8 is because timer_tick_period 
           is programmed based on policer_base_rate value in bytes */
        if ((pol_cfg->ckbits_burst * 1000) < 
            ((pol_cfg->ckbits_sec * 8)/base_rate)) {
            POLICER_WARN(("%s: ckbits_burst (%d) less than recommended value "
                "(%d) for specified CIR (%d kbps). May result in incorrect "
                "policing \n", FUNCTION_NAME(), (pol_cfg->ckbits_burst * 1000),
                ((pol_cfg->ckbits_sec * 8)/base_rate), pol_cfg->ckbits_sec));
        }
        if ((pol_cfg->pkbits_burst * 1000) < 
            ((pol_cfg->pkbits_sec * 8)/base_rate)) {
            POLICER_WARN(("%s: pkbits_burst (%d) less than recommended value "
                "(%d) for specified EIR (%d kbps). May result in incorrect "
                "policing \n", FUNCTION_NAME(), (pol_cfg->pkbits_burst * 1000),
                ((pol_cfg->pkbits_sec * 8)/base_rate), pol_cfg->pkbits_sec));
        }
    }

    switch (pol_cfg->mode) {
    case bcmPolicerModeSrTcm: /* SRTCM --> RFC2697 */
        config.uCIR = POLICER_RATE_UNITS(pol_cfg->ckbits_sec, base_rate);
        config.uCBS = KBITS_TO_BYTES(pol_cfg->ckbits_burst);
        config.uEIR = 0;
        config.uEBS = KBITS_TO_BYTES(pol_cfg->pkbits_burst);
        config.bCoupling = 1;
        config.uRfcMode = SB_FE_2000_PM_PMODE_RFC_2697;
        break;
    case bcmPolicerModeTrTcm: /* TRTCM --> RFC2698 */
        config.uCIR = POLICER_RATE_UNITS(pol_cfg->ckbits_sec, base_rate);
        config.uCBS = KBITS_TO_BYTES(pol_cfg->ckbits_burst);
        config.uEIR = POLICER_RATE_UNITS(pol_cfg->pkbits_sec, base_rate);
        config.uEBS = KBITS_TO_BYTES(pol_cfg->pkbits_burst);
        config.bCoupling = 0;
        config.uRfcMode = SB_FE_2000_PM_PMODE_RFC_2698;
        break;
    case bcmPolicerModeCoupledTrTcmDs: /* MEF 10.11*/
        config.uCIR = POLICER_RATE_UNITS(pol_cfg->ckbits_sec, base_rate);
        config.uCBS = KBITS_TO_BYTES(pol_cfg->ckbits_burst);
        config.uEIR = POLICER_RATE_UNITS(pol_cfg->pkbits_sec, base_rate);
        config.uEBS = KBITS_TO_BYTES(pol_cfg->pkbits_burst);
        config.bCoupling = 1;
        config.uRfcMode = SB_FE_2000_PM_PMODE_RFC_4115;
        break;
    case bcmPolicerModeTrTcmDs: /* Diff Serv TRTCM --> 4115 */
        config.uCIR = POLICER_RATE_UNITS(pol_cfg->ckbits_sec, base_rate);
        config.uCBS = KBITS_TO_BYTES(pol_cfg->ckbits_burst);
        config.uEIR = POLICER_RATE_UNITS(pol_cfg->pkbits_sec, base_rate);
        config.uEBS = KBITS_TO_BYTES(pol_cfg->pkbits_burst);
        config.bCoupling = 0;
        config.uRfcMode = SB_FE_2000_PM_PMODE_RFC_4115;
        break;
    case bcmPolicerModePassThrough:
        config.uCIR = POLICER_RATE_UNITS(pol_cfg->ckbits_sec, base_rate);
        config.uCBS = KBITS_TO_BYTES(pol_cfg->ckbits_burst);
        config.uEIR = POLICER_RATE_UNITS(pol_cfg->pkbits_sec, base_rate);
        config.uEBS = KBITS_TO_BYTES(pol_cfg->pkbits_burst);
        config.bCoupling = 1;
        config.uRfcMode = SB_FE_2000_PM_PMODE_RFC_2698;
        config.bCBSNoDecrement = 1;
        config.bEBSNoDecrement = 1;
        break;
    default:
        POLICER_ERR(("Invalid policer mode. \n"));
        return BCM_E_CONFIG;
    }

    /* allow over-riding if specified */
    if (pol_cfg->flags & BCM_POLICER_COLOR_BLIND) {
        config.bBlindMode = 1;
    }
    if (pol_cfg->flags & BCM_POLICER_DROP_RED) {
        config.bDropOnRed = 1;
    }

    sbRv = sbFe2000PmPolicerCreate(pPm, policer_id, &config, &id_out);

    if (sbRv != SB_OK) {
        rv = translate_sbx_result(sbRv);
        POLICER_ERR(("%s: failed. return value %d (%s) \n", FUNCTION_NAME(),
                     rv, _SHR_ERRMSG(rv)));
        return rv;
    }

    if ((pol_cfg->flags & BCM_POLICER_WITH_ID) && (policer_id != id_out)) {
        /* if a specific ID is requested, check for the assigned id */
        rv = BCM_E_INTERNAL;
    }

    if (BCM_FAILURE(rv)) {
        sbRv = sbFe2000PmPolicerDelete(pPm, id_out);
        if (sbRv != SB_OK) {
            rv = translate_sbx_result(sbRv);
            POLICER_ERR(("%s: failed. return value %d (%s)\n", FUNCTION_NAME(),
                        rv, _SHR_ERRMSG(rv)));
        }

    }

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_is_stat_enabled
 * Purpose
 *      check if stats are enabled on a policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id to check
 *      (out) monitor_id    = if enabled...monitor id being used
 * Returns
 *      int                 = 0 if not enabled
 *                            1 if enabled
 */
int
_bcm_fe2000_is_stat_enabled(int unit, bcm_policer_t policer_id, 
                            int *monitor_id)
{
    int idx = 0;

    if (SOC_IS_SBX_G2P3(unit) &&
        IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode) &&
        _policer[unit].policers[policer_id].cookie) {
        /* G2P3 supports ucode policer stats */
        return 1;
    } else {
        /* default to HW monitors */
        for (idx=0; idx<BCM_FE2000_NUM_MONITORS; idx++) {
            if (_policer[unit].mon_use_map[idx] == policer_id) {
                *monitor_id = idx;
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Function
 *      _bcm_fe2000_is_monitor_stat_enabled
 * Purpose
 *      check if monitor is attached to this policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id to check
 *      (out) monitor_id    = if enabled...monitor id being used
 * Returns
 *      int                 = 0 if not enabled
 *                            1 if enabled
 */
int
_bcm_fe2000_is_monitor_stat_enabled(int unit, bcm_policer_t policer_id, 
                                    int *monitor_id)
{
    int idx = 0;

    for (idx=0; idx<BCM_FE2000_NUM_MONITORS; idx++) {
        if (_policer[unit].mon_use_map[idx] == policer_id) {
            *monitor_id = idx;
            return 1;
        }
    }

    return 0;
}

/*
 * Function
 *      _bcm_fe2000_monitor_mem_lookup
 * Purpose
 *      Get the memory location of the counter
 * Parameters
 *      (in) unit           = unit number
 *      (in) stat           = stat type
 *      (out) mem           = location of the stat in memory
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_fe2000_monitor_mem_lookup(int unit, bcm_policer_stat_t stat, int *mem)
{
    int rv = BCM_E_NONE;

    switch (stat) {
    case bcmPolicerStatGreenToGreenPackets: *mem = 1; break;
    case bcmPolicerStatGreenToGreenBytes: *mem = 2; break;
    case bcmPolicerStatGreenToYellowPackets: *mem = 3; break;
    case bcmPolicerStatGreenToYellowBytes: *mem = 4; break;
    case bcmPolicerStatGreenToRedPackets: *mem = 5; break;
    case bcmPolicerStatGreenToRedBytes: *mem = 6; break;
    case bcmPolicerStatGreenToDropPackets: *mem = 7; break;
    case bcmPolicerStatGreenToDropBytes: *mem = 8; break;
    case bcmPolicerStatYellowToGreenPackets: *mem = 9; break;
    case bcmPolicerStatYellowToGreenBytes: *mem = 10; break;
    case bcmPolicerStatYellowToYellowPackets: *mem = 11; break;
    case bcmPolicerStatYellowToYellowBytes: *mem = 12; break;
    case bcmPolicerStatYellowToRedPackets: *mem = 13; break;
    case bcmPolicerStatYellowToRedBytes: *mem = 14; break;
    case bcmPolicerStatYellowToDropPackets: *mem = 15; break;
    case bcmPolicerStatYellowToDropBytes: *mem = 16; break;
    case bcmPolicerStatRedToGreenPackets: *mem = 17; break;
    case bcmPolicerStatRedToGreenBytes: *mem = 18; break;
    case bcmPolicerStatRedToYellowPackets: *mem = 19; break;
    case bcmPolicerStatRedToYellowBytes: *mem = 20; break;
    case bcmPolicerStatRedToRedPackets: *mem = 21; break;
    case bcmPolicerStatRedToRedBytes: *mem = 22; break;
    case bcmPolicerStatRedToDropPackets: *mem = 23; break;
    case bcmPolicerStatRedToDropBytes: *mem = 24; break;
    default: rv = BCM_E_PARAM; break;
    }

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_monitor_stat_get
 * Purpose
 *      get the specified stat value
 * Parameters
 *      (in) unit           = unit number
 *      (in) monitor_id     = monitor id
 *      (in) stat           = stat type
 *      (out)val            = stat value
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_fe2000_monitor_stat_get(int unit, int monitor_id, 
                            bcm_policer_stat_t stat, uint64 *val)
{
    uint32      counter_pair[2];
    int         mem;
    int         rv;
    sbhandle    sbh;

    if (monitor_id >= BCM_FE2000_NUM_MONITORS) {
        POLICER_ERR(("Invalid monitor id %d. Valid values 0-7\n", monitor_id));
        return BCM_E_INTERNAL;
    }

    /* ERRATA work around. Green-to-Green Packet counter cannot be supported */
    if ((stat == bcmPolicerStatGreenToGreenPackets) && 
        (SOC_INFO(unit).chip_type == SOC_INFO_FE2000)) {
        return BCM_E_UNAVAIL;
    }

    rv = _bcm_fe2000_monitor_mem_lookup(unit,stat, &mem);
    if (rv != BCM_E_NONE) {
        POLICER_ERR(("Invalid stat type %d. Valid stats 0-%d \n", stat, 
                     bcmPolicerStatCount));
        return rv;
    }

    sbh = SOC_SBX_SBHANDLE(unit);
    rv = sbFe2000UtilReadIndir(sbh, 0, 0, 
                               SAND_HAL_FE2000_REG_OFFSET(sbh, PM_COUNT_MEM_ACC_CTRL),
                               ((16 * monitor_id) + ((mem - 1)/2)),
                               2, counter_pair);
    if (rv) {
        POLICER_ERR(("Internal Error reading from PMU memory \n"));
        return BCM_E_INTERNAL;
    }

    if (mem % 2) {
        /* packets */
        COMPILER_64_SET(*val, 0, (counter_pair[1] >> 3));
    } else {
        /* bytes */
        COMPILER_64_SET(*val, (counter_pair[1] & 0x7), counter_pair[0]);
    }

    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_fe2000_monitor_stat_set
 * Purpose
 *      Set the counter value
 * Parameters
 *      (in) unit           = unit number
 *      (in) monitor_id     = monitor id
 *      (in) stat           = stat type
 *      (in)val             = stat value
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 * Notes
 *      The set to value can only be zero. And the counter pair is set 
 *      when either stat value is indicated to be set.
 */
int
_bcm_fe2000_monitor_stat_set(int unit, int monitor_id, 
                            bcm_policer_stat_t stat, uint64 val)
{
    uint32      counter_pair[2];
    int         mem;
    int         rv;
    sbhandle    sbh;

    if (monitor_id >= BCM_FE2000_NUM_MONITORS) {
        POLICER_ERR(("Invalid monitor id %d. Valid values 0-7\n", monitor_id));
        return BCM_E_INTERNAL;
    }

    rv = _bcm_fe2000_monitor_mem_lookup(unit,stat, &mem);
    if (rv != BCM_E_NONE) {
        POLICER_ERR(("Invalid stat type %d. Valid stats 0-%d \n", stat, 
                     bcmPolicerStatCount));
        return rv;
    }

    COMPILER_64_TO_32_HI(counter_pair[1], val);
    COMPILER_64_TO_32_LO(counter_pair[0], val);

    if ((counter_pair[0] != 0) || (counter_pair[1] != 0)) {
        POLICER_ERR(("Counter can only be set to zero \n"));
        return BCM_E_PARAM;
    }

    sbh = SOC_SBX_SBHANDLE(unit);
    rv = sbFe2000UtilWriteIndir(sbh, 0, 0,
                                SAND_HAL_FE2000_REG_OFFSET(sbh, PM_COUNT_MEM_ACC_CTRL),
                                ((16 * monitor_id) + ((mem - 1)/2)),
                                2, counter_pair);
    if (rv) {
        POLICER_ERR(("Internal error setting counters \n"));
        return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_fe2000_attach_monitor
 * Purpose
 *      Finds a free monitor resource and attaches to a policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = id of the policer to detach monitor from
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_fe2000_attach_monitor(int unit, bcm_policer_t policer_id)
{
    uint32          reg_addr = 0;
    uint32          reg_value = 0;
    int             monitor_id = 0;
    sbhandle        sbh;
    int             stat;
    uint64          val64;
    
    for (monitor_id=0; monitor_id<BCM_FE2000_NUM_MONITORS; monitor_id++) {
        if (_policer[unit].mon_use_map[monitor_id] == -1) {
            break;
        }
    }

    if (monitor_id >= BCM_FE2000_NUM_MONITORS) {
        POLICER_ERR(("No HW resources to enable stats on policer id %d \n",
                     policer_id));
        return BCM_E_RESOURCE;
    }

    COMPILER_64_SET(val64, 0, 0);
    for (stat=0; stat<bcmPolicerStatRedToDropBytes; stat++) {
        /* clear the PM count memory of this monitor */
        _bcm_fe2000_monitor_stat_set(unit, monitor_id, stat, val64);
    }

    sbh = SOC_SBX_SBHANDLE(unit);
    reg_addr = SAND_HAL_FE2000_REG_OFFSET(sbh, PM_MONITOR0_CONFIG) + (4 * monitor_id);
    reg_value = (SAND_HAL_FE2000_SET_FIELD(sbh, PM_MONITOR0_CONFIG, ENABLE, 1)) |
                (SAND_HAL_FE2000_SET_FIELD(sbh, PM_MONITOR0_CONFIG, POLID, policer_id));
    SAND_HAL_WRITE_OFFS(sbh, reg_addr, reg_value);

    _policer[unit].mon_use_map[monitor_id] = policer_id;

    return BCM_E_NONE;
}

/*
 * Function
 *      _bcm_fe2000_g2p3_num_counters_get
 * Purpose
 *      Returns the number of counters in this policer group mode
 * Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode 
 *      (out)ncounters  = number of counters in this mode
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_fe2000_g2p3_num_counters_get(int unit, bcm_policer_group_mode_t mode, 
                                  int *ncounters)
{
    int rv = BCM_E_NONE;
    int num_cos;

    if (!ncounters) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(bcm_cosq_config_get(unit, &num_cos));

    /* Fix me:
     *    Currently g2p3 does not count control
     */
    switch (mode) {

    case bcmPolicerGroupModeSingle: 
    case bcmPolicerGroupModeSingleWithControl: 
        *ncounters = 2; /* drop/pass */
        break;
    case bcmPolicerGroupModeTyped:
    case bcmPolicerGroupModeTypedWithControl:
        *ncounters = 8; /* drop/pass per traffic (unknown uc, uc, mc, bc) */
        break;
    case bcmPolicerGroupModeTypedAll:
    case bcmPolicerGroupModeTypedAllWithControl:
        *ncounters = 7; /* drop per traffic (unknown uc, uc, mc, bc) + 
                           G + Y + R */
        break;
    case bcmPolicerGroupModeTypedIntPri:
    case bcmPolicerGroupModeTypedIntPriWithControl:
        *ncounters = 4 + 3*num_cos; 
        /* drop per traffic (unknown uc, uc, mc, bc) + (G + Y + R)*cos */
        break; 
    default:
        rv = BCM_E_INTERNAL;
    }

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_g2p3_free_counters
 * Purpose
 *      Frees the counter resources associated with this policer id
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id 
 *      (in) use_cookie = inform to use policer cookie
 *      (in) counter    = base counter if use_cookie = 0
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 * NOTE:
 *      Also updates HW to reflect the freed counter resources
 */
int
_bcm_fe2000_g2p3_free_counters(int unit,
                               bcm_policer_t policer_id,
                               int use_cookie,
                               uint32 counter)
{
    int                                 rv = BCM_E_NONE;
    soc_sbx_g2p3_lp_t                   lp;
    _bcm_policer_node_g2p3_cookie_t     *cookie = NULL;
    int                                 lpi, num_ctrs, lpi_max;
    uint32                              base_cntr;
    bcm_policer_group_mode_t            grp_mode;
    
    if (!IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
        POLICER_ERR(("%s: Invalid policer id. Not a base policer. \n", 
                     FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (use_cookie) {
        cookie = (_bcm_policer_node_g2p3_cookie_t *) 
              _policer[unit].policers[policer_id].cookie;
        if (!cookie || !cookie->base_cntr) {
            return BCM_E_INTERNAL;
        }
        base_cntr  = cookie->base_cntr;
    } else {
        if (counter == 0) {
            return BCM_E_INTERNAL;
        }
        base_cntr  = counter;
    }

    if (use_cookie) {
        lpi_max = soc_sbx_g2p3_lp_table_size_get(unit);

        
        for (lpi=0; ((lpi<lpi_max) && (rv == BCM_E_NONE)); lpi++) {
            soc_sbx_g2p3_lp_t_init(&lp);
            rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
            if (lp.policer == policer_id) {
                lp.counter = 0;
                lp.typedcount = 0;
                rv = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
            }
        }
    }

    /* free the resources */
    if (rv == BCM_E_NONE) {
        grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);
        rv = _bcm_fe2000_g2p3_num_counters_get(unit, grp_mode, &num_ctrs);
    }
    if (rv == BCM_E_NONE) {
        rv = _bcm_fe2000_stat_block_free(unit, SOC_SBX_G2P3_INGCTR_ID, 
                                         base_cntr);
    }

    if (rv == BCM_E_NONE) {
        if (use_cookie) {
            sal_free(cookie);
            _policer[unit].policers[policer_id].cookie = 0;
        }
    }
    
    return rv;
}

/*
 * Function
 *      _bcm_fe2000_g2p3_alloc_counters
 * Purpose
 *      Allocates counter resources associated with this policer id
 * Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = policer id 
 *      (in) use_cookie = whether to update policer cookie
 *      (out) counter   = allocated counter if use_cookie = 0
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 * NOTE:
 *      Does not updates HW to reflect the allocated counter resources
 */
int
_bcm_fe2000_g2p3_alloc_counters(int unit, 
                                bcm_policer_t policer_id,
                                int use_cookie,
                                uint32 *counter)
{
    int                                 rv = BCM_E_NONE;
    _bcm_policer_node_g2p3_cookie_t     *cookie = NULL;
    uint32                              base_cntr = 0;
    bcm_policer_group_mode_t            grp_mode;
    int                                 num_ctrs = 0, idx, clear;
    soc_sbx_g2p3_counter_value_t        soc_val;

    if (!IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
        POLICER_ERR(("%s: Invalid policer id. Not a base policer. \n", 
                     FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (!use_cookie && counter == NULL) {
        return BCM_E_INTERNAL;
    }

    if (use_cookie) {
        cookie = (_bcm_policer_node_g2p3_cookie_t *) 
                  sal_alloc(sizeof(_bcm_policer_node_g2p3_cookie_t), 
                            "policer counter cookie");
    }

    if (use_cookie && !cookie) {
        return BCM_E_MEMORY;
    }

    grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);
    rv = _bcm_fe2000_g2p3_num_counters_get(unit, grp_mode, &num_ctrs);
    
    if (rv == BCM_E_NONE) {
        rv = _bcm_fe2000_stat_block_alloc(unit, SOC_SBX_G2P3_INGCTR_ID, 
                                          &base_cntr, num_ctrs);
    }

    if (rv == BCM_E_NONE) {
        /* clear all the counters */
        clear=1;
        for (idx=0; ((idx<num_ctrs) && (rv == BCM_E_NONE)); idx++) {
            rv = soc_sbx_g2p3_ingctr_get(unit, clear, (base_cntr + idx), 
                                         &soc_val);
        }
        if (rv == BCM_E_NONE) {
            if (use_cookie) {
                sal_memset(cookie, 0, sizeof(_bcm_policer_node_g2p3_cookie_t));
                cookie->base_cntr = base_cntr;
                _policer[unit].policers[policer_id].cookie = (uint32)cookie;
            } else {
                *counter = base_cntr;
            }
        } else {
            POLICER_ERR(("%s: failed to clear counter %d \n", 
                         FUNCTION_NAME(), (base_cntr + idx)));
            _bcm_fe2000_stat_block_free(unit, SOC_SBX_G2P3_INGCTR_ID, 
                                        base_cntr);
            if (use_cookie) {
                sal_free(cookie);
            }
        }
    } else {
        if (use_cookie) {
            sal_free(cookie);
        }
    }

    return rv;
}

/*
 * Function
 *      _bcm_fe2000_detach_monitor
 * Purpose
 *      Detach a monitor from policer
 * Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id to detach from
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_fe2000_detach_monitor(int unit, bcm_policer_t policer_id)
{
    uint32      reg_addr = 0;
    uint32      reg_value = 0;
    int         monitor_id = 0;
    sbhandle    sbh;

    for (monitor_id=0; monitor_id<BCM_FE2000_NUM_MONITORS; monitor_id++) {
        if (_policer[unit].mon_use_map[monitor_id] == policer_id) {
            break;
        }
    }

    if (monitor_id >= BCM_FE2000_NUM_MONITORS) {
        POLICER_ERR(("Monitor not attached to policer id %d \n", policer_id));
        return BCM_E_INTERNAL;
    }
    
    sbh = SOC_SBX_SBHANDLE(unit);
    reg_addr = SAND_HAL_FE2000_REG_OFFSET(sbh, PM_MONITOR0_CONFIG) + (4 * monitor_id);
    reg_value = SAND_HAL_FE2000_SET_FIELD(sbh, PM_MONITOR0_CONFIG, ENABLE, 0);
    SAND_HAL_WRITE_OFFS(sbh, reg_addr, reg_value);

    _policer[unit].mon_use_map[monitor_id] = -1;

    return BCM_E_NONE;
}

/*
 *  Function
 *      bcm_fe2000_policer_create
 *  Purpose
 *      Create a policer with a given config.
 *  Parameters
 *      (in) unit           = unit number
 *      (in) pol_cfg        = policer config
 *      (out) policer_id    = id of the created policer
 *  Returns
 *      bcm_error_t         = BCM_E_NONE if successful
 *                            BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_policer_create(int unit,
                          bcm_policer_config_t *pol_cfg,
                          bcm_policer_t *policer_id)
{
    sbFe2000Pm_t    *pPm;
    int             rv;


    POLICER_INIT_CHECK(unit);

    if ((pol_cfg == NULL) || (policer_id == NULL)) {
        return BCM_E_PARAM;
    }

    POLICER_PM_GET_ERROR_RETURN(rv, pPm);

    POLICER_UNIT_LOCK(unit);

    if (pol_cfg->flags & BCM_POLICER_WITH_ID) {
        /* policer_id contains the requested id, validate it */
        rv = _bcm_policer_id_check(unit, *policer_id);
        if (BCM_FAILURE(rv)) {
            POLICER_ERR(("Invalid policer id specified.\n"));
            POLICER_UNIT_UNLOCK(unit);
            return BCM_E_CONFIG;
        }
        rv = _bcm_policer_id_reserve(unit, *policer_id);
        if (BCM_FAILURE(rv)) {
            POLICER_ERR(("Faild to reserve policer id specified.\n"));
            POLICER_UNIT_UNLOCK(unit);
            return BCM_E_CONFIG;
        }
    } else {
        /* auto assign a policer id */
        if (_bcm_policer_id_assign(unit, policer_id) != BCM_E_NONE) {
            POLICER_ERR(("%s: _bcm_policer_id_assign failed. \n", 
                         FUNCTION_NAME()));
            POLICER_UNIT_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }
    }

    /* commit policer to hardware */
    rv = _bcm_policer_hw_create(unit, *policer_id, pol_cfg);

    /* allcoate and save software state */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_policer_state_alloc(unit, *policer_id,
                                      bcmPolicerGroupModeSingle, 1);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_policer_state_set(unit, *policer_id, pol_cfg);
    }

    if (BCM_FAILURE(rv)) {
        _bcm_policer_state_remove(unit, *policer_id);
        /* ensure the id is freed on error, for the case where it is reserved,
         * but not added to the internal state
         */
        _bcm_policer_id_free(unit, *policer_id);
        sbFe2000PmPolicerDelete(pPm, *policer_id);
    }

    POLICER_UNIT_UNLOCK(unit);
    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_destroy
 *  Purpose
 *      Destroy a policer with specified id
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer to destroy
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_policer_destroy(int unit, bcm_policer_t policer_id)
{
    int             bcm_status = BCM_E_NONE;
    sbFe2000Pm_t    *pPm;
    sbStatus_t      sb_status = SB_OK; /* gu2k calls status */


    POLICER_INIT_CHECK(unit);
    POLICER_PM_GET_ERROR_RETURN(bcm_status, pPm);

    sb_status = sbFe2000PmPolicerDelete(pPm, (uint32_t) policer_id);
    if (sb_status != SB_OK) {
        bcm_status = translate_sbx_result(sb_status);
        POLICER_ERR(("%s: failed. return value %d (%s)", FUNCTION_NAME(),
                    bcm_status, _SHR_ERRMSG(bcm_status)));
        return bcm_status;
    }

    POLICER_UNIT_LOCK(unit);
    if (_bcm_policer_state_remove(unit, policer_id) != BCM_E_NONE) {
        POLICER_ERR(("%s: failed. ", FUNCTION_NAME()));
        bcm_status = BCM_E_INTERNAL;
    }
    POLICER_UNIT_UNLOCK(unit);

    return bcm_status;
}

/*
 *  Function
 *      bcm_fe2000_policer_destroy_all
 *  Purpose
 *      Destroy all policers on a given unit
 *  Parameters
 *      (in) unit   = unit number
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_policer_destroy_all(int unit)
{
    int                 bcm_status = BCM_E_NONE;
    sbFe2000Pm_t        *pPm;
    sbStatus_t          sb_status = SB_OK; /* gu2k calls status */
    bcm_policer_t       id;
    int                 idx, pol_count;
    _bcm_policer_node_t *cur;


    POLICER_INIT_CHECK(unit);
    POLICER_PM_GET_ERROR_RETURN(bcm_status, pPm);

    POLICER_UNIT_LOCK(unit);
    cur = _policer[unit].pol_head;
    pol_count = _policer[unit].pol_count;
    for (idx = 0; ((idx < pol_count) && (cur)); idx++) {
        id = cur->id;
        cur = cur->next;
        sb_status = sbFe2000PmPolicerDelete(pPm, (uint32_t) id);
        if (sb_status != SB_OK) {
            bcm_status = translate_sbx_result(sb_status);
            POLICER_ERR(("sbFe2000PmPolicerDelete failed. return value %d (%s)",
                         bcm_status, _SHR_ERRMSG(bcm_status)));
        } else {
            bcm_status = _bcm_policer_state_remove(unit, id);
        }

        if (bcm_status != BCM_E_NONE) {
            POLICER_ERR(("%s: failed. ", FUNCTION_NAME()));
            break;
        }
    }
    POLICER_UNIT_UNLOCK(unit);

    return bcm_status;

}


/*
 *  Function
 *      bcm_fe2000_policer_set
 *  Purpose
 *      Set/Override the config of a policer with a given id and
 *      specified config
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer whose config to change
 *      (in) pol_cfg    = new config
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_policer_set(int unit, bcm_policer_t policer_id,
                       bcm_policer_config_t *pol_cfg)
{
    int                         rv = BCM_E_CONFIG;
    bcm_policer_group_mode_t    group_mode;
    sbFe2000Pm_t                *pPm;

    POLICER_INIT_CHECK(unit);
    POLICER_PM_GET_ERROR_RETURN(rv, pPm);

    /* only supported for group_created id's, so must be WITH_ID and
     * the group_mode must not be single
     */
    if (POLICER_ID_INVALID(unit, policer_id) || (pol_cfg == NULL) ||
        policer_id <= BCM_FE2000_RESERVED_POLICERS ||
        ((pol_cfg->flags & BCM_POLICER_WITH_ID) == 0))
    {
        return BCM_E_PARAM;
    }

    rv = _bcm_fe2000_policer_group_mode_get(unit, policer_id, &group_mode);
    if (BCM_FAILURE(rv) || group_mode == bcmPolicerGroupModeSingle) {
        POLICER_ERR(("%s: Failed to get group mode, or invalid mode=%d\n",
                     FUNCTION_NAME(), group_mode));
        return BCM_E_PARAM;
    }

    /* validate the ID */
    rv = shr_aidxres_list_elem_state(_policer[unit].idlist, policer_id);
    if (rv != BCM_E_EXISTS) {
        POLICER_ERR(("%s: Invalid policer id:%d\n", 
                     FUNCTION_NAME(), policer_id));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);

    /* commit the policer config to hardware */
    rv = _bcm_policer_hw_create(unit, policer_id, pol_cfg);

    /* Policer IDs were stored when the group was created, only need to update
     * the software state with the config
     */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_policer_state_set(unit, policer_id, pol_cfg);
    }

    if (BCM_FAILURE(rv)) {
        POLICER_ERR(("%s: failed. ", FUNCTION_NAME()));
        sbFe2000PmPolicerDelete(pPm, policer_id);
    }

    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_get
 *  Purpose
 *      Retrieve the config of a policer with a given id
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer whose config to retrieve
 *      (out) pol_cfg   = config of the policer with specified id
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_policer_get(int unit, bcm_policer_t id,
                       bcm_policer_config_t *pol_cfg)
{
    int                     status = BCM_E_NONE;
    int                     idx;
    int                     success = 0;
    _bcm_policer_node_t     *cur;

    POLICER_INIT_CHECK(unit);
    POLICER_NULL_PARAM_CHECK(pol_cfg);

    if (POLICER_ID_INVALID(unit, id)) {
        POLICER_ERR(("%s: invalid policer id %d or pol_cfg (%x) \n",
                    FUNCTION_NAME(), id, (uint32)pol_cfg));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);
    cur = _policer[unit].pol_head;
    for (idx = 0; ((idx < _policer[unit].pol_count) && (cur)); idx++) {
        if ((cur->id == id) && (cur->pol_cfg)) {
            /* found the node */
            sal_memcpy(pol_cfg, cur->pol_cfg, sizeof(bcm_policer_config_t));
            success = 1;
            break;
        }
        cur = cur->next;
    }

    if (!success) {
        POLICER_ERR(("%s: could not find policer id %d. \n", 
                     FUNCTION_NAME(), id));
        status = BCM_E_PARAM;
    }
    POLICER_UNIT_UNLOCK(unit);

    return status;
}

/*
 *  Function
 *      bcm_fe2000_policer_traverse
 *  Purpose
 *      Retrieve the config of a policer with a given id
 *  Parameters
 *      (in) unit       = unit number
 *      (in) policer_id = id of policer whose config to retrieve
 *      (out) pol_cfg   = config of the policer with specified id
 *  Returns
 *      bcm_error_t     = BCM_E_NONE if successful
 *                        BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_policer_traverse(int unit,
                            bcm_policer_traverse_cb traverse_callback,
                            void *cookie)
{
    int                     idx, rv;
    _bcm_policer_node_t     *cur;
    bcm_policer_config_t    *pol_cfg;

    rv = BCM_E_NONE;
    POLICER_INIT_CHECK(unit);

    if (!traverse_callback) {
        POLICER_ERR(("%s: Invalid callback function \n", FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (!(pol_cfg = (bcm_policer_config_t *)
                    sal_alloc(sizeof(bcm_policer_config_t), "pol_cfg"))) {
        return BCM_E_MEMORY;
    }

    POLICER_UNIT_LOCK_nr(unit, rv);
    
    if (BCM_SUCCESS(rv)) {
        cur = _policer[unit].pol_head;
        for (idx = 0; ((idx < _policer[unit].pol_count) && (cur)); idx++) {
            sal_memcpy(pol_cfg, cur->pol_cfg, sizeof(bcm_policer_config_t));
            (*traverse_callback)(unit, cur->id, pol_cfg, cookie);
            cur = cur->next;
        }
        POLICER_UNIT_UNLOCK_nr(unit, rv);
    }

    sal_free(pol_cfg);

    return rv;
}

/*  Function
 *      _bcm_reserved_policers_init
 *  Purpose
 *      Configure the reserved policers for exceptions & ports
 *  Parameters
 *      (in) unit   = unit number
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
_bcm_reserved_policers_init(int unit)
{
    bcm_policer_config_t    pol_cfg;
    bcm_policer_t           pol_id;
    int                     idx;
    int                     status = BCM_E_NONE;

    sal_memset(&(pol_cfg), 0, sizeof(pol_cfg));

    pol_cfg.flags |= BCM_POLICER_WITH_ID;
    pol_cfg.mode = bcmPolicerModePassThrough;
    pol_cfg.ckbits_burst = BYTES_TO_KBITS(SBX_DEFAULT_MTU_SIZE);
    
    pol_cfg.ckbits_sec = 1000;
    pol_cfg.pkbits_burst = BYTES_TO_KBITS(SBX_DEFAULT_MTU_SIZE);
    pol_cfg.pkbits_sec = 1000;
    for (idx = 1;
         idx <= (BCM_FE2000_RESERVED_POLICERS - BCM_FE2000_SPECIAL_POLICERS);
         idx++) {
        pol_id = idx;
        status = bcm_fe2000_policer_create(unit, &pol_cfg, &pol_id);
        if (status != BCM_E_NONE) {
            POLICER_ERR(("%s: _bcm_port_policers_init failed. \n",
                         FUNCTION_NAME()));
            break;
        }
    }
    /*
     *  Special policers require explicit initialisation.
     */
    /* drop everything policer */
    if (BCM_E_NONE == status) {
        POLICER_VVERB(("initialised unit %d exception+port policers 1..%d\n",
                       unit,
                       idx - 1));
        pol_cfg.ckbits_burst = 0;
        pol_cfg.ckbits_sec = 0;
        pol_cfg.pkbits_burst = 0;
        pol_cfg.pkbits_sec = 0;
        pol_cfg.mode = bcmPolicerGroupModeSingle;
        pol_cfg.flags = BCM_POLICER_COLOR_BLIND |
                        BCM_POLICER_DROP_RED |
                        BCM_POLICER_WITH_ID;
        pol_id = BCM_FE2000_SPEC_POL_DROP_ALL;
        status = bcm_fe2000_policer_create(unit, &pol_cfg, &pol_id);
        if (BCM_E_NONE == status) {
            POLICER_VVERB(("initialised unit %d drop policer %d\n",
                           unit,
                           pol_id));
        } else {
            POLICER_ERR(("unable to initialise unit %d drop policer (%d): %d (%s)\n",
                         unit,
                         BCM_FE2000_SPEC_POL_DROP_ALL,
                         status,
                         _SHR_ERRMSG(status)));
        }
    }
    return status;
}

int
bcm_fe2000_policer_detach(int unit)
{
    if (_policer[unit].policers) {
        sal_free(_policer[unit].policers);
        _policer[unit].policers = NULL;
    }

    if (_policer[unit].res_idlist) {
        shr_idxres_list_destroy(_policer[unit].res_idlist);
        _policer[unit].res_idlist = NULL;
    }

    /* sal_mutex_destroy(_policer_glob_lock); */
    sal_mutex_destroy(_policer[unit].lock);
    _policer[unit].lock = NULL;

    return BCM_E_NONE;
}


void 
_bcm_fe2000_soc_pm_sw_dump(int unit)
{
    sbFe2000Pm_t                *pPm;
    int                          rv;

    rv = _bcm_fe2000_policer_pm_get(unit, &pPm);
    if (BCM_FAILURE(rv)) {
        soc_cm_print("Failed to get soc pointer: %s\n", bcm_errmsg(rv));
        return;
    }
    sbFe2000PmSwDump(pPm);
}

int
_bcm_policer_id_recover(int unit, int id, 
                        bcm_policer_config_t *policer_config)
{
    sbFe2000Pm_t                *pPm;
    sbStatus_t                  sbrv;
    int                         rv;
    uint32                      profile_id;

    POLICER_PM_GET_ERROR_RETURN(rv, pPm);

    /* read the policer from hardware */
    rv = _bcm_fe2000_policer_get_hw(unit, id, policer_config, &profile_id);
    if (BCM_FAILURE(rv)) {
        POLICER_ERR(("Failed to get policer 0x%04x: %s\n",
                     id, bcm_errmsg(rv)));
        return rv;
    }

    /* recover the policer soc state (profile & ref counts) */
    sbrv = sbFe2000PmRecoverPolicer(pPm, id, profile_id);
    if (sbrv != SB_OK) {
        POLICER_ERR(("Failed to recover policer 0x%04x from soc "
                     "layer: 0x%08x\n",
                     id, sbrv));
        return translate_sbx_result(sbrv);
    }

    return rv;
}

/*
 * Function
 *      bcm_fe2000_policer_recover
 * Purpose
 *      recover the internal policer state during a warm boot
 * Parameters
 *      (in) unit       = unit number
 * Returns
 *   BCM_E_*
 */
int
bcm_fe2000_policer_recover(int unit)
{
    int                      rv, num_lp, lpi, policer, plid;
    uint32                   num_policers;
    soc_sbx_g2p3_lp_t        lp;
    bcm_policer_group_mode_t group_mode;
    bcm_policer_config_t     policer_config;
#ifdef BROADCOM_DEBUG
    sal_usecs_t t0, t1;
#endif

    rv = BCM_E_NONE;
    num_lp = soc_sbx_g2p3_lp_table_size_get(unit);

    POLICER_TIMESTAMP(t0);
    for (lpi = 0; lpi < num_lp; lpi++) {

        rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        if (BCM_FAILURE(rv)) {
            POLICER_ERR(("Failed to read LP[0x%x]: %s\n", 
                         lpi, bcm_errmsg(rv)));
            return rv;
        }

        if (lp.policer) {

            /* decode policer group from logical port */
            rv = _bcm_fe2000_g2p3_policer_lp_decode(unit, &lp, &group_mode);
            if (BCM_FAILURE(rv)) {
                POLICER_ERR(("Failed to decode policer 0x%04x from"
                             "lp 0x%04x: %s\n", 
                             lp.policer, lpi, bcm_errmsg(rv)));
                return rv;
            }

            rv = _bcm_fe2000_policer_group_num_get(unit, 
                                                   group_mode, &num_policers);
            if (BCM_FAILURE(rv)) {
                POLICER_ERR(("Failed to get mode %s policer count: %s\n", 
                             _bcm_fe2000_policer_group_mode_to_str(group_mode),
                             bcm_errmsg(rv)));
                return rv;
            }

            for (policer=0; policer < num_policers; policer++) {
                plid = lp.policer + policer;

                POLICER_VERB(("Recovering policer 0x%x from lpi 0x%04x "
                              "groupMode=%s\n", plid, lpi, 
                              _bcm_fe2000_policer_group_mode_to_str(group_mode)));
                
                rv = _bcm_policer_id_recover(unit, plid, &policer_config);
                if (BCM_FAILURE(rv)) {
                    POLICER_ERR(("Failed to recover policer id 0x%04x: %s\n",
                                 plid, bcm_errmsg(rv)));
                    return rv;
                }

                rv = _bcm_policer_id_reserve(unit, plid);
                if (BCM_FAILURE(rv)) {
                    POLICER_ERR(("Failed to reserve policer id 0x%04x: %s\n",
                                 plid, bcm_errmsg(rv)));
                    return rv;
                }

                rv = _bcm_policer_state_alloc(unit, plid,
                                              group_mode, !policer);
                if (BCM_FAILURE(rv)) {
                    POLICER_ERR(("Failed to allocate policer 0x%04x state: %s\n",
                                 plid, bcm_errmsg(rv)));
                    return rv;
                }

                rv = _bcm_policer_state_set(unit, plid, &policer_config);
                if (BCM_FAILURE(rv)) {
                    POLICER_ERR(("Failed to set policer 0x%04x state: %s\n",
                                 plid, bcm_errmsg(rv)));
                    return rv;
                }
            }
        }
    }
    POLICER_TIMESTAMP(t1);

#ifdef BROADCOM_DEBUG
    t0 = SAL_USECS_SUB(t1, t0);
    POLICER_OUT(0, ("%s: %d us (%d s)\n", FUNCTION_NAME(),
                    t0, t0 / 1000000));
#endif /* BROADCOM_DEBUG */
    
    return rv;
}


/*
 *  Function
 *      bcm_fe2000_policer_init
 *  Purpose
 *      Initialise the policer APIs.
 *  Parameters
 *      (in) unit   = unit number
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_fe2000_policer_init(int unit)
{
    int             status = BCM_E_NONE;
    sal_mutex_t     local_lock;
    int             idx;

    POLICER_UNIT_CHECK(unit);

    if (_policer[unit].lock) {
        status = bcm_fe2000_policer_detach(unit);
        BCM_IF_ERROR_RETURN(status);
        /* _policer_glob_lock = NULL; */
    }

    if (!_policer_glob_lock) {
        /* create and initialize the global lock */
        local_lock = sal_mutex_create("_policer_glob_lock");
        if (!local_lock) {
            POLICER_ERR(("%s: sal_mutex_create failed. \n", FUNCTION_NAME()));
            return BCM_E_RESOURCE;
        }
        if (sal_mutex_take(local_lock, sal_mutex_FOREVER) == 0) {
            /* initialize the global struct */
            sal_memset(&(_policer), 0, sizeof(_policer));
            if (sal_mutex_give(local_lock) != 0) {
                POLICER_ERR(("%s: sal_mutex_give failed.\n", FUNCTION_NAME()));
                sal_mutex_destroy(local_lock);
                return BCM_E_INTERNAL;
            }
        } else {
            POLICER_ERR(("%s: sal_mutex_take failed. \n", FUNCTION_NAME()));
            sal_mutex_destroy(local_lock);
            return BCM_E_INTERNAL;
        }
        _policer_glob_lock = local_lock;
    }

    /* get global lock */
    if (sal_mutex_take(_policer_glob_lock, sal_mutex_FOREVER) != 0) {
        POLICER_ERR(("%s: sal_mutex_take failed. \n", FUNCTION_NAME()));
        sal_mutex_destroy(_policer_glob_lock); 
        return BCM_E_INTERNAL;
    }

    status = BCM_E_NONE;

    for (idx=0; idx<BCM_FE2000_NUM_MONITORS; idx++) {
        /* reset the monitor usage map */
        _policer[unit].mon_use_map[idx] = -1;
    }

    /* verify the exception policers are in the expected location - Bank 0, Id 0
     * Needed when selecting group configs for policer_set
     */
    if (BCM_SUCCESS(status)) {
        soc_sbx_g2p3_policer_params_t pp;

        status = soc_sbx_g2p3_xtpol_policer_params_get(unit, &pp);
        if (BCM_FAILURE(status)) {
            POLICER_ERR(("%s: Failed to get policer params: %d %s\n",
                         FUNCTION_NAME(), status, bcm_errmsg(status)));
        }
        
        if (BCM_SUCCESS(status)) {
            if (pp.size != (BCM_FE2000_XT_POLICERS + 1)) {
                POLICER_ERR(("%s: Unexpected number of exception policers: %d\n",
                             FUNCTION_NAME(), pp.size));
                status = BCM_E_CONFIG;

            } else if (pp.bank != 0) {
                POLICER_ERR(("%s: Incorrect policer gruop, must be bank 0: %d\n",
                             FUNCTION_NAME(), pp.bank));
                status = BCM_E_CONFIG;
            } else {
                _policer[unit].numExceptionPolicers = pp.size;
            }
        }
    }

    if (_policer[unit].lock == 0 && BCM_SUCCESS(status)) {
        /* Now initialize the per unit policer data */
        local_lock = sal_mutex_create("_policer_unit_lock");
        if (local_lock) {
            _policer[unit].policers = sal_alloc((sizeof(_bcm_policer_node_t) *
                                                 _bcm_policer_num_policers_get(unit)),
                                                 "_policer_unit_data");
            if (_policer[unit].policers == 0) {
                status = BCM_E_MEMORY;
            }

            if (status == BCM_E_NONE) {

                /* Start ID managment at 1 to consider 0 as used by ucode -
                 * SOC_SBX_G2P3_NOP_POLICER_ID
                 */
                status = shr_idxres_list_create(& _policer[unit].res_idlist,
                                                1,
                                                BCM_FE2000_RESERVED_POLICERS,
                                                0,
                                                BCM_FE2000_RESERVED_POLICERS,
                                                "Reserved policers ids");
                if (status != BCM_E_NONE) {
                    POLICER_ERR(("%s: shr_idxres_list_create for res policer "
                                 "ids failed on unit %d\n", 
                                 FUNCTION_NAME(), unit));
                    _policer[unit].res_idlist = NULL;
                }
            }

            if (status == BCM_E_NONE) {
                int num_cos = NUM_COS(unit);
                int cos_pow;

                if (num_cos < 1 ) {
                    num_cos = 1;
                }

                /* compute the resource list block factor by finding the
                 * log2(cos), and adding 1.  The block factor is
                 * log2(largest_block_size), the largest block of policers
                 * currenty required is NUM_COS+1.
                 */
                num_cos--;
                for (cos_pow = 0; num_cos; cos_pow++) {
                    num_cos = num_cos >> 1;
                }
                cos_pow++;

                status = shr_aidxres_list_create(& _policer[unit].idlist,
                                                 BCM_FE2000_RESERVED_POLICERS+1,
                                                 _bcm_policer_max_id_get(unit),
                                                 0,
                                                 _bcm_policer_max_id_get(unit),
                                                 cos_pow,
                                                 "Policers ids");

                if (status != BCM_E_NONE) {
                    POLICER_ERR(("%s: shr_idxres_list_create for policer ids "
                                 "failed unit %d\n", FUNCTION_NAME(), unit));
                    _policer[unit].idlist = NULL;
                }
            }

            if (status != BCM_E_NONE) {
                if (_policer[unit].policers) {
                    sal_free(_policer[unit].policers);
                    _policer[unit].policers = NULL;
                }
                if (_policer[unit].res_idlist) {
                    shr_idxres_list_destroy(_policer[unit].res_idlist);
                    _policer[unit].res_idlist = NULL;
                }
                sal_mutex_destroy(local_lock);
                local_lock = 0;
            }
        } else {
            POLICER_ERR(("%s: sal_mutex_create failed. \n", FUNCTION_NAME()));
            status = BCM_E_RESOURCE;
        }

        if (status == BCM_E_NONE) {
            if (sal_mutex_take(local_lock, sal_mutex_FOREVER) == 0) {
                _policer[unit].lock = local_lock;
            } else {
                POLICER_ERR(("%s: sal_mutex_take failed \n", FUNCTION_NAME()));
                status = BCM_E_INTERNAL;
            }
        }
    } else {
        /* lock already created...meaning initialized before */
        /*
        if (sal_mutex_take(_policer[unit].lock, sal_mutex_FOREVER)) {
            POLICER_ERR(("%s: sal_mutex_take failed for unit %d. \n",
                        FUNCTION_NAME(), unit));
            status = BCM_E_INTERNAL; \
        }
        */
        
        POLICER_ERR(("%s: failed. Previously initialized\n", FUNCTION_NAME()));
        status = BCM_E_INTERNAL;
    }

    /* now initialize the per unit policer structure */
    if (BCM_SUCCESS(status)) {
        _policer[unit].pol_count = 0;
        _policer[unit].pol_head = NULL;
        sal_memset(_policer[unit].policers, 0, (sizeof(_bcm_policer_node_t) *
                                                _bcm_policer_num_policers_get(unit)));
        if (sal_mutex_give(_policer[unit].lock)) {
            POLICER_ERR(("%s: sal_mutex_give failed for unit %d. \n",
                        FUNCTION_NAME(), unit));
            status = BCM_E_INTERNAL;
        }
    }

    if (sal_mutex_give(_policer_glob_lock) != 0) {
        POLICER_ERR(("%s: sal_mutex_give failed. \n", FUNCTION_NAME()));
        status = BCM_E_INTERNAL;
    }
    
    if (status == BCM_E_NONE) {
        if ((status = _bcm_reserved_policers_init(unit)) != BCM_E_NONE) {
            POLICER_ERR(("%s: _bcm_port_policers_init failed. \n", 
                         FUNCTION_NAME()));
        }
    }

    POLICER_VERB(("bcm_policer_init: unit=%d rv=%d(%s)\n",
               unit, status, bcm_errmsg(status)));

    if (status != BCM_E_NONE) {
        sal_mutex_destroy(_policer_glob_lock);
    }

    if (SOC_WARM_BOOT(unit)) {
        status = bcm_fe2000_policer_recover(unit);
        if (BCM_FAILURE(status)) {
            POLICER_ERR(("Failed to recover policers: %s\n", 
                         bcm_errmsg(status)));
        }
    }

    return status;
}


/*
 *  Function
 *      bcm_fe2000_policer_group_create
 *  Purpose
 *      Allocates a group of policers
 *  Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode
 *      (out)policer_id = base policer id allocated
 *      (out)npolicers  = number of policers allocated
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *  NOTE
 *      policers are just allocated. They are not configured.
 */
int
bcm_fe2000_policer_group_create(int unit, bcm_policer_group_mode_t mode,
                                bcm_policer_t *policer_id, int *npolicers)
{
    uint32_t num_to_alloc = 0;
    int rv = BCM_E_NONE;

    POLICER_INIT_CHECK(unit);

    rv = _bcm_fe2000_policer_group_num_get(unit, mode, &num_to_alloc);

    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);

    *npolicers = 0;

    if (num_to_alloc == 1) {
        rv = shr_aidxres_list_alloc(_policer[unit].idlist,
                                    (uint32*) policer_id);
    } else {
        rv = shr_aidxres_list_alloc_block(_policer[unit].idlist,
                                          num_to_alloc,
                                          (uint32*) policer_id);
    }


    if (BCM_SUCCESS(rv)) {
        int i;

        POLICER_VERB(("%s: reserved ids %d-%d\n", FUNCTION_NAME(), *policer_id,
                     *policer_id + num_to_alloc - 1));
        *npolicers = num_to_alloc;

        /* create the sw state to track these policer ids */
        for (i=0; i<num_to_alloc && BCM_SUCCESS(rv); i++) {
            rv = _bcm_policer_state_alloc(unit, *policer_id + i, mode, (!i));
        }

        if (BCM_FAILURE(rv)) {
            POLICER_ERR(("%s: failed to create sw state for base policer "
                         "id=%d\n", FUNCTION_NAME(), *policer_id));

            /* only need to free sw sate at this point because no hw state
             * has been written
             */
            for (; i>=0; i--) {
                _bcm_policer_state_remove(unit, *policer_id + i);
            }
        }
    }

    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_stat_enable_get
 *  Purpose
 *      Check if stats are enabled on the policer
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (out) enable        = out value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_fe2000_policer_stat_enable_get(int unit, bcm_policer_t policer_id,
                                   int *enable)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     idx = 0;

    POLICER_NULL_PARAM_CHECK(enable);

    /* check if policer exists */
    rv = bcm_fe2000_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);
    *enable = 0;
    if (SOC_IS_SBX_G2P3(unit)) {
        /* G2P3 supports ucode policer stats */
        if (IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
            if (_policer[unit].policers[policer_id].cookie) {
                *enable = 1;
            }
        } else {
            POLICER_ERR(("%s: Invalid policer id. Not a base policer. \n", 
                         FUNCTION_NAME()));
            rv = BCM_E_PARAM;
        }
    } else {
        /* default to HW monitors */
        for (idx=0; idx<BCM_FE2000_NUM_MONITORS; idx++) {
            if (_policer[unit].mon_use_map[idx] == policer_id) {
                *enable = 1;
                break;
            }
        }
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_stat_enable_set
 *  Purpose
 *      Enables/disables stats on the policer
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) enable         = enable value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_fe2000_policer_stat_enable_set(int unit, bcm_policer_t policer_id,
                                   int enable)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     monitor_id;

    /* check if policer exists */
    rv = bcm_fe2000_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /* only 0 & 1 are valid values for enable */
    if ((enable != 0) && (enable != 1)) {
        POLICER_ERR(("%s: Invalid enable value (%d). valid values - 0,1 \n",
                     FUNCTION_NAME(), enable));
        return BCM_E_PARAM;
    }

    POLICER_UNIT_LOCK(unit);
    if (SOC_IS_SBX_G2P3(unit)) {
        /* check if this is base policer */
        if (!IS_BASE_POLICER(_policer[unit].policers[policer_id].group_mode)) {
            POLICER_ERR(("%s: Invalid policer id. Not a base policer. \n", 
                         FUNCTION_NAME()));
            rv = BCM_E_PARAM;
        }
    }
    if (rv == BCM_E_NONE) {
        if (_bcm_fe2000_is_stat_enabled(unit, policer_id, &monitor_id)) {
            if (enable == 0) {
                if (SOC_IS_SBX_G2P3(unit)) {
                    /* free the counters from the policers */
                    rv = _bcm_fe2000_g2p3_free_counters(unit, policer_id, 1, 0);
                } else {
                    /* default to using HW monitors */
                    /* detach monitor from policer */
                    rv = _bcm_fe2000_detach_monitor(unit, policer_id);
                }
            } /* else...do nothing. Already enabled */
        } else {
            if (enable == 1) {
                if (SOC_IS_SBX_G2P3(unit)) {
                    rv = _bcm_fe2000_g2p3_alloc_counters(unit, policer_id, 1, NULL);
                } else {
                    /* default to using HW monitors */
                    /* attach a monitor to policer */
                    rv = _bcm_fe2000_attach_monitor(unit, policer_id);
                }
            } /* else...do nothing. Already disabled */
        }
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_stat_get
 *  Purpose
 *      gets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to get
 *      (out) val           = out value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_fe2000_policer_stat_get(int unit, bcm_policer_t policer_id, bcm_cos_t cos,
                            bcm_policer_stat_t stat, uint64 *val)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     monitor_id, clear;

    POLICER_NULL_PARAM_CHECK(val);

    /* check if policer exists */
    rv = bcm_fe2000_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);
    if (_bcm_fe2000_is_stat_enabled(unit, policer_id, &monitor_id)) {
        if (SOC_IS_SBX_G2P3(unit)) {
            clear = 0; 
            rv = _bcm_fe2000_g2p3_policer_stat_get(unit, policer_id, stat,
                                                   cos, 1, 0, clear, val);
        } else {
            /* default to HW monitors */
            rv = _bcm_fe2000_monitor_stat_get(unit, monitor_id, stat, val);
        }
        if (rv != BCM_E_NONE) {
            POLICER_ERR(("%s: Failed. \n", FUNCTION_NAME()));
        }
    } else {
        POLICER_ERR(("%s: Stats not enabled on policer id %d \n", 
                     FUNCTION_NAME(), policer_id));
        rv = BCM_E_DISABLED;
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_stat_get32
 *  Purpose
 *      gets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to get
 *      (out) val           = out value
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_fe2000_policer_stat_get32(int unit, bcm_policer_t policer_id, 
                              bcm_cos_t cos, bcm_policer_stat_t stat, 
                              uint32 *val)
{
    uint64  val64;
    int     rv = BCM_E_NONE;

    POLICER_NULL_PARAM_CHECK(val);

    rv = bcm_fe2000_policer_stat_get(unit, policer_id, cos, stat, &val64);
    if (rv == BCM_E_NONE) {
        if (val64 > 0x00000000FFFFFFFFull) {
            *val = 0xFFFFFFFF;
        } else {
            COMPILER_64_TO_32_LO(*val, val64);
        }
    }
    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_stat_set
 *  Purpose
 *      sets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to set
 *      (in) val            = value to set to
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_fe2000_policer_stat_set(int unit, bcm_policer_t policer_id, bcm_cos_t cos,
                            bcm_policer_stat_t stat, uint64 val)
{
    bcm_policer_config_t    pol_cfg;
    int                     rv = BCM_E_NONE;
    int                     monitor_id;

    /* check if policer exists */
    rv = bcm_fe2000_policer_get(unit, policer_id, &pol_cfg);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    POLICER_UNIT_LOCK(unit);
    if (_bcm_fe2000_is_stat_enabled(unit, policer_id, &monitor_id)) {
        if (SOC_IS_SBX_G2P3(unit)) {
            rv = _bcm_fe2000_g2p3_policer_stat_set(unit, policer_id, stat,
                                                   cos, 1, 0, val);
        } else {
            /* default to HW monitors */
            rv = _bcm_fe2000_monitor_stat_set(unit, monitor_id, stat, val);
        }
    } else {
        POLICER_ERR(("%s: Stats not enabled on policer id %d \n", 
                     FUNCTION_NAME(), policer_id));
        rv = BCM_E_DISABLED;
    }
    POLICER_UNIT_UNLOCK(unit);

    return rv;
}

/*
 *  Function
 *      bcm_fe2000_policer_stat_set32
 *  Purpose
 *      sets the specified stat on the given policer id
 *  Parameters
 *      (in) unit           = unit number
 *      (in) policer_id     = policer id
 *      (in) stat           = stat type to set
 *      (in) val            = value to set to
 *  Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int 
bcm_fe2000_policer_stat_set32(int unit, bcm_policer_t policer_id, 
                              bcm_cos_t cos, bcm_policer_stat_t stat, 
                              uint32 val)
{
    uint64  val64;
    int     rv;

    COMPILER_64_SET(val64, 0, val);
    rv = bcm_fe2000_policer_stat_set(unit, policer_id, cos, stat, val64);

    return rv;
}

const char *
_bcm_fe2000_policer_group_mode_to_str(bcm_policer_group_mode_t groupMode) 
{
    static const char* strs[] =
        { "bcmPolicerGroupModeSingle",
          "bcmPolicerGroupModeTrafficType",
          "bcmPolicerGroupModeDlfAll",
          "bcmPolicerGroupModeDlfIntPri",
          "bcmPolicerGroupModeTyped",
          "bcmPolicerGroupModeTypedAll",
          "bcmPolicerGroupModeTypedIntPri"} ;

    if (groupMode >= 0 && groupMode <= bcmPolicerGroupModeTypedIntPri) {
        return strs[groupMode];
    }
    return "<unknown>";
}

/*
 * Function
 *      _bcm_fe2000_g2p3_num_counters_get
 * Purpose
 *      Returns the number of counters in this policer group mode
 * Parameters
 *      (in) unit       = unit number
 *      (in) mode       = group mode 
 *      (out)ncounters  = number of counters in this mode
 * Returns
 *      bcm_error_t         = BCM_E_* appropriately
 */
int
_bcm_fe2000_g2p3_num_policer_counters_get(int unit,
                                          bcm_policer_t policer_id,
                                          int *ncounters)
{
    int                     rv = BCM_E_NONE;
    bcm_policer_config_t    pol_cfg;

    /* check if policer exists */
    rv = bcm_fe2000_policer_get(unit, policer_id, &pol_cfg);
    if (BCM_SUCCESS(rv) && ncounters) {
        bcm_policer_group_mode_t  grp_mode;
        POLICER_UNIT_LOCK(unit);

        grp_mode = POLICER_GROUP_MODE_GET(unit, policer_id);
        rv = _bcm_fe2000_g2p3_num_counters_get(unit, grp_mode, ncounters);

        POLICER_UNIT_UNLOCK(unit);
        
    } else {
        rv = BCM_E_PARAM;
    }
    return rv;
}
