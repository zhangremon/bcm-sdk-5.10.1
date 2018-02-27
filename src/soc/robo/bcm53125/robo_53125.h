/*
 * $Id: robo_53125.h 1.3 Broadcom SDK $
 *
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
 */
 
#ifndef _ROBO_53125_H
#define _ROBO_53125_H


#define DRV_MCAST_GROUP_NUM         128
#define DRV_AGE_TIMER_MAX           1048575
#define DRV_TRUNK_GROUP_NUM         2
#define DRV_TRUNK_MAX_PORT_NUM      4
#define DRV_MSTP_GROUP_NUM          8
#define DRV_SEC_MAC_NUM_PER_PORT    1
#define DRV_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_AUTH_SUPPORT_PBMP       0x0000003f
#define DRV_RATE_CONTROL_SUPPORT_PBMP 0x00000003f
#define DRV_VLAN_ENTRY_NUM  4095
#define DRV_BPDU_NUM    1
#define DRV_CFP_TCAM_SIZE 256
#define DRV_CFP_UDFS_NUM 93
#define DRV_CFP_UDFS_OFFSET_MAX (2 * (32 - 1))
#define DRV_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED)
                                        
#define DRV_MAC_LOW_POWER_SUPPORT_PBMP 0x0000001f                                        

/* defined for bcm53115 specific egress basis table : 
 *  - the table access adddress is defined by SW per device designed Spec.
 *      that this table is port basis table.
 *      1. 256 entries per port. (port0~port5 and IMP )
 *      2. table index keys are : egress_port_id + classification_id(from CFP)
 *  - Designed entry index format : 0x00000pdd.
 *      1. p : is egress port id. (0 ~ 6 is valid)
 *      2. dd : is entry id (0 ~ 255 is valid)
 */
#define DRV_EGRESS_V2V_NUM_PER_PORT     256
#define DRV_EGR_V2V_IMP_PORT_SEARCH_ID  8 
#define DRV_EGR_V2V_IMP_PORT_ADDR_ID    6   /* original bit id is "8" */
#define DRV_EGR_V2V_PORT_ADDR_OFFSET    8  /* bit offset in address */
#define DRV_EGR_V2V_ENTRY_ADDR_OFFSET   0   /* bit offset in address */
#define DRV_EGR_V2V_PORT_ADDR_MASK      0x0000FF00 /* port_id mask in addr */
#define DRV_EGR_V2V_ENTRY_ADDR_MASK     0x000000FF /*entry_id mask in addr*/
/* macro to build the EGR_V2V_Address */
#define DRV_EGR_V2V_ENTRY_ID_BUILD(_p, _id) \
            ((((((_p) == CMIC_PORT(unit))?DRV_EGR_V2V_IMP_PORT_ADDR_ID:(_p)) \
                    << DRV_EGR_V2V_PORT_ADDR_OFFSET) & \
                        DRV_EGR_V2V_PORT_ADDR_MASK) | \
            (((_id) << DRV_EGR_V2V_ENTRY_ADDR_OFFSET) & \
                        DRV_EGR_V2V_ENTRY_ADDR_MASK))

#define DRV_EGR_V2V_VIDOP_ASIS    0 /* as the VID in MMU */
#define DRV_EGR_V2V_VIDOP_ASRX    1 /* as the received on the ingress port */
#define DRV_EGR_V2V_VIDOP_REMOVE  2 /* Remove the VID */
#define DRV_EGR_V2V_VIDOP_MODIFY  3 /* modify the VID */

/* pre-defined Egress VLAN Remark(EVR) table index for defined action. */
/* EVR_ID : 
 *  0 : default EVR entry for handling isp/none-isp outer tag action.
 *  1~65 : for vlan translation (transparent/mapping mode) usage.
 *  66~255 : customer used.
 *
 * SW designed to support VT up to 64 VT entryies :
 *  - Current SW designed as system basis VT.
 *  - One VT entry on bcm53115 need 2 CFP entries to serve on driving CalssId
 *      one is for ISP and the other is for none-ISP.
 */
/* EVR entry ID for the default outer/inner tag action */
#define DRV_EVRID_TAG_ACTION_DEF    0
#define DRV_EVRID_VT_ACTION_FISRT   1   /* the first ID for VT */
#define DRV_EVRID_VT_SW_MAX_CNT     64  /* SW max of supporting VT entry */
#define DRV_EVRID_VT_ACTION_LAST    \
            (DRV_EVRID_VT_ACTION_FISRT + DRV_EVRID_VT_SW_MAX_CNT - 1)

#define DRV_EVRID_CUSTOMER_ACTION_FIRST \
            (DRV_EVRID_VT_ACTION_LAST + 1)  /* the first ID for customer */
#define DRV_EVRID_CUSTOMER_ACTION_LAST    \
            (DRV_EGRESS_V2V_NUM_PER_PORT - 1)
#define DRV_EVRID_CUSTOMER_ACTION_COUNT  \
            (DRV_EVRID_CUSTOMER_ACTION_LAST - \
             DRV_EVRID_CUSTOMER_ACTION_FIRST + 1)

/* double tagging mode on bcm53115, bitmap function indicator */
#define DRV_VLAN_DT_MODE_MASK           0x3
#define DRV_VLAN_DT_MODE_DISABLE        0x0
#define DRV_VLAN_FALCON_DT_MODE         0x1
#define DRV_VLAN_INTELLIGENT_DT_MODE    0x2 /* bcm53115 new feautre */

/* Preferred Double Tagging Mode : 
 *  - there are two double tagging modes in bcm53115, i.e. DT_mode/iDT_mode.
 *  - Original design is preferred at DT_mode. Such design is for the idea 
 *    on concerning the most compatible DT feature with old DT mode (like 
 *    falson). In this old mode, user have to handling the VLAN.untagbitmap 
 *    by themself for the SP-Tag untagging. 
 *  - When the preferred iDT_mode is set, all the SP-TAg untaging behavior  
 *    will be take over by default EVR entry(id=0), no VLAN.untagbitmap is 
 *    requirred. No matter packet is comming from normal ingress or CPU egress
 *    direct.
 *  - In bcm53115, on ingress side, both ISP and None-ISP port can recoganize 
 *    all tagging types of packet.
 *    (i.e. Untagged/Single SP_Tagged/Single C-Tagged/Double Tagged)
 *  - The rules about SP-Tag untagging behavior for egress are :
 *      1. Untag-In, Egress to ISP              : SingleOuterTag-Out
 *      2. Untag-IN, Egress to None-ISP         : Untag-Out
 *      3. SingleOuter-In, Egress to ISP        : SingleOuterTag-Out
 *      4. SingleOuter-In, Egress to None-ISP   : Untag-Out
 *      5. SingleInner-In, Egress to ISP        : DoubleTag-Out
 *      6. SingleInner-In, Egress to None-ISP   : SingleInner-Out
 *      7. DoubleTag-In, Egress to ISP          : DoubleTag-Out
 *      8. DoubleTag-In, Egress to None-ISP     : SingleInner-Out
 *
 *  [Valid Value] : 
 *      1. DRV_VLAN_FALCON_DT_MODE      : DT_Mode
 *      2. DRV_VLAN_INTELLIGENT_DT_MODE : iDT_Mode
 */
#define DRV_VLAN_PREFER_DT_MODE     DRV_VLAN_INTELLIGENT_DT_MODE

#define MIN_MII_PORT_STATE_PAGE 0x10
#define MAX_MII_PORT_STATE_PAGE 0x15

#define IS_MII_PORT_STATE_PAGE(page) \
    ((page >= MIN_MII_PORT_STATE_PAGE) && (page <= MAX_MII_PORT_STATE_PAGE)) ? \
    TRUE : FALSE
    
#define IS_PHY_STATUS_REGS(unit, addr) \
        ( (addr == SOC_REG_INFO(unit, LNKSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, LNKSTSCHGr).offset) || \
          (addr == SOC_REG_INFO(unit, SPDSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, DUPSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, PAUSESTSr).offset) ) ? \
          TRUE : FALSE
#endif
