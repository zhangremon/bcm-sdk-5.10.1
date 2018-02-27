/* 
 * $Id: l2cache.h 1.12 Broadcom SDK $
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
 * File:        l2cache.h
 * Purpose:     L2 Cache internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_L2_CACHE_H_
#define _BCM_INT_SBX_L2_CACHE_H_

#include <bcm/types.h>                             

/* This struct maintains info for l2 cache entry */
typedef struct {
    uint32  flags;
    uint32  cookie;     /* for L2CP, its l2cp index, else ptr to l2key */
    uint32  fte_idx;    /* FTE index */
    uint32  encap_id;   /* encapsulation index */
    uint32  src_port;   /* source port */

} _l2_cache_entry;

/* This struct maintains usage info for the l2cpmac table */
typedef struct {
    uint32  index;
    uint32  ref_cnt;
} _l2cpmac_entry;

typedef struct  {
  uint32 ulMac3;
  uint32 ulMac2;
  uint32 ulMac1;
  uint32 ulMac0;
  uint32 ulVlan;
  uint32 ulMac5;
  uint32 ulMac4;
  uint32 cookie;
} _l2_cache_key;


#define L2CACHE_ENTRY_VALID         0x00000001 /* entry is valid */
#define L2CACHE_ENTRY_L2CP          0x00000002 /* entry is L2CP */
#define L2CACHE_ENTRY_DMAC          0x00000004 /* entry is regular DMAC */
#define L2CACHE_ENTRY_FTE_VALID     0x00000008 /* entry has a valid FTE */
#define L2CACHE_ENTRY_ENCAP_VALID   0x00000010 /* entry has a valid encap id */
#define L2CACHE_ENTRY_ENCAP_UNKNOWN 0x00000020 /* entry does not know about 
                                                  this encap id. User value */
#define L2CACHE_ENTRY_COPY_TO_CPU   0x00000040 /* entry has copy to cpu set */
#define L2CACHE_ENTRY_BYPASS        0x00000080 /* no l2cp processing */
#define L2CACHE_ENTRY_DROP          0x00000100 /* drop action */
#define L2CACHE_ENTRY_FTE_MPLS_GPORT 0x00000200 /* fte points to mpls gport */
#define L2CACHE_ENTRY_MULTICAST     0x00000400 /* fte points to multicast Q */
#define L2CACHE_ENTRY_PREFIX5_LOOKUP 0x00000800 /* l2 lookup on 5 prefix bytes */
#define L2CACHE_ENTRY_SUBTYPE       0x00001000 /* entry has valid subtype */
#define L2CACHE_ENTRY_BPDU          0x00002000 /* entry has BPDU properties */
#ifdef BCM_WARM_BOOT_SUPPORT
/* Utilize unused bits of flags structure for warmboot information.
 * WB stores flags in 16-bit space.  Update if more flags are added.
 */
#define L2CACHE_WB_ENTRY_SUBTYPE_ALL 0x00004000
#define L2CACHE_WB_ENTRY_PORT_ALL    0x00008000
#endif

/* Check the first five bytes of DMAC */
#define L2CACHE_RESERVED_DMAC_CHECK(mac, result) \
do { \
    result = 1;\
    if (!(mac[0] == 0x01) || !(mac[1] == 0x80) || !(mac[2] == 0xc2) \
        || !(mac[3] == 0x0) || !(mac[4] == 0x0)) {\
        result = 0;\
    }\
} while (0)

/* Set the first five byes of DMAC to reserved address */
#define L2CACHE_RESERVED_DMAC_SET(mac) \
do { \
    mac[0] = 0x01; \
    mac[1] = 0x80; \
    mac[2] = 0xc2; \
    mac[3] = 0x0; \
    mac[4] = 0x0; \
} while (0)

#define L2CACHE_WARN(stuff)        BCM_DEBUG(BCM_DBG_WARN, stuff)
#define L2CACHE_ERR(stuff)         BCM_DEBUG(BCM_DBG_ERR, stuff)
#define L2CACHE_OUT(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_L2, stuff)
#define L2CACHE_VERB(stuff)        L2CACHE_OUT(BCM_DBG_VERBOSE, stuff)
#define L2CACHE_VVERB(stuff)       L2CACHE_OUT(BCM_DBG_VVERBOSE, stuff)

typedef struct {
    sal_mutex_t                 lock;
    shr_idxres_list_handle_t    idlist;
    uint32                      pass_thru_ohi;
    uint32                      cpu_fte;
    _l2_cache_entry             *l2_cache_entries;
    uint32                      _l2cpmac_table;
    uint32                      max_idx;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32               wb_hdl;
    uint32               scache_size;
#endif
}_l2_cache_t;

extern _l2_cache_t _l2_cache[BCM_MAX_NUM_UNITS];

#define L2CACHE_DEFAULT_OHI(unit)   (_l2_cache[unit].pass_thru_ohi)
#define L2CACHE_CPU_FTE(unit)       (_l2_cache[unit].cpu_fte)
#define L2CACHE_L2CPMAC_TABLE_ENTRY(unit, index) \
               (((_l2cpmac_entry *)(_l2_cache[unit]._l2cpmac_table)) + (index))

#define L2CACHE_MAX_IDX         (_l2_cache[unit].max_idx)

#define L2CACHE_ENTRY_GET_RV(unit, idx, entry, rv) \
do { \
    if (idx >= L2CACHE_MAX_IDX) { \
        rv = BCM_E_PARAM; \
    } \
    entry = (_l2_cache_entry *) (((uint32)_l2_cache[unit].l2_cache_entries) \
                                  + (sizeof(_l2_cache_entry)) * idx); \
    if (!entry) { \
        rv = BCM_E_PARAM; \
    } \
} while (0)

#define L2CACHE_ENTRY_GET(unit, idx, entry) \
do { \
    L2CACHE_ENTRY_GET_RV(unit, idx, entry, rv); \
    if (rv == BCM_E_PARAM) { \
        return rv; \
    } \
} while (0)

#define SB_COMMIT_COMPLETE   0xffffffff  /* Force completion on G2k call */

#define L2CACHE_L2CP_TYPES (256)
#define L2CACHE_L2CP_SUBTYPES (16)
#define L2CACHE_L2CP_SUBTYPES_OFFSET (SBX_MAX_PORTS * L2CACHE_L2CP_TYPES)

#define L2KEY_TO_MAC(key, mac) \
do { \
    mac[5] = key->ulMac0; \
    mac[4] = key->ulMac1; \
    mac[3] = key->ulMac2; \
    mac[2] = key->ulMac3; \
    mac[1] = key->ulMac4; \
    mac[0] = key->ulMac5; \
} while (0)

#define MAC_TO_L2KEY(mac, key) \
do { \
    key->ulMac0 = mac[5]; \
    key->ulMac1 = mac[4]; \
    key->ulMac2 = mac[3]; \
    key->ulMac3 = mac[2]; \
    key->ulMac4 = mac[1]; \
    key->ulMac5 = mac[0]; \
} while (0)

#define IS_PREFIX5_MASK(mask, result) \
do { \
    result = 0; \
    if ((mask[0] == 0xff) && (mask[1] == 0xff) && (mask[2] == 0xff) && \
        (mask[3] == 0xff) && (mask[4] == 0xff) && (mask[5] == 0)) { \
        result = 1; \
    } \
} while (0)

#define IS_NULL_MASK(mask, result) \
do { \
    result = 0; \
    if ((mask[0] == 0x0) && (mask[1] == 0x0) && (mask[2] == 0x0) && \
        (mask[3] == 0x0) && (mask[4] == 0x0) && (mask[5] == 0x0)) { \
        result = 1; \
    } \
} while (0)

#define IS_EXACT_MATCH_MASK(mask, result) \
do { \
    result = 0; \
    if ((mask[0] == 0xff) && (mask[1] == 0xff) && (mask[2] == 0xff) && \
        (mask[3] == 0xff) && (mask[4] == 0xff) && (mask[5] == 0xff)) { \
        result = 1; \
    } \
} while (0)


#endif /* _BCM_INT_SBX_L2_CACHE_H_ */
