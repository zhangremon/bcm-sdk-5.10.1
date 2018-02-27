/*
 * $Id: robo_53101.h 1.7 Broadcom SDK $
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
 
#ifndef _ROBO_53101_H
#define _ROBO_53101_H


#define DRV_MCAST_GROUP_NUM         256
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
#define DRV_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED)

#define DRV_MAC_LOW_POWER_SUPPORT_PBMP 0x0000001f

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

/* LED functions defintions (for LED_FUNC0_CTL) */
#define _ROBO53101_LED_FUNC_COL         (1 << 0)
#define _ROBO53101_LED_FUNC_SP_10       (2 << 1)
#define _ROBO53101_LED_FUNC_SP_100_200  (1 << 2)
#define _ROBO53101_LED_FUNC_SP_1G       (1 << 3)
#define _ROBO53101_LED_FUNC_DPX_COL     (1 << 4)
#define _ROBO53101_LED_FUNC_DPX         (1 << 5)
/* bit6 and bit5 reserved */
#define _ROBO53101_LED_FUNC_10_ACT      (1 << 8)
#define _ROBO53101_LED_FUNC_100_200_ACT (1 << 9)
#define _ROBO53101_LED_FUNC_1G_ACT      (1 << 10)
#define _ROBO53101_LED_FUNC_ACT         (1 << 11)
#define _ROBO53101_LED_FUNC_LNK_ACT_SP  (1 << 12)
#define _ROBO53101_LED_FUNC_LINK_ACT    (1 << 13)
#define _ROBO53101_LED_FUNC_LNK         (1 << 14)
#define _ROBO53101_LED_FUNC_AVB_LINK    (1 << 15)

#endif
