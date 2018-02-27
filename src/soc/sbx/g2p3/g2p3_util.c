/*
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
 * $Id: g2p3_util.c 1.10.30.2 Broadcom SDK $
 *
 *-----------------------------------------------------------------------------*/

#include <soc/sbx/g2p3/g2p3_int.h>
#include <bcm_int/sbx/fe2000/allocator.h>

extern _sbx_gu2_hw_res_attrs_t _g_sbx_gu2_res_attrs[];
extern _sbx_gu2_hw_table_attrs_t _g_sbx_gu2_table_attrs[];


int soc_sbx_g2p3_allocator_shell_print(int unit);
int soc_sbx_g2p3_allocator_shell_print(int unit)
{
    int j, k, i=0;
    int fw = 10;
#if 0
    soc_cm_print("HW RESOURCES\n");
    soc_cm_print("-------------\n");
    for (i = 0; i < SBX_GU2K_HW_RES_MAX; i++) {
        /* TBD add alloc style */
        soc_cm_print("  %s:\n", _g_sbx_gu2_res_attrs[i].name);
        soc_cm_print("    Base:   %-*s0x%x\n", fw, "", _g_sbx_gu2_res_attrs[i].base);
        soc_cm_print("    Max:    %-*s0x%x\n", fw, "", _g_sbx_gu2_res_attrs[i].base + _g_sbx_gu2_res_attrs[i].max_count);
        soc_cm_print("    Count:  %-*s0x%x\n", fw, "", _g_sbx_gu2_res_attrs[i].max_count);
        soc_cm_print("    Tables: %-*s0x%08x\n", fw, "", _g_sbx_gu2_res_attrs[i].tables);
        soc_cm_print("\n");

    }
#endif
    /* for each table type find all resources which map to it */
    soc_cm_print("TABLE ALLOCATIONS\n");
    for (j = 0; j < SBX_GU2K_TABLE_MAX; j++) {
        if ( j == 0) {
            soc_cm_print("---------------------------------------------\n");
            soc_cm_print("TABLE: %-8s%-*s%-8s   %-*s%-8s\n", "", 19, "", "START", fw, "", "END");
            continue;
        }else{
            soc_cm_print("TABLE:%-8s%-*s%-8s   %-*s%-8s\n", _g_sbx_gu2_table_attrs[j].name, 19, "", "", fw, "", "");
        }

        for (k = 0; k < SBX_GU2K_HW_RES_MAX; k++) {
            /* get resource handle */
            if (_g_sbx_gu2_res_attrs[k].tables & 
                _g_sbx_gu2_table_attrs[j].mask) {
                soc_cm_print("  %-20s %-*s0x%08x %-*s0x%08x\n", _g_sbx_gu2_res_attrs[k].name, fw, "",
                             _g_sbx_gu2_res_attrs[k].base, fw, "",
                             _g_sbx_gu2_res_attrs[k].base +
                             _g_sbx_gu2_res_attrs[k].max_count - 1);
            }

        }
    }

    soc_cm_print("\nFTE PARTITIONS\n");
    soc_cm_print("--------------------------------------------------------------\n");
    soc_cm_print("%-26s %10s %10s\n", "TABLE", "START", "END");
    for (j=0; j < SOC_SBX_FSEG_MAX-1; j++) {
        uint32 first = SOC_SBX_CFG_FE2000(unit)->fteMap[j];
        uint32 last = SOC_SBX_CFG_FE2000(unit)->fteMap[j + 1] - 1;

        if (j > 0 && !first) {
            last = 0;
        } else if (!SOC_SBX_CFG_FE2000(unit)->fteMap[j + 1]) {
            i = j+2;
            while((i <= SOC_SBX_FSEG_END) && 
                  (SOC_SBX_CFG_FE2000(unit)->fteMap[i] == 0)) {
                i++;
            }

            if (i <= SOC_SBX_FSEG_END) {
                last = SOC_SBX_CFG_FE2000(unit)->fteMap[i]-1;
            }
        }

        soc_cm_print( "%-26s 0x%08x 0x%08x\n", 
                      soc_sbx_fte_segment_names[j], first, last);
    }


    return SOC_E_NONE;
}

int soc_sbx_g2p3_is_oam_rx_enabled(int unit)
{
  return SOC_SBX_OAM_RX_ENABLE(unit);
}

int soc_sbx_g2p3_is_v4mc_str_sel(int unit)
{
  return SOC_SBX_V4MC_STR_SEL(unit);
}

int soc_sbx_g2p3_is_v4uc_str_sel(int unit)
{
 return SOC_SBX_V4UC_STR_SEL(unit);
}
