/*
 * $Id: g2p3_eml_comp.h 1.4 Broadcom SDK $
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
 */

#ifndef _SOC_SBX_G2P3_EML_COMP_H
#define _SOC_SBX_G2P3_EML_COMP_H

#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/fe2k_common/sbEML.h>

typedef struct {
    SB_EML_TABLE2_BASE;
    uint32_t payload[0];
} soc_sbx_g2p3_eml_entry_t;

#define SOC_SBX_G2P3_EML_ENTRY_SIZE(b) \
    (sizeof(soc_sbx_g2p3_eml_entry_t) + ((((b) + 3) / 4) * 4))

typedef struct soc_sbx_g2p3_eml_comp_bank_chunk_s {
    int size;
    int offset;
    int boffset;
} soc_sbx_g2p3_eml_comp_bank_chunk_t;

typedef struct soc_sbx_g2p3_eml_comp_bank_s {
    int memory;
    int base;
    int size;
    int esize;
    int stride;
    int nchunks;
    soc_sbx_g2p3_eml_comp_bank_chunk_t *chunks;
} soc_sbx_g2p3_eml_comp_bank_t;

#define SOC_SBX_G2P3_EML_MAX_BANKS SOC_SBX_EML_MAX_BANKS

typedef struct soc_sbx_g2p3_eml_comp_s {
    int unit;
    char *name;
    sbEMLCtx_p eml;
    sbPaylMgrCtxt_t pmgr;
    int l1memory;
    uint32_t l1base;
    int behost;
    int ksize;
    int psize;
    int esize;
    int agebank;
    int agedmawords;
    int agedmaoff;
    int agepos;
    int agewidth;
    int dontagepos;
    int nbanks;
    int maxent;
    int agenext;
    uint32 *agebuf;
    soc_sbx_g2p3_eml_comp_bank_t banks[SOC_SBX_G2P3_EML_MAX_BANKS];
    soc_sbx_g2p3_eml_comp_bank_chunk_t *chunks;
    uint8_t *getbuf;
} soc_sbx_g2p3_eml_comp_t;
#endif /* BCM_SBX_SUPPORT */
#endif
