/*
 * $Id: trunk.c 1.6 Broadcom SDK $
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
#include "bcm5324/robo_5324.h"

/* chip value  BCM5324 */
#define 	TRUNK_HASH_FIELD_MACDA_VALUE_5324  2
#define 	TRUNK_HASH_FIELD_MACSA_VALUE_5324  1
/* chip value  for others */
#define 	TRUNK_HASH_FIELD_MACDA_VALUE  1
#define 	TRUNK_HASH_FIELD_MACSA_VALUE  2

#define 	TRUNK_HASH_FIELD_MACDASA_VALUE  0

#define TRUNK_HASH_FIELD_VALID_VALUE  (DRV_TRUNK_HASH_FIELD_MACDA | \
                                                                       DRV_TRUNK_HASH_FIELD_MACSA)

static uint32   default_trunk_seed  = TRUNK_HASH_FIELD_MACDASA_VALUE;
/*
 *  Function : _drv_fe_trunk_enable_set
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
_drv_fe_trunk_enable_set(int unit, int tid, uint32 enable)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;
#if 0
    int		reg_len, reg_index, field_index;

    switch (tid) 
    {
        case 0:
            reg_index = TRUNK_GRP0_CTLr;
            field_index = EN_TRUNK_GRP0f;
            break;
        case 1:
            reg_index = TRUNK_GRP1_CTLr;
            field_index = EN_TRUNK_GRP1f;
            break;
        default:
            return SOC_E_PARAM;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    if (enable){
        temp = 1;
    }else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, &reg_value, field_index, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));
#endif

    /* Enable LOCAL TRUNK */ /* should move to somewhere to initialize it */
    if (enable) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, GLOBAL_TRUNK_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, GLOBAL_TRUNK_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, GLOBAL_TRUNK_CTLr, &reg_value, EN_TRUNK_LOCALf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));
    }
    

    return SOC_E_NONE;
}

/*
 *  Function : _drv_fe_trunk_enable_get
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
_drv_fe_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len, reg_index, field_index;

    switch (tid) 
    {
        case 0:
            reg_index = TRUNK_GRP0_CTLr;
            field_index = EN_TRUNK_GRP0f;
            break;
        case 1:
            reg_index = TRUNK_GRP1_CTLr;
            field_index = EN_TRUNK_GRP1f;
            break;
        default:
            return SOC_E_PARAM;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, &reg_value, field_index, &temp));
    if (temp){
        *enable = 1;
    } else {
        *enable = 0;
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_ge_trunk_enable_set
 *
 *  Purpose :
 *      Set the status of the selected GE trunk.
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
_drv_ge_trunk_enable_set(int unit, int tid, uint32 enable)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, GLOBAL_TRUNK_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, GLOBAL_TRUNK_CTLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, GLOBAL_TRUNK_CTLr, &reg_value, EN_TRUNK_GIGAf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_value,reg_len));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_ge_trunk_enable_get
 *
 *  Purpose :
 *      Get the status of the selected GE trunk.
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
int _drv_ge_trunk_enable_get(int unit, int tid, uint32 *enable)
{

    uint32	reg_addr, reg_value, temp;
    int		reg_len;

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, GLOBAL_TRUNK_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, GLOBAL_TRUNK_CTLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, GLOBAL_TRUNK_CTLr, &reg_value, EN_TRUNK_GIGAf, &temp));
    if (temp) {
        *enable =1;
    } else {
        *enable =0;
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
drv_trunk_set(int unit, int tid, soc_pbmp_t bmp, uint32 flag, uint32 hash_op)
{
    uint32	       reg_addr, reg_value, temp;
    int		reg_len, reg_index, field_index, en_field_index, action;
    uint32  bmp_value = 0;

    bmp_value = SOC_PBMP_WORD_GET(bmp, 0);

    soc_cm_debug(DK_PORT, 
    "drv_trunk_set: unit = %d, trunk id = %d, bmp = %x, flag = %x\n",
        unit, tid, bmp_value, flag);
    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (hash_op & DRV_TRUNK_HASH_FIELD_MACDA) {
            if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                default_trunk_seed = TRUNK_HASH_FIELD_MACDASA_VALUE;
            } else {
                if (SOC_IS_ROBO5324(unit)) {
                    default_trunk_seed = TRUNK_HASH_FIELD_MACDA_VALUE_5324;
                } else {
                    default_trunk_seed = TRUNK_HASH_FIELD_MACDA_VALUE;
                }
            }
        } else if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
            if (SOC_IS_ROBO5324(unit)) {
                default_trunk_seed = TRUNK_HASH_FIELD_MACSA_VALUE_5324;
            } else {
                default_trunk_seed = TRUNK_HASH_FIELD_MACSA_VALUE;
            }
        }
        return SOC_E_NONE;
    }
    
    switch (tid) {
        case 0:
            reg_index = TRUNK_GRP0_CTLr;
            en_field_index = EN_TRUNK_GRP0f;
            field_index = TRUNK_PORT_MAP0f;
            break;
        case 1:
            reg_index = TRUNK_GRP1_CTLr;
            en_field_index = EN_TRUNK_GRP1f;
            field_index = TRUNK_PORT_MAP1f;
            break;
        case 2: /* giga trunk */ 
            /* enable giga trunk */
            /* check only GE ports */
            if (bmp_value & ~(SOC_PBMP_WORD_GET(PBMP_GE_ALL(unit), 0))) {
                return SOC_E_PARAM;
            }
            if ((flag & DRV_TRUNK_FLAG_ENABLE) &&
                 (bmp_value == (SOC_PBMP_WORD_GET(PBMP_GE_ALL(unit), 0)))){
                SOC_IF_ERROR_RETURN(
                    _drv_ge_trunk_enable_set(unit, tid, TRUE));
            } else if (flag & DRV_TRUNK_FLAG_DISABLE) {
                SOC_IF_ERROR_RETURN(
                    _drv_ge_trunk_enable_set(unit, tid, FALSE));
            }
            return SOC_E_NONE;
        default:
            return SOC_E_PARAM;        
    }

    action = 0;

    /* Read the trunk group control by trunk id */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));

    /* FE port trunk */
    if ((flag & DRV_TRUNK_FLAG_ENABLE) &&
	(flag & DRV_TRUNK_FLAG_BITMAP) && bmp_value) {
	/* Enable global FE trunk, keep it enabled all the time */
        SOC_IF_ERROR_RETURN(
            _drv_fe_trunk_enable_set(unit, tid, TRUE));

	temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, &reg_value, en_field_index, &temp));

        /* check only FE ports */
        if (bmp_value & ~(SOC_PBMP_WORD_GET(PBMP_FE_ALL(unit), 0))) {
            return SOC_E_PARAM;
        }
        temp = bmp_value;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, &reg_value, field_index, &temp));

	action = 1;
    } else if ((flag & DRV_TRUNK_FLAG_DISABLE) ||
	       ((flag & DRV_TRUNK_FLAG_ENABLE) &&
		(flag & DRV_TRUNK_FLAG_BITMAP) && !bmp_value)){
        /* disable FE port trunk, nothing in this calling */
        SOC_IF_ERROR_RETURN(
        _drv_fe_trunk_enable_set(unit, tid, FALSE));

	temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, &reg_value, en_field_index, &temp));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, &reg_value, field_index, &temp));

	action = 1;
    }

    if (action) {
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
drv_trunk_get(int unit, int tid, soc_pbmp_t *bmp, uint32 flag, uint32 *hash_op)
{
    uint32      reg_addr, reg_value, temp, enable;
    int         reg_len, reg_index, field_index;

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (SOC_IS_ROBO5324(unit)) {
            switch (default_trunk_seed) {    
              case TRUNK_HASH_FIELD_MACDASA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA | DRV_TRUNK_HASH_FIELD_MACSA;
                break;
              case TRUNK_HASH_FIELD_MACDA_VALUE_5324:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA;
                break;
              case TRUNK_HASH_FIELD_MACSA_VALUE_5324:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACSA;
                break;
              default:
		return SOC_E_INTERNAL;
            }
        } else {
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
        }
        soc_cm_debug(DK_PORT, 
        "drv_trunk_get: unit = %d, trunk id = %d, value = 0x%x, flag = %x\n",
        unit, tid, *hash_op, flag);
        return SOC_E_NONE;
    }
    
    /* Get trunk group enable bit */
    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        if (tid < (DRV_TRUNK_GROUP_NUM -1)) {
            SOC_IF_ERROR_RETURN(
                _drv_fe_trunk_enable_get(unit, tid, &enable));
        } else if (tid == (DRV_TRUNK_GROUP_NUM -1)) { 
            /* max trunk id -> giga trunk */
            SOC_IF_ERROR_RETURN(
            _drv_ge_trunk_enable_get(unit, tid, &enable));
        } else { /* tid > max trunk id */
            return SOC_E_PARAM;
        }
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }

    temp = 0;
    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        switch (tid) {
            case 0:
                reg_index = TRUNK_GRP0_CTLr;
                field_index = TRUNK_PORT_MAP0f;
                break;
            case 1:
                reg_index = TRUNK_GRP1_CTLr;
                field_index = TRUNK_PORT_MAP1f;
                break;
            case 2:
                _drv_ge_trunk_enable_get(unit, tid, &enable);
                if (enable) {
                    SOC_PBMP_ASSIGN(*bmp, PBMP_GE_ALL(unit));
                } else {
                    SOC_PBMP_CLEAR(*bmp);
                }
                return SOC_E_NONE;
            default:
                return SOC_E_PARAM;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, &reg_value, field_index, &temp));
        SOC_PBMP_WORD_SET(*bmp, 0, temp);
    }
    soc_cm_debug(DK_PORT, 
        "drv_trunk_get: unit = %d, trunk id = %d, value = 0x%x, flag = %x\n",
        unit, tid, SOC_PBMP_WORD_SET(*bmp, 0, temp), flag);
    return SOC_E_NONE;
}

/*
 *  Function : drv_trunk_hash_field_add
 *
 *  Purpose :
 *      Add trunk hash field type
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      tid   :   trunk id.
 *      field_type   :   trunk hash field type to be add.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_trunk_hash_field_add(int unit, uint32 field_type)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;

    soc_cm_debug(DK_PORT, 
        "drv_trunk_hash_field_add: unit = %d, field type = %d\n",
        unit, field_type);

    /* check the valid trunk hash field types */
    if (field_type & ~TRUNK_HASH_FIELD_VALID_VALUE) {
        return SOC_E_UNAVAIL;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, GLOBAL_TRUNK_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, GLOBAL_TRUNK_CTLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, GLOBAL_TRUNK_CTLr, &reg_value, TRUNK_SEEDf, &temp));

    if (SOC_IS_ROBO5324(unit) ||SOC_IS_ROBO5348(unit)||
        SOC_IS_ROBO5347(unit)) {
        switch (temp) {
          case TRUNK_HASH_FIELD_MACDASA_VALUE: /* DA SA */
              /* nothing to do */
            break;
          case TRUNK_HASH_FIELD_MACSA_VALUE_5324: /* SA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
                temp = TRUNK_HASH_FIELD_MACDASA_VALUE;
            } 
            break;
          case TRUNK_HASH_FIELD_MACDA_VALUE_5324: /* DA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                temp = TRUNK_HASH_FIELD_MACDASA_VALUE;
            }
            break;
          case 3:
             /*  should not be happen */
            return SOC_E_INTERNAL;
        }           
    } else {
        switch (temp) {
          case TRUNK_HASH_FIELD_MACDASA_VALUE: /* DA SA */
            /* nothing to do */
            break;
          case TRUNK_HASH_FIELD_MACSA_VALUE: /* SA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
                temp = TRUNK_HASH_FIELD_MACDASA_VALUE;
            } 
            break;
          case TRUNK_HASH_FIELD_MACDA_VALUE: /* DA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                temp = TRUNK_HASH_FIELD_MACDASA_VALUE;
            }
            break;
          case 3:
            /*  should not be happen */
            return SOC_E_INTERNAL;
        }
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
         (unit, GLOBAL_TRUNK_CTLr, &reg_value, TRUNK_SEEDf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
         (unit, reg_addr, &reg_value,reg_len));

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
 *      
 *
 */
int 
drv_trunk_hash_field_remove(int unit, uint32 field_type)
{
    uint32	reg_addr, reg_value, temp;
    int		reg_len;

    soc_cm_debug(DK_PORT, 
        "drv_trunk_hash_field_remove: unit = %d, field type = %d\n",
        unit, field_type);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, GLOBAL_TRUNK_CTLr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, GLOBAL_TRUNK_CTLr);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_value,reg_len));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, GLOBAL_TRUNK_CTLr, &reg_value, TRUNK_SEEDf, &temp));
    
    if (SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit)||
        SOC_IS_ROBO5347(unit)) {    
        switch (temp) {
          case TRUNK_HASH_FIELD_MACDASA_VALUE: /* DA SA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
                if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                    temp  = default_trunk_seed;
                } else {
                    temp = TRUNK_HASH_FIELD_MACSA_VALUE_5324;
                }
            } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                temp = TRUNK_HASH_FIELD_MACDA_VALUE_5324;
            } else {
                return SOC_E_NONE;
            }
            break;
          case TRUNK_HASH_FIELD_MACSA_VALUE_5324: /* SA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                temp = default_trunk_seed;
            } 
            break;
          case TRUNK_HASH_FIELD_MACDA_VALUE_5324: /* DA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
                temp = default_trunk_seed;
            }
            break;
          case 3:
            /*  should not be happen */
            return SOC_E_INTERNAL;
        }
    } else {
        switch (temp) {
          case TRUNK_HASH_FIELD_MACDASA_VALUE: /* DA SA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
                if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                    temp  = default_trunk_seed;
                } else {
                    temp = TRUNK_HASH_FIELD_MACSA_VALUE;
                }
            } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                temp = TRUNK_HASH_FIELD_MACDA_VALUE;
            } else {
                return SOC_E_NONE;
            }
            break;
          case TRUNK_HASH_FIELD_MACSA_VALUE: /* SA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
                temp = default_trunk_seed;
            } 
            break;
          case TRUNK_HASH_FIELD_MACDA_VALUE: /* DA */
            if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
                temp = default_trunk_seed;
            }
            break;
          case 3:
            /*  should not be happen */
            return SOC_E_INTERNAL;
        }
    }    
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
         (unit, GLOBAL_TRUNK_CTLr, &reg_value, TRUNK_SEEDf, &temp));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
         (unit, reg_addr, &reg_value,reg_len));

    return SOC_E_NONE;
}
