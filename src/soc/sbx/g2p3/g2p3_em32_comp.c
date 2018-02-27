/******************************************************************************
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
 *
 * g2p3_em32_comp.c: EM32 complex compiler support functions for utg.pl
 *
 * $Id: g2p3_em32_comp.c 1.5.60.1 Broadcom SDK $
 *
 * --------------------------------------------------------------------------*/

#include <soc/sbx/g2p3/g2p3_int.h>

#include <soc/sbx/g2p3/g2p3_em32.h>
#include <soc/sbx/g2p3/g2p3_em32_comp.h>

/*
 * Currenly only support a 16-bit/16-bit key split
 * This means 16 bits of key in the table, and 16 bits of key used for
 * table addressing (128K word total table bank size)
 */
   
#define G2P3_EM32_ADDRESS_BITS 16

#define G2P3_EM32_SLAB_SIZE 0x2000


int
soc_sbx_g2p3_em32_recover_ext(int unit, void* cstate) 
{
    soc_sbx_g2p3_em32_comp_t *em32c =
        (soc_sbx_g2p3_em32_comp_t*)cstate;
    
    return  soc_sbx_translate_status(g2p3_em32Recover(em32c->em32));
}

int
soc_sbx_g2p3_em32_key_valid(uint32 key) 
{  
    /* The ucode requires an invalid KEY per EM32 instance.  Since the current
     * EM32 instance contains 4 tables, consider the pvv2e table where keys
     * resolve to ports > 50; all entries from all other tables are valid.
     *
     * This is extremely brittle and dependent on the key layout of a pvv2e
     * entry.  The check should be incorporated higher up for more flexibility.
     */
    return (key & 0xc0000000) || (((key & 0x0003f000) >> 12) < 51);
}

int
soc_sbx_g2p3_em32_init_ext(int unit, void **cstate, char *name,
                           int ncbanks,
                           soc_sbx_g2p3_complex_bank_desc_t *cbanks)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (!ncbanks || !cbanks[0].size) {
        return SOC_E_NONE;
    }

    if (ncbanks != 6) {
        return SOC_E_PARAM;
    }

    /*
     * Currently only accept a very limted layout:
     * + 6 complex banks (3 left & 3 right)
     * + bank 0 & 3: 32 bits of payload
     * + bank 1 & 4: 16 bits of payload
     * + bank 2 & 5: 16 bits of key in the same memory bank as banks 1 & 4
     */
    if (cbanks[0].type != SOC_SBX_G2P3_PAYLOAD_COMPLEX_BANK_TYPE) {
        return SOC_E_PARAM;
    }
    if (cbanks[0].pos != 0) {
        return SOC_E_PARAM;
    }
    if (cbanks[0].width != 32) {
        return SOC_E_PARAM;
    }
    if (cbanks[0].size != 2 * (1 << G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[1].type != SOC_SBX_G2P3_PAYLOAD_COMPLEX_BANK_TYPE) {
        return SOC_E_PARAM;
    }
    if (cbanks[1].pos != 32) {
        return SOC_E_PARAM;
    }
    if (cbanks[1].width != (32 - G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[1].size != 2 * (1 << G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[0].memory == cbanks[1].memory
        && cbanks[0].base == cbanks[1].base) {
        return SOC_E_PARAM;
    }
    if (cbanks[2].type != SOC_SBX_G2P3_KEY_COMPLEX_BANK_TYPE) {
        return SOC_E_PARAM;
    }
    if (cbanks[2].pos != 0) {
        return SOC_E_PARAM;
    }
    if (cbanks[2].width != G2P3_EM32_ADDRESS_BITS) {
        return SOC_E_PARAM;
    }
    if (cbanks[2].size != 2 * (1 << G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[1].memory != cbanks[2].memory
        && cbanks[1].base != cbanks[2].base) {
        return SOC_E_PARAM;
    }
    if (cbanks[3].type != SOC_SBX_G2P3_PAYLOAD_COMPLEX_BANK_TYPE) {
        return SOC_E_PARAM;
    }
    if (cbanks[3].pos != 0) {
        return SOC_E_PARAM;
    }
    if (cbanks[3].width != 32) {
        return SOC_E_PARAM;
    }
    if (cbanks[3].size != 2 * (1 << G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if ((cbanks[0].memory == cbanks[3].memory
         && cbanks[0].base == cbanks[3].base)
        || (cbanks[1].memory == cbanks[3].memory
            && cbanks[1].base == cbanks[3].base)) {
        return SOC_E_PARAM;
    }
    if (cbanks[4].type != SOC_SBX_G2P3_PAYLOAD_COMPLEX_BANK_TYPE) {
        return SOC_E_PARAM;
    }
    if (cbanks[4].pos != 32) {
        return SOC_E_PARAM;
    }
    if (cbanks[4].width != (32 - G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[4].size != 2 * (1 << G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[3].memory == cbanks[4].memory
        && cbanks[3].base == cbanks[4].base) {
        return SOC_E_PARAM;
    }
    if ((cbanks[0].memory == cbanks[4].memory
         && cbanks[0].base == cbanks[4].base)
        || (cbanks[1].memory == cbanks[4].memory
            && cbanks[1].base == cbanks[4].base)) {
        return SOC_E_PARAM;
    }
    if (cbanks[5].type != SOC_SBX_G2P3_KEY_COMPLEX_BANK_TYPE) {
        return SOC_E_PARAM;
    }
    if (cbanks[5].pos != 0) {
        return SOC_E_PARAM;
    }
    if (cbanks[5].width != (32 - G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[5].size != 2 * (1 << G2P3_EM32_ADDRESS_BITS)) {
        return SOC_E_PARAM;
    }
    if (cbanks[4].memory != cbanks[5].memory
        && cbanks[4].base != cbanks[5].base) {
        return SOC_E_PARAM;
    }

    em32c = sal_alloc(sizeof(*em32c), "g2p3 em32c");
    if (!em32c) {
        return SOC_E_MEMORY;
    }

    em32c->unit = unit;
    em32c->name = name;

    s = g2p3_em32Init(&em32c->em32, fe, fe->pDmaMgr, &fe->cParams,
                      soc_sbx_g2p3_em32_key_valid,
                      cbanks[1].base, cbanks[4].base,
                      cbanks[0].base, cbanks[3].base,
                      cbanks[1].memory, cbanks[4].memory,
                      cbanks[0].memory, cbanks[3].memory,
                      1 << G2P3_EM32_ADDRESS_BITS,
                      1 << G2P3_EM32_ADDRESS_BITS,
                      (cbanks[0].width + cbanks[1].width + cbanks[2].width) /8,
                      0x2000, G2P3_EM32_ADDRESS_BITS,
                      0, 0, 0, 0, 0);
    if (s == SB_OK) {
#ifdef BCM_SBX_MPLSTP_SUPPORT
        if (strcmp(name, "outerlabelcuckoo")==0) 
            g2p3_em32InitCuckooSeed(em32c->em32,
                                    soc_sbx_g2p3_labelcuckoo_abseed_set,
                                    soc_sbx_g2p3_labelcuckoo_abseed_get);
        else 
#endif        
            g2p3_em32InitCuckooSeed(em32c->em32,
                                    soc_sbx_g2p3_cuckoo_abseed_set,
                                    soc_sbx_g2p3_cuckoo_abseed_get);

        s = g2p3_em32HWInitStart(em32c->em32, NULL, 0);
    }

    if (s == SB_OK) {
        *cstate = em32c;
    } else {
        sal_free(em32c);
    }

   return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_em32_uninit_ext(int unit, void *cstate)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;

    s = g2p3_em32Uninit(em32c->em32);
    sal_free(em32c);

    return soc_sbx_translate_status(s);
}


int
soc_sbx_g2p3_em32_add_ext(int unit, void *cstate, uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;
    uint32_t k = *(uint32_t *) ikey;

    s = sbG2P3AddEM32(em32c->em32, k, e);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_em32_renew_ext(int unit, void *cstate, uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;
    uint32_t k = *(uint32_t *) ikey;

    s = sbG2P3UpdateEM32(em32c->em32, k, e);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_em32_get_ext(int unit, void *cstate, uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;
    uint32_t k = *(uint32_t *) ikey;

    s = sbG2P3GetEM32(em32c->em32, k, e);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_em32_remove_ext(int unit, void *cstate, uint8 *ikey)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;
    uint32_t k = *(uint32_t *) ikey;

    s = sbG2P3RemEM32(em32c->em32, k);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_em32_commit_ext(int unit, void *cstate, int runlength)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;

    unit = unit;
    runlength = runlength;

    s = g2p3_em32CommitStart(em32c->em32);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_em32_first_ext(int unit, void *cstate, uint8 *ikey)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;

    s = sbG2P3FirstKey(em32c->em32, (uint32_t *) ikey);
    return soc_sbx_translate_status(s);
}

int 
soc_sbx_g2p3_em32_next_ext(int unit, void *cstate, uint8 *ikey, uint8 *nikey)
{
    sbStatus_t s;
    soc_sbx_g2p3_em32_comp_t *em32c = (soc_sbx_g2p3_em32_comp_t *) cstate;
    uint32_t k = *(uint32_t *) ikey;

    s = sbG2P3NextKey(em32c->em32, k, (uint32_t *) nikey);
    return soc_sbx_translate_status(s);
}
