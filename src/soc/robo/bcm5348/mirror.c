/*
 * $Id: mirror.c 1.4 Broadcom SDK $
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
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_mirror_set
 *
 *  Purpose :
 *      Set ingress and egress ports of mirroring
 *
 *  Parameters :
 *      unit        :   unit id
 *      enable   :   enable/disable.
 *      mport_bmp     :   monitor port bitmap.
 *      ingress_bmp     :   ingress port bitmap.
 *      egress_bmp      :   egress port bitmap
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5348_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp)
{
    uint32      reg_addr, reg_len;
    uint64      reg_value, temp;
    int         rv= SOC_E_NONE;
    uint32      temp32;

    soc_cm_debug(DK_PORT, 
        "drv_mirror_set : unit %d, %sable, mport_bmp = 0x%x 0x%x, \
        ingress_bmp = 0x%x 0x%x, egress_bmp = 0x%x 0x%x\n",
        unit, enable ? "en" : "dis", \
        SOC_PBMP_WORD_GET(mport_bmp, 0), SOC_PBMP_WORD_GET(mport_bmp, 1),\
        SOC_PBMP_WORD_GET(ingress_bmp, 0), SOC_PBMP_WORD_GET(ingress_bmp, 1),\
        SOC_PBMP_WORD_GET(egress_bmp, 0), SOC_PBMP_WORD_GET(egress_bmp, 1));

    /* check ingress mirror */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, IGMIRCTLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, IGMIRCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }
    /* write ingress mirror mask */
    if (SOC_INFO(unit).port_num > 32) {
        soc_robo_64_pbmp_to_val(unit, &ingress_bmp, &temp);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, IGMIRCTLr, (uint32 *)&reg_value, IN_MIR_MSKf, (uint32 *)&temp));
    } else {
        temp32 = SOC_PBMP_WORD_GET(ingress_bmp, 0);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, IGMIRCTLr, (uint32 *)&reg_value, IN_MIR_MSKf, (uint32 *)&temp32));
    }
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }
    /* check egress mirror */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, EGMIRCTLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, EGMIRCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }
    /* write egress mirror mask */
    if (SOC_INFO(unit).port_num > 32) {
        soc_robo_64_pbmp_to_val(unit, &egress_bmp, &temp);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, EGMIRCTLr, (uint32 *)&reg_value, OUT_MIR_MSKf, (uint32 *)&temp));
    } else {
        temp32 = SOC_PBMP_WORD_GET(egress_bmp, 0);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, EGMIRCTLr, (uint32 *)&reg_value, OUT_MIR_MSKf, (uint32 *)&temp32));
    }
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }

    /* check mirror control */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MIRCAPCTLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MIRCAPCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }

    if (enable) { /* enable mirror */
        temp32 = 1;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, MIRCAPCTLr, (uint32 *)&reg_value, MIR_ENf, &temp32));
    } else { /* disable mirror */
         temp32 = 0;
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, MIRCAPCTLr, (uint32 *)&reg_value, MIR_ENf, &temp32));

        /* clear mirror-to ports mask when disable mirror */
        SOC_PBMP_CLEAR(mport_bmp);
    }

    /* write mirror-to ports mask */
    if (SOC_INFO(unit).port_num > 32) {
        soc_robo_64_pbmp_to_val(unit, &mport_bmp, &temp);    
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, MIRCAPCTLr, (uint32 *)&reg_value, SMIR_CAP_PORTf, (uint32 *)&temp));
    } else {
        temp32 = SOC_PBMP_WORD_GET(mport_bmp, 0);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_set)
            (unit, MIRCAPCTLr, (uint32 *)&reg_value, SMIR_CAP_PORTf, (uint32 *)&temp32));
    }

    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_value,reg_len));

    return rv;
}

/*
 *  Function : drv_mirror_get
 *
 *  Purpose :
 *      Get ingress and egress ports of mirroring
 *
 *  Parameters :
 *      unit        :   unit id
 *      enable   :   enable/disable.
 *      mport_bmp     :   monitor port id.
 *      ingress_bmp     :   ingress port bitmap.
 *      egress_bmp      :   egress port bitmap
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5348_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp)
{
    uint32      reg_addr, reg_len;
    uint64      reg_value, temp;
    int         rv= SOC_E_NONE;
    uint32      temp32;

    /* ingress mask */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, IGMIRCTLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, IGMIRCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }
    if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)
            (unit, IGMIRCTLr, (uint32 *)&reg_value, IN_MIR_MSKf, (uint32 *)&temp));
        soc_robo_64_val_to_pbmp(unit, ingress_bmp, temp);
    } else {
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)
            (unit, IGMIRCTLr, (uint32 *)&reg_value, IN_MIR_MSKf, (uint32 *)&temp32));
        SOC_PBMP_WORD_SET(*ingress_bmp, 0, temp32);
    }

    /* egress mask */
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, EGMIRCTLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, EGMIRCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }
    if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)
            (unit, EGMIRCTLr, (uint32 *)&reg_value, OUT_MIR_MSKf, (uint32 *)&temp));
        soc_robo_64_val_to_pbmp(unit, egress_bmp, temp);
    } else {
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_field_get)
            (unit, EGMIRCTLr, (uint32 *)&reg_value, OUT_MIR_MSKf, (uint32 *)&temp32));
        SOC_PBMP_WORD_SET(*egress_bmp, 0, temp32);
    }
        
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, MIRCAPCTLr);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, MIRCAPCTLr, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_value, reg_len)) < 0) {
        return rv;
    }
    /* enable value */
    temp32 = 0;
    SOC_IF_ERROR_RETURN(
        (DRV_SERVICES(unit)->reg_field_get)
        (unit, MIRCAPCTLr, (uint32 *)&reg_value, MIR_ENf, &temp32));
    if (temp32) {
        /* enable value */
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    /* monitor port */
    SOC_PBMP_CLEAR(*mport_bmp);
    if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, MIRCAPCTLr, (uint32 *)&reg_value, SMIR_CAP_PORTf, (uint32 *)&temp));
        soc_robo_64_val_to_pbmp(unit, mport_bmp, temp);
    } else {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, MIRCAPCTLr, (uint32 *)&reg_value, SMIR_CAP_PORTf, (uint32 *)&temp32));
        SOC_PBMP_WORD_SET(*mport_bmp, 0, temp32);
    }

    soc_cm_debug(DK_PORT, 
        "drv_mirror_get : unit %d, %sable, mport_bmp = 0x%x 0x%x, \
        ingress_bmp = 0x%x 0x%x, egress_bmp = 0x%x 0x%x\n",
        unit, *enable ? "en" : "dis", \
        SOC_PBMP_WORD_GET(*mport_bmp, 0), SOC_PBMP_WORD_GET(*mport_bmp, 1), \
        SOC_PBMP_WORD_GET(*ingress_bmp, 0), SOC_PBMP_WORD_GET(*ingress_bmp, 1), \
        SOC_PBMP_WORD_GET(*egress_bmp, 0), SOC_PBMP_WORD_GET(*egress_bmp, 1));
    return rv;
}
