/*
 * $Id: trunk.c 1.5 Broadcom SDK $
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
#include "bcm5396/robo_5396.h"

#define 	TRUNK_HASH_FIELD_MACDA_VALUE		1
#define 	TRUNK_HASH_FIELD_MACSA_VALUE		2
#define 	TRUNK_HASH_FIELD_MACDASA_VALUE		0
static uint32   default_trunk_seed  = TRUNK_HASH_FIELD_MACDASA_VALUE;
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
_drv_trunk_enable_set(int unit, uint32 enable)
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
_drv_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32	reg_addr, reg_value, temp, bmp = 0, t_grp = 0, t_field = 0;
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
	 if(SOC_IS_ROBO5396(unit)) {
           reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
           reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

           SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
               (unit, reg_addr, &reg_value,reg_len));

           bmp = 0;
           SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
               (unit, TRUNK_GRP_CTLr, &reg_value, TRUNK_PORT_MAP0f, &bmp));

	 } else if(SOC_IS_ROBO5389(unit)) {
           reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
           reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

           SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
               (unit, reg_addr, &reg_value,reg_len));

           bmp = 0;
           SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
               (unit, TRUNK_GRP_CTLr, &reg_value, EN_TRUNK_GRP0f, &bmp));

	 } else if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
	    switch(tid) {
                case 0:
		      t_grp = TRUNK_GRP0_CTLr;
		      t_field = TRUNK_PORT_MAP0f;
                    break;
                case 1:
		      t_grp = TRUNK_GRP1_CTLr;
		      t_field = TRUNK_PORT_MAP1f;
                    break;
                case 2:
		      t_grp = TRUNK_GRP2_CTLr;
		      t_field = TRUNK_PORT_MAP2f;
                    break;
                case 3:
		      t_grp = TRUNK_GRP3_CTLr;
		      t_field = TRUNK_PORT_MAP3f;
                    break;
                default:
                    return SOC_E_PARAM;                                
            }
	     reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, t_grp, 0, tid);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, t_grp);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));

            bmp = 0;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, t_grp, &reg_value, t_field, &bmp));

	 }
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
drv_bcm5396_trunk_set(int unit, int tid, soc_pbmp_t bmp, uint32 flag, uint32 hash_op)
{
    uint32	       reg_addr, reg_value, temp, c_temp, t_grp = 0, t_field = 0;
    int		reg_len;
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
        _drv_trunk_enable_set(unit, TRUE);
    } else if (flag & DRV_TRUNK_FLAG_DISABLE){
        _drv_trunk_enable_set(unit, FALSE);
    }

    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        if(SOC_IS_ROBO5396(unit)){
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));

            if (bmp_value & ~(SOC_PBMP_WORD_GET(PBMP_PORT_ALL(unit), 0))) {
                return SOC_E_PARAM;
            }
            temp = bmp_value;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, TRUNK_GRP_CTLr, &reg_value, TRUNK_PORT_MAP0f, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
        } else if(SOC_IS_ROBO5389(unit)){
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
			
            switch(tid) {
                case 0: /* port 0-1*/
                    if(bmp_value & 0xfc) {
                        return SOC_E_PARAM;
                    }
                    temp = 1;
                    break;
                case 1: /* port 2-3*/
                    if(bmp_value & 0xf3) {
                        return SOC_E_PARAM;
                    }
                    temp = 2;
                    break;
                case 2: /* port 4-5*/
                    if(bmp_value & 0xcf) {
                        return SOC_E_PARAM;
                    }
                    temp = 4;
                    break;
                case 3: /* port 6-7*/
                    if(bmp_value & 0x3f) {
                        return SOC_E_PARAM;
                    }                    
                    temp = 8;
                    break;
                default:
                    return SOC_E_PARAM;                                
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, TRUNK_GRP_CTLr, &reg_value, EN_TRUNK_GRP0f, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));
            } else if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {        
              c_temp =0;
            if(bmp_value != 0x0) {
                 temp = 0x1;
	          while(temp < 0x100) {
                    if(bmp_value & temp) {
		           c_temp ++;
		        }
                    if(c_temp > 4) {
                        return SOC_E_PARAM;
                    }
		        temp <<= 1;
	          }
		}

             switch(tid) {
                 case 0:
	           t_grp = TRUNK_GRP0_CTLr;
	           t_field = TRUNK_PORT_MAP0f;
                     break;
                 case 1:
	           t_grp = TRUNK_GRP1_CTLr;
	           t_field = TRUNK_PORT_MAP1f;
                    break;
                 case 2:
	           t_grp = TRUNK_GRP2_CTLr;
	           t_field = TRUNK_PORT_MAP2f;
                     break;
                 case 3:
	           t_grp = TRUNK_GRP3_CTLr;
	           t_field = TRUNK_PORT_MAP3f;
                     break;
                 default:
                     return SOC_E_PARAM;
             }
			
            temp = bmp_value;
	     reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, t_grp, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, t_grp);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
			
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, t_grp, &reg_value, t_field, &temp));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len));

        }
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
drv_bcm5396_trunk_get(int unit, int tid, soc_pbmp_t *bmp, uint32 flag, uint32 *hash_op)
{
    uint32      reg_addr, reg_value, temp, t_grp = 0, t_field = 0, enable;
    int           reg_len;

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
                _drv_trunk_enable_get(unit, tid, &enable));
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }

    
    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        if(SOC_IS_ROBO5396(unit)) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, tid);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, TRUNK_GRP_CTLr, &reg_value, TRUNK_PORT_MAP0f, &temp));
            SOC_PBMP_WORD_SET(*bmp, 0, temp);
        } else if(SOC_IS_ROBO5389(unit)){
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, TRUNK_GRP_CTLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, TRUNK_GRP_CTLr);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, TRUNK_GRP_CTLr, &reg_value, EN_TRUNK_GRP0f, &temp));
            switch(tid) {
                case 0: /* port 0-1*/
                    if( temp & 0x1 ) {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0x3);
                    } else {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0);
                    }
                    break;
                case 1: /* port 2-3*/
                    if( temp & 0x2 ) {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0xc);
                    } else {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0);
                    }
                    break;
                case 2: /* port 4-5*/
                    if( temp & 0x4 ) {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0x30);
                    } else {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0);
                    }
                    break;
                case 3: /* port 6-7*/
                    if( temp & 0x8 ) {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0xc0);
                    } else {
                        SOC_PBMP_WORD_SET(*bmp, 0, 0);
                    }
                    break;
                default:
                    return SOC_E_PARAM;                                
            }
        } else if (SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit)) {
            switch(tid) {
                case 0:
		      t_grp = TRUNK_GRP0_CTLr;
		      t_field = TRUNK_PORT_MAP0f;
                    break;
                case 1:
		      t_grp = TRUNK_GRP1_CTLr;
		      t_field = TRUNK_PORT_MAP1f;
                    break;
                case 2:
		      t_grp = TRUNK_GRP2_CTLr;
		      t_field = TRUNK_PORT_MAP2f;
                    break;
                case 3:
		      t_grp = TRUNK_GRP3_CTLr;
		      t_field = TRUNK_PORT_MAP3f;
                    break;
                default:
                    return SOC_E_PARAM;                                
            }
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, t_grp, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, t_grp);

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len));
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, t_grp, &reg_value, t_field, &temp));
            SOC_PBMP_WORD_SET(*bmp, 0, temp);
	
        }
    }
    soc_cm_debug(DK_PORT, 
        "drv_trunk_get: unit = %d, trunk id = %d, value = 0x%x, flag = %x\n",
        unit, tid, SOC_PBMP_WORD_GET(*bmp, 0), flag);
    return SOC_E_NONE;
}
