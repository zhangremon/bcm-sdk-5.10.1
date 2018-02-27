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
 * $Id: g2p3_mac.c 1.1.162.1 Broadcom SDK $
 */

#include <soc/sbx/g2p3/g2p3_int.h>

/*
 * 'Regular' (customer) MAC table interface
 */
int
soc_sbx_g2p3_cmac_add_ext(int unit,
                          soc_sbx_g2p3_6_byte_t imac,
                          int ivlan,
                          soc_sbx_g2p3_cmac_t *e)
{
    soc_sbx_g2p3_mac_t mac;
    int s;

    soc_sbx_g2p3_mac_t_init(&mac);
    mac.age = e->age;
    mac.pid = e->pid;
    mac.dcopy = e->dcopy;
    mac.dcopycos = e->dcopycos;
    mac.ddrop = e->ddrop;
    mac.sdrop = e->sdrop;
    mac.dontage = e->dontage;

    s = soc_sbx_g2p3_mac_add(unit, imac, ivlan, 0, &mac);

    return s;
}

int
soc_sbx_g2p3_cmac_update_ext(int unit,
                             soc_sbx_g2p3_6_byte_t imac,
                             int ivlan,
                             soc_sbx_g2p3_cmac_t *e)
{
    soc_sbx_g2p3_mac_t mac;
    int s;

    soc_sbx_g2p3_mac_t_init(&mac);
    mac.age = e->age;
    mac.pid = e->pid;
    mac.dcopy = e->dcopy;
    mac.dcopycos = e->dcopycos;
    mac.ddrop = e->ddrop;
    mac.sdrop = e->sdrop;
    mac.dontage = e->dontage;

    s = soc_sbx_g2p3_mac_update(unit, imac, ivlan, 0, &mac);

    return s;
}

int
soc_sbx_g2p3_cmac_get_ext(int unit,
                          soc_sbx_g2p3_6_byte_t imac,
                          int ivlan,
                          soc_sbx_g2p3_cmac_t *e)
{
    soc_sbx_g2p3_mac_t mac;
    int s;

    s = soc_sbx_g2p3_mac_get(unit, imac, ivlan, 0, &mac);

    if (!s) {
        e->age = mac.age;
        e->pid = mac.pid;
        e->dcopy = mac.dcopy;
        e->dcopycos = mac.dcopycos;
        e->ddrop = mac.ddrop;
        e->sdrop = mac.sdrop;
        e->dontage = mac.dontage;
    }

    return s;
}

int
soc_sbx_g2p3_cmac_remove_ext(int unit,
                             soc_sbx_g2p3_6_byte_t imac,
                             int ivlan)
{
    int s;

    s = soc_sbx_g2p3_mac_remove(unit, imac, ivlan, 0);
    return s;
}

int
soc_sbx_g2p3_cmac_commit_ext(int unit, int runlength)
{
    int s;

    s = soc_sbx_g2p3_mac_commit(unit, runlength);
    return s;
}

int
soc_sbx_g2p3_cmac_first_ext(int unit, soc_sbx_g2p3_6_byte_t nimac,
                             int *nivlan)
{
    int s, bmac;


    for(s = soc_sbx_g2p3_mac_first(unit, nimac, nivlan, &bmac);
        s == SOC_E_NONE && bmac;
        s = soc_sbx_g2p3_mac_next(unit,
                                  nimac, *nivlan, bmac,
                                  nimac, nivlan, &bmac));
    
    return s;
}

int
soc_sbx_g2p3_cmac_next_ext(int unit, soc_sbx_g2p3_6_byte_t imac, int ivlan, 
                           soc_sbx_g2p3_6_byte_t nimac, int *nivlan)
{
    int s, bmac;


    for(s = soc_sbx_g2p3_mac_next(unit, imac, ivlan, 0, nimac, nivlan, &bmac);
        s == SOC_E_NONE && bmac;
        s = soc_sbx_g2p3_mac_next(unit,
                                  nimac, *nivlan, bmac,
                                  nimac, nivlan, &bmac));
    
    return s;
}

/*
 * Backbone MAC table interface
 */
int
soc_sbx_g2p3_bmac_add_ext(int unit,
                          soc_sbx_g2p3_6_byte_t imac,
                          int ivlan,
                          soc_sbx_g2p3_bmac_t *e)
{
    soc_sbx_g2p3_mac_t mac;
    int s;

    soc_sbx_g2p3_mac_t_init(&mac);
    mac.bsdrop = e->bsdrop;
    mac.bpid = e->bpid;
    mac.btid = e->btid;

    s = soc_sbx_g2p3_mac_add(unit, imac, ivlan, 1, &mac);

    return s;
}

int 
soc_sbx_g2p3_bmac_update_ext(int unit,
                            soc_sbx_g2p3_6_byte_t imac,
                            int ivlan,
                            soc_sbx_g2p3_bmac_t *e)
{
    soc_sbx_g2p3_mac_t mac;
    int s;

    soc_sbx_g2p3_mac_t_init(&mac);
    mac.bsdrop = e->bsdrop;
    mac.bpid = e->bpid;
    mac.btid = e->btid;

    s = soc_sbx_g2p3_mac_update(unit, imac, ivlan, 1, &mac);

    return s;
}

int
soc_sbx_g2p3_bmac_get_ext(int unit,
                          soc_sbx_g2p3_6_byte_t imac,
                          int ivlan,
                          soc_sbx_g2p3_bmac_t *e)
{
    soc_sbx_g2p3_mac_t mac;
    int s;

    s = soc_sbx_g2p3_mac_get(unit, imac, ivlan, 1, &mac);

    if (!s) {
        e->bsdrop = mac.bsdrop;
        e->bpid = mac.bpid;
        e->btid = mac.btid;
    }

    return s;
}

int
soc_sbx_g2p3_bmac_remove_ext(int unit,
                             soc_sbx_g2p3_6_byte_t imac,
                             int ivlan)
{
    int s;

    s = soc_sbx_g2p3_mac_remove(unit, imac, ivlan, 1);
    return s;
}

int
soc_sbx_g2p3_bmac_commit_ext(int unit, int runlength)
{
    int s;

    s = soc_sbx_g2p3_mac_commit(unit, runlength);
    return s;
}

int
soc_sbx_g2p3_bmac_first_ext(int unit, soc_sbx_g2p3_6_byte_t nimac,
                             int *nivlan)
{
    int s, bmac;


    for(s = soc_sbx_g2p3_mac_first(unit, nimac, nivlan, &bmac);
        s == SOC_E_NONE && !bmac;
        s = soc_sbx_g2p3_mac_next(unit,
                                  nimac, *nivlan, bmac,
                                  nimac, nivlan, &bmac));
    
    return s;
}

int
soc_sbx_g2p3_bmac_next_ext(int unit, soc_sbx_g2p3_6_byte_t imac, int ivlan, 
                           soc_sbx_g2p3_6_byte_t nimac, int *nivlan)
{
    int s, bmac;


    for(s = soc_sbx_g2p3_mac_next(unit, imac, ivlan, 1, nimac, nivlan, &bmac);
        s == SOC_E_NONE && !bmac;
        s = soc_sbx_g2p3_mac_next(unit,
                                  nimac, *nivlan, bmac,
                                  nimac, nivlan, &bmac));
    
    return s;
}
