/*
 * $Id: robo_5348.h 1.11 Broadcom SDK $
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
 
#ifndef _ROBO_5348_H
#define _ROBO_5348_H



#define DRV_MCAST_GROUP_NUM         4096
#define DRV_AGE_TIMER_MAX               1048575
#define DRV_TRUNK_GROUP_NUM         6
#define DRV_TRUNK_MAX_PORT_NUM      8
#define DRV_MSTP_GROUP_NUM          256
#define DRV_SEC_MAC_NUM_PER_PORT    1   /* only one EAP_DA/port allowed */
#define DRV_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_AUTH_SUPPORT_PBMP       0xffffffff
#define DRV_AUTH_SUPPORT_PBMP_HI	0x001effff
#define DRV_RATE_CONTROL_SUPPORT_PBMP 0xffffffff
#define DRV_RATE_CONTROL_SUPPORT_PBMP_HI	0x001fffff
#define DRV_AUTH_SUPPORT_PBMP_BCM5347       0x1effffff
#define DRV_AUTH_SUPPORT_PBMP_HI_BCM5347	0x0
#define DRV_RATE_CONTROL_SUPPORT_PBMP_BCM5347 0x1fffffff
#define DRV_RATE_CONTROL_SUPPORT_PBMP_HI_BCM5347	0x0
#define DRV_VLAN_ENTRY_NUM  4095
#define DRV_BPDU_NUM    3
#define DRV_INTERNAL_MII_PBMP       0xff000000
#define DRV_INTERNAL_MII_PBMP_HI    0x0000ffff
#define DRV_INTERNAL_MII_PBMP_BCM5347       0xffffff
#define DRV_INTERNAL_MII_PBMP_HI_BCM5347    0x0
#define DRV_EXTERNAL_MII_PBMP       0xffffff
#define DRV_EXTERNAL_MII_PBMP_HI    0
#define DRV_CFP_TCAM_SIZE 128
#define DRV_CFP_UDFS_NUM 10
#define DRV_CFP_UDFS_OFFSET_MAX 80
#define DRV_CFP_RNG_NUM 1
#define DRV_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_VIOLATION_SA_NUM |\
                                        DRV_SECURITY_VIOLATION_SA_MATCH )


#define MIN_INTERNAL_MII_PORT_STATE_PAGE 0xa0
#define MAX_INTERNAL_MII_PORT_STATE_PAGE 0xb7
#define MIN_EXTERNAL_MII_PORT_STATE_PAGE 0xc0
#define MAX_EXTERNAL_MII_PORT_STATE_PAGE 0xdc

#define IS_MII_PORT_STATE_PAGE(page) \
    ((page >= MIN_INTERNAL_MII_PORT_STATE_PAGE) && (page <= MAX_INTERNAL_MII_PORT_STATE_PAGE) || \
     (page >= MIN_EXTERNAL_MII_PORT_STATE_PAGE) && (page <= MAX_EXTERNAL_MII_PORT_STATE_PAGE)) ? \
    TRUE : FALSE
    
#define IS_PHY_STATUS_REGS(unit, addr) \
        ( (addr == SOC_REG_INFO(unit, LNKSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, LNKSTSCHGr).offset) || \
          (addr == SOC_REG_INFO(unit, SPDSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, DUPSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, PAUSESTSr).offset) ) ? \
          TRUE : FALSE

#endif
