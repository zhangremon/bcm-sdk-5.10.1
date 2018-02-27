/*
 * $Id: trunk.c 1.10 Broadcom SDK $
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
#include "robo_53115.h"

#define 	TRUNK53115_HASH_FIELD_MACDA_VALUE  1
#define 	TRUNK53115_HASH_FIELD_MACSA_VALUE  2
#define 	TRUNK53115_HASH_FIELD_MACDASA_VALUE  0

/* trunk seed in bcm53115 is named as trunk hash */
static uint32   default_trunk_seed  = TRUNK53115_HASH_FIELD_MACDASA_VALUE;
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
_drv53115_trunk_enable_set(int unit, uint32 enable)
{
    uint32	reg_value, temp;

    /* Enable LOCAL TRUNK */ /* should move to somewhere to initialize it */

    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));
    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }

    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_set
        (unit, &reg_value, EN_TRUNK_LOCALf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MAC_TRUNK_CTLr
        (unit, &reg_value));

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
_drv53115_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32	reg_value, temp, bmp;

    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));
    temp = 0;
    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_get
        (unit, &reg_value, EN_TRUNK_LOCALf, &temp));

    if (temp){
         SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));
         bmp = 0;
         SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_get
            (unit, &reg_value, EN_TRUNK_GRPf, &bmp));
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
 *      unit    :   RoboSwitch unit number.
 *      tid     :   trunk id.
 *      bmp     :   trunk member port bitmap.
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
drv_bcm53115_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
                uint32 flag, uint32 hash_op)
{
    uint32  reg_value, temp, c_temp;
    int  port;
    uint32  bmp_value = 0, trunk_prop;

    bmp_value = SOC_PBMP_WORD_GET(bmp, 0);
    soc_cm_debug(DK_PORT, 
        "drv_bcm53115_trunk_set: unit = %d, trunk id = %d, bmp = %x, flag = %x\n",
        unit, tid, bmp_value, flag);
    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (hash_op & DRV_TRUNK_HASH_FIELD_MACDA) {
            if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                default_trunk_seed = TRUNK53115_HASH_FIELD_MACDASA_VALUE;
            } else {
                default_trunk_seed = TRUNK53115_HASH_FIELD_MACDA_VALUE;
            }
        } else if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
            default_trunk_seed = TRUNK53115_HASH_FIELD_MACSA_VALUE;
        }
        return SOC_E_NONE;
    }

    /* Check TRUNK MAX ID */
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_prop));
    if (tid > (trunk_prop- 1) ){
        return SOC_E_PARAM;
    }        

    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        _drv53115_trunk_enable_set(unit, TRUE);
    } else if (flag & DRV_TRUNK_FLAG_DISABLE){
        _drv53115_trunk_enable_set(unit, FALSE);
    }

    if (flag & DRV_TRUNK_FLAG_BITMAP) {   
        /* Check per trunk member port number */
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_TRUNK_MAX_PORT_NUM, &trunk_prop));

        /* check pbmp parameter (port count = 0 is allowed) */
        SOC_PBMP_COUNT(bmp, c_temp); 
        if (c_temp) {
            if (c_temp > trunk_prop){
                return SOC_E_PARAM;
            }
        }
        
        /* check valid port bitmap */
        SOC_PBMP_ITER(bmp, port){
            if (!SOC_PORT_VALID(unit, port)){
                return SOC_E_PARAM;
            }
            
            if (IS_CPU_PORT(unit, port)){
                return SOC_E_PARAM;
            }
        }

        temp = bmp_value;
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));

        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_set
            (unit, &reg_value, EN_TRUNK_GRPf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));

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
drv_bcm53115_trunk_get(int unit, int tid, 
                soc_pbmp_t *bmp, uint32 flag, uint32 *hash_op)
{
    uint32  reg_value, temp, enable;
    uint32  trunk_prop;

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        switch (default_trunk_seed) {
            case TRUNK53115_HASH_FIELD_MACDASA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA |  
                        DRV_TRUNK_HASH_FIELD_MACSA;
                break;
            case TRUNK53115_HASH_FIELD_MACDA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA;
                break;
            case TRUNK53115_HASH_FIELD_MACSA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACSA;
                break;
            default:
                return SOC_E_INTERNAL;
        }
        soc_cm_debug(DK_PORT, 
        "drv_bcm53115_trunk_get: unit = %d, trunk id = %d, value = 0x%x, flag = %x\n",
        unit, tid, *hash_op, flag);
        return SOC_E_NONE;
    }
    /* Check TRUNK MAX ID */
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_prop));
    if (tid > (trunk_prop - 1) ){
        return SOC_E_PARAM;
    }
    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        SOC_IF_ERROR_RETURN(
                _drv53115_trunk_enable_get(unit, tid, &enable));
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }

    
    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_get
            (unit, &reg_value, EN_TRUNK_GRPf, &temp));
        SOC_PBMP_WORD_SET(*bmp, 0, temp);
    }
	
    soc_cm_debug(DK_PORT, 
        "drv_bcm53115_trunk_get: unit = %d, trunk id = %d, value = 0x%x, flag = %x\n",
        unit, tid, SOC_PBMP_WORD_GET(*bmp, 0), flag);
    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm53115_trunk_hash_field_add
 *
 *  Purpose :
 *      Add trunk hash field type
 *
 *  Parameters :
 *      unit    :   RoboSwitch unit number.
 *      tid     :   trunk id.
 *      field_type   :   trunk hash field type to be add.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. For the hash type field in bcm53115 allowed 3 hash types only.
 *          (ie. MAC_DASA=0; MAC_DA=1; MAC_SA=2), here the final type value 
 *          to set to register will be :
 *          - no change : when add new hash is the same with current hash.
 *          - MAC_SADA : add to DASA.
 *          - MAC_DA : add DA. (no matter what current type is)
 *          - MAC_SA : add SA. (no matter what current type is)
 *
 */
int 
drv_bcm53115_trunk_hash_field_add(int unit, uint32 field_type)
{
    uint32	reg_value, temp;

    soc_cm_debug(DK_VERBOSE, 
        "drv_bcm53115_trunk_hash_field_add: unit = %d, field type = %d\n",
        unit, field_type);

    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_get
        (unit, &reg_value, HASH_SELf,  &temp));

    if ((field_type & DRV_TRUNK_HASH_FIELD_MACDA) &&  
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)){
        if (temp == TRUNK53115_HASH_FIELD_MACDASA_VALUE){
            return SOC_E_NONE;
        } else {
            temp = TRUNK53115_HASH_FIELD_MACDASA_VALUE;
        }
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACDA){
        if (temp == TRUNK53115_HASH_FIELD_MACDA_VALUE){
            return SOC_E_NONE;
        } else {
            temp = TRUNK53115_HASH_FIELD_MACDA_VALUE;
        }
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA){
        if (temp == TRUNK53115_HASH_FIELD_MACSA_VALUE){
            return SOC_E_NONE;
        } else {
            temp = TRUNK53115_HASH_FIELD_MACSA_VALUE;
        }
    } else {
        soc_cm_debug(DK_WARN, 
            "drv_bcm53115_trunk_hash_field_add:hash type=%d, is invalid!\n",
            field_type);
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_set
        (unit, &reg_value, HASH_SELf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MAC_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_trunk_hash_field_remove
 *
 *  Purpose :
 *      Remove trunk hash field type
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      tid   :   trunk id.
 *      field_type   :   trunk hash field type to be remove.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. For the hash type field in bcm53115 allowed 3 hash types only.
 *          (ie. MAC_DASA=0; MAC_DA=1; MAC_SA=2), here the final type value 
 *          to set to register will be :
 *          - MAC_DA : when remove DA+SA
 *          - MAC_SADA : when current is SA only and remove SA.
 *          - MAC_SADA : when current is DA only and remove DA.
 *          - MAC_SADA : when other hask key applying but no SA or DA.
 *
 */
int 
drv_bcm53115_trunk_hash_field_remove(int unit, uint32 field_type)
{
    uint32	reg_value, temp, current_hash;

    soc_cm_debug(DK_VERBOSE, 
        "drv_bcm53115_trunk_hash_field_remove: unit = %d, field type = %d\n",
        unit, field_type);
    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_get
        (unit, &reg_value, HASH_SELf, &temp));

    /* hash key with 2 bits length but "3" is not a valid value : 
     *  - in such case, the hash key is treat as DASA.
     */
    current_hash = (temp == 3) ? TRUNK53115_HASH_FIELD_MACDASA_VALUE : temp;
    
    if ((field_type & DRV_TRUNK_HASH_FIELD_MACDA) &&  
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)){
        temp = TRUNK53115_HASH_FIELD_MACDA_VALUE;
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACDA){
        if (current_hash == TRUNK53115_HASH_FIELD_MACDA_VALUE){
            temp = TRUNK53115_HASH_FIELD_MACDASA_VALUE;
        }
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA){
        if (current_hash == TRUNK53115_HASH_FIELD_MACSA_VALUE){
            temp = TRUNK53115_HASH_FIELD_MACDASA_VALUE;
        }
    }

    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_set
        (unit,  &reg_value, HASH_SELf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MAC_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}
