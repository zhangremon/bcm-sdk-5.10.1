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
drv_bcm53280_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp)
{
    uint32  reg_value, temp;
    int  rv= SOC_E_NONE;

    soc_cm_debug(DK_PORT, "drv_mirror_set : unit %d, %sable, mport_bmp = 0x%x, \
        ingress_bmp = 0x%x, egress_bmp = 0x%x\n",
        unit, enable ? "en" : "dis", \
        SOC_PBMP_WORD_GET(mport_bmp, 0), \
        SOC_PBMP_WORD_GET(ingress_bmp, 0), \
        SOC_PBMP_WORD_GET(egress_bmp, 0));

    /* check ingress mirror */
    SOC_IF_ERROR_RETURN(REG_READ_IGMIRCTLr
        (unit, &reg_value));
    /* write ingress mirror mask */
    temp = SOC_PBMP_WORD_GET(ingress_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_IGMIRCTLr_field_set
        (unit, &reg_value, IN_MIR_MSKf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_IGMIRCTLr
        (unit, &reg_value));

    /* check egress mirror */
    SOC_IF_ERROR_RETURN(REG_READ_EGMIRCTLr
        (unit, &reg_value));
    /* write egress mirror mask */
    temp = SOC_PBMP_WORD_GET(egress_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_EGMIRCTLr_field_set
        (unit, &reg_value, OUT_MIR_MSKf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_EGMIRCTLr
        (unit, &reg_value));
        
    if (enable) { /* enable mirror */
        /* check mirror control */
        SOC_IF_ERROR_RETURN(REG_READ_MIRRORCTLr
            (unit, &reg_value));
        /* Enable both ingress and egress mirroring */
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_set
            (unit, &reg_value, EN_IN_MIR_FLTRf, &temp));
        SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_set
            (unit, &reg_value, EN_OUT_MIR_FLTRf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_MIRRORCTLr
            (unit, &reg_value));
    } else { /* disable mirror */
        SOC_IF_ERROR_RETURN(REG_READ_MIRRORCTLr
            (unit, &reg_value));
        /* Disable both ingress and egress mirroring */
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_set
            (unit, &reg_value, EN_IN_MIR_FLTRf, &temp));
        SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_set
            (unit, &reg_value, EN_OUT_MIR_FLTRf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_MIRRORCTLr
            (unit, &reg_value));

        /* clear mirror-to ports mask when disable mirror */
        SOC_PBMP_CLEAR(mport_bmp);
    }

    /* write mirror-to ports mask */
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, &reg_value));
    temp = SOC_PBMP_WORD_GET(mport_bmp, 0);
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_set
        (unit, &reg_value, MIR_CAP_PORTf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MIRCAPCTLr
        (unit, &reg_value));

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
drv_bcm53280_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp)
{
    uint32  reg_value, temp;
    int  rv= SOC_E_NONE;

    /* ingress mask */
    SOC_IF_ERROR_RETURN(REG_READ_IGMIRCTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_IGMIRCTLr_field_get
        (unit, &reg_value, IN_MIR_MSKf, &temp));
    SOC_PBMP_WORD_SET(*ingress_bmp, 0, temp);

    /* egress mask */
    SOC_IF_ERROR_RETURN(REG_READ_EGMIRCTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_EGMIRCTLr_field_get
        (unit, &reg_value, OUT_MIR_MSKf, &temp));
    SOC_PBMP_WORD_SET(*egress_bmp, 0, temp);

    SOC_IF_ERROR_RETURN(REG_READ_MIRRORCTLr
        (unit, &reg_value));
    /* enable value */
    temp = 0;
    SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_get
        (unit, &reg_value, EN_IN_MIR_FLTRf, &temp));
    if (!temp) {
        SOC_IF_ERROR_RETURN(soc_MIRRORCTLr_field_get
            (unit, &reg_value, EN_OUT_MIR_FLTRf, &temp));
    }

    if (temp) {
        /* enable value */
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    /* monitor port */
    SOC_PBMP_CLEAR(*mport_bmp);
    SOC_IF_ERROR_RETURN(REG_READ_MIRCAPCTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_MIRCAPCTLr_field_get
        (unit, &reg_value, MIR_CAP_PORTf, &temp));
    SOC_PBMP_WORD_SET(*mport_bmp, 0, temp);

    soc_cm_debug(DK_PORT, "drv_mirror_get : unit %d, %sable, mport_bmp = 0x%x, \
        ingress_bmp = 0x%x, egress_bmp = 0x%x\n",
        unit, *enable ? "en" : "dis", \
        SOC_PBMP_WORD_GET(*mport_bmp, 0), \
        SOC_PBMP_WORD_GET(*ingress_bmp, 0), \
        SOC_PBMP_WORD_GET(*egress_bmp, 0));
    return rv;
}
