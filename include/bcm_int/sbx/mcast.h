/*
 * $Id: mcast.h 1.6 Broadcom SDK $
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

#ifndef _BCM_INT_SBX_MCAST_H_
#define _BCM_INT_SBX_MCAST_H_

#include <bcm/types.h>
#include <bcm/mcast.h>

/*
 *  This section is configuration for the module.
 *
 *  MCAST_NO_MVT_LOCAL, if zero, allows the mcast module to assume it owns
 *  MVTEs in the range MCAST_MVT_BASE through MCAST_MVT_BASE +
 *  MCAST_ENTRY_COUNT - 1. It will then manage the MVTEs locally (not through
 *  the QE.  If this is set to one, however, any call that either explicitly or
 *  implicitly requires mcast to allocate an MVT will make a call to the
 *  qe2000_mvt allocation function.
 *
 *  MCAST_NO_FT_LOCAL, if zero allows the mcast module to assume it owns FTEs
 *  in the range MCAST_FT_BASE through MCAST_FT_BASE + MCAST_ENTRY_COUNT - 1.
 *  It will then manage the FTEs locally.  If this is nonzero, it will use the
 *  allocator function to manage the FTEs.  Note that when nonzero, the init
 *  function does not know which L2 entries that have multicast addresses
 *  belong to this module, so init will obliterate all of them (where it will
 *  only remove the addresses that belong to this module when zero).
 */
#define MCAST_NO_MVT_LOCAL       1 /* nonzero to disable local MVT alloc/free */
#define MCAST_NO_FT_LOCAL        1 /* nonzero to disable local FT alloc/free */
#define MCAST_FT_INVALID_EXC  0x7F /* exception code for invalid FT */

/*
 *  Externally, we also expect additional configuration defines...
 *
 *  BCM_SBX_MVTHELPER, if nonzero, will use the MVT helper functions to set up
 *  the MVTs from here.  If zero, the caller must have set up an MVT before
 *  calling these functions, and must provide the MVT index as the l2mc_index.
 */
#define BCM_SBX_MVTHELPER 0

/*
 *  Private functions exported from SBX MCast code to other SBX      .
 *  modules.. These should never be called except from the designated.
 *  unit type                                                        .
 */

#define MCAST_QID_BASE           SBX_MC_QID_BASE /* Starting QID */
#if (0 == (MCAST_NO_FT_LOCAL && MCAST_NO_MVT_LOCAL))
#define MCAST_ENTRY_COUNT        4096 /* number of L2 multicast entries */
#endif /* (0 == (MCAST_NO_FT_LOCAL || MCAST_NO_MVT_LOCAL)) */
#if (0 == MCAST_NO_FT_LOCAL)
#define MCAST_FT_BASE            8192 /* base FTE index */
#endif /* (0 == MCAST_NO_FT_LOCAL) */
#if (0 == MCAST_NO_MVT_LOCAL)
#define MCAST_MVT_BASE           4096 /* base MVTindex/mcGroup */
#endif /* (0 == MCAST_NO_MVT_LOCAL) */

/*
 *  These are the spec_flags for the function below (yes, it's a bitmap).
 *
 *  ...AUTO_L2MCIDX = add function chooses l2mc_index
 *  ...SPEC_L2MCIDX = add function tries to use provided l2mc_index
 *  ...INCLIDE_ARG = add function includes all_routers group ports
 *  ...INHIBIT_ARG = add function does not include all_routers group ports
 */
#define BCM_FE2000_MCAST_ADD_AUTO_L2MCIDX    0x00000000
#define BCM_FE2000_MCAST_ADD_SPEC_L2MCIDX    0x00000001
#define BCM_FE2000_MCAST_ADD_INCLUDE_ARG     0x00000000
#define BCM_FE2000_MCAST_ADD_INHIBIT_ARG     0x00000002

/*
 *   Function
 *      _bcm_fe2000_mcast_addr_add
 *   Purpose
 *      Adds a multicast address to the hardware tables
 *   Parameters
 *      (in) int unit = unit number
 *      (in) bcm_mcast_addr_t *mcAddr = information about the address to add
 *      (in) uint32 flags = BCM_L2_* flags from early in l2.h
 *      (out) uint32 *new_ftIndex = where to put the resulting ftIndex
 *      (out) uint32 *new_mcGroup = where to put the resulting mcGroup
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Flags currently honoured:
 *          BCM_L2_DISCARD_DST
 *          BCM_L2_COPY_TO_CPU
 *          BCM_L2_STATIC
 *      The spec_flags affect how the operation is performed (see above).
 *      Set new_ftIndex to NULL if you don't want it.
 *      Set new_mcGroup to NULL if you don't want it.
 */
extern int
_bcm_fe2000_mcast_addr_add(int unit,
                           bcm_mcast_addr_t *mcAddr,
                           uint32 flags,
                           uint32 spec_flags,
                           uint32 *new_ftIndex,
                           uint32 *new_mcGroup);

#endif  /* _BCM_INT_SBX_MCAST_H_ */
