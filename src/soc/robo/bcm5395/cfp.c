/*
 * $Id: cfp.c 1.36.64.1 Broadcom SDK $
 *
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
#include <assert.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/drv_if.h>
#include <soc/cfp.h>

/* Ram select code for bcm5395 */
#define CFP_5395_RAM_SEL_OB_STAT 0x10
#define CFP_5395_RAM_SEL_IB_STAT 0x8
#define CFP_5395_RAM_SEL_METER 0x4
#define CFP_5395_RAM_SEL_ACT 0x2
#define CFP_5395_RAM_SEL_TCAM 0x1

/* OP code */
#define CFP_5395_OP_NONE 0x0
#define CFP_5395_OP_READ 0x1
#define CFP_5395_OP_WRITE 0x2
#define CFP_5395_OP_SEARCH 0x4

/* Flood and drop value for BCM5395 */
#define CFP_5395_DEST_FLOOD 0x3f
#define CFP_5395_DEST_DROP 0x0

/* User defined fields for BCM5395 */ 
#define CFP_5395_UDF_NUM_MAX 9
#define CFP_5395_UDF_OFFSET_MAX 80

/* 1Gb */
#define CFP_5395_METER_RATE_MAX (1000 * 1000)
/* 64Kbits/sec */
#define CFP_5395_METER_RATE_MIN 64
/* 128Mb */
#define CFP_5395_METER_BURST_MAX (128 * 1000)   
/* 64K */
#define CFP_5395_METER_BURST_MIN 64


/* Slice 0~4 qualify set */
static int s0_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_8023_OR_EII,
                                  DRV_CFP_QUAL_8022_LLC,
                                  DRV_CFP_QUAL_8022_SNAP,
                                  DRV_CFP_QUAL_MAC_DA,
                                  DRV_CFP_QUAL_MAC_SA,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_ETYPE,
                                  DRV_CFP_QUAL_UDF0,
                                  DRV_CFP_QUAL_INVALID};
static int s1_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_8023_OR_EII,
                                  DRV_CFP_QUAL_8022_LLC,
                                  DRV_CFP_QUAL_8022_SNAP,
                                  DRV_CFP_QUAL_MAC_DA,
                                  DRV_CFP_QUAL_MAC_SA,
                                  DRV_CFP_QUAL_IPV4,
                                  DRV_CFP_QUAL_IP_DA,
                                  DRV_CFP_QUAL_IP_SA,
                                  DRV_CFP_QUAL_UDP,
                                  DRV_CFP_QUAL_TCP,
                                  DRV_CFP_QUAL_TCP_FLAG,
                                  DRV_CFP_QUAL_INVALID};
static int s2_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_8023_OR_EII,
                                  DRV_CFP_QUAL_8022_LLC,
                                  DRV_CFP_QUAL_8022_SNAP,
                                  DRV_CFP_QUAL_MAC_SA,
                                  DRV_CFP_QUAL_ETYPE,
                                  DRV_CFP_QUAL_IPV4,
                                  DRV_CFP_QUAL_IP_DA,
                                  DRV_CFP_QUAL_IP_SA,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_L4_DST,
                                  DRV_CFP_QUAL_L4_SRC,
                                  DRV_CFP_QUAL_INVALID};
static int s3_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_8023_OR_EII,
                                  DRV_CFP_QUAL_8022_LLC,
                                  DRV_CFP_QUAL_8022_SNAP,
                                  DRV_CFP_QUAL_MAC_DA,
                                  DRV_CFP_QUAL_ETYPE,
                                  DRV_CFP_QUAL_IPV4,
                                  DRV_CFP_QUAL_IP_DA,
                                  DRV_CFP_QUAL_IP_SA,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_L4_DST,
                                  DRV_CFP_QUAL_L4_SRC,
                                  DRV_CFP_QUAL_INVALID};


static int s4_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_8023_OR_EII,
                                  DRV_CFP_QUAL_8022_LLC,
                                  DRV_CFP_QUAL_8022_SNAP,
                                  DRV_CFP_QUAL_ETYPE,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_IP_PROTO,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP_VER,
                                  DRV_CFP_QUAL_IP_TOS,
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP_TTL,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
                                  DRV_CFP_QUAL_UDF4A,
                                  DRV_CFP_QUAL_UDF4B,
                                  DRV_CFP_QUAL_UDF4C,
                                  DRV_CFP_QUAL_INVALID};
static int s5_qset[] = { DRV_CFP_QUAL_SRC_PBMP,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_8023_OR_EII,
                                  DRV_CFP_QUAL_8022_LLC,
                                  DRV_CFP_QUAL_8022_SNAP,
                                  DRV_CFP_QUAL_UDF5A,
                                  DRV_CFP_QUAL_UDF5B,
                                  DRV_CFP_QUAL_UDF5C,
                                  DRV_CFP_QUAL_UDF5D,
                                  DRV_CFP_QUAL_UDF5E,
                                  DRV_CFP_QUAL_INVALID};

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? BYTES2WORDS((m)->bytes)-1-(v) : (v))

/*
 * Function: _drv_cfp_field_mapping
 *
 * Purpose:
 *     Translate the driver field type to chip field index.
 *
 * Parameters:
 *     unit - BCM device number
 *     field_type - driver field value
 *     field_id (OUT) - chip field index
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_BADID - unknown driver field value
 */
int
_drv5395_cfp_field_mapping(int unit, uint32 field_type, uint32 *field_id)
{
    int rv = SOC_E_NONE;
   
    switch (field_type) {
        case DRV_CFP_FIELD_VALID:
            *field_id = VALID_Rf;
            break;
        case DRV_CFP_FIELD_SLICE_ID:
            *field_id = SLICEIDf;
            break;
        case DRV_CFP_FIELD_SRC_PORT:
            *field_id = SRC_PMAPf;
            break;
        case DRV_CFP_FIELD_IN_PBMP:
            *field_id = SRC_PMAPf;
            break;
        case DRV_CFP_FIELD_1QTAGGED:
            *field_id = VLANTAGGEDf;
            break;
        case DRV_CFP_FIELD_SPTAGGED:
            *field_id = SPTAGGEDf;
            break;
        case DRV_CFP_FIELD_EII_OR_8023:
            *field_id = EII_OR_8023f;
            break;
        case DRV_CFP_FIELD_BRCM_TAGGED:
            *field_id = BRCM_TAGGEDf;
            break;
        case DRV_CFP_FIELD_IEEE_LLC:
            *field_id = IEEE_802_2_LLCf;
            break;
        case DRV_CFP_FIELD_IEEE_SNAP:
            *field_id = IEEE_802_2_SNAPf;
            break;
        case DRV_CFP_FIELD_MAC_DA:
            *field_id = MAC_DAf;
            break;
        case DRV_CFP_FIELD_MAC_SA:
            *field_id = MAC_SAf;
            break;
        case DRV_CFP_FIELD_SP_VID:
            *field_id = SP_VIDf;
            break;
        case DRV_CFP_FIELD_USR_VID:
            *field_id = USR_VIDf;
            break;
         case DRV_CFP_FIELD_USR_PRI:
            *field_id = USR_PRIf;
            break;
         case DRV_CFP_FIELD_USR_CFI:
            *field_id = USR_CFIf;
            break;
        case DRV_CFP_FIELD_ETYPE:
            *field_id = ETYPEf;
            break;
        case DRV_CFP_FIELD_UDF0_VALID:
            *field_id = UDF0_VLDf;
            break;
        case DRV_CFP_FIELD_UDF0:
            *field_id = UDF0f;
            break;
        case DRV_CFP_FIELD_IPV4_VALID:
            *field_id = IPV4_VALIDf;
            break;
        case DRV_CFP_FIELD_IP_DA:
            *field_id = IP_DAf;
            break;
        case DRV_CFP_FIELD_IP_SA:
            *field_id = IP_SAf;
            break;
        case DRV_CFP_FIELD_SAME_IP:
            *field_id = SAMEIPADDRf;
            break;
        case DRV_CFP_FIELD_TCPUDP_VALID:
            *field_id = TCP_UDP_VALIDf;
            break;
        case DRV_CFP_FIELD_UDP_VALID:
            *field_id = UDP_VALIDf;
            break;
        case DRV_CFP_FIELD_TCP_VALID:
            *field_id = TCP_VALIDf;
            break;
        case DRV_CFP_FIELD_L4DST:
            *field_id = L4DSTf;
            break;
        case DRV_CFP_FIELD_L4SRC:
            *field_id = L4SRCf;
            break;
        case DRV_CFP_FIELD_SAME_L4PORT:
            *field_id = SAMEL4PORTf;
            break;
        case DRV_CFP_FIELD_L4SRC_LESS1024:
            *field_id = L4SRC_LESS_1024f;
            break;
        case DRV_CFP_FIELD_TCP_FRAME:
            *field_id = TCP_VALIDf;
            break;
        case DRV_CFP_FIELD_TCP_SEQ_ZERO:
            *field_id = TCP_SEQUENCE_ZEROf;
            break;
        case DRV_CFP_FIELD_TCP_HDR_LEN:
            *field_id = TCP_HEADER_LENf;
            break;
        case DRV_CFP_FIELD_TCP_FLAG:
            *field_id = TCP_FLAGf;
            break;
        case DRV_CFP_FIELD_IP_PROTO:
            *field_id = IP_PROTOCOLf;
            break;
        case DRV_CFP_FIELD_IP_VER:
            *field_id = IP_VERSIONf;
            break;
        case DRV_CFP_FIELD_IP_TOS:
            *field_id = IP_TOSf;
            break;
        case DRV_CFP_FIELD_IP_TTL:
            *field_id = IP_TTLf;
            break;
        case DRV_CFP_FIELD_UDF4A_VALID:
            *field_id = UDF4A_VLDf;
            break;
        case DRV_CFP_FIELD_UDF4A:
            *field_id = UDF4Af;
            break;
        case DRV_CFP_FIELD_UDF4B_VALID:
            *field_id = UDF4B_VLDf;
            break;
        case DRV_CFP_FIELD_UDF4B:
            *field_id = UDF4Bf;
            break;
        case DRV_CFP_FIELD_UDF4C_VALID:
            *field_id = UDF4C_VLDf;
            break;
        case DRV_CFP_FIELD_UDF4C:
            *field_id = UDF4Cf;
            break;
        case DRV_CFP_FIELD_UDF5A_VALID:
            *field_id = UDF5A_VLDf;
            break;
        case DRV_CFP_FIELD_UDF5A:
            *field_id = UDF5Af;
            break;
        case DRV_CFP_FIELD_UDF5B_VALID:
            *field_id = UDF5B_VLDf;
            break;
        case DRV_CFP_FIELD_UDF5B:
            *field_id = UDF5Bf;
            break;
        case DRV_CFP_FIELD_UDF5C_VALID:
            *field_id = UDF5C_VLDf;
            break;
        case DRV_CFP_FIELD_UDF5C:
            *field_id = UDF5Cf;
            break;
        case DRV_CFP_FIELD_UDF5D_VALID:
            *field_id = UDF5D_VLDf;
            break;
        case DRV_CFP_FIELD_UDF5D:
            *field_id = UDF5Df;
            break;
        case DRV_CFP_FIELD_UDF5E_VALID:
            *field_id = UDF5E_VLDf;
            break;
        case DRV_CFP_FIELD_UDF5E:
            *field_id = UDF5Ef;
            break;
        /* ACT_POLm */
        case DRV_CFP_FIELD_MOD_PRI_EN_IB:
            *field_id = CHANGE_TC_IBf;
            break;
        case DRV_CFP_FIELD_MOD_PRI_MAP_IB:
            *field_id = NEW_TC_IBf;
            break;
        case DRV_CFP_FIELD_MOD_TOS_EN_IB:
            *field_id = CHANGE_TOS_DSCP_IBf;
            break;
        case DRV_CFP_FIELD_MOD_TOS_IB:
            *field_id = NEW_TOS_DSCP_IBf;
            break;
        case DRV_CFP_FIELD_REDIRECT_EN_IB:
            *field_id = CHANGE_FWRD_MAP_IBf;
            break;
        case DRV_CFP_FIELD_ADD_CHANGE_DEST_IB:
            *field_id = COPY_REDIRECT_DEST_PORT_IBf;
            break;
        case DRV_CFP_FIELD_NEW_DEST_IB:
            *field_id = NEW_DEST_IBf;
            break;
        case DRV_CFP_FIELD_MOD_PRI_EN_OB:
            *field_id = CHANGE_TC_OBf;
            break;
        case DRV_CFP_FIELD_MOD_PRI_MAP_OB:
            *field_id = NEW_TC_OBf;
            break;
        case DRV_CFP_FIELD_REDIRECT_EN_OB:
            *field_id = CHANGE_FWRD_MAP_OBf;
            break;
        case DRV_CFP_FIELD_ADD_CHANGE_DEST_OB:
            *field_id = COPY_REDIRECT_DEST_PORT_OBf;
            break;
        case DRV_CFP_FIELD_NEW_DEST_OB:
            *field_id = NEW_DEST_PORT_OBf;
            break;
         case DRV_CFP_FIELD_MOD_TOS_EN_OB:
            *field_id = CHANGE_TOS_DSCP_OBf;
            break;
        case DRV_CFP_FIELD_MOD_TOS_OB:
            *field_id = NEW_TOS_DSCP_OBf;
            break;
        /* METER */
        case DRV_CFP_FIELD_CURR_QUOTA:
            *field_id = CURR_QUOTAf;
            break;
        case DRV_CFP_FIELD_RATE_REFRESH_EN:
            *field_id = RATE_REFRESH_ENf;
            break;
        case DRV_CFP_FIELD_REF_CAP:
            *field_id = REF_CAPf;
            break;
        case DRV_CFP_FIELD_RATE:
            *field_id = TOKEN_NUMf;
            break;
        case DRV_CFP_FIELD_IB_CNT:
            *field_id = IN_BAND_CNTf;
            break;
        case DRV_CFP_FIELD_OB_CNT:
            *field_id = OUT_BAND_CNTf;
            break;
        default:
            rv = SOC_E_BADID;
    }

    return rv;
}


/*
 * Function: _drv_cfp_meter_rate2chip
 *
 * Purpose:
 *     Translate the driver rate value to register value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_sec - driver rate value
 *     chip_val(OUT) - register value
 *
 * Returns:
 *     Nothing
 */
void
_drv5395_cfp_meter_rate2chip(int unit, uint32 kbits_sec, uint32 *chip_val)
{
    /* case 1 : greater than 100M */
    if (kbits_sec > 1024 * 100 ) {
        *chip_val = (kbits_sec / (1000 * 8)) + 115;
    } else if (kbits_sec > 1792) {
    /* case 2 : greater than 1792K */
        *chip_val = (kbits_sec / 1000 ) + 27;
    } else {
    /* case 3 : the rest (64K ~ 1792K) */
        *chip_val = kbits_sec / 64;
    }
}

/*
 * Function: _drv_cfp_meter_chip2rate
 *
 * Purpose:
 *     Translate the register value to driver rate value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_sec(OUT) - driver rate value
 *     chip_val - register value
 *
 * Returns:
 *     Nothing
 */
void
_drv5395_cfp_meter_chip2rate(int unit, uint32 *kbits_sec, uint32 chip_val)
{
    if (chip_val > 127) {
        *kbits_sec = (chip_val - 115) * 8 * 1000;
    } else if (chip_val > 28) {
        *kbits_sec = (chip_val - 27) * 1000;
    } else {
        *kbits_sec = chip_val * 64;
    }
}

/*
 * Function: _drv_cfp_meter_burst2chip
 *
 * Purpose:
 *     Translate the driver burst value to register value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_burst - driver burst value
 *     chip_val(OUT) - chip value
 *
 * Returns:
 *     Nothing
 */
void
_drv5395_cfp_meter_burst2chip(int unit, uint32 kbits_burst, uint32 *chip_val)
{
    uint32  burst;
    int i;
    
    burst = CFP_5395_METER_BURST_MAX;
    for (i = 0; i < 12; i++) {
        if (kbits_burst >= burst) {
            *chip_val = i;
            break;
        }
        burst = burst / 2;
    }
}

/*
 * Function: _drv_cfp_meter_chip2burst
 *
 * Purpose:
 *     Translate the register value to driver burst value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_burst(OUT) - driver burst value
 *     chip_val - register value
 *
 * Returns:
 *     Nothing
 */
void
_drv5395_cfp_meter_chip2burst(int unit, uint32 *kbits_burst, uint32 chip_val)
{
    uint32 burst = CFP_5395_METER_BURST_MAX;
    int i;

    for (i = 0; i < chip_val ; i++) {
        burst = burst / 2;
    }
    *kbits_burst = burst ;
}

/*
 * Function: _drv_cfp_read
 *
 * Purpose:
 *     Read the CFP raw data by ram type from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry(OUT) -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv5395_cfp_read(int unit, uint32 ram_type, 
                         uint32 index, drv_cfp_entry_t *entry)
{
    int i, rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int index_max, mem_len;
    uint32  *data_p, *mask_p, data_reg_addr, mask_reg_addr;

    assert(entry);
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
            mem_id = CFP_ACT_POLm;
            ram_val = CFP_5395_RAM_SEL_ACT;
            break;
        case DRV_CFP_RAM_METER:
            mem_id = CFP_METERm;
            ram_val = CFP_5395_RAM_SEL_METER;
            break;
        case DRV_CFP_RAM_TCAM:
            assert(entry->tcam_mask);
            mem_id = CFP_TCAM_S0m;
            ram_val = CFP_5395_RAM_SEL_TCAM;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    soc_cm_debug(DK_VERBOSE, 
        "_drv_cfp_read : mem_id = %d, ram_val = %d, index_max = %d\n",
        mem_id, ram_val, index_max);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform TCAM read operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_read_exit;
    }
    fld_val = CFP_5395_OP_READ;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OP_SELf, &fld_val);

    fld_val = ram_val;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, RAM_SELf, &fld_val);

    fld_val = index;    
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);

    fld_val = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_read_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_read_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_read_exit;
    }

    switch (ram_type) {
    case DRV_CFP_RAM_TCAM:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->tcam_data;
        mask_p = entry->tcam_mask;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_DATAr, 0, 0);
        mask_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_MASKr, 0, 0);
        for (i = 0; i < mem_len; i++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_read_exit;
            }
            *(data_p + i) = reg_val;
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (mask_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_read_exit;
            }
            *(mask_p + i) = reg_val;
        }
        break;
    case DRV_CFP_RAM_ACT:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->act_data;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ACT_POL_DATA0r, 0, 0);
        for (i = 0; i < mem_len; i++) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_read_exit;
            }
            *(data_p + i) = reg_val;
        }
        break;
    case DRV_CFP_RAM_METER:
        data_p = entry->meter_data;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_METER0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_read_exit;
        }
        *data_p = reg_val;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_METER1r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_read_exit;
        }
        *(data_p + 1) = reg_val;
        break;
    default:
        rv = SOC_E_UNAVAIL;
        goto cfp_read_exit;
    }

    cfp_read_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv_cfp_stat_read
 *
 * Purpose:
 *     Read the counter raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     counter type - in-band/ out-band
 *     index -entry index
 *     entry(OUT) -counter raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv5395_cfp_stat_read(int unit, uint32 counter_type, 
                                uint32 index, uint32 *counter)
{
    int rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int index_max;
    uint32  data_reg_addr;

    assert(counter);
    
    switch (counter_type) {
        case DRV_CFP_RAM_STAT_IB:
            mem_id = CFP_STAT_IBm;
            ram_val = CFP_5395_RAM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = CFP_STAT_OBm;
            ram_val = CFP_5395_RAM_SEL_OB_STAT;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform read operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_read_exit;
    }
    fld_val = CFP_5395_OP_READ;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OP_SELf, &fld_val);
    
    fld_val = ram_val;     
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, RAM_SELf, &fld_val);

    fld_val = index;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);

    fld_val = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_read_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_read_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_stat_read_exit;
    }
    
    switch (counter_type) {
    case DRV_CFP_RAM_STAT_IB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
           (unit, RATE_INBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_stat_read_exit;
        }
        *counter = reg_val;
        break;
    case DRV_CFP_RAM_STAT_OB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_OUTBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_stat_read_exit;
        }
        *counter = reg_val;
        break;
    default:
        rv = SOC_E_UNAVAIL;
        goto cfp_stat_read_exit;
    }

    cfp_stat_read_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv_cfp_stat_write
 *
 * Purpose:
 *     Set the counter raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     counter type - in-band/ out-band
 *     index -entry index
 *     entry -counter raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv5395_cfp_stat_write(int unit, uint32 counter_type, 
                                      uint32 index, uint32 counter)
{
    int rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int index_max;
    uint32  data_reg_addr, data_reg_val;
    
    switch (counter_type) {
        case DRV_CFP_RAM_STAT_IB:
            mem_id = CFP_STAT_IBm;
            ram_val = CFP_5395_RAM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = CFP_STAT_OBm;
            ram_val = CFP_5395_RAM_SEL_OB_STAT;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform write operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_write_exit;
    }
    fld_val = CFP_5395_OP_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OP_SELf, &fld_val);
    
    fld_val = ram_val;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, RAM_SELf, &fld_val);

    fld_val = index; 
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);

    fld_val = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);

    /* Set counter value */
    data_reg_val = counter;
    switch (counter_type) {
    case DRV_CFP_RAM_STAT_IB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_INBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, &data_reg_val, 4)) < 0) {
            goto cfp_stat_write_exit;
        }
        break;
    case DRV_CFP_RAM_STAT_OB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_OUTBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, &data_reg_val, 4)) < 0) {
            goto cfp_stat_write_exit;
        }
        break;
    }

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_write_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_write_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_stat_write_exit;
    }

    cfp_stat_write_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}


/*
 * Function: _drv_cfp_tcam_search
 *
 * Purpose:
 *     Search the Valid TCAM raw data
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - search flags
 *     index(OUT) -entry index
 *     entry(OUT) -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv5395_cfp_tcam_search(int unit, uint32 flags, uint32* index, 
            drv_cfp_entry_t *entry)
{
    int i, rv = SOC_E_NONE;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int mem_len;
    uint32 src_port;
    uint32  *data_p, *mask_p, data_reg_addr, mask_reg_addr;

    assert(entry);
    assert(index);

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            return rv;
    }

    /* Get SEARCH DONE bit */
    if (flags & DRV_CFP_SEARCH_DONE) {
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);
        *index = (fld_val)? 0 : 1;
        goto cfp_search_exit;
    }

    /* Get valid TCAM entry */
    if(flags & DRV_CFP_SEARCH_GET) {
        /*
         * set error times ???
         */
         
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, SERCH_STSf, &fld_val);
        if (fld_val) {
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);
            *index = fld_val;
            mem_len = soc_mem_entry_words(unit, CFP_TCAM_S0m);
            data_p = entry->tcam_data;
            mask_p = entry->tcam_mask;
            data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_DATAr, 0, 0);
            mask_reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_MASKr, 0, 0);
            for (i = 0; i < mem_len; i++) {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (mask_reg_addr + i * 4), &reg_val, 4)) < 0) {
                    goto cfp_search_exit;
                }
                *(mask_p + i) = reg_val;
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                    goto cfp_search_exit;
                }
                *(data_p + i) = reg_val;
            }
            rv = SOC_E_NONE;
        } else {
            rv = SOC_E_EMPTY;
        }
        goto cfp_search_exit;
    }


    if (flags & DRV_CFP_SEARCH_START) {
        /* Set search op code */
        fld_val = CFP_5395_OP_SEARCH;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, CFP_ACCr, &reg_val, OP_SELf, &fld_val);
        /* Set TCAM */
        fld_val = CFP_5395_RAM_SEL_TCAM;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, CFP_ACCr, &reg_val, RAM_SELf, &fld_val);
        /* Set initial search address */
        fld_val = 0;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);

        /* Set search option (per port or all) */
        if (flags & DRV_MEM_OP_SEARCH_PORT) {
            if ((rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SRC_PORT, 
                    entry, &src_port)) < 0) {
                rv = SOC_E_INTERNAL;
                goto  cfp_search_exit;
            }
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_ACCr, &reg_val, SEARCH_OPTIONf, &src_port);
            
        } else {
            src_port = 0x3f;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_ACCr, &reg_val, SEARCH_OPTIONf, &src_port);
        }
        /* start search */
        fld_val = 1;
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, CFP_ACCr, &reg_val, OP_STR_DONEf, &fld_val);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_search_exit;
        }
    }
    
    cfp_search_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv_cfp_write
 *
 * Purpose:
 *     Write the CFP raw data by ram type to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv5395_cfp_write(int unit, uint32 ram_type, 
                              uint32 index, drv_cfp_entry_t *entry)
{
    int i, rv = SOC_E_NONE;
    uint32  mem_id = 0;
    int ram_val, retry;
    uint32 reg_acc, reg_val, reg_addr, reg_len, fld_val;
    int index_max, mem_len;
    uint32  *data_p, *mask_p, data_reg_addr, mask_reg_addr;

    assert(entry);
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
            mem_id = CFP_ACT_POLm;
            ram_val = CFP_5395_RAM_SEL_ACT;
            break;
        case DRV_CFP_RAM_METER:
            mem_id = CFP_METERm;
            ram_val = CFP_5395_RAM_SEL_METER;
            break;
        case DRV_CFP_RAM_STAT_IB:
            mem_id = CFP_STAT_IBm;
            ram_val = CFP_5395_RAM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = CFP_STAT_OBm;
            ram_val = CFP_5395_RAM_SEL_OB_STAT;
            break;
        case DRV_CFP_RAM_TCAM:
            mem_id = CFP_TCAM_S0m;
            ram_val = CFP_5395_RAM_SEL_TCAM;
            break;
        default:
            rv = SOC_E_UNAVAIL;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform TCAM read operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_acc, reg_len)) < 0) {
            goto cfp_write_exit;
    }
    fld_val = CFP_5395_OP_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_acc, OP_SELf, &fld_val);

    fld_val = index;       
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_acc, XCESS_ADDRf, &fld_val);

    switch (ram_type) {
    case DRV_CFP_RAM_TCAM:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->tcam_data;
        mask_p = entry->tcam_mask;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_DATAr, 0, 0);
        mask_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_MASKr, 0, 0);
        for (i = 0; i < mem_len; i++) {
            reg_val = *(data_p + i);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_write_exit;
            }
            reg_val = *(mask_p + i);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (mask_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_write_exit;
            }
        }
        break;
    case DRV_CFP_RAM_ACT:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->act_data;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ACT_POL_DATA0r, 0, 0);
        for (i = 0; i < mem_len; i++) {
            reg_val = *(data_p + i);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (data_reg_addr + i * 4), &reg_val, 4)) < 0) {
                goto cfp_write_exit;
            }
        }
        break;
    case DRV_CFP_RAM_METER:
        data_p = entry->meter_data;
        reg_val = *data_p;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_METER0r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_write_exit;
        }
        reg_val = *(data_p + 1);
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_METER1r, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_write_exit;
        }
        break;
    default:
        rv = SOC_E_PARAM;
        goto cfp_write_exit;
    }

    fld_val = ram_val;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_acc, RAM_SELf, &fld_val);
    fld_val = 1;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_acc, OP_STR_DONEf, &fld_val);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_acc, reg_len)) < 0) {
            goto cfp_write_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_acc, reg_len)) < 0) {
            goto cfp_write_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_acc, OP_STR_DONEf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_write_exit;
    }

    cfp_write_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}


/*
 * Function: drv5395_cfp_init
 *
 * Purpose:
 *     Initialize the CFP module. 
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 */
int 
drv5395_cfp_init(int unit)
{
    int port;
    pbmp_t pbm;
    int     reg_len, rv = SOC_E_NONE;
    uint32     reg_addr, reg_value, temp;


    /* 
     * Gnat 16725 workaround.
     * Diabling DSCP remarking to fix this issue.
     */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRREG_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRREG_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }
    temp = 0;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, TRREG_CTRLr, &reg_value, DSCP_RMK_ENf, &temp);
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    
    /* Enable CFP */
    pbm = PBMP_E_ALL(unit);

    PBMP_ITER(pbm, port) {
         (DRV_SERVICES(unit)->cfp_control_set)
            (unit, DRV_CFP_ENABLE, port, 1);
    }
    /* Clear HW TABLE */
    (DRV_SERVICES(unit)->cfp_control_set)
        (unit, DRV_CFP_TCAM_RESET, 0, 0);

    return SOC_E_NONE;
}

/*
 * Function: drv5395_cfp_action_get
 *
 * Purpose:
 *     Get the CFP action type and parameters value from 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action(IN/OUT) - driver action type
 *     entry -cfp entry
 *     act_param(OUT) - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 *     The action types were divided into 2 groups.
 *     One is to changed priority and the other is switch behavior 
 *     (drop/flood/append/redirect). So the user configure the input paramter,
 *     'action', to decide which action type you need to get. 
 *     action                              act_param
 *     DRV_CFP_ACT_IB_MOD_INT_PRI     Priority
 *     DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL     XXX
 *     DRV_CFP_ACT_IB_NONE           XXX
 *     DRV_CFP_ACT_IB_REDIRECT     port number
 *     DRV_CFP_ACT_IB_APPEND     port number
 *     DRV_CFP_ACT_IB_FLOOD     XXX
 *     DRV_CFP_ACT_IB_DROP     XXX
 */
int
drv5395_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param)
{
    int rv = SOC_E_NONE, i;
    uint32  fld_val;
    
    switch (*action) {
        case DRV_CFP_ACT_IB_MOD_INT_PRI:
        case DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_IB, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_MOD_PRI_MAP_IB, entry, &fld_val);
                *action = DRV_CFP_ACT_IB_MOD_INT_PRI;
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL;
            }
            break;
        case DRV_CFP_ACT_IB_NONE:
        case DRV_CFP_ACT_IB_REDIRECT:
        case DRV_CFP_ACT_IB_APPEND:
        case DRV_CFP_ACT_IB_FLOOD:
        case DRV_CFP_ACT_IB_DROP:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_IB, entry, &fld_val);
            if (!fld_val) {
                *action = DRV_CFP_ACT_IB_NONE;
            } else {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ADD_CHANGE_DEST_IB, entry, &fld_val);
                if (fld_val) {
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_DEST_IB, entry, &fld_val);
                    *action = DRV_CFP_ACT_IB_APPEND;
                    for ( i = 0; i < 6; i++ ) {
                        if (fld_val & (0x1 << i)) {
                            *act_param = i;
                            break;
                        }
                    }
                    if (*act_param == 5) {
                        /* BIT 5 is IMP port */
                        *act_param = CMIC_PORT(unit);
                    }
                } else {
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_DEST_IB, entry, &fld_val);
                    if (fld_val == CFP_5395_DEST_FLOOD) {
                        *action = DRV_CFP_ACT_IB_FLOOD;
                    } else if (fld_val == CFP_5395_DEST_DROP) {
                        *action = DRV_CFP_ACT_IB_DROP;
                    } else {
                        *action = DRV_CFP_ACT_IB_REDIRECT;
                        for ( i = 0; i < 6; i++ ) {
                            if (fld_val & (0x1 << i)) {
                                *act_param = i;
                                break;
                            }
                        }
                        if (*act_param == 5) {
                            /* BIT 5 is IMP port */
                            *act_param = CMIC_PORT(unit);
                        }
                    }
                }
            }
            break;
        case DRV_CFP_ACT_OB_MOD_INT_PRI:
        case DRV_CFP_ACT_OB_MOD_INT_PRI_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_OB, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_MOD_PRI_MAP_OB, entry, &fld_val);
                *action = DRV_CFP_ACT_OB_MOD_INT_PRI;
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_OB_MOD_INT_PRI_CANCEL;
            }
            break;
        case DRV_CFP_ACT_OB_NONE:
        case DRV_CFP_ACT_OB_REDIRECT:
        case DRV_CFP_ACT_OB_APPEND:
        case DRV_CFP_ACT_OB_FLOOD:
        case DRV_CFP_ACT_OB_DROP:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_OB, entry, &fld_val);
            if (!fld_val) {
                *action = DRV_CFP_ACT_OB_NONE;
            } else {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_ADD_CHANGE_DEST_OB, entry, &fld_val);
                if (fld_val) {
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_DEST_OB, entry, &fld_val);
                    *action = DRV_CFP_ACT_OB_APPEND;
                    for ( i = 0; i < 6; i++ ) {
                        if (fld_val & (0x1 << i)) {
                            *act_param = i;
                            break;
                        }
                    }
                    if (*act_param == 5) {
                        /* BIT 5 is IMP port */
                        *act_param = CMIC_PORT(unit);
                    }
                } else {
                    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_DEST_OB, entry, &fld_val);
                    if (fld_val == CFP_5395_DEST_FLOOD) {
                        *action = DRV_CFP_ACT_OB_FLOOD;
                    } else if (fld_val == CFP_5395_DEST_DROP) {
                        *action = DRV_CFP_ACT_OB_DROP;
                    } else {
                        *action = DRV_CFP_ACT_OB_REDIRECT;
                        for ( i = 0; i < 6; i++ ) {
                            if (fld_val & (0x1 << i)) {
                                *act_param = i;
                                break;
                            }
                        }
                        if (*act_param == 5) {
                            /* BIT 5 is IMP port */
                            *act_param = CMIC_PORT(unit);
                        }
                    }
                }
            }
            break;
        case DRV_CFP_ACT_CHANGE_TC:
        case DRV_CFP_ACT_CHANGE_TC_CANCEL:
            /* Get the value from Outband TC field */
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_OB, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_MOD_PRI_MAP_OB, entry, &fld_val);
                *action = DRV_CFP_ACT_CHANGE_TC;
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_CHANGE_TC_CANCEL;
            }
            break;
        case DRV_CFP_ACT_IB_COSQ_NEW:
        case DRV_CFP_ACT_IB_COSQ_CANCEL:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_ACT_OB_COSQ_NEW:
        case DRV_CFP_ACT_OB_COSQ_CANCEL:
            rv = SOC_E_UNAVAIL;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    
    return rv;
}

/*
 * Function: drv5395_cfp_action_set
 *
 * Purpose:
 *     Set the CFP action type and parameters value to 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action - driver action type
 *     entry(OUT) -cfp entry
 *     act_param - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 *     action                              act_param
 *     DRV_CFP_ACT_IB_MOD_INT_PRI     Priority
 *     DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL     XXX
 *     DRV_CFP_ACT_IB_NONE           XXX
 *     DRV_CFP_ACT_IB_REDIRECT     port number
 *     DRV_CFP_ACT_IB_APPEND     port number
 *     DRV_CFP_ACT_IB_FLOOD     XXX
 *     DRV_CFP_ACT_IB_DROP     XXX
 */
int
drv5395_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    assert(entry);

    switch (action) {
        case DRV_CFP_ACT_IB_NONE:
            fld_val = 0;
            /* action */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_MOD_INT_PRI:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_IB, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_MAP_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_REDIRECT:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_IB, entry, &fld_val);
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_IB, entry, &fld_val);
            /* Bit 5 indicate port 8. (IMP port) */
            if (act_param1 & 0x100) {
                fld_val = (act_param1 | 0x20) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_APPEND:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_IB, entry, &fld_val);
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_IB, entry, &fld_val);
            /* Bit 5 indicate port 8. (IMP port) */
            if (act_param1 & 0x100) {
                fld_val = (act_param1 | 0x20) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_FLOOD:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_IB, entry, &fld_val);
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_IB, entry, &fld_val);
            fld_val = CFP_5395_DEST_FLOOD;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_DROP:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_IB, entry, &fld_val);
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_IB, entry, &fld_val);
            fld_val = CFP_5395_DEST_DROP;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_NONE:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_MOD_INT_PRI:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_OB, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_MAP_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_MOD_INT_PRI_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_REDIRECT:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_OB, entry, &fld_val);
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_OB, entry, &fld_val);
            /* Bit 5 indicate port 8. (IMP port) */
            if (act_param1 & 0x100) {
                fld_val = (act_param1 | 0x20) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_APPEND:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_OB, entry, &fld_val);
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_OB, entry, &fld_val);
            /* Bit 5 indicate port 8. (IMP port) */
            if (act_param1 & 0x100) {
                fld_val = (act_param1 | 0x20) & 0xff;
            } else {
                fld_val = act_param1;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_FLOOD:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_OB, entry, &fld_val);
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_OB, entry, &fld_val);
            fld_val = CFP_5395_DEST_FLOOD;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_DROP:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_REDIRECT_EN_OB, entry, &fld_val);
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_ADD_CHANGE_DEST_OB, entry, &fld_val);
            fld_val = CFP_5395_DEST_DROP;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DEST_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_DSCP_NEW:
        case DRV_CFP_ACT_IB_DSCP_CANCEL:
        case DRV_CFP_ACT_OB_DSCP_NEW:
        case DRV_CFP_ACT_OB_DSCP_CANCEL:
            /* Gnat 16725 
             * Diabling DSCP remarking to fix this issue.
             */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_ACT_CHANGE_TC:
            /* Inband action */
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_IB, entry, &fld_val);
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_OB, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_MAP_IB, entry, &fld_val);
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_MAP_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_CHANGE_TC_CANCEL:
             fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_OB, entry, &fld_val);
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_MOD_PRI_EN_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_COSQ_NEW:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_ACT_OB_COSQ_NEW:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_ACT_IB_COSQ_CANCEL:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_ACT_OB_COSQ_CANCEL:
            rv = SOC_E_UNAVAIL;
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function: drv5395_cfp_control_get
 *
 * Purpose:
 *     Get the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2(OUT) -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     control_type     param1     param2
 *     DRV_CFP_ENABLE, port number, enable
 *     DRV_CFP_FLOOD_TRUNK, XXX, enable
 *     DRV_CFP_FLOOD_VLAN, XXX, enable
 *     DRV_CFP_ISPVLAN_DELIMITER, XXX, ISP delimter
 *     DRV_CFP_LLC_ENCAP, XXX, DSAP+SSAP+Control field
 *     DRV_CFP_SLICE_SELECT, port number, slice id map
 *     DRV_CFP_TCAM_RESET, XXX, XXX
 *     
 */
int
drv5395_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2)
{
    int rv = SOC_E_NONE;
    uint32  reg_addr, reg_len, fld_32, reg_val;
    uint64  reg_64, fld_64;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_CTL_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_CTL_REGr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
                return rv;
            }
            if (SOC_INFO(unit).port_num > 32) {
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, CFP_CTL_REGr, (uint32 *)&reg_64, 
                    CFP_EN_MAPf, (uint32 *)&fld_64);
                if (COMPILER_64_BITTEST(fld_64, param1) != 0) {
                    *param2 = TRUE;
                } else {
                    *param2 = FALSE;
                }
            } else {
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, CFP_CTL_REGr, (uint32 *)&reg_64, 
                    CFP_EN_MAPf, (uint32 *)&fld_32);
                if ((fld_32 & (0x1 << param1))  != 0) {
                    *param2 = TRUE;
                } else {
                    *param2 = FALSE;
                }
            }
            break;
        case DRV_CFP_FLOOD_TRUNK:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_FLOOD_VLAN:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_ISPVLAN_DELIMITER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ISP_VIDr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ISP_VIDr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ISP_VIDr, (uint32 *)&reg_val, 
                ISP_VLAN_DELIMITERf, &fld_32);
            *param2 = fld_32;
            break;
        case DRV_CFP_LLC_ENCAP:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, IEEE802P2_LLCr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, IEEE802P2_LLCr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            *param2 = 0; 
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, IEEE802P2_LLCr, (uint32 *)&reg_val, CTRLf, &fld_32);
            *param2 += fld_32;
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, IEEE802P2_LLCr, (uint32 *)&reg_val, CTRLf, &fld_32);
            *param2 += (fld_32 << 8);
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, IEEE802P2_LLCr, (uint32 *)&reg_val, CTRLf, &fld_32);
            *param2 += (fld_32 << 16);
            break;
        case DRV_CFP_SLICE_SELECT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, SLICE_MAP_Pr, param1, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, SLICE_MAP_Pr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, SLICE_MAP_Pr, (uint32 *)&reg_val, 
                SLICE_SEL_MAPf, &fld_32);
            *param2 = fld_32;
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function: drv5395_cfp_control_set
 *
 * Purpose:
 *     Set the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2 -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     control_type     param1     param2
 *     DRV_CFP_ENABLE, port number, enable
 *     DRV_CFP_FLOOD_TRUNK, XXX, enable
 *     DRV_CFP_FLOOD_VLAN, XXX, enable
 *     DRV_CFP_ISPVLAN_DELIMITER, XXX, ISP demimeter
 *     DRV_CFP_LLC_ENCAP, XXX, DSAP+SSAP+Control field
 *     DRV_CFP_SLICE_SELECT, port number, slice id map
 *     DRV_CFP_TCAM_RESET, XXX, XXX
 *     
 */
int
drv5395_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2)
{
    int rv = SOC_E_NONE;
    uint32  reg_addr, reg_len, fld_32, reg_val, temp_32;
    uint64  reg_64, fld_64, temp_64;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_CTL_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_CTL_REGr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
                return rv;
            }
            if (SOC_INFO(unit).port_num > 32) {
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, CFP_CTL_REGr, (uint32 *)&reg_64, 
                    CFP_EN_MAPf, (uint32 *)&fld_64);
                COMPILER_64_ZERO(temp_64);
                fld_32 = 1;
                COMPILER_64_SET(temp_64, 0, fld_32);
                COMPILER_64_SHL(temp_64, param1);
                if (param2) {
                    COMPILER_64_OR(fld_64, temp_64);
                } else {
                    COMPILER_64_XOR(fld_64, temp_64);
                }
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, CFP_CTL_REGr, (uint32 *)&reg_64, 
                    CFP_EN_MAPf, (uint32 *)&fld_64);
            } else {
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, CFP_CTL_REGr, (uint32 *)&reg_64, 
                    CFP_EN_MAPf, (uint32 *)&fld_32);;
                temp_32 = 0x1 << param1;
                if (param2) {
                    fld_32 |= temp_32;
                } else {
                    fld_32 &= ~temp_32;
                }
                (DRV_SERVICES(unit)->reg_field_set)
                    (unit, CFP_CTL_REGr, (uint32 *)&reg_64, 
                    CFP_EN_MAPf, (uint32 *)&fld_32);
            }
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_FLOOD_TRUNK:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_FLOOD_VLAN:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_CFP_ISPVLAN_DELIMITER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ISP_VIDr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ISP_VIDr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ISP_VIDr, (uint32 *)&reg_val, 
                ISP_VLAN_DELIMITERf, &fld_32);
            fld_32 = param2; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ISP_VIDr, (uint32 *)&reg_val, 
                ISP_VLAN_DELIMITERf, &fld_32);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_LLC_ENCAP:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, IEEE802P2_LLCr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, IEEE802P2_LLCr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            fld_32 = param2 & 0xff; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, IEEE802P2_LLCr, (uint32 *)&reg_val, CTRLf, &fld_32);
            fld_32 = (param2 >> 8) & 0xff; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, IEEE802P2_LLCr, (uint32 *)&reg_val, SSAPf, &fld_32);
            fld_32 = (param2 >> 16) & 0xff; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, IEEE802P2_LLCr, (uint32 *)&reg_val, DSAPf, &fld_32);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_SLICE_SELECT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, SLICE_MAP_Pr, param1, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, SLICE_MAP_Pr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            fld_32 = param2; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, SLICE_MAP_Pr, (uint32 *)&reg_val, 
                SLICE_SEL_MAPf, &fld_32);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_val, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_TCAM_RESET:
            MEM_LOCK(unit, CFP_TCAM_S0m);
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_val, reg_len)) < 0) {
                MEM_UNLOCK(unit, CFP_TCAM_S0m);
                return rv;
            }
            fld_32 = 1;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_ACCr, &reg_val, TCAM_RSTf, &fld_32);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_val, reg_len)) < 0) {
                MEM_UNLOCK(unit, CFP_TCAM_S0m);
                return rv;
            }
            MEM_UNLOCK(unit, CFP_TCAM_S0m);
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}


/*
 * Function: drv5395_cfp_entry_read
 *
 * Purpose:
 *     Read the TCAM/ACTION/POLICY/METER raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 */
int
drv5395_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n", 
                    index, rv);
                return rv;
            }
            if ( (rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            if ((rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
 "drv_cfp_entry_read : failed to read meter ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
            if ((rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
      "drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_ACT:
            if ((rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_METER:
            if ((rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv5395_cfp_entry_search
 *
 * Purpose:
 *     Search the valid CFP entry from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - search flags
 *     index(OUT) - CFP entry index
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *
 */
int
drv5395_cfp_entry_search(int unit, uint32 flags, uint32 *index, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    if ((rv = _drv5395_cfp_tcam_search(unit, flags, 
            index, entry)) != SOC_E_NONE) {
        soc_cm_debug(DK_ERR,
            "drv_cfp_entry_search : failed to serach TCAM rv = %d\n",
            rv);
        return rv;
    }
    /* If found valid TCAM entry, get the correspoding ACT and Meter data */
    if ((flags & DRV_CFP_SEARCH_GET) && (rv == SOC_E_NONE)) {
        if ((rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_ACT, *index, entry)) 
            != SOC_E_NONE) {
            soc_cm_debug(DK_ERR,
"drv_cfp_entry_search:failed to read action ram with index = 0x%x, rv = %d. \n",
                *index, rv);
            return rv;
        }
        if ((rv = _drv5395_cfp_read(unit, DRV_CFP_RAM_METER, *index, entry)) 
            != SOC_E_NONE) {
            soc_cm_debug(DK_ERR,
"drv_cfp_entry_search :failed to read meter ram with index = 0x%x, rv = %d. \n",
                *index, rv);
            return rv;
        }
        entry->id = *index;
    }
    return rv;
}


/*
 * Function: drv5395_cfp_entry_write
 *
 * Purpose:
 *     Write the TCAM/ACTION/POLICY/METER raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 * Note:
 *     
 */
int
drv5395_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv5395_cfp_write(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
    "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            if ((rv = _drv5395_cfp_write(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_write:failed to write action ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            if ((rv = _drv5395_cfp_write(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_write :failed to write meter ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
             if ((rv = _drv5395_cfp_write(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
    "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_ACT:
             if ((rv = _drv5395_cfp_write(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
 "drv_cfp_entry_write : failed to write ACT ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_METER:
             if ((rv = _drv5395_cfp_write(unit, 
                DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_write :failed to write METER ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index; 
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv5395_cfp_field_get
 *
 * Purpose:
 *     Get the field value from the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry -cfp entry
 *     fld_val(OUT) - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv5395_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int mem_id;
    uint32  fld_id;
    uint32  mask, mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;

    assert(entry);
    assert(fld_val);
    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
        if ((field_type == DRV_CFP_FIELD_SLICE_ID) || 
            (field_type == DRV_CFP_FIELD_SRC_PORT)) {
            /* For slice ID field, don;t care the TCAM type */
            mem_id = CFP_TCAM_S0m; 
        } else {
            if (( rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID,
                    entry, &fld_id)) < 0 ) {
                soc_cm_debug(DK_ERR,
                    "drv_cfp_field_get : UNKNOW SLICE ID. \n");
                return rv;
            }
            switch (fld_id) {
                case 0:
                    mem_id = CFP_TCAM_S0m;
                    break;
                case 1:
                    mem_id = CFP_TCAM_S1m;
                    break;
                case 2:
                    mem_id = CFP_TCAM_S2m;
                    break;
                case 3:
                    mem_id = CFP_TCAM_S3m;
                    break;
                case 4:
                    mem_id = CFP_TCAM_S4m;
                    break;
                case 5:
                    mem_id = CFP_TCAM_S5m;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
        }
        if (mem_type == DRV_CFP_RAM_TCAM) {
            data_p = entry->tcam_data;
        } else { /* mask data */
            data_p = entry->tcam_mask;
        }
        break;
    case DRV_CFP_RAM_ACT:
        mem_id = CFP_ACT_POLm;
        data_p = entry->act_data;
        break;
    case DRV_CFP_RAM_METER:
        mem_id = CFP_METERm;
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    }
    if (( rv = _drv5395_cfp_field_mapping(unit, field_type, &fld_id)) < 0) {
        soc_cm_debug(DK_ERR,
            "drv_cfp_field_get : UNKNOW FIELD ID. \n");
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);
    bp = fieldinfo->bp;

    if (fieldinfo->len < 32) {
        mask = (1 << fieldinfo->len) - 1;
    } else {
        mask = -1;
    }
    
    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_val[0] = ((data_p[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {
        if (fieldinfo->flags & SOCF_LE) {
            for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
                if (len > 32) {
                    mask = 0xffffffff;
                } else {
                    mask = (1 << len) - 1;
                }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
                mask_lo = mask;
                mask_hi = 0;
                /* if field is not aligned with 32-bit word boundary */
                /* adjust hi and lo masks accordingly. */
                if (bp) {
                    mask_lo = mask << bp;
                    mask_hi = mask >> (32 - bp);
                }
                /* get field value --- 32 bits each time */
                fld_val[i] = (data_p[FIX_MEM_ORDER_E(wp++, meminfo)] 
                    & mask_lo) >> bp;
                if (mask_hi) {
                    fld_val[i] |= (data_p[FIX_MEM_ORDER_E(wp, meminfo)] 
                        & mask_hi) << (32 - bp);
                }
                i++;
            }
        } else {
            i = (len - 1) / 32;

            while (len > 0) {
                assert(i >= 0);
                fld_val[i] = 0;
                do {
                    fld_val[i] = (fld_val[i] << 1) |
                    ((data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                    (bp & (32 - 1))) & 1);
                    len--;
                    bp++;
                } while (len & (32 - 1));
                i--;
            }
        }
    }
    return rv;
}

/*
 * Function: drv5395_cfp_field_set
 *
 * Purpose:
 *     Set the field value to the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry(OUT) -cfp entry
 *     fld_val - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv5395_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int mem_id;
    uint32  fld_id;
    uint32  mask, mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;

    assert(entry);
    assert(fld_val);
    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
        if (field_type == DRV_CFP_FIELD_SLICE_ID) {
            /* For slice ID field, don;t care the TCAM type */
            mem_id = CFP_TCAM_S0m; 
        } else {
            if (( rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID,
                    entry, &fld_id)) < 0 ) {
                soc_cm_debug(DK_ERR,
                    "drv_cfp_field_set : UNKNOW SLICE ID. \n");
                return rv;
            }
            switch (fld_id) {
                case 0:
                    mem_id = CFP_TCAM_S0m;
                    break;
                case 1:
                    mem_id = CFP_TCAM_S1m;
                    break;
                case 2:
                    mem_id = CFP_TCAM_S2m;
                    break;
                case 3:
                    mem_id = CFP_TCAM_S3m;
                    break;
                case 4:
                    mem_id = CFP_TCAM_S4m;
                    break;
                case 5:
                    mem_id = CFP_TCAM_S5m;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
        }
        if (mem_type == DRV_CFP_RAM_TCAM) {
            data_p = entry->tcam_data;
        } else { /* mask data */
            data_p = entry->tcam_mask;
        }
        break;
    case DRV_CFP_RAM_ACT:
        mem_id = CFP_ACT_POLm;
        data_p = entry->act_data;
        break;
    case DRV_CFP_RAM_METER:
        mem_id = CFP_METERm;
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    }
    
    if (( rv = _drv5395_cfp_field_mapping(unit, field_type, &fld_id)) < 0) {
        soc_cm_debug(DK_ERR,
            "drv_cfp_field_set : UNKNOW FIELD ID. \n");
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);
    /* 
     * Get the value to set into each entry's valid field. 
     * The valid value is depend on chips.
     */
    if ((fld_id == VALID_Rf) &&  *fld_val != 0){
        len = fieldinfo->len;
        *fld_val = 0;
        for (i = 0; i < len; i++) {
            *fld_val |= (0x1 << i);
        }
    }    
    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }

            /* set field value --- 32 bits each time */
            data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            data_p[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_val[i] << bp) & mask_lo);
            if (mask_hi) {
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_val[i] >> (32 - bp)) & mask_hi);
            }

            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_val[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
    return rv;
}

/*
 * Function: drv5395_cfp_meter_get
 *
 * Purpose:
 *     Get the meter value from CFP entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry -cfp entry
 *     kbits_sec(OUT) -meter rate (kbits per second)
 *     kbits_burst(OUT) -meter bucket size (kbits)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv5395_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, 
            uint32 *kbits_burst)    
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    assert(entry);
    assert(kbits_sec);
    assert(kbits_burst);
    
    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_RATE_REFRESH_EN, entry, &fld_val);
    if (fld_val) {
        (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE, entry, &fld_val);
        _drv5395_cfp_meter_chip2rate(unit, kbits_sec, fld_val);
    } else {
        *kbits_sec = 0;
    }

    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_REF_CAP, entry, &fld_val);
    _drv5395_cfp_meter_chip2burst(unit, kbits_burst, fld_val);

    return rv;
}

/*
 * Function: drv5395_cfp_meter_set
 *
 * Purpose:
 *     Set the meter value to CFP entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry(OUT) -cfp entry
 *     kbits_sec -meter rate (kbits per second)
 *     kbits_burst -meter bucket size (kbits)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv5395_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, 
            uint32 kbits_burst)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    assert(entry);
    if (kbits_sec) {
        if ((kbits_sec > CFP_5395_METER_RATE_MAX) ||
            (kbits_sec < CFP_5395_METER_RATE_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv_cfp_meter_set : rate unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        _drv5395_cfp_meter_rate2chip(unit, kbits_sec, &fld_val);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_RATE, entry, &fld_val);
        fld_val = 1;
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE_REFRESH_EN, entry, &fld_val);
    } else {
        fld_val = 0;
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE_REFRESH_EN, entry, &fld_val);
    }

    if (kbits_burst) {
        if ((kbits_burst > CFP_5395_METER_BURST_MAX) ||
            (kbits_burst < CFP_5395_METER_BURST_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv_cfp_meter_set : burst size unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        _drv5395_cfp_meter_burst2chip(unit, kbits_burst, &fld_val);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_CAP, entry, &fld_val);
    } else {
        _drv5395_cfp_meter_burst2chip(unit, CFP_5395_METER_BURST_MIN, &fld_val);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_CAP, entry, &fld_val);
    }
    return rv;
}

int
drv5395_cfp_meter_rate_transform(int unit, uint32 kbits_sec, uint32 kbits_burst, 
                uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit)
{
    int rv = SOC_E_NONE;

    if (kbits_sec) {
        /*    coverity[unsigned_compare]    */
        if ((kbits_sec > CFP_5395_METER_RATE_MAX) ||
            (kbits_sec < CFP_5395_METER_RATE_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv5395_cfp_meter_rate_transform : rate unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        _drv5395_cfp_meter_rate2chip(unit, kbits_sec, ref_cnt);
    } else {
        *ref_cnt = 0;
    }

    if (kbits_burst) {
        /*    coverity[unsigned_compare]    */
        if ((kbits_burst > CFP_5395_METER_BURST_MAX) ||
            (kbits_burst < CFP_5395_METER_BURST_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv5395_cfp_meter_rate_transform : burst size unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        
        _drv5395_cfp_meter_burst2chip(unit, kbits_burst, bucket_size);
    } else {
        *bucket_size = 0;
    }

    return rv;
}


/*
 * Function: drv5395_cfp_qset_get
 *
 * Purpose:
 *     Get the qualify bit value from CFP entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -CFP entry
 *     val(OUT) -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv5395_cfp_qset_get(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 *val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (entry->w[wp] & (1 << bp)) {
        *val = TRUE;
    } else {
        *val = FALSE;
    }

    return rv;
}

/*
 * Function: drv5395_cfp_qset_set
 *
 * Purpose:
 *     Set/Reset the qualify bit value to CFP entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -CFP entry
 *     val -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv5395_cfp_qset_set(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp, temp = 0;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (val) {
        temp = 1;
    }

    if (temp) {
        entry->w[wp] |= (1 << bp);
    } else {
        entry->w[wp] &= ~(1 << bp);
    }
    
    return rv;
}

/*
 * Function: drv5395_cfp_slice_id_select
 *
 * Purpose:
 *     According to this entry's fields to select which slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - driver cfp entry
 *     slice_id(OUT) - slice id for this entry
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_RESOURCE - Can't found suitable slice id for this entry.
 */
int
drv5395_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    uint32 i;
    int match;
    uint32  slice[(DRV_CFP_QUAL_COUNT / 32) + 1];

    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    match = TRUE;
    i = 0;
    while (s0_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s0_qset[i]/32)] |= (0x1 << (s0_qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 0; /* SLICE 0 */
        return SOC_E_NONE;
    }

    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    i = 0;
    while (s1_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s1_qset[i]/32)] |= (0x1 << (s1_qset[i] % 32));
        i++;
    }
    match = TRUE;
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 1; /* SLICE 1 */
        return SOC_E_NONE;
    }
 
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    match = TRUE;
    i = 0;
    while (s2_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s2_qset[i]/32)] |= (0x1 << (s2_qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 2; /* SLICE 2 */
        return SOC_E_NONE;
    }

    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    match = TRUE;
    i = 0;
    while (s3_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s3_qset[i]/32)] |= (0x1 << (s3_qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 3; /* SLICE 3 */
        return SOC_E_NONE;
    }

    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    match = TRUE;
    i = 0;
    while (s4_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s4_qset[i]/32)] |= (0x1 << (s4_qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 4; /* SLICE 4 */
        return SOC_E_NONE;
    }

    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    match = TRUE;
    i = 0;
    while (s5_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s5_qset[i]/32)] |= (0x1 << (s5_qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 5; /* SLICE 4 */
        return SOC_E_NONE;
    }
    return SOC_E_RESOURCE;
    
}


/*
 * Function: drv5395_cfp_slice_to_qset
 *
 * Purpose:
 *     According to slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry(OUT) - driver cfp entry
 *     slice_id - slice id 
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - un-support slice id.
 */
int
drv5395_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry)
{
    uint32 i;
    uint32  slice[(DRV_CFP_QUAL_COUNT / 32) + 1];
    int     *qset;

    switch(slice_id) {
        case 0:
            qset = s0_qset;
            break;
        case 1:
            qset = s1_qset;
            break;
        case 2:
            qset = s2_qset;
            break;
        case 3:
            qset = s3_qset;
            break;
        case 4:
            qset = s4_qset;
            break;
        case 5:
            qset = s4_qset;
            break;
        default:
            return SOC_E_PARAM;
    }

    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    i = 0;
    while (qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(qset[i]/32)] |= (0x1 << (qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        entry->w[i] = slice[i];
    }
    return SOC_E_NONE;
    
}


/*
 * Function: drv5395_cfp_stat_get
 *
 * Purpose:
 *     Get the counter value from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     stat_type - In-band/Out-band counter/Both
 *     index -entry index
 *     counter(OUT) -counter value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
drv5395_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter)
{
    int rv = SOC_E_NONE;
    uint32  index_max, temp;
    
    assert(counter);

   index_max = soc_robo_mem_index_max(unit, CFP_STAT_IBm);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            _drv5395_cfp_stat_read(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            break;
        case DRV_CFP_STAT_OUTBAND:
            _drv5395_cfp_stat_read(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        case DRV_CFP_STAT_ALL:
            _drv5395_cfp_stat_read(unit, DRV_CFP_RAM_STAT_IB, index, 
                &temp);
            *counter = temp;
            _drv5395_cfp_stat_read(unit, DRV_CFP_RAM_STAT_OB, index, 
                &temp);
            *counter += temp;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv5395_cfp_stat_set
 *
 * Purpose:
 *     Set the CFP counter value to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     stat_type - In-band/Out-band counter/Both
 *     index -entry index
 *     counter -counter value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
drv5395_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter)
{
    int rv = SOC_E_NONE;
    uint32  index_max;

   index_max = soc_robo_mem_index_max(unit, CFP_STAT_IBm);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            _drv5395_cfp_stat_write(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            break;
        case DRV_CFP_STAT_OUTBAND:
            _drv5395_cfp_stat_write(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        case DRV_CFP_STAT_ALL:
            _drv5395_cfp_stat_write(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            _drv5395_cfp_stat_write(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv5348_cfp_udf_get
 *
 * Purpose:
 *     Get the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - port numbrt
 *     udf_index -the index of user defined fields
 *     offset(OUT) -offset value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     For BCM5348, there are 10 UDFs.
 *     
 */
int 
drv5395_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base)
{
    int rv = SOC_E_NONE;
    uint32  reg_index, fld_index, reg_addr, reg_len, temp;
    uint32  reg_32;
    uint64  reg_64;

    if (udf_index >=  CFP_5395_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }

    /* For BCM5395 
     * 0 : UDF0
       1 : UDF4A
       2 : UDF4B
       3 : UDF4C
       4 : UDF5A
       5 : UDF5B
       6 : UDF5C
       7 : UDF5D
       8 : UDF5E
     */  
    
    switch (udf_index) {
        case 0 :
            reg_index = UDF_OFFSET0_Pr;
            fld_index = UDF_OFFSETf;
            break;
        case 1 :
            reg_index = UDF_OFFSET4_Pr;
            fld_index = UDF4A_OFFSETf;
            break;
        case 2 :
            reg_index = UDF_OFFSET4_Pr;
            fld_index = UDF4B_OFFSETf;
            break;
        case 3 :
            reg_index = UDF_OFFSET4_Pr;
            fld_index = UDF4C_OFFSETf;
            break;
        case 4 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5A_OFFSETf;
            break;
        case 5 : 
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5B_OFFSETf;
            break;
        case 6 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5C_OFFSETf;
            break;
        case 7 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5D_OFFSETf;
            break;
        case 8 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5E_OFFSETf;
            break;
        default:
            return SOC_E_INTERNAL;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, reg_index, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    if ( reg_len > 4) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
            return rv;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_64, fld_index, &temp);
    } else {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_32, reg_len)) < 0) {
            return rv;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, &reg_32, fld_index, &temp);
    }
    *offset = temp;
    return rv;
}

/*
 * Function: drv5395_cfp_udf_set
 *
 * Purpose:
 *     Set the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - port numbrt
 *     udf_index -the index of user defined fields
 *     offset -offset value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     For BCM5348, there are 10 UDFs.
 *     
 */
int 
drv5395_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base)
{
    int rv = SOC_E_NONE;
    uint32  reg_index, fld_index, reg_addr, reg_len, temp;
    uint32  reg_32;
    uint64  reg_64;

    if (udf_index >=  CFP_5395_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }
    if (offset > CFP_5395_UDF_OFFSET_MAX) {
        return SOC_E_CONFIG;
    }

    /* For BCM5395 
     * 0 : UDF0
       1 : UDF4A
       2 : UDF4B
       3 : UDF4C
       4 : UDF5A
       5 : UDF5B
       6 : UDF5C
       7 : UDF5D
       8 : UDF5E
     */  
    
    switch (udf_index) {
        case 0 :
            reg_index = UDF_OFFSET0_Pr;
            fld_index = UDF_OFFSETf;
            break;
        case 1 :
            reg_index = UDF_OFFSET4_Pr;
            fld_index = UDF4A_OFFSETf;
            break;
        case 2 :
            reg_index = UDF_OFFSET4_Pr;
            fld_index = UDF4B_OFFSETf;
            break;
        case 3 :
            reg_index = UDF_OFFSET4_Pr;
            fld_index = UDF4C_OFFSETf;
            break;
        case 4 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5A_OFFSETf;
            break;
        case 5 : 
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5B_OFFSETf;
            break;
        case 6 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5C_OFFSETf;
            break;
        case 7 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5D_OFFSETf;
            break;
        case 8 :
            reg_index = UDF_OFFSET5_Pr;
            fld_index = UDF5E_OFFSETf;
            break;
        default:
            return SOC_E_INTERNAL;
    }
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, reg_index, port, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    temp = offset;
    if ( reg_len > 4) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
            return rv;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, (uint32 *)&reg_64, fld_index, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
            return rv;
        }
    } else {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_32, reg_len)) < 0) {
            return rv;
        }
        (DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, &reg_32, fld_index, &temp);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_32, reg_len)) < 0) {
            return rv;
        }
    }
    return rv;
}

/*
 * Function: drv5395_cfp_ranger
 *
 * Purpose:
 *     Check ranger type and parameters.
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - types of ranger
 *     min - range lower bound
 *     max - ranger upper bound
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv5395_cfp_ranger(int unit, uint32 flags, uint32 min, uint32 max)
{
    int rv = SOC_E_NONE;

    if (flags & DRV_FIELD_RANGE_DSTPORT) {
        rv = SOC_E_PARAM;
    } else if (flags & DRV_FIELD_RANGE_SRCPORT) {
        if (min != 0 || max != 1023) {
            rv = SOC_E_UNAVAIL;
        }
    } else {
        rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv5395_cfp_range_set
 *
 * Purpose:
 *     Check ranger type and parameters.
 *
 * Parameters:
 *     unit - BCM device number
 *     type - types of ranger
 *     id - ranger's id (if applicable)
 *     param1 - for L4 src/dst port and vlan, it's the minimum value
 *              for other rangers, it's the value to set to chip
 *     param2 - for L4 src/dst port and vlan, it's the maximum value
 *              for other rangers, it's reserved
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv5395_cfp_range_set(int unit, uint32 type, uint32 id, uint32 param1, uint32 param2)
{
    /* 
     * BCM5395 doesn't support range check by setting register.
     */
    return SOC_E_UNAVAIL;
}


/*
 * Function: drv5395_cfp_range_get
 *
 * Purpose:
 *     Get ranger parameters.
 *
 * Parameters:
 *     unit - BCM device number
 *     type - types of ranger
 *     id - ranger's id (if applicable)
 *     param1 - for L4 src/dst port and vlan, it's the minimum value
 *              for other rangers, it's the value to set to chip
 *     param2 - for L4 src/dst port and vlan, it's the maximum value
 *              for other rangers, it's reserved
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv5395_cfp_range_get(int unit, uint32 type, uint32 id, uint32 *param1, uint32 *param2)
{
    /* 
     * BCM5395 doesn't support range check by setting register.
     */
    return SOC_E_UNAVAIL;
}
