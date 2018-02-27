/* 
 * $Id: port.c 1.5 Broadcom SDK $
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
 * Common driver routines for accessing the CMIC functional unit.
 */

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>


int 
drv_bcm53101_port_cross_connect_set(int unit,uint32 port,soc_pbmp_t bmp)
{
    int rv = SOC_E_NONE;
    uint32 reg_value = 0;
    uint32 temp = 0;
    uint32  field_val = 0;
    
    soc_cm_debug(DK_PORT, 
        "drv53101_port_cross_connect_set: unit=%d port = %d, bmp=%x\n",
        unit, port, SOC_PBMP_WORD_GET(bmp, 0));
    
    SOC_IF_ERROR_RETURN(
        REG_READ_PORT_CROSS_CONNECTr(unit, port, &reg_value));

    temp = SOC_PBMP_WORD_GET(bmp, 0);
    soc_PORT_CROSS_CONNECTr_field_set(unit, &reg_value, 
                PCC_DESTf, &temp);

    if (SOC_PBMP_IS_NULL(bmp)) {
        temp = 0;
    } else {
        temp = 1;
    }
    /* Configure the port cross connect enable bit */
    soc_PORT_CROSS_CONNECTr_field_set(unit, &reg_value, 
                PCC_ENABLEf, &temp);

    SOC_IF_ERROR_RETURN(
        REG_WRITE_PORT_CROSS_CONNECTr(unit, port, &reg_value));

    /* Configure the preserve ingress packets format */
    SOC_IF_ERROR_RETURN(
        REG_READ_PRESERVE_PKT_FORMATr(unit, &reg_value));

    soc_PRESERVE_PKT_FORMATr_field_get(unit, &reg_value, 
                PRESERVE_PACKET_FORMATf, &field_val);
    if (temp) {
        field_val |= (0x1 << port);
    } else {
        field_val &= ~(0x1 << port);
    }
    soc_PRESERVE_PKT_FORMATr_field_set(unit, &reg_value, 
                PRESERVE_PACKET_FORMATf, &temp);

    SOC_IF_ERROR_RETURN(
        REG_WRITE_PRESERVE_PKT_FORMATr(unit, &reg_value));

    return rv;
}

int 
drv_bcm53101_port_cross_connect_get(int unit,uint32 port,soc_pbmp_t *bmp)
{
    int rv = SOC_E_NONE;
    uint32 reg_value = 0;
    uint32 temp = 0;
    
    soc_cm_debug(DK_PORT, 
        "drv53101_port_cross_connect_get: unit=%d port = %d\n",
        unit, port);

    SOC_IF_ERROR_RETURN(
        REG_READ_PORT_CROSS_CONNECTr(unit, port, &reg_value));

    soc_PORT_CROSS_CONNECTr_field_get(unit, &reg_value, 
                PCC_DESTf, &temp);
    
    SOC_PBMP_WORD_SET(*bmp, 0 , temp);
    
    return rv;
}
