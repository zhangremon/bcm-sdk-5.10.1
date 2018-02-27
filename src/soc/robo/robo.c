/*
 * $Id: robo.c 1.41.2.2 Broadcom SDK $
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
 * File:        robo.c
 * Purpose:
 * Requires:
 */
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/mcm/robo/driver.h>
#include <soc/error.h>

#include <soc/debug.h>

#ifdef BCM_ROBO_SUPPORT

/* Will be added and changed later */
/*
 * Robo chip driver functions.  Common across Robo devices for now.
 * These may get broken out by chip in the future, but not needed yet.
 */
soc_functions_t soc_robo_drv_funs = {
    soc_robo_misc_init,
    soc_robo_mmu_init,
    soc_robo_age_timer_get,
    soc_robo_age_timer_max_get,
    soc_robo_age_timer_set
};

int bcm53222_attached = 0;

/*
 * soc_robo_5324_mmu_default_set():
 *   - Do 5324M MMU default Configuration.
 */
int
soc_robo_5324_mmu_default_set(int unit) 
{
    uint32          reg_addr, reg_len, temp;
    uint32          reg_value = 0;
    int             rv = SOC_E_NONE;

    /* FCON_FLOWMIXr: Page Offset = 0x0A 0x30 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_FLOWMIXr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_FLOWMIXr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_MCAST_DROPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_UCAST_DROPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_TXQ_PAUSEf, &temp));
    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_RX_DROPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_RX_PAUSEf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_RX_FCON_CTRLr: Page Offset = 0x0A 0x92 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_RX_FCON_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_RX_FCON_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RX_FCON_CTRLr, &reg_value, EN_UNPAUSE_HDLf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_GLOB_TH_CTRL_2r: Page Offset = 0x0A 0x10 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_GLOB_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_GLOB_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xe7;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_2r, &reg_value, FCON_GLOB_DROP_THf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_2r, &reg_value, FCON_GLOB_MCDROP_THf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_GLOB_TH_CTRL_1r: Page Offset = 0x0A 0x0E */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_GLOB_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_GLOB_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xa3;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_1r, &reg_value, FCON_GLOB_PAUSE_THf, &temp));
    temp = 0x99;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_1r, &reg_value, FCON_GLOB_HYST_THf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q0_100_TH_CTRL_2r: Page Offset = 0x0A 0x08 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q0_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q0_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x2e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q1_100_TH_CTRL_2r: Page Offset = 0x0A 0x6C */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q1_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q1_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x2e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q2_100_TH_CTRL_2r: Page Offset = 0x0A 0x7A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q2_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q2_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x2e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q3_100_TH_CTRL_2r: Page Offset = 0x0A 0x88 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q3_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q3_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x2e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q0_100_TH_CTRL_1r: Page Offset = 0x0A 0x06 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q0_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q0_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x0b;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));
    temp = 0x7;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q1_100_TH_CTRL_1r: Page Offset = 0x0A 0x6A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q1_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q1_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x0b;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));
    temp = 0x7;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q2_100_TH_CTRL_1r: Page Offset = 0x0A 0x78 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q2_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q2_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x0b;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));
    temp = 0x7;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q3_100_TH_CTRL_1r: Page Offset = 0x0A 0x86 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q3_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q3_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x0b;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));
    temp = 0x7;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_MISC_TXFLOW_CTRLr: Page Offset = 0x0A 0x4A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_MISC_TXFLOW_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_MISC_TXFLOW_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_MISC_TXFLOW_CTRLr, &reg_value, RESERVE_BLANCEf, &temp));
    temp = 0xe7;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_MISC_TXFLOW_CTRLr, &reg_value, TOTAL_1QTH_DROPf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_RSRV_BUFNUMr: Page Offset = 0x0A 0x90 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_RSRV_BUFNUMr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_RSRV_BUFNUMr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x8;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RSRV_BUFNUMr, &reg_value, RXBASE_RESV_BUFNUMf, &temp));
    temp = 0x4;
    if (SOC_IS_ROBO5324_A1(unit)) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, FCON_RSRV_BUFNUMr, &reg_value, RUNOFFf, &temp));
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, FCON_RSRV_BUFNUMr, &reg_value, RESERVED_Rf, &temp));
    }

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    return SOC_E_NONE;
}

/*
 * soc_robo_5348_mmu_default_set():
 *   - Do 5348 MMU default Configuration.
 */
int
soc_robo_5348_mmu_default_set(int unit) 
{
    uint32          reg_addr, reg_len, temp;
    uint32          reg_value = 0;
    int             rv = SOC_E_NONE;

    /* FCON_RX_FCON_CTRLr: Page Offset = 0x0A 0x92 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_RX_FCON_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_RX_FCON_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x63e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RX_FCON_CTRLr, &reg_value, RXFCON_PAUSE_THf, &temp));

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RX_FCON_CTRLr, &reg_value, EN_RXCON_PAUSE_REMAPf, &temp));

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RX_FCON_CTRLr, &reg_value, EN_UNPAUSE_HDLf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_FLOWMIXr: Page Offset = 0x0A 0x30 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_FLOWMIXr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_FLOWMIXr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_MCAST_DROPf, &temp));

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_UCAST_DROPf, &temp));

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_TXQ_PAUSEf, &temp));

    temp = 0x0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_RX_DROPf, &temp));

    temp = 0x0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_RX_PAUSEf, &temp));

    temp = 0x3;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, QOS_RSRV_QUOTA_OPTf, &temp));

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_PARKING_PREVENTIONf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_GLOB_TH_CTRL_2r: Page Offset = 0x0A 0x10 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_GLOB_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_GLOB_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xed;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_2r, &reg_value, FCON_GLOB_DROP_THf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_2r, &reg_value, FCON_GLOB_MCDROP_THf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_GLOB_TH_CTRL_1r: Page Offset = 0x0A 0x0E */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_GLOB_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_GLOB_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xaf;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_1r, &reg_value, FCON_GLOB_PAUSE_THf, &temp));
    temp = 0x88;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_1r, &reg_value, FCON_GLOB_HYST_THf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q0_100_TH_CTRL_2r: Page Offset = 0x0A 0x08 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q0_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q0_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q1_100_TH_CTRL_2r: Page Offset = 0x0A 0x6C */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q1_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q1_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q2_100_TH_CTRL_2r: Page Offset = 0x0A 0x7A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q2_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q2_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q3_100_TH_CTRL_2r: Page Offset = 0x0A 0x88 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q3_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q3_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));

    temp = 0xde;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q0_100_TH_CTRL_1r: Page Offset = 0x0A 0x06 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q0_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q0_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x27;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    temp = 0x1e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q1_100_TH_CTRL_1r: Page Offset = 0x0A 0x6A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q1_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q1_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x27;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    temp = 0x1e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q2_100_TH_CTRL_1r: Page Offset = 0x0A 0x78 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q2_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q2_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x27;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    temp = 0x1e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q3_100_TH_CTRL_1r: Page Offset = 0x0A 0x86 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q3_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q3_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x27;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    temp = 0x1e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_MISC_TXFLOW_CTRLr: Page Offset = 0x0A 0x4A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_MISC_TXFLOW_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_MISC_TXFLOW_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_MISC_TXFLOW_CTRLr, &reg_value, RESERVE_BLANCEf, &temp));
    temp = 0xed;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_MISC_TXFLOW_CTRLr, &reg_value, TOTAL_LQTH_DROPf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_RXBASE_BUFNUMr: Page Offset = 0x0A 0x8A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_RXBASE_BUFNUMr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_RXBASE_BUFNUMr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x8;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RXBASE_BUFNUMr, &reg_value, RXBASE_RESV_BUFNUMf, &temp));

    temp = 0x8;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RXBASE_BUFNUMr, &reg_value, RXBASE_HYST_BUFNUMf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_RSRV_BUFNUMr: Page Offset = 0x0A 0x90 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_RSRV_BUFNUMr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_RSRV_BUFNUMr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x8;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RSRV_BUFNUMr, &reg_value, RUNOFFf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_STRICT_HIQ_PRE_CTRL0r: Page Offset = 0x0A 0x40 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_STRICT_HIQ_PRE_CTRL0r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_STRICT_HIQ_PRE_CTRL0r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xffff;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_STRICT_HIQ_PRE_CTRL0r, &reg_value, EN_STRICT_HIQ_PREf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_STRICT_HIQ_PRE_CTRL1r: Page Offset = 0x0A 0x42 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_STRICT_HIQ_PRE_CTRL1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_STRICT_HIQ_PRE_CTRL1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xffff;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_STRICT_HIQ_PRE_CTRL1r, &reg_value, EN_STRICT_HIQ_PREf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_STRICT_HIQ_PRE_CTRL2r: Page Offset = 0x0A 0x44 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_STRICT_HIQ_PRE_CTRL2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_STRICT_HIQ_PRE_CTRL2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xffff;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_STRICT_HIQ_PRE_CTRL2r, &reg_value, EN_STRICT_HIQ_PREf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_STRICT_HIQ_PRE_CTRL3r: Page Offset = 0x0A 0x46 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_STRICT_HIQ_PRE_CTRL3r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_STRICT_HIQ_PRE_CTRL3r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x1f;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_STRICT_HIQ_PRE_CTRL3r, &reg_value, EN_STRICT_HIQ_PREf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    return SOC_E_NONE;
}

/*
 * soc_robo_53242_mmu_default_set():
 *   - Do 53242 MMU default Configuration.
 */
int
soc_robo_53242_mmu_default_set(int unit) 
{
    uint32          reg_addr, reg_len, temp;
    uint32          reg_value = 0;
    int             rv = SOC_E_NONE;

    /* FCON_Q0_100_TH_CTRL_1r: Page Offset = 0x0A 0x06 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q0_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q0_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x13;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q0_100_TH_CTRL_2r: Page Offset = 0x0A 0x08 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q0_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q0_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x98;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q0_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_GLOB_TH_CTRL_1r: Page Offset = 0x0A 0x0E */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_GLOB_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_GLOB_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x44;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_1r, &reg_value, FCON_GLOB_HYST_THf, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_1r, &reg_value, FCON_GLOB_PAUSE_THf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_GLOB_TH_CTRL_2r: Page Offset = 0x0A 0x10 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_GLOB_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_GLOB_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x9b;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_2r, &reg_value, FCON_GLOB_DROP_THf, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_GLOB_TH_CTRL_2r, &reg_value, FCON_GLOB_MCDROP_THf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_FLOWMIXr: Page Offset = 0x0A 0x30 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_FLOWMIXr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_FLOWMIXr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x3;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, QOS_RSRV_QUOTA_OPTf, &temp));
    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_PARKING_PREVENTIONf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_MCAST_BLANCEf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_MCAST_DROPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_UCAST_DROPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_TXQ_PAUSEf, &temp));
    temp = 0x0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_RX_DROPf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_FLOWMIXr, &reg_value, EN_RX_PAUSEf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_MISC_TXFLOW_CTRLr: Page Offset = 0x0A 0x4A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_MISC_TXFLOW_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_MISC_TXFLOW_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_MISC_TXFLOW_CTRLr, &reg_value, RESERVE_BLANCEf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q1_100_TH_CTRL_1r: Page Offset = 0x0A 0x6A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q1_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q1_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x13;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q1_100_TH_CTRL_2r: Page Offset = 0x0A 0x6C */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q1_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q1_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x98;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q1_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q2_100_TH_CTRL_1r: Page Offset = 0x0A 0x78 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q2_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q2_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x13;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q2_100_TH_CTRL_2r: Page Offset = 0x0A 0x7A */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q2_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q2_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x98;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q2_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q3_100_TH_CTRL_1r: Page Offset = 0x0A 0x86 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q3_100_TH_CTRL_1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q3_100_TH_CTRL_1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x13;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_1r, &reg_value, BT100_HYST_THRSf, &temp));
    temp = 0x1c;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_1r, &reg_value, BT100_PAUS_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_Q3_100_TH_CTRL_2r: Page Offset = 0x0A 0x88 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_Q3_100_TH_CTRL_2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_Q3_100_TH_CTRL_2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x98;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_2r, &reg_value, BT100_DROP_THRSf, &temp));
    temp = 0x73;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_Q3_100_TH_CTRL_2r, &reg_value, BT100_MCDROP_THRSf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_RX_FCON_CTRLr: Page Offset = 0x0A 0x92 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_RX_FCON_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_RX_FCON_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_RX_FCON_CTRLr, &reg_value, EN_UNPAUSE_HDLf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_DLF_TH_CTRLr: Page Offset = 0x0A 0x94 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_DLF_TH_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_DLF_TH_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_DLF_TH_CTRLr, &reg_value, TOTAL_INDV_DLFTH_DROPf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* FCON_BCST_TH_CTRLr: Page Offset = 0x0A 0x96 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, FCON_BCST_TH_CTRLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, FCON_BCST_TH_CTRLr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, FCON_BCST_TH_CTRLr, &reg_value, TOTAL_INDV_BCSTTH_DROPf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_HYST_THRESH_Q1r: Page Offset = 0x0A 0xC0 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_HYST_THRESH_Q1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_HYST_THRESH_Q1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x46;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_HYST_THRESH_Q1r, &reg_value, TL_HYST_TH_Q1f, &temp));
    temp = 0x80;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_HYST_THRESH_Q1r, &reg_value, TL_PAUSE_TH_Q1f, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_DROP_THRESH_Q1r: Page Offset = 0x0A 0xC2 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_DROP_THRESH_Q1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_DROP_THRESH_Q1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x9d;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DROP_THRESH_Q1r, &reg_value, TL_DROP_TH_Q1f, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DROP_THRESH_Q1r, &reg_value, RESERVED_Rf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_HYST_THRESH_Q2r: Page Offset = 0x0A 0xC4 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_HYST_THRESH_Q2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_HYST_THRESH_Q2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x48;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_HYST_THRESH_Q2r, &reg_value, TL_HYST_TH_Q2f, &temp));
    temp = 0x82;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_HYST_THRESH_Q2r, &reg_value, TL_PAUSE_TH_Q2f, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_DROP_THRESH_Q2r: Page Offset = 0x0A 0xC6 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_DROP_THRESH_Q2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_DROP_THRESH_Q2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x9f;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DROP_THRESH_Q2r, &reg_value, TL_DROP_TH_Q2f, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DROP_THRESH_Q2r, &reg_value, RESERVED_Rf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_HYST_THRESH_Q3r: Page Offset = 0x0A 0xC8 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_HYST_THRESH_Q3r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_HYST_THRESH_Q3r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x4a;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_HYST_THRESH_Q3r, &reg_value, TL_HYST_TH_Q3f, &temp));
    temp = 0x84;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_HYST_THRESH_Q3r, &reg_value, TL_PAUSE_TH_Q3f, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_DROP_THRESH_Q3r: Page Offset = 0x0A 0xCA */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_DROP_THRESH_Q3r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_DROP_THRESH_Q3r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0xa1;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DROP_THRESH_Q3r, &reg_value, TL_DROP_TH_Q3f, &temp));
    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DROP_THRESH_Q3r, &reg_value, RESERVED_Rf, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_DLF_DROP_THRESH_Q1r: Page Offset = 0x0A 0xD0 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_DLF_DROP_THRESH_Q1r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_DLF_DROP_THRESH_Q1r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DLF_DROP_THRESH_Q1r, &reg_value, TOTAL_DLF_DROP_THRESH_Q1f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DLF_DROP_THRESH_Q1r, &reg_value, TOTAL_BC_DROP_THRESH_Q1f, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_DLF_DROP_THRESH_Q2r: Page Offset = 0x0A 0xD2 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_DLF_DROP_THRESH_Q2r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_DLF_DROP_THRESH_Q2r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DLF_DROP_THRESH_Q2r, &reg_value, TOTAL_DLF_DROP_THRESH_Q2f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DLF_DROP_THRESH_Q2r, &reg_value, TOTAL_BC_DROP_THRESH_Q2f, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    /* TOTAL_DLF_DROP_THRESH_Q3r: Page Offset = 0x0A 0xD4 */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TOTAL_DLF_DROP_THRESH_Q3r, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TOTAL_DLF_DROP_THRESH_Q3r);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    temp = 0x7e;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DLF_DROP_THRESH_Q3r, &reg_value, TOTAL_DLF_DROP_THRESH_Q3f, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, TOTAL_DLF_DROP_THRESH_Q3r, &reg_value, TOTAL_BC_DROP_THRESH_Q3f, &temp));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    return SOC_E_NONE;
}

/* soc_misc_init() :
 *  - allowed user to do the init by chip dependant configuration.
 *
 *  Note : 
 *   1. below routine is for all Roob chip related init routine.
 *   2. different robo chip init section may separated by 
 *      "SOC_IS_ROBO53xx(unit)"
 */
int
soc_robo_misc_init(int unit)
{
    uint32          temp;
    uint32          reg_value = 0;
    int             rv = SOC_E_NONE;
    soc_pbmp_t pbmp;
    
    
    /* CheckMe: bcm5324 misc init may change to this function.
     *       (cucrrently the bcm5324 misc init is located in bcm_robo_init())
     */
    if (SOC_IS_ROBO5324(unit)){
        soc_robo_5324_mmu_default_set(unit);
    } else if (SOC_IS_ROBO5396(unit)){
        /* enable Exterenal PHY auto-polling */
        rv = REG_READ_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        temp = 1;
        soc_EXTPHY_SCAN_CTLr_field_set(unit, &reg_value, PHY_SCAN_ENf, &temp);

        rv = REG_WRITE_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
        
        SOC_ERROR_PRINT((DK_ERR, 
                    "misc_init: External PHY auto-polling enabled\n"));
        
    } else if (SOC_IS_ROBO5397(unit)){
#ifdef WAN_PORT_SUPPORT

        rv = REG_READ_WAN_PORT_SELr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
        temp = 0x80; /* Select WAN port */
        soc_WAN_PORT_SELr_field_set(unit, &reg_value, WAN_SELECTf, &temp);
        temp = 1; /* Set non-Egress direct to WAN port */
        soc_WAN_PORT_SELr_field_set(unit, &reg_value, EN_MAN2WANf, &temp);

        rv = REG_WRITE_WAN_PORT_SELr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
#endif
    } else if (SOC_IS_ROBO5348(unit) ||  SOC_IS_ROBO5347(unit)){
        soc_robo_5348_mmu_default_set(unit);
    } else if (SOC_IS_ROBO53242(unit) ||  SOC_IS_ROBO53262(unit)){
        soc_robo_53242_mmu_default_set(unit);

        rv = REG_READ_BONDING_PADr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        reg_value &= 0x1f;
        reg_value >>= 1;

        if (reg_value == 0xf) {
            bcm53222_attached = 1;
        }
    } else if (SOC_IS_ROBO53115(unit)||SOC_IS_ROBO53101(unit)||
        SOC_IS_ROBO53125(unit)){

        /* Configure WAN ports if any */
        pbmp = soc_property_get_pbmp(unit, spn_PBMP_WAN_PORT, 0);
        SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
        if (SOC_PBMP_NOT_NULL(pbmp)) {
            rv = REG_READ_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
            temp = SOC_PBMP_WORD_GET(pbmp, 0);
            soc_WAN_PORT_SELr_field_set(unit, &reg_value, WAN_SELECTf, &temp);
    
            rv = REG_WRITE_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
        }

        /* Enable dual-imp mode */
        temp = soc_property_get(unit, spn_DUAL_IMP_ENABLE, 0);

        if (SOC_PBMP_MEMBER(pbmp, 5)) {
            /* Port 5 can be selected WAN port only when dual-imp disabled */
            temp = 0;
        }

        if (temp) {
            /* Dual-IMP */
            temp = 0x3;
        } else {
            /* Single-IMP */
            temp = 0x2;
        }

        rv = REG_READ_GMNGCFGr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
        soc_GMNGCFGr_field_set(unit, &reg_value, FRM_MNGPf, &temp);

        rv = REG_WRITE_GMNGCFGr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
    } else if (SOC_IS_ROBO53118(unit)||SOC_IS_ROBO5395(unit) ||
        SOC_IS_ROBO53128(unit)){

        /* Configure WAN ports if any */
        pbmp = soc_property_get_pbmp(unit, spn_PBMP_WAN_PORT, 0);
        SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
        if (SOC_PBMP_NOT_NULL(pbmp)) {
#ifdef BCM_53128
            if (SOC_IS_ROBO53128(unit)) {
                rv = REG_READ_MDIO_PORT7_ADDRr(unit, &reg_value);
                SOC_IF_ERROR_RETURN(rv);
                temp = SOC_PBMP_WORD_GET(pbmp, 0);
                soc_MDIO_PORT7_ADDRr_field_set(unit, &reg_value, WAN_SELECTf, &temp);
        
                rv = REG_WRITE_MDIO_PORT7_ADDRr(unit, &reg_value);
                SOC_IF_ERROR_RETURN(rv);
            } else {
#endif /* BCM_53128 */
            rv = REG_READ_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
            temp = SOC_PBMP_WORD_GET(pbmp, 0);
            soc_WAN_PORT_SELr_field_set(unit, &reg_value, WAN_SELECTf, &temp);
    
            rv = REG_WRITE_WAN_PORT_SELr(unit, &reg_value);
            SOC_IF_ERROR_RETURN(rv);
#ifdef BCM_53128
            }
#endif /* BCM_53128 */
        }

    /* appended for next robo chip */
    } else {
    }
    
    /* disable device base PHY auto-scan behavior : 
    *   - checkMe : check if only bcm5324, bcm5348 and bcm5396 has device 
    *           base enable/disable setting for phy auto-scan.
    */
    if  (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) || 
            SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit)||
            SOC_IS_ROBO53262(unit) || SOC_IS_TBX(unit)){
        rv = REG_READ_PHYSCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        temp = 0;            
        soc_PHYSCAN_CTLr_field_set(unit,  &reg_value, EN_PHY_SCANf, &temp);

        rv = REG_WRITE_PHYSCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

    } else if (SOC_IS_ROBO5396(unit)){    /* bcm5396 only */
        rv = REG_READ_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);

        temp = 0;
        soc_EXTPHY_SCAN_CTLr_field_set(unit, &reg_value, PHY_SCAN_ENf, &temp);

        rv = REG_WRITE_EXTPHY_SCAN_CTLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
    }

        
    /* TB misc_init :
     *  1. Port Mask table reset.
     *  2. Flow Control init :
     *      - global XOFF enable.
     *      - Port basis XOFF disable.
     *  3. User MAC address
     */
    if (SOC_IS_TBX(unit)){
        uint64  reg64_val;
        bcm_pbmp_t  mport_vctr;
        uint8       mport_addr[6];
        
        /* Reset PORTMASK */
        SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_PORTMASK));

        /* Flow control init : enable global XOFF */
        temp = 1;
        SOC_IF_ERROR_RETURN(REG_READ_NEW_CONTROLr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NEW_CONTROLr_field_set(
                unit, &reg_value, EN_SW_FLOW_CONf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NEW_CONTROLr(unit, &reg_value));
        
        /* Flow control init : diable XOFF on each port */
        COMPILER_64_ZERO(reg64_val);
        SOC_IF_ERROR_RETURN(REG_WRITE_SW_XOFF_PORT_CTLr(unit, &reg64_val));
        
        /* User MAC addresses init process
         *  1. clear user address(global will be disabled if all user 
         *      addresses were deleted)
         *  2. Set the the VLAN bypass default setting about user address.
         */
        SOC_PBMP_CLEAR(mport_vctr);
        ENET_SET_MACADDR(mport_addr, _soc_mac_all_zeroes);
        SOC_IF_ERROR_RETURN(DRV_MAC_SET(
                unit, mport_vctr, DRV_MAC_MULTIPORT_0, mport_addr, 0));
        SOC_IF_ERROR_RETURN(DRV_MAC_SET(
                unit, mport_vctr, DRV_MAC_MULTIPORT_1, mport_addr, 0));
                
        /* default at VLAN bypass l2 address is for the lagcy support 
         *  - DVAPI for all robo chip will test such bypass behavior
         */
        SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET(
                unit, DRV_VLAN_PROP_BYPASS_L2_USER_ADDR, TRUE));
    }
    
    /* reset the Traffic remarking on Non-ResEPacket */
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)){        
        soc_port_t  port;
        uint32      pri;
        
        /* bcm5395 on this feature is not implemented, so this reset exclude 
         * bcm5395 related setting.
         */
         
        /* reset the control register */
        rv = REG_READ_TRREG_CTRLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
        temp = 0;
        soc_TRREG_CTRLr_field_set(unit, &reg_value, PCP_RMK_ENf, &temp);
        if (!(SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53128(unit))) {
        	soc_TRREG_CTRLr_field_set(unit, &reg_value, CFI_RMK_ENf, &temp);
        }

        rv = REG_WRITE_TRREG_CTRLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);
        
        
        /* reset the TC2PCP mapping */
        PBMP_ALL_ITER(unit, port){
            for (pri = 0; pri <=7; pri++){
                
                /* the new-pri is formed as {CFI(bit4),PRI(bit3-bit0)} 
                 *  - in the reset value, the CFI is rewrite to 0 always!
                 *      (default is 1 on the RV=1 field)
                 */
                SOC_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET(unit, port, 
                        DRV_PORT_OP_NORMAL_TC2PCP, pri, 0, pri, 0));
                
                /* outband TC2PCP is supported on bcm53115 only */
                if (SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53125(unit)){
                    SOC_IF_ERROR_RETURN(DRV_PORT_PRI_MAPOP_SET(unit, port, 
                            DRV_PORT_OP_OUTBAND_TC2PCP, pri, 0, pri, 0));
                    
                }
            }
        }
    }

    /* Enable the SA learning of reserved mutilcasts */
    if (SOC_IS_ROBO5389(unit) || SOC_IS_ROBO5396(unit) || 
        SOC_IS_ROBO5395(unit) || SOC_IS_ROBO5397(unit) ||
        SOC_IS_ROBO5398(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {

        temp = 1;
        SOC_IF_ERROR_RETURN(
            REG_READ_RSV_MCAST_CTRLr(unit, &reg_value));
        soc_RSV_MCAST_CTRLr_field_set(unit, &reg_value, 
            EN_RES_MUL_LEARNf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_RSV_MCAST_CTRLr(unit, &reg_value));
    }
    
    SOC_ERROR_PRINT((DK_VERBOSE, "soc_robo_misc_init: OK\n"));
    
    return rv;
}

int
soc_robo_mmu_init(int unit)
{
    return SOC_E_NONE;
}

int
soc_robo_age_timer_get(int unit, int *age_seconds, int *enabled)
{
    return SOC_E_NONE;
}

int
soc_robo_age_timer_max_get(int unit, int *max_seconds)
{
    return SOC_E_NONE;
}

int
soc_robo_age_timer_set(int unit, int age_seconds, int enable)
{
    return SOC_E_NONE;
}

/*
 * soc_robo_64_val_to_pbmp() :
 *     -- Get port bitmap from unsigned 64-bits integer variable.
 * 
 * unit: unit
 * *pbmp: (OUT) returned port bitmap
 * value64: (IN) data value for transfering into port bitmap
 */
int
soc_robo_64_val_to_pbmp(int unit, soc_pbmp_t *pbmp, uint64 value64)
{
    uint32 value32;

    COMPILER_64_TO_32_LO(value32, value64);
    SOC_PBMP_WORD_SET(*pbmp, 0, value32);
    if (SOC_INFO(unit).port_num > 32) {
        COMPILER_64_TO_32_HI(value32, value64);
        SOC_PBMP_WORD_SET(*pbmp, 1, value32);
    } else {
        SOC_PBMP_WORD_SET(*pbmp, 1, 0);
    }
    
    return SOC_E_NONE;
}

/*
 * soc_robo_64_pbmp_to_val() :
 *     -- Transfer port bitmap into unsigned 64-bits integer variable.
 * 
 * unit: unit
 * *pbmp: (IN) returned port bitmap
 * *value64: (OUT) data value for transfering into port bitmap
 */
int
soc_robo_64_pbmp_to_val(int unit, soc_pbmp_t *pbmp, uint64 *value64)
{
    uint32 value_h, value_l;

    value_l = SOC_PBMP_WORD_GET(*pbmp, 0);
    if (SOC_INFO(unit).port_num > 32) {
        value_h = SOC_PBMP_WORD_GET(*pbmp, 1);
    } else {
        value_h = 0;
    }
    COMPILER_64_SET(*value64, value_h, value_l);

    return SOC_E_NONE;
}

int
soc_robo_loop_detect_enable_set(int unit, int enable)
{
    uint32 reg_addr, reg_len;
    uint32 reg_value = 0;
    int rv = SOC_E_NONE;

    /* enable Loop Detection */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, LPDET_CFGr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, LPDET_CFGr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    /* enable/disable loop detection */
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, LPDET_CFGr, &reg_value, EN_LPDETf, (uint32 *) &enable));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value, reg_len));

    return SOC_E_NONE;
}

int
soc_robo_loop_detect_enable_get(int unit, int *enable)
{
    uint32 reg_addr, reg_len;
    uint32 reg_value = 0;
    int rv = SOC_E_NONE;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, LPDET_CFGr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, LPDET_CFGr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value, reg_len)) < 0) {
        return rv;
    }

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, LPDET_CFGr, &reg_value, EN_LPDETf, (uint32 *) enable));

    return SOC_E_NONE;
}

int
soc_robo_loop_detect_address_set(int unit, sal_mac_addr_t macaddr)
{
    uint32 reg_addr, reg_len;
    uint64 reg_v64, mac_field;

    SAL_MAC_ADDR_TO_UINT64(macaddr, mac_field);

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, LD_FRM_SAr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, LD_FRM_SAr);

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, LD_FRM_SAr, (uint32 *)&reg_v64, LD_SAf, (uint32 *)&mac_field));

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_v64, reg_len));

    return SOC_E_NONE;
}

int
soc_robo_loop_detect_pbmp_get(int unit, soc_pbmp_t *pbmp)
{
    uint32 reg_addr, reg_len, reg_value32, pbmp_value32;
    uint64 pbmp_value64, reg_value64;
    int rv = SOC_E_NONE;

    COMPILER_64_ZERO(reg_value64);

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
                    (unit, LED_PORTMAPr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
                    (unit, LED_PORTMAPr);
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
            return rv;
        }
        if (SOC_IS_ROBO5348(unit)) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, LED_PORTMAPr, (uint32 *)&reg_value64, 
            LED_WARNING_PORTMAPf, (uint32 *)&pbmp_value64));
        soc_robo_64_val_to_pbmp(unit, pbmp, pbmp_value64);
        } else { /* 5347 */
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, LED_PORTMAPr, (uint32 *)&reg_value64, 
                LED_WARNING_PORTMAPf, (uint32 *)&pbmp_value32));
            SOC_PBMP_WORD_SET(*pbmp, 0, pbmp_value32);
        }
    } else {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_value32, reg_len)) < 0) {
            return rv;
        }
    
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, LED_PORTMAPr, &reg_value32, LED_WARNING_PORTMAPf, &pbmp_value32));
    
        SOC_PBMP_WORD_SET(*pbmp, 0, pbmp_value32);
    }

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_dev_control_set
 * Purpose:
 *	    Set the system basis management or control functions. Especially for 
 *      those functions which provides a simple on/off or enable/disable 
 *      option to control device's working feature.
 *      Also, this driver interface provide some kinds of flexibility for 
 *      designer on implementing the device management mode configuration 
 *      instead of creating a individual driver interface.
 * Parameters:
 *	    ctrl_cnt    - (IN/OUT) the number count to indicate how many pairs of 
 *                      control types and values.
 *                      >> Output the number of proceeded control set items.
 *      type_list   - (IN) control type list
 *      value_list  - (IN) control value list
 * Returns:
 *	    
 * Notes:
 *  1. the contorl type and value of the same list index must be matched as 
 *      a pair.
 */

int
drv_dev_control_set(int unit,uint32 *ctrl_cnt,
                uint32 *type_list,int *value_list)
{
    uint32  op_cnt = 0;
    uint32  reg_value = 0, fld_valud = 0;
    int     i, rv = SOC_E_NONE;

    /* Null check */
    if (op_cnt){
        assert(type_list && value_list);
    }
    
    for (i = 0; i < *ctrl_cnt; i++){

        fld_valud = (value_list[i] == 0) ? 0 : 1;
        if (type_list[i] == DRV_DEV_CTRL_RESERVED_MCAST_SA_LEARN){
#if defined(BCM_5389) || defined(BCM_5396) || defined(BCM_5395) || defined(BCM_5398) || defined(BCM_5397) || defined(BCM_53115) || defined(BCM_53118) || defined(BCM_53101) || defined(BCM_53125) || defined(BCM_53128)
            if (SOC_IS_ROBO5389(unit) || SOC_IS_ROBO5396(unit) || 
                    SOC_IS_ROBO5395(unit) || SOC_IS_ROBO5397(unit) ||
                    SOC_IS_ROBO5398(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {

                SOC_IF_ERROR_RETURN(
                    REG_READ_RSV_MCAST_CTRLr(unit, &reg_value));
                soc_RSV_MCAST_CTRLr_field_set(unit, &reg_value, 
                    EN_RES_MUL_LEARNf, &fld_valud);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_RSV_MCAST_CTRLr(unit, &reg_value));

            } else {
                rv = SOC_E_UNAVAIL;
            }
#else 
            rv = SOC_E_UNAVAIL;
#endif
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXULF){
            if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) || 
                    SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit) ||
                    SOC_IS_ROBO53262(unit) || SOC_IS_ROBO5389(unit) ||
                    SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
                    SOC_IS_ROBO5396(unit) || SOC_IS_ROBO5395(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_set(unit, &reg_value, 
                        MIRX_UC_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_set(unit, &reg_value, 
                        RX_UCST_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
            } else {
                rv = SOC_E_UNAVAIL;
            }
        
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXMLF){
            if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) || 
                    SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit) ||
                    SOC_IS_ROBO53262(unit) || SOC_IS_ROBO5389(unit) ||
                    SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
                    SOC_IS_ROBO5396(unit) || SOC_IS_ROBO5395(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_set(unit, &reg_value, 
                        MIRX_MC_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_set(unit, &reg_value, 
                        RX_MCST_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
            } else {
                rv = SOC_E_UNAVAIL;
            }
        
        } else {
            if (type_list[i] < DRV_DEV_CTRL_CNT){
                rv = SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        }
        
        if (rv){
            *ctrl_cnt = op_cnt;
            break;
            
        }
        op_cnt++;
    }
    
    return rv;

}

/*
 * Function: 
 *	    drv_dev_control_get
 * Purpose:
 *	    Get the system basis management or control functions. Especially for 
 *      those functions which provides a simple on/off or enable/disable 
 *      option to control device's working feature.
 *      Also, this driver interface provide some kinds of flexibility for 
 *      designer on implementing the device management mode configuration 
 *      instead of creating a individual driver interface.
 * Parameters:
 *	    ctrl_cnt    - (IN/OUT) the number count to indicate how many pairs of 
 *                      control types and values.
 *                      >> Output the number of proceeded control set items.
 *      type_list   - (IN) control type list
 *      value_list  - (OUT) control value list
 * Returns:
 *	    
 * Notes:
 *  1. the contorl type and value of the same list index must be matched as 
 *      a pair.
 */

int
drv_dev_control_get(int unit,uint32 *ctrl_cnt,
                uint32 *type_list,int *value_list)
{
    uint32  op_cnt = 0;
    uint32  reg_value = 0, field_val = 0;
    int     i, rv = SOC_E_NONE;

    /* Null check */
    if (op_cnt){
        assert(type_list && value_list);
    }
    
    for (i = 0; i < *ctrl_cnt; i++){
        if (type_list[i] == DRV_DEV_CTRL_RESERVED_MCAST_SA_LEARN){
#if defined(BCM_5389) || defined(BCM_5396) || defined(BCM_5395) || defined(BCM_5398) || defined(BCM_5397) || defined(BCM_53115) || defined(BCM_53118) || defined(BCM_53101) || defined(BCM_53125) || defined(BCM_53128)
            if (SOC_IS_ROBO5389(unit) || SOC_IS_ROBO5396(unit) || 
                    SOC_IS_ROBO5395(unit) || SOC_IS_ROBO5397(unit) ||
                    SOC_IS_ROBO5398(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)) {

                SOC_IF_ERROR_RETURN(
                    REG_READ_RSV_MCAST_CTRLr(unit, &reg_value));
                soc_RSV_MCAST_CTRLr_field_get(unit, &reg_value, 
                    EN_RES_MUL_LEARNf, &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }

#else
            rv = SOC_E_UNAVAIL;
#endif
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXULF){
            if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) || 
                    SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit) ||
                    SOC_IS_ROBO53262(unit) || SOC_IS_ROBO5389(unit) ||
                    SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
                    SOC_IS_ROBO5396(unit) || SOC_IS_ROBO5395(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_get(unit, &reg_value, 
                        MIRX_UC_ENf, &field_val);
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_get(unit, &reg_value, 
                        RX_UCST_ENf, &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
        
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXMLF){
            if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) || 
                    SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit) ||
                    SOC_IS_ROBO53262(unit) || SOC_IS_ROBO5389(unit) ||
                    SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
                    SOC_IS_ROBO5396(unit) || SOC_IS_ROBO5395(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_get(unit, &reg_value, 
                        MIRX_MC_ENf, &field_val);
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_get(unit, &reg_value, 
                        RX_MCST_ENf, &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
        
        } else {
            if (type_list[i] < DRV_DEV_CTRL_CNT){
                rv = SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        }
        value_list[i] = field_val;    

        if (rv){
            *ctrl_cnt = op_cnt;
            break;
            
        }
        op_cnt++;
    }

    return rv;
}

/* ROBO LED solution about LED mode :
 *  
 * - Can be retrieved from led_mode_map_0 and led_mode_map_1 
 *   at specific port bit. The combined value is the led mode.
 *  >> Mode[1:0]
 *  >> b00 : Off
 *  >> b01 : On
 *  >> b10 : Blink
 *  >> b11 : Auto
 */
#define ROBO_LED_MODE_RETRIEVE(_mode0, _mode1)   \
        (((_mode1) == 0) ? \
            (((_mode0) == 0) ? DRV_LED_MODE_OFF : DRV_LED_MODE_ON) : \
            (((_mode0) == 0) ? DRV_LED_MODE_BLINK : DRV_LED_MODE_AUTO))

#define ROBO_LED_MODE_RESOLVE(mode, _mode0, _mode1)   \
        if((mode) == DRV_LED_MODE_OFF) {\
            (_mode1) = 0;   \
            (_mode0) = 0;   \
        } else if ((mode) == DRV_LED_MODE_ON){  \
            (_mode1) = 0;   \
            (_mode0) = 1;   \
        } else if ((mode) == DRV_LED_MODE_BLINK){   \
            (_mode1) = 1;   \
            (_mode0) = 0;   \
        } else if ((mode) == DRV_LED_MODE_AUTO){    \
            (_mode1) = 1;   \
            (_mode0) = 1;   \
        } else {    \
            (_mode1) = -1;  \
            (_mode0) = -1;  \
        }
/*
 * Function: 
 *	    drv_led_mode_get
 * Purpose:
 *	    Get the port based LED working mode.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_mode    - (OUT) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 *  2. This routine is used to serve the related register read/write in 
 *      32 bits length boundery only.
 */
int
drv_led_mode_get(int unit,int port, uint32 *led_mode)
{
    uint32  mode32_val0 = 0, mode32_val1 = 0;
    uint32  fld32_val = 0;
    int     temp0, temp1;

    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || 
            SOC_IS_ROBO5389(unit)){
        return SOC_E_UNAVAIL;
    }

    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        soc_cm_debug(DK_WARN, 
                "%s, Improper driver service been proceeded\n", 
                FUNCTION_NAME());
        return SOC_E_INTERNAL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_LED_MODE_MAP_0r(unit, &mode32_val0));
    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) || 
            SOC_IS_ROBO53128(unit)){
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAP0f, &fld32_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAPf, &fld32_val));
    }
    temp0 = (fld32_val & (0x1 << port)) ? 1 : 0;

    SOC_IF_ERROR_RETURN(REG_READ_LED_MODE_MAP_1r(unit, &mode32_val1));
    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) || 
            SOC_IS_ROBO53128(unit)){
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAP1f, &fld32_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAPf, &fld32_val));
    }
    temp1 = (fld32_val & (0x1 << port)) ? 1 : 0;

    *led_mode = ROBO_LED_MODE_RETRIEVE(temp0, temp1);

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_mode_set
 * Purpose:
 *	    Set the port based LED working mode.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_mode    - (OUT) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 *  2. This routine is used to serve the related register read/write in 
 *      32 bits length boundery only.
 */
int
drv_led_mode_set(int unit,int port, uint32 led_mode)
{
    uint32  mode32_val0 = 0, mode32_val1 = 0;
    uint32  fld32_val = 0;
    int     temp0, temp1;

    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || 
            SOC_IS_ROBO5389(unit)){
        return SOC_E_UNAVAIL;
    }

    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        soc_cm_debug(DK_WARN, 
                "%s, Improper driver service been proceeded\n", 
                FUNCTION_NAME());
        return SOC_E_INTERNAL;
    }

    ROBO_LED_MODE_RESOLVE(led_mode, temp0, temp1);
    if (temp0 == -1 && temp1 == -1 ) {
        return SOC_E_PARAM;
    } else {
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_MODE_MAP_0r(unit, &mode32_val0));
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_MODE_MAP_1r(unit, &mode32_val1));
    }
    
    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) || 
            SOC_IS_ROBO53128(unit)){
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAP0f, &fld32_val));
        if (temp0 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_set
            (unit, &mode32_val0, LED_MODE_MAP0f, &fld32_val));

        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAP1f, &fld32_val));
        if (temp1 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_set
            (unit, &mode32_val1, LED_MODE_MAP1f, &fld32_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
            (unit, &mode32_val0, LED_MODE_MAPf, &fld32_val));
        if (temp0 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_set
            (unit, &mode32_val0, LED_MODE_MAPf, &fld32_val));
        
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
            (unit, &mode32_val1, LED_MODE_MAPf, &fld32_val));
        if (temp1 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_set
            (unit, &mode32_val1, LED_MODE_MAPf, &fld32_val));
    }

    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_MODE_MAP_0r(unit, &mode32_val0));
    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_MODE_MAP_1r(unit, &mode32_val1));
    
    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_funcgrp_select_get
 * Purpose:
 *	    Get the port based selctions of working LED function group.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_group   - (OUT) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 *  2. This routine is used to serve the related register read/write in 
 *      32 bits length boundery only.
 */
int
drv_led_funcgrp_select_get(int unit,int port, int *led_group)
{
    uint32  reg32_val = 0, fld32_val = 0;
    
    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || 
            SOC_IS_ROBO5389(unit)){
        return SOC_E_UNAVAIL;
    }

    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        soc_cm_debug(DK_WARN, 
                "%s, Improper driver service been proceeded\n", 
                FUNCTION_NAME());
        return SOC_E_INTERNAL;
    }

    SOC_IF_ERROR_RETURN(
            REG_READ_LED_FUNC_MAPr(unit, &reg32_val));
    SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
        (unit, &reg32_val, LED_FUNC_MAPf, &fld32_val));
    *led_group = (fld32_val & (0x1 << port)) ? 
            DRV_LED_FUNCGRP_1 : DRV_LED_FUNCGRP_0;

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_funcgrp_select_set
 * Purpose:
 *	    Set the port based selctions of working LED function group.
 * Parameters:
 *	    port        - (IN) local port id
 *      led_group   - (IN) led function group id
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 */
int
drv_led_funcgrp_select_set(int unit,int port, int led_group)
{
    int     ledgrp = 0;
    uint32  reg32_val = 0, fld32_val = 0;
    
    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || 
            SOC_IS_ROBO5389(unit)){
        return SOC_E_UNAVAIL;
    }

    /* check if the processing register length is not suitable in this 
     * routine to prevent improper register read/write been proceeded.
     */
    if (DRV_REG_LENGTH_GET(unit, INDEX(LED_FUNC_MAPr)) > 4){
        soc_cm_debug(DK_WARN, 
                "%s, Improper driver service been proceeded\n", 
                FUNCTION_NAME());
        return SOC_E_INTERNAL;
    }

    ledgrp = (led_group == DRV_LED_FUNCGRP_0) ? 0 : 
            (led_group == DRV_LED_FUNCGRP_1) ? 1 : -1;
    if (ledgrp == -1){
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(
            REG_READ_LED_FUNC_MAPr(unit, &reg32_val));
    SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
            (unit, &reg32_val, LED_FUNC_MAPf, &fld32_val));
    if (ledgrp == 0){
        fld32_val &= ~(0x1 << port);
    } else {
        fld32_val |= 0x1 << port;
    }
    SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_set
            (unit, &reg32_val, LED_FUNC_MAPf, &fld32_val));
    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_FUNC_MAPr(unit, &reg32_val));

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_func_get
 * Purpose:
 *	    Get the combined LED functions in the specific LED function group.
 * Parameters:
 *	    led_group     - (IN) led group id
 *      led_functions - (OUT) bitmap format. 
 *                          Each bit indicated a single LED function.
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 */
int
drv_led_func_get(int unit,int led_group, uint32 *led_functions)
{

    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || 
            SOC_IS_ROBO5389(unit)){
        return SOC_E_UNAVAIL;
    }

    *led_functions = 0;
    if (led_group == DRV_LED_FUNCGRP_0){
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_FUNC0_CTLr(unit, led_functions));        
    } else if (led_group == DRV_LED_FUNCGRP_1){
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_FUNC1_CTLr(unit, led_functions));        
    } else {
        return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_led_func_set
 * Purpose:
 *	    Set the combined LED functions in the specific LED function group.
 * Parameters:
 *	    led_group     - (IN) led group id
 *      led_functions - (OUT) bitmap format. 
 *                          Each bit indicated a single LED function.
 * 
 * Note:
 *	1. Most of the ROBO chips support the same LED_FUNCITON group architecture
 *      - 2 LED funciton groups.
 *      - bcm5324, bcm5396 and bcm5389 doesn't support this feature.
 */
int
drv_led_func_set(int unit,int led_group, uint32 led_functions)
{
    uint32  supported_led_functions = 0;
    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5396(unit) || 
            SOC_IS_ROBO5389(unit)){
        return SOC_E_UNAVAIL;
    } 

    /* check if the assigning led functions is out of supporting list */
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS, &supported_led_functions));
    if (led_functions & ~(supported_led_functions)){
        return SOC_E_PARAM;
    }
    
    if (led_group == DRV_LED_FUNCGRP_0){
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC0_CTLr(unit, &led_functions));
    } else if (led_group == DRV_LED_FUNCGRP_1){
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC1_CTLr(unit, &led_functions));
    } else {
        return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_bcm5348_bcm53242_led_mode_get
 * Purpose:
 *	    Get the port based LED working mode.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_mode    - (OUT) led function group id
 * 
 * Note:
 *	1. this routine is designed for 64bits length process.
 *  2. suitable ROBO chips are : bcm5348/ 5347/ 53242/ 53262
 */
int
drv_bcm5348_bcm53242_led_mode_get(int unit, 
                int port, uint32 *led_mode)
{
    uint64  mode64_val0, mode64_val1, fld64_val;
    uint32  fld32_val = 0;
    int     temp0, temp1;

    if (!(SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) || 
            SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
        /* this routine is designed for bcm5348/5347/53242/53262 only */
        return SOC_E_INTERNAL;
    }
    
    SOC_IF_ERROR_RETURN(
            REG_READ_LED_MODE_MAP_0r(unit, (uint32 *)&mode64_val0));
    SOC_IF_ERROR_RETURN(
            REG_READ_LED_MODE_MAP_1r(unit, (uint32 *)&mode64_val1));
    
    if (SOC_IS_ROBO5348(unit)){
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
                (unit,  (uint32 *)&mode64_val0, LED_MODE_MAPf, 
                (uint32 *)&fld64_val));
        temp0 = (COMPILER_64_BITTEST(fld64_val, port)) ? 1 : 0;
        
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
                (unit, (uint32 *)&mode64_val1, LED_MODE_MAPf, 
                (uint32 *)&fld64_val));
        temp1 = (COMPILER_64_BITTEST(fld64_val, port)) ? 1 : 0;
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
                (unit, (uint32 *)&mode64_val0, LED_MODE_MAPf, 
                &fld32_val));
        temp0 = (fld32_val & (0x1 << port)) ? 1 : 0;
        
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
                (unit,  (uint32 *)&mode64_val1, LED_MODE_MAPf, 
                &fld32_val));
        temp1 = (fld32_val & (0x1 << port)) ? 1 : 0;
    }
    
    *led_mode = ROBO_LED_MODE_RETRIEVE(temp0, temp1);
    
    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_bcm5348_bcm53242_led_mode_set
 * Purpose:
 *	    Set the port based LED working mode.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_mode    - (OUT) led function group id
 * 
 * Note:
 *	1. this routine is designed for 64bits length process.
 *  2. suitable ROBO chips are : bcm5348/ 5347/ 53242/ 53262
 */
int
drv_bcm5348_bcm53242_led_mode_set(int unit,
                int port, uint32 led_mode)
{
    uint64  mode64_val0, mode64_val1;
    uint64  fld64_val, temp64;
    uint32  fld32_val = 0, temp_h = 0, temp_l = 0;
    int     temp0, temp1;

    if (!(SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) || 
            SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
        /* this routine is designed for bcm5348/5347/53242/53262 only */
        return SOC_E_INTERNAL;
    }

    ROBO_LED_MODE_RESOLVE(led_mode, temp0, temp1);
    if (temp0 == -1 && temp1 == -1 ) {
        return SOC_E_PARAM;
    } else {
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_MODE_MAP_0r(unit, (uint32 *)&mode64_val0));
        SOC_IF_ERROR_RETURN(
                REG_READ_LED_MODE_MAP_1r(unit, (uint32 *)&mode64_val1));
    }

    /* set the target port bit in the uint64 bitmap format */
    temp_h = 0;
    temp_l = 1;
    COMPILER_64_SET(temp64, temp_h, temp_l);
    COMPILER_64_SHL(temp64, port);
        
    if (SOC_IS_ROBO5348(unit)){
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
                (unit, (uint32 *)&mode64_val0, LED_MODE_MAPf, 
                (uint32 *)&fld64_val));
        if (temp0 == 0){
            COMPILER_64_NOT(temp64);
            COMPILER_64_AND(fld64_val, temp64);
            COMPILER_64_NOT(temp64); /* recover to original temp64 */
        } else {
            COMPILER_64_OR(fld64_val, temp64);
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_set
                (unit, (uint32 *)&mode64_val0, LED_MODE_MAPf, 
                (uint32 *)&fld64_val));

        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
                (unit, (uint32 *)&mode64_val1, LED_MODE_MAPf, 
                (uint32 *)&fld64_val));
        if (temp1 == 0){
            COMPILER_64_NOT(temp64);
            COMPILER_64_AND(fld64_val, temp64);
            COMPILER_64_NOT(temp64); /* recover to original temp64 */
        } else {
            COMPILER_64_OR(fld64_val, temp64);
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_set
                (unit, (uint32 *)&mode64_val1, LED_MODE_MAPf, 
                (uint32 *)&fld64_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_get
                (unit, (uint32 *)&mode64_val0, LED_MODE_MAPf, 
                &fld32_val));
        if (temp0 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_0r_field_set
                (unit, (uint32 *)&mode64_val0, LED_MODE_MAPf, 
                &fld32_val));
        
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_get
                (unit, (uint32 *)&mode64_val1, LED_MODE_MAPf, 
                &fld32_val));
        if (temp1 == 0){
            fld32_val &= ~(0x1 << port); 
        } else {
            fld32_val |= (0x1 << port); 
        }
        SOC_IF_ERROR_RETURN(soc_LED_MODE_MAP_1r_field_set
                (unit, (uint32 *)&mode64_val1, LED_MODE_MAPf, 
                &fld32_val));
    }

    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_MODE_MAP_0r(unit, (uint32 *)&mode64_val0));
    SOC_IF_ERROR_RETURN(
            REG_WRITE_LED_MODE_MAP_1r(unit, (uint32 *)&mode64_val1));
    
    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_bcm5348_bcm53242_led_funcgrp_select_get
 * Purpose:
 *	    Get the port based selctions of working LED function group.
 * Parameters:
 *	    port        - (IN) local port id.
 *      led_group   - (OUT) led function group id
 * 
 * Note:
 *	1. this routine is designed for 64bits length process.
 *  2. suitable ROBO chips are : bcm5348/ 5347/ 53242/ 53262
 */
int
drv_bcm5348_bcm53242_led_funcgrp_select_get(int unit,
                int port, int *led_group)
{
    uint32  fld32_val = 0;
    uint64  reg64_val, fld64_val;
    
    if (!(SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) || 
            SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
        /* this routine is designed for bcm5348/5347/53242/53262 only */
        return SOC_E_INTERNAL;
    }

    SOC_IF_ERROR_RETURN(
            REG_READ_LED_FUNC_MAPr(unit, (uint32 *)&reg64_val));

    if (SOC_IS_ROBO5348(unit)){
        SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
                (unit, (uint32 *)&reg64_val, LED_FUNC_MAPf, 
                (uint32 *)&fld64_val));
        *led_group = COMPILER_64_BITTEST(fld64_val, port) ? 
                DRV_LED_FUNCGRP_1 : DRV_LED_FUNCGRP_0;
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
                (unit, (uint32 *)&reg64_val, LED_FUNC_MAPf, 
                 (uint32 *)&fld32_val));
        *led_group = (fld32_val & (0x1 << port)) ? 
                DRV_LED_FUNCGRP_1 : DRV_LED_FUNCGRP_0;
    }

    return SOC_E_NONE;
}

/*
 * Function: 
 *	    drv_bcm5348_bcm53242_led_funcgrp_select_set
 * Purpose:
 *	    Set the port based selctions of working LED function group.
 * Parameters:
 *	    port        - (IN) local port id
 *      led_group   - (IN) led function group id
 * 
 * Note:
 *	1. this routine is designed for 64bits length process.
 *  2. suitable ROBO chips are : bcm5348/ 5347/ 53242/ 53262
 */
int
drv_bcm5348_bcm53242_led_funcgrp_select_set(int unit, 
                int port, int led_group)
{
    uint64  reg64_val, fld64_val, temp64;
    uint32  fld32_val = 0, temp_h = 0, temp_l = 0;
    int     ledgrp = 0;
    
    if (!(SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) || 
            SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit))){
        /* this routine is designed for bcm5348/5347/53242/53262 only */
        return SOC_E_INTERNAL;
    }

    ledgrp = (led_group == DRV_LED_FUNCGRP_0) ? 0 : 
            (led_group == DRV_LED_FUNCGRP_1) ? 1 : -1;
    if (ledgrp == -1){
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(
            REG_READ_LED_FUNC_MAPr(unit, (uint32 *)&reg64_val));
    
    if (SOC_IS_ROBO5348(unit)){
        temp_h = 0;
        temp_l = 1;
        COMPILER_64_SET(temp64, temp_h, temp_l);
        COMPILER_64_SHL(temp64, port);
        
        SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
                (unit, (uint32 *)&reg64_val, LED_FUNC_MAPf, 
                (uint32 *)&fld64_val));
        if (ledgrp == 0) {
            COMPILER_64_NOT(temp64);
            COMPILER_64_AND(fld64_val, temp64);
            COMPILER_64_NOT(temp64); /* recover to original temp64 */
        } else {
            COMPILER_64_OR(fld64_val, temp64);
        }
        SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_set
                (unit, (uint32 *)&reg64_val, LED_FUNC_MAPf, 
                (uint32 *)&fld64_val));
    } else {
        SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_get
                (unit, (uint32 *)&reg64_val, LED_FUNC_MAPf, 
                 (uint32 *)&fld32_val));
        if (ledgrp == 0){
            fld32_val &= ~(0x1 << port);
        } else {
            fld32_val |= 0x1 << port;
        }
        SOC_IF_ERROR_RETURN(soc_LED_FUNC_MAPr_field_set
                (unit, (uint32 *)&reg64_val, LED_FUNC_MAPf, 
                 (uint32 *)&fld32_val));
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC_MAPr
            (unit, (uint32 *)&reg64_val));

    return SOC_E_NONE;
}

#endif  /* BCM_ROBO_SUPPORT */
