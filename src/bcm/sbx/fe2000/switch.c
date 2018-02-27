/*
 * $Id: switch.c 1.72 Broadcom SDK $
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
 * File:        switch.c
 * Purpose:     BCM definitions  for bcm_switch_control
 *              
 */

#include <soc/drv.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/fe2k/sbFe2000Pp.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#include <bcm_int/sbx/fe2000/g2p3.h>
extern int bcm_fe2000_field_wb_state_sync(int unit, int sync);
#endif /* BCM_FE2000_P3_SUPPORT */

#include <bcm/switch.h>
#include <bcm/error.h>
#include <bcm/stack.h>
#include <bcm/debug.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/switch.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/l2.h>
#include <bcm_int/sbx/l3.h>
#include <bcm_int/sbx/ipmc.h>
#include <bcm_int/sbx/mcast.h>
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/recovery.h>

#define SWITCH_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_STK, stuff)
#define SWITCH_OUT(stuff)           SWITCH_DEBUG(0, stuff)
#define SWITCH_WARN(stuff)          SWITCH_DEBUG(BCM_DBG_WARN, stuff)
#define SWITCH_ERR(stuff)           SWITCH_DEBUG(BCM_DBG_ERR, stuff)
#define SWITCH_VERB(stuff)          SWITCH_DEBUG(BCM_DBG_VERBOSE, stuff)
 

/* max number of G2 exceptions mapped to a single bcmSwitchDosAttack* type */
#define MAX_MAPPED   10


/* global drop preference for captured BroadShield packets */
STATIC uint8_t _drop[BCM_LOCAL_UNITS_MAX];

/* Structures to translate between the BCM Broadshield types and the gu2k
 * implementation.  In general, there are three types:
 *  BOOL - Enable or Disable the check with a global drop preference
 *  CPU  - Define the drop preference for a particular check that is
 *         always enabled.
 *  REG  - For 'integer' value switch controls; these have specific registers
 *         associated to influence hardware checking.
 */
typedef enum {
    sbx_sw_ctrl_type_bool,
    sbx_sw_ctrl_type_reg,
    sbx_sw_ctrl_type_cpu,
} sbx_sw_ctrl_type;

typedef struct {
    sbFe2000PpExcType_t sb_type;
    sbx_sw_ctrl_type  ctrl_type;  /* reg type, bool type, to cpu type */
    
    /*  valid only for reg type */
    uint32_t reg_offset;
    uint32_t reg_data_mask;
    uint32_t reg_data_shift;
} sbx_sw_ctrl_desc_t;

#define CPU_TYPE(x)  (x.ctrl_type == sbx_sw_ctrl_type_cpu)
#define REG_TYPE(x)  (x.ctrl_type == sbx_sw_ctrl_type_reg)
#define BOOL_TYPE(x) (x.ctrl_type == sbx_sw_ctrl_type_bool)


/* Array initializer used to determine the set of BroadShield exceptions from
 * G2.  Used when changing the global drop preference
 */
#define G2_BOOL_TYPES_ARRAY   \
{                                                    \
    SB_FE2000_INV_IPV4_SA_EQ_DA,                      \
    SB_FE2000_IPV6_SA_EQ_DA,                          \
    SB_FE2000_ENET_SMAC_EQ_DMAC_ZERO,                 \
    SB_FE2000_L4_TINY_FRAG,                           \
    SB_FE2000_IPV6_RUNT_PKT,                          \
    SB_FE2000_IPV6_PKT_LEN_ERR,                       \
    SB_FE2000_IPV4_RUNT_PKT,                          \
    SB_FE2000_INV_IPV4_RUNT_HDR,                      \
    SB_FE2000_INV_IPV4_LEN_ERR,                       \
    SB_FE2000_INV_IPV4_PKT_LEN_ERR,                   \
    SB_FE2000_IPV4_FRAG_ICMP_PROTOCOL,                \
    SB_FE2000_TCP_OR_UDP_DP_EQUAL_SP,                 \
    SB_FE2000_TCP_SYN_AND_FIN_BOTH_SET,               \
    SB_FE2000_TCP_SQ_EQ_ZERO_AND_FIN_URG_PSH_ZERO,    \
    SB_FE2000_TCP_SQ_EQ_ZERO_AND_FLAG_ZERO,           \
    SB_FE2000_L4_SYN_SPORT_LT_1024,                   \
}

#define G2_CPU_TYPES_ARRAY    \
{                                                     \
    SB_FE2000_INV_IPV4_CHECKSUM,                      \
    SB_FE2000_INV_IPV4_VER,                           \
    SB_FE2000_INV_IPV4_SA,                            \
    SB_FE2000_INV_IPV4_DA,                            \
    SB_FE2000_INV_IPV6_VER,                           \
    SB_FE2000_INV_IPV6_SA,                            \
    SB_FE2000_INV_IPV6_DA,                            \
    SB_FE2000_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS,       \
    SB_FE2000_INV_IPV4_SA_OR_DA_IS_LOOPBACK,          \
    SB_FE2000_IPV6_SA_OR_DA_LOOPBACK,                 \
    SB_FE2000_ENET_SMAC_EQ_DMAC,                      \
    SB_FE2000_UNK_MPLS_LBL_LABEL0,                    \
    SB_FE2000_UNK_MPLS_LBL_LABEL1,                    \
    SB_FE2000_UNK_MPLS_LBL_LABEL2,                    \
    SB_FE2000_IPV4_OPTIONS,                           \
    SB_FE2000_ENET_SMAC_EQ_MULTICAST,                 \
    SB_FE2000_INV_PPP_ADDR_CTL,                       \
    SB_FE2000_INV_PPP_PID,                            \
    SB_FE2000_ENET_TYPE_BETWEEN_1501_AND_1536,        \
    SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG1,               \
    SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG2,               \
    SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG3,               \
    SB_FE2000_INV_GRE_RES0,                           \
}



typedef enum _fe2k_xt_cos_override_ids_e {
    fe2k_xt_learn                   = 0,
    
    fe2k_xt_cos_override_ids_count
} fe2k_xt_cos_override_ids_e;

typedef struct _fe2k_xt_cos_override_t {
    uint32_t xt;
    int      cos;  /* as added to the BASE, not the default, Higher the cos
                    * numerically, the LOWER the prirority in the QE*/
} fe2k_xt_cos_override_t;

/*
 * Function:
 *   _bcm_to_gu2_broad_shield_type_get
 * Purpose:
 *   Return the 1-many mapping of bcmSwitchDosAttack* type to G2 Exceptions
 */
int
_bcm_to_gu2_broad_shield_type_get(int unit,
                                  bcm_switch_control_t bcm_type,
                                  sbx_sw_ctrl_desc_t ctrl_desc[],
                                  int *num_mapped)
{
    int num = 0;
    int rv = BCM_E_NONE;

    if (*num_mapped < MAX_MAPPED) {
        return BCM_E_PARAM;
    }
    *num_mapped = 0;
   
    /* Set 'ctrl_desc' members and Increment 'num' index ONCE */
#define STI_BASE(sbt, t) \
     ctrl_desc[num].sb_type = sbt;    \
     ctrl_desc[num].ctrl_type = t;   \
     num++;

    /* Macros for setting the switch control data type and 
     * incrementing the num_mapped counter
     */
#define STI_BOOL(sbt)  STI_BASE(sbt, sbx_sw_ctrl_type_bool)
#define STI_CPU(sbt)   STI_BASE(sbt, sbx_sw_ctrl_type_cpu)
#define STI_REG(unit, reg, field) \
      ctrl_desc[num].reg_offset = SAND_HAL_FE2000_REG_OFFSET(unit, reg); \
      ctrl_desc[num].reg_data_mask = SAND_HAL_FE2000_FIELD_MASK(unit, reg, field); \
      ctrl_desc[num].reg_data_shift = SAND_HAL_FE2000_FIELD_SHIFT(unit, reg, field); \
      STI_BASE(0, sbx_sw_ctrl_type_reg); 

    switch (bcm_type)
    {
    case bcmSwitchDosAttackSipEqualDip:
        STI_BOOL(SB_FE2000_INV_IPV4_SA_EQ_DA);
        STI_BOOL(SB_FE2000_IPV6_SA_EQ_DA);
        break;

    case bcmSwitchSourceMacZeroDrop:
        STI_BOOL(SB_FE2000_ENET_SMAC_EQ_DMAC_ZERO);
        break;

    case bcmSwitchDosAttackMinTcpHdrSize:
        
        STI_REG(unit, PP_MIN_TFRAG, MIN_TFRAG);
        break;
    case bcmSwitchDosAttackTcpFrag:
        STI_BOOL(SB_FE2000_L4_TINY_FRAG);
        break;

    case bcmSwitchL3HeaderErrToCpu:
        STI_CPU(SB_FE2000_INV_IPV4_CHECKSUM);
        STI_CPU(SB_FE2000_INV_IPV4_VER);
        STI_CPU(SB_FE2000_INV_IPV4_SA);
        STI_CPU(SB_FE2000_INV_IPV4_DA);
        STI_CPU(SB_FE2000_INV_IPV6_VER);
        STI_CPU(SB_FE2000_INV_IPV6_SA);
        STI_CPU(SB_FE2000_INV_IPV6_DA;);
        break;

    case bcmSwitchDosAttackL3Header: /* length errors */
        STI_BOOL(SB_FE2000_IPV6_RUNT_PKT);
        STI_BOOL(SB_FE2000_IPV6_PKT_LEN_ERR);
        STI_BOOL(SB_FE2000_IPV4_RUNT_PKT);
        STI_BOOL(SB_FE2000_INV_IPV4_RUNT_HDR);
        STI_BOOL(SB_FE2000_INV_IPV4_LEN_ERR);
        STI_BOOL(SB_FE2000_INV_IPV4_PKT_LEN_ERR);
        break;

        /* Martian address check is both a CPU type and a BOOL type which
         * allows it to be both enabled/disabled and sent to cpu or dropped.
         * It is NOT controlled by teh global drop preference
         */
    case bcmSwitchMartianAddr:
        STI_BOOL(SB_FE2000_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS);
        STI_BOOL(SB_FE2000_INV_IPV4_SA_OR_DA_IS_LOOPBACK);
        STI_BOOL(SB_FE2000_IPV6_SA_OR_DA_LOOPBACK);
        break;

    case bcmSwitchMartianAddrToCpu:
        STI_CPU(SB_FE2000_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS);
        STI_CPU(SB_FE2000_INV_IPV4_SA_OR_DA_IS_LOOPBACK);
        STI_CPU(SB_FE2000_IPV6_SA_OR_DA_LOOPBACK);
        break;

    case bcmSwitchMplsInvalidActionToCpu:
        STI_CPU(SB_FE2000_UNK_MPLS_LBL_LABEL0);
        STI_CPU(SB_FE2000_UNK_MPLS_LBL_LABEL1);
        STI_CPU(SB_FE2000_UNK_MPLS_LBL_LABEL2);
        break;

    case bcmSwitchL3SlowpathToCpu:
        STI_CPU(SB_FE2000_IPV4_OPTIONS);
        break;

    case bcmSwitchMcastUnknownErrToCpu: 
        STI_CPU(SB_FE2000_ENET_SMAC_EQ_MULTICAST);
        break;

    case bcmSwitchL2InvalidCtlToCpu:
        STI_CPU(SB_FE2000_INV_PPP_ADDR_CTL);
        STI_CPU(SB_FE2000_INV_PPP_PID);
        STI_CPU(SB_FE2000_ENET_TYPE_BETWEEN_1501_AND_1536);
        STI_CPU(SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG1);
        STI_CPU(SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG2);
        STI_CPU(SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG3);
        break;

    case bcmSwitchInvalidGreToCpu:
        STI_CPU(SB_FE2000_INV_GRE_RES0);
        break;

    case bcmSwitchDosAttackMACSAEqualMACDA:
        STI_CPU(SB_FE2000_ENET_SMAC_EQ_DMAC);
        break;

    case bcmSwitchDosAttackIcmpFragments:
        STI_BOOL(SB_FE2000_IPV4_FRAG_ICMP_PROTOCOL);
        break;

    case bcmSwitchDosAttackUdpPortsEqual:  /* fall thru */
    case bcmSwitchDosAttackTcpPortsEqual:
        STI_BOOL(SB_FE2000_TCP_OR_UDP_DP_EQUAL_SP);
        break;

    case bcmSwitchDosAttackTcpFlagsSF:
        STI_BOOL(SB_FE2000_TCP_SYN_AND_FIN_BOTH_SET);
        break;

    case bcmSwitchDosAttackTcpXMas:
        STI_BOOL(SB_FE2000_TCP_SQ_EQ_ZERO_AND_FIN_URG_PSH_ZERO);
        break;

    case bcmSwitchDosAttackFlagZeroSeqZero:
        STI_BOOL(SB_FE2000_TCP_SQ_EQ_ZERO_AND_FLAG_ZERO);
        break;

    case bcmSwitchDosAttackSynFrag:
        STI_BOOL(SB_FE2000_L4_SYN_SPORT_LT_1024);
        break;
    default:
        return BCM_E_UNAVAIL;
    }

#ifdef BROADCOM_DEBUG
    /* verify the mapping returned is in the list of supported types when
     * changing bcmSwitchDosAttackToCpu
     */
    if (num && BCM_SUCCESS(rv)) {
        int bool_types[] = G2_BOOL_TYPES_ARRAY;
        int num_supported = COUNTOF(bool_types);
        int i, j;

        /* if this error occurs, then the type returned MUST be added to
         * the G2_BOOL_TYPES_ARRAY macro !  The macro is used to determine
         * the set of attributes to modify when changing the forwarding
         * path of the exception packet - drop or cpu
         * Exception :
         *    bcmSwitchMartianAddr -
         *       SB_FE2000_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS
         *       SB_FE2000_INV_IPV4_SA_OR_DA_IS_LOOPBACK
         *       SB_FE2000_IPV6_SA_OR_DA_LOOPBACK
         */
        for (j=0; j<num; j++) {
            if (ctrl_desc[j].ctrl_type == sbx_sw_ctrl_type_bool &&
                ctrl_desc[j].sb_type != SB_FE2000_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS &&
                ctrl_desc[j].sb_type != SB_FE2000_INV_IPV4_SA_OR_DA_IS_LOOPBACK    &&
                ctrl_desc[j].sb_type != SB_FE2000_IPV6_SA_OR_DA_LOOPBACK)
            {
                for (i=0; i<num_supported; i++) {
                    if (ctrl_desc[j].sb_type == bool_types[i] ) {
                        break;
                    }
                }

                if ((i >= num_supported) || 
                    (ctrl_desc[j].sb_type != bool_types[i])) {
                    rv = BCM_E_INTERNAL;
                    break;
                }               
            }
        }

    }
#endif

    *num_mapped = num;
    return rv;
}

void
_rmw_reg(int unit, sbx_sw_ctrl_desc_t *ctrl_desc, uint32_t val)
{  
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    uint32_t data;

    data = SAND_HAL_READ_OFFS(sbx->sbhdl, ctrl_desc->reg_offset);

    data &= ~ctrl_desc->reg_data_mask;
    data |= (val << ctrl_desc->reg_data_shift) & ctrl_desc->reg_data_mask;

    SAND_HAL_WRITE_OFFS(sbx->sbhdl, ctrl_desc->reg_offset, data);
}


void
_read_reg(int unit, sbx_sw_ctrl_desc_t *ctrl_desc, uint32_t *val)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    uint32_t data;

    data = SAND_HAL_READ_OFFS(sbx->sbhdl, ctrl_desc->reg_offset);
    *val = (data & ctrl_desc->reg_data_mask) >> ctrl_desc->reg_data_shift;

}

#ifdef BCM_FE2000_P3_SUPPORT
/*
 * Function: 
 *   _bcm_g2p3_exception_drop_set
 * Purpose:
 *  Set the g2p3 exception type's drop preference
 */
int
_bcm_g2p3_exception_drop_set(int unit, sbFe2000PpExcType_t eType,
                        uint8_t bDrop)
{
    int rv = BCM_E_INTERNAL;
#ifdef BCM_QE2000_SUPPORT
    soc_sbx_g2p3_xt_t xt;

    rv = soc_sbx_g2p3_xt_get(unit, eType, &xt);
    if (SOC_SUCCESS(rv)) {
        xt.forward = !bDrop;
        rv = soc_sbx_g2p3_xt_set(unit, eType, &xt);
    }
#endif /* BCM_QE2000_SUPPORT */

    return rv;
}

/*
 * Function: 
 *   _bcm_g2p3_exception_cos_set
 * Purpose:
 *  Set the g2p3 exception type's drop cos
 */
int
_bcm_g2p3_exception_cos_set(int unit, sbFe2000PpExcType_t eType,
                        uint8_t uCos)
{
    int rv = BCM_E_INTERNAL;
#ifdef BCM_QE2000_SUPPORT
    soc_sbx_g2p3_xt_t xt;
    uint32_t num_cos, mask;

    rv = soc_sbx_g2p3_xt_get(unit, eType, &xt);
    if (SOC_SUCCESS(rv)) {
	num_cos = sbx_num_cosq[unit];
        if (num_cos > 4) {
            mask = 0x7;
        }else if (num_cos > 2) {
            mask = 0x3;
        }else{
            mask = 0x1;
        }

        xt.qid = (xt.qid & (~mask)) | uCos;
        rv = soc_sbx_g2p3_xt_set(unit, eType, &xt);
    }
#endif /* BCM_QE2000_SUPPORT */

    return rv;
}

/*
 * Function: 
 *   _bcm_g2p3_exception_cos_get
 * Purpose:
 *  Get the g2p3 exception type's cos
 */
int
_bcm_g2p3_exception_cos_get(int unit, sbFe2000PpExcType_t eType,
                        uint8_t* uCos)
{
    int rv = BCM_E_INTERNAL;
#ifdef BCM_QE2000_SUPPORT
    soc_sbx_g2p3_xt_t xt;
    uint32_t num_cos, mask;

    rv = soc_sbx_g2p3_xt_get(unit, eType, &xt);
    if (SOC_SUCCESS(rv)) {
        num_cos = sbx_num_cosq[unit];
        if (num_cos > 4) {
            mask = 0x7;
        }else if (num_cos > 2) {
            mask = 0x3;
        }else{
            mask = 0x1;
        }
        *uCos = xt.qid & mask;
    }
#endif /* BCM_QE2000_SUPPORT */

    return rv;
}
/*
 * Function:
 *   _bcm_g2p3_drop_on_exception_set
 * Purpose:
 *   Iterate over all BOOL_TYPES_ARRAY that are currently enabled and
 *   set the drop preference.
 */
int
_bcm_g2p3_drop_on_exception_set(int unit, uint8_t drop)
{
    int                     i, rv = BCM_E_NONE;
    sbFe2000PpExcType_t     eType;
    sbFe2000PpExcType_t     bool_types[] = G2_BOOL_TYPES_ARRAY;
    int                     num_supported = COUNTOF(bool_types);
    uint8_t                 bEnable = 1;
    soc_sbx_g2p3_state_t    *fe = SOC_SBX_CONTROL(unit)->drv;

    if (!fe) {
        return BCM_E_INIT;
    }

    for (i=0; ((i<num_supported) && (SOC_SUCCESS(rv))); i++) {
        eType = bool_types[i];
        rv = soc_sbx_g2p3_pp_broad_shield_check_get(fe, eType, &bEnable);
        if (SOC_SUCCESS(rv)) {
            /* update the exception path only if it is enabled */
            if (bEnable) {
                rv = _bcm_g2p3_exception_drop_set(unit, eType, drop);
            }
        }
    }
    
    return rv;
}


int
_bcm_fe2000_g2p3_switch_exc_table_set(int unit, int base_qid, int rmw)
{
    int                     rv  = BCM_E_INTERNAL;
#ifdef BCM_QE2000_SUPPORT
    fe2k_xt_cos_override_t  override_cos[fe2k_xt_cos_override_ids_count];
    int                     ovrIdx, override, i;
    int                     qid = 0;
    int                     xt_table_size = soc_sbx_g2p3_xt_table_size_get(unit);
    soc_sbx_g2p3_xt_t       xt;

    /* give learning exceptions a slightly lower priority to allow for
     * bpdus to faster service when under a high rate of learning
     */
    BCM_IF_ERROR_RETURN(
        soc_sbx_g2p3_exc_smac_learn_idx_get(unit, 
                                            &override_cos[fe2k_xt_learn].xt));
    override_cos[fe2k_xt_learn].cos = _BCM_SWITCH_EXC_DEFAULT_COS + 1;

    rv = BCM_E_NONE;
    for (i=0; ((i < xt_table_size) && (SOC_SUCCESS(rv))); i++) {
        if (rmw) {
            rv = soc_sbx_g2p3_xt_get(unit, i, &xt);
        } else {
            soc_sbx_g2p3_xt_t_init(&xt);
        }
        
        if (SOC_SUCCESS(rv)) {
            override = 0;
            
            qid = base_qid;
            for (ovrIdx = 0; 
                 ovrIdx < fe2k_xt_cos_override_ids_count && !override; 
                 ovrIdx++) 
            {
                if (override_cos[ovrIdx].xt == i) {
                    qid += override_cos[ovrIdx].cos;
                    override = 1;
                }
            }
            
            if (!override) {
                qid += _BCM_SWITCH_EXC_DEFAULT_COS;
            }
             
            xt.qid = qid;
            rv = soc_sbx_g2p3_xt_set(unit, i, &xt);
        } else {
            SWITCH_ERR((_SBX_D(unit, "Failed get xti=%d\n"), i));
        }
    }
    
    if (SOC_FAILURE(rv)) {
        SWITCH_ERR((_SBX_D(unit, "Failed to set xti=%d to qid=0x%x\n"),
                    i, qid));
    }

#endif
    return rv;
}

int
_bcm_fe2000_g2p3_switch_control_init(int unit)
{
    int                 i, rv = BCM_E_NONE;
    int cpu_types[] = G2_CPU_TYPES_ARRAY;
    int num_cpu_types = COUNTOF(cpu_types);
#if 0 
    int dont_drop[] = G2_DONT_DROP_ARRAY;
    int num_dont_drop = COUNTOF(dont_drop);
#endif
    int bool_types[] = G2_BOOL_TYPES_ARRAY;
    int num_bool_types = COUNTOF(bool_types);
    soc_sbx_g2p3_state_t *fe = SOC_SBX_CONTROL(unit)->drv;

    if (!fe) {
        return BCM_E_INIT;
    }
    _drop[unit] = 1;

#ifdef BCM_QE2000_SUPPORT
    rv = _bcm_fe2000_g2p3_switch_exc_table_set(unit, SBX_EXC_QID_BASE(unit),
                                               FALSE);
#endif
 
    /* Initialize all the BCM *ToCPU types in G2 to Enabled and Drop */
    for (i=0; ((i<num_cpu_types) && (SOC_SUCCESS(rv))); i++) {
        /* disable the check by default */
        rv = soc_sbx_g2p3_pp_broad_shield_check_set(fe, cpu_types[i], 0);

        /* set the exception to drop */
        if (SOC_SUCCESS(rv)) {
            rv = _bcm_g2p3_exception_drop_set(unit, cpu_types[i], 1);
        }
    }

#if 0    /* enable ucode to insert VSI into learning exceptions OHI field */
    rv = soc_sbx_g2p3_en_vlan_on_exc_set(unit, 1);
#endif

#if 0 
    for (i=0; ((i<num_dont_drop) && (SOC_SUCCESS(rv))); i++) {
        /* set the exception to not drop */
        rv = _bcm_g2p3_exception_drop_set(unit, dont_drop[i], 0);
    }
#endif

    for (i=0; ((i<num_bool_types) && (SOC_SUCCESS(rv))); i++) {
        /* disable the check by default */
        rv = soc_sbx_g2p3_pp_broad_shield_check_set(fe, bool_types[i], 0);
    }

    return rv;
}


int
_bcm_fe2000_g2p3_switch_control_set(int unit, bcm_switch_control_t type, 
                                    int arg)
{
    int                 i, rv = BCM_E_NONE, num_mapped = MAX_MAPPED;
    bcm_gport_t         gport;
    uint32_t            qe_mod, qe_port, node, qid = 0, temp;
    uint8_t             bDrop, uCos;
    sbx_sw_ctrl_desc_t  desc[MAX_MAPPED];
    soc_sbx_g2p3_state_t *fe = SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ft_t   cpu_ft;
    uint32_t exc_idx;

    if (!fe) {
        return BCM_E_INIT;
    }

    switch (type) {

#ifdef BCM_WARM_BOOT_SUPPORT
    case bcmSwitchControlSync:
        BCM_IF_ERROR_RETURN(bcm_fe2000_field_wb_state_sync(unit, 1));
        rv = soc_scache_commit(unit);
        break;
    case bcmSwitchStableSelect:
        rv = soc_stable_set(unit, arg, 0);
        break;
    case bcmSwitchStableSize:
        rv = soc_stable_size_set(unit, arg);
        break;
    case bcmSwitchWarmBoot:
        /* If true, set the Warm Boot state; clear otherwise */
        if (arg) {
            SOC_WARM_BOOT_START(unit);
            rv = BCM_E_NONE;
        } else {
            SOC_WARM_BOOT_DONE(unit);
            
            /* perform warmboot clean up tasks */
            rv = _bcm_fe2000_recovery_done(unit);
        }
        break;

#else  /* BCM_WARM_BOOT_SUPPORT */
    case bcmSwitchControlSync:
    case bcmSwitchStableSelect:
    case bcmSwitchStableSize:
    case bcmSwitchStableUsed:
    case bcmSwitchWarmBoot:
        rv = BCM_E_UNAVAIL;
        break;
#endif  /* BCM_WARM_BOOT_SUPPORT */

    case bcmSwitchCpuProtoBpduPriority:
        uCos = arg;
        if (uCos >= sbx_num_cosq[unit]) {
            return BCM_E_PARAM;
        }
        rv = soc_sbx_g2p3_exc_l2cp_copy_idx_get(unit, &exc_idx);
        if (BCM_SUCCESS(rv)) {
            rv = _bcm_g2p3_exception_cos_set(unit, exc_idx, uCos);
        }
        break;
    case bcmSwitchDosAttackToCpu:
        bDrop = !arg;
        if (_drop[unit] != bDrop) {
            /* iterate over all G2 types that were previously enabled and
             * change the drop preference.
             */
            rv = _bcm_g2p3_drop_on_exception_set(unit, bDrop);

            /* set the global drop preference for future 'sets */
            _drop[unit] = bDrop;
        }
        break;
    case bcmSwitchUnknownIpmcToCpu:
    case bcmSwitchL3UrpfFailToCpu:
        rv = BCM_E_UNAVAIL;
        break;
    case bcmSwitchCpuCopyDestination:
        gport = arg;
        qe_mod = BCM_GPORT_MODPORT_MODID_GET(gport);
        qe_port = BCM_GPORT_MODPORT_PORT_GET(gport);
        if (qe_mod == -1) {
            rv = BCM_E_PARAM;
        }
        /* check for switch gport*/
        if (!BCM_STK_MOD_IS_NODE(qe_mod)) {
            if (qe_mod < SBX_MAX_MODIDS) {
                temp = SOC_SBX_CONTROL(unit)->modport[qe_mod][qe_port];
                qe_port = temp & 0xffff;
                qe_mod = (temp >> 16) & 0xffff;
            } else {
                rv = BCM_E_PARAM;
            }
        }
        
        if (BCM_SUCCESS(rv)) {
            node = BCM_STK_MOD_TO_NODE(qe_mod);
            /* assume maximum number of cos levels to CPU */
            qid = SOC_SBX_NODE_PORT_TO_QID(unit,node, qe_port, NUM_COS(unit));
            
            rv = _bcm_fe2000_g2p3_switch_exc_table_set(unit, qid, TRUE);
        }

        /* set the cpu fte */
        if (BCM_SUCCESS(rv)) {
            rv = soc_sbx_g2p3_ft_get(unit, SBX_CPU_FTE(unit), &cpu_ft);
            if (BCM_SUCCESS(rv)) {
                cpu_ft.qid = qid;
                cpu_ft.lag = 0;
                cpu_ft.lagbase = 0;
                cpu_ft.lagsize = 0;

                rv = soc_sbx_g2p3_ft_set(unit, SBX_CPU_FTE(unit), &cpu_ft);
                if (BCM_FAILURE(rv)) {
                    SWITCH_ERR((_SBX_D(unit, "Set fti=%d failed: %d %s\n"), 
                                SBX_CPU_FTE(unit), rv, bcm_errmsg(rv)));
                }
            } else { 
                SWITCH_ERR((_SBX_D(unit, "get fti=%d failed: %d %s\n"), 
                            SBX_CPU_FTE(unit), rv, bcm_errmsg(rv)));
            }
        }

        break;
    case bcmSwitchL2Cache:
        if (arg) {
            SOC_SBX_STATE(unit)->cache_l2 = TRUE;
        }else{
            SOC_SBX_STATE(unit)->cache_l2 = FALSE;
        }
        break;
    case bcmSwitchL2Commit:
        /* force flush of any cached changes */
        _bcm_fe2000_l2_flush_cache(unit);
        break;
    case bcmSwitchL2AgeDelete:
    {
        if (arg) {
            SOC_SBX_STATE(unit)->l2_age_delete = TRUE;
        }else{
            SOC_SBX_STATE(unit)->l2_age_delete = FALSE;
        }
        break;
    }
    case bcmSwitchIpmcCache:
        if (arg) {
            SOC_SBX_STATE(unit)->cache_ipmc = TRUE;
        }else{
            SOC_SBX_STATE(unit)->cache_ipmc = FALSE;
        }
         break;
    case bcmSwitchIpmcCommit:
        /* force flush of any cached changes */
        _bcm_fe2000_ipmc_flush_cache(unit);
         break;
    case bcmSwitchL3HostCommit:
        /* force flush of any cached changes */
        _bcm_fe2000_l3_flush_cache(unit, arg);
        break;
    case bcmSwitchL3RouteCache:
        if (arg) {
            SOC_SBX_STATE(unit)->cache_l3route = TRUE;
        }else{
            SOC_SBX_STATE(unit)->cache_l3route = FALSE;
        }
        break;
    case bcmSwitchL3RouteCommit:
        /* force flush of any cached changes */
        rv= _bcm_fe2000_l3_flush_cache(unit, arg);
        break;

    case bcmSwitchReserveLowVlanPort:
        if (arg) {
            arg = BCM_GPORT_VLAN_PORT_ID_GET(arg);
            arg = VLAN_VGPORT_ID_TO_FT_INDEX(unit, arg);
        }
        rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT,
                                    FALSE, (uint32_t)arg);
        break;

    case bcmSwitchReserveHighVlanPort:
        if (arg) {
            arg = BCM_GPORT_VLAN_PORT_ID_GET(arg);
            arg = VLAN_VGPORT_ID_TO_FT_INDEX(unit, arg);
        }
        rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT,
                                    TRUE, (uint32_t)arg);
        break;

    case bcmSwitchReserveLowMplsPort:
        /* convert FTI based gportId to the VSI */
        if( SOC_SBX_CFG(unit)->mplstp_ena) {
            
            if (arg) {
                arg = BCM_GPORT_MPLS_PORT_ID_GET(arg);
            }
            rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT,
                                        FALSE, (uint32_t)arg);
        } else {
            if (arg) {
                arg = BCM_GPORT_MPLS_PORT_ID_GET(arg) - SBX_VID_VSI_FTE_BASE(unit);
            }
            rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_VSI,
                                        FALSE, (uint32_t)arg);
        }
        break;

    case bcmSwitchReserveHighMplsPort:
        /* convert FTI based gportId to the VSI */
        if( SOC_SBX_CFG(unit)->mplstp_ena) {
            
            if (arg) {
                arg = BCM_GPORT_MPLS_PORT_ID_GET(arg);
            }
            rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT,
                                        TRUE, (uint32_t)arg);
        } else {
            /* convert FTI based gportId to the VSI */
            if (arg) {
                arg = BCM_GPORT_MPLS_PORT_ID_GET(arg) - SBX_VID_VSI_FTE_BASE(unit);
            }
            rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_VSI,
                                        TRUE, (uint32_t)arg);
        }
        break;

#define SBX_GLOBAL_GPORT_SIZE(unit) (SBX_GLOBAL_GPORT_FTE_END(unit) - SBX_GLOBAL_GPORT_FTE_BASE(unit))
    case bcmSwitchReserveLowMimPort:
        /* verify arg */
        if ((arg < 0) || (arg > SBX_GLOBAL_GPORT_SIZE(unit))) {
            rv = BCM_E_PARAM;
        } else {
            if (arg) {
                /* convert to 0 based mim port id */
                arg += SBX_GLOBAL_GPORT_FTE_BASE(unit); 
            }
            rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT,
                                        FALSE, (uint32_t)arg);
        }
        break;
    case bcmSwitchReserveHighMimPort:
        /* verify arg */
        if ((arg < 0) || (arg > SBX_GLOBAL_GPORT_SIZE(unit))) {
            rv = BCM_E_PARAM;
        } else {
            if (arg) {
                /* convert to 0 based mim port id */
                arg += SBX_GLOBAL_GPORT_FTE_BASE(unit);
            }
            rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT,
                                        TRUE, (uint32_t)arg);
        }
        break;
    case bcmSwitchReserveLowEncap:
        if (arg) {
            arg = SOC_SBX_OHI_FROM_ENCAP_ID(arg);
        }
        rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_OHI,
                                    FALSE, (uint32_t)arg);
        break;
    case bcmSwitchReserveHighEncap:
        if (arg) {
            arg = SOC_SBX_OHI_FROM_ENCAP_ID(arg);
        }
        rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_OHI,
                                    TRUE, (uint32_t)arg);
        break;
    case bcmSwitchReserveLowL2Egress:
        rv = bcm_fe2000_l2_egress_range_reserve(unit, FALSE, (uint32_t)arg);
        break;
    case bcmSwitchReserveHighL2Egress:
        rv = bcm_fe2000_l2_egress_range_reserve(unit, TRUE, (uint32_t)arg);
        break;
    case bcmSwitchReserveLowVpn:
        rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_LINE_VSI,
                                    FALSE, (uint32_t)arg);

        break;
    case bcmSwitchReserveHighVpn:
        rv = _sbx_gu2_range_reserve(unit, SBX_GU2K_USR_RES_LINE_VSI,
                                    TRUE, (uint32_t)arg);
        break;
    case bcmSwitchSynchronousPortClockSource:
        if (!SOC_IS_SBX_FE2KXT(unit)){
            return BCM_E_UNAVAIL;
        }
        if (arg < SB_FE2000_MAX_AG_PORTS) {
            SAND_HAL_WRITE(unit, C2, AG0_RXCLK_MUX, arg);
            SAND_HAL_WRITE(unit, C2, AG1_RXCLK_MUX, 0xf);
        }else if (arg < (SB_FE2000_MAX_AG_PORTS * 2) ) {
            SAND_HAL_WRITE(unit, C2, AG0_RXCLK_MUX, 0xf);
            SAND_HAL_WRITE(unit, C2, AG1_RXCLK_MUX, (arg - SB_FE2000_MAX_AG_PORTS));
        }else if (arg == -1) {
            SAND_HAL_WRITE(unit, C2, AG0_RXCLK_MUX, 0xf);
            SAND_HAL_WRITE(unit, C2, AG1_RXCLK_MUX, 0xf);
        }else{
            return BCM_E_PARAM;
        }

        break;
    case bcmSwitchIgmpPktDrop:
        rv = soc_sbx_g2p3_igmp_proxy_mode_set(unit, (uint32_t)arg);
        break;
/*
 *  MLD snooping is controlled by IGMP snooping control: bcmSwitchIgmpPktDrop
 *
    case bcmSwitchMldPktDrop:
        rv = soc_sbx_g2p3_igmp_proxy_mode_set(unit, (uint32_t)arg);
        break;
 */
    case bcmSwitchMeterAdjust:
        rv = soc_sbx_g2p3_switch_meter_adjust_set(unit, (uint32_t)arg);
        break;
    default:
        /* Get mapping of bcm switch control broad shield type to
         * Gu2 exceptions
         */
        rv = _bcm_to_gu2_broad_shield_type_get(unit, type, desc, &num_mapped);

        for (i=0; i<num_mapped; i++) {

            if (REG_TYPE(desc[i])) {
                _rmw_reg(unit, &desc[i], (uint32_t)arg);

            } else if (BOOL_TYPE(desc[i]) || CPU_TYPE(desc[i])) {

                /* CPU type is assumed to be ON, instead use the caller
                 * supplied argument to specify the drop preference in the
                 * exception table
                 *
                 * BOOL type uses the global _drop preference and the caller
                 * supplied argument is used to enable/disable the check
                 */
                rv = soc_sbx_g2p3_pp_broad_shield_check_set(fe, 
                                                            desc[i].sb_type,
                                                            arg);
                if (BCM_FAILURE(rv)) {
                    return rv;
                }

                if (BOOL_TYPE(desc[i])) {
                    bDrop = _drop[unit];

                } else {
                    /* CPU type */
                    bDrop = !arg; /* == toCpu */
                }

                rv = _bcm_g2p3_exception_drop_set(unit, desc[i].sb_type, bDrop);
                if (BCM_FAILURE(rv)) {
                    return rv;
                }
                
            } else {
                return BCM_E_INTERNAL;
            }
        }
    }
    return rv;
}

int
_bcm_fe2000_g2p3_switch_control_get(int unit, bcm_switch_control_t type,
                                    int *arg)
{
    int                     rv = BCM_E_NONE, num_mapped = MAX_MAPPED;
    uint32_t                junk;
    soc_sbx_g2p3_ft_t       cpu_ft;
    int                     modid, qe_node, qe_port;
    soc_sbx_g2p3_state_t    *fe = SOC_SBX_CONTROL(unit)->drv;
    uint8_t                 bEnable, uCos;
    sbx_sw_ctrl_desc_t      desc[MAX_MAPPED];
#ifdef BCM_QE2000_SUPPORT
    soc_sbx_g2p3_xt_t       xt;
#endif /* BCM_QE2000_SUPPORT */
    uint32_t exc_idx;
    uint32_t reg;

    if (!fe) {
        return BCM_E_INIT;
    }

    switch (type) {
#ifdef BCM_WARM_BOOT_SUPPORT
    case bcmSwitchControlSync:
        rv = BCM_E_NONE;
        break;
    case bcmSwitchStableSelect:
        {
            uint32_t flags;
            rv = soc_stable_get(unit, arg, &flags);
        }
        break;
    case bcmSwitchStableSize:
        rv = soc_stable_size_get(unit, arg);
        break;
    case bcmSwitchStableUsed:
        rv = soc_stable_used_get(unit, arg);
        break;
    case bcmSwitchWarmBoot:
        *arg = SOC_WARM_BOOT(unit);
        break;

#else  /* BCM_WARM_BOOT_SUPPORT */
    case bcmSwitchStableSelect:
    case bcmSwitchStableSize:
    case bcmSwitchStableUsed:
    case bcmSwitchWarmBoot:
        rv = BCM_E_UNAVAIL;
        break;
#endif  /* BCM_WARM_BOOT_SUPPORT */


    case bcmSwitchCpuProtoBpduPriority:
        rv = soc_sbx_g2p3_exc_l2cp_copy_idx_get(unit, &exc_idx);
        if (BCM_SUCCESS(rv)) {
            rv = _bcm_g2p3_exception_cos_get(unit, exc_idx, &uCos);
            if (BCM_SUCCESS(rv)) {
                *arg = uCos;
            }
        }
        break;
    case bcmSwitchDosAttackToCpu:
        *arg = !_drop[unit];
        break;
    case bcmSwitchL3UrpfFailToCpu:
    case bcmSwitchUnknownIpmcToCpu:
        rv = BCM_E_UNAVAIL;
        break;
    case bcmSwitchCpuCopyDestination:
        rv = soc_sbx_g2p3_ft_get(unit, SBX_CPU_FTE(unit), &cpu_ft);
        if (SOC_SUCCESS(rv)) {
            SOC_SBX_NODE_PORT_FROM_QID(unit, cpu_ft.qid, qe_node, qe_port, 
                                       SBX_MAX_COS);
            modid = BCM_STK_NODE_TO_MOD(qe_node);
            BCM_GPORT_MODPORT_SET(*arg, modid, qe_port);
        }
        break;
    case bcmSwitchL2Cache:
        if (SOC_SBX_STATE(unit)->cache_l2) {
            *arg = TRUE;
        }else{
            *arg = FALSE;
        }
        break;
    case bcmSwitchL2Commit:
        *arg = FALSE;
        break;
    case bcmSwitchL2AgeDelete:
        if (SOC_SBX_STATE(unit)->l2_age_delete) {
            *arg = TRUE;
        }else{
            *arg = FALSE;
        }
        break;
    case bcmSwitchL3HostCommit:
        *arg = FALSE;
        break;
    case bcmSwitchL3RouteCache:
        if (SOC_SBX_STATE(unit)->cache_l3route) {
            *arg = TRUE;
        }else{
            *arg = FALSE;
        }
        break;
    case bcmSwitchL3RouteCommit:
        *arg = FALSE;
        break;

    case bcmSwitchReserveLowVlanPort:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT,
                                      (uint32_t*)arg, &junk);
        *arg = VLAN_FT_INDEX_TO_VGPORT_ID(unit, *arg);
        BCM_GPORT_VLAN_PORT_ID_SET(*arg, *arg);
        break;

    case bcmSwitchReserveHighVlanPort:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_FTE_LOCAL_GPORT,
                                      &junk, (uint32_t*)arg);
        *arg = VLAN_FT_INDEX_TO_VGPORT_ID(unit, *arg);
        BCM_GPORT_VLAN_PORT_ID_SET(*arg, *arg);
        break;

    case bcmSwitchReserveLowMplsPort:
        if( SOC_SBX_CFG(unit)->mplstp_ena) {
            
            rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT,
                                          (uint32_t*)arg, &junk);
        } else {
            rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_VSI,
                                          (uint32_t*)arg, &junk);
            /* convert VSI to DLF FTI for FTI based gportId */
            *arg += SBX_VID_VSI_FTE_BASE(unit);
        }
        BCM_GPORT_MPLS_PORT_ID_SET(*arg, *arg);
        break;
    case bcmSwitchReserveHighMplsPort:
       if( SOC_SBX_CFG(unit)->mplstp_ena) {
            
            rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT,
                                          &junk, (uint32_t*)arg);
        } else {
            rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_VSI,
                                      &junk, (uint32_t*)arg);
            /* convert VSI to DLF FTI for FTI based gportId */
            *arg += SBX_VID_VSI_FTE_BASE(unit);
        }
        BCM_GPORT_MPLS_PORT_ID_SET(*arg, *arg);
        break;
    case bcmSwitchReserveLowMimPort:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT,
                                      (uint32_t*)arg, &junk);
        *arg -= SBX_GLOBAL_GPORT_FTE_BASE(unit); /* convert to 0 based mim port id */
        break;
    case bcmSwitchReserveHighMimPort:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT,
                                      &junk, (uint32_t*)arg);
        *arg -= SBX_GLOBAL_GPORT_FTE_BASE(unit); /* convert to 0 based mim port id */
        break;
    case bcmSwitchReserveLowEncap:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_OHI,
                                      (uint32_t*)arg, &junk);
        *arg = SOC_SBX_ENCAP_ID_FROM_OHI(*arg);
        break;
    case bcmSwitchReserveHighEncap:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_OHI,
                                      &junk, (uint32_t*)arg);
        *arg = SOC_SBX_ENCAP_ID_FROM_OHI(*arg);
        break;
    case bcmSwitchReserveLowL2Egress:
        rv = bcm_fe2000_l2_egress_range_get(unit, (uint32_t*)arg, &junk);
        break;
    case bcmSwitchReserveHighL2Egress:
        *arg = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(*arg);
        rv = bcm_fe2000_l2_egress_range_get(unit, &junk, (uint32_t*)arg);
        break;
    case bcmSwitchReserveLowVpn:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_LINE_VSI,
                                      (uint32_t*)arg, &junk);
        break;
    case bcmSwitchReserveHighVpn:
        rv = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_LINE_VSI,
                                      &junk, (uint32_t*)arg);
        break;
    case bcmSwitchSynchronousPortClockSource:
        if (!SOC_IS_SBX_FE2KXT(unit)){
            return BCM_E_UNAVAIL;
        }
        reg = SAND_HAL_READ(unit, C2, AG0_RXCLK_MUX);
        *arg = SAND_HAL_GET_FIELD(C2, AG0_RXCLK_MUX, SELECT, reg);
        if (*arg != 0xf) {
            break;
        }
        reg = SAND_HAL_READ(unit, C2, AG1_RXCLK_MUX);
        *arg = SAND_HAL_GET_FIELD(C2, AG0_RXCLK_MUX, SELECT, reg);
        if (*arg != 0xf) {
            *arg = *arg + SB_FE2000_MAX_AG_PORTS;
            break;
        }
        *arg = -1;

        break;
    case bcmSwitchIgmpPktDrop:
        rv = soc_sbx_g2p3_igmp_proxy_mode_get(unit, (uint32_t*)arg);
        break;
/*
 *  MLD snooping is controlled by IGMP snooping control
 *
    case bcmSwitchMldPktDrop:
        rv = soc_sbx_g2p3_igmp_proxy_mode_get(unit, (uint32_t*)arg);
        break;
 */
    case bcmSwitchMeterAdjust:
        rv = soc_sbx_g2p3_switch_meter_adjust_get(unit, (uint32_t*)arg);
        break;

    default:
        rv = _bcm_to_gu2_broad_shield_type_get(unit, type, desc, &num_mapped);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        /* There may be multple SB types mapped to a single BCM type,
         * assuem they're all the same value, so only need to 'get' one
         */
        if (REG_TYPE(desc[0])) {
            _read_reg(unit, &desc[0], (uint32_t*)arg);

        } else if (BOOL_TYPE(desc[0])) {
            rv = soc_sbx_g2p3_pp_broad_shield_check_get(fe, desc[0].sb_type, 
                                                        &bEnable);
            if (SOC_SUCCESS(rv)) {
                *arg = bEnable;
            }
        } else if (CPU_TYPE(desc[0])) {
#ifdef BCM_QE2000_SUPPORT
            rv = soc_sbx_g2p3_xt_get(unit, desc[0].sb_type, &xt);
            if (SOC_SUCCESS(rv)) {
                *arg = xt.forward; /* == toCPU */
            }
#else
            rv = BCM_E_INTERNAL;
#endif /* BCM_QE2000_SUPPORT */
        } else {
            return BCM_E_INTERNAL;
        }
    }

    return rv;
}

int
_bcm_fe2000_g2p3_switch_control_port_set(int unit, bcm_port_t port,
                                         bcm_switch_control_t type, int arg)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g2p3_ep2e_t     ep2e;

    switch (type) {
    case bcmSwitchKeepEgressRtHdr:
        rv = soc_sbx_g2p3_ep2e_get(unit, port, &ep2e);
        if (SOC_SUCCESS(rv)) {
            ep2e.keeperh = arg;
            rv = soc_sbx_g2p3_ep2e_set(unit, port, &ep2e);
        }
        break;
    default:
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

int
_bcm_fe2000_g2p3_switch_control_port_get(int unit, bcm_port_t port, 
                                         bcm_switch_control_t type, int *arg)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g2p3_ep2e_t     ep2e;
    
    switch (type) {
    case bcmSwitchKeepEgressRtHdr:
        rv = soc_sbx_g2p3_ep2e_get(unit, port, &ep2e);
        if (SOC_SUCCESS(rv)) {
            *arg = ep2e.keeperh;
        }
        break;
    default:
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}
#endif /* BCM_FE2000_P3_SUPPORT */

int
_bcm_fe2000_switch_control_init(int unit)
{
    int rv = BCM_E_INTERNAL;

   switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_switch_control_init(unit);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
#ifdef BCM_FE2000_G2XX_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2XX:
        rv = BCM_E_UNAVAIL;
        break;
#endif /* BCM_FE2000_G2XX_SUPPORT */
   default:
       SBX_UNKNOWN_UCODE_WARN(unit);
       rv = BCM_E_INTERNAL;
   }

    return rv;
}

int
bcm_fe2000_switch_control_set(int unit, bcm_switch_control_t type, 
                              int arg)
{
    int rv = BCM_E_INTERNAL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_switch_control_set(unit, type, arg);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
#ifdef BCM_FE2000_G2XX_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2XX:
        rv = BCM_E_UNAVAIL;
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    return rv;
}

int
bcm_fe2000_switch_control_get(int unit, bcm_switch_control_t type, 
                              int *arg)
{
    int rv = BCM_E_INTERNAL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_switch_control_get(unit, type, arg);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    return rv;
}

int
bcm_fe2000_switch_control_port_set(int unit, 
                                   bcm_port_t port,
                                   bcm_switch_control_t type,
                                   int arg)
{
    int rv = BCM_E_INTERNAL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_switch_control_port_set(unit, port, type, arg);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    return rv;
}


int
bcm_fe2000_switch_control_port_get(int unit, 
                                   bcm_port_t port,
                                   bcm_switch_control_t type,
                                   int *arg)
{
    int rv = BCM_E_INTERNAL;
    
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_switch_control_port_get(unit, port, type, arg);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 
    
    return rv;
}
