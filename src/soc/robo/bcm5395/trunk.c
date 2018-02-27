/*
 * $Id: trunk.c 1.4 Broadcom SDK $
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
#include "robo_5395.h"

#define 	TRUNK5395_HASH_FIELD_MACDA_VALUE		1
#define 	TRUNK5395_HASH_FIELD_MACSA_VALUE		2
#define 	TRUNK5395_HASH_FIELD_MACDASA_VALUE		0
static uint32   default_trunk_seed  = TRUNK5395_HASH_FIELD_MACDASA_VALUE;
/*
 *  Function : _drv_trunk_enable_set
 *
 *  Purpose :
 *      Set the status of the selected FE trunk.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      tid   :   trunk id.
 *      enable   :   status of the trunk id.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
_drv5395_trunk_enable_set(int unit, uint32 enable)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;

    /* Enable LOCAL TRUNK */ /* should move to somewhere to initialize it */

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, GLOBAL_TRUNK_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, GLOBAL_TRUNK_CTLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }

    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, GLOBAL_TRUNK_CTLr, &reg_value, EN_TRUNK_LOCALf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));
    

    return SOC_E_NONE;
}

/*
 *  Function : _drv_trunk_enable_get
 *
 *  Purpose :
 *      Get the status of the selected FE trunk.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      tid   :   trunk id.
 *      enable   :   status of the trunk id.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
_drv5395_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32	reg_addr, reg_value, temp, bmp;
    int		reg_len;


    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, GLOBAL_TRUNK_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)
        (unit, GLOBAL_TRUNK_CTLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));

    temp = 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, GLOBAL_TRUNK_CTLr, &reg_value, EN_TRUNK_LOCALf, &temp));

    if (temp){
         reg_addr = (DRV_SERVICES(unit)->reg_addr)
             (unit, TRUNK_GRP_CTLr, tid, 0);
         reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);
         SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
         bmp = 0;
         SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, TRUNK_GRP_CTLr, &reg_value, TRUNK_PORT_MAPf, &bmp));
	 if(bmp) {
            *enable = 1;
        }else {
            *enable = 0;
        }
    } else {
        *enable = 0;
    }

    return SOC_E_NONE;
}


/*
 *  Function : drv_trunk_set
 *
 *  Purpose :
 *      Set the member ports to a trunk group.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      tid   :   trunk id.
 *      bmp   :   trunk member port bitmap.
 *      flag    :   trunk flag
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5395_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
                                    uint32 flag, uint32 hash_op)
{
    uint32	       reg_addr, reg_value, temp, c_temp;
    int		reg_len;
    uint32  bmp_value = 0, trunk_prop;

    bmp_value = SOC_PBMP_WORD_GET(bmp, 0);
    soc_cm_debug(DK_PORT, 
    "drv_trunk_set: unit = %d, trunk id = %d, bmp = %x, flag = %x\n",
        unit, tid, bmp_value, flag);
    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (hash_op & DRV_TRUNK_HASH_FIELD_MACDA) {
            if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                default_trunk_seed = TRUNK5395_HASH_FIELD_MACDASA_VALUE;
            } else {
                default_trunk_seed = TRUNK5395_HASH_FIELD_MACDA_VALUE;
            }
        } else if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
            default_trunk_seed = TRUNK5395_HASH_FIELD_MACSA_VALUE;
        }
        return SOC_E_NONE;
    }

    /* Check TRUNK MAX ID */
    (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_prop);
    if (tid > (trunk_prop- 1) ){
        return SOC_E_PARAM;
    }        

    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        _drv5395_trunk_enable_set(unit, TRUE);
    } else if (flag & DRV_TRUNK_FLAG_DISABLE){
        _drv5395_trunk_enable_set(unit, FALSE);
    }

    if (flag & DRV_TRUNK_FLAG_BITMAP) {   
        /* Check per trunk member port number */
        c_temp =0;
        (DRV_SERVICES(unit)->dev_prop_get)
            (unit, DRV_DEV_PROP_TRUNK_MAX_PORT_NUM, &trunk_prop);
        if(bmp_value != 0x0) {
            temp = 0x1;
            while(temp < 0x100) {
                if(bmp_value & temp) {
                    c_temp ++;
                }
                if(c_temp > trunk_prop) {
                    return SOC_E_PARAM;
                }
                temp <<= 1;
            }
        }
			
        temp = bmp_value;
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, TRUNK_GRP_CTLr, 0, tid);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, TRUNK_GRP_CTLr);

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
		
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, TRUNK_GRP_CTLr, &reg_value, 
            TRUNK_PORT_MAPf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_trunk_get
 *
 *  Purpose :
 *      Get the member ports to a trunk group.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      tid   :   trunk id.
 *      bmp   :   trunk member port bitmap.
 *      flag    :   trunk flag
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5395_trunk_get(int unit, int tid, 
                 soc_pbmp_t *bmp, uint32 flag, uint32 *hash_op)
{
    uint32      reg_addr, reg_value, temp, enable;
    int           reg_len;
    uint32  trunk_prop;

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        switch (default_trunk_seed) {
            case TRUNK5395_HASH_FIELD_MACDASA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA |  
                    DRV_TRUNK_HASH_FIELD_MACSA;
                break;
            case TRUNK5395_HASH_FIELD_MACDA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA;
                break;
            case TRUNK5395_HASH_FIELD_MACSA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACSA;
                break;
            default:
                return SOC_E_INTERNAL;
        }
        soc_cm_debug(DK_PORT, 
        "drv_trunk_get: unit = %d, trunk id = %d, value = 0x%x, flag = %x\n",
        unit, tid, *hash_op, flag);
        return SOC_E_NONE;
    }
    /* Check TRUNK MAX ID */
    (DRV_SERVICES(unit)->dev_prop_get)
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_prop);
    if (tid > (trunk_prop - 1) ){
        return SOC_E_PARAM;
    }
    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        SOC_IF_ERROR_RETURN(
                _drv5395_trunk_enable_get(unit, tid, &enable));
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }

    
    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, TRUNK_GRP_CTLr, 0, tid);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, TRUNK_GRP_CTLr);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, TRUNK_GRP_CTLr, &reg_value, TRUNK_PORT_MAPf, &temp));
            SOC_PBMP_WORD_SET(*bmp, 0, temp);
    }
	
    soc_cm_debug(DK_PORT, 
        "drv_trunk_get: unit = %d, trunk id = %d, value = 0x%x, flag = %x\n",
        unit, tid, SOC_PBMP_WORD_GET(*bmp, 0), flag);
    return SOC_E_NONE;
}
