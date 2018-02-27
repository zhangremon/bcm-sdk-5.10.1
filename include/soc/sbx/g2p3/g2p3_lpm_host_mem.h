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

  $Id: g2p3_lpm_host_mem.h 1.2.110.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_HOST_MEM_H_
#define _G2P3_LPM_HOST_MEM_H_

#include "sbTypes.h"
#include "sbTypesGlue.h"
#include "sbStatus.h"
#include "sbFeISupport.h"

/* HOST MEMORY MANAGER
 *
 * Allocates and deallocates non-DMAble host memory at runtime.
 * Required because malloc() and free() may only be called
 * at init and uninit time, respectively.
 * All memory is allocatedd at init time and deallocated at uninit time via
 * functions provided to g2p3_lpmHostMemInit(); memory is never
 * deallocated if the deallocating function provided is null.
 */ 

/* abstract host memory handle */
typedef struct g2p3_lpmHostMem_s *g2p3_lpmHostMemHandle_t;

/* requires: pHostMem non-null; nbytes > 0; hostAlloc() non-null
 * modifies: *pHostMem; allocates using hostAlloc()
 * effects:  allocates a host memory area of nbytes using hostAlloc()
 *           and creates new host memory manager for it
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmHostMemInit(g2p3_lpmHostMemHandle_t *pHostMem,
                        size_t nbytes,
                        sbMalloc_f_t hostAlloc, void *hostAllocToken,
                        sbFree_f_t hostFree, void *hostFreeToken);

/* requires: hostMem initialized; pPointer non-null; nbytes > 0
 * modifies: *hostMem
 * effects:  allocates a contiguous area of nbytes bytes
 *           stores pointer to start of area in *pPointer
 * returns:  SB_OK on success
 *           SB_LPM_OUT_OF_HOST_MEMORY if allocation failed
 */
sbStatus_t
g2p3_lpmHostMemAlloc(g2p3_lpmHostMemHandle_t hostMem,
                         void **pPointer, size_t nbytes, char*owner);

/* requires: hostMem initialized; nbytes previously successfully allocated
 *           at pointer using g2p3_lpmHostMemAlloc()
 * modifies: *hostMem
 * effects:  frees previously allocated contiguous memory area at pointer
 * returns:  SB_OK on success
 *           SB_LPM_ADDRES_NOT_FOUND if address or nwords does not match
 */
sbStatus_t
g2p3_lpmHostMemFree(g2p3_lpmHostMemHandle_t hostMem,
                        void *pointer, size_t nbytes);

/* requires: *pHostMem initialized
 * modifies: *pHostMem; deallocates using hostFree() given at creation time
 *           if hostFree non-null
 * effects:  destroys and frees all memory used by manager at *pHostMem
 * returns:  SB_OK on success; error otherwise
 */
sbStatus_t
g2p3_lpmHostMemUninit(g2p3_lpmHostMemHandle_t *pHostMem);

#endif
