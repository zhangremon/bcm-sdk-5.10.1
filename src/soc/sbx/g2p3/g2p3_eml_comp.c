/*
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
 * $Id: g2p3_eml_comp.c 1.17.112.1 Broadcom SDK $
 */

#include <assert.h>
#include <soc/sbx/sbx_drv.h>
#include <sal/core/libc.h>
#include <sal/appl/sal.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#include <soc/sbx/fe2k_common/sbEML.h>
#include <soc/sbx/fe2k_common/sbPaylMgr.h>
#include <soc/sbx/g2p3/g2p3_eml_comp.h>


int
soc_sbx_g2p3_eml_recover_ext(int unit, void* cstate) 
{
    soc_sbx_g2p3_eml_comp_t *emlc =
        (soc_sbx_g2p3_eml_comp_t*)cstate;
    
    return  soc_sbx_translate_status(sbEMLCompRecover(emlc->eml));
}

uint32_t
g2p3_eml_l2_size_get(sbEMLCtx_p cstate, uint32_t n)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;

    return n * ((emlc->ksize + emlc->psize) / 4);
}

void
g2p3_eml_key_get(sbEMLCtx_p cstate, sbEMLPackedKeyPayload_p_t pkp,
                 sbEMLPackedKey_t k)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    int pad = 4 - (emlc->ksize % 4);

    if (pad == 4) pad = 0;
    sal_memcpy(k, pkp, emlc->ksize);
    sal_memset(((uint8 *) k) + emlc->ksize, 0, pad);
}

void
g2p3_eml_key_pack(sbEMLCtx_p cstate, sbEMLKey_p_t k, sbEMLPackedKey_t pk)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    int pad = 4 - (emlc->ksize % 4);

    if (pad == 4) pad = 0;
    sal_memcpy(pk, k, emlc->ksize);
    sal_memset(((uint8 *) pk) + emlc->ksize, 0, pad);
}

void
g2p3_eml_key_unpack(sbEMLCtx_p cstate, sbEMLKey_p_t k, sbEMLPackedKey_t pk)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;

    sal_memcpy(k, pk, emlc->ksize);
}

void
g2p3_eml_key_payload_pack(sbEMLCtx_p cstate,
                          sbEMLKey_p_t k, sbEMLPayload_p_t p,
                          sbEMLPackedKeyPayload_p_t pkp)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    int kbytes = emlc->ksize;

    sal_memcpy(pkp, k, kbytes);
    sal_memcpy(((uint8_t *) pkp) + kbytes, p, emlc->psize);
}

void
g2p3_eml_key_payload_update(sbEMLCtx_p cstate,
                            sbEMLPackedKeyPayload_p_t pkp,
                            sbEMLKey_p_t k, sbEMLPayload_p_t p)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    int kbytes = emlc->ksize;

    /*
     * I'm not clear if the key is already in pkp, but previous implementations
     * built pkp in its entirety, so we do the same.
     */
    sal_memcpy(pkp, k, kbytes);
    sal_memcpy(((uint8_t *) pkp) + kbytes, p, emlc->psize);
}

sbFe2000DmaSlab_t *
g2p3_eml_slab_entry_write_format(sbEMLCtx_p cstate,
                                 sbPHandle_t ph,
                                 sbEMLTable2_t *p,
                                 sbFe2000DmaSlab_t *slab)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    soc_sbx_g2p3_eml_entry_t *e = (soc_sbx_g2p3_eml_entry_t *) p;
    soc_sbx_g2p3_eml_comp_bank_t *b;
    soc_sbx_g2p3_eml_comp_bank_chunk_t *c;
    int i, j;
    uint32_t addr, offset;

    offset = addr = sbPayloadGetAddr(emlc->pmgr, ph) ; 

    for (i = 0; i < emlc->nbanks; i++) {
        b = &emlc->banks[i];
        if (!sbPayloadAligned(emlc->pmgr)) {
            /* relative addressing, make it direct */
            addr = offset + b->base;
        }
        slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
        slab->feAddress = 
            SB_FE2000_DMA_MAKE_ADDRESS(b->memory,
                                       addr + p->slotNo * b->stride);
        sal_memset(slab->data, 0, b->esize);
        for (j = 0; j < b->nchunks; j++) {
            c = &b->chunks[j];
            sal_memcpy(((uint8_t *) slab->data) + c->boffset,
                       ((uint8_t *) e->payload) + c->offset,
                       c->size);
        }
        slab->words = b->esize / 4;
        slab = SB_FE2000_SLAB_NEXT(slab);
    }

    return slab;
}

sbFe2000DmaSlab_t *
g2p3_eml_slab_entry_read_format(sbEMLCtx_p cstate,
                                sbPHandle_t ph,
                                sbEMLTable2_t *p,
                                sbFe2000DmaSlab_t *slab)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    soc_sbx_g2p3_eml_comp_bank_t *b;
    int i;
    uint32_t addr, offset;

    offset = addr = sbPayloadGetAddr(emlc->pmgr, ph); 

    for (i = 0; i < emlc->nbanks; i++) {
        b = &emlc->banks[i];
        if (!sbPayloadAligned(emlc->pmgr)) {
           /* Relative addressing used, make it direct */
           addr = offset + b->base;
        }
        slab->opcode = SB_FE2000_DMA_OPCODE_READ;
        slab->feAddress = 
            SB_FE2000_DMA_MAKE_ADDRESS(b->memory,
                                       addr + p->slotNo * b->stride);

        slab->words = b->esize / 4;
        slab = SB_FE2000_SLAB_NEXT(slab);
    }

    return slab;
}

static void
g2p3_eml_slab_entry_extract(sbEMLCtx_p cstate,
                            sbEMLPackedKeyPayload_p_t pkp,
                            sbFe2000DmaSlab_t *slab,
                            uint slotNo)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    uint8_t *p = (uint8_t *) pkp;
    soc_sbx_g2p3_eml_comp_bank_t *b;
    soc_sbx_g2p3_eml_comp_bank_chunk_t *c;
    int i, j;

    for (i = 0; i < emlc->nbanks; i++) {
        b = &emlc->banks[i];
        for (j = 0; j < b->nchunks; j++) {
            c = &b->chunks[j];
            sal_memcpy(p + c->offset,
                       ((uint8_t *) slab->data) + c->boffset,
                       c->size);
        }
        slab = SB_FE2000_SLAB_NEXT(slab);
    }
}     

sbFe2000DmaSlab_t *
g2p3_eml_slab_l2_table_format(sbEMLCtx_p cstate,
                              sbPHandle_t ph,
                              uint32_t usemap,
                              sbEMLTable2_t *entries,
                              sbFe2000DmaSlab_t *slab)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    sbEMLTable2_t *e;
    soc_sbx_g2p3_eml_entry_t *p;
    soc_sbx_g2p3_eml_comp_bank_t *b;
    soc_sbx_g2p3_eml_comp_bank_chunk_t *c;
    uint8_t *dps[SOC_SBX_G2P3_EML_MAX_BANKS];
    int i, j, npay, off;
    uint32_t addr, offset;

    offset = addr = sbPayloadGetAddr(emlc->pmgr, ph) ; 

    for (npay = 0, e = entries; e; npay++, e = e->next);

    for (i = 0; i < emlc->nbanks; i++) {
        b = &emlc->banks[i];
        if (!sbPayloadAligned(emlc->pmgr)) {
            /* Payload address are relative, make it direct */
            addr = offset + b->base;
        }
        slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
        slab->feAddress = SB_FE2000_DMA_MAKE_ADDRESS(b->memory, addr);
        slab->words = (b->esize / 4) * npay;
        dps[i] = (uint8_t *) slab->data;
        slab = SB_FE2000_SLAB_NEXT(slab);
    }

    for (p = (soc_sbx_g2p3_eml_entry_t *) entries;
         p;
         p = (soc_sbx_g2p3_eml_entry_t *) p->next) {
        for (i = 0; i < emlc->nbanks; i++) {
            b = &emlc->banks[i];
            off = b->esize * p->slotNo;
            if (emlc->agebank == i) {
                p->client1 = addr + emlc->agedmaoff + 
                    (off / (emlc->agedmawords * 4));
            }
            sal_memset(dps[i] + off, 0, b->esize);
            for (j = 0; j < b->nchunks; j++) {
                c = &b->chunks[j];
                sal_memcpy(dps[i] + off + c->boffset,
                           ((uint8_t *) p->payload) + c->offset,
                           c->size);
            }
        }
    }

    return slab;
}

static uint32
g2p3_eml_l1_table_addr_get(sbEMLCtx_p cstate, int u)
{
  soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
  return SB_FE2000_DMA_MAKE_ADDRESS (emlc->l1memory, (u + emlc->l1base)); 
}

static void
g2p3_eml_l1_table_entry_parse(sbEMLCtx_p cstate, uint32 *s, 
                              uint32 *p, uint32 *m, uint8 *b)
{
  soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
  soc_sbx_g2p3_emll1_t l1;

  soc_sbx_g2p3_emll1_unpack(emlc->unit, &l1, b, 8);
  *s = l1.salt;
  *m = l1.usemap;
  *p = l1.pointer;
}


static sbFe2000DmaSlab_t *
g2p3_eml_l1_table_format(sbEMLCtx_p cstate,
                         uint32_t i,
                         uint32_t seed,
                         sbPHandle_t ph,
                         uint32_t usemap,
                         sbFe2000DmaSlab_t *slab)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    uint32_t addr;
    soc_sbx_g2p3_emll1_t l1;

    addr = sbPayloadGetAddr(emlc->pmgr, ph) ; 
    l1.usemap = usemap;
    l1.salt = seed;
    /* For unaligned EML tables, Payload address is relative.
     * in this case ucode must use base + index addressing. 
     */
    l1.pointer = addr;
    soc_sbx_g2p3_emll1_pack(emlc->unit, &l1, (uint8 *) slab->data, 8);
    slab->words = 2;
    slab->feAddress = SB_FE2000_DMA_MAKE_ADDRESS (emlc->l1memory,
                                                  (i + emlc->l1base));
    slab->opcode = SB_FE2000_DMA_OPCODE_WRITE;
    slab = SB_FE2000_SLAB_NEXT(slab);

    return slab;
}

int
soc_sbx_g2p3_eml_init_ext(int unit,
                          void **cstate, char *name, int ncbanks,
                          soc_sbx_g2p3_complex_bank_desc_t *cbanks,
                          int saltwidth, int agepos, int agewidth,
                          int dontagepos, int align)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc;
    int i, j, serr, esize, kwidth, pwidth, mwsize, totalchunks;
    int nchunks[SOC_SBX_G2P3_EML_MAX_BANKS];
    int cbankpos;
    uint32 v;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbEMLInit_t initL2Struct;
    soc_sbx_g2p3_eml_comp_bank_t *b;
    soc_sbx_g2p3_eml_comp_bank_chunk_t *c;
    sbPayMgrInit_t initPayMac;
    soc_sbx_g2p3_memory_params_t mp;

    if (!ncbanks || !cbanks[0].size) {
        return SOC_E_NONE;
    }

    emlc = sal_alloc(sizeof(*emlc), "g2p3 emlc");
    if (!emlc) {
        return SOC_E_MEMORY;
    }

    sal_memset(emlc, 0, sizeof(*emlc));
    emlc->unit = unit;
    emlc->name = name;
    emlc->agewidth = agewidth;
    emlc->agepos = -1;
    emlc->dontagepos = -1;
    v = 0x1;
    emlc->behost = !*((uint8 *) &v);

    if (1 << cbanks[0].width != cbanks[0].size) {
        soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
        return SOC_E_INTERNAL;
    }
    emlc->l1memory = cbanks[0].memory;
    emlc->l1base = cbanks[0].base;

    /*
     * Currently, the EML compiler infrastructure has quite
     * severe restrictions on what it supports:
     * o Must have a 64-bit L1 memory region (cbanks[0])
     * o Must have 2-3 L2 memory regions with the same base address
     *   (therefore, in different memories)
     * o Memory regions can contain key, payload, or both
     * o Key and payload positions are all byte multiples
     *
     * The actual restrictions currently seem even more complicated
     * and hard to describe (e.g. if the table has 2 banks, the bank member
     * sizes can be free, but if the bank has 3 banks, the bank member
     * sizes seem to be restricted to 1 word).  It seems like the only
     * internal dependency on host memory layout is in the sbEMLComp.c
     * initSRAM() function.
     *
     * Bet if we cleaned that up, we could support sets of banks of
     * arbitrary shapes.  the triBank flag is a good clue (along
     * with payloadMgr) to find the offending locations.
     * If this were clean, the unit of payloadMgr management could be
     * an entire payload (rather than this whacky #banks * payload
     * granularity situation).  Most of the compiler relies on the client
     * (i.e. this code) to formulate FE addresses, and to pack payloads
     * into the appropriate banks, etc, in memory.
     */
    kwidth = 0;
    pwidth = 0;
    totalchunks = 0;
    sal_memset(nchunks, 0, sizeof(nchunks));
    for (i = 1; i < ncbanks; i++) {
        if (cbanks[i].width % 8 || cbanks[i].pos % 8) {
            /* Can lift the byte-aligned chunk restriction eventually */
            soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
            return SOC_E_PARAM;
        }
        b = NULL;
        for (j = 0; j < emlc->nbanks; j++) {
            if (cbanks[i].memory == emlc->banks[j].memory
                && cbanks[i].base == emlc->banks[j].base) {
                b = &emlc->banks[j];
                break;
            }
            if (cbanks[i].memory == emlc->banks[j].memory) {
                /* Can lift the 1 bank per memory restriction eventually */
                soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
                return SOC_E_PARAM;
            }
        }
        if (!b) {
            if (j >= SOC_SBX_G2P3_EML_MAX_BANKS) {
                soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
                return SOC_E_PARAM;
            }
            b = &emlc->banks[j];
            b->memory = cbanks[i].memory;
            b->base = cbanks[i].base;
            b->size = cbanks[i].size;
            b->nchunks = 0;
            b->esize = 0;
            emlc->nbanks++;
        }
        switch (cbanks[i].type) {
        case SOC_SBX_G2P3_PAYLOAD_COMPLEX_BANK_TYPE:
            pwidth += cbanks[i].width;
            break;
        case SOC_SBX_G2P3_KEY_COMPLEX_BANK_TYPE:
            kwidth += cbanks[i].width;
            break;
        case SOC_SBX_G2P3_SPACER_COMPLEX_BANK_TYPE:
            continue;
        }
        nchunks[j]++;
        totalchunks++;
    }

    emlc->chunks = sal_alloc(sizeof(soc_sbx_g2p3_eml_comp_bank_chunk_t) 
                              * totalchunks, "g2p3 eml chunks");
    if (!emlc->chunks) {
        soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
        return SOC_E_MEMORY;
    }

    j = 0;
    for (i = 0; i < emlc->nbanks; i++) {
        b = &emlc->banks[i];
        b->chunks = &emlc->chunks[j];
        j += nchunks[i];
    }

    emlc->ksize = kwidth / 8;
    emlc->psize = pwidth / 8;
    for (i = 1; i < ncbanks; i++) {
        b = NULL;
        for (j = 0; j < emlc->nbanks; j++) {
            if (cbanks[i].memory == emlc->banks[j].memory
                && cbanks[i].base == emlc->banks[j].base) {
                b = &emlc->banks[j];
                break;
            }
        }
        assert(b);
        if (cbanks[i].type == SOC_SBX_G2P3_SPACER_COMPLEX_BANK_TYPE) {
            b->esize += cbanks[i].width / 8;
            continue;
        }
        c = &b->chunks[b->nchunks];
        c->size = cbanks[i].width / 8;
        c->boffset = b->esize;
        b->esize += c->size;
        c->offset = (cbanks[i].pos 
                     + (cbanks[i].type == SOC_SBX_G2P3_KEY_COMPLEX_BANK_TYPE
                        ? 0 : kwidth)) / 8;
        if (cbanks[i].type == SOC_SBX_G2P3_PAYLOAD_COMPLEX_BANK_TYPE
            && cbanks[i].pos <= agepos
            && agepos < cbanks[i].pos + cbanks[i].width) {
            assert(emlc->agepos == -1);
            emlc->agebank = j;
            serr = soc_sbx_g2p3_memory_params_get(unit, b->memory, &mp);
            if (serr) {
                soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
                return serr;
            }
            emlc->agedmawords = mp.width / 32;
            cbankpos = agepos - cbanks[i].pos;
            emlc->agedmaoff = (cbankpos + c->boffset * 8) / mp.width;
            emlc->agepos = (cbankpos + c->boffset * 8) % mp.width;
            if (cbanks[i].pos + cbanks[i].width <= agepos + agewidth
                || mp.width <= emlc->agepos + agewidth) {
                /*
                 * Age must fall completely within a single chunk
                 * and must fit within a single memory word
                 */
                soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
                return SOC_E_PARAM;
            }
            if (0 <= dontagepos) {
                cbankpos = dontagepos - cbanks[i].pos;
                if (dontagepos < cbanks[i].pos
                    || ((cbankpos + c->boffset * 8) / mp.width
                        != emlc->agedmaoff)) {
                    /* dontage & age must be in the same memory word */
                    soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
                    return SOC_E_PARAM;
                }
                emlc->dontagepos = (cbankpos + c->boffset * 8) % mp.width;
            }
        }
        b->nchunks++;
    }

    for (i = 0; i < emlc->nbanks; i++) {
        b = &emlc->banks[i];
        serr = soc_sbx_g2p3_memory_params_get(unit, b->memory, &mp);
        if (serr) {
            soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
            return serr;
        }
        mwsize = mp.width / 8;
        b->esize = ((b->esize + (mwsize - 1)) / mwsize) * mwsize;
        b->stride = b->esize / mwsize;
    }

    if (emlc->nbanks < 2) {
        soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
        return SOC_E_PARAM;
    }

    esize = ((emlc->ksize + emlc->psize + 3) / 4) * 4;
    emlc->esize = SOC_SBX_G2P3_EML_ENTRY_SIZE(esize);
    emlc->getbuf = sal_alloc(esize, "g2p3 eml getbuf");
    if (!emlc->getbuf) {
        soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
        return SOC_E_MEMORY;
    }

    if (0 <= agepos) {
        emlc->agebuf = soc_cm_salloc(unit, 8, "age dma");
        if (!emlc->agebuf) {
            soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
            return SOC_E_MEMORY;
        }
    }

    emlc->maxent = emlc->banks[0].size / emlc->banks[0].stride;

    initL2Struct.ctx = emlc;
    initL2Struct.cParams = &fe->cParams;
    initL2Struct.maxDirty = 1024; /* TODO: what is this? */
    initL2Struct.table1_count = cbanks[0].size;
    initL2Struct.slab_size = 64 * 1024;
    initL2Struct.key_size = kwidth;
    initL2Struct.seed_bits = saltwidth;
    initL2Struct.ipv6 = 0;
    initL2Struct.new_hash = 1;
    initL2Struct.table2_max = emlc->maxent;
    initL2Struct.table2_size = emlc->esize;
    initL2Struct.payload_size = esize;
    initL2Struct.getKey = g2p3_eml_key_get;
    initL2Struct.packKey = g2p3_eml_key_pack;
    initL2Struct.unPackKey = g2p3_eml_key_unpack;
    initL2Struct.packKeyPayload = g2p3_eml_key_payload_pack;
    initL2Struct.updatePacked = g2p3_eml_key_payload_update;
    initL2Struct.table2size = g2p3_eml_l2_size_get;
    initL2Struct.slabTable2entry = g2p3_eml_slab_entry_write_format;
    initL2Struct.slabGetTable2entry = g2p3_eml_slab_entry_read_format;
    initL2Struct.unslabTable2Entry = g2p3_eml_slab_entry_extract;
    initL2Struct.slabTable2 = g2p3_eml_slab_l2_table_format;
    initL2Struct.slabSetTable1ptr = g2p3_eml_l1_table_format;
    initL2Struct.table1EntryParse = g2p3_eml_l1_table_entry_parse;
    initL2Struct.table1Addr = g2p3_eml_l1_table_addr_get;
    initL2Struct.commitDone = NULL;
    initL2Struct.updateDone = NULL;
    initL2Struct.getDone = NULL;
    initL2Struct.updClsId = NULL;
    initL2Struct.nbank = emlc->nbanks;
    initL2Struct.sharedFreeList = NULL;

    /* Payload address are relative if align is not set.
     * In that case, Use the bank base to get actual address */
    initPayMac.aligned = align;
    if (align) {
        initPayMac.sramStart = emlc->banks[0].base; 
    } else {
        initPayMac.sramStart = 0;
    }
    initPayMac.sramSize = emlc->banks[0].size; 
    initPayMac.nbank = emlc->nbanks;
    initPayMac.payloadMax = emlc->maxent; 
    /* table1bits unused when payloadMax set */
    initPayMac.sbmalloc = fe->cParams.sbmalloc;
    initPayMac.sbfree = fe->cParams.sbfree;
    initPayMac.clientData = fe->cParams.clientData;

    s = sbPayloadMgrInit(&emlc->pmgr, &initPayMac);
    if (s) {
        soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
        return soc_sbx_translate_status(s);
    }
        
    s = sbEMLCompInit(&emlc->eml, &initL2Struct,
                      &fe->cParams, emlc->pmgr, fe->pDmaMgr);
    if (s) {
        soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
        return soc_sbx_translate_status(s);
    }

    s = sbEMLCompInitHW(emlc->eml, NULL, NULL);
    if (s) {
        soc_sbx_g2p3_eml_uninit_ext(unit, emlc);
        return soc_sbx_translate_status(s);
    }

    *cstate = emlc;

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_eml_uninit_ext(int unit, void *cstate)
{
    sbStatus_t s = SB_OK, s0;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;

    if (emlc->eml) {
        s = sbEMLCompUninit(emlc->eml);
    }
    if (emlc->pmgr) {
        s0 = sbPayloadMgrUninit(&emlc->pmgr);
        if (!s) s = s0;
    }
    if (emlc->agebuf) {
        soc_cm_sfree(unit, emlc->agebuf);
    }
    if (emlc->getbuf) {
        sal_free(emlc->getbuf);
    }
    if (emlc->chunks) {
        sal_free(emlc->chunks);
    }
    sal_free(emlc);
    
    return soc_sbx_translate_status(s);
}

int 
soc_sbx_g2p3_eml_add_ext(int unit, void *cstate, uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    soc_sbx_g2p3_eml_entry_t *t2;
    sbEMLCollisions_t cols;

    if (!emlc) {
        return SOC_E_FAIL;
    }
    cols.entries = 1;

    s = sbEMLAdd(emlc->eml, ikey, e, &cols, (sbEMLTable2_t **) &t2);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_eml_update_ext(int unit, void *cstate, uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    soc_sbx_g2p3_eml_entry_t *t2;

    if (!emlc) {
        return SOC_E_FAIL;
    }
    s = sbEMLUpdate(emlc->eml, ikey, e, (sbEMLTable2_t **) &t2);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_eml_get_ext(int unit, void *cstate, uint8 *ikey, void *e)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;

    if (!emlc) {
        return SOC_E_FAIL;
    }
    s = sbEMLGet(emlc->eml, ikey,
                 (sbEMLPackedKeyPayload_p_t) emlc->getbuf, NULL);
    if (s == SB_OK) {
        sal_memcpy(e, emlc->getbuf + emlc->ksize, emlc->psize);
    }

    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_eml_remove_ext(int unit, void *cstate, uint8 *ikey)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;

    if (!emlc) {
        return SOC_E_FAIL;
    }
    s = sbEMLDel(emlc->eml, ikey);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_eml_commit_ext(int unit, void *cstate, int runlength)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    uint32_t rl = runlength;

    if (!emlc) {
        return SOC_E_FAIL;
    }
    s = sbEMLCommit(emlc->eml, &rl);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_eml_first_ext(int unit, void *cstate, uint8 *ikey)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;

    if (!emlc) {
        return SOC_E_FAIL;
    }
    s = sbEMLIteratorFirst(emlc->eml, ikey);
    return soc_sbx_translate_status(s);
}

int
soc_sbx_g2p3_eml_next_ext(int unit, void *cstate, uint8 *ikey, uint8 *nikey)
{
    sbStatus_t s;
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;

    if (!emlc) {
        return SOC_E_FAIL;
    }
    s = sbEMLIteratorNext(emlc->eml, ikey, nikey);
    return soc_sbx_translate_status(s);
}

/*
 * Age scanner
 */
int
soc_sbx_g2p3_eml_run_aging(int unit, void *cstate,
                           int now, int old, uint32 ecount,
                           soc_sbx_g2p3_eml_old_f_t oldcb)
{
    soc_sbx_g2p3_eml_comp_t *emlc = (soc_sbx_g2p3_eml_comp_t *) cstate;
    soc_sbx_g2p3_eml_entry_t *e;
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbFe2000DmaOp_t op;
    sbStatus_t s;
    uint32 v, dontagemask, agemask;
    int memory, ageword, dontageword, ageshift, age;
    int be;

    if(! emlc) {
        return SOC_E_INTERNAL;
    }
    e = sbEMLTable2Array(emlc->eml);
    be = emlc->behost;

    if (emlc->agepos < 0) {
        return SOC_E_INTERNAL;
    }

    if (emlc->agenext >= emlc->maxent) {
        emlc->agenext = 0;
    }

    op.opcode = SB_FE2000_DMA_OPCODE_READ;
    op.words = emlc->agedmawords;
    op.hostAddress = emlc->agebuf;
    op.data = NULL;
    op.bForcePio = 0;
    memory = emlc->banks[emlc->agebank].memory;

    ageword = emlc->agepos / 32;
    ageshift = emlc->agepos % 32;
    agemask = (1 << emlc->agewidth) - 1;
    dontageword = emlc->dontagepos / 32;
    dontagemask = 0;
    if (0 <= emlc->dontagepos) {
        dontagemask = 1 << (emlc->dontagepos % 32);
        if (be) dontagemask = _shr_swap32(dontagemask);
    }

    for (e = (soc_sbx_g2p3_eml_entry_t *)
             (((char *) e) + emlc->agenext * emlc->esize);
         ecount && emlc->agenext < emlc->maxent;
         emlc->agenext++, 
         e = (soc_sbx_g2p3_eml_entry_t *) (((char *) e) + emlc->esize)) {
        if (e->valid) {
            op.feAddress = SB_FE2000_DMA_MAKE_ADDRESS(memory, e->client1);
            s = sbFe2000DmaRequest(fe->pDmaMgr, INT_TO_PTR(unit), &op);
            if (s) {
                return SOC_E_FAIL;
            }
            ecount--;
            if (!(dontagemask & emlc->agebuf[dontageword])) {
                v = emlc->agebuf[ageword];
                if (be) v = _shr_swap32(v);
                age = (v >> ageshift) & agemask;
                if (old <= ((now - age) & agemask)) {
                    (oldcb)(unit, (void *) e->payload);
                }
            }
        }
    }

    return emlc->agenext < emlc->maxent ? SOC_E_NONE : SOC_E_NOT_FOUND;
}
