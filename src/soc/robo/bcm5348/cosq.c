/*
 * $Id: cosq.c 1.9 Broadcom SDK $
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

/* enable/disable selected mapping - prio/diffserv/tos */
/*
 *  Function : drv_bcm5348_queue_mapping_state_set
 *
 *  Purpose :
 *      Set the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      mapping_type   :   queue mapping type (prio/tos/diffserv).
 *      state  :   The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5348_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state)
{
    uint32	reg_addr, reg_value, temp, val_32;
    int		reg_len;
    uint64  reg_value64, pbmp_value, temp64;

    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_set: unit %d, bmp = %x %x, type = %d,  %sable\n",
        unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1),
        mapping_type, state ? "en" : "dis");
    COMPILER_64_ZERO(temp64);
    switch (mapping_type) {
    case DRV_QUEUE_MAP_NONE:
        /* disable QOS_IP_EN register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
            (uint32 *)&temp64));
        soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
        COMPILER_64_NOT(pbmp_value);
        COMPILER_64_AND(temp64, pbmp_value);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
            (uint32 *)&temp64));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
                (uint32 *)&temp));
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            temp &= ~val_32;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
                (uint32 *)&temp));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        /* disable QOS_TOS_DIF_EN register */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
        COMPILER_64_NOT(pbmp_value);
        COMPILER_64_AND(temp64, pbmp_value);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            temp &= ~val_32;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        break;
    case DRV_QUEUE_MAP_PRIO:
        /* enable/disable QOS_1P_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
            (uint32 *)&temp64));
        soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
        if (state) {
            COMPILER_64_OR(temp64, pbmp_value);
        } else {
            COMPILER_64_NOT(pbmp_value);
            COMPILER_64_AND(temp64, pbmp_value);
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
            (uint32 *)&temp64));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
                (uint32 *)&temp));
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
                (uint32 *)&temp));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

        /* don't need to disable QOS_TOS_DIF_CTLr : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_TOS:
        /* set QOS_TOS_DIF_CTLr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN(
            (DRV_SERVICES(unit)->reg_read)(unit, reg_addr, &reg_value,reg_len));
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        /* enable / diaable QOS_TOS_DIF_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
        if (state) {
            COMPILER_64_OR(temp64, pbmp_value);
        } else {
            COMPILER_64_NOT(pbmp_value);
            COMPILER_64_AND(temp64, pbmp_value);
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

        /* don't need to disable QOS_1P_ENr : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_DFSV:
        /* set QOS_TOS_DIF_CTLr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        temp = 0;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_value,reg_len));

        /* enable / diaable QOS_TOS_DIF_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
        if (state) {
            COMPILER_64_OR(temp64, pbmp_value);
        } else {
            COMPILER_64_NOT(pbmp_value);
            COMPILER_64_AND(temp64, pbmp_value);
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));

        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));

        /* don't need to disable QOS_1P_ENr : 
          * the setting is separated and chosen by QoS priority decision flowchart
          */
        break;
    case DRV_QUEUE_MAP_PORT:
        /* enable/disable QOS_1P_ENr */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_CTLr, (uint32 *)&reg_value64, PRI_PMASKf, 
            (uint32 *)&temp64));
        soc_robo_64_pbmp_to_val(unit, &bmp, &pbmp_value);
        if (state) {
            COMPILER_64_OR(temp64, pbmp_value);
        } else {
            COMPILER_64_NOT(pbmp_value);
            COMPILER_64_AND(temp64, pbmp_value);
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_CTLr, (uint32 *)&reg_value64, PRI_PMASKf, 
            (uint32 *)&temp64));
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_CTLr, (uint32 *)&reg_value64, PRI_PMASKf, 
                (uint32 *)&temp));
            val_32 = SOC_PBMP_WORD_GET(bmp, 0);
            if (state) {
                temp |= val_32;
            } else {
                temp &= ~val_32;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, QOS_CTLr, (uint32 *)&reg_value64, PRI_PMASKf, 
                (uint32 *)&temp));
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        break;
    case DRV_QUEUE_MAP_MAC:
    case DRV_QUEUE_MAP_HYBRID:
        return SOC_E_UNAVAIL;
    default :
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_bcm5348_queue_mapping_state_get
 *
 *  Purpose :
 *      Get the state to the specific queue mapping type.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port    :   port number.
 *      mapping_type   :   queue mapping type (prio/tos/diffserv).
 *      state  :   The state of the selected mapping type.
 *
 *  Return :
 *      SOC_E_XXX.
 *
 *  Note :
 *      
 *
 */
int 
drv_bcm5348_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state)
{
    uint32	reg_addr, reg_value;
    int		reg_len;
    uint32	temp;
    uint64  reg_value64, temp64;
    soc_pbmp_t pbmp;

    switch (mapping_type)
    {
    case DRV_QUEUE_MAP_NONE:
        return SOC_E_PARAM;
    case DRV_QUEUE_MAP_PRIO:
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_1P_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_1P_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
            (uint32 *)&temp64);
        SOC_PBMP_CLEAR(pbmp);
        soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
        } else {
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_1P_ENr, (uint32 *)&reg_value64, QOS_1P_ENf, 
                (uint32 *)&temp);
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
        }
        if (SOC_PBMP_MEMBER(pbmp, port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_TOS:
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        SOC_PBMP_CLEAR(pbmp);
        soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
        }
        if (!SOC_PBMP_MEMBER(pbmp, port)) {
            *state = FALSE;
            break;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        if (temp) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_DFSV:
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_ENr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_ENr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp64));
        SOC_PBMP_CLEAR(pbmp);
        soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
        } else {
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_TOS_DIF_ENr, (uint32 *)&reg_value64, 
                QOS_TOS_DIFF_ENf, (uint32 *)&temp));
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
        }
        if (!SOC_PBMP_MEMBER(pbmp, port)) {
            *state = FALSE;
            break;
        }
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_TOS_DIF_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_TOS_DIF_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_value,reg_len));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, QOS_TOS_DIF_CTLr, &reg_value, TOS_DIFF_SELf, &temp));
        if (temp) {
            *state = FALSE;
        } else {
            *state = TRUE;
        }
        break;
    case DRV_QUEUE_MAP_PORT:
        /* get Port-based QoS status */
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, QOS_CTLr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, QOS_CTLr);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_value64,reg_len));
        if (SOC_INFO(unit).port_num > 32) {
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_CTLr, (uint32 *)&reg_value64, PRI_PMASKf, 
                (uint32 *)&temp64);
            SOC_PBMP_CLEAR(pbmp);
            soc_robo_64_val_to_pbmp(unit, &pbmp, temp64);
        } else {
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, QOS_CTLr, (uint32 *)&reg_value64, PRI_PMASKf, 
                (uint32 *)&temp);
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
        }
        if (SOC_PBMP_MEMBER(pbmp, port)) {
            *state = TRUE;
        } else {
            *state = FALSE;
        }
        break;
    case DRV_QUEUE_MAP_MAC:
    case DRV_QUEUE_MAP_HYBRID:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }
    soc_cm_debug(DK_VERBOSE, 
        "drv_queue_mapping_type_get: unit %d, port = %d, type = %d,  %sable\n",
        unit, port, mapping_type, *state ? "en" : "dis");
    return SOC_E_NONE;
}
