/*
 * $Id: l3_defip.c 1.5.198.2 Broadcom SDK $
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
 * XGS L3 Default IP table testing routines
 */

#include <appl/diag/l3_defip.h>
#include <appl/diag/system.h>
#include <bcm/l3.h>
#include <sal/core/sync.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/mbcm.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <sal/appl/io.h>

#ifdef INCLUDE_L3
#ifdef BCM_XGS_SWITCH_SUPPORT

/* NOTE : This must be same as LPM_BITS_PER_BLK in draco/l3.c */
#define LPM_BITS_PER_BLK	(4)

extern int _lpm_defip_sw_tbl_empty(int unit, int *count);
extern int _lpm_defip_hw_tbl_empty(int unit, int *count);
extern int _lpm_check_table(int unit);
extern int _bcm_xgs_lpm_delete(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int _bcm_xgs_lpm_insert(int unit, _bcm_defip_cfg_t *lpm_cfg);
extern int _bcm_xgs_lpm_lookup(int unit, ip_addr_t iproute, 
                                 _bcm_defip_cfg_t* lpm_cfg);

/*
 * Function:
 *	diag_defip_check
 * Purpose:
 *	Perform sanity check on DEFIP HW and SW tables
 * Parameters:
 *	unit - StrataSwitch unit number.
 * Returns:
 *	BCM_E_XXX
 */
int
diag_defip_check(int unit, int flags)
{
    int count = 0;

    switch (flags) {
    case 0:
        if (mbcm_driver[unit]->mbcm_lpm_check_table(unit) != TRUE) {
            soc_cm_debug(DK_L3,
                         "The S/W and H/W LPM tables are not in sync\n");
        } else {
            soc_cm_debug(DK_L3,
                         "The S/W and H/W LPM tables are in sync\n");
        }
        break;
    case 1:
        if (mbcm_driver[unit]->mbcm_lpm_defip_sw_tbl_empty(unit, &count) 
            != TRUE) {
            soc_cm_debug(DK_L3,
               " SW Copy of LPM(valid count = %d) Table is not empty\n",count);
        } else {
            soc_cm_debug(DK_L3,
              " SW Copy of LPM(valid count = %d) Table is empty\n",count);
        }
        break;
    case 2:
        if (mbcm_driver[unit]->mbcm_lpm_defip_hw_tbl_empty(unit, &count) 
            != TRUE) {
            soc_cm_debug(DK_L3,
              " HW Copy of LPM(valid count = %d) Table is not empty\n",count);
        } else {
            soc_cm_debug(DK_L3,
              " HW Copy of LPM(valid count = %d) Table is empty\n",count);
        }
        break;
    default:
        soc_cm_debug(DK_L3,
              "Unknown sub command for check. use either cmp/hw/sw\n");
        soc_cm_debug(DK_L3,
              "cmp -> Compares HW table with SW copy\n");
        soc_cm_debug(DK_L3,
              "hw -> Checks if the HW LPM Table is completely empty\n");
        soc_cm_debug(DK_L3,
            "sw -> Checks if the SW copy of LPM Table is completely empty\n");
        break;
    }

    return (0);
}

/*
 * Function:
 *	diag_defip_test
 * Purpose:
 *	DEFIP test (MUST GET RID OF THIS)
 * Parameters:
 *	unit - StrataSwitch unit number.
 * Returns:
 */

int
diag_defip_test(int unit)
{
#ifdef NO_FILEIO
    printk("diag_defip_test: no filesystem\n");
    return 0;
#else
    FILE *f;
    char *fname = "ipaddr";
    char str[128], cmd, *ipaddr_str, *mask_str, *tmp_str;
    ip_addr_t ipaddr;
    int mask, nibbles, lpm;
    int i_add_fail = 0, i_add_success = 0,i = 0,add = 0,
        i_del_fail = 0, i_del_success = 0;
    _bcm_defip_cfg_t cfg;

#if 0
    static int init_done = 0;
   
    if (!init_done) {
        if (_bcm_draco_defip_table_init(unit) < 0) {
            soc_cm_debug(DK_L3, "if_defip_test() : DEFIP INIT FAILED\n");
            return 0;
        }
        init_done = 1;
    }
#endif

    f = sal_fopen(fname, "r");
    if (!f) {
        perror(fname);
        return (-1);
    }

    while (fgets(str, sizeof(str), f)) {
        ++i;

        /* Strip trailing whitespace */
        tmp_str = str;
        while (*tmp_str != 0) {
            if (*tmp_str <= ' ') {
                *tmp_str = 0;
                break;
            }
            tmp_str++;
        }

        if (i == 1){
            if (!strcmp(str,"add")) {
                add=1;
                continue;
            }
        }

        /* Parse "%c%d.%d.%d.%d/%d" (add=1) or "%d.%d.%d.%d/%d" (add=0) */
        cmd = 0;
        mask = 0;
        ipaddr_str = str;
        if (add) {
            cmd = *ipaddr_str++;
        }
        mask_str = ipaddr_str;
        while (*mask_str != 0) {
            if (*mask_str == '/') {
                *mask_str++ = 0;
                break;
            }
            mask_str++;
        }
        if (!isint(mask_str)) {
            printk("diag_defip_test: bad IP mask: %s\n", mask_str);
            continue;
        }
        mask = parse_integer(mask_str);
        if (parse_ipaddr(ipaddr_str, &ipaddr) != 0) {
            printk("diag_defip_test: bad IP address: %s\n", ipaddr_str);
            continue;
        }
        nibbles = (mask+LPM_BITS_PER_BLK-1)/LPM_BITS_PER_BLK;

        if (add == 0 || cmd == 'a') {
            sal_memset(&cfg,0,sizeof(cfg));
            cfg.defip_ip_addr = ipaddr;
            cfg.defip_sub_len = mask;
            /* need vlid data here */
            sal_memset(cfg.defip_mac_addr, 0, sizeof(sal_mac_addr_t)); 
            cfg.defip_port_tgid = 0; /* need vlid data here */
            cfg.defip_modid = 0; /* need vlid data here */

            if (mbcm_driver[unit]->mbcm_lpm_insert(unit, &cfg) < 0) {

                i_add_fail++;
                soc_cm_debug(DK_L3, "populate: %s nibbles %d Fail\n", 
                             str+1, nibbles);
            } else {
                i_add_success++;
                soc_cm_debug(DK_L3, "populate: %s nibbles %d OK\n", 
                             str+1, nibbles);
            }
        }else if (add == 1 && cmd == 'd') {
            sal_memset(&cfg,0,sizeof(cfg));
            cfg.defip_ip_addr = ipaddr;
            cfg.defip_sub_len = mask;
            /* need vlid data here */
            sal_memset(cfg.defip_mac_addr, 0, sizeof(sal_mac_addr_t)); 
            cfg.defip_port_tgid = 0; /* need vlid data here */
            cfg.defip_modid = 0; /* need vlid data here */

            if (mbcm_driver[unit]->mbcm_lpm_delete(unit, &cfg) < 0) {
                i_del_fail++;
                soc_cm_debug(DK_L3, "delete: %s nibbles %d Fail\n", 
                             str+1, nibbles);
            } else {
                i_del_success++;
                soc_cm_debug(DK_L3, "delete: %s nibbles %d OK\n", 
                             str+1, nibbles);
            }
        }else if (add == 1 && cmd == 'l') {
            sal_memset(&cfg, 0, sizeof(cfg));
            lpm = mbcm_driver[unit]->mbcm_lpm_lookup(unit, ipaddr, &cfg);
            soc_cm_debug(DK_L3,
                         "lpm %d.%d.%d.%d for %s subnet_len = %d\n", 
                         (lpm>>24)&0xff,
                         (lpm>>16)&0xff,
                         (lpm>>8)&0xff,
                         (lpm)&0xff,
                         ipaddr_str, mask);
        }
    }

    soc_cm_debug(DK_L3, "p_from_file: Success %d, Fail %d\n",
                 i_add_success,i_add_fail);
    if (add) {
        soc_cm_debug(DK_L3, "del_from_file: Success %d, Fail %d\n",
                     i_del_success, i_del_fail);
    }

    if (mbcm_driver[unit]->mbcm_lpm_check_table(unit) != TRUE) {
        soc_cm_debug(DK_L3, "The S/W and H/W LPM tables are not in sync\n");
    }

#if 0 /* can not access the s/w l3 defip table */
    soc_cm_debug(DK_L3, "The Number blocks Used is %d\n",
        (LPM_SIZE/LPM_BLK_SIZE) - l3_defip_table[unit].lpm_block_count);
#endif
    sal_fclose(f);

    return (0);
#endif /* NO_FILEIO */
}

#endif /* BCM_XGS_SWITCH_SUPPORT */
#endif /* INCLUDE_L3 */
