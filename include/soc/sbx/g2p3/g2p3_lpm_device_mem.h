/* -*- mode:c; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */

/******************************************************************************

  $Copyright: Copyright 2011 Broadcom Corporation.
  This program is the proprietary software of Broadcom Corporation
  and/or its licensors, and may only be used, duplicated, modified
  or distributed pursuant to the terms and conditions of a separate,
  written license agreement executed between you and Broadcom
  (an "Authorized License").  Except as set forth in an Authorized
  License, Broadcom grants no license (express or implied), right
  to use, or waiver of any kind with respect to the Software, and
  Broadcom expressly reserves all rights in and to the Software
  and all intellectual property rights therein.  IF YOU HAVE
  NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
  IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
  ALL USE OF THE SOFTWARE.  
   
  Except as expressly set forth in the Authorized License,
   
  1.     This program, including its structure, sequence and organization,
  constitutes the valuable trade secrets of Broadcom, and you shall use
  all reasonable efforts to protect the confidentiality thereof,
  and to use this information only in connection with your use of
  Broadcom integrated circuit products.
   
  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
  PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
  REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
  OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
  DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
  NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
  ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
  CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
  OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
  
  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
  BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
  ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
  TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
  THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
  ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$

  $Id: g2p3_lpm_device_mem.h 1.2.186.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_DEVICE_MEM_H_
#define _G2P3_LPM_DEVICE_MEM_H_

#include "sbTypes.h"
#include "g2p3_lpm_types.h"
#include "sbStatus.h"
#include "sbFeISupport.h"
#include "g2p3_lpm_host_mem.h"


/* DEVICE MEMORY MANAGER
 *
 * Allocates and deallocates areas of device SRAM
 */

/* abstract device memory handle */
typedef struct g2p3_lpmDeviceMem_s *g2p3_lpmDeviceMemHandle_t;

/* area of memory [start, start + nwords) */
typedef struct g2p3_lpmDeviceMemArea_s {
    g2p3_lpmSramAddr start;  /* start of exclusion area */
    size_t            nwords; /* number of contiguous words to exclude */
} g2p3_lpmDeviceMemArea_t, *g2p3_lpmDeviceMemArea_p_t;

/* requires: pDeviceMem non-null; hostMem initialized; nwords > 0;
 *           exclusions[] null or terminated by a {0,0} exclusion;
 *           forall e in exclusions[]: e.nwords > 0,
 *               e.start >= start, e.start + e.nwords <= start + nwords
 * modifies: *pDeviceMem; allocates using hostMem
 * effects:  creates new manager for SRAM for addresses [start, start + nwords)
 *           with unallocatable areas corresponding to exclusions[]
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmDeviceMemInit(g2p3_lpmDeviceMemHandle_t *pDeviceMem,
                          g2p3_lpmHostMemHandle_t hostMem,
                          g2p3_lpmSramAddr start, size_t nwords,
                          g2p3_lpmDeviceMemArea_t exclusions[],
                          sbBool_t ipv6);

/* requires: deviceMem initialized; pAddress non-null
 * modifies: *pAddress, *deviceMem
 * effects:  allocates a contiguous area of nwords addresses
 *           stores initial address in *pAddress
 * returns:  SB_OK on success
 *           SB_LPM_OUT_OF_DEVICE_MEMORY if allocation failed
 */
sbStatus_t
g2p3_lpmDeviceMemAlloc(g2p3_lpmDeviceMemHandle_t deviceMem,
                           g2p3_lpmSramAddr *pAddress, size_t nwords);

/* requires: deviceMem initialized; nwords previously successfully allocated
 *           at address using g2p3_lpmDeviceMemAlloc()
 * modifies: *deviceMem
 * effects:  frees previously allocated contiguous area of nwords at address
 * returns:  SB_OK on success
 *           SB_LPM_ADDRES_NOT_FOUND if address or nwords does not match
 */
sbStatus_t
g2p3_lpmDeviceMemFree(g2p3_lpmDeviceMemHandle_t deviceMem,
                          g2p3_lpmSramAddr address, size_t nwords);

/* requires: *pDeviceMem initialized
 * modifies: *pDeviceMem; deallocates in hostMem given at creation time
 * effects:  destroys and frees all memory used by manager at *pDeviceMem
 *           sets *pDeviceMem to null
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmDeviceMemUninit(g2p3_lpmDeviceMemHandle_t *pDeviceMem);

/* dumps statistics */
void g2p3_lpmDeviceMem_dump(g2p3_lpmDeviceMemHandle_t *pDeviceMem);
#endif
