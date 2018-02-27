/*
 * $Id: bm9600_properties.h 1.4 Broadcom SDK $
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
 */
#ifndef BM9600_ZPOLARISPROPERTIES_H
#define BM9600_ZPOLARISPROPERTIES_H

/* $Id: bm9600_properties.h,v 1.4 2008/10/27 23:04:17 satapati Exp $ */
/**
 * @file
 * @brief This file will contain all the defines for the Polaris chip.  This file should 
 *        only contain defines.  There shall be no includes, class forwarding, enumerations
 *        or c++ specific keywords (e.g. const).
 * @version 0.0
 */

#define BM9600_NUM_LINKS (96)
#define BM9600_NUM_NODES (72)
#define BM9600_CHANNELS_PER_LINK (2)
#define BM9600_NUM_TIMESLOTSIZE (32)

#define BM9600_QE_KA_LINE_RATE_GBPS (20)
#define BM9600_QE_SS_LINE_RATE_GBPS (50)
#define BM9600_QE_NODE_NUM_LINKS 20

#define BM9600_QE_NODE_NUM_SFI_LINKS (24)
#define BM9600_QE_NODE_NUM_SCI_LINKS (2)


#define BM9600_MAX_NUM_LOGICAL_XBARS (24)
#define BM9600_MAX_LOGICAL_XBAR_MASK (0xFFFFFF)
#define BM9600_MAX_NUM_PHYSICAL_XBARS_PER_PLANE (24) 
#define BM9600_MAX_NUM_PHYSICAL_BM_PER_PLANE (1)
#define BM9600_MAX_NUM_PLANES (2)
#define BM9600_PRIMARY_PLANE (0)
#define BM9600_SECONDARY_PLANE (1)

#define BM9600_BW_WRED_DP_NUM (3)
#define BM9600_BW_WRED_CURVE_TABLE_SIZE (256)
#define BM9600_BW_WRED_ROWS_PER_TABLE_ENTRY (8)
#define BM9600_BW_REPOSITORY_SIZE (0x7000)
#define BM9600_BW_AI_CHAIN_NUM (3)
#define BM9600_BW_MAX_BAG_NUM (0x1000)
#define BM9600_BW_MAX_VOQ_NUM (0x4000)
#define BM9600_BW_MAX_VOQS_IN_BAG (16)

#define BM9600_ARB_CLOCK_PERIOD_IN_NS (5)

#define BM9600_NUMBER_OF_SI       (BM9600_NUM_LINKS)
#define BM9600_NUMBER_OF_AI_INA   (BM9600_NUM_NODES)
#define BM9600_NUMBER_OF_HC 24

#define BM9600_DEFAULT_BASE_ADDRESS  (0x80000000)
#define BM9600_DEFAULT_REGMAP_OFFSET (0x00100000)


#define NODE_GROUP_REGMAP_BASE_ADDRESS (0x90000000)
#define NODE_GROUP_REGMAP_OFFSET       (0x00020000)

#define BM9600_BROADCAST_OFFSET          (127)

#define BM9600_DEFAULT_TIMESLOT_SIZE (760)

#define BM9600_MAX_NUM_SYSPORTS 2816
#define BM9600_MAX_MULTICAST_EXTENDED_ESETS 1024
#define BM9600_MAX_MULTICAST_ESETS 128
#define BM9600_MAX_PORTSETS 176

#define BM9600_BW_CLOCK_PERIOD_200MHZ 5

#define BM9600_ESET_PER_ESET_ENTRY 16

#define BM9600_FO_NUM_FORCE_LINK_SELECT (6)
#define BM9600_FO_NUM_FORCE_LINK_ACTIVE (4)

#define BM9600_NUM_CLKS_PER_GENERATOR 10

#define BM9600_SS_CHANNELS_PER_LINK (2)
#define BM9600_SS_SERX_CLK_PERIOD_PS 1600
#define BM9600_KA_CHANNELS_PER_LINK (1)
#define BM9600_KA_SERX_CLK_PERIOD_PS 3200

#endif /* BM9600_ZPOLARISPROPERTIES_H */
