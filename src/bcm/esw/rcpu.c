/*
 * $Id: rcpu.c 1.83.6.6 Broadcom SDK $
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
 * Remote CPU module for XGS devices supporting EB mode.
 *
 */


/* We need to call top-level APIs on other units */
#ifdef BCM_HIDE_DISPATCHABLE
#undef BCM_HIDE_DISPATCHABLE
#endif

#include <shared/alloc.h>
#include <sal/core/libc.h>
#include <sal/core/sync.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/mem.h>
#include <soc/pbsmh.h>
#include <soc/higig.h>

#include <bcm/pkt.h>
#include <bcm/rx.h>
#include <bcm/tx.h>

#include <bcm_int/control.h>
#include <bcm_int/esw/rcpu.h>
#include <bcm_int/esw/rx.h>
#include <bcm_int/esw/trunk.h>
#include <bcm_int/esw/tx.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/api_xlate_port.h>
#include <bcm_int/esw_dispatch.h>

#if defined(INCLUDE_RCPU) && defined(BCM_XGS3_SWITCH_SUPPORT)

#if defined(BCM_OOB_RCPU_SUPPORT)
#include <drv/eth/eth_drv.h>
#endif /* BCM_OOB_RCPU_SUPPORT */

#define RCPU_HDR_F_SOBMH     0x01
#define RCPU_HDR_F_HIGIG     0x02

#define RCPU_COPY_HDR(b, s, l, inc_buf_ptr)     \
           sal_memcpy((uint8*)(b), (s), (l));   \
           if ((inc_buf_ptr)) {                 \
                (b) += (l);             \
            }

#define RCPU_CLR_HDR(b, l, inc_buf_ptr)         \
           sal_memset((uint8*)(b), 0, (l));     \
           if ((inc_buf_ptr)) {                 \
               (b) += (l);              \
            }

#ifdef BCM_XGS3_SWITCH_SUPPORT
/* Lookup table: Return the first bit set. -1 if no bit set. */
static int8 bpos[256] = {
 /* 0x00 */ -1, /* 0x01 */  0, /* 0x02 */  1, /* 0x03 */  0,
 /* 0x04 */  2, /* 0x05 */  0, /* 0x06 */  1, /* 0x07 */  0,
 /* 0x08 */  3, /* 0x09 */  0, /* 0x0a */  1, /* 0x0b */  0,
 /* 0x0c */  2, /* 0x0d */  0, /* 0x0e */  1, /* 0x0f */  0,
 /* 0x10 */  4, /* 0x11 */  0, /* 0x12 */  1, /* 0x13 */  0,
 /* 0x14 */  2, /* 0x15 */  0, /* 0x16 */  1, /* 0x17 */  0,
 /* 0x18 */  3, /* 0x19 */  0, /* 0x1a */  1, /* 0x1b */  0,
 /* 0x1c */  2, /* 0x1d */  0, /* 0x1e */  1, /* 0x1f */  0,
 /* 0x20 */  5, /* 0x21 */  0, /* 0x22 */  1, /* 0x23 */  0,
 /* 0x24 */  2, /* 0x25 */  0, /* 0x26 */  1, /* 0x27 */  0,
 /* 0x28 */  3, /* 0x29 */  0, /* 0x2a */  1, /* 0x2b */  0,
 /* 0x2c */  2, /* 0x2d */  0, /* 0x2e */  1, /* 0x2f */  0,
 /* 0x30 */  4, /* 0x31 */  0, /* 0x32 */  1, /* 0x33 */  0,
 /* 0x34 */  2, /* 0x35 */  0, /* 0x36 */  1, /* 0x37 */  0,
 /* 0x38 */  3, /* 0x39 */  0, /* 0x3a */  1, /* 0x3b */  0,
 /* 0x3c */  2, /* 0x3d */  0, /* 0x3e */  1, /* 0x3f */  0,
 /* 0x40 */  6, /* 0x41 */  0, /* 0x42 */  1, /* 0x43 */  0,
 /* 0x44 */  2, /* 0x45 */  0, /* 0x46 */  1, /* 0x47 */  0,
 /* 0x48 */  3, /* 0x49 */  0, /* 0x4a */  1, /* 0x4b */  0,
 /* 0x4c */  2, /* 0x4d */  0, /* 0x4e */  1, /* 0x4f */  0,
 /* 0x50 */  4, /* 0x51 */  0, /* 0x52 */  1, /* 0x53 */  0,
 /* 0x54 */  2, /* 0x55 */  0, /* 0x56 */  1, /* 0x57 */  0,
 /* 0x58 */  3, /* 0x59 */  0, /* 0x5a */  1, /* 0x5b */  0,
 /* 0x5c */  2, /* 0x5d */  0, /* 0x5e */  1, /* 0x5f */  0,
 /* 0x60 */  5, /* 0x61 */  0, /* 0x62 */  1, /* 0x63 */  0,
 /* 0x64 */  2, /* 0x65 */  0, /* 0x66 */  1, /* 0x67 */  0,
 /* 0x68 */  3, /* 0x69 */  0, /* 0x6a */  1, /* 0x6b */  0,
 /* 0x6c */  2, /* 0x6d */  0, /* 0x6e */  1, /* 0x6f */  0,
 /* 0x70 */  4, /* 0x71 */  0, /* 0x72 */  1, /* 0x73 */  0,
 /* 0x74 */  2, /* 0x75 */  0, /* 0x76 */  1, /* 0x77 */  0,
 /* 0x78 */  3, /* 0x79 */  0, /* 0x7a */  1, /* 0x7b */  0,
 /* 0x7c */  2, /* 0x7d */  0, /* 0x7e */  1, /* 0x7f */  0,
 /* 0x80 */  7, /* 0x81 */  0, /* 0x82 */  1, /* 0x83 */  0,
 /* 0x84 */  2, /* 0x85 */  0, /* 0x86 */  1, /* 0x87 */  0,
 /* 0x88 */  3, /* 0x89 */  0, /* 0x8a */  1, /* 0x8b */  0,
 /* 0x8c */  2, /* 0x8d */  0, /* 0x8e */  1, /* 0x8f */  0,
 /* 0x90 */  4, /* 0x91 */  0, /* 0x92 */  1, /* 0x93 */  0,
 /* 0x94 */  2, /* 0x95 */  0, /* 0x96 */  1, /* 0x97 */  0,
 /* 0x98 */  3, /* 0x99 */  0, /* 0x9a */  1, /* 0x9b */  0,
 /* 0x9c */  2, /* 0x9d */  0, /* 0x9e */  1, /* 0x9f */  0,
 /* 0xa0 */  5, /* 0xa1 */  0, /* 0xa2 */  1, /* 0xa3 */  0,
 /* 0xa4 */  2, /* 0xa5 */  0, /* 0xa6 */  1, /* 0xa7 */  0,
 /* 0xa8 */  3, /* 0xa9 */  0, /* 0xaa */  1, /* 0xab */  0,
 /* 0xac */  2, /* 0xad */  0, /* 0xae */  1, /* 0xaf */  0,
 /* 0xb0 */  4, /* 0xb1 */  0, /* 0xb2 */  1, /* 0xb3 */  0,
 /* 0xb4 */  2, /* 0xb5 */  0, /* 0xb6 */  1, /* 0xb7 */  0,
 /* 0xb8 */  3, /* 0xb9 */  0, /* 0xba */  1, /* 0xbb */  0,
 /* 0xbc */  2, /* 0xbd */  0, /* 0xbe */  1, /* 0xbf */  0,
 /* 0xc0 */  6, /* 0xc1 */  0, /* 0xc2 */  1, /* 0xc3 */  0,
 /* 0xc4 */  2, /* 0xc5 */  0, /* 0xc6 */  1, /* 0xc7 */  0,
 /* 0xc8 */  3, /* 0xc9 */  0, /* 0xca */  1, /* 0xcb */  0,
 /* 0xcc */  2, /* 0xcd */  0, /* 0xce */  1, /* 0xcf */  0,
 /* 0xd0 */  4, /* 0xd1 */  0, /* 0xd2 */  1, /* 0xd3 */  0,
 /* 0xd4 */  2, /* 0xd5 */  0, /* 0xd6 */  1, /* 0xd7 */  0,
 /* 0xd8 */  3, /* 0xd9 */  0, /* 0xda */  1, /* 0xdb */  0,
 /* 0xdc */  2, /* 0xdd */  0, /* 0xde */  1, /* 0xdf */  0,
 /* 0xe0 */  5, /* 0xe1 */  0, /* 0xe2 */  1, /* 0xe3 */  0,
 /* 0xe4 */  2, /* 0xe5 */  0, /* 0xe6 */  1, /* 0xe7 */  0,
 /* 0xe8 */  3, /* 0xe9 */  0, /* 0xea */  1, /* 0xeb */  0,
 /* 0xec */  2, /* 0xed */  0, /* 0xee */  1, /* 0xef */  0,
 /* 0xf0 */  4, /* 0xf1 */  0, /* 0xf2 */  1, /* 0xf3 */  0,
 /* 0xf4 */  2, /* 0xf5 */  0, /* 0xf6 */  1, /* 0xf7 */  0,
 /* 0xf8 */  3, /* 0xf9 */  0, /* 0xfa */  1, /* 0xfb */  0,
 /* 0xfc */  2, /* 0xfd */  0, /* 0xfe */  1, /* 0xff */  0,
};

/*
 * Extract port from PBM
 */
#define _pbm2port(bmp) \
    ((bpos[(bmp >>  0) & 0xFF] != -1) ? (0  + bpos[(bmp >>  0) & 0xFF]) : \
     (bpos[(bmp >>  8) & 0xFF] != -1) ? (8  + bpos[(bmp >>  8) & 0xFF]) : \
     (bpos[(bmp >> 16) & 0xFF] != -1) ? (16 + bpos[(bmp >> 16) & 0xFF]) : \
     (bpos[(bmp >> 24) & 0xFF] != -1) ? (24 + bpos[(bmp >> 24) & 0xFF]) : \
     (-1) \
    )

#endif

/********** global variables ****************/

_bcm_rcpu_control_t      *_bcm_rcpu_control[BCM_RCPU_MAX_UNITS];

STATIC bcm_mac_t    rcpu_default_mac = {0x00, 0x00, 0x11, 0x22, 0x33, 0x00};
STATIC bcm_mac_t    src_cpu_default_mac = {0x00, 0xaa, 0xbb, 0x22, 0x33, 0x00};


/************** local functions *****************************/
STATIC uint8
_bcm_rcpu_ctoi(const char *s)
{
    uint32 n = 0, base = 16;

    if (*s == '0') {
        s++;
    }

    for (n = 0; ((*s >= 'a' && *s <= 'z') ||
                 (*s >= 'A' && *s <= 'Z') ||
                 (*s >= '0' && *s <= '9')); s++) {
         n = n * base +
	        (*s >= 'a' ? *s - 'a' + 10 :
	         *s >= 'A' ? *s - 'A' + 10 :
	         *s - '0');
    }
    return (uint8)n;
}

STATIC int
_bcm_rcpu_parse_macaddr(char *s, bcm_mac_t mac)
{
    char *p = s;
    int i;

    /* Must be of the form 00:01:02:03:AA:BB */
    if (17 != sal_strlen(s)) {
        return -1;
    }

    for (i = 0; i < 6; i++) {
        mac[i] = _bcm_rcpu_ctoi(p);
        p += 3;
    }
    return 0;
}

STATIC int
_bcm_rcpu_get_config(int unit)
{
    soc_pbmp_t  pbmp;

    char *mac_str;

    /* Get MAC address of slave unit */
    mac_str = soc_property_get_str(unit, spn_RCPU_LMAC);

    if (mac_str != NULL) {
    	if (_bcm_rcpu_parse_macaddr(mac_str, BCM_RCPU_CFG_LMAC(unit)) < 0 ) { 
             return BCM_E_INTERNAL;
        }
    } else { 
        sal_memcpy(BCM_RCPU_CFG_LMAC(unit), rcpu_default_mac, sizeof(bcm_mac_t)); 
        BCM_RCPU_CFG(unit)->lmac[5] = unit;
    }

    /* Get MAC address used by master unit */
    mac_str = soc_property_get_str(unit, spn_RCPU_SRC_MAC);

    if (mac_str != NULL) {
    	if (_bcm_rcpu_parse_macaddr(mac_str, BCM_RCPU_CFG_SRC_MAC(unit)) < 0 ) { 
             return BCM_E_INTERNAL;
        }
    } else {
        sal_memcpy(BCM_RCPU_CFG_SRC_MAC(unit), src_cpu_default_mac, sizeof(bcm_mac_t));
    }

    /* Get the default VLAN and TPID */
    BCM_RCPU_CFG(unit)->vlan = soc_property_get(unit, spn_RCPU_VLAN,
                                                BCM_VLAN_DEFAULT);
    BCM_RCPU_CFG(unit)->tpid = ENET_DEFAULT_TPID;

    /* Get the RCPU communication mode (OOB - Out of band through external
     * GMIII/MAC or through local XGS device. */
    if (soc_property_get(unit, spn_RCPU_USE_OOB, 1)) {
#if defined(BCM_OOB_RCPU_SUPPORT)
        /* Set the flags .
         *      BCM_RCPU_F_ETHER    - use external MAC/GMII port.
         *      BCM_RCPU_F_PACK     - compact the packet in signle buffer.
         */
        BCM_RCPU_CFG(unit)->flags |= BCM_RCPU_F_ETHER | BCM_RCPU_F_PACK;
        BCM_RCPU_CFG(unit)->remunit = -1;
        BCM_RCPU_CFG(unit)->eth_unit = 
            soc_property_get(unit, spn_RCPU_OOB_CHANNEL, 0);

#endif /* BCM_OOB_RCPU_SUPPORT */
    } else {
        BCM_RCPU_CFG(unit)->flags |= BCM_RCPU_F_XGS;
        BCM_RCPU_CFG(unit)->remunit = 
            soc_property_get(unit, spn_RCPU_MASTER_UNIT, 0);
        BCM_RCPU_CFG(unit)->eth_unit = -1;
        bcm_rx_cfg_get(unit, &BCM_RCPU_CFG(unit)->rx_cfg);
    }

    /* get the valid PBM for remote MAC. Valid PBM is the ports
     * on which the RCPU packets can be received by the remote CMIC. */
    pbmp = soc_property_get_pbmp(unit, spn_RCPU_RX_PBMP, 0);
    if (SOC_PBMP_IS_NULL(pbmp)) {
        pbmp = PBMP_PORT_ALL(unit);
    }
    SOC_PBMP_ASSIGN(BCM_RCPU_CFG(unit)->pbmp, pbmp);

    /* Get the Signature and Ethertype to be used for remote CMIC packets. */
    BCM_RCPU_CFG(unit)->signature = BCM_RCPU_CONTROL(unit)->info->signature;
    BCM_RCPU_CFG(unit)->ether_type = BCM_RCPU_CONTROL(unit)->info->pkt_type;

    /* Get the various RCPU packet priorities */
    if (soc_feature(unit, soc_feature_rcpu_priority)) {
        BCM_RCPU_CFG(unit)->dot1pri[0] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS0, 0);
        BCM_RCPU_CFG(unit)->mh_tc[0] = soc_property_get(unit, spn_RCPU_MH_TC_COS0, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[0] = soc_property_get(unit, spn_RCPU_CPU_TC_COS0, 0);
        BCM_RCPU_CFG(unit)->dot1pri[1] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS1, 0);
        BCM_RCPU_CFG(unit)->mh_tc[1] = soc_property_get(unit, spn_RCPU_MH_TC_COS1, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[1] = soc_property_get(unit, spn_RCPU_CPU_TC_COS1, 0);
        BCM_RCPU_CFG(unit)->dot1pri[2] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS2, 0);
        BCM_RCPU_CFG(unit)->mh_tc[2] = soc_property_get(unit, spn_RCPU_MH_TC_COS2, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[2] = soc_property_get(unit, spn_RCPU_CPU_TC_COS2, 0);
        BCM_RCPU_CFG(unit)->dot1pri[3] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS3, 0);
        BCM_RCPU_CFG(unit)->mh_tc[3] = soc_property_get(unit, spn_RCPU_MH_TC_COS3, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[3] = soc_property_get(unit, spn_RCPU_CPU_TC_COS3, 0);
        BCM_RCPU_CFG(unit)->dot1pri[4] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS4, 0);
        BCM_RCPU_CFG(unit)->mh_tc[4] = soc_property_get(unit, spn_RCPU_MH_TC_COS4, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[4] = soc_property_get(unit, spn_RCPU_CPU_TC_COS4, 0);
        BCM_RCPU_CFG(unit)->dot1pri[5] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS5, 0);
        BCM_RCPU_CFG(unit)->mh_tc[5] = soc_property_get(unit, spn_RCPU_MH_TC_COS5, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[5] = soc_property_get(unit, spn_RCPU_CPU_TC_COS5, 0);
        BCM_RCPU_CFG(unit)->dot1pri[6] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS6, 0);
        BCM_RCPU_CFG(unit)->mh_tc[6] = soc_property_get(unit, spn_RCPU_MH_TC_COS6, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[6] = soc_property_get(unit, spn_RCPU_CPU_TC_COS6, 0);
        BCM_RCPU_CFG(unit)->dot1pri[7] = soc_property_get(unit, spn_RCPU_DOT1PRI_COS7, 0);
        BCM_RCPU_CFG(unit)->mh_tc[7] = soc_property_get(unit, spn_RCPU_MH_TC_COS7, 0);
        BCM_RCPU_CFG(unit)->cpu_tc[7] = soc_property_get(unit, spn_RCPU_CPU_TC_COS7, 0);
    }
    return BCM_E_NONE;
}

STATIC int
_bcm_rcpu_find_unit_from_lmac(bcm_mac_t *lmac, int *u)
{
    int         unit;

    for (unit = 0; unit < BCM_RCPU_MAX_UNITS; unit++) {
        if (!BCM_RCPU_CONTROL(unit) ||
            sal_memcmp(lmac, BCM_RCPU_CFG_LMAC(unit), 
                        sizeof(bcm_mac_t))) {
            continue;
        }

        *u = unit;
        return BCM_E_NONE;
    }
    return BCM_E_NOT_FOUND;
}

STATIC int
_bcm_get_pkt_len(bcm_pkt_t *pkt)
{
    int pkt_len;

    BCM_PKT_BLK_BYTES_CALC(pkt, pkt_len);

    return pkt_len;
}

STATIC void
_rcpu_tx_callback(int unit, bcm_pkt_t *rcpu_pkt, void *cookie)
{
    bcm_pkt_t   *pkt;

    if (cookie) {
        pkt = (bcm_pkt_t *) cookie;

        if (pkt->call_back) {
            pkt->call_back(pkt->unit, pkt, rcpu_pkt->cookie);
        }
    }
    bcm_pkt_free(rcpu_pkt->unit, rcpu_pkt);
    return;
}

/* tx_list callback data */
typedef struct _rcpu_tx_list_callback_data_s {
    bcm_pkt_cb_f      call_back;    /* Packet callback function */
    bcm_pkt_cb_f      all_done;     /* List callback function */
    void              *cookie;      /* Original pkt->cookie */
    bcm_pkt_t         *pkt_list;    /* head of packte list */
} _rcpu_tx_list_callback_data_t;

STATIC void
_rcpu_tx_list_callback(int unit, bcm_pkt_t *pkt, void *cookie)
{
    _rcpu_tx_list_callback_data_t *data =
        (_rcpu_tx_list_callback_data_t *)pkt->cookie;

    /* Restore pkt data that was used */
    pkt->cookie = data->cookie;
    pkt->call_back = data->call_back;

    /* Packet callback, if any */
    if (pkt->call_back) {
        pkt->call_back(unit, pkt, cookie);
    }

    /* There's should always be a list callback, otherwise this
       function would not have been called */
    data->all_done(unit, data->pkt_list, cookie);

    sal_free(data);
    
}

STATIC INLINE void
_rcpu_get_mac_vlan_ptrs(bcm_pkt_t *pkt, uint8 **src_mac,
                   uint8 **vlan_ptr, int *block_offset, int *byte_offset)
{
    /* Assume everything is in block 0 */
    *src_mac = &pkt->pkt_data[0].data[sizeof(bcm_mac_t)];
    *block_offset = 0;

    if (BCM_PKT_NO_VLAN_TAG(pkt) || 
        BCM_PKT_HAS_HGHDR(pkt)) { /* Get VLAN from _vtag pkt member */
        /* If the packet has a vlan tag AND a Higig header, then
           skip over the vlan tag. */
        if (BCM_PKT_NO_VLAN_TAG(pkt)) {
        *byte_offset = 2 * sizeof(bcm_mac_t);
        } else {
            *byte_offset = 2 * sizeof(bcm_mac_t) + sizeof(uint32);
        }
        *vlan_ptr = pkt->_vtag;

        if ((uint32)pkt->pkt_data[0].len < 2 * sizeof(bcm_mac_t)) {
            /* Src MAC in block 1 */
            *src_mac = pkt->pkt_data[1].data;
            *block_offset = 1;
            *byte_offset = sizeof(bcm_mac_t);
        }
    } else { /* Packet has VLAN tag */
        *byte_offset = 2 * sizeof(bcm_mac_t) + sizeof(uint32);
        *vlan_ptr = &pkt->pkt_data[0].data[2 * sizeof(bcm_mac_t)];

        if ((uint32)pkt->pkt_data[0].len < 2 * sizeof(bcm_mac_t)) {
            /* Src MAC in block 1; assume VLAN there too at first */
            *src_mac = pkt->pkt_data[1].data;
            *vlan_ptr = &pkt->pkt_data[1].data[sizeof(bcm_mac_t)];
            *block_offset = 1;
            *byte_offset = sizeof(bcm_mac_t) + sizeof(uint32);
            if ((uint32)pkt->pkt_data[1].len < sizeof(bcm_mac_t) + sizeof(uint32)) {
                /* Oops, VLAN in block 2 */
                *vlan_ptr = pkt->pkt_data[2].data;
                *block_offset = 2;
                *byte_offset = sizeof(uint32);
            }
        } else if ((uint32)pkt->pkt_data[0].len <
                   2 * sizeof(bcm_mac_t) + sizeof(uint32)) {
            /* VLAN in block 2 */
            *block_offset = 1;
            *byte_offset = sizeof(uint32);
            *vlan_ptr = pkt->pkt_data[1].data;
        }
    }
}

STATIC int
_tx_rcpu_pkt_vtag_port_info_get(int unit, bcm_pkt_t *pkt, 
                                    int *out_port, int *vtag_remove)
{
    uint32 bmap = 0;
    int port = -1;
    int port_adj = 0;

    bmap = SOC_PBMP_WORD_GET(pkt->tx_pbmp, 0);
    port = _pbm2port(bmap);

    if (port == -1) {
        if (soc_feature(unit, soc_feature_register_hi)) {
            bmap = SOC_PBMP_WORD_GET(pkt->tx_pbmp, 1);
            port = _pbm2port(bmap);
            port_adj = 32;
        }
        if (port == -1) {
            return BCM_E_PARAM;
        }
    }
    if ((bmap & (~(1 << port))) != 0) {
         return BCM_E_PARAM;
    }

    port += port_adj;
    if (PBMP_MEMBER(pkt->tx_upbmp, port)) {
        *vtag_remove = 1;
    } else {
        *vtag_remove = 0; /* don't care */
    }

    *out_port = port;

    return BCM_E_NONE;
}

STATIC int
_tx_rcpu_higig_hdr_setup(int unit, bcm_pkt_t *pkt, uint8 *hdr_buf,
                         int *hdr_size, int *blk_ofst, 
                         int *byte_ofst, uint32 *flags)
{
    int byte_offset = 0;
    uint8 *src_mac, *vlan_ptr, *buf_start = hdr_buf;
    int block_offset = 0;
    soc_pbsmh_hdr_t *pbh = NULL;
    int port = -1;
    uint32      tx_hdr_flags = 0;
    int i;
    uint32 prio_val;
    int src_mod;

    src_mod = (pkt->flags & BCM_TX_SRC_MOD) ?
    pkt->src_mod : 0;
    prio_val = (pkt->flags & BCM_TX_PRIO_INT) ? pkt->prio_int : pkt->cos;

    if ((uint32)pkt->pkt_data[0].len < sizeof(bcm_mac_t)) {
        return BCM_E_PARAM;
    }

    /* Get pointers to srcmac and vlan; check if bad block count */
    _rcpu_get_mac_vlan_ptrs(pkt, &src_mac, &vlan_ptr, &block_offset,
                       &byte_offset);
    if (block_offset >= pkt->blk_count) {
        return BCM_E_PARAM;
    }

    if (byte_offset >= pkt->pkt_data[block_offset].len) {
        byte_offset = 0;
        block_offset++;
    }

    *flags = 0; /* clear the flags */

    /*
     *  XGS3: Decide whether to put Higig header or PB (Pipe Bypass)
     *  header in the TX descriptor
     *  1.      Fabric mapped mode (HG header in descriptor)
     *  2.      Raw Ethernet packet steered mode (PB header in descriptor)
     */
    if (!BCM_PKT_TX_ETHER(pkt)) {
        int vtag_remove_chk;
        /*
         *  Raw packet steered mode (PB header in descriptor)
         */
        pbh = (soc_pbsmh_hdr_t *)pkt->_pb_hdr;

        BCM_IF_ERROR_RETURN(_tx_rcpu_pkt_vtag_port_info_get(unit, pkt, &port, 
                                                       &vtag_remove_chk));
        if (vtag_remove_chk == 1) {
            pkt->flags |= BCM_PKT_F_TX_UNTAG;
        } else {
            pkt->flags &= (~BCM_PKT_F_TX_UNTAG);
        }

        BCM_API_XLATE_PORT_A2P(unit, &port);

        /* Setup PBH header */
        if (SOC_DCB_TYPE(unit) == 23) {
            PBS_MH_V7_W0_START_SET(pbh);
            PBS_MH_V7_W1_DPORT_SET(pbh, port);
            PBS_MH_V7_W2_SMOD_COS_QNUM_SET(pbh, src_mod, 1, pkt->cos,
                                           (64*port), prio_val);
            
            if (pkt->flags & BCM_PKT_F_TIMESYNC) {
                PBS_MH_V7_TS_PKT_SET(pbh);
            }
        } else {
            PBS_MH_W0_START_SET(pbh);
            PBS_MH_W1_RSVD_SET(pbh);

            if ((SOC_DCB_TYPE(unit) == 12) || (SOC_DCB_TYPE(unit) == 15) ||
                (SOC_DCB_TYPE(unit) == 17) || (SOC_DCB_TYPE(unit) == 18)) {
                PBS_MH_V2_W2_SMOD_DPORT_COS_SET(pbh, src_mod, port,
                                                pkt->cos, prio_val, 0);
                if (pkt->flags & BCM_PKT_F_TIMESYNC) {
                    PBS_MH_V2_TS_PKT_SET(pbh);
                } 
            } else if ((SOC_DCB_TYPE(unit) == 14) || (SOC_DCB_TYPE(unit) == 19) ||
                       (SOC_DCB_TYPE(unit) == 20)) {
                if (!(pkt->flags & BCM_TX_PRIO_INT)) {
                    /* If BCM_TX_PRIO_INT is not specified, the int_pri
                     * is set to the cos value. However in version 3 of 
                     * the PBS_MH, cos is 6 bit, but int_prio is only 4.
                     * To be safe, set the int_prio to 0 if it wasn't 
                     * explicitly set as indicated by BCM_TX_PRIO_INT flag.
                     */
                    prio_val = 0;
                }
                PBS_MH_V3_W2_SMOD_DPORT_COS_SET(pbh, src_mod, port,
                                                pkt->cos, prio_val, 0);
                if (pkt->flags & BCM_PKT_F_TIMESYNC) {
                    PBS_MH_V3_TS_PKT_SET(pbh);
                }
            } else if (SOC_DCB_TYPE(unit) == 16) {
                PBS_MH_V4_W2_SMOD_DPORT_COS_SET(pbh, src_mod, port,
                                                pkt->cos, prio_val, 0);
            }  else if (SOC_DCB_TYPE(unit) == 21) {
                PBS_MH_V5_W1_SMOD_SET(pbh, src_mod, 1, 0, 0);
                PBS_MH_V5_W2_DPORT_COS_SET(pbh, port, pkt->cos, prio_val);
                if (pkt->flags & BCM_PKT_F_TIMESYNC) {
                    PBS_MH_V5_TS_PKT_SET(pbh);
                }
            } else if (SOC_DCB_TYPE(unit) == 24) {
                PBS_MH_V5_W1_SMOD_SET(pbh, src_mod, 1, 0, 0);
                PBS_MH_V6_W2_DPORT_COS_QNUM_SET(pbh, port, pkt->cos,
                                                (64*port), prio_val);
                
                if (pkt->flags & BCM_PKT_F_TIMESYNC) {
                    PBS_MH_V5_TS_PKT_SET(pbh);
                }
            } else {
                PBS_MH_V1_W2_SMOD_DPORT_COS_SET(pbh, src_mod, port,
                                                pkt->cos, prio_val, 0);
            }
        }

        /* copy PBH to header buffer */
        RCPU_COPY_HDR(hdr_buf, pbh, sizeof(soc_pbsmh_hdr_t), TRUE);
        RCPU_CLR_HDR(hdr_buf, 0x20 - sizeof(soc_pbsmh_hdr_t), TRUE);

        tx_hdr_flags |= RCPU_HDR_F_SOBMH;

        /*
         * BCM_PKT_F_HGHDR flag indicates higig header is part of
         * packet data stream.
         */
        if (BCM_PKT_HAS_HGHDR(pkt)) {
            uint8 *tmp_pkt_hg_hdr = BCM_PKT_HG_HDR(pkt);
            int hg_hdr_len = SOC_HIGIG_HDR_SIZE;
            /*
             *      XGS3: Raw Higig packet steered mode.
             *            Make higig header part of packet stream and PB
             *            header part of descriptor
             */
#ifdef BCM_HIGIG2_SUPPORT
            if (tmp_pkt_hg_hdr[0] == SOC_HIGIG2_START) {
                hg_hdr_len = SOC_HIGIG2_HDR_SIZE;
            }
#endif /* BCM_HIGIG2_SUPPORT */
            /* copy HiGig header to buffer */
            RCPU_COPY_HDR(hdr_buf, tmp_pkt_hg_hdr, hg_hdr_len, TRUE);
            tx_hdr_flags |= RCPU_HDR_F_HIGIG;
        }
    } else {
        /* Non-SOBMH HiGig hdr */
        if (BCM_PKT_TX_HG_READY(pkt)) {
            uint8 *tmp_pkt_hg_hdr = BCM_PKT_HG_HDR(pkt);
            uint8 pad[32];
            int hg_hdr_len = SOC_HIGIG_HDR_SIZE;
#ifdef BCM_HIGIG2_SUPPORT
            if (tmp_pkt_hg_hdr[0] == SOC_HIGIG2_START) {
                hg_hdr_len = SOC_HIGIG2_HDR_SIZE;
            }
#endif /* BCM_HIGIG2_SUPPORT */
            for (i = 0; i < (32 - hg_hdr_len); i++) {
                pad[i] = 0;
            }
            /* copy HiGig header to buffer */
            RCPU_COPY_HDR(hdr_buf, tmp_pkt_hg_hdr, hg_hdr_len, TRUE);
            RCPU_COPY_HDR(hdr_buf, pad, (32 - hg_hdr_len), TRUE);
            tx_hdr_flags |= RCPU_HDR_F_HIGIG;
        }
    }

    /* Dest mac */
    RCPU_COPY_HDR(hdr_buf, pkt->pkt_data[0].data, sizeof(bcm_mac_t), TRUE);
     
    /* Source mac */
    RCPU_COPY_HDR(hdr_buf, src_mac, sizeof(bcm_mac_t), TRUE);

    /* VLAN tag */
    /* No VLAN tag for Fabric mapped Higig TX as well */
    if (!BCM_PKT_HAS_HGHDR(pkt) &&
        !(pkt->flags & BCM_PKT_F_TX_UNTAG) &&
        !BCM_PKT_TX_FABRIC_MAPPED(pkt)) { /* No HG Hdr, so add VLAN tag */
        RCPU_COPY_HDR(hdr_buf, vlan_ptr, sizeof(uint32), TRUE);
    }

    /* SL TAG */
    if (pkt->flags & BCM_PKT_F_SLTAG) {
        RCPU_COPY_HDR(hdr_buf, pkt->_sltag, sizeof(uint32), TRUE);
    }

    *blk_ofst = block_offset;
    *byte_ofst = byte_offset;
    *flags = tx_hdr_flags;
    *hdr_size = hdr_buf - buf_start;

    soc_cm_debug(DK_TX, 
                 "_tx_rcpu_higig_hdr_setup: flags 0x%08x rcpu_flags 0x%08x\n",
                 pkt->flags, *flags);

    return BCM_E_NONE;
}


void
rx_higig_info_decode(int unit, bcm_pkt_t *pkt);

STATIC bcm_pkt_t *
_bcm_rcpu_rx_process_dcb(int unit, dcb_t *dcb, bcm_pkt_t *rx_pkt)
{
  /** hawkeye doesn't have higig header which no higig information */
  /** higig word field contains 0 */
  rx_pkt->rx_port = SOC_DCB_RX_INGPORT_GET(unit, dcb);

  if (SOC_IS_XGS_SWITCH(unit)) { /* Get XGS HiGig info from DMA desc */
#ifdef BCM_ESW_SUPPORT
#ifdef BCM_XGS_SUPPORT
    int src_port_tgid;

    rx_pkt->opcode = SOC_DCB_RX_OPCODE_GET(unit, dcb);
    rx_pkt->dest_mod = SOC_DCB_RX_DESTMOD_GET(unit, dcb);
    rx_pkt->dest_port = SOC_DCB_RX_DESTPORT_GET(unit, dcb);
    rx_pkt->src_mod = SOC_DCB_RX_SRCMOD_GET(unit, dcb);
    src_port_tgid = SOC_DCB_RX_SRCPORT_GET(unit, dcb);
    if (!soc_feature(unit, soc_feature_trunk_group_overlay) &&
        (src_port_tgid & BCM_TGID_TRUNK_INDICATOR(unit))) {
        rx_pkt->src_trunk = src_port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit);
        rx_pkt->flags |= BCM_PKT_F_TRUNK;
        rx_pkt->src_port = -1;
    } else {
        rx_pkt->src_port = src_port_tgid;
        rx_pkt->src_trunk = -1;
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (soc_feature(unit, soc_feature_trunk_extended)) {
            int rv = BCM_E_NONE;
            bcm_trunk_t tid;
    	
            tid = -1;
            rv = _bcm_xgs3_trunk_get_port_property(unit, rx_pkt->src_mod,
                                                   src_port_tgid, &tid);
            if (BCM_SUCCESS(rv) && (tid != -1)) {
                rx_pkt->src_trunk = tid;
                rx_pkt->flags |= BCM_PKT_F_TRUNK;
                rx_pkt->src_port = -1;
            }
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    }
    rx_pkt->cos = SOC_DCB_RX_COS_GET(unit, dcb);

    rx_pkt->prio_int = BCM_PKT_VLAN_PRI(rx_pkt);
    rx_pkt->vlan = BCM_PKT_VLAN_ID(rx_pkt);
   

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        bcm_vlan_t outer_vid;
        int hg_hdr_size;
      
        if (SOC_DCB_RX_MIRROR_GET(unit, dcb)) {
            rx_pkt->flags |= BCM_RX_MIRRORED;
        } 
        rx_pkt->rx_classification_tag = SOC_DCB_RX_CLASSTAG_GET(unit, dcb);
        outer_vid = SOC_DCB_RX_OUTER_VID_GET(unit, dcb);
        if (outer_vid) {
            /* Don't overwrite if we don't have info */
            rx_pkt->vlan = outer_vid;
        }
        rx_pkt->vlan_pri = SOC_DCB_RX_OUTER_PRI_GET(unit, dcb);
        rx_pkt->vlan_cfi = SOC_DCB_RX_OUTER_CFI_GET(unit, dcb);
        rx_pkt->inner_vlan = SOC_DCB_RX_INNER_VID_GET(unit, dcb);
        rx_pkt->inner_vlan_pri = SOC_DCB_RX_INNER_PRI_GET(unit, dcb);
        rx_pkt->inner_vlan_cfi = SOC_DCB_RX_INNER_CFI_GET(unit, dcb);
        rx_pkt->rx_unit = rx_pkt->unit = unit;
        rx_pkt->rx_reason = SOC_DCB_RX_REASON_GET(unit, dcb);

#ifdef BCM_HIGIG2_SUPPORT
        if (soc_feature(unit, soc_feature_higig2)) {
            hg_hdr_size = sizeof(soc_higig2_hdr_t);
        } else
#endif /* BCM_HIGIG2_SUPPORT */
        {
            hg_hdr_size = sizeof(soc_higig_hdr_t);
        }
        /* Put XGS hdr into pkt->_higig */
        sal_memcpy(rx_pkt->_higig, SOC_DCB_MHP_GET(unit, dcb),
                   hg_hdr_size);
#ifdef  LE_HOST
        {
	    /* Higig field accessors expect network byte ordering,
         * so we must reverse the bytes on LE hosts */
            int word;
            uint32 *hg_data = (uint32 *) rx_pkt->_higig;
            for (word = 0; word < BYTES2WORDS(hg_hdr_size); word++) {
                hg_data[word] = _shr_swap32(hg_data[word]);
            }
        }
#endif
        rx_higig_info_decode(unit, rx_pkt);
    }
    rx_pkt->rx_matched = SOC_DCB_RX_MATCHRULE_GET(unit, dcb);
    SOC_DCB_RX_REASONS_GET(unit, dcb, &rx_pkt->rx_reasons);

#endif
    
    if (soc_feature(unit, soc_feature_rx_timestamp)) {
        /* Get time stamp value for TS protocol packets or OAM DM */
        rx_pkt->rx_timestamp = SOC_DCB_RX_TIMESTAMP_GET(unit, dcb); 
    }
    
    if (soc_feature(unit, soc_feature_rx_timestamp_upper)) {
        /* Get upper 32-bit of time stamp value for OAM DM */
        rx_pkt->rx_timestamp_upper = SOC_DCB_RX_TIMESTAMP_UPPER_GET(unit, dcb);
    }
#endif
#endif
  }
    return 0;
}



typedef struct rcpu1_cmic_pkt_hdr_s {
    bcm_mac_t	        dst_mac;    /* Destination MAC address  */
    bcm_mac_t	        src_mac;    /* Source MAC address       */
    uint16              tpid;       /* vlan TPID                */
    uint16              vlan;       /* vlan tag                 */
    uint16              ethertype;  /*  Ether type              */
    uint16              signature;  /* RCPU packet signature    */
    uint8               operation;  /* op-code                  */
    uint8               flags;      /*  flags                   */
    uint16              transid;    /* transaction number       */
    uint16              datalen;    /* length of data (in bytes)*/
    uint16              replen;     /* expected data length     */
    uint32              reserved;   /* reserved must be 0       */
} rcpu1_cmic_pkt_hdr_t;

#define MAX_OUTSTANDING_SCHAN_OPS 16 /* Max number of threads which can be doing a schan_op ?? */
#define MAX_RETRY_ATTEMPTS 3
struct {
    char data[CMIC_SCHAN_WORDS_ALLOC*4];
    int waiting_for_reply;
} schan_reply[MAX_OUTSTANDING_SCHAN_OPS];


/* Enable VLAN translation for the vlan on given unit and port. */
STATIC int
rcpu_vlan_translate(int unit, int vlan, int port)
{
    BCM_IF_ERROR_RETURN
        (bcm_vlan_translate_add(unit, port, vlan, vlan, -1));
    BCM_IF_ERROR_RETURN
        (bcm_vlan_translate_egress_add(unit, port, vlan, vlan, -1));
    BCM_IF_ERROR_RETURN
        (bcm_vlan_control_port_set(unit, port,
                                   bcmVlanTranslateIngressEnable, TRUE));
    BCM_IF_ERROR_RETURN
        (bcm_vlan_control_port_set(unit, port,
                                   bcmVlanTranslateEgressEnable, TRUE));
    BCM_IF_ERROR_RETURN
        (bcm_vlan_control_port_set(unit, port,
                                   bcmVlanTranslateEgressMissUntag, FALSE));

    return BCM_E_NONE;
}


STATIC int
rcpu1_init(int unit)
{
    uint32              rval, MacOui, MacNonOui;
    uint8               *mac;
    bcm_l2_addr_t	    l2addr;
    bcm_pbmp_t		    pbm;
    bcm_pbmp_t		    upbm;
    bcm_pbmp_t		pbmcpu;
    int                 i, masterunit;
    bcm_module_t        mymodid;
    int                 rcpu_vlan;
    int                 rcpu_encap;
    int                 rcpu_port, port;

    masterunit = soc_property_get(unit, spn_RCPU_MASTER_UNIT, -1);
    if (masterunit == unit) {
        return BCM_E_CONFIG;
    }

    rcpu_vlan = BCM_RCPU_CFG(unit)->vlan & 0xfff;
    rcpu_port = soc_property_get(unit, spn_RCPU_PORT, 3);
    BCM_IF_ERROR_RETURN(bcm_port_encap_get(unit, rcpu_port, &rcpu_encap));

    rval = 0;
    SOC_PBMP_ITER(BCM_RCPU_CFG(unit)->pbmp, port) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_control_set(unit, port, 
                                     bcmPortControlRemoteCpuEnable, 1));
    }

    /* enable all COS */
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, 
                               bcmSwitchRemoteCpuToCpuDestPortAllReasons, 0)); 
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, 
                               bcmSwitchRemoteCpuToCpuDestMacAllReasons, 1)); 

    rval = 0;
    soc_reg_field_set(unit, CMIC_PKT_REASON_MINIr, &rval, REASONSf, 0);
    SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_REASON_MINIr(unit, rval));

    /* enable all COS */
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuForceScheduling, 1)); 

    /* Set dest mac address */
    mac = BCM_RCPU_CFG_SRC_MAC(unit);
    MacOui = (((uint32)mac[0]) << 16 | ((uint32)mac[1]) << 8 | ((uint32)mac[2]));
    MacNonOui = (((uint32)mac[3]) << 16 | ((uint32)mac[4]) << 8 | ((uint32)mac[5]));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuDestMacOui, 
                                   MacOui));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuDestMacNonOui, 
                                   MacNonOui));

    /* Set local mac address */
    mac = BCM_RCPU_CFG_LMAC(unit);
    MacOui = (((uint32)mac[0]) << 16 | ((uint32)mac[1]) << 8 | ((uint32)mac[2]));
    MacNonOui = (((uint32)mac[3]) << 16 | ((uint32)mac[4]) << 8 | ((uint32)mac[5]));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuLocalMacOui, 
                                   MacOui));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuLocalMacNonOui, 
                                   MacNonOui));
    /* set CMIC EtherType and Signature */
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuEthertype, 
                                   BCM_RCPU_CFG(unit)->ether_type));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuSignature, 
                                   BCM_RCPU_CFG(unit)->signature));

    /* Set the vlan and tpid values */
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuTpid, 
                                   BCM_RCPU_CFG(unit)->tpid));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuVlan, 
                                   BCM_RCPU_CFG(unit)->vlan));

    /* Enable vlan/lmac1 checking and Schan op */
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuSchanEnable, 1));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuMatchLocalMac, 1));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuMatchVlan, 1));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuToCpuEnable, 1));
    BCM_IF_ERROR_RETURN(
        bcm_esw_switch_control_set(unit, bcmSwitchRemoteCpuFromCpuEnable, 1));
    
#if defined(BCM_TRX_SUPPORT) || defined(BCM_RAVEN_SUPPORT)
    if (soc_feature(unit, soc_feature_rcpu_priority)) {
        uint8 property;
        SOC_IF_ERROR_RETURN(READ_CMIC_PKT_CTRLr(unit, &rval));
        property = soc_property_get(unit, spn_RCPU_MH_SRC_PID_ENABLE, 0);
        soc_reg_field_set(unit, CMIC_PKT_CTRLr, &rval, MH_SRC_PID_ENABLEf, property);
        property = soc_property_get(unit, spn_RCPU_MH_CPU_COS_ENABLE, 0);
        soc_reg_field_set(unit, CMIC_PKT_CTRLr, &rval, CPU_TC_ENABLEf, property);
        property = soc_property_get(unit, spn_RCPU_MH_TC_MAP_ENABLE, 0);
        soc_reg_field_set(unit, CMIC_PKT_CTRLr, &rval, MH_TC_MAP_ENABLEf, property); 
        property = soc_property_get(unit, spn_RCPU_DOT1PRI_MAP_ENABLE, 0);  
        soc_reg_field_set(unit, CMIC_PKT_CTRLr, &rval, IEEE_802_1_PRI_MAP_ENABLEf, property);
        SOC_IF_ERROR_RETURN(WRITE_CMIC_PKT_CTRLr(unit, rval));
    }
#endif /* BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */
    
    /*
     * Insert L2 entry on remote unit and local unit.
     */
    bcm_l2_addr_t_init(&l2addr, 
                       BCM_RCPU_CFG(unit)->lmac, 
                       rcpu_vlan);
    l2addr.flags |= BCM_L2_STATIC | BCM_L2_COPY_TO_CPU;
    BCM_IF_ERROR_RETURN(
        bcm_stk_my_modid_get(unit, &mymodid));
    l2addr.modid = soc_property_get(unit, spn_RCPU_SLAVE_MODID, mymodid);
    bcm_l2_addr_add(unit, &l2addr);

    if ( masterunit >= 0 ) {
      l2addr.flags &= ~(BCM_L2_COPY_TO_CPU);
      BCM_IF_ERROR_RETURN(
          bcm_stk_my_modid_get(masterunit, &mymodid));
      if ( rcpu_encap != BCM_PORT_ENCAP_IEEE ) {
          l2addr.modid = soc_property_get(unit, spn_RCPU_SLAVE_MODID, mymodid);
          l2addr.port = CMIC_PORT(unit);
      } else {
      l2addr.modid = soc_property_get(masterunit, spn_RCPU_MASTER_MODID, mymodid);
      l2addr.port = rcpu_port;
      }
      bcm_l2_addr_add(masterunit, &l2addr);
    }

    bcm_l2_addr_t_init(&l2addr, 
                       BCM_RCPU_CFG_SRC_MAC(unit), 
                       rcpu_vlan);

    l2addr.flags = BCM_L2_STATIC;
    if ( masterunit >= 0 ) {
        BCM_IF_ERROR_RETURN(
            bcm_stk_my_modid_get(masterunit, &mymodid));
        l2addr.modid = soc_property_get(masterunit, spn_RCPU_MASTER_MODID, mymodid);
        bcm_l2_addr_add(masterunit, &l2addr);
    }    

    BCM_IF_ERROR_RETURN(
        bcm_stk_my_modid_get(unit, &mymodid));

    if ( rcpu_encap != BCM_PORT_ENCAP_IEEE ) {
      /** for higig, we want to do it on the higig port */
      l2addr.modid = soc_property_get(masterunit, spn_RCPU_MASTER_MODID, mymodid);
      l2addr.port = CMIC_PORT(masterunit);  /* destination should be CMIC port */
    } else {
        l2addr.modid = soc_property_get(unit, spn_RCPU_SLAVE_MODID, mymodid);
        l2addr.port = rcpu_port;
    }
    bcm_l2_addr_add(unit, &l2addr);

    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_CLEAR(upbm);
    BCM_PBMP_CLEAR(pbmcpu);
    BCM_PBMP_PORT_SET(pbm, l2addr.port);
    BCM_PBMP_PORT_SET(pbmcpu, l2addr.port);
    BCM_PBMP_PORT_ADD(pbmcpu, CMIC_PORT(unit));


    /* If the RCPU vlan is not the default, create the vlan */
    if (BCM_SUCCESS(bcm_vlan_create(unit, rcpu_vlan))) {
        if (rcpu_vlan != BCM_VLAN_DEFAULT &&
            rcpu_encap == BCM_PORT_ENCAP_IEEE) {
            /* Remove RCPU port from VLAN 1 if RCPU VLAN is not 1,
             and the RCPU port encapsulation is IEEE */
            BCM_IF_ERROR_RETURN(bcm_vlan_port_remove(unit, 1, pbm));

            /* Prepare VLAN translation on the RCPU port and CPU port if
               double tagging mode gets enabled. This forces the service
               vlan to always be the RCPU vlan for RCPU protocol packets. */

            if (soc_feature(unit, soc_feature_vlan_translation)) {
                BCM_IF_ERROR_RETURN(rcpu_vlan_translate(unit, rcpu_vlan, 
                                                        l2addr.port));
                BCM_IF_ERROR_RETURN(rcpu_vlan_translate(unit, rcpu_vlan,
                                                        CMIC_PORT(unit)));
            }
        }
    }
    BCM_IF_ERROR_RETURN
	(bcm_vlan_port_remove(unit, rcpu_vlan, pbmcpu));
    BCM_IF_ERROR_RETURN
	(bcm_vlan_port_add(unit, rcpu_vlan, pbmcpu,
                           upbm));

    if ( masterunit >= 0 ) {
        /* Error code here is ignored in purpose */
        (void)bcm_vlan_create(masterunit, rcpu_vlan);

        BCM_IF_ERROR_RETURN
        (bcm_vlan_port_remove(masterunit, rcpu_vlan, pbmcpu));
        BCM_IF_ERROR_RETURN
        (bcm_vlan_port_add(masterunit, rcpu_vlan, pbmcpu, upbm));
        if (rcpu_vlan != BCM_VLAN_DEFAULT &&
            rcpu_encap == BCM_PORT_ENCAP_IEEE) {
            BCM_IF_ERROR_RETURN
                (bcm_vlan_port_remove(masterunit, 1, pbm));
        
            /* Prepare VLAN translation on the RCPU port and CPU port
               if double tagging mode gets enabled for the master
               unit. This forces the service vlan to always be the
               RCPU vlan for RCPU protocol packets. */

            if (soc_feature(unit, soc_feature_vlan_translation)) {
                BCM_IF_ERROR_RETURN(rcpu_vlan_translate(masterunit, rcpu_vlan,
                                                        l2addr.port));
                BCM_IF_ERROR_RETURN(rcpu_vlan_translate(masterunit, rcpu_vlan,
                                                        CMIC_PORT(masterunit)));
            }
        }
    }
    
    for (i = 0; i < MAX_OUTSTANDING_SCHAN_OPS; i++) {
        schan_reply[i].waiting_for_reply = FALSE;
    }

#if defined(BCM_TRX_SUPPORT) || defined(BCM_RAVEN_SUPPORT)
    /* Set the various packet priorities */
    if (soc_feature(unit, soc_feature_rcpu_priority)) {
        for (i = 0; i < NUM_COS(unit); i++) {
            uint32  flags = 0;
            int     val = 0;

            if (BCM_RCPU_CFG(unit)->dot1pri[i]) {
                flags |= BCM_SWITCH_REMOTE_CPU_ENCAP_IEEE;
                val = BCM_RCPU_CFG(unit)->dot1pri[i];
            } else if (BCM_RCPU_CFG(unit)->mh_tc[i]) {
                flags |= BCM_SWITCH_REMOTE_CPU_ENCAP_HIGIG2;
                val = BCM_RCPU_CFG(unit)->mh_tc[i];
            }
            BCM_IF_ERROR_RETURN(
                bcm_esw_switch_rcpu_encap_priority_map_set(unit, flags, i, val));
        }
    }
#endif /* BCM_TRX_SUPPORT || BCM_RAVEN_SUPPORT */

#if defined(BCM_TRX_SUPPORT)

#define BCM_TRX_CPU_QUEUE_MAX      31

    /* Set the traffic class to CPU queue mapping */
    if (soc_feature(unit, soc_feature_rcpu_tc_mapping)) {
        int queue, tc, tc_total = soc_mem_index_count(unit, CPU_TS_MAPm);
        cpu_ts_map_entry_t tc_map;

        for (tc = 0; tc < tc_total; tc++) {
            sal_memset(&tc_map, 0, sizeof(tc_map));
            queue = soc_property_suffix_num_get(unit, tc, spn_RCPU_CPU_QUEUE,
                                                "tc", 0);
            if (queue > BCM_TRX_CPU_QUEUE_MAX) {
                queue = BCM_TRX_CPU_QUEUE_MAX;
            }
            soc_mem_field32_set(unit, CPU_TS_MAPm, &tc_map,
                                CPU_QUEUE_IDf, queue);
            SOC_IF_ERROR_RETURN
                (WRITE_CPU_TS_MAPm(unit, MEM_BLOCK_ALL, tc, &tc_map));
        }
    }
#endif /* BCM_TRX_SUPPORT */
    return BCM_E_NONE;
}

STATIC int
_bcm_generic_fill_cmic_hdr(int unit, rcpu1_cmic_pkt_hdr_t *p_rcpu_hdr, 
                          int payload_len, uint32 flags, 
                          int opcode, int transid, int replen)
{
    sal_memcpy(p_rcpu_hdr->dst_mac, 
               BCM_RCPU_CFG_LMAC(unit), sizeof(bcm_mac_t));
    sal_memcpy(p_rcpu_hdr->src_mac, 
               BCM_RCPU_CFG_SRC_MAC(unit), sizeof(bcm_mac_t));
    p_rcpu_hdr->tpid = soc_htons(BCM_RCPU_CFG(unit)->tpid);
    p_rcpu_hdr->vlan = soc_htons(BCM_RCPU_CFG(unit)->vlan);
    p_rcpu_hdr->ethertype = soc_htons(BCM_RCPU_CFG(unit)->ether_type);
    p_rcpu_hdr->signature = soc_htons(BCM_RCPU_CFG(unit)->signature);
    p_rcpu_hdr->operation = opcode;

    if (flags & (RCPU_HDR_F_HIGIG | RCPU_HDR_F_SOBMH)) {
        p_rcpu_hdr->flags |= BCM_RCPU_FLAG_MODHDR;
    }

    if (opcode == BCM_RCPU_OP_SCHAN_REQ) {
        p_rcpu_hdr->flags |= BCM_RCPU_FLAG_REPLY;
    }
    
    p_rcpu_hdr->transid = transid;
    p_rcpu_hdr->replen = replen;
    p_rcpu_hdr->reserved = 0;
    p_rcpu_hdr->datalen = soc_htons(payload_len);

    return BCM_E_NONE;
}

STATIC int
rcpu1_tx(int unit, bcm_pkt_t *pkt, void * cookie)
{
    int         rcpu_pkt_len = 0, pkt_len, remunit;
#ifdef BCM_HIGIG2_SUPPORT
    uint8       *tmp_pkt_hg_hdr = BCM_PKT_HG_HDR(pkt);
#endif /* BCM_HIGIG2_SUPPORT */
    int         hg_hdr_len = SOC_HIGIG_HDR_SIZE;
    int         hdr_size, blk_ofst, byte_ofst, i, rv = BCM_E_NONE;
    uint32      flags, tmp_len, payload_len;
    uint8       *tx_buf, *buf_ptr;
    bcm_pkt_t   *new_pkt = NULL;

    if ((pkt_len = _bcm_get_pkt_len(pkt)) <= 0) {
        return BCM_E_PARAM;
    }

    /* allocate pkt/buffer big enough to accommodate the packet 
     * after all the header manipulation is done.
     */
    rcpu_pkt_len = pkt_len + sizeof(rcpu1_cmic_pkt_hdr_t);
    if (!BCM_PKT_TX_ETHER(pkt)) {
        /* SOBMH header needs to be appended to the packet. */
        rcpu_pkt_len += 32;
        if (BCM_PKT_HAS_HGHDR(pkt)) {
#ifdef BCM_HIGIG2_SUPPORT
            if (tmp_pkt_hg_hdr[0] == SOC_HIGIG2_START) {
                hg_hdr_len = SOC_HIGIG2_HDR_SIZE;
            }
#endif /* BCM_HIGIG2_SUPPORT */
            rcpu_pkt_len += hg_hdr_len ; 
            if (!BCM_PKT_NO_VLAN_TAG(pkt)) {
               rcpu_pkt_len -= sizeof(uint32);
            }

        } else if (!BCM_PKT_NO_VLAN_TAG(pkt)) {
            int port;
            int vtag_remove_chk;

           BCM_IF_ERROR_RETURN(_tx_rcpu_pkt_vtag_port_info_get(unit, pkt, &port,
                                                       &vtag_remove_chk));
           if (vtag_remove_chk == 1) {
               /* vtag will be removed later, so reduce the rcpu_pkt_len */
              rcpu_pkt_len -= sizeof(uint32);
           }
       }

    } else {
        /* Non-SOBMH HiGig hdr */
        if (BCM_PKT_TX_HG_READY(pkt)) {
#ifdef BCM_HIGIG2_SUPPORT
            if (tmp_pkt_hg_hdr[0] == SOC_HIGIG2_START) {
                hg_hdr_len = SOC_HIGIG2_HDR_SIZE;
            }
#endif /* BCM_HIGIG2_SUPPORT */
            rcpu_pkt_len += hg_hdr_len; 
        }
    }

    if (BCM_RCPU_CFG(unit)->flags & BCM_RCPU_F_ETHER) {
#if defined(BCM_OOB_RCPU_SUPPORT)
        if (BCM_RCPU_CFG(unit)->flags & BCM_RCPU_F_PACK) {
            int min_len;
            /* tx on oob ethernet driver */
            tx_buf = BCM_RCPU_CONTROL(unit)->tx_buf;
            sal_memset(tx_buf, 0, RCPU_MAX_BUF_LEN);

            hdr_size = blk_ofst = byte_ofst = flags = 0;

            RCPU_UNIT_LOCK(unit);

            _tx_rcpu_higig_hdr_setup(unit, pkt, 
                              tx_buf + sizeof(rcpu1_cmic_pkt_hdr_t), 
                              &hdr_size, &blk_ofst, &byte_ofst, &flags);

            payload_len = hdr_size;
            buf_ptr = tx_buf + sizeof(rcpu1_cmic_pkt_hdr_t) + hdr_size;
            for (i = blk_ofst; i < pkt->blk_count; i++) {
                tmp_len = pkt->pkt_data[i].len - byte_ofst;
                sal_memcpy(buf_ptr, 
                           &pkt->pkt_data[i].data[byte_ofst],
                           tmp_len);
                buf_ptr += tmp_len;
                payload_len += tmp_len;
                byte_ofst = 0;
            }

            /* If CRC allocated, adjust min length */
            min_len = ENET_MIN_PKT_SIZE;
            if (pkt->flags & BCM_TX_CRC_ALLOC) {
                min_len = ENET_MIN_PKT_SIZE - ENET_FCS_SIZE;
            }

            /* Pad runt packets */
            if ((rcpu_pkt_len < min_len) && !(pkt->flags & BCM_TX_NO_PAD)) {
                sal_memset(buf_ptr, 0, min_len - rcpu_pkt_len);
            }

            /* Now construct the CMIC header */
            _bcm_generic_fill_cmic_hdr(unit, (rcpu1_cmic_pkt_hdr_t *)tx_buf, 
                        payload_len, flags, BCM_RCPU_OP_FROMCPU_PKT, 0, 0);

            /* call driver tx */
            rv = (eth_drv_tx(BCM_RCPU_CFG(unit)->eth_unit, tx_buf, 
                             payload_len + sizeof(rcpu1_cmic_pkt_hdr_t)) == 0) ?
                BCM_E_NONE : BCM_E_INTERNAL;

            RCPU_UNIT_UNLOCK(unit);

            pkt->_dv = cookie; 
            _bcm_rcpu_tx_packet_done_cb(unit, pkt);

            /* update tx stats */
            if (rv == 0) {
                BCM_RCPU_CONTROL(unit)->tx_pkt++;
            } else {
                BCM_RCPU_CONTROL(unit)->tx_fail++;
            }

        } else {
            /* pkt SG not supported */
            return BCM_E_UNAVAIL;
        }
#endif /* BCM_OOB_RCPU_SUPPORT */
    } else if (BCM_RCPU_CFG(unit)->flags & BCM_RCPU_F_XGS) {
        /* tx on xgs unit */
        int pad_len = 0;
        int add_len = 0;
        int crc = pkt->flags & BCM_TX_CRC_ALLOC;
        int min_len = ENET_MIN_PKT_SIZE;

        if (!BCM_PKT_NO_VLAN_TAG(pkt) && !BCM_PKT_HAS_HGHDR(pkt) && 
            !BCM_PKT_TX_FABRIC_MAPPED(pkt)) {
            min_len += 4;
        }
        if (pkt_len < (min_len - ENET_FCS_SIZE)) {
            add_len = (min_len - pkt_len);
            pad_len = add_len - ENET_FCS_SIZE;
        } else if (pkt_len <= min_len) {
            if (crc) {
                add_len = ENET_FCS_SIZE;
            } else {
                add_len = pad_len = min_len - pkt_len;
            }
        } else if (crc) {
            add_len += ENET_FCS_SIZE;
        }

        rcpu_pkt_len += add_len;
        remunit = BCM_RCPU_CFG(unit)->remunit;
        bcm_pkt_alloc(remunit, rcpu_pkt_len, 0, &new_pkt);
        if (!new_pkt) {
            return BCM_E_MEMORY;
        }

        tx_buf = new_pkt->pkt_data[0].data;
        sal_memset(tx_buf, 0, sizeof(rcpu1_cmic_pkt_hdr_t));
        hdr_size = blk_ofst = byte_ofst = flags = 0;

        _tx_rcpu_higig_hdr_setup(unit, pkt, 
                          tx_buf + sizeof(rcpu1_cmic_pkt_hdr_t), 
                          &hdr_size, &blk_ofst, &byte_ofst, &flags);

        /* copy the pkt payload to tx_buf */
        payload_len = hdr_size;
        buf_ptr = tx_buf + sizeof(rcpu1_cmic_pkt_hdr_t) + hdr_size;
        for (i = blk_ofst; i < pkt->blk_count; i++) {
            tmp_len = pkt->pkt_data[i].len - byte_ofst;
            sal_memcpy(buf_ptr, 
                       &pkt->pkt_data[i].data[byte_ofst],
                       tmp_len);
            buf_ptr += tmp_len;
            payload_len += tmp_len;
            byte_ofst = 0;
        }

        /* Add padding bytes */
        sal_memset(buf_ptr, 0, pad_len);

        soc_cm_debug(DK_TX, 
                 "_tx_rcpu_higig_hdr_setup: flags 0x%02x \n", flags);
        /* Now construct the CMIC header */
        _bcm_generic_fill_cmic_hdr(unit, (rcpu1_cmic_pkt_hdr_t *)tx_buf, 
                        payload_len, flags, BCM_RCPU_OP_FROMCPU_PKT, 0, 0);

        new_pkt->cookie = cookie;

        /* setup the new_pkt flags */
        new_pkt->flags |= BCM_TX_ETHER;
        new_pkt->flags &= ~BCM_TX_HG_READY;

        /* setup new)pkt call back options */
        /* If original pkt was synchronous then RCPU pkt should be synchronous */
        if (NULL == pkt->call_back) {
            new_pkt->call_back = NULL;
        } else {
        new_pkt->call_back = _rcpu_tx_callback;
        }
        
        /* call driver tx */
        rv = bcm_tx(remunit, new_pkt, (void*) pkt);
        if ((BCM_SUCCESS(rv)) && (NULL == new_pkt->call_back)) {
            _rcpu_tx_callback(unit, new_pkt, (void *)pkt);
        }
    }

    return rv;
}


int
rcpu1_schan_op(int unit, schan_msg_t *msg, int dwc_write, int dwc_read)
{
    uint8 *tx_buf;
    uint8 *buf_ptr;
    int i, remunit, rcpu_schan_pkt_len = 0;
    static int transid = -1;
    int cur_transid;
    static int cur_outstanding_schan_ops = 0;
    soc_timeout_t to;
    int timeout;
    int num_retries;
    int rv = SOC_E_NONE;
    bcm_pkt_t *new_pkt;

    if (cur_outstanding_schan_ops == MAX_OUTSTANDING_SCHAN_OPS) {
        return SOC_E_FAIL;
    }
    cur_outstanding_schan_ops++;
    
    RCPU_UNIT_LOCK(unit);
    transid = (transid + 1) % MAX_OUTSTANDING_SCHAN_OPS;
    while (schan_reply[transid].waiting_for_reply == TRUE) {
        transid = (transid + 1) % MAX_OUTSTANDING_SCHAN_OPS;
    }
    schan_reply[transid].waiting_for_reply = TRUE;
    cur_transid = transid;
    RCPU_UNIT_UNLOCK(unit);

    if (soc_property_get(unit, spn_RCPU_USE_OOB, 1)) {
        remunit = 0;
    } else {
        remunit = BCM_RCPU_CFG(unit)->remunit;
    }
    rcpu_schan_pkt_len = sizeof(rcpu1_cmic_pkt_hdr_t) + 32 + CMIC_SCHAN_WORDS_ALLOC*4;

    for (num_retries = 0; num_retries < MAX_RETRY_ATTEMPTS; num_retries++) {
        rv = SOC_E_NONE;

        bcm_pkt_alloc(remunit, rcpu_schan_pkt_len, 0, &new_pkt);
        if (!new_pkt) {
            return BCM_E_MEMORY;
        }
        
        tx_buf = new_pkt->pkt_data[0].data;
        sal_memset(tx_buf, 0, rcpu_schan_pkt_len);
        buf_ptr = tx_buf + sizeof(rcpu1_cmic_pkt_hdr_t);
        
        for (i = 0; i < 4 * dwc_write; i++) {
            buf_ptr[i] = msg->bytes[i];
        }
        
        /* Now construct the CMIC header */
        /* If dwc_read is 0, get a SCHAN_REPLY as an ACK (1 word) */
        _bcm_generic_fill_cmic_hdr(unit, (rcpu1_cmic_pkt_hdr_t *)tx_buf, 
                                  4 * dwc_write, 0, BCM_RCPU_OP_SCHAN_REQ, 
                                  cur_transid, dwc_read ? 4 * dwc_read : 4);

        if (soc_property_get(unit, spn_RCPU_USE_OOB, 1)) {
#if defined(BCM_OOB_RCPU_SUPPORT)
            /* call driver tx */
            eth_drv_tx(BCM_RCPU_CFG(unit)->eth_unit, tx_buf, 
                       4 * dwc_write + sizeof(rcpu1_cmic_pkt_hdr_t));
            bcm_pkt_free(unit, new_pkt);
#endif /* BCM_OOB_RCPU_SUPPORT */
        } else {
            /* setup the new_pkt flags */
            new_pkt->flags |= BCM_TX_ETHER;
            new_pkt->flags &= ~BCM_TX_HG_READY;
            new_pkt->call_back = _rcpu_tx_callback;

            /* call driver tx */
            rv = bcm_tx(remunit, new_pkt, (void *)NULL);
        }

        /* Now wait for the schan reply */
        timeout = 1000000; /* microseconds */
        soc_timeout_init(&to, timeout, 100);

        while (schan_reply[cur_transid].waiting_for_reply == TRUE) {
            if (soc_timeout_check(&to)) {
                rv = SOC_E_TIMEOUT;
                break;
            }
        }

        if (rv == SOC_E_NONE) {
            uint32 *pdata = (uint32 *)schan_reply[cur_transid].data;
            for (i = 0; i < dwc_read; i++) {
                msg->dwords[i] = soc_ntohl(pdata[i]);
            }
            break;
        }
    }

    schan_reply[cur_transid].waiting_for_reply = FALSE;
    cur_outstanding_schan_ops--;
    return rv;
}

extern int rcpu_rx_pkt_enqueue(int unit, bcm_pkt_t *ptk);

STATIC int 
rcpu1_rx(int unit, bcm_pkt_t *rx_pkt)
{
    return (rx_pkt) ? 
        rcpu_rx_pkt_enqueue(unit, rx_pkt) : BCM_E_PARAM;
}

STATIC bcm_rcpu_info_t bcm_rcpu_info_tbl[] = {
    { 
        soc_feature_rcpu_1,
        0x8874, 0x5600, 32, 32,
        rcpu1_tx,
        rcpu1_init,
        rcpu1_rx,
    },

    /* Donot modify this. This is always the last entry */
    { 0, 0, 0, 0, 0, NULL, NULL, NULL }
};



/*
 * Function:
 *      _bcm_rcpu_tx_list
 * Purpose:
 *      Transmit a linked list of packets via RCPU mechanism
 * Parameters:
 *      unit - transmission unit
 *      pkt - Pointer to linked list of packets
 *      all_done_cb - callback when complete
 *      cookie - Callback cookie.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This mimics the behavior of the XGS implementation of
 *      of bcm_tx_list().
 *      
 */
int 
_bcm_rcpu_tx_list(int unit, bcm_pkt_t *pkt, bcm_pkt_cb_f all_done_cb, void *cookie)
{
    bcm_pkt_t *cur_pkt, *next_pkt;

    for (cur_pkt = pkt; cur_pkt; cur_pkt = next_pkt) {
        next_pkt = cur_pkt->next;
        if (!cur_pkt->next && all_done_cb) {
            _rcpu_tx_list_callback_data_t *data;

            /* If this is the last packet and there's a completion
               callback, then chain _rcpu_tx_list_callback to this
               packet's callback. */
            data = sal_alloc(sizeof(_rcpu_tx_list_callback_data_t),
                             "_bcm_rcpu_tx_list");
            if (!data) {
                return BCM_E_MEMORY;
            }

            data->call_back = cur_pkt->call_back;
            data->cookie = cur_pkt->cookie;
            data->all_done = all_done_cb;
            data->pkt_list = pkt;
            cur_pkt->call_back = _rcpu_tx_list_callback;
            cur_pkt->cookie  = (void *)data;
        }

        BCM_IF_ERROR_RETURN(
            _bcm_rcpu_tx(unit, cur_pkt, cookie));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_rcpu_tx_array
 * Purpose:
 *      Transmit am array of packets via RCPU mechanism
 * Parameters:
 *      unit - transmission unit
 *      pkt - Pointer to an array of packets
 *      count - number of packets in the array
 *      all_done_cb - callback when complete
 *      cookie - Callback cookie.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This mimics the behavior of the XGS implementation of
 *      of bcm_tx_array().
 *      
 */
int 
_bcm_rcpu_tx_array(int unit, bcm_pkt_t **pkt, int count, 
                   bcm_pkt_cb_f all_done_cb, void *cookie)
{
    int       i;

    for (i = 0; i < count; i++) {
        if ((i == count - 1) && all_done_cb) {
            _rcpu_tx_list_callback_data_t *data;

            /* If this is the last packet and there's a completion
               callback, then chain _rcpu_tx_list_callback to this
               packet's callback. */
            data = sal_alloc(sizeof(_rcpu_tx_list_callback_data_t),
                             "_bcm_rcpu_tx_list");
            if (!data) {
                return BCM_E_MEMORY;
            }

            data->call_back = pkt[i]->call_back;
            data->cookie = pkt[i]->cookie;
            data->all_done = all_done_cb;
            data->pkt_list = pkt[0];
            pkt[i]->call_back = _rcpu_tx_list_callback;
            pkt[i]->cookie  = (void *)data;
        }

        BCM_IF_ERROR_RETURN(
            _bcm_rcpu_tx(unit, pkt[i], cookie));
    }

    return BCM_E_NONE;
}

int 
_bcm_rcpu_tx(int unit, bcm_pkt_t *pkt, void *cookie)
{

    if (!BCM_RCPU_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_RCPU_CONTROL(unit) == NULL) {
        return BCM_E_UNIT;
    }

    pkt->unit = unit;

    if (BCM_RCPU_CONTROL(unit)->info->_tx) {
        return BCM_RCPU_CONTROL(unit)->info->_tx(unit, pkt, cookie);
    }

    return BCM_E_PARAM;
}

STATIC int 
_bcm_rcpu_raw_rx(int eth_unit, uint8 * pkt, int pkt_len)
{
    int             unit, len, rv, hdr_size = 0, offset = 0;
    bcm_pkt_t       *rx_pkt = NULL;
    dcb_t           *dcb;
    rcpu1_cmic_pkt_hdr_t *rcpu_hdr;

    BCM_IF_ERROR_RETURN(
          _bcm_rcpu_find_unit_from_lmac((bcm_mac_t *) (pkt + 6), &unit));
    rcpu_hdr = (rcpu1_cmic_pkt_hdr_t *) pkt;
    
    soc_cm_debug(DK_RX, "RX matched unit %d pkt_type 0x%x:0x%x "
                 "ether_type 0x%x:0x%x len:%d\n", 
                 unit, BCM_RCPU_CONTROL(unit)->info->pkt_type,
                 soc_ntohs(rcpu_hdr->ethertype),
                 BCM_RCPU_CFG(unit)->signature,
                 soc_ntohs(rcpu_hdr->signature), pkt_len);

    if ((BCM_RCPU_CONTROL(unit)->info->pkt_type != 
                                soc_ntohs(rcpu_hdr->ethertype))) {
        /* No VLAN tag */ 
        offset = 4;
        rcpu_hdr = (rcpu1_cmic_pkt_hdr_t *) (pkt - 4);
    }

    if ((BCM_RCPU_CONTROL(unit)->info->pkt_type != 
                                soc_ntohs(rcpu_hdr->ethertype)) ||
        (BCM_RCPU_CFG(unit)->signature != 
                                soc_ntohs(rcpu_hdr->signature))) {
        return BCM_E_PARAM;
    }

    if (rcpu_hdr->operation == BCM_RCPU_OP_SCHAN_REPLY) {
        int transid = rcpu_hdr->transid;
        /* 
         * Check Fail flag 
	 *     If FAIL, then set length appropriately
         */
        if (rcpu_hdr->flags & BCM_RCPU_FLAG_FAIL) {
            if (soc_cm_debug_check(DK_RX)) {
                soc_cm_print("****FAIL****\n");
                soc_dma_ether_dump(0, "SCHAN_REP ", pkt, pkt_len, 0);
            }
            rcpu_hdr->datalen = pkt_len - sizeof(rcpu1_cmic_pkt_hdr_t) - offset;
        }
        
        if (schan_reply[transid].waiting_for_reply == TRUE) {
            sal_memcpy(schan_reply[transid].data, 
                       pkt + sizeof(rcpu1_cmic_pkt_hdr_t) - offset, 
                       rcpu_hdr->datalen);
            schan_reply[transid].waiting_for_reply = FALSE;
        }
        return BCM_E_NONE;
    }

    hdr_size = BCM_RCPU_CONTROL(unit)->info->cmic_hdr_size +
               BCM_RCPU_CONTROL(unit)->info->dcb_hdr_size + 
               ((uint8 *)rcpu_hdr -  pkt); 

    len = pkt_len - hdr_size;


    /* Allocate pkt. */
    bcm_pkt_rx_alloc(unit, len, &rx_pkt);
    if (!rx_pkt) {
        return BCM_E_MEMORY;
    }
    sal_memcpy(rx_pkt->pkt_data[0].data, pkt + hdr_size, len);
    rx_pkt->pkt_len = len;
    rx_pkt->tot_len = len;
    rx_pkt->blk_count = 1;
    rx_pkt->alloc_ptr = rx_pkt->_pkt_data.data;
    
    /** take care of no-tag case */
    dcb = (dcb_t *) ((uint8*) rcpu_hdr + 
                     BCM_RCPU_CONTROL(unit)->info->cmic_hdr_size - 8); 
    _bcm_rcpu_rx_process_dcb(unit, dcb, rx_pkt);

    rv =  BCM_RCPU_CONTROL(unit)->info->_rx(unit, rx_pkt);
    if (BCM_FAILURE(rv)) {
         bcm_pkt_rx_free(unit, rx_pkt);
    }
    return rv;
}


int rx_rcpu_intr_process_packet(bcm_pkt_t *rx_pkt, int *runit)
{
    int                     unit, len, hdr_size, pkt_len;
    dcb_t                   *dcb;
    rcpu1_cmic_pkt_hdr_t    *rcpu_hdr;
    uint8                   *pkt, *pkt_start_p, *pkt_data_p;
    bcm_port_t              src_port_tgid;
    
    hdr_size = 0;
    pkt = pkt_start_p = rx_pkt->pkt_data[0].data;
    pkt_len = rx_pkt->pkt_len;

    BCM_IF_ERROR_RETURN(
        _bcm_rcpu_find_unit_from_lmac((bcm_mac_t *) (pkt + 6), &unit));
    rcpu_hdr = (rcpu1_cmic_pkt_hdr_t *) pkt;

    if ((BCM_RCPU_CONTROL(unit)->info->pkt_type != 
                                soc_ntohs(rcpu_hdr->ethertype))) {
        rcpu_hdr = (rcpu1_cmic_pkt_hdr_t *) (pkt + 4);
    }
    if ((BCM_RCPU_CONTROL(unit)->info->pkt_type != 
                                soc_ntohs(rcpu_hdr->ethertype)) ||
        (BCM_RCPU_CFG(unit)->signature != 
                                soc_ntohs(rcpu_hdr->signature))) {
        return BCM_E_PARAM;
    }

    hdr_size = BCM_RCPU_CONTROL(unit)->info->cmic_hdr_size +
               BCM_RCPU_CONTROL(unit)->info->dcb_hdr_size + 
               ((uint8 *)rcpu_hdr -  pkt); 

    len = pkt_len - hdr_size;

    /* Construct packet information */
    dcb = (dcb_t *) ((uint8*) pkt + 
             BCM_RCPU_CONTROL(unit)->info->cmic_hdr_size - 8); 
    rx_pkt->opcode = SOC_DCB_RX_OPCODE_GET(unit, dcb);
    rx_pkt->dest_mod = SOC_DCB_RX_DESTMOD_GET(unit, dcb);
    rx_pkt->dest_port = SOC_DCB_RX_DESTPORT_GET(unit, dcb);
    rx_pkt->src_mod = SOC_DCB_RX_SRCMOD_GET(unit, dcb);
    src_port_tgid = SOC_DCB_RX_SRCPORT_GET(unit, dcb);
    if (!soc_feature(unit, soc_feature_trunk_group_overlay) &&
        (src_port_tgid & BCM_TGID_TRUNK_INDICATOR(unit))) {
        rx_pkt->src_trunk = src_port_tgid & BCM_TGID_PORT_TRUNK_MASK(unit);
        rx_pkt->flags |= BCM_PKT_F_TRUNK;
        rx_pkt->src_port = -1;
    } else {
        rx_pkt->src_port = src_port_tgid;
        rx_pkt->src_trunk = -1;
    }
    rx_pkt->cos = SOC_DCB_RX_COS_GET(unit, dcb);
    rx_pkt->prio_int = BCM_PKT_VLAN_PRI(rx_pkt);
    rx_pkt->vlan = BCM_PKT_VLAN_ID(rx_pkt);
    
    if (SOC_DCB_RX_MIRROR_GET(unit, dcb)) {
        rx_pkt->flags |= BCM_RX_MIRRORED;
    } 
    rx_pkt->rx_classification_tag = SOC_DCB_RX_CLASSTAG_GET(unit, dcb);
    rx_pkt->rx_matched = SOC_DCB_RX_MATCHRULE_GET(unit, dcb);

    if (soc_feature(unit, soc_feature_rx_timestamp)) {
        /* Get time stamp value for TS protocol packets */
        rx_pkt->rx_timestamp = SOC_DCB_RX_TIMESTAMP_GET(unit, dcb); 
    }
    
    rx_pkt->rx_unit = rx_pkt->unit = unit;
    rx_pkt->rx_reason = SOC_DCB_RX_REASON_GET(unit, dcb);
    SOC_DCB_RX_REASONS_GET(unit, dcb, &rx_pkt->rx_reasons);

    /* Move the data to the head of the packet */
    pkt_data_p = pkt + hdr_size;

    sal_memcpy(pkt_start_p, pkt_data_p, len);

    rx_pkt->pkt_data[0].len = len;
    rx_pkt->pkt_len = len;
    rx_pkt->tot_len = len;
    rx_pkt->blk_count = 1;
    rx_pkt->alloc_ptr = rx_pkt->_pkt_data.data;

    *runit = unit;

    return BCM_E_NONE;
}



STATIC bcm_rx_t
_bcm_rcpu_pkt_rx(int unit, bcm_pkt_t * rx_pkt, void * cookie)
{
    int rv = BCM_RX_NOT_HANDLED;

    if (rx_pkt->blk_count == 1) {
        if (_bcm_rcpu_raw_rx(unit, rx_pkt->pkt_data[0].data,
                         rx_pkt->pkt_len) == BCM_E_NONE) {
            rv = BCM_RX_HANDLED;
        }
    }
    return rv;
}

STATIC void 
_rcpu_modid_chg_cb(int unit, soc_switch_event_t  event, uint32 arg1, 
                   uint32 arg2, uint32 arg3, void* userdata)
{
    switch (event) {
        case SOC_SWITCH_EVENT_MODID_CHANGE:
            rcpu1_init(unit);
            break;
        default:
            break;
    }

    return;
}

int
_bcm_esw_rcpu_init(int unit)
{
    bcm_rcpu_info_t     *p_info = bcm_rcpu_info_tbl;
    uint8               *tx_buf;
    soc_pbmp_t          pbmp;
    int                 rcpu_slave = FALSE;
    int                 rv = BCM_E_NONE;

    if ((!BCM_UNIT_VALID(unit)) || (!SOC_UNIT_VALID(unit))){
        return BCM_E_UNIT;
    }

    if (SOC_IS_RCPU_ONLY(unit)) {
        return BCM_E_NONE;
    }

    if (!SOC_IS_RCPU_UNIT(unit)) {
        pbmp = soc_property_get_pbmp(unit, spn_RCPU_RX_PBMP, 0);
        if (!SOC_PBMP_IS_NULL(pbmp)) {
            rcpu_slave = TRUE;
        } else {
            return BCM_E_NONE;
        }
    }

    if ( BCM_RCPU_CONTROL(unit) && BCM_RCPU_CONTROL(unit)->flags & BCM_RCPU_F_INITED )
      return BCM_E_NONE;

    /* Map device to remote CMIC type. */
    while(p_info->rcpu_type && !soc_feature(unit, p_info->rcpu_type)) {
        p_info++;
    }

    if (!p_info->rcpu_type) {
        goto err_no_match;
    }
    
    if (BCM_RCPU_CONTROL(unit) == NULL) {
        BCM_RCPU_CONTROL(unit) = sal_alloc(sizeof(_bcm_rcpu_control_t), 
                                           "rcpu_control");
        if (BCM_RCPU_CONTROL(unit) == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(BCM_RCPU_CONTROL(unit), 0, sizeof(_bcm_rcpu_control_t));

        BCM_RCPU_CONTROL(unit)->lock = sal_mutex_create("rcpu_unit_mutex");
        if (BCM_RCPU_CONTROL(unit)->lock == NULL) {
            return BCM_E_MEMORY;
        }

        tx_buf = (uint8 *)sal_alloc(RCPU_MAX_BUF_LEN, "rcpu_tx_buffer");
        if (!tx_buf) {
            soc_cm_debug(DK_ERR, 
                     "bcm_rcpu_init: Failed to allocate memory.\n");
            return BCM_E_MEMORY;
        }
        BCM_RCPU_CONTROL(unit)->tx_buf = tx_buf;
    }

    BCM_RCPU_CONTROL(unit)->info = p_info;

    /*
     * Get the config for the device.
     */
    BCM_IF_ERROR_RETURN(_bcm_rcpu_get_config(unit));

    if (TRUE == rcpu_slave) {
        return (BCM_RCPU_CONTROL(unit)->info->_hw_init(unit));
    }

#if defined(BCM_OOB_RCPU_SUPPORT)
    if (BCM_RCPU_CONTROL(unit)->flags & BCM_RCPU_F_INITED) {
        if (BCM_RCPU_CFG(unit)->flags & BCM_RCPU_F_ETHER) {
            eth_drv_stop(BCM_RCPU_CFG(unit)->eth_unit);
        }
    }
#endif /* BCM_OOB_RCPU_SUPPORT */

    if (BCM_RCPU_CFG(unit)->flags & BCM_RCPU_F_ETHER) {
#if defined(BCM_OOB_RCPU_SUPPORT)
        if (eth_drv_init(BCM_RCPU_CFG(unit)->eth_unit) ||
            eth_drv_register(BCM_RCPU_CFG(unit)->eth_unit, _bcm_rcpu_raw_rx) ||
            eth_drv_get_mac(BCM_RCPU_CFG(unit)->eth_unit,
                            BCM_RCPU_CFG_SRC_MAC(unit)) ||
            BCM_RCPU_CONTROL(unit)->info->_hw_init(unit) ||
            eth_drv_start(BCM_RCPU_CFG(unit)->eth_unit)) {
            soc_cm_debug(DK_ERR, 
                     "bcm_rcpu_init: Failed to init ethernet driver.\n");
            return BCM_E_INTERNAL;
        }
        BCM_RCPU_CONTROL(unit)->flags |= BCM_RCPU_F_INITED;
#endif /* BCM_OOB_RCPU_SUPPORT */
    } else {
        BCM_IF_ERROR_RETURN(BCM_RCPU_CONTROL(unit)->info->_hw_init(unit));

        SOC_IF_ERROR_RETURN(bcm_rx_register(BCM_RCPU_CFG(unit)->remunit,
                        "RCPU RX", _bcm_rcpu_pkt_rx,
                        BCM_RCPU_RX_PRIO, NULL, BCM_RCO_F_ALL_COS));
        rv = bcm_rx_start(BCM_RCPU_CFG(unit)->remunit,
                            &BCM_RCPU_CFG(unit)->rx_cfg);
        if (rv == BCM_E_BUSY) {
            rv = BCM_E_NONE;
        }
    }

    /* Register a call back function to be notified upon modid changes */
    BCM_IF_ERROR_RETURN(
        soc_event_register(unit, _rcpu_modid_chg_cb, NULL));

    return rv;

err_no_match:

    if (BCM_RCPU_CONTROL(unit)) {
        sal_free(BCM_RCPU_CONTROL(unit));
        BCM_RCPU_CONTROL(unit) = NULL;
    }

    return BCM_E_NOT_FOUND;
}
/*
 * Function:
 *      _bcm_esw_rcpu_master_init
 * Purpose:
 *      Initialize RCPU master units before BCM module init so each bcm module
 *      can access s-channel registers/tables via RCPU mechanism
 * Parameters:
 *      unit - (IN) unit number
 * Returns:
 *      BCM_E_XXX
 *
 */
int
_bcm_esw_rcpu_master_init(int unit)
{
    bcm_rcpu_info_t     *p_info = bcm_rcpu_info_tbl;
    uint8               *tx_buf;
    int                  rv = BCM_E_NONE;

    if ((!BCM_UNIT_VALID(unit)) || (!SOC_UNIT_VALID(unit))){
        return BCM_E_UNIT;
    }

    if (!SOC_IS_RCPU_ONLY(unit)) {
        return BCM_E_NONE;
    }

    if ( BCM_RCPU_CONTROL(unit) && BCM_RCPU_CONTROL(unit)->flags & BCM_RCPU_F_INITED )
      return BCM_E_NONE;

    /* Map device to remote CMIC type. */
    while(p_info->rcpu_type && !soc_feature(unit, p_info->rcpu_type)) {
        p_info++;
    }

    if (!p_info->rcpu_type) {
        goto err_no_match;
    }
    
    if (BCM_RCPU_CONTROL(unit) == NULL) {
        BCM_RCPU_CONTROL(unit) = sal_alloc(sizeof(_bcm_rcpu_control_t), 
                                           "rcpu_control");
        if (BCM_RCPU_CONTROL(unit) == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(BCM_RCPU_CONTROL(unit), 0, sizeof(_bcm_rcpu_control_t));

        BCM_RCPU_CONTROL(unit)->lock = sal_mutex_create("rcpu_unit_mutex");
        if (BCM_RCPU_CONTROL(unit)->lock == NULL) {
            return BCM_E_MEMORY;
        }

        tx_buf = (uint8 *)sal_alloc(RCPU_MAX_BUF_LEN, "rcpu_tx_buffer");
        if (!tx_buf) {
            soc_cm_debug(DK_ERR, 
                     "bcm_rcpu_init: Failed to allocate memory.\n");
            return BCM_E_MEMORY;
        }
        BCM_RCPU_CONTROL(unit)->tx_buf = tx_buf;
    }

    BCM_RCPU_CONTROL(unit)->info = p_info;
#if defined(BCM_OOB_RCPU_SUPPORT)
    if (BCM_RCPU_CONTROL(unit)->flags & BCM_RCPU_F_INITED) {
        if (BCM_RCPU_CFG(unit)->flags & BCM_RCPU_F_ETHER) {
            eth_drv_stop(BCM_RCPU_CFG(unit)->eth_unit);
        }
    }
#endif /* BCM_OOB_RCPU_SUPPORT */
    /*
     * Get the config for the device.
     */
    BCM_IF_ERROR_RETURN(_bcm_rcpu_get_config(unit));

    if (BCM_RCPU_CFG(unit)->flags & BCM_RCPU_F_ETHER) {
#if defined(BCM_OOB_RCPU_SUPPORT)
        if (eth_drv_init(BCM_RCPU_CFG(unit)->eth_unit) ||
            eth_drv_register(BCM_RCPU_CFG(unit)->eth_unit, _bcm_rcpu_raw_rx) ||
            eth_drv_get_mac(BCM_RCPU_CFG(unit)->eth_unit,
                            BCM_RCPU_CFG_SRC_MAC(unit)) ||
            eth_drv_start(BCM_RCPU_CFG(unit)->eth_unit)) {
            soc_cm_debug(DK_ERR, 
                     "bcm_rcpu_init: Failed to init ethernet driver.\n");
            return BCM_E_INTERNAL;
        }
        BCM_RCPU_CONTROL(unit)->flags |= BCM_RCPU_F_INITED;
#endif /* BCM_OOB_RCPU_SUPPORT */
    } else {
        SOC_IF_ERROR_RETURN(bcm_rx_register(BCM_RCPU_CFG(unit)->remunit,
                        "RCPU RX", _bcm_rcpu_pkt_rx,
                        BCM_RCPU_RX_PRIO, NULL, BCM_RCO_F_ALL_COS));
        rv = bcm_rx_start(BCM_RCPU_CFG(unit)->remunit,
                            &BCM_RCPU_CFG(unit)->rx_cfg);
        if (rv == BCM_E_BUSY) {
            rv = BCM_E_NONE;
        }
    }

    SOC_IF_ERROR_RETURN(
        soc_rcpu_schan_op_register(unit, rcpu1_schan_op));

    return rv;

err_no_match:

    if (BCM_RCPU_CONTROL(unit)) {
        sal_free(BCM_RCPU_CONTROL(unit));
        BCM_RCPU_CONTROL(unit) = NULL;
    }

    return BCM_E_NOT_FOUND;
}
#else
int
_bcm_esw_rcpu_init(int unit)
{
    COMPILER_REFERENCE(unit);
    return BCM_E_UNAVAIL;
}

int
_bcm_esw_rcpu_master_init(int unit)
{
    COMPILER_REFERENCE(unit);
    return BCM_E_UNAVAIL;
}

int 
_bcm_rcpu_tx(int unit, bcm_pkt_t *pkt, void *cookie)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(pkt);
    COMPILER_REFERENCE(cookie);
    return BCM_E_UNAVAIL;
}

int 
_bcm_rcpu_tx_list(int unit, bcm_pkt_t *pkt, bcm_pkt_cb_f all_done_cb, void *cookie)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(pkt);
    COMPILER_REFERENCE(all_done_cb);
    COMPILER_REFERENCE(cookie);
    return BCM_E_UNAVAIL;
}

int 
_bcm_rcpu_tx_array(int unit, bcm_pkt_t **pkt, int count, 
                   bcm_pkt_cb_f all_done_cb, void *cookie)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(pkt);
    COMPILER_REFERENCE(count);
    COMPILER_REFERENCE(all_done_cb);
    COMPILER_REFERENCE(cookie);
    return BCM_E_UNAVAIL;
}


#endif /* INCLUDE_RCPU && BCM_XGS3_SWITCH_SUPPORT */
