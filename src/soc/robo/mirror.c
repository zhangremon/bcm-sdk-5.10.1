/*
 * $Id: mirror.c 1.12.2.2 Broadcom SDK $
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
drv_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp)
{
    uint32	 reg_value, temp, mport = 0;
    int  count = 0;
    int  rv = SOC_E_NONE;

    soc_cm_debug(DK_PORT, "drv_mirror_set : unit %d, %sable, mport_bmp =  0x%x, \
        ingress_bmp = 0x%x, egress_bmp = 0x%x\n",
        unit, enable ? "en" : "dis", \
        SOC_PBMP_WORD_GET(mport_bmp, 0), \
        SOC_PBMP_WORD_GET(ingress_bmp, 0), \
        SOC_PBMP_WORD_GET(egress_bmp, 0));

    /* check mirror capture port : no support multiple mirror-to ports (only one mirror-to port) */
    if (!SOC_IS_ROBO5324(unit)) {
        SOC_PBMP_COUNT(mport_bmp, count);
        if (count >= 2) {
            return SOC_E_UNAVAIL;
        }
    }

    /* check ingress mirror */
    if ((rv = REG_READ_IGMIRCTLr(unit, &reg_value)) < 0) {
        return rv;
    }
    /* write ingress mirror mask */
    temp = SOC_PBMP_WORD_GET(ingress_bmp, 0);
    soc_IGMIRCTLr_field_set(unit, &reg_value, 
        IN_MIR_MSKf, &temp);
    if ((rv = REG_WRITE_IGMIRCTLr(unit, &reg_value)) < 0) {
        return rv;
    }
    /* check egress mirror */
    if ((rv = REG_READ_EGMIRCTLr(unit, &reg_value)) < 0) {
        return rv;
    }
    /* write egress mirror mask */
    temp = SOC_PBMP_WORD_GET(egress_bmp, 0);
    soc_EGMIRCTLr_field_set(unit, &reg_value, 
        OUT_MIR_MSKf, &temp);
    if ((rv = REG_WRITE_EGMIRCTLr(unit, &reg_value)) < 0) {
        return rv;
    }

    /* check mirror control */
    if ((rv = REG_READ_MIRCAPCTLr(unit, &reg_value)) < 0) {
        return rv;
    }

    if (enable) { /* enable mirror */
        temp = 1;
        soc_MIRCAPCTLr_field_set(unit, &reg_value, 
            MIR_ENf, &temp);
    } else { /* disable mirror */
        temp = 0;
        soc_MIRCAPCTLr_field_set(unit, &reg_value, 
            MIR_ENf, &temp);

        /* clear mirror-to ports mask when disable mirror */
        SOC_PBMP_CLEAR(mport_bmp);
    }

     /* write mirror-to ports mask */
    if (SOC_IS_ROBO5324(unit)) {
        temp = SOC_PBMP_WORD_GET(mport_bmp, 0);
    } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5395(unit) ||
               SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
               SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if (SOC_PBMP_IS_NULL(mport_bmp)) {
            mport = 0;
        } else {
            PBMP_ITER(mport_bmp, mport) {
                break;
            }
        }
        temp = mport;
    }
    soc_MIRCAPCTLr_field_set(unit, &reg_value, 
            SMIR_CAP_PORTf, &temp);

    if ((rv = REG_WRITE_MIRCAPCTLr(unit, &reg_value)) < 0) {
        return rv;
    }

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
drv_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp)
{
    uint32	 reg_value, temp = 0;
    int  rv = SOC_E_NONE;

    /* ingress mask */
    if ((rv = REG_READ_IGMIRCTLr(unit, &reg_value)) < 0) {
        return rv;
    }
    soc_IGMIRCTLr_field_get(unit, &reg_value, 
        IN_MIR_MSKf, &temp);
    SOC_PBMP_WORD_SET(*ingress_bmp, 0, temp);

    /* egress mask */
    if ((rv = REG_READ_EGMIRCTLr(unit, &reg_value)) < 0) {
        return rv;
    }
    soc_EGMIRCTLr_field_get(unit, &reg_value, 
        OUT_MIR_MSKf, &temp);
    SOC_PBMP_WORD_SET(*egress_bmp, 0, temp);

    if ((rv = REG_READ_MIRCAPCTLr(unit, &reg_value)) < 0) {
        return rv;
    }
    /* enable value */
    temp = 0;
    soc_MIRCAPCTLr_field_get(unit, &reg_value, 
            MIR_ENf, &temp);
    if (temp) {
        /* enable value */
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    /* monitor port */
    SOC_PBMP_CLEAR(*mport_bmp);
    soc_MIRCAPCTLr_field_get(unit, &reg_value, 
            SMIR_CAP_PORTf, &temp);
    if (SOC_IS_ROBO5324(unit)) {
        SOC_PBMP_WORD_SET(*mport_bmp, 0, temp);
    } else if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5395(unit)||
               SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
               SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        if (*enable) {
            SOC_PBMP_PORT_SET(*mport_bmp, temp);
        } else {
            /* when mirror is disable, temp = 0 return mport_bmp = zero */
            if (temp == 0) {
                SOC_PBMP_CLEAR(*mport_bmp);
            } else {
                SOC_PBMP_PORT_SET(*mport_bmp, temp);
            }
        }
    }

    soc_cm_debug(DK_PORT, "drv_mirror_get : unit %d, %sable, mport_bmp = 0x%x, \
        ingress_bmp = 0x%x, egress_bmp = 0x%x\n",
        unit, *enable ? "en" : "dis", \
        SOC_PBMP_WORD_GET(*mport_bmp, 0), \
        SOC_PBMP_WORD_GET(*ingress_bmp, 0), \
        SOC_PBMP_WORD_GET(*egress_bmp, 0));
    return rv;
}
