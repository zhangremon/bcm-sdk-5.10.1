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

  $Id: g2p3_lpm_types.h 1.3.218.1 Broadcom SDK $

 *****************************************************************************/

#ifndef _G2P3_LPM_TYPES_H_
#define _G2P3_LPM_TYPES_H_

#include <soc/sbx/sbTypes.h>
#include <soc/sbx/sbStatus.h>

typedef uint32_t g2p3_lpmSramAddr;
typedef uint32_t g2p3_lpmClassifierId_t;
typedef void (*g2p3_lpmDmaSlabRequest_f_t) (void); 
typedef uint32_t sbDmaAddress_t;


/* PAYLOAD PACKING FUNCTION:
 * packs a <host payload, clsA, clsB> triplet into device representation */

/* requires: payload, dest, destExtra non-null
 *           dest points to buffer sufficiently large for device payload
 * modifies: *dest, *destExtra
 * effects:  packs host payload representation triple <payload, clsA, clsB>
 *           into device memory representation
 *           *dest is the payload area
 *           *destExtra is ten bits in a leaf node; 0 <= *destExtra <= 0x3ff
 * returns:  SB_OK on success; error status otherwise
 */
typedef sbStatus_t
(*g2p3_lpmPayloadPack_f_t)(void *dest,
                               uint16_t *destExtra,
                               void *payload,
                               g2p3_lpmClassifierId_t clsA,
                               g2p3_lpmClassifierId_t clsB,
                               uint32_t dropMaskLo, 
                               uint32_t dropMaskHi,
                               uint32_t unit);

/* generic continuation callback function: takes application-specific data,
 * and the status of the previous operation */
typedef void
(*g2p3_lpmCallback_f_t)(void *userData, sbStatus_t status);

#endif
