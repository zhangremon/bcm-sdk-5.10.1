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
 * $Id: g2p3_lpm_comp.c 1.14.42.1 Broadcom SDK $
 */

#include <assert.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#include <soc/sbx/g2p3/g2p3_lpm_comp.h>
#include <bcm_int/sbx/state.h>

/*
 * Silly work-around for the fact that the pack function doesn't
 * get called with any sort of token.  So we manage a pool of
 * unique pack functions that get assigned to each new lpm instance
 */
#define MAX_PACKS 8
soc_sbx_g2p3_complex_pack_f_t soc_sbx_g2p3_lpm_packs[MAX_PACKS] =
    {0, 0, 0, 0, 0, 0, 0, 0};
int soc_sbx_g2p3_lpm_pack_sizes[MAX_PACKS];

sbStatus_t
soc_sbx_g2p3_lpm_pack0(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[0])(unit, payload, dest,
                                    soc_sbx_g2p3_lpm_pack_sizes[0]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

sbStatus_t
soc_sbx_g2p3_lpm_pack1(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[1])(unit, payload, dest,
                                soc_sbx_g2p3_lpm_pack_sizes[1]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

sbStatus_t
soc_sbx_g2p3_lpm_pack2(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[2])(unit, payload, dest,
                                soc_sbx_g2p3_lpm_pack_sizes[2]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

sbStatus_t
soc_sbx_g2p3_lpm_pack3(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[3])(unit, payload, dest,
                                soc_sbx_g2p3_lpm_pack_sizes[3]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

sbStatus_t
soc_sbx_g2p3_lpm_pack4(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[4])(unit, payload, dest,
                                soc_sbx_g2p3_lpm_pack_sizes[4]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

sbStatus_t
soc_sbx_g2p3_lpm_pack5(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[5])(unit, payload, dest,
                                soc_sbx_g2p3_lpm_pack_sizes[5]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

sbStatus_t
soc_sbx_g2p3_lpm_pack6(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[6])(unit, payload, dest,
                                soc_sbx_g2p3_lpm_pack_sizes[6]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

sbStatus_t
soc_sbx_g2p3_lpm_pack7(void *dest, uint16_t *destExtra, void *payload,
                       g2p3_lpmClassifierId_t clsA,
                       g2p3_lpmClassifierId_t clsB,
                       uint32_t dropMaskLo, uint32_t dropMaskHi, uint32_t unit)
{
    int s;

    s = (soc_sbx_g2p3_lpm_packs[7])(unit, payload, dest,
                                soc_sbx_g2p3_lpm_pack_sizes[7]);
    *destExtra = clsA;
    return s ? SB_OTHER_ERR_CODE : SB_OK;
}

#define LPM_MAX_SLICES 16
#define LPM_MAX_PAYLOAD_BANKS 2

int
soc_sbx_g2p3_lpm_init_ext(int unit,
                          void **state, char *name, int ncbanks,
                          soc_sbx_g2p3_complex_bank_desc_t *cbanks, int esize,
                          soc_sbx_g2p3_complex_pack_f_t pack, void *dentry)
{
    sbStatus_t s;
    int i, singleton, hbytes, allocSize;
    int npbanks, pbank0, pbank0odd, pwidth;
    int slabsize;
    int sram_factor;
    soc_sbx_g2p3_lpm_comp_t *lpmc;
    uint8 slices[LPM_MAX_SLICES + 1];
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t (*ipack)(void *dest, uint16_t *destExtra, void *payload,
                        g2p3_lpmClassifierId_t clsA,
                        g2p3_lpmClassifierId_t clsB,
                        uint32_t dropMaskLo, uint32_t dropMaskHi,
                        uint32_t unit);

    *state = NULL;

    /*
     * Current LPM infrastructure makes some assumptions about table layout:
     *    Slices (including the top) alternate banks
     *    Payload resides in 1 or 2 banks that are the same as the slice banks
     *    Payloads words % # payload banks == 0
     *    Multibank payloads alternate words in each bank
     *    Multibank payloads have the same # of words in each bank
     *    When singleton is enabled, it must be for all slices <= 7 bits
     *    Only 32 bits of each bank is used
     */

    /* This function assumes:
     *    Slice banks come before payload banks
     *    Slice banks are sorted by bit position and are contiguous
     */
    if (ncbanks < 4) {
        return SOC_E_PARAM;
    }

    if (!cbanks[0].size) {
        return SOC_E_NONE;
    }

    singleton = 0;
    npbanks = 0;
    pwidth = 0;
    pbank0odd = 0;
    for (i = 0; i < ncbanks; i++) {
        switch (cbanks[i].type) {
        case SOC_SBX_G2P3_PAYLOAD_COMPLEX_BANK_TYPE:
            if (!npbanks) {
                slices[i] = 0;
                pbank0 = i;
                pbank0odd =
                    cbanks[i].memory == cbanks[1].memory
                    && cbanks[i].base == cbanks[1].base
                    && cbanks[i].size == cbanks[1].size;
            }
            if (npbanks == LPM_MAX_PAYLOAD_BANKS) {
                return SOC_E_PARAM;
            }
            if ((cbanks[i].memory != cbanks[(npbanks + pbank0odd) % 2].memory)
                || (cbanks[i].base != cbanks[(npbanks + pbank0odd) % 2].base)
                || (cbanks[i].size != cbanks[(npbanks + pbank0odd)% 2].size)) {
                return SOC_E_PARAM;
            }
            pwidth += cbanks[i].width;
            npbanks++;
            break;
        case SOC_SBX_G2P3_KEY_COMPLEX_BANK_TYPE:
            if (npbanks) {
                return SOC_E_INTERNAL;
            }
            if (!i 
                && (cbanks[i].memory != cbanks[2].memory
                    || cbanks[i].size < 1 << cbanks[i].width)) {
                return SOC_E_PARAM;
            }
            if (i 
                && ((cbanks[i].memory != cbanks[2 - i % 2].memory)
                    || (cbanks[i].base != cbanks[2 - i % 2].base)
                    || (cbanks[i].size != cbanks[2 - i % 2].size))) {
                return SOC_E_PARAM;
            }
            slices[i] = cbanks[i].width;
            singleton |= cbanks[i].singleton;
            break;
        default:
            return SOC_E_PARAM;
        }
    }
    if ((npbanks == 0) || (npbanks >= LPM_MAX_PAYLOAD_BANKS)) {
        return SOC_E_PARAM;
    }

    for (i = 0; cbanks[i].type == SOC_SBX_G2P3_KEY_COMPLEX_BANK_TYPE; i++) {
        if (singleton && !cbanks[i].singleton && slices[i] <= MAX_SKIP_LEN) {
            return SOC_E_PARAM;
        }
    }

    lpmc = sal_alloc(sizeof(soc_sbx_g2p3_lpm_comp_t), "g2p3 lpm");
    if (!lpmc) {
        return SOC_E_MEMORY;
    }
    sal_memset(lpmc, 0, sizeof(*lpmc));
    *state = lpmc;
    lpmc->name = name;

    allocSize = sizeof(g2p3_lpmDeviceMemHandle_t) * LPM_MAX_PAYLOAD_BANKS;
    lpmc->deviceMem = sal_alloc(allocSize, "g2p3 lpm deviceMem");
    if (!lpmc->deviceMem) {
        return SOC_E_MEMORY;
    }

    lpmc->dentry = sal_alloc(esize, "g2p3 lpm dentry");
    if (!lpmc->dentry) {
        return SOC_E_MEMORY;
    }
    sal_memcpy(lpmc->dentry, dentry, esize);

    ipack = NULL;
    for (i = 0; !ipack && i < MAX_PACKS; i++) {
        if (!soc_sbx_g2p3_lpm_packs[i]) {
            soc_sbx_g2p3_lpm_packs[i] = pack;
            soc_sbx_g2p3_lpm_pack_sizes[i] = pwidth / 8;
            lpmc->packindex = i;
            switch (i) {
            case 0: ipack = soc_sbx_g2p3_lpm_pack0; break;
            case 1: ipack = soc_sbx_g2p3_lpm_pack1; break;
            case 2: ipack = soc_sbx_g2p3_lpm_pack2; break;
            case 3: ipack = soc_sbx_g2p3_lpm_pack3; break;
            case 4: ipack = soc_sbx_g2p3_lpm_pack4; break;
            case 5: ipack = soc_sbx_g2p3_lpm_pack5; break;
            case 6: ipack = soc_sbx_g2p3_lpm_pack6; break;
            case 7: ipack = soc_sbx_g2p3_lpm_pack7; break;
            }
        }
    }

    if (!ipack) {
        return SOC_E_INTERNAL;
    }

    lpmc->esize = esize;

    /*
     * The host memory breaks down heuristically to:
     * 32 * memory size         (for prefixes)
     * + 8 * memory size / 8    (4 * estimated max payloads)
     * + 4 * 2 * cbanks[1].size (total L2 device memory)
     */
    sram_factor = soc_property_get(unit, 
                          spn_L3_LPM_HOST_SRAM_FACTOR, 32);
    sram_factor = (sram_factor < 32) ? 32 : sram_factor;

    hbytes = sram_factor * cbanks[1].size + 4 * cbanks[1].size / 8;

    s = g2p3_lpmHostMemInit(&lpmc->hostMem, hbytes,
                            fe->cParams.sbmalloc, fe->cParams.clientData,
                            fe->cParams.sbfree, fe->cParams.clientData);

    for (i = 0; s == SB_OK && i < LPM_MAX_PAYLOAD_BANKS; i++) {
        s = g2p3_lpmDeviceMemInit(&lpmc->deviceMem[i], lpmc->hostMem,
                                  cbanks[2 - i].base, cbanks[2 - i].size,
                                  NULL, FALSE);
    }
    lpmc->deviceMems = i;

    if (s == SB_OK) {
        slabsize = 64 * cbanks[1].size / 1024 * 1024;
        if (slabsize < 4 * 1024) {
            slabsize = 4096;
        }
        if (slabsize > 64 * 1024) {
            slabsize = 64 * 1024;
        }
        s = g2p3_lpmDmaInit(&lpmc->dma, lpmc->hostMem,
                            fe->cParams.sbmalloc, fe->cParams.clientData,
                            fe->cParams.sbfree, fe->cParams.clientData,
                            fe->pDmaMgr, fe, fe->cParams.clientData,
                            16, slabsize);
    }

    if (s == SB_OK) {
        s = g2p3_lpmInit(fe, &lpmc->lpm,
                         lpmc->hostMem,
                         lpmc->deviceMem[0], lpmc->deviceMem[1], 
                         lpmc->dma,
                         cbanks[0].memory, cbanks[1].memory,
                         cbanks[1 - pbank0odd].memory,
                         cbanks[pbank0odd].memory,
                         npbanks,
                         cbanks[0].base,
                         slices,
                         lpmc->dentry,
                         0, 0, 0, 0,
                         esize,
                         pwidth / 32,
                         ipack, singleton, FALSE);
    }

    if (s == SB_OK) {
        s = g2p3_lpmAdd(lpmc->lpm, 0, 0,
                        dentry, G2P3_LPM_PAYLOAD_MAIN, 1);
    }

    if (s == SB_OK) {
        s = g2p3_lpmCommit(lpmc->lpm, NULL, NULL);
    }

    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_uninit_ext(int unit, void *cstate)
{
    sbStatus_t s = SB_OK, s0;
    int i;
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;

    if (lpmc->lpm) {
        s = g2p3_lpmUninit(&lpmc->lpm);
    }
    if (lpmc->dma) {
        s0 = g2p3_lpmDmaUninit(&lpmc->dma);
        if (s == SB_OK) s = s0;
    }
    if (lpmc->deviceMems) {
        for (i = 0; i < lpmc->deviceMems; i++) {
            if (lpmc->deviceMem[i]) {
                s0 = g2p3_lpmDeviceMemUninit(&lpmc->deviceMem[i]);
                if (s == SB_OK) s = s0;
            }
        }
        sal_free(lpmc->deviceMem);
    }
    if (lpmc->hostMem) {
        s0 = g2p3_lpmHostMemUninit(&lpmc->hostMem);
        if (s == SB_OK) s = s0;
    }

    soc_sbx_g2p3_lpm_packs[lpmc->packindex] = NULL;
    sal_free(lpmc->dentry);
    sal_free(lpmc);

    return soc_sbx_translate_status(s);
}

int 
soc_sbx_g2p3_lpm_add_ext(int unit, void *cstate, int iprefixbits, uint8 *ikey,
                         void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;
    g2p3_lpmAddress_t a = *(g2p3_lpmAddress_t *) ikey;

    if (!lpmc) {
        return SOC_E_FAIL;
    }
    if (iprefixbits == 0) {
        return SOC_E_PARAM;
    }
    s = g2p3_lpmAdd(lpmc->lpm, a, iprefixbits, 
                    e, G2P3_LPM_PAYLOAD_MAIN,
                    SOC_SBX_STATE(unit)->cache_l3route);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_renew_ext(int unit, void *cstate, int iprefixbits,
                           uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;

    if (!lpmc) {
        return SOC_E_FAIL;
    }

    s = g2p3_lpmUpdate(lpmc->lpm, *(g2p3_lpmAddress_t *) ikey, iprefixbits, 
                       e, G2P3_LPM_PAYLOAD_MAIN,
                       SOC_SBX_STATE(unit)->cache_l3route);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_get_ext(int unit, void *cstate, int iprefixbits,
                             uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;
    void *e0;

    if (!lpmc) {
        return SOC_E_FAIL;
    }

    s = g2p3_lpmFind(lpmc->lpm, &e0, *(g2p3_lpmAddress_t *) ikey, iprefixbits,
                     G2P3_LPM_PAYLOAD_MAIN);
    if (s == SB_OK) {
        sal_memcpy(e, e0, lpmc->esize);
    }
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_remove_ext(int unit, void *cstate, int iprefixbits,
                            uint8 *ikey)
{
    sbStatus_t s;
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;

    if (!lpmc) {
        return SOC_E_FAIL;
    }
    if (iprefixbits == 0) {
        return SOC_E_PARAM;
    }
    s = g2p3_lpmRemove(lpmc->lpm, *(g2p3_lpmAddress_t *) ikey, iprefixbits,
                       G2P3_LPM_PAYLOAD_MAIN,
                       SOC_SBX_STATE(unit)->cache_l3route);

    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_commit_ext(int unit, void *cstate, int runlength)
{
    sbStatus_t s;
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;
    
    if (!lpmc) {
        return SOC_E_FAIL;
    }
    s = g2p3_lpmCommit(lpmc->lpm, NULL, NULL);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_first_ext(int unit, void *cstate, int *iprefixbits,
                           uint8 *ikey)
{
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;
    uint8 len;
    void *p;
    g2p3_lpmPayloadKind_t pk;
    uint32 dml, dmh;
    g2p3_lpmClassifierId_t ca, cb, cc;
    sbStatus_t s;

    if (!lpmc) {
        return SOC_E_FAIL;
    }
    s = g2p3_lpmWalkReset(lpmc->lpm, &lpmc->iterator);
    if (s == SB_OK) {
        s = g2p3_lpmWalk(lpmc->lpm, 0, 0, (g2p3_lpmAddress_t *) ikey, &len, 
                         &pk, &p, &dml, &dmh, &ca, &cb, &cc, lpmc->iterator);
        *iprefixbits = len;
    }

    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_next_ext(int unit, void *cstate, int iprefixbits, uint8 *ikey,
                          int *niprefixbits, uint8 *nikey)
{
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;
    g2p3_lpmAddress_t a = *(g2p3_lpmAddress_t *) ikey;
    uint8 len;
    void *p;
    g2p3_lpmPayloadKind_t pk;
    uint32 dml, dmh;
    g2p3_lpmClassifierId_t ca, cb, cc;
    sbStatus_t s;

    if (!lpmc) {
        return SOC_E_FAIL;
    }
    len = iprefixbits;
    s = g2p3_lpmWalk(lpmc->lpm, a, len, (g2p3_lpmAddress_t *) nikey, &len,
                     &pk, &p, &dml, &dmh, &ca, &cb, &cc, lpmc->iterator);

    *niprefixbits = len;
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_lpm_stats_clear(int unit, void *cstate)
{
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;
    
    if (!lpmc) {
        return SOC_E_NOT_FOUND;
    }
    g2p3_lpm_stats_clear(lpmc->lpm);

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_lpm_stats_dump(int unit, void *cstate, int verbose)
{
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;
    
    if (!lpmc) {
        return SOC_E_NOT_FOUND;
    }

    soc_cm_print("\n ++++++++++++++++++++++++++++++++++");
    soc_cm_print("\n           %s Statistics ", lpmc->name);
    soc_cm_print("\n ++++++++++++++++++++++++++++++++++");
    g2p3_lpm_stats_dump(lpmc->lpm, verbose);

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_lpm_singleton_set(int unit, void *cstate, int flag)
{
    soc_sbx_g2p3_lpm_comp_t *lpmc = (soc_sbx_g2p3_lpm_comp_t *) cstate;

    if (!lpmc) {
        return SOC_E_NOT_FOUND;
    }

    g2p3_lpmSingletonSet(lpmc->lpm, flag);

    return SOC_E_NONE;
}
