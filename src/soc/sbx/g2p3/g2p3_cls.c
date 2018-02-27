/*
 * $Id: g2p3_cls.c 1.137 Broadcom SDK $
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

#include <soc/sbx/g2p3/g2p3_int.h>
#include <soc/sbx/g2p3/g2p3_cls_int.h>
#include <soc/sbx/fe2k/sbFe2000Driver.h>
#include <soc/sbx/fe2k/sbFe2000ClsComp.h>
#include <soc/sbx/fe2k/sbFe2000Driver.h>
#include <soc/sbx/fe2k/sbZfFe2000RcSbPattern.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcResultResolverEntry.hx>
#include <soc/sbx/fe2kxt/sbFe2000XtImfDriver.h>
#include <shared/mem_avl.h>

#define DYNAMIC_MEMMGR
#define PUBLIC
#define PRIVATE static

#define MAX_CLS_INSTR_SIZE      (400) /* in reality we can find it on fly */
#define MASK_N(X)               ( ((X) == 32) ? (0xFFFFFFFF) : ( (1 << (X))  -1) )
#define CLS_OP_WIDTH(x)         (x->uStart-x->uEnd+1)
#define CLS_OP_ENCODE_RESUME(x)        ( (x)++ )
#define CLS_OP_DECODE_RESUME(x)        ( (x)++ )

#define SB_G2P3_MAX_G2_PORTS  32

#define CLS_OP_DONTCARE  (0xFFFF)

/* FOR DEBUG int sb_log_verbose = 3 or whatever */

/*#define RULESET_DEBUG 1*/

struct soc_sbx_g2p3_cls_wb_s{
    uint32_t uRcInstance;
    uint32_t uPhyProg;
    uint32_t uLogProg;
    uint8_t  uActive;
    uint32_t uLength;
    uint32_t uBaseAddress;
    sbDmaMemoryHandle_t dmaHandle;
    uint32_t *dmaBuffer;
};
typedef struct soc_sbx_g2p3_cls_wb_s  soc_sbx_g2p3_cls_wb_t;

extern sbStatus_t
soc_sbx_g2p3_cls_init_c2(soc_sbx_g2p3_state_t *pFe,
                  sbFe2000DmaMgr_t  *pDmaCtxt,
                  sbCommonConfigParams_p_t    cParams,
                  sbIngressConfigParams_t*    iParams,
                  void *pWbData);

extern sbStatus_t 
soc_sbx_g2p3_ace_wb_rule_recover(soc_sbx_g2p3_state_t *pFe,
                                       soc_sbx_g2p3_cls_schema_t *pSchema,
                                       uint32_t uGroupId,
                                       uint32_t uRule,
                                       soc_sbx_g2p3_cls_ace_pattern_t *pPattern);
extern sbStatus_t
soc_sbx_g2p3_cls_wb_init_c2(soc_sbx_g2p3_state_t *pFe, void **wbOut);

extern sbStatus_t
soc_sbx_g2p3_cls_wb_recover_c2(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_cls_schema_t *pSchema);

extern sbStatus_t
soc_sbx_g2p3_cls_wb_uninit_c2(soc_sbx_g2p3_state_t *pFe);

#ifdef CLS_PAT_DEBUG

#define PAT_DEBUG_ARRAY_SIZE 14 /* 448 /32 */

uint32_t patDebugArray[PAT_DEBUG_ARRAY_SIZE];

void patDebugArrayZero()
{
  uint32_t i;

  for(i=0; i < PAT_DEBUG_ARRAY_SIZE; i++) {
    patDebugArray[i] = 0;
  }
}

void patDebugArrayPrint()
{
  uint32_t i, loEnd, hiEnd, maskBit;
  int mask;

  for(i=0; i < PAT_DEBUG_ARRAY_SIZE; i++) {
    loEnd = i*32;
    hiEnd = loEnd + 31;
    SB_LOG( "Patt %3d : %3d = 0x%08x Bits set : ", hiEnd, loEnd, patDebugArray[i] );

    for(mask=31; mask >= 0; mask--) {
        maskBit = 1 << mask;
        if( maskBit & patDebugArray[i] ) {
          SB_LOG("%d ", (i*32)+mask );
        }
    }
    SB_LOG("\n");

  }
}

void patDebugSetBit(uint32_t offset)
{
  uint32_t index, bitpos;

  if(offset > 447) {
      SB_LOG("Index %d out of range\n", offset);
      return;
  }

  index = offset/32;
  bitpos = offset % 32;

  patDebugArray[index] |= 1 << bitpos;

}

#endif

uint8_t sbG2FePortArray[SB_G2P3_CLS_MAX_PORTS] =
{ 49,48,47,46,45,44,43,42,41,40\
 ,39,38,37,36,35,34,33,32,31,30\
 ,29,28,27,26,25,24,23,22,21,20\
 ,19,18,17,16,15,14,13,12,11,10\
 , 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

/*
 *  * fix me: chnage the field start and end to width specification
 *   */
static sbFe2000ClsTemplateField_t soc_sbx_g2p3_cls_g2p3_template[] =
{
    { "PortMask      ", SB_FE2K_CLS_OP_MULTIPORT, 253, 248, 50, &sbG2FePortArray[0] },
    { "PRI           ", SB_FE2K_CLS_OP_WEQ      ,  63,  61,  0, NULL },
    { "Fragment      ", SB_FE2K_CLS_OP_WEQ      , 246, 246,  0, NULL },
    { "TCP URG       ", SB_FE2K_CLS_OP_WEQ      , 245, 245,  0, NULL },
    { "TCP ACK       ", SB_FE2K_CLS_OP_WEQ      , 244, 244,  0, NULL },
    { "TCP PSH       ", SB_FE2K_CLS_OP_WEQ      , 243, 243,  0, NULL },
    { "TCP RST       ", SB_FE2K_CLS_OP_WEQ      , 242, 242,  0, NULL },
    { "TCP SYN       ", SB_FE2K_CLS_OP_WEQ      , 241, 241,  0, NULL },
    { "TCP FIN       ", SB_FE2K_CLS_OP_WEQ      , 240, 240,  0, NULL },
    { "DSCP          ", SB_FE2K_CLS_OP_WEQ      , 239, 234,  0, NULL },
    { "ECN           ", SB_FE2K_CLS_OP_WEQ      , 233, 232,  0, NULL },
    { "Protocol      ", SB_FE2K_CLS_OP_WEQ      , 231, 224,  0, NULL },
    { "LLC DSAP SSAP ", SB_FE2K_CLS_OP_RANGE    , 223, 208,  0, NULL },
    { "LLC CTRL DATA ", SB_FE2K_CLS_OP_RANGE    , 207, 192,  0, NULL },
    { "IPV4 SA       ", SB_FE2K_CLS_OP_LPM      , 191, 160,  0, NULL },
    { "IPV4 DA       ", SB_FE2K_CLS_OP_LPM      , 159, 128,  0, NULL },
    { "EtherType     ", SB_FE2K_CLS_OP_WEQ      , 127, 112,  0, NULL },
    { "SMAC_47_0     ", SB_FE2K_CLS_OP_LPM      , 111,  64,  0, NULL },
    { "VSI           ", SB_FE2K_CLS_OP_WEQ      ,  59,  48,  0, NULL },
    { "DMAC_47_0     ", SB_FE2K_CLS_OP_LPM      ,  47,   0,  0, NULL },
};




typedef enum {
  SB_FE2K_CLS_G2P3_SCHEMA_OP_PORT          =  0,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_PRI           =  1,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_FRAGMENT      =  2,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_URG_FLAG  =  3,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_ACK_FLAG  =  4,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_PSH_FLAG  =  5,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_RST_FLAG  =  6,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_SYN_FLAG  =  7,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_FIN_FLAG  =  8,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DSCP     =  9,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_ECN      = 10,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_PROTOCOL = 11,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_DSAPSSAP  = 12,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_CTRLDATA  = 13,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_SA       = 14,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DA       = 15,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_ETH_ETYPE     = 16,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_SMAC_47_0     = 17,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_VSI           = 18,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_DMAC_47_0     = 19,
  /* DIX/SNAP TCP ports over layed on LLC */
  SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_SRCPORT    = SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_DSAPSSAP,
  SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_DSTPORT    = SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_CTRLDATA
} soc_sbx_g2p3_cls_g2p3_schema_enum_t;


/*----------------------------------------------------------------------------
 * Forward declare module-local types
 *----------------------------------------------------------------------------*/
PRIVATE
sbStatus_t soc_sbx_g2p3_cls_schema_init(soc_sbx_g2p3_state_t *pFe, uint32_t isC2);

PRIVATE
void soc_sbx_g2p3_cls_g2p3_schema_encoder(soc_sbx_g2p3_cls_schema_p_t pSchema,
                                uint32_t                   uRIndex,
                                void * pRule,
                                soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory);
PRIVATE
void soc_sbx_g2p3_cls_g2p3_schema_decoder(soc_sbx_g2p3_cls_schema_p_t pSchema,
                                uint32_t                   uRIndex,
                                void * pRule,
                                sbZfFe2000RcSbPattern_t *pPatternMemory[],
                                uint32_t *isNull);

sbStatus_t soc_sbx_g2p3_cls_allocate_dma_slabs( soc_sbx_g2p3_state_t *pFe,
                                          sbCommonConfigParams_p_t    cParams,
                                          uint32_t uMaxSlabSize,
                                          uint32_t isC2);

PRIVATE
sbStatus_t soc_sbx_g2p3_cls_de_allocate_dma_slabs( soc_sbx_g2p3_state_t *pFe );

void soc_sbx_g2p3_cls_dma_cb(sbFe2000DmaOp_t  *pOp);

/* Rule Emit operations */
PRIVATE
void EmitWildCardEQ(sbZfFe2000RcSbPattern_t *pPattern,
                    uint32_t *puOffset,
                    uint32_t uRuleId,
                    int32_t  uMsb,
                    int32_t  uLsb,
                    uint32_t isC2);
PRIVATE
void EmitMatchLpm(sbZfFe2000RcSbPattern_t *pPattern,
                  uint32_t *puOffset,
                  uint32_t  uRuleId,
                  uint32_t uMatchLength,
                  uint32_t uDontCareLength,
                  soc_sbx_g2p3_bit_vector_t *pVec);
PRIVATE
void DecodeMatchLpm(sbZfFe2000RcSbPattern_t *pPattern,
             uint32_t *puOffset,
             uint32_t  uRuleId,
             uint32_t *uMatchLength,
             uint32_t uWidth,
             soc_sbx_g2p3_bit_vector_t *pVector);
#if 0
PRIVATE
void EmitExactMatchEQ(sbZfFe2000RcSbPattern_t *pPattern,
                       uint32_t *puOffset,
                       uint32_t uRuleId,
                       uint32_t  uSrcStart,
                       uint32_t uWidth,
                       soc_sbx_g2p3_bit_vector_t *pVec);
#endif
PRIVATE
void EmitExactMatch32(sbZfFe2000RcSbPattern_t *pPattern,
                      uint32_t *puOffset,
                      uint32_t uRuleId,
                      uint32_t uMsb,
                      uint32_t uLsb,
                      uint32_t uValue,
                      uint32_t isC2);

PRIVATE
void EmitPbmp(sbZfFe2000RcSbPattern_t *pPattern,
              uint32_t *puOffset,
              uint32_t uRuleId,
              uint64_t uuPbmp,
              uint32_t uWidth);

PRIVATE
void EmitBits32(sbZfFe2000RcSbPattern_t *pPattern,
                uint32_t *puOffset,
                uint32_t uRuleId,
                int32_t  uMsb,
                int32_t  uLsb,
                uint32_t uValue );
void
EmitClearBits(sbZfFe2000RcSbPattern_t *pPattern,
               uint32_t *puOffset,
               uint32_t uRuleId,
               int32_t  uMsb,
               int32_t  uLsb );
void
EmitFromVector(sbZfFe2000RcSbPattern_t *pPattern,
                 uint32_t *puOffset,
                 uint32_t uRuleId,
                 uint32_t  uSrcStart,
                 uint32_t uWidth,
                 soc_sbx_g2p3_bit_vector_t *pVector);

PRIVATE
uint32_t getBit(soc_sbx_g2p3_bit_vector_t *v, uint32_t uBitid);

PRIVATE
uint32_t getBitFromWord(uint32_t v, uint32_t uBitid);
PRIVATE
uint32_t getLpmBit(soc_sbx_g2p3_bit_vector_t *v, uint32_t uBit, uint32_t uLpmLenght);
PRIVATE
void setLpmBit(soc_sbx_g2p3_bit_vector_t *v, uint32_t uBit, uint32_t uLpmLength, uint8_t uBitValue);
PRIVATE
void MakeMacArrayFromUint64(uint64_t uMac, uint8_t *pArray);
PRIVATE
void MakeUint64FromMacArray(uint8_t *pArray, uint64_t *uMac);

PRIVATE
void DecodePbmp(sbZfFe2000RcSbPattern_t *pPattern,
              uint32_t *puOffset,
              uint32_t uRuleId,
              uint64_t *uuPbmp,
              uint8_t  *bPortAny,
              uint32_t uWidth);
PRIVATE
void DecodeWildCardEQOrExactMatch(sbZfFe2000RcSbPattern_t *pPattern,
               uint32_t *puOffset,
               uint32_t uRuleId,
               int32_t  uMsb,
               int32_t  uLsb,
               uint32_t *uFieldValue,
               uint8_t  *bFieldAny);
PRIVATE
void DecodeBits32(sbZfFe2000RcSbPattern_t *pPattern,
            uint32_t *puOffset,
            uint32_t uRuleId,
            int32_t  uMsb,
            int32_t  uLsb,
            uint32_t *uValue );

/* forward declarations */
int
soc_sbx_g2p3_cls_schema_to_logprogram(soc_sbx_g2p3_state_t *pFe,
                                      soc_sbx_g2p3_cls_schema_enum_t sType,
                                      uint8_t *pLogProgNumber);
sbStatus_t
soc_sbx_g2p3_cls_wb_recover(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_cls_schema_t *pSchema)
{
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_wb_t *wbData;
    uint32_t uNumRs, uRs, uSb, uNumBlocks, uBlock, uBlockOffset, uId, uRule, uRuleOffset;
    sbZfFe2000RcSbPattern_t *pPatternMemoryDecode[SB_FE2000_RC_NUM_SUPERBLOCKS];
    uint32_t uBufSize;
    sbDmaMemoryHandle_t junk = {0};
    sbZfFe2000RcDmaFormat_t zDmaFrameDecode;
    sbFe2000RcDmaFormatAlias_t   zAliasDecode;
    sbZfFe2000RcSbPattern_t *pPatternDecode;
    sbZfFe2000RcSbPatternAlias_t zPatternAliasDecode;
    uint32_t *pDmaBuf;
    uint32_t isNull;
    soc_sbx_g2p3_cls_ace_pattern_t aceRule;
    sbStatus_t err;


    if (SAND_HAL_IS_FE2KXT(pFe->regSet)) {
        return soc_sbx_g2p3_cls_wb_recover_c2(pFe, pSchema);
    }

    pClsComp = pFe->pClsComp;

    /* allocate pattern memory for a ruleset */
    uBufSize = (pSchema->uInstCount)*sizeof(sbZfFe2000RcSbPattern_t);
    for (uSb=0; uSb < 2; uSb++){
        sbStatus_t err = pClsComp->sbmalloc(pClsComp->clientData,
                                                SB_ALLOC_INTERNAL,
                                                uBufSize,
                                                (void*)&pPatternMemoryDecode[uSb], NULL);
        if (err != SB_OK) {
            SB_ASSERT(0);
            return err;
        }
    }

    uNumBlocks = pSchema->uInstCount / 4;

    /* use sType to select correct WB data structure */
    switch (pSchema->sType) {
    case SB_G2P3_FE_CLS_G2P3_SCHEMA:
        wbData = (soc_sbx_g2p3_cls_wb_t*)(pClsComp->wbData);
        break;
    case SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA:
        wbData = (soc_sbx_g2p3_cls_wb_t*)(pClsComp->wbData);
        wbData = &(wbData[1]);
        break;
    default:
        SB_ASSERT(0);
        return SOC_E_INTERNAL;
    }
    pDmaBuf = wbData->dmaBuffer;
    err = soc_sbx_g2p3_cls_get_ctr_offset(pFe, wbData->uPhyProg, &uRuleOffset);

    /*
     * for each program (ingress, egress)
     *   for each ruleset
     *      decode ruleset
     *         for each rule
     *            invoke ACE call back to create rule
     */
    uNumRs = wbData[0].uLength / pSchema->uInstCount;

    uBlockOffset = 0;
    for (uRs=0; uRs<uNumRs; uRs++) {
        for (uBlock=0; uBlock<uNumBlocks; uBlock++){
            /* unpack from DMA buffer into zDmaFrame and alias */
            sbZfFe2000RcDmaFormat_InitInstance(&zDmaFrameDecode);
            sbZfFe2000RcDmaFormatAlias(&zDmaFrameDecode, &zAliasDecode);
            sbZfFe2000RcDmaFormat_Unpack(&zDmaFrameDecode, (uint8_t*)&pDmaBuf[uBlockOffset], SB_ZF_FE2000RCDMAFORMAT_SIZE_IN_BYTES);
            /* ignore instructions in DmaFrame */
            for (uId=0; uId<32; uId++) {
              pPatternDecode = pPatternMemoryDecode[uId / 16];
              sbZfFe2000RcSbPatternAlias(&pPatternDecode[(uBlock*4)+(uId/4)%4], &zPatternAliasDecode);
               *(zPatternAliasDecode.pPattern[uId%4])= *(zAliasDecode.pPattern[uId]);
            }
            uBlockOffset += 64;
        }
        /* decode rules */
        isNull = 0;
        for (uRule=0; uRule<G2P3_MAX_CLS_RULES_PER_RULESET; uRule++) {
            soc_sbx_g2p3_cls_ace_pattern_init(&aceRule);
            pSchema->pDecoder(pSchema,
                              uRule,
                              &aceRule,
                              pPatternMemoryDecode,
                              &isNull);
            if (isNull) {
                break;
            }

            /* rule is in hardware so its enabled */
            aceRule.bEnabled = 1;
            err = soc_sbx_g2p3_ace_wb_rule_recover(pFe, 
                                                   pSchema,
                                                   0, /* not used for CA */
                                                   uRule + uRuleOffset + (G2P3_MAX_CLS_RULES_PER_RULESET * uRs), 
                                                   &aceRule);
            if (err) {
                SB_ASSERT(0);
                return err;
            }
        }

        if (isNull) {
            break;
        }
    }

    for (uSb=0; uSb < 2; uSb++){
        err = pClsComp->sbfree(pClsComp->clientData,
                                                SB_ALLOC_INTERNAL,
                                                uBufSize,
                                                pPatternMemoryDecode[uSb], junk);
        if (err != SB_OK) {
            SB_ASSERT(0);
            return err;
        }
    }

    return SB_OK;
}

sbStatus_t
soc_sbx_g2p3_cls_wb_uninit(soc_sbx_g2p3_state_t *pFe)
{
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_wb_t *wbData;
    sbDmaMemoryHandle_t dummy = {0};
    sbStatus_t err = SB_OK;

    if (SAND_HAL_IS_FE2KXT(pFe->regSet)) {
        return soc_sbx_g2p3_cls_wb_uninit_c2(pFe);
    }

    pClsComp = pFe->pClsComp;

    /* deallocate RCE program cache */
    wbData = (soc_sbx_g2p3_cls_wb_t*)(pClsComp->wbData);
    err = thin_free(0, SB_ALLOC_CLS_DMA,
                    -1, wbData->dmaBuffer, wbData->dmaHandle);
    if (err != SB_OK){
        return SOC_E_INTERNAL;
    }
    wbData = &(wbData[1]);
    err = thin_free(0, SB_ALLOC_CLS_DMA,
                    -1, wbData->dmaBuffer, wbData->dmaHandle);
    if (err != SB_OK){
        return SOC_E_INTERNAL;
    }

    err = thin_free(0, SB_ALLOC_INTERNAL,
                         -1, (void*)pClsComp->wbData, dummy);
    if (err != SB_OK){
        return SOC_E_INTERNAL;
    }
    pClsComp->wbData = NULL;

    return SB_OK;
}
sbStatus_t
soc_sbx_g2p3_cls_wb_init(soc_sbx_g2p3_state_t *pFe, void **wbOut)
{
    uint32_t i, j, uData, uOffset;
    soc_sbx_g2p3_cls_wb_t *wbData;
    sbFe2000DmaOp_t dmaOp;
    uint32_t uNumDmaBlocks, uLength;
    sbStatus_t status;

    if (SAND_HAL_IS_FE2KXT(pFe->regSet)) {
        return soc_sbx_g2p3_cls_wb_init_c2(pFe, wbOut);
    }

    sal_memset(&dmaOp, 0, sizeof(sbFe2000DmaOp_t));
    status = thin_malloc(0, SB_ALLOC_INTERNAL,
                         sizeof(soc_sbx_g2p3_cls_wb_t) * G2P3_MAX_LOGICAL_PROGRAMS, (void*)&wbData, NULL);
    if (status != SOC_E_NONE){
        return status;
    }

    /* set returned poiner */
    *wbOut = wbData;

    /* G2P3 on CA uses RC0 for ingress program, RC1 for egress program.
     * For this reason, RC instance can be inferred from logical program.
     */
    for (i=0; i < G2P3_MAX_LOGICAL_PROGRAMS; i++) {
        SB_MEMSET(&wbData[i], 0, sizeof(soc_sbx_g2p3_cls_wb_t));
        wbData[i].uRcInstance = i;
        wbData[i].uLogProg = i;
    }

    uOffset = SAND_HAL_REG_OFFSET(CA, LU_RCE_PROGRAM0);

    for (i=0; i < G2P3_MAX_LOGICAL_PROGRAMS; i++) {
        /* get ingress active program */
        uData = SAND_HAL_FE2000_READ_OFFS(pFe->regSet, uOffset + (i * 4));
        wbData[i].uPhyProg = (uData >> (4 * i)) & 0xf;

        sbFe2000RcProgramControlGet(pFe->regSet, i, wbData[i].uPhyProg,
                                    &wbData[i].uActive,
                                    &wbData[i].uLength,
                                    &wbData[i].uBaseAddress);

        /* DMA out RC program memory */
        uNumDmaBlocks = wbData[i].uLength/4;
        status = thin_malloc(0, SB_ALLOC_CLS_DMA,
                             sizeof(sbZfFe2000RcDmaFormat_t) * uNumDmaBlocks, (void*)&wbData[i].dmaBuffer, &wbData[i].dmaHandle);
        if (status != SB_OK) {
            return status;
        }
        SB_MEMSET(wbData[i].dmaBuffer, 0, sizeof(sbZfFe2000RcDmaFormat_t) * uNumDmaBlocks);

        j=0;
        dmaOp.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
        dmaOp.feAddress   = SB_FE2000_DMA_MAKE_ADDRESS(                   \
                                (i == 0) ? SB_FE2000_MEM_CLS_0 : SB_FE2000_MEM_CLS_1, \
                                (wbData[i].uBaseAddress * 16));

        dmaOp.hostAddress = wbData[i].dmaBuffer;
        dmaOp.opcode      = SB_FE2000_DMA_OPCODE_READ;
        dmaOp.data        = pFe;
        uLength = wbData[i].uLength*16;
        while (uLength){
            dmaOp.words = 224;
            dmaOp.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
            dmaOp.cb = soc_sbx_g2p3_cls_dma_cb;
            uLength -= (uLength > 224) ? 224 : uLength;
            status = sbFe2000DmaRequest(pFe->pDmaMgr, pFe->DmaCfg.handle,
                                        &dmaOp);

            if (status != SB_OK) {
              return status;
            }
            j+=224;
            dmaOp.feAddress += 224;
            dmaOp.hostAddress += 224;
        }
    }

    return SOC_E_NONE;
}

sbStatus_t
soc_sbx_g2p3_cls_init(soc_sbx_g2p3_state_t *pFe,
                  sbFe2000DmaMgr_t  *pDmaCtxt,
                  sbCommonConfigParams_p_t    cParams,
                  sbIngressConfigParams_t*    iParams,
                  void *pWbData)
{
  void *v;
  int  index=0, ingrSchemaLen=0, egrSchemaLen=0, progNumber=0;
  sbStatus_t err;
  soc_sbx_g2p3_cls_t *pClsCompiler = NULL;
  soc_sbx_g2p3_cls_prog_attr_t *pProgAttr = NULL;
  uint8_t dummy;
  soc_sbx_g2p3_cls_wb_t *wbData = pWbData;

  if (SAND_HAL_IS_FE2KXT(pFe->regSet)) {
    return soc_sbx_g2p3_cls_init_c2(pFe, pDmaCtxt, cParams, iParams, pWbData);
  }

  /* reset all the data */
  err = cParams->sbmalloc(cParams->clientData,SB_ALLOC_INTERNAL,
                          sizeof(soc_sbx_g2p3_cls_t), &v, NULL);

  if(SB_OK != err){
      return SB_MALLOC_FAILED;
  }

  /* For CA the layout of the CRR register is:
   * 10  9  8  7  6  5  4  3  2  1  0
   * DB | set| SB| filter
   *.the upper most bit (DB) toggles between A/B banks of
   * the program   Size IRT/ERT to accomodate.
   */
  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_irt_table_size_set(pFe->unit, G2P3_MAX_CLS_RULESETS_PER_SCHEMA * G2P3_MAX_CLS_RULES_PER_RULESET * 2));
  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ert_table_size_set(pFe->unit,  G2P3_MAX_CLS_RULESETS_PER_SCHEMA * G2P3_MAX_CLS_RULES_PER_RULESET * 2));

#if 0
  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ertctr_counter_size_set(pFe->unit, 2048));
  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_irtctr_counter_size_set(pFe->unit, 2048));
#endif
  SB_MEMSET(v, 0, sizeof(soc_sbx_g2p3_cls_t));

  pFe->pClsComp = (soc_sbx_g2p3_cls_t *) v;
  pClsCompiler  = pFe->pClsComp;
  pClsCompiler->bDebug = FALSE;
  pClsCompiler->isC2 = FALSE;
  pClsCompiler->wbData =  wbData;

  /* save the handlers for later use */
  pClsCompiler->sbmalloc   = cParams->sbmalloc;
  pClsCompiler->sbfree     = cParams->sbfree;
  pClsCompiler->clientData = cParams->clientData;
  pClsCompiler->uMaxSchema = SB_G2P3_FE_CLS_MAX_NUM_SCHEMA_C1;
  soc_sbx_g2p3_cls_schema_init(pFe, pClsCompiler->isC2);

  /* initialize the dma context cookie */
  pClsCompiler->pDmaCtxt  = pDmaCtxt;

  /*
   * Each Dma Block of the RCE is 64 Words. And Each Dma Block Takes
   * 4 instructions.  so if there are n instructions there would be
   * n/4 dma blocks. And (n/4)*64 Words required to support dma of
   * of all the blocks at the same time. So total block size required
   * is (n/4)*64*4. For P programs we need P*(n/4)*64*4 bytes of memory.
   * FIX ME: This really needs to be one per db
   */

  /*
   * Given Schema layout information, the maximum slab size can be determined from
   * schema that has the max number of rule set
   */
  pClsCompiler->uMaxSlabSize = pClsCompiler->uMaxRs * (MAX_CLS_INSTR_SIZE)*16*4;

  /* create the dma slabs */
  soc_sbx_g2p3_cls_allocate_dma_slabs(pFe,cParams, pClsCompiler->uMaxSlabSize, 0);

  ingrSchemaLen = \
         pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].uInstCount;
  egrSchemaLen = \
         pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA].uInstCount;

  SB_ASSERT( ingrSchemaLen <= MAX_CLS_INSTR_SIZE);
  SB_ASSERT( egrSchemaLen <= MAX_CLS_INSTR_SIZE);

  for(index=0; index<G2P3_MAX_CLS_PROGRAMS; index++) {
      err = cParams->sbmalloc(cParams->clientData,SB_ALLOC_INTERNAL,
                      sizeof(soc_sbx_g2p3_cls_prog_attr_t), &v, NULL);

      if(SB_OK != err){
          return SB_MALLOC_FAILED;
      }
      pClsCompiler->pProgAtrib[index] = \
                        (soc_sbx_g2p3_cls_prog_attr_t*)v;
      SB_MEMSET(pClsCompiler->pProgAtrib[index], 0, sizeof(soc_sbx_g2p3_cls_prog_attr_t));
      pClsCompiler->pProgAtrib[index]->uValid = 0;
  }

  /* Allocate program attribute memory for primary programs */
  for(index=0, progNumber=G2P3_INGRESS_PROGRAM_BASE;\
      index <= 1; index++) {
      pProgAttr               = pClsCompiler->pProgAtrib[progNumber];
      pProgAttr->uNumber      = progNumber++;
      pProgAttr->uValid       = 1;
      pProgAttr->sType = SB_G2P3_FE_CLS_G2P3_SCHEMA;
      if (SOC_WARM_BOOT(pFe->unit)) {
          pProgAttr->uActive      = (wbData[0].uPhyProg == pProgAttr->uNumber);
          if (pProgAttr->uActive) {
              pProgAttr->uBaseAddress = wbData[0].uBaseAddress;
              pProgAttr->uLength = wbData[0].uLength;
          }else{
              sbFe2000RcProgramControlGet(pFe->regSet, 0, pProgAttr->uNumber, 
                                          &dummy,
                                          &pProgAttr->uLength,
                                          &pProgAttr->uBaseAddress);
          }
      }else{
          pProgAttr->uActive = (index == 0) ? 1: 0;  /* set 0 active */
          pProgAttr->uBaseAddress = index * ingrSchemaLen * \
                                    pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRs;
          pProgAttr->uBaseAddress += 32 - (pProgAttr->uBaseAddress % 32);
          pProgAttr->uLength      = ingrSchemaLen * \
                                    pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRs;
      }

  }
  if( pClsCompiler->pProgAtrib[G2P3_INGRESS_PROGRAM_BASE]->uActive ) {
    pClsCompiler->sbyProgNumber[SB_G2P3_FE_CLS_G2P3_SCHEMA] = G2P3_INGRESS_PROGRAM_BASE + 1;
  }else{
    pClsCompiler->sbyProgNumber[SB_G2P3_FE_CLS_G2P3_SCHEMA] = G2P3_INGRESS_PROGRAM_BASE;
  }
  /* Allocate memory for Egress Program and bookmark them */
  for(index=0, progNumber=G2P3_EGRESS_PROGRAM_BASE;\
      index <= 1; index++) {
      pProgAttr               = pClsCompiler->pProgAtrib[progNumber];
      pProgAttr->uNumber      = progNumber++;
      pProgAttr->uValid       = 1;
      pProgAttr->sType = SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA;

      if (SOC_WARM_BOOT(pFe->unit)) {
          pProgAttr->uActive      = (wbData[1].uPhyProg == pProgAttr->uNumber);
          if (pProgAttr->uActive) {
              pProgAttr->uBaseAddress = wbData[1].uBaseAddress;
              pProgAttr->uLength = wbData[1].uLength;
          }else{
              sbFe2000RcProgramControlGet(pFe->regSet, 1, pProgAttr->uNumber, 
                                          &dummy,
                                          &pProgAttr->uLength,
                                          &pProgAttr->uBaseAddress);
          }
      }else{
          pProgAttr->uActive = (index == 0) ? 1 : 0;
          /* odd batch number correspond to egress schema */
          pProgAttr->uBaseAddress = egrSchemaLen * index * pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRs;
          pProgAttr->uBaseAddress += 32 - (pProgAttr->uBaseAddress % 32);

          pProgAttr->uLength      = egrSchemaLen * \
                                    pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRs;
      }
  }
  if( pClsCompiler->pProgAtrib[G2P3_EGRESS_PROGRAM_BASE]->uActive ) {
    pClsCompiler->sbyProgNumber[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA] = G2P3_EGRESS_PROGRAM_BASE + 1;
  }else{
    pClsCompiler->sbyProgNumber[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA] = G2P3_EGRESS_PROGRAM_BASE;
  }

  return SB_OK;
}


sbStatus_t
soc_sbx_g2p3_cls_un_init(soc_sbx_g2p3_state_t *pFe)
{
  uint32_t uRc, uBank, i;

  if (pFe->pClsComp->pTmpCtrMem !=NULL) {
      thin_host_free(pFe->pClsComp->pTmpCtrMem);
  }
  for ( uRc = 0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
      for ( uBank = 0; uBank < 2; uBank++) {
          if (pFe->pClsComp->mem_avl_ptr[uRc][uBank] !=NULL) {
              shr_mem_avl_destroy(pFe->pClsComp->mem_avl_ptr[uRc][uBank]);
          }
      }
  }
  soc_sbx_g2p3_cls_de_allocate_dma_slabs(pFe);

  for(i=0; i<G2P3_MAX_CLS_PROGRAMS; i++) {
     thin_host_free(pFe->pClsComp->pProgAtrib[i]);
  }

  if ( SOC_IS_SBX_FE2KXT(pFe->unit) ){
      /* free C2 specific structures */
      thin_host_free(pFe->pClsComp->sRsDb);
  }

  thin_host_free(pFe->pClsComp);
  pFe->pClsComp = 0;

  return SB_OK;
}

/* @TODO push or pull arguments from ucode if required. */
int
soc_sbx_g2p3_cls_schema_layout_init(soc_sbx_g2p3_state_t *pFe, uint32_t isC2)
{
  int index;
  sbStatus_t err;
  void *v;
  uint32_t uRc, uRSet, uBankedMode = 0;
  soc_sbx_g2p3_cls_schema_t *pSchema = NULL;
  soc_sbx_g2p3_cls_ruleset_t *pRs = NULL;
  soc_sbx_g2p3_cls_t *pClsCompiler = pFe->pClsComp;
#ifdef DYNAMIC_MEMMGR
  uint32_t result;
#endif

  SB_ASSERT(pClsCompiler);
  pClsCompiler->uMaxRs = 0;
  if (isC2) {
    uBankedMode = pClsCompiler->schemadb[0].layout.uBankedMode;
    if (uBankedMode == 0) {
        pClsCompiler->uMaxRs = 21;
    } else {
        pClsCompiler->uMaxRs = 20;
    }
  }
  
  for(index=0; index < pClsCompiler->uMaxSchema; index++) {
    
    pSchema = &pClsCompiler->schemadb[index];

    if (isC2) {
      pSchema->layout.uNumRcInstance  = 2;
      pSchema->layout.uBaseRcInstance = 0;
      uBankedMode = pSchema->layout.uBankedMode;
      if (!pSchema->layout.uBankedMode) {
#ifdef DYNAMIC_MEMMGR
        pSchema->layout.uNumRs = (RCE_INST_MEM_SIZE / pSchema->uInstCount);
#else
        pSchema->layout.uNumRs = (RCE_INST_MEM_SIZE /
                                             pClsCompiler->uMaxInstCount);
#endif
        /* Counter memory map limits num of rules to 32256 rules
         * limiting number of Rulesets in each RC to 21 */
        if(pSchema->layout.uNumRs > 21) {
          pSchema->layout.uNumRs = 21;
        }
      }else{
        /* A/B banked mode.
         * Even distribution between ingress/egress
         */
        pSchema->layout.uNumRs = (RCE_INST_MEM_SIZE / 
                                pSchema->uInstCount);
        if(pSchema->layout.uNumRs > 21) {
          pSchema->layout.uNumRs = 21;
        }
        if ((pSchema->layout.uNumRs % 2) == 1) {
          pSchema->layout.uNumRs -= 1;
        }
      }
    }else{
      pSchema->layout.uBankedMode = 1;
      pSchema->layout.uNumRs = G2P3_CA_MAX_INST_PER_RC_20G / pSchema->uInstCount;
      if (pSchema->layout.uNumRs > G2P3_MAX_CLS_RULESETS_PER_SCHEMA) {
          pSchema->layout.uNumRs = G2P3_MAX_CLS_RULESETS_PER_SCHEMA;
      }
      /* RC0 dedicate ingress, RC1 dedicated egress */
      pSchema->layout.uNumRcInstance  = 1;
      pSchema->layout.uBaseRcInstance = 0;

    }
#ifdef RULESET_DEBUG
    soc_cm_print("NUMRULESETS PER RC = 0x%x\n", pSchema->layout.uNumRs);
#endif

    pSchema->layout.uNumRulesPerRs = (isC2 ? G2P3_MAX_CLS_RULES_PER_RULESET_C2 : G2P3_MAX_CLS_RULES_PER_RULESET);

    if(pSchema->layout.uNumRs > pClsCompiler->uMaxRs){
      pClsCompiler->uMaxRs = pSchema->layout.uNumRs;
    }

    /* sanity check */
    if(0 == pSchema->layout.uNumRcInstance){
      SB_ASSERT(0);
    }

    if((pSchema->layout.uNumRcInstance > G2P3_MAX_RC_INSTANCE) || \
       (pSchema->layout.uBaseRcInstance + pSchema->layout.uNumRcInstance > \
        G2P3_MAX_RC_INSTANCE)){
      SB_ASSERT(0);
    }

  }

  if (isC2) {
      /* allocate rule set data structure */
      pSchema = &pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA];
      err = pClsCompiler->sbmalloc(pClsCompiler->clientData,
                                   SB_ALLOC_INTERNAL,
                                   (sizeof(soc_sbx_g2p3_cls_ruleset_t) * pClsCompiler->uMaxRs \
                                   * pSchema->layout.uNumRcInstance),
                                   &v, NULL);
      if(err != SB_OK) {
        return err;
      }
      SB_MEMSET(v, 0, sizeof(soc_sbx_g2p3_cls_ruleset_t) * pClsCompiler->uMaxRs * pSchema->layout.uNumRcInstance);
      pClsCompiler->sRsDb = (soc_sbx_g2p3_cls_ruleset_t*)v;

      /* initialize ruleset database */
      for (uRc=0; uRc < pSchema->layout.uNumRcInstance; uRc++) {
        /* use RC0 for ingress, Rc1 for egress */
        pSchema = &pClsCompiler->schemadb[uRc];
        for (uRSet=0; uRSet < pClsCompiler->uMaxRs; uRSet++) {
          pRs = &pClsCompiler->sRsDb[uRc*pClsCompiler->uMaxRs + uRSet];
          /* number RC instances from last tofirst as rule sets will be programmed from back to front 
           (lowest priority to highest) */
          pRs->uRcInstance = G2P3_MAX_RC_INSTANCE - uRc - 1;
          pRs->uProgNumber = -1;
          pRs->uPrevRs = -1;
          pRs->uNextRs = -1;
          /* NOTE:  Assumes identical ingress/egress schema length...*/
    /*      pRs->uBaseAddress = pSchema->uInstCount * uRSet; */
#ifdef DYNAMIC_MEMMGR
          pRs->uBaseAddress = 0;
#else
          pRs->uBaseAddress = pClsCompiler->uMaxInstCount * uRSet; 
#endif
          /* NOTE: Rule offset assumes a direct mapping between rulesets and the action table. */
          pRs->uRuleOffset = pSchema->layout.uNumRulesPerRs * \
                                  (uRSet + (pRs->uRcInstance * pClsCompiler->uMaxRs));
          /* BankedMode configuration */
          pRs->uBank = 0;
          if (pSchema->layout.uBankedMode) {
            /* preallocate rulesets - RC0 for ingress (program 0), RC1 for egress (program 1) */
    /*        pRs->uRcInstance = uRc;
            pRs->uProgNumber = uRc; */
            if (uRSet >= (pClsCompiler->uMaxRs/2)) {
                pRs->uBank = 1;
            }
          }
        }
      }
  }

#ifdef DYNAMIC_MEMMGR
  if (isC2) {
      for (uRc = 0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
          /* initialize memory and Rs usage data */
          for(index = 0; index < pClsCompiler->uMaxSchema; index++) {
              pClsCompiler->usagePerSchema[index][uRc].uRsUsed = 0;
              pClsCompiler->usagePerSchema[index][uRc].uMemUsed = 0;
              pClsCompiler->usagePerSchema[index][uRc].uRsTobeUsed = 0;
              pClsCompiler->usagePerSchema[index][uRc].uMemTobeUsed = 0;
          }
          if (uBankedMode == 1) {
              pClsCompiler->usagePerRc[uRc].uMemTotal = RCE_INST_MEM_SIZE / 2;
              pClsCompiler->usagePerRc[uRc].uRsMax = pClsCompiler->uMaxRs / 2;
          } else {
              pClsCompiler->usagePerRc[uRc].uMemTotal = RCE_INST_MEM_SIZE;
              pClsCompiler->usagePerRc[uRc].uRsMax = pClsCompiler->uMaxRs;
          }
          pClsCompiler->usagePerRc[uRc].uRsUsed = 0;
          pClsCompiler->usagePerRc[uRc].uMemUsed = 0;
          pClsCompiler->usagePerRc[uRc].uRsTobeUsed = 0;
          pClsCompiler->usagePerRc[uRc].uMemTobeUsed = 0;

          if ( (!SOC_WARM_BOOT(pFe->unit)) && (uBankedMode == 0) ) {
              uRSet = (G2P3_MAX_RC_INSTANCE - uRc - 1) * pClsCompiler->uMaxRs;
              pRs = &pClsCompiler->sRsDb[uRSet];
              result = shr_mem_avl_malloc(
                                   pClsCompiler->mem_avl_ptr[uRc][pRs->uBank],
                                   pClsCompiler->uMaxInstCount,
                                   &pRs->uBaseAddress);
              if (result != 0) {
#ifdef RULESET_DEBUG
                  soc_cm_print("\n Memory allocation for swap ruleset failed");
                  soc_cm_print(" in RC%d\n", uRc);
#endif
                  return result;
              }
              /* Reduce swap memory from usable memory space */
              pClsCompiler->usagePerRc[uRc].uMemTotal =
                      pClsCompiler->usagePerRc[uRc].uMemTotal -
                                          pClsCompiler->uMaxInstCount;
              /* Reduce swap Rs from max Rs */
              pClsCompiler->usagePerRc[uRc].uRsMax = 
                                   pClsCompiler->usagePerRc[uRc].uRsMax - 1;
          
              pRs->uAlloc = 1;
              pRs->uSize = pClsCompiler->uMaxInstCount;
              pRs->sType = -1;
              pRs->uProgNumber = -1;
              pRs->uCommitted = 0;
              pClsCompiler->uSwapRs[uRc] = uRSet;
              pClsCompiler->isSwapRsUsed[uRc] = 0;
          }
      }
  }
#endif  
  return 0;
}

sbStatus_t
soc_sbx_g2p3_cls_schema_init(soc_sbx_g2p3_state_t *pFe, uint32_t isC2)
{
  sbFe2000ClsTemplate_t *pTemplate = NULL;
  soc_sbx_g2p3_cls_schema_t *pSchema       = NULL;
  soc_sbx_g2p3_cls_schema_t *pPrevSchema   = NULL;
  soc_sbx_g2p3_cls_t *pClsCompiler    = pFe->pClsComp;
  uint32_t uPrevOffset = 0;

  pClsCompiler->uMaxInstCount = 0;

  /* G2P3 Ingress Schema Initialization  */
  pSchema                       = &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA];
  pTemplate                     = &pSchema->schema;
  pSchema->pInstructions        = NULL;
  pSchema->pEncoder             = soc_sbx_g2p3_cls_g2p3_schema_encoder;
  pSchema->pDecoder             = soc_sbx_g2p3_cls_g2p3_schema_decoder;
  pSchema->uInstCount           = 0;
  pSchema->sType                = SB_G2P3_FE_CLS_G2P3_SCHEMA;

  /* set banked mode based on a SOC property */
  pSchema->layout.uBankedMode = soc_property_get(pFe->unit, spn_FIELD_ATOMIC_UPDATE, 0);

  pTemplate->ops         = &soc_sbx_g2p3_cls_g2p3_template[0];
  pTemplate->uCount      = sizeof(soc_sbx_g2p3_cls_g2p3_template)/sizeof(sbFe2000ClsTemplateField_t);
  pTemplate->bDebug      = pClsCompiler->bDebug;
  SB_STRCPY(pTemplate->sName, "G2P3Schema");

  sbFe2000ClsGenerateInstructions(pTemplate,
                                  &pSchema->pInstructions,&pSchema->uInstCount,
                                  &pSchema->uInstNoPadCount,TRUE, isC2);

  pSchema->uOffset       = uPrevOffset;
  if(pClsCompiler->bDebug) {
    sbFe2000ClsPrintTemplate(pTemplate);
    sbFe2000ClsPrintInstructions(pSchema->pInstructions,pSchema->uInstCount, isC2);
  }

  pPrevSchema   = pSchema;

  /* G2P3 Egress Schema Initialization  */
  pSchema                       = &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA];
  pTemplate                     = &pSchema->schema;
  pSchema->pInstructions        = NULL;
  pSchema->pEncoder             = soc_sbx_g2p3_cls_g2p3_schema_encoder;
  pSchema->pDecoder             = soc_sbx_g2p3_cls_g2p3_schema_decoder;
  pSchema->uInstCount           = 0;
  pSchema->sType                = SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA;

  pTemplate->ops         = &soc_sbx_g2p3_cls_g2p3_template[0];
  pTemplate->uCount      = sizeof(soc_sbx_g2p3_cls_g2p3_template)/sizeof(sbFe2000ClsTemplateField_t);
  pTemplate->bDebug      = pClsCompiler->bDebug;
  SB_STRCPY(pTemplate->sName, "G2P3EgrSchema");

  sbFe2000ClsGenerateInstructions(pTemplate,
                                  &pSchema->pInstructions,&pSchema->uInstCount,
                                  &pSchema->uInstNoPadCount,TRUE, isC2);

  pSchema->uOffset       = pPrevSchema->uOffset + (pPrevSchema->uInstCount)*2;
  if(pClsCompiler->bDebug) {
    sbFe2000ClsPrintTemplate(pTemplate);
    sbFe2000ClsPrintInstructions(pSchema->pInstructions,pSchema->uInstCount, isC2);
  }

  /* Generate layout information based on microcode */
  /* set banked mode based on a SOC property */
  pSchema->layout.uBankedMode = soc_property_get(pFe->unit, spn_FIELD_ATOMIC_UPDATE, 0);
  pClsCompiler->uMaxInstCount = pSchema->uInstCount;
  soc_sbx_g2p3_cls_schema_layout_init(pFe, isC2);


  /* For banked mode on C2, RC0 is used for ingress, RC1 for egress */
  if ( (isC2) && (pSchema->layout.uBankedMode) ){
    SAND_HAL_FE2000_WRITE(pFe->regSet, LU_RCE_ACTIVATE0, 0x5555);
    SAND_HAL_FE2000_WRITE(pFe->regSet, LU_RCE_ACTIVATE1, 0xaaaa);
  }

  if (!isC2) {
    SAND_HAL_FE2000_WRITE(pFe->regSet, LU_RCE_ACTIVATE0, 0x5555);
    SAND_HAL_FE2000_WRITE(pFe->regSet, LU_RCE_ACTIVATE1, 0xaaaa);
  }

  return SB_OK;
}



soc_sbx_g2p3_cls_schema_t *
soc_sbx_g2p3_cls_schema_handler_get(soc_sbx_g2p3_state_t *pFe,
                                    soc_sbx_g2p3_cls_schema_enum_t eName)
{
  if(eName == SB_G2P3_FE_CLS_G2P3_SCHEMA) {
    return &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA];
  } else if(eName == SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA) {
    return &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA];
  } else if(eName == SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA) {
    return &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA];
  } else if(eName == SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA) {
    return &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA];
  }
  return NULL;
}

sbStatus_t
soc_sbx_g2p3_cls_allocate_dma_slabs( soc_sbx_g2p3_state_t *pFe,
                               sbCommonConfigParams_p_t    cParams,
                               uint32_t uMaxSlabSize,
                               uint32_t isC2)
{
  sbStatus_t err;
  soc_sbx_g2p3_cls_t *pComp = pFe->pClsComp;

  /* Initialize the structures */

  /* fix me to use for loop with min. code */
  /* RC0 BANK A dma op */
  pComp->bankASlabs[0].uRcDb                = 0;
  pComp->bankASlabs[0].pDmaMemory           = NULL;
  pComp->bankASlabs[0].uSize                = uMaxSlabSize;
  pComp->bankASlabs[0].state                = SB_G2P3_FE_DMA_STATE_IDLE;
  pComp->bankASlabs[0].dmaop.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
  pComp->bankASlabs[0].dmaop.cb             = soc_sbx_g2p3_cls_dma_cb;
  pComp->bankASlabs[0].dmaop.data           = (void *) &pComp->bankASlabs[0];


  /* RC1 BANK A dma op */
  pComp->bankASlabs[1].uRcDb                = 1;
  pComp->bankASlabs[1].pDmaMemory           = NULL;
  pComp->bankASlabs[1].uSize                = isC2 ? 0 : uMaxSlabSize;
  pComp->bankASlabs[1].state                = SB_G2P3_FE_DMA_STATE_IDLE;
  pComp->bankASlabs[1].dmaop.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
  pComp->bankASlabs[1].dmaop.cb             = soc_sbx_g2p3_cls_dma_cb;
  pComp->bankASlabs[1].dmaop.data           = (void *) &pComp->bankASlabs[1];

  /* RC0 BANK B dma op */
  pComp->bankBSlabs[0].uRcDb                = 0;
  pComp->bankBSlabs[0].pDmaMemory           = NULL;
  pComp->bankBSlabs[0].uSize                = isC2 ? 0 : uMaxSlabSize;
  pComp->bankBSlabs[0].state                = SB_G2P3_FE_DMA_STATE_IDLE;
  pComp->bankBSlabs[0].dmaop.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
  pComp->bankBSlabs[0].dmaop.cb             = soc_sbx_g2p3_cls_dma_cb;
  pComp->bankBSlabs[0].dmaop.data           = (void *) &pComp->bankBSlabs[0];

  /* RC1 BANK B dma op */
  pComp->bankBSlabs[1].uRcDb                = 1;
  pComp->bankBSlabs[1].pDmaMemory           = NULL;
  pComp->bankBSlabs[1].uSize                = isC2 ? 0 : uMaxSlabSize;
  pComp->bankBSlabs[1].state                = SB_G2P3_FE_DMA_STATE_IDLE;
  pComp->bankBSlabs[1].dmaop.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
  pComp->bankBSlabs[1].dmaop.cb             = soc_sbx_g2p3_cls_dma_cb;
  pComp->bankBSlabs[1].dmaop.data           = (void *) &pComp->bankBSlabs[0];


  /* allocate the RC0 bank A slab memory */
  err = cParams->sbmalloc(cParams->clientData,
                          SB_ALLOC_CLS_DMA,
                          uMaxSlabSize,
                          &pComp->bankASlabs[0].pDmaMemory,
                          &pComp->bankASlabs[0].dmaop.dmaHandle );
  if(err != SB_OK) {
    return err;
  }

  /* Caladan2 shares the RC0 Bank A allocation */
  if (!isC2) {
    /* allocate the RC1 bank A slab memory */
    err = cParams->sbmalloc(cParams->clientData,
                            SB_ALLOC_CLS_DMA,
                            uMaxSlabSize,
                            &pComp->bankASlabs[1].pDmaMemory,
                            &pComp->bankASlabs[1].dmaop.dmaHandle );
    if(err != SB_OK) {
      return err;
    }

    /* allocate the RC0 bank B slab memory */
    err = cParams->sbmalloc(cParams->clientData,
                            SB_ALLOC_CLS_DMA,
                            uMaxSlabSize,
                            &pComp->bankBSlabs[0].pDmaMemory,
                            &pComp->bankBSlabs[0].dmaop.dmaHandle );
    if(err != SB_OK) {
      return err;
    }


    /* allocate the RC1 bank B slab memory */
    err = cParams->sbmalloc(cParams->clientData,
                            SB_ALLOC_CLS_DMA,
                            uMaxSlabSize,
                            &pComp->bankBSlabs[1].pDmaMemory,
                            &pComp->bankBSlabs[1].dmaop.dmaHandle );
    if(err != SB_OK) {
      return err;
    }
  }
  return SB_OK;
}

sbStatus_t
soc_sbx_g2p3_cls_de_allocate_dma_slabs( soc_sbx_g2p3_state_t *pFe )
{
  sbStatus_t err = SB_OK;
  soc_sbx_g2p3_cls_t *pClsComp = pFe->pClsComp;

  /* free up Bank A slab  */
  err = pClsComp->sbfree(pClsComp->clientData,
                          SB_ALLOC_CLS_DMA,
                          pClsComp->uMaxSlabSize,
                          pClsComp->bankASlabs[0].pDmaMemory,
                          pClsComp->bankASlabs[0].dmaop.dmaHandle );
  if(err != SB_OK) {
    return err;
  }

  /* Caladan2 shares the Bank A RC0 memory */
  if ( !SOC_IS_SBX_FE2KXT(pFe->unit) ){
    /* free up Bank A slab  */
    err = pClsComp->sbfree(pClsComp->clientData,
                            SB_ALLOC_CLS_DMA,
                            pClsComp->uMaxSlabSize,
                            pClsComp->bankASlabs[1].pDmaMemory,
                            pClsComp->bankASlabs[1].dmaop.dmaHandle );
    if(err != SB_OK) {
      return err;
    }


    /* free up Bank B slab  */
    err = pClsComp->sbfree(pClsComp->clientData,
                            SB_ALLOC_CLS_DMA,
                            pClsComp->uMaxSlabSize,
                            pClsComp->bankBSlabs[0].pDmaMemory,
                            pClsComp->bankBSlabs[0].dmaop.dmaHandle );
    if(err != SB_OK) {
      return err;
    }


    /* free up Bank B slab  */
    err = pClsComp->sbfree(pClsComp->clientData,
                            SB_ALLOC_CLS_DMA,
                            pClsComp->uMaxSlabSize,
                            pClsComp->bankBSlabs[1].pDmaMemory,
                            pClsComp->bankBSlabs[1].dmaop.dmaHandle );
    if(err != SB_OK) {
      return err;
    }
  }

  return SB_OK;
}

void soc_sbx_g2p3_cls_g2p3_schema_encoder(soc_sbx_g2p3_cls_schema_p_t pSchema,
                                uint32_t                   uRIndex,
                                void * pRule1,
                                soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory)
{
  uint32_t uOffset = 0;
  uint32_t uFieldValue = 0;
  int32_t uWidth=0,uMsb=0,uLsb=0;
  uint32_t uRs =0 , uRuleId=0, uRuleSb=0, uRcInstance =0;
  soc_sbx_g2p3_bit_vector_t *vec = NULL;
  uint32_t uDontCareLength  =0, uMatchLength =0;
  uint8_t uIpv4Da[4],uIpv4Sa[4];
  uint8_t uMac[6];
  sbFe2000ClsTemplateField_t  *pOperation = NULL;
  sbFe2000ClsTemplate_t       *pTemplate  = &pSchema->schema;
  sbZfFe2000RcSbPattern_t *pPattern = NULL;
  soc_sbx_g2p3_cls_ace_pattern_p_t pRule = pRule1;

  #define STUFF_RESUME { \
    while ( (uOffset % 8 != 0) && \
            (uOffset % 8 != 1) ){ \
      uOffset++; \
    } \
    uOffset++; \
    while (uOffset % 8 != 0) { \
      uOffset++; \
    } \
  }

  #ifdef CLS_PAT_DEBUG
    patDebugArrayZero();
  #endif

    /* get the pattern block that this rule is going to go */
    uRs    = uRIndex / pSchema->layout.uNumRulesPerRs;
    uRuleId     = uRIndex % pSchema->layout.uNumRulesPerRs;
    uRuleSb     = uRuleId / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;

    /* RC0 for ingress, RC1 for egress */
    if (pSchema->sType == SB_G2P3_FE_CLS_G2P3_SCHEMA) {
        uRcInstance = 0;
    }else{
        uRcInstance = 1;
    }

    uRs    = uRs % pSchema->layout.uNumRs;

    pPattern = pPatternMemory->pSbPattern[uRs][uRcInstance][uRuleSb];

    /* Within a super block it's mod 128 */
    uRuleId = uRuleId % G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;

    /* increment the offset for the restart */
    CLS_OP_ENCODE_RESUME(uOffset);

   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_PMASK at offset %d\n", uOffset);
    #endif
    /* step 0: SB_FE2K_CLS_G2P3_SCHEMA_OP_PMASK  MultiPort */
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_PORT];
    uWidth = SB_G2P3_CLS_MAX_PORTS;
    EmitPbmp(pPattern,&uOffset,uRuleId,pRule->uuPbmp,uWidth);

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 1: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_PRI  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_PRI at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_PRI];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    if(pRule->bPriorityAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uPriority & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }
    
    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 2: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_FRAGMENT */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_FRAGMENT at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_FRAGMENT];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    if(pRule->bFragmentAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uFragment & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 3: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_URG_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_URG_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_URG_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    if(pRule->bTcpFlagURGAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uTcpFlagURG & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 4: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_ACK_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_ACK_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_ACK_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    if(pRule->bTcpFlagACKAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uTcpFlagACK & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 5: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_PSH_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_PSH_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_PSH_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    if(pRule->bTcpFlagPSHAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uTcpFlagPSH & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 6: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_RST_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_RST_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_RST_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    if(pRule->bTcpFlagRSTAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uTcpFlagRST & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 7: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_SYN_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_SYN_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_SYN_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    if(pRule->bTcpFlagSYNAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uTcpFlagSYN & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 8: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_FIN_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_FIN_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_FIN_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    if(pRule->bTcpFlagFINAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uTcpFlagFIN & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset);  /* encode the resume bit */
    
    /* step 9: SB_FE2K_CLS_G2_G2_SCHEMA_OP_IPV4_DSCP */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DSCP at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DSCP];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    if(pRule->bDscpAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uDscp & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

    /* step 10: SB_FE2K_CLS_G2_G2_SCHEMA_OP_IPV4_ECN */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_ECN at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_ECN];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    if(pRule->bEcnAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uEcn & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */
    
    /* step 11: SB_FE2K_CLS_G2_EGR_SCHEMA_OP_IPV4_PROTOCOL */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_PROTOCOL at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_PROTOCOL];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    if(pRule->bProtocolAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uProtocol & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */
    
    if(!pRule->bEtherTypeAny && !pRule->uEtherType){
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_DSAPSSAP at offset %d\n", uOffset);
   #endif
      /* step 12:LLC DSAP  Range Check */
      pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_DSAPSSAP];
      uWidth = CLS_OP_WIDTH(pOperation);
      uMsb   = uWidth-1; uLsb =0;
      uFieldValue = pRule->uLlcDsapSsapLow & MASK_N(uWidth); /* Min */
      if(pRule->bDsapAny) {
        uFieldValue = 0;
      }
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

      uFieldValue = pRule->uLlcDsapSsapHigh & MASK_N(uWidth); /* Max */
       if(pRule->bDsapAny) {
        uFieldValue = MASK_N(uWidth);
      }
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

      /* step 14: LLC CTRLDATA Range Check */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_CTRLDATA at offset %d\n", uOffset);
   #endif
      pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_CTRLDATA];
      uWidth = CLS_OP_WIDTH(pOperation);
      uMsb   = uWidth-1; uLsb =0;
      uFieldValue = pRule->uLlcCtrlDataLow & MASK_N(uWidth); /* Min */
      if(pRule->bLlcCtrlAny) {
        uFieldValue = 0;
      }
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

      uFieldValue = pRule->uLlcCtrlDataHigh & MASK_N(uWidth); /* Max */
      if(pRule->bLlcCtrlAny) {
        uFieldValue = MASK_N(uWidth);
      }
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

    } else {
   #ifdef CLS_PAT_DEBUG
      SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_SRCPORT at offset %d\n", uOffset);
   #endif
      /* step 12: SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_SRCPORT  Range Check */
      pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_SRCPORT];
      uWidth = CLS_OP_WIDTH(pOperation);
      uMsb   = uWidth-1; uLsb =0;
      uFieldValue = pRule->uL4SrcPortLow & MASK_N(uWidth); /* Min */
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

      uFieldValue = pRule->uL4SrcPortHigh & MASK_N(uWidth); /* Max */
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

      /* step 14: SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_DSTPORT  Range Check */
   #ifdef CLS_PAT_DEBUG
      SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_DSTPORT at offset %d\n", uOffset);
   #endif
      pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_DSTPORT];
      uWidth = CLS_OP_WIDTH(pOperation);
      uMsb   = uWidth-1; uLsb =0;
      uFieldValue = pRule->uL4DstPortLow & MASK_N(uWidth); /* Min */
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

      uFieldValue = pRule->uL4DstPortHigh & MASK_N(uWidth); /* Max */
      EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

      CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */
    }

    /* step 16: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_IPV4_SA */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_SA at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_SA];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    uIpv4Sa[0] = (pRule->uIpv4Sa >> 24) & 0xFF;
    uIpv4Sa[1] = (pRule->uIpv4Sa >> 16) & 0xFF;
    uIpv4Sa[2] = (pRule->uIpv4Sa >>  8) & 0xFF;
    uIpv4Sa[3] = (pRule->uIpv4Sa >>  0) & 0xFF;
    if(pRule->bIpv4SaAny) {
      EmitMatchLpm(pPattern, &uOffset,uRuleId, 0, uWidth, NULL);
    } else {
      vec = (soc_sbx_g2p3_bit_vector_t *)&uIpv4Sa[0];
      uDontCareLength  = 32-pRule->uIpv4SaLen;
      uMatchLength     = pRule->uIpv4SaLen;
      EmitMatchLpm(pPattern,&uOffset,uRuleId,uMatchLength,uDontCareLength, vec);
    }

    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

    /* step 17: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_IPV4_DA */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DA at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DA];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    uIpv4Da[0] = (pRule->uIpv4Da >> 24) & 0xFF;
    uIpv4Da[1] = (pRule->uIpv4Da >> 16) & 0xFF;
    uIpv4Da[2] = (pRule->uIpv4Da >>  8) & 0xFF;
    uIpv4Da[3] = (pRule->uIpv4Da >>  0) & 0xFF;
    if(pRule->bIpv4DaAny) {
      EmitMatchLpm(pPattern, &uOffset,uRuleId, 0, uWidth, NULL);
    } else {
      vec = (soc_sbx_g2p3_bit_vector_t *)&uIpv4Da[0];
      uDontCareLength  = 32-pRule->uIpv4DaLen;
      uMatchLength     = pRule->uIpv4DaLen;
      EmitMatchLpm(pPattern,&uOffset,uRuleId,uMatchLength,uDontCareLength,vec);
    }
    
    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

    /* step 18: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_ETH_ETYPE */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_ETH_ETYPE at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_ETH_ETYPE];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    if(pRule->bEtherTypeAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uEtherType & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

    /* step 19: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_SMAC_47_0 */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_SMAC_47_0 at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_SMAC_47_0];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    /* make the mac address array */
    MakeMacArrayFromUint64(pRule->uuSmac, &uMac[0]);
    if(pRule->bSmacAny) {
      EmitMatchLpm(pPattern, &uOffset, uRuleId, 0, uWidth, NULL);
    } else {
      vec = (soc_sbx_g2p3_bit_vector_t *)&uMac[0];
      uDontCareLength = 48 - pRule->uuSmacLen;
      uMatchLength    = pRule->uuSmacLen;
      EmitMatchLpm(pPattern,&uOffset,uRuleId,uMatchLength,uDontCareLength,vec);
    }

    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

    /* step 20: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_VSI */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_VSI at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_VSI];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    if(pRule->bVlanAny) {
      EmitWildCardEQ(pPattern, &uOffset, uRuleId,uMsb,uLsb,0);
    } else {
      uFieldValue = pRule->uVlan & MASK_N(uWidth);
      EmitExactMatch32(pPattern,&uOffset,uRuleId,uMsb,uLsb,uFieldValue,0);
    }

    CLS_OP_ENCODE_RESUME(uOffset); /* encode the resume bit */

   /* Ugliness alert - For C2 we have a jump instruction 24 from the end
    * This happens to fall between bit offsets 16 and 17 of the DMAC
    * So we have to encode the DMAC pattern in two parts
    * first from  47:17 and then 16:0 with a skip in between them
    */
    /* step 21: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_DMAC_47_0 */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_DMAC_47_0 at offset %d\n", uOffset);
   #endif

    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_DMAC_47_0];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    /* make the mac address array */
    MakeMacArrayFromUint64(pRule->uuDmac, &uMac[0]);
    if(pRule->bDmacAny) {
        EmitMatchLpm(pPattern, &uOffset, uRuleId, 0, uWidth, NULL);
    } else {
        vec = (soc_sbx_g2p3_bit_vector_t *)&uMac[0];
        uDontCareLength = 48-pRule->uuDmacLen;
        uMatchLength    = pRule->uuDmacLen;
        EmitMatchLpm(pPattern,&uOffset,uRuleId,uMatchLength,uDontCareLength,vec);
    }
    
  #ifdef CLS_PAT_DEBUG
    patDebugArrayPrint();
    SB_LOG("Final value of uOffset is %d\n\n", uOffset);
  #endif

  return;
}

void soc_sbx_g2p3_cls_g2p3_schema_decoder(soc_sbx_g2p3_cls_schema_p_t pSchema,
                                uint32_t uRIndex,
                                void * pRule1,
                                sbZfFe2000RcSbPattern_t *pPatternMemory[],
                                uint32_t *isNull)
{
  uint32_t uOffset = 0, uOffsetSave;
  int32_t uWidth=0;
  uint32_t uRuleId=0, uRuleSb=0;
  int32_t uMsb=0,uLsb=0;
  soc_sbx_g2p3_bit_vector_t *vec = NULL;
  uint32_t uMatchLength =0;
  uint8_t uMac[6] = {0};
  uint8_t uIpv4Da[4],uIpv4Sa[4];
  sbFe2000ClsTemplateField_t  *pOperation = NULL;
  sbFe2000ClsTemplate_t       *pTemplate  = &pSchema->schema;
  sbZfFe2000RcSbPattern_t *pPattern = NULL;
  soc_sbx_g2p3_cls_ace_pattern_p_t pRule = pRule1;

  #ifdef CLS_PAT_DEBUG
    patDebugArrayZero();
  #endif

    /* get the pattern block that this rule is going to go */
    uRuleId     = uRIndex % pSchema->layout.uNumRulesPerRs;
    uRuleSb     = uRuleId / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;

    pPattern = pPatternMemory[uRuleSb];

    /* Within a super block it's mod 128 */
    uRuleId = uRuleId % G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;

    *isNull = sbZfFe2000RcSbPatternIsNull(pPattern,
                                    uRuleId,
                                    pSchema->uInstCount);
    if (*isNull){
        return;
    }

    /* skip the offset for the restart */
    CLS_OP_DECODE_RESUME(uOffset);

   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_PMASK at offset %d\n", uOffset);
    #endif
    /* step 0: SB_FE2K_CLS_G2P3_SCHEMA_OP_PMASK  MultiPort */
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_PORT];
    uWidth = SB_G2P3_CLS_MAX_PORTS;
    DecodePbmp(pPattern,&uOffset,uRuleId,&pRule->uuPbmp,&pRule->bPortAny, uWidth);
    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */
    /* step 1: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_PRI  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_PRI at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_PRI];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uPriority, &pRule->bPriorityAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 2: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_FRAGMENT */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_FRAGMENT at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_FRAGMENT];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uFragment, &pRule->bFragmentAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 3: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_URG_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_URG_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_URG_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uTcpFlagURG, &pRule->bTcpFlagURGAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 4: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_ACK_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_ACK_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_ACK_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uTcpFlagACK, &pRule->bTcpFlagACKAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 5: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_PSH_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_PSH_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_PSH_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uTcpFlagPSH, &pRule->bTcpFlagPSHAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 6: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_RST_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_RST_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_RST_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uTcpFlagRST, &pRule->bTcpFlagRSTAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 7: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_SYN_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_SYN_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_SYN_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uTcpFlagSYN, &pRule->bTcpFlagSYNAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */

    /* step 8: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_FIN_FLAG  */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2_INGR_SCHEMA_OP_TCP_FIN_FLAG at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_TCP_FIN_FLAG];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uTcpFlagFIN, &pRule->bTcpFlagFINAny);

    CLS_OP_DECODE_RESUME(uOffset);  /* encode the resume bit */
    
    /* step 9: SB_FE2K_CLS_G2_G2_SCHEMA_OP_IPV4_DSCP */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DSCP at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DSCP];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uDscp, &pRule->bDscpAny);

    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

    /* step 10: SB_FE2K_CLS_G2_G2_SCHEMA_OP_IPV4_ECN */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_ECN at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_ECN];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uEcn, &pRule->bEcnAny);

    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */
    
    /* step 11: SB_FE2K_CLS_G2_EGR_SCHEMA_OP_IPV4_PROTOCOL */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_PROTOCOL at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_PROTOCOL];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uProtocol, &pRule->bProtocolAny);

    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

   /* Decode both LLC and L4PORT info.  Determine which is valid later
    * based on the Ethertype fields.
    */
   uOffsetSave = uOffset;

    #ifdef CLS_PAT_DEBUG
      SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_DSAPSSAP at offset %d\n", uOffset);
    #endif
    /* step 12:LLC DSAP  Range Check */
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_DSAPSSAP];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uLlcDsapSsapLow);
    CLS_OP_DECODE_RESUME(uOffset);
    DecodeBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uLlcDsapSsapHigh);
    if ( (pRule->uLlcDsapSsapLow == 0) &&
         (pRule->uLlcDsapSsapHigh == MASK_N(uWidth)) ) {
        pRule->bDsapAny = 1;
    }
    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */
  

    /* step 14: LLC CTRLDATA Range Check */
  #ifdef CLS_PAT_DEBUG
  SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_CTRLDATA at offset %d\n", uOffset);
  #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_LLC_CTRLDATA];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uLlcCtrlDataLow);
    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */
    DecodeBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uLlcCtrlDataHigh);
    if ( (pRule->uLlcCtrlDataLow == 0) &&
         (pRule->uLlcCtrlDataHigh == MASK_N(uWidth)) ) {
        pRule->bLlcCtrlAny = 1;
    }
    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_SRCPORT at offset %d\n", uOffsetSave);
   #endif
    /* step 12: SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_SRCPORT  Range Check */
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_SRCPORT];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeBits32(pPattern, &uOffsetSave, uRuleId, uMsb, uLsb, &pRule->uL4SrcPortLow);
    CLS_OP_DECODE_RESUME(uOffsetSave); /* encode the resume bit */
    DecodeBits32(pPattern, &uOffsetSave, uRuleId, uMsb, uLsb, &pRule->uL4SrcPortHigh);
    CLS_OP_DECODE_RESUME(uOffsetSave); /* encode the resume bit */

    /* step 14: SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_DSTPORT  Range Check */
 #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_DSTPORT at offset %d\n", uOffsetSave);
 #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_L4_DSTPORT];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeBits32(pPattern, &uOffsetSave, uRuleId, uMsb, uLsb, &pRule->uL4DstPortLow);
    CLS_OP_DECODE_RESUME(uOffsetSave); /* encode the resume bit */
    DecodeBits32(pPattern, &uOffsetSave, uRuleId, uMsb, uLsb, &pRule->uL4DstPortHigh);
    CLS_OP_DECODE_RESUME(uOffsetSave); /* encode the resume bit */

    SB_ASSERT(uOffset == uOffsetSave);

    /* step 16: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_IPV4_SA */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_SA at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_SA];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    uIpv4Sa[0] = 0;
    uIpv4Sa[1] = 0;
    uIpv4Sa[2] = 0;
    uIpv4Sa[3] = 0;
    pRule->uIpv4Sa = 0;
    vec = (soc_sbx_g2p3_bit_vector_t *)&uIpv4Sa[0];
    DecodeMatchLpm(pPattern, &uOffset, uRuleId, &uMatchLength, uWidth, vec);

    pRule->uIpv4Sa |= uIpv4Sa[0] << 24;
    pRule->uIpv4Sa |= uIpv4Sa[1] << 16;
    pRule->uIpv4Sa |= uIpv4Sa[2] << 8;
    pRule->uIpv4Sa |= uIpv4Sa[3] << 0;

    pRule->uIpv4SaLen = uMatchLength;
    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

    /* step 17: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_IPV4_DA */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DA at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_IPV4_DA];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    uIpv4Da[0] = 0;
    uIpv4Da[1] = 0;
    uIpv4Da[2] = 0;
    uIpv4Da[3] = 0;
    pRule->uIpv4Da = 0;
    vec = (soc_sbx_g2p3_bit_vector_t *)&uIpv4Da[0];
    DecodeMatchLpm(pPattern, &uOffset, uRuleId, &uMatchLength, uWidth, vec);

    pRule->uIpv4Da |= uIpv4Da[0] << 24;
    pRule->uIpv4Da |= uIpv4Da[1] << 16;
    pRule->uIpv4Da |= uIpv4Da[2] << 8;
    pRule->uIpv4Da |= uIpv4Da[3] << 0;

    pRule->uIpv4DaLen = uMatchLength;
    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

    /* step 18: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_ETH_ETYPE */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_ETH_ETYPE at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_ETH_ETYPE];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uEtherType, &pRule->bEtherTypeAny);

    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

    /* Determine whether rule uses LLC fields or L4 ports */
    if (!pRule->bEtherTypeAny && !pRule->uEtherType){
        /* LLC fields valid, clear L4 port info */
        pRule->uL4SrcPortHigh = 0xffff;
        pRule->uL4SrcPortLow = 0x0;
        pRule->uL4DstPortHigh = 0xffff;
        pRule->uL4DstPortLow = 0x0;
    }else{
        /* L4 ports valids, clear LLC fields */
        pRule->uLlcDsapSsapHigh = 0xffff;
        pRule->uLlcDsapSsapLow = 0x0;
        pRule->bDsapAny = 0x1;
        pRule->uLlcCtrlDataHigh = 0xffff;
        pRule->uLlcCtrlDataLow = 0x0;
        pRule->bLlcCtrlAny = 0x1;
        
    }

    /* step 19: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_SMAC_47_0 */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_SMAC_47_0 at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_SMAC_47_0];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    vec = (soc_sbx_g2p3_bit_vector_t *)&uMac[0];
    DecodeMatchLpm(pPattern, &uOffset, uRuleId, &uMatchLength, uWidth, vec);
    pRule->uuSmacLen = uMatchLength;
    if (uMatchLength) {
        MakeUint64FromMacArray(&uMac[0], &pRule->uuSmac);
    }

    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

    /* step 20: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_VSI */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_VSI at offset %d\n", uOffset);
   #endif
    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_VSI];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    DecodeWildCardEQOrExactMatch(pPattern, &uOffset, uRuleId, uMsb, uLsb, &pRule->uVlan, &pRule->bVlanAny);

    CLS_OP_DECODE_RESUME(uOffset); /* encode the resume bit */

    /* step 21: SB_FE2K_CLS_G2_INGR_SCHEMA_OP_DMAC_47_0 */
   #ifdef CLS_PAT_DEBUG
    SB_LOG("Start SB_FE2K_CLS_G2P3_SCHEMA_OP_DMAC_47_0 at offset %d\n", uOffset);
   #endif

    pOperation = &pTemplate->ops[SB_FE2K_CLS_G2P3_SCHEMA_OP_DMAC_47_0];
    uWidth = CLS_OP_WIDTH(pOperation);
    uMsb   = uWidth-1; uLsb =0;
    vec = (soc_sbx_g2p3_bit_vector_t *)&uMac[0];
    DecodeMatchLpm(pPattern, &uOffset, uRuleId, &uMatchLength, uWidth, vec);
    pRule->uuDmacLen = uMatchLength;
    if (uMatchLength) {
        MakeUint64FromMacArray(&uMac[0], &pRule->uuDmac);
    }

  #ifdef CLS_PAT_DEBUG
    patDebugArrayPrint();
    SB_LOG("Final value of uOffset is %d\n\n", uOffset);
  #endif

  return;
}

sbStatus_t
soc_sbx_g2p3_cls_commit(soc_sbx_g2p3_state_t *pFe,
                uint8_t  uBankId,
                soc_sbx_g2p3_cls_schema_t *pSchema,
                soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory,
                uint8_t *pProgramNumber)
{

  sbStatus_t status = SB_OK;
  uint32_t uIndex=0,uId=0, uRSet=0,uRcInstance=0;
  uint32_t uPhysicalProgramNumber =0;
  uint32_t uNumberOfDmaBlocks = (pSchema->uInstCount)/4;
  uint32_t uNewBkupProgramNumer = 0;
  uint8_t  uProgramNumber=0, uLogProgNumber=0;
  soc_sbx_g2p3_cls_prog_attr_t *pNewActProgAttr = NULL;
  soc_sbx_g2p3_cls_prog_attr_t *pNewBkupProgAttr = NULL;

  sbZfFe2000RcDmaFormat_t      zDmaFrame;
  sbFe2000RcDmaFormatAlias_t   zAlias;
  sbZfFe2000RcSbPatternAlias_t zPatternAlias;
  sbZfFe2000RcSbPattern_t *pPattern       = NULL;
  soc_sbx_g2p3_cls_bank_transfer_t *pBankHandler   = NULL;
  uint32_t *pDmaBase                      = NULL;
  uint32_t *pOrigDmaBase                  = NULL;
  sbFe2000DmaOp_t  *pDmaOp                = NULL;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;

  SB_ASSERT(pFe);

  SB_ASSERT(!SOC_IS_SBX_FE2KXT(pFe->unit));

  pClsComp = pFe->pClsComp;

  /* obtain the program attributes and switch the states of programs */
  soc_sbx_g2p3_cls_schema_to_program(pFe,
                                    pSchema->sType,
                                    &uProgramNumber);

  soc_sbx_g2p3_cls_schema_to_logprogram(pFe, pSchema->sType, &uLogProgNumber);
  uNewBkupProgramNumer      = uProgramNumber;
  uPhysicalProgramNumber    = pClsComp->sbyProgNumber[pSchema->sType];
  pNewActProgAttr           = pClsComp->pProgAtrib[uPhysicalProgramNumber];
  pNewBkupProgAttr          = pClsComp->pProgAtrib[uNewBkupProgramNumer];
  *pProgramNumber           = uPhysicalProgramNumber;
  /* Set active program info */
  pNewActProgAttr->uActive  = 1;

  /* Set Backup program info */
  pNewBkupProgAttr->uActive = 0;
  pClsComp->sbyProgNumber[pSchema->sType] = uNewBkupProgramNumer;

  for(uRcInstance=0; uRcInstance < G2P3_MAX_RC_INSTANCE; uRcInstance++) {

    pBankHandler   = (uBankId) ? (&pClsComp->bankASlabs[uRcInstance]) :
                                 (&pClsComp->bankBSlabs[uRcInstance]);
    pDmaOp        = &pBankHandler->dmaop;
    pDmaBase      = (uint32_t *) pBankHandler->pDmaMemory;
    pOrigDmaBase  = pDmaBase;


    /* RC0 dedicated to ingress, RC1 for egress */
    if ((pSchema->sType == SB_G2P3_FE_CLS_G2P3_SCHEMA) &&
        (uRcInstance == 1) ) {
        continue;
    }
    if ( (pSchema->sType == SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA) &&
               (uRcInstance == 0) ) {
        continue;
    }
    sbFe2000RcProgramControlSet(pFe->regSet,
                             uRcInstance,
                             uPhysicalProgramNumber,
                             TRUE,
                             pNewActProgAttr->uLength,
                             pNewActProgAttr->uBaseAddress);
        
#ifdef CLS_COMP_DEBUG
    soc_cm_print("\n RCE Program Ctrl: Len[%d] Addr[%d] pno[%d] inst[%d]",\
                pNewActProgAttr->uLength,
                pNewActProgAttr->uBaseAddress,
                uPhysicalProgramNumber, uRcInstance);
#endif
    for(uRSet=0; uRSet < pSchema->layout.uNumRs;uRSet++) {

      for(uIndex=0; uIndex < uNumberOfDmaBlocks; uIndex++) {

        sbZfFe2000RcDmaFormat_InitInstance(&zDmaFrame);

        sbZfFe2000RcDmaFormatAlias(&zDmaFrame, &zAlias);
        zDmaFrame.uInstruction0 = pSchema->pInstructions[(uIndex*4)];
        zDmaFrame.uInstruction1 = pSchema->pInstructions[(uIndex*4)+1];
        zDmaFrame.uInstruction2 = pSchema->pInstructions[(uIndex*4)+2];
        zDmaFrame.uInstruction3 = pSchema->pInstructions[(uIndex*4)+3];
        if( (uIndex*4)+0  == pSchema->uInstNoPadCount-1) {
          if(pPatternMemory->uSaveResult[uRSet][uRcInstance]) {
            zDmaFrame.uInstruction0 = pSchema->pInstructions[(uIndex*4)+3+1];
          }
        }
        if( (uIndex*4)+1  == pSchema->uInstNoPadCount-1) {
          if(pPatternMemory->uSaveResult[uRSet][uRcInstance]) {
            zDmaFrame.uInstruction1 = pSchema->pInstructions[(uIndex*4)+3+1];
          }
        }
        if( (uIndex*4)+2  == pSchema->uInstNoPadCount-1) {
          if(pPatternMemory->uSaveResult[uRSet][uRcInstance]) {
            zDmaFrame.uInstruction2 = pSchema->pInstructions[(uIndex*4)+3+1];
          }
        }
        if( (uIndex*4)+3  == pSchema->uInstNoPadCount-1) {
          if(pPatternMemory->uSaveResult[uRSet][uRcInstance]) {
            zDmaFrame.uInstruction3 = pSchema->pInstructions[(uIndex*4)+3+1];
          }
        }

        for(uId=0; uId  < 32; uId++) {
          pPattern = pPatternMemory->pSbPattern[uRSet][uRcInstance][uId / 16];
          sbZfFe2000RcSbPatternAlias(&pPattern[(uIndex*4)+(uId/4)%4], &zPatternAlias);
          *(zAlias.pPattern[uId]) = *(zPatternAlias.pPattern[uId%4]);
        }

        sbZfFe2000RcDmaFormat_Pack(&zDmaFrame, (uint8_t*)&pDmaBase[(uIndex*64)], SB_ZF_FE2000RCDMAFORMAT_SIZE_IN_BYTES);
      }
      /* move the dma base by rule set size */
      pDmaBase += (uNumberOfDmaBlocks*64);

    }  /*RSet*/

    /* Dma offset is given by the ProgramOffset*sizeof(rc dmablock)/4 ( # of words) */
    pDmaOp->words       = (pSchema->uInstCount*16) * pSchema->layout.uNumRs;

    pDmaOp->feAddress   = SB_FE2000_DMA_MAKE_ADDRESS(                   \
                              (uRcInstance == 0) ? SB_FE2000_MEM_CLS_0  \
                                                 : SB_FE2000_MEM_CLS_1, \
                               pNewActProgAttr->uBaseAddress * 16);

    pDmaOp->hostAddress = pOrigDmaBase;
    pDmaOp->opcode      = SB_FE2000_DMA_OPCODE_WRITE;
    pDmaOp->data        = pFe;

    status = sbFe2000DmaRequest(pClsComp->pDmaCtxt, pFe->DmaCfg.handle,
                                pDmaOp);

    if (status != SB_OK) {
      SB_ASSERT(0);
      return status;
    }
  } /*uRcInstance*/

  /* make the switch to the new program */
  sbFe2000RcMapLogicaltoPhysicalProgram(pFe->regSet, /* user device handle */
                                        uLogProgNumber,
                                       (uLogProgNumber % 2)?
                                        G2P3_EGRESS_LOGICAL_PROGRAM_NUMBER:
                                        G2P3_INGRESS_LOGICAL_PROGRAM_NUMBER,
                                        uPhysicalProgramNumber );

#ifdef CLS_COMP_DEBUG
  soc_cm_print("\n Switch Log[%d] to Phy[%d]", (uPhysicalProgramNumber % 2)?
                                          G2P3_EGRESS_LOGICAL_PROGRAM_NUMBER:
                                          G2P3_INGRESS_LOGICAL_PROGRAM_NUMBER,
                                          uPhysicalProgramNumber);
#endif

  return SB_OK;
}

void soc_sbx_g2p3_cls_dma_cb(sbFe2000DmaOp_t *pOp)
{
  soc_sbx_g2p3_state_t *pFe;
  sbFeAsyncCallback_f_t pfCb;
  sbFeAsyncCallbackArgument_t cba;

  SB_LOG("%s called", __FUNCTION__);

  /* decode back reference */
  pFe = pOp->data;

  SB_ASSERT(pFe);
  SB_ASSERT(pFe->pClsComp);

  pfCb = pFe->cParams.asyncCallback;

  cba.type = SB_FE_ASYNC_CLS_COMMIT_DONE;
  cba.status = pOp->status;
  cba.clientData = pFe->cParams.clientData; /* hdl */

  (*pfCb)(&cba);
}

sbStatus_t
soc_sbx_g2p3_cls_alloc_pattern_memory(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_schema_t *pSchema,
                            soc_sbx_g2p3_cls_pattern_memory_t **ppPatternMemory)
{
  sbStatus_t err = SB_OK;
  uint32_t uBufferSize = 0;
  uint32_t uIndex = 0;
  uint32_t uRc, uSb, uMaxSb, uMaxRs;
  void  *v;

  soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory = NULL;
  soc_sbx_g2p3_cls_t *pClsCompiler            = pFe->pClsComp;

  /* create the pattern memory container */
  err = pClsCompiler->sbmalloc(pClsCompiler->clientData,
                               SB_ALLOC_INTERNAL,
                               sizeof(soc_sbx_g2p3_cls_pattern_memory_t),
                               &v, NULL);
  if(err != SB_OK) {
    return err;
  }

  pPatternMemory = (soc_sbx_g2p3_cls_pattern_memory_t *)v;
  SB_MEMSET(v, 0, sizeof(soc_sbx_g2p3_cls_pattern_memory_t));

  /* Initialize the number of Instructions count */
  pPatternMemory->uCount  = pSchema->uInstCount;

  uMaxSb = pSchema->layout.uNumRulesPerRs / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;
  if (SOC_IS_SBX_FE2KXT(pFe->unit)){
      uMaxRs = 1;
  }else{
      uMaxRs = pSchema->layout.uNumRs;
  }

  for(uIndex=0; uIndex < uMaxRs; uIndex++) {

    /* create the super classifier super block containers */
    for (uRc=0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
      pPatternMemory->uSaveResult[uIndex][uRc] = 0;
      for (uSb=0; uSb < uMaxSb; uSb++) {
        uBufferSize = (pSchema->uInstCount)*sizeof(sbZfFe2000RcSbPattern_t);
        err = pClsCompiler->sbmalloc(pClsCompiler->clientData,
                                     SB_ALLOC_INTERNAL,
                                     uBufferSize,
                                     &v, NULL);
        if(err != SB_OK) {
          return err;
        }
        pPatternMemory->pSbPattern[uIndex][uRc][uSb] = (sbZfFe2000RcSbPattern_t *)v;
      }
    }
  }

  /* create the soc_sbx_g2p3_bit_vector_t */
  uBufferSize =  (pSchema->uInstCount/8) +1;
  err = pClsCompiler->sbmalloc(pClsCompiler->clientData,
                               SB_ALLOC_INTERNAL,
                               uBufferSize,
                               &v, NULL);
  if(err != SB_OK) {
    return err;
  }
  pPatternMemory->pVector = (soc_sbx_g2p3_bit_vector_t *)v;

  /* copy the pointer to the memory */
  *ppPatternMemory = pPatternMemory;
  return SB_OK;;
}

sbStatus_t
soc_sbx_g2p3_cls_free_pattern_memory(soc_sbx_g2p3_state_t *pFe,
                           soc_sbx_g2p3_cls_schema_t *pSchema,
                           soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory)
{
  sbStatus_t err = SB_OK;
  uint32_t uBufferSize = 0;
  uint32_t uIndex =0, uRc=0, uSb=0, uMaxSb, uMaxRs;
  soc_sbx_g2p3_cls_t *pClsCompiler = pFe->pClsComp;
  sbDmaMemoryHandle_t junk = {0};

  uBufferSize = pSchema->uInstCount*sizeof(sbZfFe2000RcSbPattern_t);

  uMaxSb = pSchema->layout.uNumRulesPerRs / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;

  if (SOC_IS_SBX_FE2KXT(pFe->unit)){
      uMaxRs = 1;
  }else{
      uMaxRs = pSchema->layout.uNumRs;
  }
  for(uIndex=0; uIndex < uMaxRs; uIndex++) {

    for (uRc=0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
      pPatternMemory->uSaveResult[uIndex][uRc] = 0;
      for (uSb=0; uSb < uMaxSb; uSb++) {
        /* free the pattern memory */
        err = pClsCompiler->sbfree(pClsCompiler->clientData,
                                   SB_ALLOC_INTERNAL,
                                   uBufferSize,
                                   pPatternMemory->pSbPattern[uIndex][uRc][uSb],
                                   junk );
        if(err != SB_OK) {
          return err;
        }
      }
    }
  }

  /* free the bit_vector */
  err = pClsCompiler->sbfree(pClsCompiler->clientData,
                             SB_ALLOC_INTERNAL,
                             (pSchema->uInstCount/8)+1,
                             pPatternMemory->pVector,
                             junk );
  if(err != SB_OK) {
    return err;
  }

  /* free the container */
  err = pClsCompiler->sbfree(pClsCompiler->clientData,
                             SB_ALLOC_INTERNAL,
                             sizeof(soc_sbx_g2p3_cls_pattern_memory_t),
                             pPatternMemory,
                             junk );
  if(err != SB_OK) {
    return err;
  }
  return err;
}

sbStatus_t
soc_sbx_g2p3_cls_clear_pattern_memory(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_schema_t *pSchema,
                            soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory)
{
  uint32_t uIndex=0;
  uint32_t uRSet=0, uRc=0, uSb=0, uMaxSb, uMaxRs;
  sbZfFe2000RcSbPattern_t *pSbPattern = NULL;

  uMaxSb = pSchema->layout.uNumRulesPerRs / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;
  if (SOC_IS_SBX_FE2KXT(pFe->unit)){
      uMaxRs = 1;
  }else{
      uMaxRs = pSchema->layout.uNumRs;
  }

  for(uRSet=0; uRSet < uMaxRs;uRSet++ ) {
    for (uRc=0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
      for (uSb=0; uSb < uMaxSb; uSb++) {
        pSbPattern = pPatternMemory->pSbPattern[uRSet][uRc][uSb];
        for(uIndex=0; uIndex < pSchema->uInstCount; uIndex++) {
          sbZfFe2000RcSbPattern_InitInstance(&pSbPattern[uIndex]);
        }
      }
    }
  }


  SB_MEMSET((void *)pPatternMemory->pVector,0x0, sizeof(uint8_t)*(pSchema->uInstCount/8)+1);
  return SB_OK;
}

void
EmitMatchLpm(sbZfFe2000RcSbPattern_t *pPattern,
             uint32_t *puOffset,
             uint32_t  uRuleId,
             uint32_t uMatchLength,
             uint32_t uDontCareLength,
             soc_sbx_g2p3_bit_vector_t *pVector)
{
  int uIndex =0;
  uint32_t uOffset = *puOffset;
  uint32_t uValue = 0;

  /* program the don't care bits */
  for(uIndex= 0; uIndex < uDontCareLength; uIndex++) {
 #ifdef CLS_PAT_DEBUG
    patDebugSetBit(uOffset);
 #endif
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 1);
  }

  /* mask to exact match transition */
  sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 0);

  /* program the prefix from lsb to msb */
  for(uIndex= 0; uIndex < uMatchLength; uIndex++) {
    uValue = getLpmBit(&pVector[0],uIndex,uMatchLength);
#ifdef CLS_PAT_DEBUG
    if(uValue) {
      patDebugSetBit(uOffset);
    }
 #endif
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, uValue);
  }
  *puOffset = uOffset;
}

void
DecodeMatchLpm(sbZfFe2000RcSbPattern_t *pPattern,
             uint32_t *puOffset,
             uint32_t  uRuleId,
             uint32_t *uMatchLength,
             uint32_t uWidth,
             soc_sbx_g2p3_bit_vector_t *pVector)
{
  int uIndex =0;
  uint32_t uOffset = *puOffset;
  uint8_t uBitValue = 1;
  uint32_t uDontCareLength = 0;

  /* decode the don't care bits and the exact match transition bit */
  while(uBitValue) {
    sbZfFe2000RcSbGetRuleBit(&pPattern[uOffset++], uRuleId, &uBitValue);
    uDontCareLength++;
    if (uDontCareLength > uWidth){
        break;
    }
  }
  /* don't count the transition bit */
  uDontCareLength--;
  *uMatchLength = uWidth - uDontCareLength;

  /* decode the prefix */
  for(uIndex= 0; uIndex < *uMatchLength; uIndex++) {
    sbZfFe2000RcSbGetRuleBit(&pPattern[uOffset++],uRuleId, &uBitValue);
    setLpmBit(&pVector[0],uIndex,*uMatchLength, uBitValue);
  }
  *puOffset = uOffset;
}
#if 0
void
EmitExactMatchEQ(sbZfFe2000RcSbPattern_t *pPattern,
                 uint32_t *puOffset,
                 uint32_t uRuleId,
                 uint32_t  uSrcStart,
                 uint32_t uWidth,
                 soc_sbx_g2p3_bit_vector_t *pVector)
{
  int uIndex =0;
  uint32_t uOffset = *puOffset;
  uint32_t uBitValue   =0;

  /* Disable the Wild Card in the Equal Operation with a zero */
  sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 0);

  for(uIndex= 0; uIndex < uWidth; uIndex++)  {
    uBitValue   = getBit(pVector,uSrcStart++);
#ifdef CLS_PAT_DEBUG
    if(uBitValue) {
       patDebugSetBit(uOffset);
    }
#endif
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, uBitValue);
  }
  *puOffset = uOffset;
} /* end EmitExactMatchEQ */
#endif

/*
 * EmitFromVector
 *
 * This function outputs a number of bits to pattern memory
 * Its the same as EmitExactMatchEQ without the initial bit for wildcard/exact
 */

void
EmitFromVector(sbZfFe2000RcSbPattern_t *pPattern,
                 uint32_t *puOffset,
                 uint32_t uRuleId,
                 uint32_t  uSrcStart,
                 uint32_t uWidth,
                 soc_sbx_g2p3_bit_vector_t *pVector)
{
  int uIndex =0;
  uint32_t uOffset = *puOffset;
  uint32_t uBitValue   =0;

  for(uIndex= 0; uIndex < uWidth; uIndex++)  {
    uBitValue   = getBit(pVector,uSrcStart++);
#ifdef CLS_PAT_DEBUG
    if(uBitValue) {
        patDebugSetBit(uOffset);
    }
#endif
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, uBitValue);
  }
  *puOffset = uOffset;
} /* end EmitFromVector */




void EmitExactMatch32(sbZfFe2000RcSbPattern_t *pPattern,
                      uint32_t *puOffset,
                      uint32_t uRuleId,
                      uint32_t uMsb,
                      uint32_t uLsb,
                      uint32_t uValue,
                      uint32_t isC2)
{
  uint32_t uOffset    = *puOffset;

  if (isC2){
    /* emit exact match transition */
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 0);

    EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uValue);
  }else{
    /* Disable the Wild Card in the Equal Operation with WildCard */
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 0);
    EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uValue);
  }

  *puOffset = uOffset;
}

void DecodeExactMatch32(sbZfFe2000RcSbPattern_t *pPattern,
                      uint32_t *puOffset,
                      uint32_t uRuleId,
                      uint32_t uMsb,
                      uint32_t uLsb,
                      uint32_t uValue,
                      uint32_t isC2)
{
  uint32_t uOffset    = *puOffset;

  if (isC2){
    /* emit exact match transition */
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 0);

    EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uValue);
  }else{
    /* Disable the Wild Card in the Equal Operation with WildCard */
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 0);
    EmitBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uValue);
  }

  *puOffset = uOffset;
}
void
EmitPbmp(sbZfFe2000RcSbPattern_t *pPattern,
         uint32_t *puOffset,
         uint32_t uRuleId,
         uint64_t  uuPbmp,
         uint32_t uWidth)
{
    int uIndex =0;
    uint32_t uOffset = *puOffset;
    uint32_t uBitValue   =0;

    for(uIndex= 0; uIndex < uWidth; uIndex++)  {
        uBitValue = ((uuPbmp >> (uWidth - 1 - uIndex)) & 0x1) ? 1 : 0;;
        sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, uBitValue);
    }
    *puOffset = uOffset;
} /* end EmitPbmp */

void
DecodePbmp(sbZfFe2000RcSbPattern_t *pPattern,
         uint32_t *puOffset,
         uint32_t uRuleId,
         uint64_t *uuPbmp,
         uint8_t  *bPortAny,
         uint32_t uWidth)
{
    int uIndex =0;
    uint32_t uOffset = *puOffset;
    uint8_t uBitValue   =0;
    *uuPbmp = 0;

    *bPortAny=1;
    for(uIndex= 0; uIndex < uWidth; uIndex++)  {
        sbZfFe2000RcSbGetRuleBit(&pPattern[uOffset++],uRuleId, &uBitValue);
        *uuPbmp <<= 1;
        *uuPbmp |= uBitValue;
        *bPortAny &= uBitValue;
    }
    *puOffset = uOffset;
} /* end DecodePbmp */

void
EmitWildCardEQ(sbZfFe2000RcSbPattern_t *pPattern,
               uint32_t *puOffset,
               uint32_t uRuleId,
               int32_t  uMsb,
               int32_t  uLsb,
               uint32_t isC2)
{

  int uIndex =0;
  uint32_t uOffset = *puOffset;
  uint32_t uPbit =0;

  /* emit the wild card */
#ifdef CLS_PAT_DEBUG
  patDebugSetBit(uOffset);
#endif
  sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 1);

  /* for CA zero the pattern, for C2 all ones (uses LPM) */
  uPbit = (isC2) ? 1 : 0;

  for(uIndex= uMsb; uIndex >= uLsb; uIndex--) {
#ifdef CLS_PAT_DEBUG
    if (isC2){
        patDebugSetBit(uOffset);
    }
#endif
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 1);
  }
  *puOffset = uOffset;
} /* end EmitWildCardEQ */

void
DecodeWildCardEQOrExactMatch(sbZfFe2000RcSbPattern_t *pPattern,
               uint32_t *puOffset,
               uint32_t uRuleId,
               int32_t  uMsb,
               int32_t  uLsb,
               uint32_t *uFieldValue,
               uint8_t  *bFieldAny)
{

  uint32_t uOffset = *puOffset;

  /* Check for wildcard bit */
  sbZfFe2000RcSbGetRuleBit(&pPattern[uOffset++],uRuleId, bFieldAny);

  if (*bFieldAny){
    uOffset += (uMsb - uLsb) + 1;
    *puOffset = uOffset;
    return;
  }

  /* decode exact match */
  DecodeBits32(pPattern, &uOffset, uRuleId, uMsb, uLsb, uFieldValue);

  *puOffset = uOffset;
} /* end DecodeWildCardEQOrExactMatch */

void
DecodeWildCardEQ(sbZfFe2000RcSbPattern_t *pPattern,
               uint32_t *puOffset,
               uint32_t uRuleId,
               int32_t  uMsb,
               int32_t  uLsb,
               uint32_t isC2)
{

  int uIndex =0;
  uint32_t uOffset = *puOffset;
  uint32_t uPbit =0;

  /* emit the wild card */
#ifdef CLS_PAT_DEBUG
  patDebugSetBit(uOffset);
#endif
  sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 1);

  /* for CA zero the pattern, for C2 all ones (uses LPM) */
  uPbit = (isC2) ? 1 : 0;

  for(uIndex= uMsb; uIndex >= uLsb; uIndex--) {
#ifdef CLS_PAT_DEBUG
    if (isC2){
        patDebugSetBit(uOffset);
    }
#endif
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 1);
  }
  *puOffset = uOffset;
} /* end DecodeWildCardEQ */

/*
 * EmitClearBits
 *
 * This function outputs a number of clear bits to the pattern memory
 * Its the same as EmitWildCardEQ without the wildcard bit setting
 */


void
EmitClearBits(sbZfFe2000RcSbPattern_t *pPattern,
               uint32_t *puOffset,
               uint32_t uRuleId,
               int32_t  uMsb,
               int32_t  uLsb )
{

  int uIndex =0;
  uint32_t uOffset = *puOffset;

  /* just 0 the pattern */
  for(uIndex= uMsb; uIndex >= uLsb; uIndex--) {
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, 0);
  }
  *puOffset = uOffset;
}




void
EmitBits32(sbZfFe2000RcSbPattern_t *pPattern,
            uint32_t *puOffset,
            uint32_t uRuleId,
            int32_t  uMsb,
            int32_t  uLsb,
            uint32_t uFieldValue )
{

  int uIndex =0;
  uint32_t uBitValue =0;
  uint32_t uOffset = *puOffset;

  for(uIndex=uMsb; uIndex >= uLsb; --uIndex) {
    uBitValue   = getBitFromWord(uFieldValue,uIndex);
#ifdef CLS_PAT_DEBUG
    if(uBitValue) {
        patDebugSetBit(uOffset);
    }
#endif

    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffset++],uRuleId, uBitValue);
  }
  *puOffset = uOffset;
}

void
DecodeBits32(sbZfFe2000RcSbPattern_t *pPattern,
            uint32_t *puOffset,
            uint32_t uRuleId,
            int32_t  uMsb,
            int32_t  uLsb,
            uint32_t *uValue )
{

  int uIndex =0;
  uint8_t uBitValue =0;
  uint32_t uOffset = *puOffset;

  for(uIndex=uMsb; uIndex >= uLsb; --uIndex) {
    sbZfFe2000RcSbGetRuleBit(&pPattern[uOffset++],uRuleId, &uBitValue);
    *uValue |= (uBitValue & 1) << uIndex;
  }
  *puOffset = uOffset;
}
/* PRIVATE Functions */
/* assume network byte order of the values */
uint32_t
getBit(soc_sbx_g2p3_bit_vector_t *v, uint32_t uBitid)
{
  uint8_t *parray = (uint8_t *)v;
  uint32_t uIndex = uBitid/8;
  uint32_t uBid   = 7-(uBitid-uIndex*8);
  if((parray[uIndex] >> uBid) & 0x1) {
    return 1;
  }
  return 0;
}
uint32_t
getLpmBit(soc_sbx_g2p3_bit_vector_t *v, uint32_t uBit, uint32_t uLpmLength)
{
  uint8_t *parray = (uint8_t *)v;
  uint32_t uIndex =0, uBid=0;

  /*
   * transform the lpm match from the reverse
   *     <----------  that's the direction we do
   * encoding.
   */
  uBit   = (uLpmLength-uBit)-1;

  /* get the byte index */
  uIndex = uBit/8;

  /* Get the bit from the bit index */
  uBid   = 7- (uBit-(uIndex*8) ) %8;

  if( (parray[uIndex] >> uBid)  & 0x1) {
    return 1;
  }
  return 0;
}
void
setLpmBit(soc_sbx_g2p3_bit_vector_t *v, uint32_t uBit, uint32_t uLpmLength, uint8_t uBitValue)
{
  uint8_t *parray = (uint8_t *)v;
  uint32_t uIndex =0, uBid=0;

  /*
   * transform the lpm match from the reverse
   *     <----------  that's the direction we do
   * encoding.
   */
  uBit   = (uLpmLength-uBit)-1;

  /* get the byte index */
  uIndex = uBit/8;

  /* Get the bit from the bit index */
  uBid   = 7- (uBit-(uIndex*8) ) %8;

  parray[uIndex] |= ((uBitValue & 0x1) << uBid);

}

uint32_t
getBitFromWord(uint32_t v, uint32_t uBitid)
{
  if( (v >> uBitid)  & 0x1 ) {
    return 1;
  }
  return 0;
}

void
MakeMacArrayFromUint64(uint64_t uMac, uint8_t *pArray)
{
  int i =0;

  for(i=5; i  >=0; i--) {
    pArray[5-i] = (uMac >> (i*8)) & 0xFF;
  }
}
void
MakeUint64FromMacArray(uint8_t *pArray, uint64_t *uMac)
{
  int i =0;

  *uMac = 0;
  for(i=5; i  >=0; i--) {
    *uMac <<= 8;
    *uMac |= pArray[5-i];
  }
}


int
soc_sbx_g2p3_cls_rule_capacity_get(uint32_t unit,
                                       soc_sbx_g2p3_cls_schema_enum_t sType,
                                       uint32_t *pRuleCount)
{
    uint32_t isC2 = SOC_IS_SBX_FE2KXT(unit);
    soc_sbx_g2p3_schema_layout_t *pLayout;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);

    if(pFe->pClsComp->uMaxSchema <= sType) {
        return SOC_E_PARAM;
    }

    pLayout = &pFe->pClsComp->schemadb[sType].layout;
    if (isC2) { 
#ifndef DYNAMIC_MEMMGR
      if (pLayout->uBankedMode){
          /* account for banking */
          *pRuleCount = (pLayout->uNumRs/2) * pLayout->uNumRulesPerRs;
          return SOC_E_NONE;
      }
      *pRuleCount = ((pLayout->uNumRs - pFe->pClsComp->uMaxSchema) + (pLayout->uNumRs - 2)) * pLayout->uNumRulesPerRs;
#else
      if (pLayout->uBankedMode){
          if (pFe->pClsComp->uMaxSchema == 2) {
              /* account for banking */
              /* 2 rulesets for dummy group of other schemas and 
                 1 ruleset for dummy group of this schema */ 
              *pRuleCount = (pFe->pClsComp->uMaxRs - 3) * 
                                       pLayout->uNumRulesPerRs;
           } else {
              /* 4 rulesets for dummy group of other schemas and 
                 1 ruleset for dummy group of this schema */ 
              *pRuleCount = (pFe->pClsComp->uMaxRs - 5) * 
                                       pLayout->uNumRulesPerRs;
           }
           return SOC_E_NONE;
      }
      if (pFe->pClsComp->uMaxSchema == 2) {
          *pRuleCount = (19 + 18) * pLayout->uNumRulesPerRs;
      } else {
          *pRuleCount = (18 + 16) * pLayout->uNumRulesPerRs;
      }
#endif

    }else{
          *pRuleCount = pLayout->uNumRs * pLayout->uNumRulesPerRs;
    }
    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cls_is_rule_capacity_available(uint32_t unit,
                               soc_sbx_g2p3_cls_schema_t *pSchema,
                               uint32_t uRuleCount,
                               uint8_t *bAvailable)
{
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_schema_enum_t sType;
    uint32_t uRsSize;
    int32_t uRc;
    uint32_t uMemUsed = 0, uRsUsed = 0;
  
    SB_ASSERT(pFe);

    pClsComp = pFe->pClsComp;
    sType = pSchema->sType;
    for (uRc = G2P3_MAX_RC_INSTANCE - 1; uRc >= 0 ; uRc--) {
        uRsUsed = pClsComp->usagePerSchema[sType][uRc].uRsUsed 
                     + uRsUsed;
        uRsUsed = pClsComp->usagePerSchema[sType][uRc].uRsTobeUsed 
                     + uRsUsed;
    }
    
    if (uRsUsed * G2P3_MAX_CLS_RULES_PER_RULESET_C2 > uRuleCount) {
        *bAvailable = 1;
        return SOC_E_NONE;
    }

    uRsUsed = 0;
    *bAvailable = 0;
    uRsSize = pSchema->uInstCount;
    for (uRc = G2P3_MAX_RC_INSTANCE - 1; uRc >= 0 ; uRc--) {
        uMemUsed = pClsComp->usagePerRc[uRc].uMemUsed
                   + pClsComp->usagePerRc[uRc].uMemTobeUsed;
        uRsUsed  = pClsComp->usagePerRc[uRc].uRsUsed
                   + pClsComp->usagePerRc[uRc].uRsTobeUsed;

        if ((uRsUsed + 1) <= pClsComp->usagePerRc[uRc].uRsMax &&
            (uMemUsed + uRsSize) < pClsComp->usagePerRc[uRc].uMemTotal) {
            /* Ruleset and mem space is available for new Rs */
            /* Update usage details */
            pClsComp->usagePerRc[uRc].uRsTobeUsed++;
            pClsComp->usagePerRc[uRc].uMemTobeUsed = uRsSize
                                + pClsComp->usagePerRc[uRc].uMemTobeUsed;
            pClsComp->usagePerSchema[sType][uRc].uRsTobeUsed++;
            pClsComp->usagePerSchema[sType][uRc].uMemTobeUsed = uRsSize
                      + pClsComp->usagePerSchema[sType][uRc].uMemTobeUsed;
            *bAvailable = 1;
            break;
        }
    }
    
    return SB_OK;

}

int
soc_sbx_g2p3_cls_schema_to_program(soc_sbx_g2p3_state_t *pFe,
                                  soc_sbx_g2p3_cls_schema_enum_t sType,
                                  uint8_t *pProgNumber)
{
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  int index;
  soc_sbx_g2p3_cls_schema_enum_t type=0;

  SB_ASSERT(pFe);
  SB_ASSERT(pProgNumber);

  pClsComp = pFe->pClsComp;

  switch (sType) {
      case SB_G2P3_FE_CLS_G2P3_SCHEMA:
      case SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA:
          type = SB_G2P3_FE_CLS_G2P3_SCHEMA;
          break;
      case SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA:
      case SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA:
          type = SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA;
          break;
      default:
          SB_ASSERT(0);
  }

  for(index=0; index < G2P3_MAX_CLS_PROGRAMS; index++) {
    if( (pClsComp->pProgAtrib[index]->uActive) &&
        (pClsComp->pProgAtrib[index]->sType == type) &&
        (pClsComp->pProgAtrib[index]->uValid)) {
      *pProgNumber = pClsComp->pProgAtrib[index]->uNumber;
      return SOC_E_NONE;
    }
  }
  return SOC_E_NOT_FOUND;
}

int
soc_sbx_g2p3_cls_get_ctr_offset(soc_sbx_g2p3_state_t *pFe,
                                uint8_t uPhyProg,
                                uint32_t *pOffset)
{
  soc_sbx_g2p3_cls_prog_attr_t *pProgAttr = NULL;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_schema_t *pSchema = NULL;

  SB_ASSERT(pFe);
  SB_ASSERT(pOffset);

  if((G2P3_MAX_CLS_PROGRAMS-1) < uPhyProg){
    return SOC_E_PARAM;
  }

  pClsComp = pFe->pClsComp;
  pProgAttr = pClsComp->pProgAtrib[uPhyProg];

  if(!pProgAttr->uValid){
    return SOC_E_PARAM;
  }

  pSchema = &pClsComp->schemadb[pProgAttr->sType];

  *pOffset = pClsComp->uMaxRs * \
             pSchema->layout.uNumRcInstance *\
             pSchema->layout.uNumRulesPerRs;

  /* For CA the layout of the CRR register is:
   * 10  9  8  7  6  5  4  3  2  1  0
   * DB | set| SB| filter
   *.the upper most bit (DB) toggles between A/B banks of
   * the program   For this reason force base offset to 1024.
   */
  if (SAND_HAL_IS_FE2KXT(pFe->regSet)==0) {
     *pOffset = 1024;
  }

  if(SB_G2P3_FE_CLS_G2P3_SCHEMA == pProgAttr->sType){
      *pOffset *= (uPhyProg - G2P3_INGRESS_PROGRAM_BASE);
  } else if(SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA == pProgAttr->sType) {
      /* egress schema */
      *pOffset *= (uPhyProg - G2P3_EGRESS_PROGRAM_BASE);
  } else {
    SB_ASSERT(0);
  }

  return SOC_E_NONE;
}

int
soc_sbx_g2p3_cls_get_stby_prog_number(soc_sbx_g2p3_state_t *pFe,
                                      soc_sbx_g2p3_cls_schema_t *pSchema,
                                      uint8_t *pProgramNumber)
{
  uint8_t uLogProgNumber=0;
  SB_ASSERT(pFe);
  SB_ASSERT(pSchema);
  SB_ASSERT(pProgramNumber);
 
  if(pSchema->sType >= pFe->pClsComp->uMaxSchema) {
    return SOC_E_PARAM;
  }

  if (SAND_HAL_IS_FE2KXT(pFe->regSet)!=0) {
      soc_sbx_g2p3_cls_schema_to_logprogram(pFe, pSchema->sType, &uLogProgNumber);
      *pProgramNumber = pFe->pClsComp->sbyProgNumber[uLogProgNumber];
  } else { 
      *pProgramNumber = pFe->pClsComp->sbyProgNumber[pSchema->sType];
  }
  return SOC_E_NONE;
}


int
soc_sbx_g2p3_cls_schema_to_logprogram(soc_sbx_g2p3_state_t *pFe,
                                      soc_sbx_g2p3_cls_schema_enum_t sType,
                                      uint8_t *pLogProgNumber)
{
    switch(sType) {
    case SB_G2P3_FE_CLS_G2P3_SCHEMA:
        *pLogProgNumber = G2P3_INGRESS_LOGICAL_PROGRAM_NUMBER;
        break;   
    case SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA:
        *pLogProgNumber = G2P3_EGRESS_LOGICAL_PROGRAM_NUMBER;
        break;   
    case SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA:
        *pLogProgNumber = G2P3_INGRESS_LOGICAL_PROGRAM_NUMBER;
        break;   
    case SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA:
        *pLogProgNumber = G2P3_EGRESS_LOGICAL_PROGRAM_NUMBER;
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;

}

int
soc_sbx_g2p3_cls_template_fields_enable(
                         sbFe2000ClsTemplate_t *pTemplate,
                         soc_sbx_g2p3_ace_qset_t qset,
                         uint8_t bEnableAll)
{
    uint8_t index;

#ifdef DYNAMIC_GROUP_DEBUG
    soc_cm_print("New Template\n");
#endif
    SBX_FIELD_QSET_ADD(qset, sbxFieldQualifyDbType);
    for (index = 0 ; index < pTemplate->uCount; index++) {
        if ((bEnableAll == TRUE) || 
            SBX_FIELD_QSET_TEST(qset, pTemplate->ops[index].uQualifier)) {
            pTemplate->ops[index].bEnabled = TRUE;
#ifdef DYNAMIC_GROUP_DEBUG
            soc_cm_print("Enabled %s\n", pTemplate->ops[index].sFieldName);
#endif
        } else {
            pTemplate->ops[index].bEnabled = FALSE;
        }
    }
    return SOC_E_NONE;
}



int
soc_sbx_g2p3_cls_print_ruleset(soc_sbx_g2p3_state_t *pFe)
{
   soc_sbx_g2p3_cls_ruleset_t *pRsDb;
   uint8_t index;

   pRsDb = pFe->pClsComp->sRsDb;
  
   soc_cm_print("Rulesets \n uRs -- uRc -- uBaseAdd -- uProg -- uPrev -- ");
   soc_cm_print("uNext -- uCom -- uTer -- uDum -- uAlloc\n");
   for(index=0;index < (2*pFe->pClsComp->uMaxRs); index++) {
      pRsDb = &pFe->pClsComp->sRsDb[index];
      soc_cm_print("%d   **  %d   **  %4x  **  %d  **  %d  **",
                           index, pRsDb->uRcInstance, pRsDb->uBaseAddress,
                           pRsDb->uProgNumber, pRsDb->uPrevRs);
      soc_cm_print("%d  **  %d  **  %d  ** %d  **  %d\n",
                            pRsDb->uNextRs,  pRsDb->uCommitted,
                            pRsDb->uTerminal, pRsDb->uDummy, pRsDb->uAlloc);
   }
   return 0;
}
