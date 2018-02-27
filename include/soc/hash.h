/*
 * $Id: hash.h 1.50.20.2 Broadcom SDK $
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
 * Hash table calculation routines
 */

#ifndef _SOC_HASH_H
#define _SOC_HASH_H

#include <soc/mem.h>

#ifdef BCM_XGS3_SWITCH_SUPPORT
typedef struct soc_xgs3_ecmp_hash_s{
    ip_addr_t  dip;            /* IPv4 destination IP.      */
    ip_addr_t  sip;            /* IPv4 source IP.           */
    ip6_addr_t dip6;           /* IPv6 destination IP.      */
    ip6_addr_t sip6;           /* IPv6 source IP.           */
    uint16     l4_src_port;    /* TCP/UDP source port.      */
    uint16     l4_dst_port;    /* TCP/UDP destination port. */
    uint8      v6;             /* IPv6 flag.                */
    int        ecmp_count;     /* Ecmp group size.          */
} soc_xgs3_ecmp_hash_t;
#endif /* BCM_XGS3_SWITCH_SUPPORT */

extern uint32   soc_draco_crc32(uint8 *data, int data_size);
extern uint16   soc_draco_crc16(uint8 *data, int data_size);
extern uint32   soc_draco15_key76_crc32(uint8 *key, int unused_data_size);
extern uint16   soc_draco15_key76_crc16(uint8 *key, int unused_data_size);

extern void     soc_draco_l2x_base_entry_to_key(int unit, l2x_entry_t *entry,
                                                uint8 *key);
extern void     soc_draco_l2x_param_to_key(sal_mac_addr_t mac, int vid,
                                           uint8 *key);
extern uint32   soc_draco_l2_hash(int unit, int hash_sel, uint8 *key);


extern void     soc_draco_l3x_base_entry_to_key(int unit, l3x_entry_t *entry,
                                                int key_sip, uint8 *key);
extern void     soc_draco_l3x_param_to_key(ip_addr_t ip, ip_addr_t src_ip,
                                           uint16 vid, uint8 *key);
extern uint32   soc_draco_l3_hash(int unit, int hash_sel, int ipmc,
                                  uint8 *key);
extern uint32   soc_draco_l3_ecmp_hash(int unit, ip_addr_t dip, ip_addr_t sip,
                     int ecmp_count, int ecmp_hash_sel, int l3_hash_sel);
extern uint32   soc_tucana_l3_ecmp_hash(int unit, ip_addr_t dip, ip_addr_t sip, 
                                        int ecmp_count);
extern uint32   soc_lynx_l3_ecmp_hash(int unit, ip_addr_t dip, ip_addr_t sip, 
                                        int ecmp_count);
#ifdef BCM_XGS3_SWITCH_SUPPORT
extern uint32   soc_xgs3_l3_ecmp_hash(int unit, soc_xgs3_ecmp_hash_t *data);
#endif /* BCM_XGS3_SWITCH_SUPPORT */
extern uint32   soc_draco_trunk_hash(sal_mac_addr_t da,
                                     sal_mac_addr_t sa, int tgsize);

extern int      soc_draco_hash_set(int unit, int hash_sel);
extern int      soc_draco_hash_get(int unit, int *hash_sel);
extern uint32   soc_lynx_dmux_entry(uint32 *data);

extern uint32   soc_fb_l2_hash(int unit, int hash_sel, uint8 *key);

extern uint32   soc_fb_l3_hash(int unit, int hash_sel,
                               int key_nbits, uint8 *key);
extern int      soc_fb_l3x_base_entry_to_key(int unit,
                                             uint32 *entry,
                                             uint8 *key);
extern uint32	soc_fb_l3x2_entry_hash(int unit, uint32 *entry);
extern uint32	soc_fb_vlan_mac_hash(int unit, int hash_sel, uint8 *key);
extern int      soc_fb_rv_vlanmac_hash_sel_get(int unit, int dual, int *hash_sel);

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
 || defined(BCM_RAPTOR_SUPPORT)
extern int soc_fb_l2x_entry_bank_hash_sel_get(int unit, int bank,
                                              int *hash_sel);
extern uint32 soc_fb_l2x_entry_hash(int unit, int hash_sel, uint32 *entry);
extern int soc_fb_l3x_entry_bank_hash_sel_get(int unit, int bank,
                                              int *hash_sel);
extern uint32 soc_fb_l3x_entry_hash(int unit, int hash_sel, uint32 *entry);
extern int    soc_fb_l3x_bank_entry_hash(int unit, int bank, uint32 *entry);    
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT || BCM_RAPTOR_SUPPORT */

#define FB2_HASH_VRF_BITS       6
#define FB2_HASH_VRF_MASK       0x3f

extern uint32   soc_er_l2_hash(int unit, int hash_sel, uint8 *key);
extern uint32   soc_er_l2ext_hash(int unit, int hash_sel, uint8 *key);

extern uint32   soc_er_l3v4_hash(int unit, int hash_sel, uint8 *key);
extern uint32   soc_er_l3v6_hash(int unit, int hash_sel, uint8 *key);
extern int      soc_er_l3x_hash(int unit, uint32 *entry, int l3v6);
extern void     soc_er_l2x_base_entry_to_key(int unit,
                                             uint32 *entry, uint8 *key);
extern void     soc_er_l3v4_base_entry_to_key(int unit,
                                              uint32 *entry, uint8 *key);
extern void     soc_er_l3v6_base_entry_to_key(int unit,
                                              uint32 *entry, uint8 *key);
extern uint32	soc_er_vlan_mac_hash(int unit, int hash_sel, uint8 *key);
extern uint32	soc_er_my_station_hash(int unit, int hash_sel, uint8 *key);

#define ER_HASH_L2_NO_TABLE     0xfff
#define ER_HASH_MVL_NO_TABLE    0x1ff

/* ER internal L2 always uses same hi bits for different table sizes */
#define ER_HASH_L2_INT_BITS     12

/* ER does not use CRC16 for the external table */
#define ER_HASH_L2_EXT_CRC16_LOWER_SHIFT        5
#define ER_HASH_L2_EXT_CRC16_UPPER_SHIFT        10

/* Triumph functions */
extern int      soc_tr_vlan_xlate_hash_sel_get(int unit, int bank, 
                                               int *hash_sel);
extern int      soc_tr_egr_vlan_xlate_hash_sel_get(int unit, int bank, 
                                                   int *hash_sel);
extern uint32   soc_tr_l2x_hash(int unit, int hash_sel, int key_nbits,
                                void *base_entry, uint8 *key);
extern int      soc_tr_l2x_base_entry_to_key(int unit, void *entry,
                                             uint8 *key);
extern uint32   soc_tr_l2x_entry_hash(int unit, int hash_sel, uint32 *entry);
extern uint32   soc_tr_l2x_bank_entry_hash(int unit, int bank, uint32 *entry);

extern uint32   soc_tr_vlan_xlate_hash(int unit, int hash_sel, int key_nbits,
                                       void *base_entry, uint8 *key);
extern int      soc_tr_vlan_xlate_base_entry_to_key(int unit, void *entry,
                                                    uint8 *key);
extern uint32   soc_tr_vlan_xlate_entry_hash(int unit, int hash_sel, 
                                             uint32 *entry);
extern uint32   soc_tr_vlan_xlate_bank_entry_hash(int unit, int bank,
                                                  uint32 *entry);

extern uint32   soc_tr_egr_vlan_xlate_hash(int unit, int hash_sel,
                                           int key_nbits, void *base_entry,
                                           uint8 *key);
extern int      soc_tr_egr_vlan_xlate_base_entry_to_key(int unit, void *entry,
                                                        uint8 *key);
extern uint32   soc_tr_egr_vlan_xlate_entry_hash(int unit, int hash_sel, 
                                                 uint32 *entry);
extern uint32   soc_tr_egr_vlan_xlate_bank_entry_hash(int unit, int bank,
                                                      uint32 *entry);
extern int      soc_tr_mpls_hash_sel_get(int unit, int bank, int *hash_sel);
extern uint32   soc_tr_mpls_hash(int unit, int hash_sel, int key_nbits,
                                 void *base_entry, uint8 *key);
extern int      soc_tr_mpls_base_entry_to_key(int unit, void *entry,
                                              uint8 *key);
extern uint32   soc_tr_mpls_entry_hash(int unit, int hash_sel, uint32 *entry);
extern uint32   soc_tr_mpls_bank_entry_hash(int unit, int bank, uint32 *entry);

extern int soc_dual_hash_recurse_depth_get(int unit, soc_mem_t mem);

#define TRX_HASH_VRF_BITS       11 
#define TRX_HASH_VRF_MASK       0x7ff

enum XGS_HASH {
    /* WARNING: values given match hardware register; do not modify */
    XGS_HASH_CRC16_UPPER = 0,
    XGS_HASH_CRC16_LOWER = 1,
    XGS_HASH_LSB = 2,
    XGS_HASH_ZERO = 3,
    XGS_HASH_CRC32_UPPER = 4,
    XGS_HASH_CRC32_LOWER = 5,
    XGS_HASH_COUNT
};

enum FB_HASH {
    /* WARNING: values given match hardware register; do not modify */
    FB_HASH_ZERO = 0,
    FB_HASH_CRC32_UPPER = 1,
    FB_HASH_CRC32_LOWER = 2,
    FB_HASH_LSB = 3,
    FB_HASH_CRC16_LOWER = 4,
    FB_HASH_CRC16_UPPER = 5,
    FB_HASH_COUNT
};

enum XGS_HASH_KEY_TYPE {
    /* WARNING: values given match hardware register; do not modify */
    XGS_HASH_KEY_TYPE_L2 = 0,
    XGS_HASH_KEY_TYPE_L3UC = 1,
    XGS_HASH_KEY_TYPE_L3MC = 2,
    XGS_HASH_KEY_TYPE_COUNT
};

enum TR_L3_HASH_KEY_TYPE {
    /* WARNING: values given match hardware register; do not modify */
    TR_L3_HASH_KEY_TYPE_V4UC = 0,
    TR_L3_HASH_KEY_TYPE_V4MC = 1,
    TR_L3_HASH_KEY_TYPE_V6UC = 2,
    TR_L3_HASH_KEY_TYPE_V6MC = 3,
    TR_L3_HASH_KEY_TYPE_LMEP = 4,
    TR_L3_HASH_KEY_TYPE_RMEP = 5,
    TR_L3_HASH_KEY_TYPE_TRILL = 6,
    TR_L3_HASH_KEY_TYPE_COUNT
};

enum TR_L2_HASH_KEY_TYPE {
    /* WARNING: values given match hardware register; do not modify */
    TR_L2_HASH_KEY_TYPE_BRIDGE = 0,
    TR_L2_HASH_KEY_TYPE_SINGLE_CROSS_CONNECT = 1,
    TR_L2_HASH_KEY_TYPE_DOUBLE_CROSS_CONNECT = 2,
    TR_L2_HASH_KEY_TYPE_VFI = 3,
    TR_L2_HASH_KEY_TYPE_VIF = 4,
    TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS = 5,
    TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG = 6,
    TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT = 7,
    TR_L2_HASH_KEY_TYPE_COUNT
};

enum TR_VLXL_HASH_KEY_TYPE {
    /* WARNING: values given match hardware register; do not modify */
    TR_VLXLT_HASH_KEY_TYPE_IVID_OVID = 0,
    TR_VLXLT_HASH_KEY_TYPE_OTAG = 1,
    TR_VLXLT_HASH_KEY_TYPE_ITAG = 2,
    TR_VLXLT_HASH_KEY_TYPE_VLAN_MAC = 3,
    TR_VLXLT_HASH_KEY_TYPE_OVID = 4,
    TR_VLXLT_HASH_KEY_TYPE_IVID = 5,
    TR_VLXLT_HASH_KEY_TYPE_PRI_CFI = 6,
    TR_VLXLT_HASH_KEY_TYPE_HPAE = 7,    /* Not supported until TR2 */
    TR_VLXLT_HASH_KEY_TYPE_VIF = 8,
    TR_VLXLT_HASH_KEY_TYPE_VIF_VLAN = 9,
    TR_VLXLT_HASH_KEY_TYPE_COUNT
};

#define	XGS_HASH_KEY_SIZE	10
#define	ER_L3V6_HASH_KEY_SIZE	16
#define SOC_LYNX_DMUX_MASK      0x3f    /* Lynx Mux/Demux only uses 6 bits */

#define SOC_MPLS_ENTRY_BUCKET_SIZE     8
#define SOC_VLAN_XLATE_BUCKET_SIZE     8
#define SOC_EGR_VLAN_XLATE_BUCKET_SIZE 8
#define SOC_VLAN_MAC_BUCKET_SIZE       8

#endif  /* !_SOC_HASH_H */
