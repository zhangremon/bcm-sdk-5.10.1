/*
 * $Id: hash.c 1.79.2.3 Broadcom SDK $
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

#include <assert.h>

#include <sal/types.h>
#include <sal/core/thread.h>

#include <soc/util.h>
#include <soc/hash.h>
#include <soc/mem.h>
#include <soc/drv.h>
#include <soc/debug.h>

#ifdef BCM_XGS_SWITCH_SUPPORT
uint32
soc_crc32b(uint8 *data, int data_nbits)
{
    uint32 rv;
    rv = _shr_crc32b(0, data, data_nbits);
    rv = _shr_bit_rev_by_byte_word32(rv);
    return rv;
}

uint16
soc_crc16b(uint8 *data, int data_nbits)
{
    uint16 rv;
    rv = _shr_crc16b(0, data, data_nbits);
    rv = _shr_bit_rev16(rv);
    return rv;
}

int 
soc_dual_hash_recurse_depth_get(int unit, soc_mem_t mem)
{
    switch(mem) {
    case L2Xm: return SOC_DUAL_HASH_MOVE_MAX_L2X(unit) ? 
        SOC_DUAL_HASH_MOVE_MAX_L2X(unit) : SOC_DUAL_HASH_MOVE_MAX(unit);
#if defined(BCM_TRX_SUPPORT)
    case MPLS_ENTRYm: return SOC_DUAL_HASH_MOVE_MAX_MPLS(unit) ?
         SOC_DUAL_HASH_MOVE_MAX_MPLS(unit) : SOC_DUAL_HASH_MOVE_MAX(unit);
    case EGR_VLAN_XLATEm: return SOC_DUAL_HASH_MOVE_MAX_EGRESS_VLAN(unit) ?
         SOC_DUAL_HASH_MOVE_MAX_EGRESS_VLAN(unit) : SOC_DUAL_HASH_MOVE_MAX(unit);
    case VLAN_XLATEm: return SOC_DUAL_HASH_MOVE_MAX_VLAN(unit) ?
         SOC_DUAL_HASH_MOVE_MAX_VLAN(unit) : SOC_DUAL_HASH_MOVE_MAX(unit);
#endif
#if defined(BCM_RAVEN_SUPPORT)
    case VLAN_MACm: return SOC_DUAL_HASH_MOVE_MAX_VLAN(unit) ?
         SOC_DUAL_HASH_MOVE_MAX_VLAN(unit) : SOC_DUAL_HASH_MOVE_MAX(unit);
#endif
#if defined(INCLUDE_L3)
    case L3_DEFIPm:
    case L3_ENTRY_ONLYm:
    case L3_ENTRY_IPV4_UNICASTm:
    case L3_ENTRY_IPV4_MULTICASTm:
    case L3_ENTRY_IPV6_UNICASTm:
    case L3_ENTRY_IPV6_MULTICASTm: return SOC_DUAL_HASH_MOVE_MAX_L3X(unit) ?
         SOC_DUAL_HASH_MOVE_MAX_L3X(unit) : SOC_DUAL_HASH_MOVE_MAX(unit);
#endif
    default: return SOC_DUAL_HASH_MOVE_MAX(unit);
    }
}

/*
 * Implement the crc32 routines so that the bit ordering matches Draco
 */

uint32
soc_draco_crc32(uint8 *data, int data_size)
{
    uint32 rv;
    rv = _shr_crc32(0, data, data_size);
    rv = _shr_bit_rev_by_byte_word32(rv);
    return rv;
}

uint16
soc_draco_crc16(uint8 *data, int data_size)
{
    uint16 rv;
    rv = _shr_crc16(0, data, data_size);
    rv = _shr_bit_rev16(rv);
    return rv;
}

/* Compute the "CRC32" of the 76-bit keys used by 5695. */
/* The 5695 CRC32 hashing algorithm is different from   */
/* that implemented in the 5690.                        */

uint32 
soc_draco15_key76_crc32(uint8 *key, int unused_data_size)
{
    uint32 crc, nibble;
    int nibnum;
    static uint32 crc32_d15_table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c, 
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    }; 

    /* This routine processes 76 bits. Ignore datasize parameter. */
    COMPILER_REFERENCE(unused_data_size);

    crc = 0;
    for (nibnum = 0; nibnum < 19; nibnum++) {
        nibble = ((uint32)key[nibnum / 2] << ((nibnum & 1) ? 24 : 28)) 
            & 0xf0000000;
        crc = (crc >> 4) ^ nibble ^ crc32_d15_table[crc & 0x0000000f];
    }
    return _shr_bit_rev32(crc);
}

/* Compute the "CRC16" of the 76-bit keys used by 5695. */
/* The 5695 CRC16 hashing algorithm is different from   */
/* that implemented in the 5690.                        */

uint16 
soc_draco15_key76_crc16(uint8 *key, int unused_data_size)
{
    uint16 crc, nibble;
    int nibnum;
    static uint16 crc16_d15_table[16] = {
        0x0000, 0xcc01, 0xd801, 0x1400, 0xf001, 0x3c00, 0x2800, 0xe401, 
        0xa001, 0x6c00, 0x7800, 0xb401, 0x5000, 0x9c01, 0x8801, 0x4400 
    };

    /* This routine processes 76 bits. Ignore datasize parameter. */
    COMPILER_REFERENCE(unused_data_size);

    crc = 0;
    for (nibnum = 0; nibnum < 19; nibnum++) {
        nibble = ((uint16)key[nibnum / 2] << ((nibnum & 1) ? 8 : 12))
            & 0xf000;
        crc = (crc >> 4) ^ nibble ^ crc16_d15_table[crc & 0x000f];
    }
    return _shr_bit_rev16(crc);
}

/*
 * And now some routines to deal with details
 */

void
soc_draco_l2x_base_entry_to_key(int unit, l2x_entry_t *entry, uint8 *key)
{
    sal_mac_addr_t mac;
    int vid;

    soc_L2Xm_mac_addr_get(unit, entry, MAC_ADDRf, mac);

    vid = soc_L2Xm_field32_get(unit, entry, VLAN_IDf);

    soc_draco_l2x_param_to_key(mac, vid, key);
}

void
soc_draco_l2x_param_to_key(sal_mac_addr_t mac, int vid, uint8 *key)
{
    int ix;

    key[0] = 0;
    for (ix = 0; ix < 6; ix++) {
        key[ix + 1] = (mac[5 - ix] >> 4) & 0x0f;
        key[ix + 0] |= (mac[5 - ix] << 4) & 0xf0;
    }

    key[6] |= (vid << 4) & 0xf0;
    key[7] = (vid >> 4) & 0xff;
}

uint32
soc_draco_l2_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l2x == 0) {
        uint32  mask;
        int     bits;

        mask = soc_mem_index_max(unit, L2X_VALIDm);
        bits = 0;
        rv = 1;
        while (rv && (mask & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_l2x = mask;
        SOC_CONTROL(unit)->hash_bits_l2x = bits;
    }

    switch (hash_sel) {
    case XGS_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 8);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l2x;
        break;

    case XGS_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 8);
        break;

    case XGS_HASH_LSB:
        rv = ((uint32)key[0] >> 4) | ((uint32)key[1] << 4);
        break;

    case XGS_HASH_ZERO:
        rv = 0;
        break;

    case XGS_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 8);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l2x;
        break;

    case XGS_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 8);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_draco_l2_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l2x;
}

void
soc_draco_l3x_base_entry_to_key(int unit, l3x_entry_t *entry,
                                int key_src_ip0, uint8 *key)
{
    ip_addr_t ip, src_ip;
    uint16 vid;

    ip = soc_L3Xm_field32_get(unit, entry, IP_ADDRf);
    src_ip = key_src_ip0 ? 0 : soc_L3Xm_field32_get(unit, entry, SRC_IP_ADDRf);
    vid = 0;

    /*
     * (S,G,V) key is either (DIP + SIP + VID) or (DIP + SIP + 0),
     * depending on IPMC_DO_VLANf field. Regardless, always 76-bit key.
     */
    
    if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
        /* We need to be be l3 sync'd here.  Likely */
        if (SOC_CONTROL(unit)->hash_key_config & L3X_IPMC_VLAN) {
                vid = soc_L3Xm_field32_get(unit, entry, VLAN_IDf);
        }        
    }

    soc_draco_l3x_param_to_key(ip, src_ip, vid, key);
}

void
soc_draco_l3x_param_to_key(ip_addr_t ip, ip_addr_t src_ip,
                           uint16 vid, uint8 *key)
{
    int ix;

    for (ix = 0; ix < 4; ix++) {
        key[ix] = (ip >> (8*ix)) & 0xff;
        key[ix + 4] = (src_ip >> (8*ix)) & 0xff;
    }

    key[8] = vid & 0xff;
    key[9] = (vid >> 8) & 0x0f;
}

uint32
soc_draco_l3_hash(int unit, int hash_sel, int ipmc, uint8 *key)
{
    uint32 rv;
    int bytes;
    uint8 *eff_key = key;
    uint8 local_key[XGS_HASH_KEY_SIZE];

    uint16 (*selected_crc16_routine)(uint8 *, int) = &soc_draco_crc16;
    uint32 (*selected_crc32_routine)(uint8 *, int) = &soc_draco_crc32;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l3x == 0) {
        uint32  mask;
        int     bits;

        mask = soc_mem_index_max(unit, L3X_VALIDm);
        bits = 0;
        rv = 1;
        while (rv && (mask & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_l3x = mask;
        SOC_CONTROL(unit)->hash_bits_l3x = bits;
    }

    if (ipmc) {
        if (soc_feature(unit, soc_feature_l3_sgv_aisb_hash)) {
            /* Draco1.5 IPMC (DIP+SIP+VID) */
            /* For IPMC, the 5695 hashes a 76 bit key. In addition, */
            /* the hashes are not the same as Draco CRC16 or CRC32  */
            selected_crc16_routine = &soc_draco15_key76_crc16;
            selected_crc32_routine = &soc_draco15_key76_crc32;
            bytes = 10; /* 76 bits, actually. */
        } else if (soc_feature(unit, soc_feature_l3_sgv)) {
            int ix;

            /* Now we have to shift up the key to properly align for CRC's */
            for (ix = 9; ix > 0; ix--) {
                local_key[ix] = (key[ix]  << 4) & 0xf0;
                local_key[ix] |= (key[ix - 1] >> 4) & 0x0f;
            }

            local_key[0] = (key[0] << 4) & 0xf0;
            bytes = 10; /* 76 bits, actually. */
            eff_key = local_key;
        }
        else {
            bytes = 8; /* IPMC (DIP+SIP) */
        }
    } else {
        bytes = 4; /* Unicast IP */
    }

    switch (hash_sel) {
    case XGS_HASH_CRC16_UPPER:
        rv = selected_crc16_routine(eff_key, bytes);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l3x;
        return rv & SOC_CONTROL(unit)->hash_mask_l3x;

    case XGS_HASH_CRC16_LOWER:
        rv = selected_crc16_routine(eff_key, bytes);
        return rv & SOC_CONTROL(unit)->hash_mask_l3x;

    case XGS_HASH_LSB:
        rv = ((uint32)eff_key[2] << 16) | 
            ((uint32)eff_key[1] << 8) | eff_key[0];
        return rv & SOC_CONTROL(unit)->hash_mask_l3x;

    case XGS_HASH_ZERO:
        return 0;

    case XGS_HASH_CRC32_UPPER:
        rv = selected_crc32_routine(eff_key, bytes);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l3x;
        return rv & SOC_CONTROL(unit)->hash_mask_l3x;

    case XGS_HASH_CRC32_LOWER:
        rv = selected_crc32_routine(eff_key, bytes);
        return rv & SOC_CONTROL(unit)->hash_mask_l3x;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_draco_l3_hash: invalid hash_sel %d\n",
                     hash_sel);
        return 0;
    }
}

/*
 * ecmp_count is zero based, i.e. it is 0 when there is one ECMP path
 */
uint32
soc_draco_l3_ecmp_hash(int unit, ip_addr_t dip, ip_addr_t sip, int ecmp_count,
                         int ecmp_hash_sel, int l3_hash_sel)
{
    uint8 key[XGS_HASH_KEY_SIZE];
    uint8 local_key[XGS_HASH_KEY_SIZE];
    uint8 *eff_key = key;
    int i, bytes, rv;

    uint16 (*selected_crc16_routine)(uint8 *, int) = &soc_draco_crc16;
    uint32 (*selected_crc32_routine)(uint8 *, int) = &soc_draco_crc32;

    /*
     * HASH_CONTROL.ECMP_HASH_SELECT = 0, use DIP + SIP (default case)
     * HASH_CONTROL.ECMP_HASH_SELECT = 1, then use SIP only
     */
    if (ecmp_hash_sel == 0) {
        bytes = 10;   /* 76 bits actually */

        /*
         * key[9] [8] ([7] [6] [5] [4]) ([3] [2] [1] [0])
         *     0   0      SIP in HEX       DIP in HEX
         */
        for (i = 0; i < 4; i++) {
            key[i]   = (dip >> (8*i)) & 0xff;  /* key[0-31]=DIP */
            key[i+4] = (sip >> (8*i)) & 0xff;  /* key[32-63]=SIP */
        }
        key[8] = key[9] = 0;                   /* key[75:64] = 0 */

        if (soc_feature(unit, soc_feature_l3_sgv_aisb_hash)) { /* Draco15 A0/A1 */
            selected_crc16_routine = &soc_draco15_key76_crc16;
            selected_crc32_routine = &soc_draco15_key76_crc32;
        } else if (soc_feature(unit, soc_feature_l3_sgv)) {  /* Draco15 B0 */
            /* Shift the key to properly align for CRC's */
            for (i = 9; i > 0; i--) {
                local_key[i]  = (key[i]  << 4) & 0xf0;
                local_key[i] |= (key[i - 1] >> 4) & 0x0f;
            }
            local_key[0] = (key[0] << 4) & 0xf0;
            eff_key = local_key;
        }
    } else {
        bytes = 4;    /* 32 bits */
        for (i = 0; i < 4; i++) {
            key[i] = (sip >> (8*i)) & 0xff;   /* key[0-31]=SIP */
        }
        for (i = 5; i < XGS_HASH_KEY_SIZE; i++) {
            key[i] = 0;
        }
    }

    switch (l3_hash_sel) {
    case XGS_HASH_CRC16_UPPER:
        rv = selected_crc16_routine(eff_key, bytes);
        rv >>= 6;   /* use the upper 10 bits */
        break;

    case XGS_HASH_CRC16_LOWER:
        rv = selected_crc16_routine(eff_key, bytes);
        break;

    case XGS_HASH_LSB:
        rv = ((uint32)key[2] << 16) | ((uint32)key[1] << 8) | key[0];
        break;

    case XGS_HASH_ZERO:
        rv = 0;
        break;

    case XGS_HASH_CRC32_UPPER:
        rv = selected_crc32_routine(eff_key, bytes);
        rv >>= 22;   /* use the upper 10 bits */
        break;

    case XGS_HASH_CRC32_LOWER:
        rv = selected_crc32_routine(eff_key, bytes);
        break;

    default:
        soc_cm_debug(DK_ERR,
             "soc_draco_l3_ecmp_hash: invalid l3_hash_sel %d\n", l3_hash_sel);
        return 0;
    }

    /* The ECMP offset is the LS 5 bits of CRC result mod (ecmp_count + 1) */
    return ((rv & 0x1F) % (ecmp_count + 1));
}

/*
 * ecmp_count is zero based, i.e. it is 0 when there is one ECMP path
 */
uint32
soc_lynx_l3_ecmp_hash(int unit, ip_addr_t dip, ip_addr_t sip, int ecmp_count)
{
    uint8 key[XGS_HASH_KEY_SIZE];
    uint8 local_key[XGS_HASH_KEY_SIZE];
    uint8 *eff_key = key;
    int i, bytes, rv;

    /* Initialize ecmp key. */
    soc_draco_l3x_param_to_key(dip, sip, 0, key);

    /* 5674 device use 76 bit key shifted by 4 bits. */
    if (soc_feature(unit, soc_feature_l3_sgv)) {  /* BCM 5674 */
        /* Shift the key 4 bits align for CRC's */
        for (i = 9; i > 0; i--) {
            local_key[i]  = (key[i] << 4) & 0xf0;
            local_key[i] |= (key[i - 1] >> 4) & 0x0f;
        }
        local_key[0] = (key[0] << 4) & 0xf0;
        eff_key = local_key;
        bytes = 10;   /* 76 bits actually */
    } else /* BCM5673 crc32(sip,dip) */{
        bytes = 8;   
    }
    rv = soc_draco_crc32(eff_key, bytes);

    /* The ECMP offset is the LS 5 bits of CRC result mod (ecmp_count + 1) */
    return (((rv & 0xFF) % (ecmp_count + 1)) << 3);
}


uint32
soc_tucana_l3_ecmp_hash(int unit, ip_addr_t dip, ip_addr_t sip, int
                        ecmp_count)
{
    uint8 key[XGS_HASH_KEY_SIZE];
    uint32 rv;

    /* Initialize ecmp key. */
    /*
     * For Tucana, ecmp_key[63:0] = {SIP, DIP}
     * key([7] [6] [5] [4]) ([3] [2] [1] [0])
     *        SIP in HEX       DIP in HEX
     */
    soc_draco_l3x_param_to_key(dip, sip, 0, key);

    /* Tucana always uses CRC16 for ECMP hash calculation */
    rv = soc_draco_crc16(key, 8);

    /*
     * From Tucana Theory of Operations :
     *  - ecmp_count is zero based, i.e. it is 0 when
     there is one ECMP path,
     *  - ECMP_OFFSET = (((CRC result) & 0xFF) % (ECMP_COUNT+1)) << 3
     *  - FINAL_L3_TABLE_INDEX = LPM->L3_TABLE_INDEX
     ECMP_OFFSET
     */
    return (((rv & 0xFF) % (ecmp_count + 1)) << 3);
}

#ifdef BCM_XGS3_SWITCH_SUPPORT
/*
 * Get the XGS3 ECMP hash result based on HW
 */
uint32 
soc_xgs3_l3_ecmp_hash(int unit, soc_xgs3_ecmp_hash_t *data)
{
    uint8    key[SOC_MAX_MEM_WORDS];
    uint8    use_l4_port = 0;
    uint8    use_dip;
    uint8    hash_sel;
    uint8    crc_val_shift;
    uint32   crc_val;
    uint32   regval;
    int      index;
    int      idx; 
    uint8    udf;
    uint16   mask;

    if (NULL == data) {
        return (SOC_E_PARAM);
    }

    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &regval));
    hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                 regval, ECMP_HASH_SELf);

    /* If hash is disabled return 0. */
    if (FB_HASH_ZERO == hash_sel) {
        return 0;
    }

    /* If hash is based on LSB of sip. */
    if (FB_HASH_LSB == hash_sel) {
        if(data->v6) {
            crc_val = data->sip6[0] & 0x1f;
        }  else {
            crc_val = data->sip & 0x1f;
        }
        return ((crc_val & 0x1F) % (data->ecmp_count + 1));
    }

    use_dip = soc_reg_field_get(unit, HASH_CONTROLr,
                                 regval, ECMP_HASH_USE_DIPf);

    udf = soc_reg_field_get(unit, HASH_CONTROLr,
                            regval, ECMP_HASH_UDFf);

    if (SOC_REG_FIELD_VALID(unit, HASH_CONTROLr, USE_TCP_UDP_PORTSf)) {
        use_l4_port = soc_reg_field_get(unit, HASH_CONTROLr,
                                        regval, USE_TCP_UDP_PORTSf);
    } 
#if defined(BCM_EASYRIDER_SUPPORT)    
    else if (SOC_REG_FIELD_VALID(unit, HASH_CONTROLr,
                                   ECMP_HASH_NO_TCP_UDP_PORTSf)){
        use_l4_port = !(soc_reg_field_get(unit, HASH_CONTROLr,
                                        regval, ECMP_HASH_NO_TCP_UDP_PORTSf));
    }
#endif /* BCM_EASYRIDER_SUPPORT */

    /* Initialize key structure. */
    sal_memset(key, 0, SOC_MAX_MEM_WORDS * sizeof (uint8));

    /*KEY FORMAT IS UDF[12] DST_PORT[10-11] SRC_PORT[8-9] DIP[4-7] SIP[0-3]*/
    if (data->v6) {
        /* IP[32] = IP[0-31] ^ IP[32-63] ^ IP[64-95] ^ IP[96-127] */
        for (idx = 0; idx < 4; idx++) {
            for (index = 3 - idx; index < 16; index += 4) {
                key[idx] ^= data->sip6[index]  & 0xff;
                if (use_dip) {
                    key[idx + 4] ^= data->dip6[index] & 0xff;
                }
            }
        }
    } else {
        for (idx = 0; idx < 4; idx++) {
            key[idx] = (data->sip >> (8*idx)) & 0xff;
            if (use_dip) {
                key[idx + 4] = (data->dip >> (8*idx)) & 0xff;
            }
        }
    }

    if (use_l4_port) {
        for (idx = 0; idx < 2; idx++) {
            key[idx + 8] = (data->l4_src_port >> (8*idx)) & 0xff;
            key[idx + 10] = (data->l4_dst_port >> (8*idx)) & 0xff;
        }
    }

    key[12] = udf & 0xff;
   
    /* XGS3 always uses CRC32 for ECMP hash calculation */
    crc_val = soc_draco_crc32(key, 13);

    crc_val_shift = 27;
    mask = 0x1F;
    if (SOC_IS_APOLLO(unit) || SOC_IS_TRIUMPH2(unit)) {
        crc_val_shift = 24;
        mask = 0xFF;
    } else if (SOC_IS_TD_TT(unit)) {
        crc_val_shift = 20;
        mask = 0xFFF;
    }

    if (FB_HASH_CRC32_UPPER == hash_sel) {
        return ((crc_val >> crc_val_shift) % (data->ecmp_count + 1));
    } else if (FB_HASH_CRC32_LOWER == hash_sel) {
        return ((crc_val & mask) % (data->ecmp_count + 1));
    }
    /* Hopefully never reached. */
    return (SOC_E_INTERNAL);
}
#endif /* BCM_XGS3_SWITCH_SUPPORT */

uint32
soc_draco_trunk_hash(sal_mac_addr_t da, sal_mac_addr_t sa, int tgsize)
{
    uint8 key[12];
    int ix;
    uint32 crc32;

    if (tgsize <= 0 || tgsize > 8) {
        return 0;
    }

    for (ix = 0; ix < 6; ix++) {
        key[ix + 0] = da[5 - ix];
    }

    for (ix = 0; ix < 6; ix++) {
        key[ix + 6] = sa[5 - ix];
    }

    crc32 = soc_draco_crc32(key, 12);

    return (crc32 & 0xffff) % tgsize;
}

int
soc_draco_hash_set(int unit, int hash_sel)
{
    uint32              hash_control, ohash;

    assert(hash_sel >= 0 && hash_sel <= 5);

    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &hash_control));
    ohash = hash_control;
    soc_reg_field_set(unit, HASH_CONTROLr, &hash_control,
                      HASH_SELECTf, hash_sel);
    if (hash_control != ohash) {
        SOC_IF_ERROR_RETURN(WRITE_HASH_CONTROLr(unit, hash_control));
    }

    return SOC_E_NONE;
}

int
soc_draco_hash_get(int unit, int *hash_sel)
{
    uint32              hash_control;

    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &hash_control));

    *hash_sel = soc_reg_field_get(unit, HASH_CONTROLr, hash_control,
                                  HASH_SELECTf);

    if (*hash_sel > 5) {
        *hash_sel = 5;
    }

    return SOC_E_NONE;
}

/* This next functions is a mess.  But that's what the HW is doing. */
uint32
soc_lynx_dmux_entry(uint32 *data)
{
    uint32 rv, local_data;
    uint8 key[16];
    int ix;

    /* Is this really endian safe? */
    for (ix = 0; ix < 3; ix++) {
        local_data = _shr_swap32(data[ix]);
        memcpy(&(key[4*ix]), &local_data, 4);
    }
    local_data = 0;
    memcpy(&(key[12]), &local_data, 4);

    rv = _shr_crc32(0, key, 16);
    rv = _shr_bit_rev_by_byte_word32(rv);

    return rv & SOC_LYNX_DMUX_MASK;
}

#ifdef BCM_XGS3_SWITCH_SUPPORT
#define GEN_KEY2(k, ks, k1)                              \
    k[ks + 0] |= (k1 << 4) & 0xf0;                       \
    k[ks + 1] = (k1 >> 4) & 0xff

#define GEN_KEY3(k, ks, k1)                              \
    k[ks + 0] |= (k1 << 4) & 0xf0;                       \
    k[ks + 1] = (k1 >> 4) & 0xff;                        \
    k[ks + 2] = (k1 >> 12) & 0xff;                       \
    k[ks + 3] = ((k1 >> 20) & 0x0f)

#define GEN_KEY4(k, ks, k1)                              \
    k[ks + 0] |= (k1 << 4) & 0xf0;                       \
    k[ks + 1] = (k1 >> 4) & 0xff;                        \
    k[ks + 2] = (k1 >> 12) & 0xff;                       \
    k[ks + 3] = (k1 >> 20) & 0xff;                       \
    k[ks + 4] = ((k1 >> 28) & 0x0f)
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT

uint32
soc_fb_l2_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l2x == 0) {
        uint32  mask, mask_bits;
        int     bits;

        /* Get the effective table max for the hash mask */
        mask = soc_mem_index_max(unit, L2_HITDA_ONLYm);
        /* Need the maximum table size for the shift bits */
        mask_bits = SOC_MEM_INFO(unit, L2_HITDA_ONLYm).index_max;
        bits = 0;
        rv = 1;
        while (rv && (mask_bits & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_l2x = mask;
        SOC_CONTROL(unit)->hash_bits_l2x = bits;
    }

    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 8);
        if (SOC_IS_HAWKEYE(unit)) {
            rv >>= 16 - (SOC_CONTROL(unit)->hash_bits_l2x + 1);
        } else {
            rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l2x;
        }
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 8);
        break;

    case FB_HASH_LSB:
        rv = (((uint32)key[0] >> 4) | ((uint32)key[1] << 4) );
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 8);
        if (SOC_IS_HAWKEYE(unit)) {
            rv >>= 32 - (SOC_CONTROL(unit)->hash_bits_l2x + 1);
        } else {
            rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l2x;
        }
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 8);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_fb_l2_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l2x;
}

uint32
soc_fb_l3_hash(int unit, int hash_sel, int key_nbits, uint8 *key)
{
    uint32 rv;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l3x == 0) {
        uint32  mask, mask_bits;
        int     bits;

        /* Get the effective table max for the hash mask */
        mask = soc_mem_index_max(unit, L3_ENTRY_HIT_ONLYm);
        /* Need the maximum table size for the shift bits */
        mask_bits = SOC_MEM_INFO(unit, L3_ENTRY_HIT_ONLYm).index_max;
        bits = 0;
        rv = 1;
        while (rv && (mask_bits & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_l3x = mask;
        SOC_CONTROL(unit)->hash_bits_l3x = bits;
    }

    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_crc16b(key, key_nbits);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l3x;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_crc16b(key, key_nbits);
        break;

    case FB_HASH_LSB:
        rv = ((uint32)key[0]) | ((uint32)key[1] << 8) | ((uint32)key[2] << 16);
        /* See _soc_fb_l3x_realign_key() to understand the shift values below */
        if ((key_nbits == 80)  || (key_nbits == 264)) {
                rv >>= 4;
        } else if ((key_nbits == 40)  || (key_nbits == 136)) {
                rv >>= 2;
        } else if ((key_nbits == 88)  || (key_nbits == 272)) {
                rv >>= 6;
        } else if ((key_nbits == 96)  || (key_nbits == 280)) {
#if defined (BCM_TRIUMPH_SUPPORT)
            if(SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV4_MULTICASTm, L3_IIFf)) {
                rv >>= (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) ? 5 : 6;
            } else 
#endif /* BCM_TRX_SUPPORT */
            {
                rv >>= 7;
            }
        } else if ((key_nbits == 48)  || (key_nbits == 144)) {
#if defined (BCM_ENDURO_SUPPORT)
           if(SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
               rv >>= 2;
           } else
#endif /* BCM_ENDURO_SUPPORT */
           {
                rv >>= 3;
            }
        }
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_crc32b(key, key_nbits);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l3x;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_crc32b(key, key_nbits);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_fb_l3_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l3x;
}

uint32
soc_fb_vlan_mac_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_vlan_mac == 0) {
        uint32  mask;
        int     bits;

        /* 4 Entries per bucket */
        mask = soc_mem_index_max(unit, VLAN_MACm) >> 2;
        bits = 0;
        rv = 1;
        while (rv && (mask & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_vlan_mac = mask;
        SOC_CONTROL(unit)->hash_bits_vlan_mac = bits;
    }

    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 6);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_vlan_mac;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 6);
        break;

    case FB_HASH_LSB:
        /* Extract more than required 8 bits. Masked below anyway */
        rv = (((uint32)key[0]) | ((uint32)key[1] << 8));
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 6);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_vlan_mac;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 6);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_fb_vlan_mac_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_vlan_mac;
}

/* In Hurricane, VRF_ID field is invalid, but the hashing algorithm retains the space
*   in the key, and expects it to be 0s. So we have special handling for Hu
*/
STATIC int
_soc_hash_generic_entry_to_key(int unit, void *entry, uint8 *key,
                               soc_mem_t mem, soc_field_t *field_list)
{
    soc_field_t field;
    int         index, key_index, val_index, fval_index;
    int         right_shift_count, left_shift_count;
    uint32      val[SOC_MAX_MEM_WORDS], fval[SOC_MAX_MEM_WORDS];
    int         bits, val_bits, fval_bits;
    int8        field_length[16];

    val_bits = 0;
    for (index = 0; field_list[index] != INVALIDf; index++) {
        field = field_list[index];
        if (field == NUM_SOC_FIELD || !SOC_MEM_FIELD_VALID(unit, mem, field)) {
#ifdef BCM_HURRICANE_SUPPORT
            if(SOC_IS_HURRICANE(unit) && (field == VRF_IDf)) {
                field_length[index] = 11;
                val_bits += field_length[index];
            }
#endif
            continue;
        }
        field_length[index] = soc_mem_field_length(unit, mem, field);
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit) && (field == PORT_GROUP_IDf)) {
            field_length[index] = 13;
        }
#endif
        val_bits += field_length[index];
    }

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_HURRICANE(unit) ||
        SOC_IS_TD_TT(unit)||SOC_IS_KATANA(unit)) {
        if (mem == L2Xm || mem == VLAN_MACm) {
            if (soc_feature(unit, soc_feature_trill)) {
                val_bits = 60 + /* VLAN_ID + MAC_ADDRESS */
                    soc_mem_field_length(unit, L2Xm, KEY_TYPEf) +
                    soc_mem_field_length(unit, L2Xm,
                                         TRILL_NONUC_NETWORK_LONG__TREE_IDf);
            } else {
                val_bits = 60 + /* VLAN_ID + MAC_ADDRESS */
                    soc_mem_field_length(unit, L2Xm, KEY_TYPEf);
            }
        } else if (mem == L3_ENTRY_ONLYm ||
                 mem == L3_ENTRY_IPV4_UNICASTm ||
                 mem == L3_ENTRY_IPV4_MULTICASTm ||
                 mem == L3_ENTRY_IPV6_UNICASTm) {
            /* Pad to the longest hashing key (L3_ENTRY_IPV6_MULTICAST) */
            val_bits = 120 + /* GROUP_IP_ADDR_* */
                128 + /* SOURCE_IP_ADDR_* */
                soc_mem_field_length(unit, L3_ENTRY_IPV6_MULTICASTm, L3_IIFf) +
                (SOC_IS_HURRICANE(unit) ? 11 : soc_mem_field_length(unit, L3_ENTRY_IPV6_MULTICASTm, VRF_IDf)) +
                soc_mem_field_length(unit, L3_ENTRY_IPV6_MULTICASTm,
                                     KEY_TYPE_0f);
        } else if (mem == VLAN_XLATEm) {
            /* Pad to the longest hashing key (VLAN_MAC) */
            val_bits = 48 + /* MAC_ADDR */
                soc_mem_field_length(unit, VLAN_MACm, KEY_TYPEf);
#ifdef BCM_HURRICANE_SUPPORT
        if(SOC_IS_HURRICANE(unit)) {
            val_bits = 41;
        }
#endif
        } else if (mem == EGR_VLAN_XLATEm) {
            /* Pad to the longest hashing key (WLAN SVP) */
            if (SOC_IS_TD_TT(unit)) {
                val_bits =
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, ENTRY_TYPEf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, OVIDf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, DST_MODIDf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, DST_PORTf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, IVIDf);
            } else if (SOC_IS_KATANA(unit)) { 
                val_bits =
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, ENTRY_TYPEf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, OVIDf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, PORT_GROUP_IDf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, DUMMY_BITSf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, IVIDf);
            } else if (!SOC_IS_HURRICANE(unit)) {
                val_bits =
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm, ENTRY_TYPEf) +
                    soc_mem_field_length(unit, EGR_VLAN_XLATEm,
                                         WLAN_SVP__KEYf);
            }
        } else if (mem == MPLS_ENTRYm) {
            /* Pad to the longest hashing key (MIM_NVP) */
            val_bits = soc_mem_field_length(unit, MPLS_ENTRYm, KEY_TYPEf) +
                soc_mem_field_length(unit, MPLS_ENTRYm, MIM_NVP__BVIDf) +
                soc_mem_field_length(unit, MPLS_ENTRYm, MIM_NVP__BMACSAf);
        }
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit)) {
        if (mem == MPLS_ENTRYm) {
            /* Pad to the longest hashing key (MIM_NVP) */
            val_bits = soc_mem_field_length(unit, MPLS_ENTRYm, KEY_TYPEf) +
                soc_mem_field_length(unit, MPLS_ENTRYm, MIM_NVP__BVIDf) +
                soc_mem_field_length(unit, MPLS_ENTRYm, MIM_NVP__BMACSAf);
        }        
    } else
#endif /* BCM_ENDURO_SUPPORT */
    {
        if (mem == VLAN_XLATEm) {
            /* Pad to the longest hashing key (KEY_TYPE 0) */
            val_bits =
                soc_mem_field_length(unit, VLAN_XLATEm, KEY_TYPEf) +
                soc_mem_field_length(unit, VLAN_XLATEm, GLPf) +
                soc_mem_field_length(unit, VLAN_XLATEm, OVIDf) +
                soc_mem_field_length(unit, VLAN_XLATEm, IVIDf);
        }
    }

    bits = (val_bits + 7) & ~0x7;
    sal_memset(val, 0, sizeof(val));
    val_bits = bits - val_bits;
    for (index = 0; field_list[index] != INVALIDf; index++) {
        field = field_list[index];
#ifdef BCM_HURRICANE_SUPPORT
        if (SOC_IS_HURRICANE(unit) && (field == VRF_IDf) ) {
            fval[0] = fval[1] = 0;
            fval_bits = 11;
        } else
#endif
        {
            if (field == NUM_SOC_FIELD || !SOC_MEM_FIELD_VALID(unit, mem, field)) {
                continue;
            }

            soc_mem_field_get(unit, mem, entry, field, fval);
            fval_bits = field_length[index];
        }

        val_index = val_bits >> 5;
        fval_index = 0;
        left_shift_count = val_bits & 0x1f;
        right_shift_count = 32 - left_shift_count;
        val_bits += fval_bits;

        if (left_shift_count) {
            for (; fval_bits > 0; fval_bits -= 32) {
                val[val_index++] |= fval[fval_index] << left_shift_count;
                val[val_index] |= fval[fval_index++] >> right_shift_count;
            }
        } else {
            for (; fval_bits > 0; fval_bits -= 32) {
                val[val_index++] = fval[fval_index++];
            }
        }
    }

    key_index = 0;
    for (val_index = 0; val_bits > 0; val_index++) {
        for (right_shift_count = 0; right_shift_count < 32;
             right_shift_count += 8) {
            if (val_bits <= 0) {
                break;
            }
            key[key_index++] = (val[val_index] >> right_shift_count) & 0xff;
            val_bits -= 8;
        }
    }

    if ((bits + 7) / 8 > key_index) {
        sal_memset(&key[key_index], 0, (bits + 7) / 8 - key_index);
    }

    return bits;
}

STATIC int
_soc_fb_l3x_ip4ucast_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        IP_ADDRf,
        VRF_IDf,
        KEY_TYPEf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key
        (unit, entry, key, L3_ENTRY_IPV4_UNICASTm, field_list);
}

STATIC int
_soc_fb_l3x_ip4mcast_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        GROUP_IP_ADDRf,
        SOURCE_IP_ADDRf,
        VLAN_IDf,
        VRF_IDf,
        KEY_TYPEf,
        KEY_TYPE_0f,
        INVALIDf
    };

    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV4_MULTICASTm, L3_IIFf)) {
        field_list[2] = L3_IIFf;
    }

    return _soc_hash_generic_entry_to_key
        (unit, entry, key, L3_ENTRY_IPV4_MULTICASTm, field_list);
}

STATIC int
_soc_fb_l3x_ip6ucast_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        IP_ADDR_LWR_64f,
        IP_ADDR_UPR_64f,
        VRF_IDf,
        VRF_ID_0f,
        KEY_TYPE_0f,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key
        (unit, entry, key, L3_ENTRY_IPV6_UNICASTm, field_list);
}

STATIC int
_soc_fb_l3x_ip6mcast_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        GROUP_IP_ADDR_LWR_64f,
        GROUP_IP_ADDR_UPR_56f,
        SOURCE_IP_ADDR_LWR_64f,
        SOURCE_IP_ADDR_UPR_64f,
        L3_IIFf,
        VLAN_ID_0f,
        VRF_IDf,
        VRF_ID_0f,
        KEY_TYPE_0f,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key
        (unit, entry, key, L3_ENTRY_IPV6_MULTICASTm, field_list);
}

#ifdef BCM_TRIUMPH2_SUPPORT
STATIC int
_soc_fb_l3x_lmep_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        LMEP__SGLPf,
        LMEP__VIDf,
        KEY_TYPEf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key,
                                          L3_ENTRY_ONLYm, field_list);
}

STATIC int
_soc_fb_l3x_rmep_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        RMEP__SGLPf,
        RMEP__VIDf,
        RMEP__MDLf,
        RMEP__MEPIDf,
        KEY_TYPEf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key,
                                          L3_ENTRY_ONLYm, field_list);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
STATIC int
_soc_fb_l3x_trill_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        TRILL__INGRESS_RBRIDGE_NICKNAMEf,
        TRILL__TREE_IDf,
        KEY_TYPEf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key,
                                          L3_ENTRY_ONLYm, field_list);
}
#endif /* BCM_TRIDENT_SUPPORT */

int
soc_fb_l3x_base_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {
        switch (soc_mem_field32_get(unit, L3_ENTRY_ONLYm, entry, KEY_TYPEf)) {
        case TR_L3_HASH_KEY_TYPE_V4UC:
            return _soc_fb_l3x_ip4ucast_entry_to_key(unit, entry, key);
        case TR_L3_HASH_KEY_TYPE_V4MC:
            return _soc_fb_l3x_ip4mcast_entry_to_key(unit, entry, key);
        case TR_L3_HASH_KEY_TYPE_V6UC:
            return _soc_fb_l3x_ip6ucast_entry_to_key(unit, entry, key);
        case TR_L3_HASH_KEY_TYPE_V6MC:
            return _soc_fb_l3x_ip6mcast_entry_to_key(unit, entry, key);
#ifdef BCM_TRIUMPH2_SUPPORT
        case TR_L3_HASH_KEY_TYPE_LMEP:
            if (!soc_feature(unit, soc_feature_oam)) {
                return 0;
            }
            return _soc_fb_l3x_lmep_entry_to_key(unit, entry, key);
        case TR_L3_HASH_KEY_TYPE_RMEP:
            if (!soc_feature(unit, soc_feature_oam)) {
                return 0;
            }
            return _soc_fb_l3x_rmep_entry_to_key(unit, entry, key);
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_TRIDENT_SUPPORT
        case TR_L3_HASH_KEY_TYPE_TRILL:
            if (!soc_feature(unit, soc_feature_trill)) {
                return 0;
            }
            return _soc_fb_l3x_trill_entry_to_key(unit, entry, key);
#endif /* BCM_TRIDENT_SUPPORT */
        default:
            return 0;
        }
    }

    if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, V6f)) {
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            return _soc_fb_l3x_ip6mcast_entry_to_key(unit, entry, key);
        } else {
            return _soc_fb_l3x_ip6ucast_entry_to_key(unit, entry, key);
        }
    } else {
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            return _soc_fb_l3x_ip4mcast_entry_to_key(unit, entry, key);
        } else {
            return _soc_fb_l3x_ip4ucast_entry_to_key(unit, entry, key);
        }
    }
}

uint32
soc_fb_l3x2_entry_hash(int unit, uint32 *entry)
{
    int             hash_sel;
    uint32          index;
    int             key_nbits;
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    uint32          tmp_hs;

    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &tmp_hs));
    hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                 tmp_hs, L3_HASH_SELECTf);
    key_nbits = soc_fb_l3x_base_entry_to_key(unit, entry, key);
    index = soc_fb_l3_hash(unit, hash_sel, key_nbits, key);

    return index;
}

#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT) \
 || defined(BCM_RAVEN_SUPPORT) || defined(BCM_SCORPION_SUPPORT)
int
soc_fb_l2x_entry_bank_hash_sel_get(int unit, int bank, int *hash_sel)
{
    uint32          tmp_hs;

    *hash_sel = -1;
    if (bank > 0) {
        SOC_IF_ERROR_RETURN(READ_L2_AUX_HASH_CONTROLr(unit, &tmp_hs));
        if (soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr,
                              tmp_hs, ENABLEf)) {
            *hash_sel = soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr,
                                         tmp_hs, HASH_SELECTf);
        }
    }

    if (*hash_sel == -1) {
        SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &tmp_hs));
        *hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                      tmp_hs, L2_AND_VLAN_MAC_HASH_SELECTf);
    }

    return SOC_E_NONE;
}

uint32
soc_fb_l2x_entry_hash(int unit, int hash_sel, uint32 *entry)
{
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    uint32          index;

    soc_draco_l2x_base_entry_to_key(unit, (l2x_entry_t *)entry, key);
    index = soc_fb_l2_hash(unit, hash_sel, key);

    return index;
}

int
soc_fb_l3x_entry_bank_hash_sel_get(int unit, int bank, int *hash_sel)
{
    uint32          tmp_hs;

    *hash_sel = -1;
    if (bank > 0) {
        SOC_IF_ERROR_RETURN(READ_L3_AUX_HASH_CONTROLr(unit, &tmp_hs));
        if (soc_reg_field_get(unit, L3_AUX_HASH_CONTROLr,
                              tmp_hs, ENABLEf)) {
            *hash_sel = soc_reg_field_get(unit, L3_AUX_HASH_CONTROLr,
                                         tmp_hs, HASH_SELECTf);
        }
    }

    if (*hash_sel == -1) {
        SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &tmp_hs));
        *hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                     tmp_hs, L3_HASH_SELECTf);
    }

    return SOC_E_NONE;
}

uint32
soc_fb_l3x_entry_hash(int unit, int hash_sel, uint32 *entry)
{
    int             key_nbits;
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    uint32          index;

    key_nbits = soc_fb_l3x_base_entry_to_key(unit, entry, key);
    index = soc_fb_l3_hash(unit, hash_sel, key_nbits, key);

    return index;
}

int
soc_fb_l3x_bank_entry_hash(int unit, int bank, uint32 *entry)
{
    int        hash_sel;

    SOC_IF_ERROR_RETURN
        (soc_fb_l3x_entry_bank_hash_sel_get(unit, bank, &hash_sel));

    return soc_fb_l3x_entry_hash(unit, hash_sel, entry);
}

#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRIUMPH_SUPPORT || BCM_RAVEN_SUPPORT */

int
soc_fb_rv_vlanmac_hash_sel_get(int unit, int dual, int *hash_sel)
{
    uint32          tmp_hs;

    *hash_sel = -1;
#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        /* From Triumph onwards VLAN_MAC uses VLAN_XLATE hash select reg */
        return soc_tr_vlan_xlate_hash_sel_get(unit, dual, hash_sel);    
    }
#endif
#if defined(BCM_RAVEN_SUPPORT)
    if (dual > 0 && SOC_REG_IS_VALID(unit, VLAN_MAC_AUX_HASH_CONTROLr)) {
        SOC_IF_ERROR_RETURN(READ_VLAN_MAC_AUX_HASH_CONTROLr(unit, &tmp_hs));
        if (soc_reg_field_get(unit, VLAN_MAC_AUX_HASH_CONTROLr,
                              tmp_hs, ENABLEf)) {
            *hash_sel = soc_reg_field_get(unit, VLAN_MAC_AUX_HASH_CONTROLr,
                                         tmp_hs, HASH_SELECTf);
        }
    }
#endif /* BCM_RAVEN_SUPPORT */

    if (*hash_sel == -1) {
        SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &tmp_hs));
        *hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                      tmp_hs, L2_AND_VLAN_MAC_HASH_SELECTf);
    }

    return SOC_E_NONE;
}

#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT

uint32
soc_er_l2_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv = 0;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l2x == 0) {
        uint32  mask;
        int     max;

        max = soc_mem_index_max(unit, L2_ENTRY_INTERNALm);
        if (max < 0) {
            SOC_CONTROL(unit)->hash_mask_l2x = ER_HASH_L2_NO_TABLE;
            SOC_CONTROL(unit)->hash_bits_l2x = -1;
        } else {
            /* 8 Entries per bucket */
            mask = max >> 3;
            rv = 1;
            while (rv && (mask & rv)) {
                rv <<= 1;
            }
            SOC_CONTROL(unit)->hash_mask_l2x = mask;
            SOC_CONTROL(unit)->hash_bits_l2x = ER_HASH_L2_INT_BITS;
        }
    }

    if (SOC_CONTROL(unit)->hash_bits_l2x < 0) {
        return ER_HASH_L2_NO_TABLE;
    }

    /* ER & FB use the same hash encodings */
    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 8);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l2x;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 8);
        break;

    case FB_HASH_LSB:
        rv = (((uint32)key[0] >> 4) | ((uint32)key[1] << 4) );
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 8);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l2x;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 8);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_er_l2_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l2x;
}

uint32
soc_er_l2ext_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv = 0;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l2x_ext == 0) {
        uint32  mask;
        int     bits, max;

        max = soc_mem_index_max(unit, L2_ENTRY_EXTERNALm);
        if (max < 0) {
            SOC_CONTROL(unit)->hash_mask_l2x_ext = ER_HASH_L2_NO_TABLE;
            SOC_CONTROL(unit)->hash_bits_l2x_ext = -1;
        } else {
            /* 4 Entries per bucket */
            mask = max >> 2;
            bits = 0;
            rv = 1;
            while (rv && (mask & rv)) {
                bits += 1;
                rv <<= 1;
            }
            SOC_CONTROL(unit)->hash_mask_l2x_ext = mask;
            SOC_CONTROL(unit)->hash_bits_l2x_ext = bits;
        }
    }

    if (SOC_CONTROL(unit)->hash_bits_l2x_ext < 0) {
        return ER_HASH_L2_NO_TABLE;
    }

    /* ER does not use CRC16 for the external table */
    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc32(key, 8) >> ER_HASH_L2_EXT_CRC16_UPPER_SHIFT;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc32(key, 8) >> ER_HASH_L2_EXT_CRC16_LOWER_SHIFT;
        break;

    case FB_HASH_LSB:
        rv = (((uint32)key[0] >> 4) | ((uint32)key[1] << 4) |
            ((uint32)key[2] << 12) );
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 8);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l2x_ext;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 8);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_er_l2ext_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l2x_ext;
}

uint32
soc_er_l3v4_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv = 0;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l3x == 0) {
        uint32  mask;
        int     bits, max;

        max = soc_mem_index_max(unit, L3_ENTRY_V4m);
        if (max < 0) {
            SOC_CONTROL(unit)->hash_mask_l3x = ER_HASH_L2_NO_TABLE;
            SOC_CONTROL(unit)->hash_bits_l3x = -1;
        } else {
            /* 8 Entries per bucket */
            mask = max >> 3;
            bits = 0;
            rv = 1;
            while (rv && (mask & rv)) {
                bits += 1;
                rv <<= 1;
            }
            SOC_CONTROL(unit)->hash_mask_l3x = mask;
            SOC_CONTROL(unit)->hash_bits_l3x = 12;
        }
    }

    if (SOC_CONTROL(unit)->hash_bits_l3x < 0) {
        return ER_HASH_L2_NO_TABLE;
    }

    /* ER & FB use the same hash encodings */
    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 8);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l3x;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 8);
        break;

    case FB_HASH_LSB:
        rv = (((uint32)key[0] >> 4) | ((uint32)key[1] << 4) );
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 8);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l3x;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 8);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_er_l3v4_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l3x;
}

uint32
soc_er_l3v6_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv = 0;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l3v6 == 0) {
        uint32  mask;
        int     bits, max;

        max = soc_mem_index_max(unit, L3_ENTRY_V6m);
        if (max < 0) {
            SOC_CONTROL(unit)->hash_mask_l3v6 = ER_HASH_L2_NO_TABLE;
            SOC_CONTROL(unit)->hash_bits_l3v6 = -1;
        } else {
            /* 4 Entries per bucket */
            mask = max >> 2;
            bits = 0;
            rv = 1;
            while (rv && (mask & rv)) {
                bits += 1;
                rv <<= 1;
            }
            SOC_CONTROL(unit)->hash_mask_l3v6 = mask;
            SOC_CONTROL(unit)->hash_bits_l3v6 = 12;
        }
    }

    if (SOC_CONTROL(unit)->hash_bits_l3v6 < 0) {
        return ER_HASH_L2_NO_TABLE;
    }

    /* ER & FB use the same hash encodings */
    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 16);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l3v6;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 16);
        break;

    case FB_HASH_LSB:
        rv = (((uint32)key[0]) | ((uint32)key[1] << 8));
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 16);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l3v6;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 16);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_er_l3v6_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l3v6;
}

int
soc_er_l3x_hash(int unit, uint32 *entry, int l3v6)
{
    uint8           key[ER_L3V6_HASH_KEY_SIZE];
    uint32          hash_sel;
    uint32          regval;
    int             index;

    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &regval));

    hash_sel= soc_reg_field_get(unit, HASH_CONTROLr, regval, L3_HASH_SELECTf);

    if (l3v6) {
        soc_er_l3v6_base_entry_to_key(unit, (uint32 *)entry, key);
        index = soc_er_l3v6_hash(unit, hash_sel, key);
    } else {
        soc_er_l3v4_base_entry_to_key(unit, (uint32 *)entry, key);
        index = soc_er_l3v4_hash(unit, hash_sel, key);
    }
    return (index);
}

uint32
soc_er_vlan_mac_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv = 0;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_vlan_mac == 0) {
        uint32  mask;
        int     bits, max;

        /* 8 Entries per bucket */
        max = soc_mem_index_max(unit, VLAN_MAC_ENTRYm) >> 3;
        if (max < 0) {
            SOC_CONTROL(unit)->hash_mask_vlan_mac = ER_HASH_MVL_NO_TABLE;
            SOC_CONTROL(unit)->hash_bits_vlan_mac = -1;
        } else {
            mask = max;
            bits = 0;
            rv = 1;
            while (rv && (mask & rv)) {
                bits += 1;
                rv <<= 1;
            }
            SOC_CONTROL(unit)->hash_mask_vlan_mac = mask;
            SOC_CONTROL(unit)->hash_bits_vlan_mac = bits;
        }
    }

    /* ER & FB use the same hash encodings */
    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 8);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_vlan_mac;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 8);
        break;

    case FB_HASH_LSB:
        rv = (((uint32)key[0] >> 4) | ((uint32)key[1] << 4) );
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 8);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_vlan_mac;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 8);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_er_vlan_mac_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_vlan_mac;
}

uint32
soc_er_my_station_hash(int unit, int hash_sel, uint8 *key)
{
    uint32 rv = 0;

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_my_station == 0) {
        uint32  mask;
        int     bits, max;

        /* 8 Entries per bucket */
        max = soc_mem_index_max(unit, MY_STATIONm) >> 3;
        if (max < 0) {
            SOC_CONTROL(unit)->hash_mask_my_station = ER_HASH_MVL_NO_TABLE;
            SOC_CONTROL(unit)->hash_bits_my_station = -1;
        } else {
            mask = max;
            bits = 0;
            rv = 1;
            while (rv && (mask & rv)) {
                bits += 1;
                rv <<= 1;
            }
            SOC_CONTROL(unit)->hash_mask_my_station = mask;
            SOC_CONTROL(unit)->hash_bits_my_station = bits;
        }
    }

    if (SOC_CONTROL(unit)->hash_bits_my_station == -1) {
        return ER_HASH_MVL_NO_TABLE;
    }

    /* ER & FB use the same hash encodings */
    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_draco_crc16(key, 8);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_my_station;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_draco_crc16(key, 8);
        break;

    case FB_HASH_LSB:
        rv = (((uint32)key[0] >> 4) | ((uint32)key[1] << 4) );
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_draco_crc32(key, 8);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_my_station;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_draco_crc32(key, 8);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_er_my_station_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_my_station;
}

#define GEN_ER_KEY2(k, ks, k1)                           \
    k[ks + 0] |= (k1 << 4) & 0xf0;                       \
    k[ks + 1] = (k1 >> 4) & 0xff;                        \
    k[ks + 2] = (k1 >> 12) & 0x0f  

void
soc_er_l2x_base_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    uint32 mac[SOC_MAX_MEM_FIELD_WORDS];
    uint32 vid = 0;

    soc_mem_field_get(unit, L2_ENTRY_INTERNALm, entry, MAC_ADDRf, mac);
    vid = soc_L2_ENTRY_INTERNALm_field32_get(unit, entry, VLAN_IDf);

    key[0] = 0;
    GEN_KEY4(key, 0, mac[0]);
    GEN_ER_KEY2(key, 4, mac[1]);
    GEN_KEY2(key, 6, vid);
}

void
soc_er_l3v4_base_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    uint32 ip;
    uint32 vrf = 0;

    ip = soc_L3_ENTRY_V4m_field32_get(unit, entry, IP_ADDRf);
    vrf = soc_L3_ENTRY_V4m_field32_get(unit, entry, VRFf);

    key[0] = 0;
    GEN_KEY4(key, 0, ip);
    GEN_KEY2(key, 4, vrf);
    GEN_ER_KEY2(key, 5, 0);
}

#define GEN_ER_KEY4(k, ks, k1)                             \
    k[ks + 0] = (k1) & 0xff;                               \
    k[ks + 1] = ((k1) >> 8) & 0xff;                        \
    k[ks + 2] = ((k1) >> 16) & 0xff;                       \
    k[ks + 3] = ((k1) >> 24) & 0xff

void
soc_er_l3v6_base_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    uint32 ip6[SOC_MAX_MEM_FIELD_WORDS];

    soc_mem_field_get(unit, L3_ENTRY_V6m, entry, IP_ADDRf, ip6);

    GEN_ER_KEY4(key, 0, ip6[0]);
    GEN_ER_KEY4(key, 4, ip6[1]);
    GEN_ER_KEY4(key, 8, ip6[2]);
    GEN_ER_KEY4(key, 12, ip6[3]);
}


#endif /* BCM_EASYRIDER_SUPPORT */

#if defined(BCM_TRIUMPH_SUPPORT) || defined(BCM_SCORPION_SUPPORT)
uint32
soc_tr_l2x_hash(int unit, int hash_sel, int key_nbits, void *base_entry,
                uint8 *key)
{
    uint32 rv;
    uint32 fval[SOC_MAX_MEM_WORDS];

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_l2x == 0) {
        uint32  mask;
        int     bits;

        mask = soc_mem_index_max(unit, L2_HITDA_ONLYm);
        bits = 0;
        rv = 1;
        while (rv && (mask & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_l2x = mask;
        SOC_CONTROL(unit)->hash_bits_l2x = bits;
    }

    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_crc16b(key, key_nbits);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_l2x;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_crc16b(key, key_nbits);
        break;

    case FB_HASH_LSB:
        if (key_nbits == 0) {
            return 0;
        }
        switch (soc_mem_field32_get(unit, L2Xm, base_entry, KEY_TYPEf)) {
        case TR_L2_HASH_KEY_TYPE_BRIDGE:
        case TR_L2_HASH_KEY_TYPE_VFI:
            soc_mem_field_get(unit, L2Xm, base_entry, MAC_ADDRf, fval);
            rv = fval[0];
            break;
        case TR_L2_HASH_KEY_TYPE_SINGLE_CROSS_CONNECT:
            rv = soc_mem_field32_get(unit, L2Xm, base_entry, OVIDf);
            break;
        case TR_L2_HASH_KEY_TYPE_DOUBLE_CROSS_CONNECT:
            rv = soc_mem_field32_get(unit, L2Xm, base_entry, OVIDf) |
                (soc_mem_field32_get(unit, L2Xm, base_entry, IVIDf) <<
                 soc_mem_field_length(unit, L2Xm, OVIDf));
            break;
#ifdef BCM_TRIDENT_SUPPORT
        case TR_L2_HASH_KEY_TYPE_VIF:
            /* use only 12 bit of the 14 bit DST_VIF */
            rv = (soc_mem_field32_get(unit, L2Xm, base_entry,
                                      VIF__DST_VIFf) & 0xfff) |
                (soc_mem_field32_get(unit, L2Xm, base_entry,
                                     VIF__NAMESPACEf) << 12);
            break;
        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS:
            soc_mem_field_get(unit, L2Xm, base_entry,
                              TRILL_NONUC_ACCESS__MAC_ADDRf, fval);
            rv = fval[0];
            break;
        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG:
            soc_mem_field_get(unit, L2Xm, base_entry,
                              TRILL_NONUC_NETWORK_LONG__MAC_ADDRESSf, fval);
            rv = fval[0];
            break;
        case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT:
            rv = soc_mem_field32_get(unit, L2Xm, base_entry,
                                     TRILL_NONUC_NETWORK_SHORT__TREE_IDf) |
                (soc_mem_field32_get(unit, L2Xm, base_entry,
                                     TRILL_NONUC_NETWORK_SHORT__VLAN_IDf) <<
                 soc_mem_field_length(unit, L2Xm,
                                      TRILL_NONUC_NETWORK_SHORT__TREE_IDf));
            rv = 0;
            break;
#endif /* BCM_TRIDENT_SUPPORT */
        default:
            rv = 0;
            break;
        }
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_crc32b(key, key_nbits);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_l2x;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_crc32b(key, key_nbits);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_tr_l2_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_l2x;
}

STATIC int
_soc_tr_l2x_bridge_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        VLAN_IDf,
        MAC_ADDRf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, L2Xm, field_list);
}

STATIC int
_soc_tr_l2x_scc_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        OVIDf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, L2Xm, field_list);
}

STATIC int
_soc_tr_l2x_dcc_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        OVIDf,
        IVIDf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, L2Xm, field_list);
}

STATIC int
_soc_tr_l2x_vfi_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        VLAN_IDf,  /* padded from VFIf */
        MAC_ADDRf,
        INVALIDf
    };
    l2x_entry_t new_entry;
    uint32 vfi;

    sal_memcpy(&new_entry, entry, sizeof(new_entry));
    vfi = soc_mem_field32_get(unit, L2Xm, &new_entry, VFIf);
    soc_mem_field32_set(unit, L2Xm, &new_entry, VLAN_IDf, vfi);

    return _soc_hash_generic_entry_to_key(unit, &new_entry, key, L2Xm,
                                          field_list);
}

#ifdef BCM_TRIDENT_SUPPORT
STATIC int
_soc_tr_l2x_vif_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        VIF__NAMESPACEf,
        VIF__DST_VIFf,
        VIF__Pf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, L2Xm, field_list);
}

STATIC int
_soc_tr_l2x_trill_access_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        TRILL_NONUC_ACCESS__VLAN_IDf,
        TRILL_NONUC_ACCESS__MAC_ADDRf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, L2Xm, field_list);
}

STATIC int
_soc_tr_l2x_trill_network_long_entry_to_key(int unit, uint32 *entry,
                                            uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        TRILL_NONUC_NETWORK_LONG__VLAN_IDf,
        TRILL_NONUC_NETWORK_LONG__MAC_ADDRESSf,
        TRILL_NONUC_NETWORK_LONG__TREE_IDf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, L2Xm, field_list);
}

STATIC int
_soc_tr_l2x_trill_network_short_entry_to_key(int unit, uint32 *entry,
                                             uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        TRILL_NONUC_NETWORK_SHORT__VLAN_IDf,
        MAC_ADDRf, /* zero */
        TRILL_NONUC_NETWORK_SHORT__TREE_IDf,
        INVALIDf
    };
    l2x_entry_t new_entry;
    sal_mac_addr_t mac;

    sal_memcpy(&new_entry, entry, sizeof(new_entry));
    sal_memset(&mac, 0, sizeof(mac));
    soc_mem_mac_addr_set(unit, L2Xm, &new_entry,
                         TRILL_NONUC_NETWORK_LONG__MAC_ADDRESSf, mac);

    return _soc_hash_generic_entry_to_key(unit, &new_entry, key, L2Xm,
                                          field_list);
}
#endif /* BCM_TRIDENT_SUPPORT */

int
soc_tr_l2x_base_entry_to_key(int unit, void *entry, uint8 *key)
{
    switch (soc_mem_field32_get(unit, L2Xm, entry, KEY_TYPEf)) {
    case TR_L2_HASH_KEY_TYPE_BRIDGE:
        return _soc_tr_l2x_bridge_entry_to_key(unit, entry, key);
    case TR_L2_HASH_KEY_TYPE_SINGLE_CROSS_CONNECT:
        return _soc_tr_l2x_scc_entry_to_key(unit, entry, key);
    case TR_L2_HASH_KEY_TYPE_DOUBLE_CROSS_CONNECT:
        return _soc_tr_l2x_dcc_entry_to_key(unit, entry, key);
    case TR_L2_HASH_KEY_TYPE_VFI:
        return _soc_tr_l2x_vfi_entry_to_key(unit, entry, key);
#ifdef BCM_TRIDENT_SUPPORT
    case TR_L2_HASH_KEY_TYPE_VIF:
        if (!soc_feature(unit, soc_feature_niv)) {
            return 0;
        }
        return _soc_tr_l2x_vif_entry_to_key(unit, entry, key);
    case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_ACCESS:
        if (!soc_feature(unit, soc_feature_trill)) {
            return 0;
        }
        return _soc_tr_l2x_trill_access_entry_to_key(unit, entry, key);
    case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_LONG:
        if (!soc_feature(unit, soc_feature_trill)) {
            return 0;
        }
        return _soc_tr_l2x_trill_network_long_entry_to_key(unit, entry, key);
    case TR_L2_HASH_KEY_TYPE_TRILL_NONUC_NETWORK_SHORT:
        if (!soc_feature(unit, soc_feature_trill)) {
            return 0;
        }
        return _soc_tr_l2x_trill_network_short_entry_to_key(unit, entry, key);
#endif /* BCM_TRIDENT_SUPPORT */
    default:
        return 0;
    }
}

uint32
soc_tr_l2x_entry_hash(int unit, int hash_sel, uint32 *entry)
{
    int             key_nbits;
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    uint32          index;

    key_nbits = soc_tr_l2x_base_entry_to_key(unit, entry, key);
    index = soc_tr_l2x_hash(unit, hash_sel, key_nbits, entry, key);

    return index;
}

uint32
soc_tr_l2x_bank_entry_hash(int unit, int bank, uint32 *entry)
{
    int             hash_sel = 0;
    uint32          tmp_hs;

    if (bank > 0) {
        SOC_IF_ERROR_RETURN(READ_L2_AUX_HASH_CONTROLr(unit, &tmp_hs));
        if (soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr,
                              tmp_hs, ENABLEf)) {
            hash_sel = soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr,
                                         tmp_hs, HASH_SELECTf);
        }
    } else {
        SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &tmp_hs));
        hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                     tmp_hs, L2_AND_VLAN_MAC_HASH_SELECTf);
    }
    return soc_tr_l2x_entry_hash(unit, hash_sel, entry);
}

int
soc_tr_vlan_xlate_hash_sel_get(int unit, int bank, int *hash_sel)
{
    uint32          tmp_hs;

    SOC_IF_ERROR_RETURN(READ_VLAN_XLATE_HASH_CONTROLr(unit, &tmp_hs));
    if (bank > 0) {
        *hash_sel = soc_reg_field_get(unit, VLAN_XLATE_HASH_CONTROLr,
                                     tmp_hs, HASH_SELECT_Bf);
    } else {
        *hash_sel = soc_reg_field_get(unit, VLAN_XLATE_HASH_CONTROLr,
                                     tmp_hs, HASH_SELECT_Af);
    }
    return SOC_E_NONE;
}

uint32
soc_tr_vlan_xlate_hash(int unit, int hash_sel, int key_nbits, void *base_entry,
                       uint8 *key)
{
    uint32 rv = 0;
    uint32 fval[SOC_MAX_MEM_WORDS];

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_vlan_mac == 0) {
        uint32  mask;
        int     bits;

        /* 8 Entries per bucket */
        mask = soc_mem_index_max(unit, VLAN_MACm) >> 3;
        bits = 0;
        rv = 1;
        while (rv && (mask & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_vlan_mac = mask;
        SOC_CONTROL(unit)->hash_bits_vlan_mac = bits;
    }

    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_crc16b(key, key_nbits);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_vlan_mac;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_crc16b(key, key_nbits);
        break;

    case FB_HASH_LSB:
        if (key_nbits == 0) {
            return 0;
        }
        switch (soc_mem_field32_get(unit, VLAN_XLATEm, base_entry,
                                    KEY_TYPEf)) {
        case TR_VLXLT_HASH_KEY_TYPE_IVID_OVID:
            rv = soc_mem_field32_get(unit, VLAN_XLATEm, base_entry, OVIDf);
            break;
        case TR_VLXLT_HASH_KEY_TYPE_OTAG:
            rv = soc_mem_field32_get(unit, VLAN_XLATEm, base_entry, OTAGf);
            break;
        case TR_VLXLT_HASH_KEY_TYPE_ITAG:
            rv = soc_mem_field32_get(unit, VLAN_XLATEm, base_entry, ITAGf);
            break;
        case TR_VLXLT_HASH_KEY_TYPE_VLAN_MAC:
            soc_mem_field_get(unit, VLAN_MACm, base_entry, MAC_ADDRf, fval);
            rv = fval[0];
            break;
        case TR_VLXLT_HASH_KEY_TYPE_OVID:
            rv = soc_mem_field32_get(unit, VLAN_XLATEm, base_entry, OVIDf);
            break;
        case TR_VLXLT_HASH_KEY_TYPE_IVID:
            rv = soc_mem_field32_get(unit, VLAN_XLATEm, base_entry, IVIDf);
            break;
        case TR_VLXLT_HASH_KEY_TYPE_PRI_CFI:
            /* Use only the upper 4 bit of OTAG */
            rv = soc_mem_field32_get(unit, VLAN_XLATEm, base_entry, OTAGf) >>
                12;
            break;
        case TR_VLXLT_HASH_KEY_TYPE_HPAE:
            rv = soc_mem_field32_get(unit, VLAN_MACm, base_entry,
                                     MAC_IP_BIND__SIPf);
            break;
        case TR_VLXLT_HASH_KEY_TYPE_VIF:
        case TR_VLXLT_HASH_KEY_TYPE_VIF_VLAN:
            rv = soc_mem_field32_get(unit, VLAN_XLATEm, base_entry,
                                     VIF__SRC_VIFf);
            break;
        default:
            rv = 0;
            break;
        }
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_crc32b(key, key_nbits);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_vlan_mac;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_crc32b(key, key_nbits);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_tr_vlan_xlate_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_vlan_mac;
}

STATIC int
_soc_tr_vlan_xlate_dtag_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        GLPf,
        OVIDf,
        IVIDf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, VLAN_XLATEm,
                                          field_list);
}

STATIC int
_soc_tr_vlan_xlate_otag_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        GLPf,
        OTAGf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, VLAN_XLATEm,
                                          field_list);
}

STATIC int
_soc_tr_vlan_xlate_itag_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        GLPf,
        ITAGf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, VLAN_XLATEm,
                                          field_list);
}

STATIC int
_soc_tr_vlan_mac_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        MAC_ADDRf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, VLAN_MACm,
                                          field_list);
}

STATIC int
_soc_tr_vlan_xlate_ovid_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        GLPf,
        OVIDf,
        IVIDf, /* zero */
        INVALIDf
    };
    vlan_xlate_entry_t new_entry;

    sal_memcpy(&new_entry, entry, sizeof(new_entry));
    soc_mem_field32_set(unit, VLAN_XLATEm, &new_entry, IVIDf, 0);

    return _soc_hash_generic_entry_to_key(unit, &new_entry, key, VLAN_XLATEm,
                                          field_list);
}

STATIC int
_soc_tr_vlan_xlate_ivid_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        GLPf,
        OVIDf, /* zero */
        IVIDf,
        INVALIDf
    };
    vlan_xlate_entry_t new_entry;

    sal_memcpy(&new_entry, entry, sizeof(new_entry));
    soc_mem_field32_set(unit, VLAN_XLATEm, &new_entry, OVIDf, 0);

    return _soc_hash_generic_entry_to_key(unit, &new_entry, key, VLAN_XLATEm,
                                          field_list);
}

STATIC int
_soc_tr_vlan_xlate_pri_cfi_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        GLPf,
        OTAGf, /* OVID portion of this field is zero */
        INVALIDf
    };

    vlan_xlate_entry_t new_entry;

    sal_memcpy(&new_entry, entry, sizeof(new_entry));
    soc_mem_field32_set(unit, VLAN_XLATEm, &new_entry, OVIDf, 0);

    return _soc_hash_generic_entry_to_key(unit, &new_entry, key, VLAN_XLATEm,
                                          field_list);
}

#ifdef BCM_TRIUMPH2_SUPPORT
STATIC int
_soc_tr_vlan_xlate_hpae_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        MAC_IP_BIND__SIPf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, VLAN_MACm,
                                          field_list);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
STATIC int
_soc_tr_vlan_xlate_vif_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        VIF__GLPf,
        VIF__SRC_VIFf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, VLAN_XLATEm,
                                          field_list);
}

STATIC int
_soc_tr_vlan_xlate_vif_vlan_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        VIF__GLPf,
        VIF__SRC_VIFf,
        VIF__VLANf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, VLAN_XLATEm,
                                          field_list);
}
#endif /* BCM_TRIDENT_SUPPORT */

int
soc_tr_vlan_xlate_base_entry_to_key(int unit, void *entry, uint8 *key)
{
    switch (soc_mem_field32_get(unit, VLAN_XLATEm, entry, KEY_TYPEf)) {
    case TR_VLXLT_HASH_KEY_TYPE_IVID_OVID:
        return _soc_tr_vlan_xlate_dtag_entry_to_key(unit, entry, key);
    case TR_VLXLT_HASH_KEY_TYPE_OTAG:
        return _soc_tr_vlan_xlate_otag_entry_to_key(unit, entry, key);
    case TR_VLXLT_HASH_KEY_TYPE_ITAG:
        return _soc_tr_vlan_xlate_itag_entry_to_key(unit, entry, key);
    case TR_VLXLT_HASH_KEY_TYPE_VLAN_MAC:
        return _soc_tr_vlan_mac_entry_to_key(unit, entry, key);
    case TR_VLXLT_HASH_KEY_TYPE_OVID:
        return _soc_tr_vlan_xlate_ovid_entry_to_key(unit, entry, key);
    case TR_VLXLT_HASH_KEY_TYPE_IVID:
        return _soc_tr_vlan_xlate_ivid_entry_to_key(unit, entry, key);
    case TR_VLXLT_HASH_KEY_TYPE_PRI_CFI:
        return _soc_tr_vlan_xlate_pri_cfi_entry_to_key(unit, entry, key);
#ifdef BCM_TRIUMPH2_SUPPORT
    case TR_VLXLT_HASH_KEY_TYPE_HPAE:
        if (!soc_feature(unit, soc_feature_ip_source_bind)) {
            return 0;
        }
        return _soc_tr_vlan_xlate_hpae_entry_to_key(unit, entry, key);
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_TRIDENT_SUPPORT
    case TR_VLXLT_HASH_KEY_TYPE_VIF:
        if (!soc_feature(unit, soc_feature_niv)) {
            return 0;
        }
        return _soc_tr_vlan_xlate_vif_entry_to_key(unit, entry, key);
    case TR_VLXLT_HASH_KEY_TYPE_VIF_VLAN:
        if (!soc_feature(unit, soc_feature_niv)) {
            return 0;
        }
        return _soc_tr_vlan_xlate_vif_vlan_entry_to_key(unit, entry, key);
#endif /* BCM_TRIDENT_SUPPORT */
    default:
        return 0;
    }
}

uint32
soc_tr_vlan_xlate_entry_hash(int unit, int hash_sel, uint32 *entry)
{
    int             key_nbits;
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    uint32          index;

    key_nbits = soc_tr_vlan_xlate_base_entry_to_key(unit, entry, key);

    index = soc_tr_vlan_xlate_hash(unit, hash_sel, key_nbits, entry, key);

    return index;
}

uint32
soc_tr_vlan_xlate_bank_entry_hash(int unit, int bank, uint32 *entry)
{
    int     hash_sel;

    SOC_IF_ERROR_RETURN
        (soc_tr_vlan_xlate_hash_sel_get(unit, bank, &hash_sel));
    return soc_tr_vlan_xlate_entry_hash(unit, hash_sel, entry);
}

int
soc_tr_egr_vlan_xlate_hash_sel_get(int unit, int bank, int *hash_sel)
{
    uint32          tmp_hs;

    SOC_IF_ERROR_RETURN(READ_EGR_VLAN_XLATE_HASH_CONTROLr(unit, &tmp_hs));
    if (bank > 0) {
        *hash_sel = soc_reg_field_get(unit, EGR_VLAN_XLATE_HASH_CONTROLr,
                                     tmp_hs, HASH_SELECT_Bf);
    } else {
        *hash_sel = soc_reg_field_get(unit, EGR_VLAN_XLATE_HASH_CONTROLr,
                                     tmp_hs, HASH_SELECT_Af);
    }
    return SOC_E_NONE;
}

uint32
soc_tr_egr_vlan_xlate_hash(int unit, int hash_sel, int key_nbits,
                           void *base_entry, uint8 *key)
{
    uint32 rv;
    uint32 fval[SOC_MAX_MEM_WORDS];

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_egr_vlan_xlate == 0) {
        uint32  mask;
        int     bits;

        /* 8 Entries per bucket */
        mask = soc_mem_index_max(unit, EGR_VLAN_XLATEm) >> 3;
        bits = 0;
        rv = 1;
        while (rv && (mask & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_egr_vlan_xlate = mask;
        SOC_CONTROL(unit)->hash_bits_egr_vlan_xlate = bits;
    }

    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_crc16b(key, key_nbits);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_egr_vlan_xlate;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_crc16b(key, key_nbits);
        break;

    case FB_HASH_LSB:
        if (key_nbits == 0) {
            return 0;
        }
        if (SOC_MEM_FIELD_VALID(unit, EGR_VLAN_XLATEm, ENTRY_TYPEf)) {
            switch (soc_mem_field32_get(unit, EGR_VLAN_XLATEm, base_entry,
                                        ENTRY_TYPEf)) {
            case 0: /* VLAN_XLATE */
            case 1: /* VLAN_XLATE_DVP */
            case 2: /* VLAN_XLATE_WLAN */
                rv = soc_mem_field32_get(unit, EGR_VLAN_XLATEm, base_entry,
                                         OVIDf);
                break;
            case 3: /* ISID_XLATE */
            case 4: /* ISID_DVP_XLATE */
                rv = soc_mem_field32_get(unit, EGR_VLAN_XLATEm, base_entry,
                                         MIM_ISID__VFIf) |
                    (soc_mem_field32_get(unit, EGR_VLAN_XLATEm, base_entry,
                                         MIM_ISID__DVPf) <<
                     soc_mem_field_length(unit, EGR_VLAN_XLATEm,
                                          MIM_ISID__VFIf));
                break;
            case 5: /* WLAN_SVP_TUNNEL */
                rv = soc_mem_field32_get(unit, EGR_VLAN_XLATEm, base_entry,
                                         WLAN_SVP__TUNNEL_IDf);
                break;
            case 6: /* WLAN_SVP_BSSID */
                soc_mem_field_get(unit, MPLS_ENTRYm, base_entry,
                                  WLAN_SVP__BSSIDf, fval);
                rv = fval[0];
                break;
            case 7: /* WLAN_SVP_BSSID_RID */
                soc_mem_field_get(unit, MPLS_ENTRYm, base_entry,
                                  WLAN_SVP__BSSIDf, fval);
                rv = soc_mem_field32_get(unit, EGR_VLAN_XLATEm, base_entry,
                                         WLAN_SVP__RIDf) |
                    (fval[0] << soc_mem_field_length(unit, EGR_VLAN_XLATEm,
                                                     WLAN_SVP__RIDf));
                break;
            default:
                rv = 0;
                break;
            }
        } else {
            rv = soc_mem_field32_get(unit, EGR_VLAN_XLATEm, base_entry, OVIDf);
        }
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_crc32b(key, key_nbits);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_egr_vlan_xlate;
        break;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_crc32b(key, key_nbits);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_tr_vlan_xlate_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_egr_vlan_xlate;
}

STATIC int
_soc_tr_egr_vlan_xlate_xlate_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        ENTRY_TYPEf,
        OVIDf,
        PORT_GROUP_IDf,
        NUM_SOC_FIELD, /* skip mark */
        IVIDf,
        INVALIDf
    };

    if (SOC_IS_TD_TT(unit)) {
        field_list[2] = DST_MODIDf;
        field_list[3] = DST_PORTf;
    } else if (SOC_IS_KATANA(unit)) {
        field_list[3] = DUMMY_BITSf; 
    } else if (SOC_MEM_FIELD_VALID(unit, EGR_VLAN_XLATEm, DVPf)) {
        field_list[2] = DVPf;
    }

    return _soc_hash_generic_entry_to_key(unit, entry, key, EGR_VLAN_XLATEm,
                                          field_list);
}

#ifdef BCM_TRIUMPH2_SUPPORT
STATIC int
_soc_tr_egr_vlan_xlate_mim_isid_entry_to_key(int unit, uint32 *entry,
                                             uint8 *key)
{
    static soc_field_t field_list[] = {
        ENTRY_TYPEf,
        MIM_ISID__VFIf,
        MIM_ISID__DVPf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, EGR_VLAN_XLATEm,
                                          field_list);
}

STATIC int
_soc_tr_egr_vlan_xlate_wlan_svp_entry_to_key(int unit, uint32 *entry,
                                             uint8 *key)
{
    static soc_field_t field_list[] = {
        ENTRY_TYPEf,
        WLAN_SVP__RIDf,
        WLAN_SVP__BSSIDf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, EGR_VLAN_XLATEm,
                                          field_list);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

int
soc_tr_egr_vlan_xlate_base_entry_to_key(int unit, void *entry, uint8 *key)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, EGR_VLAN_XLATEm, ENTRY_TYPEf)) {
        switch (soc_mem_field32_get(unit, EGR_VLAN_XLATEm, entry,
                                    ENTRY_TYPEf)) {
        case 0: /* VLAN_XLATE */
        case 1: /* VLAN_XLATE_DVP */
            return _soc_tr_egr_vlan_xlate_xlate_entry_to_key(unit, entry, key);
        case 2: /* VLAN_XLATE_WLAN */
            if (!soc_feature(unit, soc_feature_wlan)) {
                return 0;
            }
            return _soc_tr_egr_vlan_xlate_xlate_entry_to_key(unit, entry, key);
        case 3: /* ISID_XLATE */
        case 4: /* ISID_DVP_XLATE */
            if (!soc_feature(unit, soc_feature_mim)) {
                return 0;
            }
            return _soc_tr_egr_vlan_xlate_mim_isid_entry_to_key(unit, entry,
                                                                key);
        case 5: /* WLAN_SVP_TUNNEL */
        case 6: /* WLAN_SVP_BSSID */
        case 7: /* WLAN_SVP_BSSID_RID */
            if (!soc_feature(unit, soc_feature_wlan)) {
                return 0;
            }
            return _soc_tr_egr_vlan_xlate_wlan_svp_entry_to_key(unit, entry,
                                                                key);
        default:
            return 0;
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    return _soc_tr_egr_vlan_xlate_xlate_entry_to_key(unit, entry, key);
}

uint32
soc_tr_egr_vlan_xlate_entry_hash(int unit, int hash_sel, uint32 *entry)
{
    int             key_nbits;
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    uint32          index;

    key_nbits = soc_tr_egr_vlan_xlate_base_entry_to_key(unit, entry, key);
    index = soc_tr_egr_vlan_xlate_hash(unit, hash_sel, key_nbits, entry, key);

    return index;
}

uint32
soc_tr_egr_vlan_xlate_bank_entry_hash(int unit, int bank, uint32 *entry)
{
    int     hash_sel;

    soc_tr_egr_vlan_xlate_hash_sel_get(unit, bank, &hash_sel); 
    return soc_tr_egr_vlan_xlate_entry_hash(unit, hash_sel, entry);
}

int
soc_tr_mpls_hash_sel_get(int unit, int bank, int *hash_sel)
{
    uint32          tmp_hs;

    SOC_IF_ERROR_RETURN(READ_MPLS_ENTRY_HASH_CONTROLr(unit, &tmp_hs));
    if (bank > 0) {
        *hash_sel = soc_reg_field_get(unit, MPLS_ENTRY_HASH_CONTROLr,
                                     tmp_hs, HASH_SELECT_Bf);
    } else {
        *hash_sel = soc_reg_field_get(unit, MPLS_ENTRY_HASH_CONTROLr,
                                     tmp_hs, HASH_SELECT_Af);
    }
    return SOC_E_NONE;
}

uint32
soc_tr_mpls_hash(int unit, int hash_sel, int key_nbits, void *base_entry,
                 uint8 *key)
{
    uint32 rv;
    uint32 fval[SOC_MAX_MEM_WORDS];

    /*
     * Cache bucket mask and shift amount for upper crc
     */
    if (SOC_CONTROL(unit)->hash_mask_mpls == 0) {
        uint32  mask;
        int     bits;

        /* 8 Entries per bucket */
        mask = soc_mem_index_max(unit, MPLS_ENTRYm) >> 3;
        bits = 0;
        rv = 1;
        while (rv && (mask & rv)) {
            bits += 1;
            rv <<= 1;
        }
        SOC_CONTROL(unit)->hash_mask_mpls = mask;
        SOC_CONTROL(unit)->hash_bits_mpls = bits;
    }

    switch (hash_sel) {
    case FB_HASH_CRC16_UPPER:
        rv = soc_crc16b(key, key_nbits);
        rv >>= 16 - SOC_CONTROL(unit)->hash_bits_mpls;
        break;

    case FB_HASH_CRC16_LOWER:
        rv = soc_crc16b(key, key_nbits);
        break;

    case FB_HASH_LSB:
        if (key_nbits == 0) {
            return 0;
        }
        if (SOC_MEM_FIELD_VALID(unit, MPLS_ENTRYm, KEY_TYPEf)) {
            switch (soc_mem_field32_get(unit, MPLS_ENTRYm, base_entry,
                                        KEY_TYPEf)) {
            case 0: /* MPLS */
                rv = soc_mem_field32_get(unit, MPLS_ENTRYm, base_entry,
                                         MPLS_LABELf);
                break;
            case 1: /* MIM_NVP */
                soc_mem_field_get(unit, MPLS_ENTRYm, base_entry,
                                  MIM_NVP__BMACSAf, fval);
                rv = fval[0];
                break;
            case 2: /* MIM_ISID */
            case 3: /* MIM_ISID_SVP */
                rv = soc_mem_field32_get(unit, MPLS_ENTRYm, base_entry,
                                         MIM_ISID__ISIDf);
                break;
            case 4: /* WLAN_MAC */
                soc_mem_field_get(unit, MPLS_ENTRYm, base_entry,
                                  WLAN_MAC__MAC_ADDRf, fval);
                rv = fval[0];
                break;
            case 5: /* TRILL */
                rv = soc_mem_field32_get(unit, MPLS_ENTRYm, base_entry,
                                         TRILL__RBRIDGE_NICKNAMEf);
                break;
            default:
                rv = 0;
                break;
            }
        } else {
            rv = soc_mem_field32_get(unit, MPLS_ENTRYm, base_entry,
                                     MPLS_LABELf);
        }
        break;

    case FB_HASH_ZERO:
        rv = 0;
        break;

    case FB_HASH_CRC32_UPPER:
        rv = soc_crc32b(key, key_nbits);
        rv >>= 32 - SOC_CONTROL(unit)->hash_bits_mpls;
        break;

    case FB_HASH_CRC32_LOWER:
        rv = soc_crc32b(key, key_nbits);
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_tr_mpls_hash: invalid hash_sel %d\n",
                     hash_sel);
        rv = 0;
        break;
    }

    return rv & SOC_CONTROL(unit)->hash_mask_mpls;
}

STATIC int
_soc_tr_mpls_legacy_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        PORT_NUMf,
        MODULE_IDf,
        Tf,
        MPLS_LABELf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, MPLS_ENTRYm,
                                          field_list);
}

#ifdef BCM_TRIUMPH2_SUPPORT
STATIC int
_soc_tr_mpls_mim_nvp_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        MIM_NVP__BVIDf,
        MIM_NVP__BMACSAf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, MPLS_ENTRYm,
                                          field_list);
}

STATIC int
_soc_tr_mpls_mim_isid_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        MIM_ISID__ISIDf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, MPLS_ENTRYm,
                                          field_list);
}

STATIC int
_soc_tr_mpls_mim_isid_svp_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        MIM_ISID__ISIDf,
        MIM_ISID__SVPf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, MPLS_ENTRYm,
                                          field_list);
}

STATIC int
_soc_tr_mpls_wlan_mac_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        WLAN_MAC__MAC_ADDRf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, MPLS_ENTRYm,
                                          field_list);
}
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
STATIC int
_soc_tr_mpls_trill_entry_to_key(int unit, uint32 *entry, uint8 *key)
{
    static soc_field_t field_list[] = {
        KEY_TYPEf,
        TRILL__RBRIDGE_NICKNAMEf,
        INVALIDf
    };

    return _soc_hash_generic_entry_to_key(unit, entry, key, MPLS_ENTRYm,
                                          field_list);
}
#endif /* BCM_TRIDENT_SUPPORT */

int
soc_tr_mpls_base_entry_to_key(int unit, void *entry, uint8 *key)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, MPLS_ENTRYm, KEY_TYPEf)) {
        switch (soc_mem_field32_get(unit, MPLS_ENTRYm, entry, KEY_TYPEf)) {
        case 0: /* MPLS */
            return _soc_tr_mpls_legacy_entry_to_key(unit, entry, key);
        case 1: /* MIM_NVP */
            if (!soc_feature(unit, soc_feature_mim)) {
                return 0;
            }
            return _soc_tr_mpls_mim_nvp_entry_to_key(unit, entry, key);
        case 2: /* MIM_ISID */
            if (!soc_feature(unit, soc_feature_mim)) {
                return 0;
            }
            return _soc_tr_mpls_mim_isid_entry_to_key(unit, entry, key);
        case 3: /* MIM_ISID_SVP */
            if (!soc_feature(unit, soc_feature_mim)) {
                return 0;
            }
            return _soc_tr_mpls_mim_isid_svp_entry_to_key(unit, entry, key);
        case 4: /* WLAN_MAC */
            if (!soc_feature(unit, soc_feature_wlan)) {
                return 0;
            }
            return _soc_tr_mpls_wlan_mac_entry_to_key(unit, entry, key);
#ifdef BCM_TRIDENT_SUPPORT
        case 5: /* TRILL */
            if (!soc_feature(unit, soc_feature_trill)) {
                return 0;
            }
            return _soc_tr_mpls_trill_entry_to_key(unit, entry, key);
#endif /* BCM_TRIDENT_SUPPORT */
        default:
            return 0;
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    return _soc_tr_mpls_legacy_entry_to_key(unit, entry, key);

}

uint32
soc_tr_mpls_entry_hash(int unit, int hash_sel, uint32 *entry)
{
    int             key_nbits;
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    uint32          index;

    key_nbits = soc_tr_mpls_base_entry_to_key(unit, entry, key);
    index = soc_tr_mpls_hash(unit, hash_sel, key_nbits, entry, key);

    return index;
}

uint32
soc_tr_mpls_bank_entry_hash(int unit, int bank, uint32 *entry)
{
    int     hash_sel;
    
    SOC_IF_ERROR_RETURN
        (soc_tr_mpls_hash_sel_get(unit, bank, &hash_sel));
    return soc_tr_mpls_entry_hash(unit, hash_sel, entry);
}
#endif /* BCM_TRIUMPH_SUPPORT || BCM_SCORPION_SUPPORT */
#endif /* BCM_XGS_SWITCH_SUPPORT */
