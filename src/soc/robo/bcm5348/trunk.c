/*
 * $Id: trunk.c 1.3 Broadcom SDK $
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
#include "bcm5348/robo_5348.h"

#define 	TRUNK_HASH_FIELD_MACDA_VALUE		2
#define 	TRUNK_HASH_FIELD_MACSA_VALUE		1
#define 	TRUNK_HASH_FIELD_MACDASA_VALUE		0


static uint32   default_trunk_seed  = TRUNK_HASH_FIELD_MACDASA_VALUE;
/*
 *  Function : _drv_bcm5348_trunk_enable_set
 *
 *  Purpose :
 *      Enable trunk function(global).
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *       tid          :   trunk id. If tid==-1, globle trunk enable/disable
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
_drv_bcm5348_trunk_enable_set(int unit, int tid, uint32 enable)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;
    uint64  reg_value64;

    if (tid == -1){
         /* Enable LOCAL TRUNK */ 
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
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (enable) {
            temp = 1;
        } else {
            temp = 0;
        }

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, TRUNK_GRP_CTLr, (uint32 *)&reg_value64, EN_TRUNK_GRPf, &temp));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

   }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_bcm5348_trunk_enable_get
 *
 *  Purpose :
 *      Get the enable status of trunk function(global).
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      tid   :   trunk id. If tid==-1, globle trunk enable/disable
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
_drv_bcm5348_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;
    uint64  reg_value64;

    if(tid ==-1) { /* get LOCAL TRUNK*/
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, GLOBAL_TRUNK_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, GLOBAL_TRUNK_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, GLOBAL_TRUNK_CTLr, &reg_value, EN_TRUNK_LOCALf, &temp));
        if (temp) {
            *enable = TRUE;
        } else {
            *enable = FALSE;
        }
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, TRUNK_GRP_CTLr, (uint32 *)&reg_value64, EN_TRUNK_GRPf, &temp));
        if (temp) {
            *enable = TRUE;
        } else {
            *enable = FALSE;
        }
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
drv_bcm5348_trunk_set(int unit, int tid, soc_pbmp_t bmp, uint32 flag, uint32 hash_op)
{
    uint32  reg_addr, enable, temp;
    uint64  reg_value64, temp64;
    int		reg_len;

    soc_cm_debug(DK_PORT, 
    "drv_trunk_set: unit = %d, trunk id = %d, bmp = %x %x, flag = %x\n",
        unit, tid, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1), flag);
    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (hash_op & DRV_TRUNK_HASH_FIELD_MACDA) {
            if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                default_trunk_seed = TRUNK_HASH_FIELD_MACDASA_VALUE;
            } else {
                default_trunk_seed = TRUNK_HASH_FIELD_MACDA_VALUE;
            }
        } else if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
            default_trunk_seed = TRUNK_HASH_FIELD_MACSA_VALUE;
        }
        return SOC_E_NONE;
    }

    if (tid > (DRV_TRUNK_GROUP_NUM - 1) ){
        return SOC_E_PARAM;
    }        

    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        _drv_bcm5348_trunk_enable_get(unit, -1, &enable);
        if (!enable) {
            _drv_bcm5348_trunk_enable_set(unit, -1, TRUE);
        }    
        _drv_bcm5348_trunk_enable_set(unit, tid, TRUE);
    } else if (flag & DRV_TRUNK_FLAG_DISABLE){
        _drv_bcm5348_trunk_enable_set(unit, tid, FALSE);
    }

    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

        if (SOC_IS_ROBO5348(unit)) {
            soc_robo_64_pbmp_to_val(unit, &bmp, &temp64);

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, TRUNK_GRP_CTLr, (uint32 *)&reg_value64, 
                TRUNK_PORT_MAPf, (uint32 *)&temp64));
        } else { /* 5347 */
            temp = SOC_PBMP_WORD_GET(bmp, 0);
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, TRUNK_GRP_CTLr, (uint32 *)&reg_value64, 
                TRUNK_PORT_MAPf, &temp));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
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
drv_bcm5348_trunk_get(int unit, int tid, soc_pbmp_t *bmp, uint32 flag, uint32 *hash_op)
{
    uint32      reg_addr, enable, temp;
    int         reg_len;
    uint64      reg_value64, temp64;

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        switch (default_trunk_seed) {
            case TRUNK_HASH_FIELD_MACDASA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA | DRV_TRUNK_HASH_FIELD_MACSA;
                break;
            case TRUNK_HASH_FIELD_MACDA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA;
                break;
            case TRUNK_HASH_FIELD_MACSA_VALUE:
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
    
    if (tid > (DRV_TRUNK_GROUP_NUM - 1) ){
        return SOC_E_PARAM;
    }
    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        SOC_IF_ERROR_RETURN(
                _drv_bcm5348_trunk_enable_get(unit, -1, &enable));
        if (enable) {
            SOC_IF_ERROR_RETURN(
                _drv_bcm5348_trunk_enable_get(unit, tid, &enable));            
        }
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }

    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

        if (SOC_IS_ROBO5348(unit)) {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, TRUNK_GRP_CTLr, (uint32 *)&reg_value64, 
                TRUNK_PORT_MAPf, (uint32 *)&temp64));
            soc_robo_64_val_to_pbmp(unit, bmp, temp64);
        } else { /* 5347 */
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, TRUNK_GRP_CTLr, (uint32 *)&reg_value64, 
                TRUNK_PORT_MAPf, &temp));
            SOC_PBMP_WORD_SET(*bmp, 0, temp);
        }
    }

    soc_cm_debug(DK_PORT, 
        "drv_trunk_get: unit = %d, trunk id = %d, value = 0x%x 0x%x, flag = %x\n",
        unit, tid, SOC_PBMP_WORD_GET(*bmp, 0), SOC_PBMP_WORD_GET(*bmp, 1), flag);

    return SOC_E_NONE;
}
