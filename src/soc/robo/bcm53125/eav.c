/*
 * $Id: eav.c 1.3 Broadcom SDK $
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
 * Field Processor related CLI commands
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>
#include <soc/drv.h>
#include <soc/debug.h>


#define EAV_BCM53125_MAX_TICK_INC    63
#define EAV_BCM53125_MAX_TICK_ADJUST_PERIOD    15
#define EAV_BCM53125_MAX_SLOT_ADJUST_PERIOD    15
#define EAV_BCM53125_MAX_TICK_ONE_SLOT    3126
#define EAV_BCM53125_MAX_SLOT_NUMBER    31

#define EAV_BCM53125_MAX_PCP_VALUE    0x7

/* Bytes count allowed for EAV Class4/Class5 bandwidth within a slot time */
#define EAV_53125_MAX_BANDWIDTH_VALUE 16383
#define EAV_53125_MIN_BANDWIDTH_VALUE 0

static int
_drv_bcm53125_eav_mmu_init(int unit)
{
    uint32 reg_val;

    /* 1. MMU settings provided by ASIC*/
    /* Hysteresis threshold */
    reg_val = 110; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 0, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 1, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 2, &reg_val));
    reg_val = 112;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Qr(unit, 3, &reg_val));
    reg_val = 112;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q45r(unit, 0, &reg_val));
    reg_val = 112;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q45r(unit, 1, &reg_val));

    /* Pause threshold */
    reg_val = 232; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 0, &reg_val));
    reg_val = 233;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 1, &reg_val));
    reg_val = 234;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 2, &reg_val));
    reg_val = 235;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q45r(unit, 0, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q45r(unit, 1, &reg_val));

    /* Drop threshold */
    reg_val = 500;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q45r(unit, 1, &reg_val));

    /* Total reserved threshold */
    reg_val = 1; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 3, &reg_val));
    reg_val = 18;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 4, &reg_val));
    reg_val = 24;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_RSRV_Qr(unit, 5, &reg_val));

    /* IMP Hysteresis threshold */
    reg_val = 122; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 0, &reg_val));
    reg_val = 123;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 1, &reg_val));
    reg_val = 123;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 2, &reg_val));
    reg_val = 124;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_RSRV_Q_IMPr(unit, 3, &reg_val));

    /* IMP Pause threshold */
    reg_val = 244; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 0, &reg_val));
    reg_val = 245;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 1, &reg_val));
    reg_val = 246;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 2, &reg_val));
    reg_val = 247;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_PAUSE_Q_IMPr(unit, 3, &reg_val));

    /* IMP Drop threshold */
    reg_val = 511; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 1, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 2, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TXQ_TH_DROP_Q_IMPr(unit, 3, &reg_val));

    /* Total Hysteresis threshold */
    reg_val = 108; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 0, &reg_val));
    reg_val = 109;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 1, &reg_val));
    reg_val = 110;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 2, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Qr(unit, 3, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q45r(unit, 0, &reg_val));
    reg_val = 111;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q45r(unit, 1, &reg_val));

    /* Total Pause threshold */
    reg_val = 246; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 0, &reg_val));
    reg_val = 248;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 1, &reg_val));
    reg_val = 250;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 2, &reg_val));
    reg_val = 252;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q45r(unit, 1, &reg_val));

    /* Total Drop threshold */
    reg_val = 378; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 0, &reg_val));
    reg_val = 380;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 1, &reg_val));
    reg_val = 382;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 2, &reg_val));
    reg_val = 384;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Qr(unit, 3, &reg_val));
    reg_val = 511;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q45r(unit, 0, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q45r(unit, 1, &reg_val));

    /* Total IMP Hysteresis threshold */
    reg_val = 138; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 0, &reg_val));
    reg_val = 139;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 1, &reg_val));
    reg_val = 140;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 2, &reg_val));
    reg_val = 141;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_HYST_Q_IMPr(unit, 3, &reg_val));

    /* Total IMP Pause threshold */
    reg_val = 276; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 0, &reg_val));
    reg_val = 278;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 1, &reg_val));
    reg_val = 280;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 2, &reg_val));
    reg_val = 282;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_PAUSE_Q_IMPr(unit, 3, &reg_val));

    /* Total IMP Drop threshold */
    reg_val = 408; 
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 0, &reg_val));
    reg_val = 410;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 1, &reg_val));
    reg_val = 412;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 2, &reg_val));
    reg_val = 414;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_FC_TOTAL_TH_DROP_Q_IMPr(unit, 3, &reg_val));

    return SOC_E_NONE;
}

int 
drv_bcm53125_eav_control_set(int unit, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_CONTROL_TIME_STAMP_TO_IMP:
            SOC_IF_ERROR_RETURN(
                REG_READ_TM_STAMP_RPT_CTRLr(unit, &reg_value));
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            soc_TM_STAMP_RPT_CTRLr_field_set(
                unit, &reg_value, TSRPT_PKT_ENf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_TM_STAMP_RPT_CTRLr(unit, &reg_value));
            break;
        case DRV_EAV_CONTROL_MAX_AV_SIZE:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_MAX_AV_PKT_SZr(unit, &reg_value));
            temp = param;
            soc_AVB_MAX_AV_PKT_SZr_field_set(
                unit, &reg_value, MAX_AV_PKT_SZf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_MAX_AV_PKT_SZr(unit, &reg_value));
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSA_PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_CLASS_PCPr(unit, &reg_value));

            if (param > EAV_BCM53125_MAX_PCP_VALUE) {
                return SOC_E_PARAM;
            }
            soc_CLASS_PCPr_field_set(
                unit, &reg_value, CLASSA_PCPf, &param);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_CLASS_PCPr(unit, &reg_value));
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSB_PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_CLASS_PCPr(unit, &reg_value));

            if (param > EAV_BCM53125_MAX_PCP_VALUE) {
                return SOC_E_PARAM;
            }
            soc_CLASS_PCPr_field_set(
                unit, &reg_value, CLASSB_PCPf, &param);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_CLASS_PCPr(unit, &reg_value));
            break;
        case DRV_EAV_CONTROL_MMU_INIT:
            rv = _drv_bcm53125_eav_mmu_init(unit);
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}


int 
drv_bcm53125_eav_control_get(int unit, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0;
    
    switch (type) {
        case DRV_EAV_CONTROL_TIME_STAMP_TO_IMP:
            SOC_IF_ERROR_RETURN(
                REG_READ_TM_STAMP_RPT_CTRLr(unit, &reg_value));
            
            soc_TM_STAMP_RPT_CTRLr_field_get(
                unit, &reg_value, TSRPT_PKT_ENf, &temp);;
            if (temp) {
                *param = TRUE;
            } else {
                *param = FALSE;
            }
            break;
        case DRV_EAV_CONTROL_MAX_AV_SIZE:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_MAX_AV_PKT_SZr(unit, &reg_value));
            soc_AVB_MAX_AV_PKT_SZr_field_get(
                unit, &reg_value, MAX_AV_PKT_SZf, &temp);
            *param = temp;
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSA_PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_CLASS_PCPr(unit, &reg_value));

            soc_CLASS_PCPr_field_get(
                unit, &reg_value, CLASSA_PCPf, param);
            break;
        case DRV_EAV_CONTROL_STREAM_CLASSB_PCP:
            SOC_IF_ERROR_RETURN(
                REG_READ_CLASS_PCPr(unit, &reg_value));

            soc_CLASS_PCPr_field_get(
                unit, &reg_value, CLASSB_PCPf, param);
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
drv_bcm53125_eav_enable_set(int unit, uint32 port, uint32 enable)
{
    uint32 reg_value, temp = 0;

    /* Set EAV enable register */
    SOC_IF_ERROR_RETURN(
        REG_READ_AVB_AV_EN_CTRLr(unit, &reg_value));
    soc_AVB_AV_EN_CTRLr_field_get(unit, &reg_value, AV_ENf, &temp);
    if (enable){
        temp |= 0x1 << port;
    } else {
        temp &= ~(0x1 << port);
    }
    soc_AVB_AV_EN_CTRLr_field_set(unit, &reg_value, AV_ENf, &temp);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_AVB_AV_EN_CTRLr(unit, &reg_value));
    
    return SOC_E_NONE;
}

int 
drv_bcm53125_eav_enable_get(int unit, uint32 port, uint32 *enable)
{
    uint32 reg_value, temp = 0;
    
    SOC_IF_ERROR_RETURN(
        REG_READ_AVB_AV_EN_CTRLr(unit, &reg_value));
    soc_AVB_AV_EN_CTRLr_field_get(unit, &reg_value, AV_ENf, &temp);
    if (temp & (0x1 << port)){
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }
    return SOC_E_NONE;
}

int 
drv_bcm53125_eav_link_status_set(int unit, uint32 port, uint32 link)
{
    uint32 reg_value, temp = 0;

    /* Set EAV Link register */
    SOC_IF_ERROR_RETURN(
        REG_READ_EAV_LNK_STATUSr(unit, &reg_value));

    soc_EAV_LNK_STATUSr_field_get(unit, 
        &reg_value, PT_EAV_LNK_STATUSf, &temp);
    if (link){
        temp |= 0x1 << port;
    } else {
        temp &= ~(0x1 << port);
    }
    soc_EAV_LNK_STATUSr_field_set(unit, 
        &reg_value, PT_EAV_LNK_STATUSf, &temp);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_EAV_LNK_STATUSr(unit, &reg_value));
    
    return SOC_E_NONE;
}

int 
drv_bcm53125_eav_link_status_get(int unit, uint32 port, uint32 *link)
{
    uint32 reg_value, temp = 0;
    
    SOC_IF_ERROR_RETURN(
        REG_READ_EAV_LNK_STATUSr(unit, &reg_value));

    soc_EAV_LNK_STATUSr_field_get(unit, 
        &reg_value, PT_EAV_LNK_STATUSf, &temp);
    if (temp & (0x1 << port)){
        *link = TRUE;
    } else {
        *link = FALSE;
    }
    return SOC_E_NONE;
}

int
drv_bcm53125_eav_egress_timestamp_get(int unit, uint32 port,
    uint32 *timestamp)
{
    uint32 reg_value, temp = 0;

    /* Check Valid Status */
    SOC_IF_ERROR_RETURN(
        REG_READ_TM_STAMP_STATUSr(unit, &reg_value));
    soc_TM_STAMP_STATUSr_field_get(unit, &reg_value, VALID_STATUSf, &temp);
    
    if ((temp & (0x1 << port)) == 0) {
        return SOC_E_EMPTY;
    }

    /* Get Egress Time STamp Value */
    SOC_IF_ERROR_RETURN(
        REG_READ_AVB_EGRESS_TM_STAMPr(unit, port, &reg_value));
    *timestamp = reg_value;
    return SOC_E_NONE;
}


int 
drv_bcm53125_eav_time_sync_set(int unit, uint32 type, uint32 p0, uint32 p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_BASEr(unit, &reg_value));
            temp = p0;
            soc_AVB_TM_BASEr_field_set(unit, &reg_value, TM_BASEf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_TM_BASEr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_ADJr(unit, &reg_value));
            if ((p0 > EAV_BCM53125_MAX_TICK_INC) || 
                (p1 > EAV_BCM53125_MAX_TICK_ADJUST_PERIOD)) {
                return SOC_E_PARAM;
            }
            temp = p0;
            soc_AVB_TM_ADJr_field_set(unit, &reg_value, TM_INCf, &temp);
            temp = p1;
            soc_AVB_TM_ADJr_field_set(unit, &reg_value, TM_ADJ_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_TM_ADJr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
            if (p0 > EAV_BCM53125_MAX_TICK_ONE_SLOT) {
                return SOC_E_PARAM;
            }
            temp = p0;
            soc_AVB_SLOT_TICK_CNTRr_field_set(
                unit, &reg_value, TICK_CNTRf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
            if (p0 > EAV_BCM53125_MAX_SLOT_NUMBER) {
                return SOC_E_PARAM;
            }
            temp = p0;
            soc_AVB_SLOT_TICK_CNTRr_field_set(
                unit, &reg_value, SLOT_NUMf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            switch (p0) {
                case 1:
                    temp = 0;
                    break;
                case 2:
                    temp = 1;
                    break;
                case 4:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            soc_AVB_SLOT_ADJr_field_set(
                unit, &reg_value, MCRO_SLOT_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_SLOT_ADJr(unit, &reg_value));
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            switch (p0) {
                case 3125:
                    temp = 0;
                    break;
                case 3126:
                    temp = 1;
                    break;
                case 3124:
                    temp = 2;
                    break;
                default:
                    rv = SOC_E_PARAM;
                    return rv;
            }
            soc_AVB_SLOT_ADJr_field_set(
                unit, &reg_value, SLOT_ADJf, &temp);
            if (p1 >= 16) {
                rv =  SOC_E_PARAM;
                return rv;
            }
            temp = p1;
            soc_AVB_SLOT_ADJr_field_set(
                unit, &reg_value, SLOT_ADJ_PRDf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_SLOT_ADJr(unit, &reg_value));
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
drv_bcm53125_eav_time_sync_get(int unit, uint32 type, uint32 *p0, uint32 *p1)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0;
    
    switch (type) {
        case DRV_EAV_TIME_SYNC_TIME_BASE:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_BASEr(unit, &reg_value));
            soc_AVB_TM_BASEr_field_get(unit, &reg_value, TM_BASEf, &temp);
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TIME_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_TM_ADJr(unit, &reg_value));
            soc_AVB_TM_ADJr_field_get(unit, &reg_value, TM_INCf, &temp);
            *p0 = temp;
            soc_AVB_TM_ADJr_field_get(unit, &reg_value, TM_ADJ_PRDf, &temp);
            *p1 = temp;
            break;
        case DRV_EAV_TIME_SYNC_TICK_COUNTER:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
            soc_AVB_SLOT_TICK_CNTRr_field_get(
                unit, &reg_value, TICK_CNTRf, &temp);
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_SLOT_NUMBER:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_TICK_CNTRr(unit, &reg_value));
            soc_AVB_SLOT_TICK_CNTRr_field_get(
                unit, &reg_value, SLOT_NUMf, &temp);
            *p0 = temp;
            break;
        case DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD:
             SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            soc_AVB_SLOT_ADJr_field_get(
                unit, &reg_value, MCRO_SLOT_PRDf, &temp);
            switch(temp) {
                case 0:
                    *p0 = 1;
                    break;
                case 1:
                    *p0 = 2;
                    break;
                case 2:
                    *p0 = 4;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
            break;
        case DRV_EAV_TIME_SYNC_SLOT_ADJUST:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_SLOT_ADJr(unit, &reg_value));
            soc_AVB_SLOT_ADJr_field_get(
                unit, &reg_value, SLOT_ADJf, &temp);
            switch (temp) {
                case 0:
                    *p0 = 3125;
                    break;
                case 1:
                    *p0 = 3126;
                    break;
                case 2:
                    *p0 = 3124;
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
            soc_AVB_SLOT_ADJr_field_get(
                unit, &reg_value, SLOT_ADJ_PRDf, &temp);
            *p1 = temp;
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

int 
drv_bcm53125_eav_queue_control_set(int unit, 
    uint32 port, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            /* Q4 BW maxmum value = 16383(0x3fff) */
            if (param > EAV_53125_MAX_BANDWIDTH_VALUE) {
                soc_cm_debug(DK_ERR, 
                    "drv_bcm53125_eav_queue_control_set : BW unsupported. \n");
                return  SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C4_BW_CNTLr(unit, port, &reg_value));
            temp = param;
            soc_AVB_C4_BW_CNTLr_field_set(
                unit, &reg_value, C4_BWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_C4_BW_CNTLr(unit, port, &reg_value));
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            /* Q5 BW maxmum value = 16383(0x3fff) */
            /*    coverity[unsigned_compare]    */
            if (param > EAV_53125_MAX_BANDWIDTH_VALUE) {
                soc_cm_debug(DK_ERR, 
                    "drv_bcm53125_eav_queue_control_set : BW unsupported. \n");
                return  SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            temp = param;
            soc_AVB_C5_BW_CNTLr_field_set(
                unit, &reg_value, C5_BWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            soc_AVB_C5_BW_CNTLr_field_set(
                unit, &reg_value, C5_WNDWf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}
int 
drv_bcm53125_eav_queue_control_get(int unit, 
    uint32 port, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_value, temp = 0, max_value = 0;
    soc_field_info_t    *finfop = 0;
    
    switch (type) {
        case DRV_EAV_QUEUE_Q4_BANDWIDTH:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C4_BW_CNTLr(unit, port, &reg_value));
            soc_AVB_C4_BW_CNTLr_field_get(
                unit, &reg_value, C4_BWf, &temp);
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            soc_AVB_C5_BW_CNTLr_field_get(
                unit, &reg_value, C5_BWf, &temp);
            *param = temp;
            break;
        case DRV_EAV_QUEUE_Q5_WINDOW:
            SOC_IF_ERROR_RETURN(
                REG_READ_AVB_C5_BW_CNTLr(unit, port, &reg_value));
            soc_AVB_C5_BW_CNTLr_field_get(
                unit, &reg_value, C5_WNDWf, &temp);
            if (temp){
                *param = TRUE;
            } else {
                *param = FALSE;
            }
            break;
        case DRV_EAV_QUEUE_Q4_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 4 (macro slot time = 1)
              *
              * C4_Bandwidth(bytes/slot) = 
              *     Max_value(kbits/sec) * 1024 / (8 * macro slot time * 1000)
              *
              * C4_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * macro slot time* 1000)/(1024)) - 1
              */
            SOC_FIND_FIELD(C4_BWf,
                SOC_REG_INFO(unit, AVB_C4_BW_CNTLr).fields,
                SOC_REG_INFO(unit, AVB_C4_BW_CNTLr).nFields,
                finfop);
            assert(finfop);

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 1 * 1000) / (1024)) - 1;
            *param = max_value;
            break;
        case DRV_EAV_QUEUE_Q5_BANDWIDTH_MAX_VALUE:
            /*
              * Get the maximum valid bandwidth value for EAV Class 5
              *
              * Class 5 slot time is 125 us.
              * C5_Bandwidth(bytes/125us) = Max_value(kbits/sec) * 1024 / (8 * 8000)
              *
              * C5_Bandwidth (14 bits) = 0x3fff
              * Max_value = (((1<<14) * 8 * 8000)/(1024)) - 1
              */
            SOC_FIND_FIELD(C5_BWf,
                SOC_REG_INFO(unit, AVB_C5_BW_CNTLr).fields,
                SOC_REG_INFO(unit, AVB_C5_BW_CNTLr).nFields,
                finfop);
            assert(finfop);

            temp = (1 << finfop->len);
            max_value = ((temp * 8 * 8000) / 1024) - 1;
            *param = max_value;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}


int 
drv_bcm53125_eav_time_sync_mac_set(int unit, uint8* mac, uint16 ethertype)
{
    uint32 reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp;
    int rv =  SOC_E_NONE;

    /*
     * For time sync protocol, the mac should be set in Multi-address 0 register
     */

    /* 1. Set MAC and Ethertype value */
    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);

    COMPILER_64_ZERO(reg_val64);
    soc_MULTIPORT_ADDR0r_field_set(
        unit, (uint32 *)&reg_val64, MPORT_ADDRf, (uint32 *)&mac_field);
    
    if (ethertype) {
        temp = ethertype;
        soc_MULTIPORT_ADDR0r_field_set(
            unit, (uint32 *)&reg_val64, MPORT_E_TYPEf, &temp);
    }
    SOC_IF_ERROR_RETURN(
        REG_WRITE_MULTIPORT_ADDR0r(unit, (uint32 *)&reg_val64));

    /* 2. Set Forward map to CPU only */
    temp  = SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0);
    reg_val = 0;
    soc_MPORTVEC0r_field_set(unit, &reg_val, PORT_VCTRf, &temp);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_MPORTVEC0r(unit, &reg_val));

    /* 3. Enable Multi-address o */
    SOC_IF_ERROR_RETURN(
        REG_READ_MULTI_PORT_CTLr(unit, &reg_val));
    
    /* Set the match condition are MAC/Ethertype */
    if (ethertype) {
        temp = DRV_MULTIPORT_CTRL_MATCH_ETYPE_ADDR;
    } else {
        temp = DRV_MULTIPORT_CTRL_MATCH_ADDR;
    }

    soc_MULTI_PORT_CTLr_field_set(
        unit, &reg_val, MPORT_CTRL0f, &temp);
    /* Enable time stamped to CPU */
    temp = 1;
    soc_MULTI_PORT_CTLr_field_set(
        unit, &reg_val, MPORT0_TS_ENf, &temp); 

    SOC_IF_ERROR_RETURN(
        REG_WRITE_MULTI_PORT_CTLr(unit, &reg_val));

    return rv;
    
}

int 
drv_bcm53125_eav_time_sync_mac_get(int unit, uint8* mac, uint16 *ethertype)
{
    uint32 reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp = 0;
    int rv =  SOC_E_NONE;

    SOC_IF_ERROR_RETURN(
        REG_READ_MULTI_PORT_CTLr(unit, &reg_val));
    
    /* Get the value of time sync enable */
    soc_MULTI_PORT_CTLr_field_get(
        unit, &reg_val, MPORT0_TS_ENf, &temp);
    if ( temp == 0) {
        rv = SOC_E_DISABLED;
        return rv;
    }
    /* Get the Multi-address control value */
    soc_MULTI_PORT_CTLr_field_get(
        unit, &reg_val, MPORT_CTRL0f, &temp);
    if (temp == DRV_MULTIPORT_CTRL_DISABLE) {
        rv = SOC_E_DISABLED;
        return rv;
    }

    /* Get the MAC and Ethertype value */
    COMPILER_64_ZERO(reg_val64);
    COMPILER_64_ZERO(mac_field);
    SOC_IF_ERROR_RETURN(
        REG_READ_MULTIPORT_ADDR0r(unit, (uint32 *)&reg_val64));

    soc_MULTIPORT_ADDR0r_field_get(
        unit, (uint32 *)&reg_val64, MPORT_ADDRf, (uint32 *)&mac_field);
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    soc_MULTIPORT_ADDR0r_field_get(
            unit, (uint32 *)&reg_val64, MPORT_E_TYPEf, &temp);
    *ethertype = temp;

    return rv;
    
}
