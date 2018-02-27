/*
 * $Id: dos.c 1.5 Broadcom SDK $
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
 * Field Processor related CLI commands
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv_if.h>
#include <soc/drv.h>

 int 
drv_bcm5395_dos_enable_set(int unit, uint32 type, uint32 param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_DOS_NONE:
            /* Remove all preventions for DOS attack */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            temp = 0;
            /* DRV_DOS_TCP_FRAG */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SHORT_IP_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_SMURF */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SMURF_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_SYN_WITH_SP_LT1024 */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SYN_WITH_SP_LT1024_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_SYN_FIN_SCAN */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SIN_FINSCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_XMASS_SCAN */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                XMAS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_NULL_SCAN */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_BLAT */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_LAND */
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                LAND_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            /* DRV_DOS_DISABLE_LEARN */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_DIS_LRN_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_DIS_LRN_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_DIS_LRN_REGr, &reg_value, 
                DOS_DIS_LRNf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_TCP_FRAG:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SHORT_IP_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_SMURF:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SMURF_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_SYN_WITH_SP_LT1024:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SYN_WITH_SP_LT1024_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_SYN_FIN_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                SIN_FINSCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_XMASS_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                XMAS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 3;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                XMAS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_NULL_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_NULL_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 3;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_BLAT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_LAND:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_CTRLr, &reg_value, 
                LAND_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_MIN_TCP_HDR_SZ:
            /* register's value range : 0 - 60 */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MINIMUM_TCP_HDR_SZr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MINIMUM_TCP_HDR_SZr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            temp = param;
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, MINIMUM_TCP_HDR_SZr, &reg_value, 
                MIN_TCP_HDR_SZf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        case DRV_DOS_DISABLE_LEARN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_DIS_LRN_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_DIS_LRN_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if (param){
                temp = 1;
            } else {
                temp = 0;
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_set)
                (unit, DOS_DIS_LRN_REGr, &reg_value, 
                DOS_DIS_LRNf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_value,reg_len))) <0 ) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to write register : %s\n",
                    soc_errmsg(rv));
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

int 
drv_bcm5395_dos_enable_get(int unit, uint32 type, uint32 *param)
{
    int rv = SOC_E_NONE;
    uint32 reg_addr, reg_value, temp;
    int reg_len;
    
    switch (type) {
        case DRV_DOS_TCP_FRAG:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                SHORT_IP_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to get register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_SMURF:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                SMURF_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_SYN_WITH_SP_LT1024:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                SYN_WITH_SP_LT1024_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_SYN_FIN_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                SIN_FINSCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_XMASS_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                XMAS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp == 1){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                XMAS_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            
            if (temp == 3){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_NULL_SCAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp == 1){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_NULL_WITH_TCP_SEQ_ZERO:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ( (rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                NULL_SCAN_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp == 3){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_BLAT:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                BLAT_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_LAND:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_CTRLr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_CTRLr, &reg_value, 
                LAND_DROP_ENf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        case DRV_DOS_MIN_TCP_HDR_SZ:
            /* register's value range : 0 - 60 */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, MINIMUM_TCP_HDR_SZr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, MINIMUM_TCP_HDR_SZr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, MINIMUM_TCP_HDR_SZr, &reg_value, 
                MIN_TCP_HDR_SZf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            *param = temp;
            break;
        case DRV_DOS_DISABLE_LEARN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, DOS_DIS_LRN_REGr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, DOS_DIS_LRN_REGr);
            if ((rv = ((DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_value,reg_len))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to read register : %s\n",
                    soc_errmsg(rv));
            }
            if ((rv = ((DRV_SERVICES(unit)->reg_field_get)
                (unit, DOS_DIS_LRN_REGr, &reg_value, 
                DOS_DIS_LRNf, &temp))) < 0) {
                soc_cm_debug(DK_WARN,
                    "Warnning : Failed to set register field value : %s\n",
                    soc_errmsg(rv));
            }
            if (temp){
                *param = 1;
            } else {
                *param = 0;
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}
