/*
 * $Id: qe2000_util.h 1.3 Broadcom SDK $
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

#ifndef _QE2000_UTIL_H
#define _QE2000_UTIL_H

/*#include "sbTypes.h" */
#include "glue.h"

#include "fabric/sbZfHwQe2000QsPriLutEntry.hx"


#define HW_QE2000_MAX_NUM_PORTS_K 50

/*
 * Egress Buffer Memory
 */
#define HW_QE2000_EB_MEM_MAX_OFFSET          0x7FFF
#define HW_QE2000_EB_MVT_ROW_V               (12)

#define HW_QE2000_EB_MVT_ROW_GET(_index, _mcgrpsz)    ( HW_QE2000_EB_MEM_MAX_OFFSET - \
                                                    ( _index & (( 1 << ( HW_QE2000_EB_MVT_ROW_V + _mcgrpsz ))-1)) )

#define HW_QE2000_EB_MVT_COL_GET(_index, _mcgrpsz)    ( (_index >> ( HW_QE2000_EB_MVT_ROW_V + _mcgrpsz )) & 0x3)

#define HW_QE2000_EB_MVT_MIN(_mcgrpsz)                ( (1 << (HW_QE2000_EB_MVT_ROW_V + _mcgrpsz)) * 3)

/**
 * Multicast Vector Table (MVT) Entry
 *
 *
 */
typedef struct hw_qe2000_mvt_entry_s {
    /* 22087: note: field below should be sbBool_t.  Changed to unit32_t for socket transport */
    uint32_t bPortEnable[ HW_QE2000_MAX_NUM_PORTS_K ]; /**< Port Enable array - TRUE means multicast to this port. */
    uint32_t ulMvtdA;       /**< MVTD_A value - This is a 14-bit cookie that is available to the EP. */
    uint32_t ulMvtdB;       /**< MVTD_B value - this is a 4-bit cookie that is available to the EP. */
    /* 22087: note: field below should be sbBool_t.  Changed to unit32_t for socket transport */
    uint32_t bSourceKnockout; /**< Enable Source Knockout for this entry */
    uint32_t ulNext;        /**< 16-bit Offset to the next MVT 0xFFFF terminates list */
} HW_QE2000_MVT_ENTRY_ST, *HW_QE2000_MVT_ENTRY_PST;

/**
 * semaphore try & wait function pointer
 *
 * Function pointer to user supplied semaphore try & wait function.
 * Takes semaphore ID (int) and timeout in seconds.
 * Returns 0 on OK, some other value for failure.
 */
typedef int (*HW_QE2000_SEM_TRYWAIT_PF)(int nSemId, void* pUserSemData, int nTimeOut);

/**
 * semaphore give function pointer
 *
 * Function pointer to user supplied semaphore give function.
 * Takes semaphore ID (int).
 * Returns 0 on OK, some other valure for failure.
 */
typedef int (*HW_QE2000_SEM_GIVE_PF)(int nSemId, void* pUserSemData);

/**
 * Get an Multicast Vector Table Entry
 *
 *
 * @param Handle       A valid handle
 * @param pEntry       The MVT entry stored at the given index.  Upon success, the MVT
 *                     entry will be placed at this location.
 * @param uIndex       The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param uTimeOut     Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
int hwQe2000MVTGet( sbhandle                 userDeviceHandle,
		    HW_QE2000_MVT_ENTRY_PST  pEntry,
		    uint32_t                 uIndex,
		    HW_QE2000_SEM_TRYWAIT_PF pfSemTryWait,
		    HW_QE2000_SEM_GIVE_PF    pfSemGive,
		    int                      nSemId,
		    uint32_t                 uTimeOut,
		    void                     *pUserData);

/**
 * Set an Multicast Vector Table Entry
 *
 *
 * @param Handle     A valid device handle.
 *
 * @param Entry        The MVT entry to be placed in the table.
 * @param uIndex       The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param uTimeOut     Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 indicates success.  Non-zero indicates failure.
 */
int hwQe2000MVTSet( sbhandle                 userDeviceHandle,
			HW_QE2000_MVT_ENTRY_ST   Entry,
			uint32_t                 uIndex,
			HW_QE2000_SEM_TRYWAIT_PF pfSemTryWait,
			HW_QE2000_SEM_GIVE_PF    pfSemGive,
			int                      nSemId,
			uint32_t                 uTimeOut,
			void                     *pUserData);


#endif /* _QE2000_UTIL_H */
