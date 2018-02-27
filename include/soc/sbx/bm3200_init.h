/*
 * $Id: bm3200_init.h 1.6 Broadcom SDK $
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
 * ============================================================
 * == bm3200_init.h - BM3200 Initialization defines          ==
 * ============================================================
 */

#ifndef _BM3200_INIT_H
#define _BM3200_INIT_H

#include "sbTypesGlue.h"

/*
** BM3200 Initialization Structure
*/
typedef struct HW_BM3200_INIT_s {
    sbhandle      userDeviceHandle;       /* sbhandle */
    uint32_t      reset;                  /* reset BM3200 device if non-zero */
    uint32_t      bBmRunSelfTest;          /* run BM3200 device selftest if non-zero */

    /* BM3200 Parameters */
    uint32_t      bmModeLatency_ul;       /* bm3200 only */
    uint32_t      bmSerializerMask_ul;    /* mask of bm serializers used */
    uint32_t      bmLocalBmId_ul;         /* BM3200 identifier for failover */
    uint32_t      bmDefaultBmId_ul;       /* active BM3200 identifier for failover */
    uint32_t      bmCmode_ul;             /* if set, indicates cmode operation */
    uint32_t      bmEpochLength_ul;
    uint32_t      bmTimeslotSizeInNs_ul;
    uint32_t      bmEnableAutoFailover;   /* enable failover */
    uint32_t      bmEnableAutoLinkDisable;/* enable auto switchover */
    uint32_t      bmMaxFailedLinks;       /* max failed links */
    uint32_t      spMode;

    /* SE4000 Parameters */
    uint64_t      seSerializerMask_ull;   /* mask of se serializers used */

    /* LCM4000 Parameters */

#define HW_BM3200_PT_NUM_SERIALIZERS           (40)
#define HW_BM3200_MAX_NUM_PLANES                (2)

    uint32_t      lcmXcfgABInputPolarityReversed_ul; /* indicates that xcfg_switch_a_b_n polarity is reversed from the QE */
    uint64_t      lcmSerializerMask_ull;  /* mask of lcm serializers used */
    uint32_t      lcmXcfg_ul[HW_BM3200_MAX_NUM_PLANES][HW_BM3200_PT_NUM_SERIALIZERS];
    uint32_t      lcmPlaneValid_ul[HW_BM3200_MAX_NUM_PLANES];
    uint32_t      siLsThreshold_ul;
    uint32_t      siLsWindow_ul;

} HW_BM3200_INIT_ST;

uint32_t  hwBm3200Init(HW_BM3200_INIT_ST *hwBm3200Init_sp);

/* status */
#define HW_BM3200_STATUS_OK_K                            (0)
#define HW_BM3200_STATUS_INIT_BM3200_BAD_CHIP_REV_K      (1)
#define HW_BM3200_STATUS_INIT_BIST_TIMEOUT_K             (2)
#define HW_BM3200_STATUS_INIT_BM3200_BIST_BW_UNREPAIR_K  (3)
#define HW_BM3200_STATUS_INIT_BM3200_BIST_BW_TIMEOUT_K   (4)
#define HW_BM3200_STATUS_INDIRECT_ACCESS_TIMEOUT_K       (5)
#define HW_BM3200_STATUS_INIT_BM3200_SER_TIMEOUT_K       (6)
#define HW_BM3200_STATUS_INIT_BM3200_BW_TIMEOUT_K        (7)
#define HW_BM3200_STATUS_INIT_EPOCH_LENGTH_INVALID_K     (8)
#define HW_BM3200_STATUS_INIT_PARAM_ERROR_K              (9)
#define HW_BM3200_STATUS_INIT_SCHED_ACTIVE_TIMEOUT_K     (10)

/* Fix for bug 23436, resolution was wrong */
#define HW_BM3200_10_USEC_K       (10000)
#define HW_BM3200_100_USEC_K     (100000)
#define HW_BM3200_1_MSEC_K      (1000000)
#define HW_BM3200_10_MSEC_K    (10000000)
#define HW_BM3200_100_MSEC_K  (100000000)
#define HW_BM3200_500_MSEC_K  (500000000)
#define HW_BM3200_1_SEC_K    (1000000000)

/* These defines are the timeouts for acks on all BM3200 init code */
/* these can be updated to increase timeout.                       */
#define HW_BM3200_TIMEOUT_GENERAL           5
#define HW_BM3200_POLL_GENERAL              10000

#define HW_BM3200_PT_NUM_NODES                 (32)
#define HW_BM3200_PT_ID                    (0x0280)
#define HW_BM3200_PT_REV0                    (0x00)
#define HW_BM3200_PT_REV1                    (0x01)

#define HW_BM3200_PT_RANDOM_ARRAY_SIZE         (55)
#define HW_BM3200_CLOCK_SPEED_IN_HZ     (250000000)
#define HW_BM3200_STARTUP_TIMESLOT_SIZE_IN_NS (760)

#define HW_BM3200_PT_EMAP_MEM_SIZE             (32)

#define HW_BM3200_PT_MAX_CMODE_VIRTUAL_PORTS    (640)
#define HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS    (4096)
#define HW_BM3200_PT_MAX_CMODE_QUEUES   (1024)
#define HW_BM3200_PT_MAX_DMODE_QUEUES   (16384)

#define HW_BM3200_PT_MAX_ESETS (128)

#define HW_BM3200_NULL_CYCLE_COUNT             (4) /* conservative value */

#endif /* _BM3200_INIT_H */
