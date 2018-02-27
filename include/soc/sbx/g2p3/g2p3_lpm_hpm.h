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

  $Id: g2p3_lpm_hpm.h 1.2.214.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_HPM_H_
#define _G2P3_LPM_HPM_H_

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/sbStatus.h>
#include "g2p3_lpm_host_mem.h"

/* HOST PAYLOAD MANAGER
 * 
 * Allocates and keeps host payloads
 * Payloads are copied at ref() time if they are not already in the HPM,
 * and deallocated when the reference count goes down to zero
 */

/* abstract host payload manager handle */
typedef struct g2p3_lpmHPM_s *g2p3_lpmHPMHandle_t;

/* abstract payload handle */
typedef struct g2p3_lpmPayload_s *g2p3_lpmPayloadHandle_t;

/* requires: pHpm non-null, hostMem initialized
 * modifies: *pHpm, allocates using hostMem
 * effects:  creates new manager for host payloads bytesPerPayload bytes each
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmHPMInit(g2p3_lpmHPMHandle_t *pHpm,
                    g2p3_lpmHostMemHandle_t hostMem,
                    size_t bytesPerPayload, sbBool_t ipv6);

/* requires: hpm initialized; *pPayload non-null
 * modifies: *hpm; allocates in hostMem given at creation time
 * effects:  references payload at *payloadData
 *           copies payload data if payload is not currently in HPM
 *           stores a handle to payload in *pPayload
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmHPMRef(g2p3_lpmHPMHandle_t hpm,
                   g2p3_lpmPayloadHandle_t *pPayload,
                   void *payloadData);

/* requires: payload allocated using g2p3_lpmHPMRef()
 *           resulting pointer must not be modified
 * modifies: nothing
 * effects:  none
 * returns:  pointer to internal payload data
 */
void *
g2p3_lpmHPMData(g2p3_lpmPayloadHandle_t payload);

/* requires: hpm initialized; payload allocated using g2p3_lpmHPMRef()
 * modifies: *hpm; deallocates in hostMem given at creation time
 * effects:  dereferences payload
 *           deallocates payload data storage if reference count reaches zero
 * returns:  SB_OK on success; error status otherwise
 */
sbStatus_t
g2p3_lpmHPMDeref(g2p3_lpmHPMHandle_t hpm,
                     g2p3_lpmPayloadHandle_t payload);

/* requires: *pHpm initialized
 * modifies: *pHpm; deallocates in hostMem given at creation time
 * effects:  destroys and frees all memory used by manager at *pHpm
 *           sets *pHpm to null
 * returns:  SB_OK on succss; error status otherwise
 */
sbStatus_t
g2p3_lpmHPMUninit(g2p3_lpmHPMHandle_t *pHpm);

#endif
