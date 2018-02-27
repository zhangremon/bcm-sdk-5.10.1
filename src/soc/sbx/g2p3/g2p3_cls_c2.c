
/*
 * $Id: g2p3_cls_c2.c 1.11.46.1 Broadcom SDK $
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
#include <soc/sbx/fe2k/sbFe2000Driver.h>
#include <soc/sbx/fe2k/sbFe2000ClsComp.h>
#include <soc/sbx/fe2k/sbFe2000Driver.h>
#include <soc/sbx/fe2kxt/sbZfC2RcDmaFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcDmaFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000RcSbPattern.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcResultResolverEntry.hx>
#include <soc/sbx/fe2kxt/sbFe2000XtImfDriver.h>
#include <shared/mem_avl.h>
#include <soc/sbx/g2p3/g2p3_cls_int.h>
#include <soc/sbx/error.h>

#define DYNAMIC_MEMMGR
#define PUBLIC
#define PRIVATE static

#define MAX_CLS_INSTR_SIZE      (400) /* in reality we can find it on fly */
#define MASK_N(X)               ( ((X) == 32) ? (0xFFFFFFFF) : ( (1 << (X))  -1) )
#define CLS_OP_WIDTH(x)         (x->uStart-x->uEnd+1)
#define CLS_OP_ENCODE_RESUME(x)        ( (x)++ )

#define SB_G2P3_MAX_G2_PORTS  32

#define CLS_OP_DONTCARE  (0xFFFF)
typedef struct {
    uint32_t uPhyProg;
    uint32_t uLogProg;
    uint8_t uActive;
    uint32_t uBaseAddress;
}soc_sbx_g2p3_cls_wb_prog_t;

typedef struct {
    soc_sbx_g2p3_cls_wb_prog_t prog[G2P3_MAX_LOGICAL_PROGRAMS];
    sbDmaMemoryHandle_t dmaHandle;
    uint32_t *dmaBuffer;
}soc_sbx_g2p3_cls_wb_rc_t;

/* FOR DEBUG int sb_log_verbose = 3 or whatever */

/*#define RULESET_DEBUG 1*/

extern uint8_t sbG2FePortArray[];

/* Warmboot support */
#ifdef BCM_WARM_BOOT_SUPPORT

typedef soc_sbx_g2p3_cls_ruleset_t soc_sbx_g2p3_cls_wb_ruleset_t;

typedef struct soc_sbx_g2p3_cls_wb_layout_s{
    soc_sbx_g2p3_cls_wb_ruleset_t ruleset[0];
}soc_sbx_g2p3_cls_wb_layout_t;

typedef struct soc_sbx_g2p3_cls_cache_s {
    uint32_t                wb_hdl;
    uint32_t                scache_size;
} soc_sbx_g2p3_cls_cache_t;

soc_sbx_g2p3_cls_cache_t _cls_cache[BCM_MAX_NUM_UNITS];

sbStatus_t
soc_sbx_g2p3_cls_wb_init_c2(soc_sbx_g2p3_state_t *pFe, void **wbOut);
#define G2P3_CLS_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define G2P3_CLS_WB_CURRENT_VERSION            G2P3_CLS_WB_VERSION_1_0

/* Is Level 2 warmboot configured - */
#define L2CACHE_WB_L2_CONFIGURED(unit) (_cls_cache[unit].scache_size != 0)

#endif

/* TODO: split these into CA and C2 */
extern int
soc_sbx_g2p3_cls_schema_layout_init(soc_sbx_g2p3_state_t *pFe, uint32_t isC2);
extern sbStatus_t
soc_sbx_g2p3_cls_allocate_dma_slabs( soc_sbx_g2p3_state_t *pFe,
                               sbCommonConfigParams_p_t    cParams,
                               uint32_t uMaxSlabSize,
                               uint32_t isC2);
/* TODO: end split I*/
extern int
soc_sbx_g2p3_cls_schema_to_logprogram(soc_sbx_g2p3_state_t *pFe,
                                      soc_sbx_g2p3_cls_schema_enum_t sType,
                                      uint8_t *pLogProgNumber);
extern sbStatus_t 
soc_sbx_g2p3_ace_wb_rule_recover(soc_sbx_g2p3_state_t *pFe,
                                       soc_sbx_g2p3_cls_schema_t *pSchema,
                                       uint32_t uGroupId,
                                       uint32_t uRule,
                                       soc_sbx_g2p3_cls_ace_pattern_t *pPattern);
extern void clsdmatest( soc_sbx_g2p3_state_t *pFe ,
                  sbCommonConfigParams_p_t    cParams);

extern void soc_sbx_g2p3_cls_dma_cb(sbFe2000DmaOp_t  *pOp);

PRIVATE
sbStatus_t soc_sbx_g2p3_cls_schema_init_c2(soc_sbx_g2p3_state_t *pFe);

PRIVATE
void soc_sbx_g2p3_cls_g2p3_schema_encoder_c2(
    soc_sbx_g2p3_cls_schema_p_t         pSchemaHandler, 
    uint32_t                            uRIndex,
    void                                *pRule, 
    soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory);
PRIVATE
void soc_sbx_g2p3_cls_g2p3_v6_schema_encoder_c2(soc_sbx_g2p3_cls_schema_p_t pSchemaHandler,
                                uint32_t                   uRIndex,
                                void                       *pRule,
                                soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory);

PRIVATE
void soc_sbx_g2p3_cls_g2p3_schema_decoder_c2(
    soc_sbx_g2p3_cls_schema_p_t         pSchemaHandler, 
    uint32_t                            uRIndex,
    void                                *pRule, 
    sbZfFe2000RcSbPattern_t    *pPatternMemory[],
    uint32_t *isNull);
PRIVATE
void soc_sbx_g2p3_cls_g2p3_v6_schema_decoder_c2(soc_sbx_g2p3_cls_schema_p_t pSchemaHandler,
                                uint32_t                   uRIndex,
                                void                       *pRule,
                                sbZfFe2000RcSbPattern_t    *pPatternMemory[],
                                uint32_t *isNull);

PRIVATE
sbStatus_t soc_sbx_g2p3_cls_rr_init(soc_sbx_g2p3_state_t *pFe);

PRIVATE
sbStatus_t
soc_sbx_g2p3_cls_program_adjuster(soc_sbx_g2p3_state_t *pFe,
                                 soc_sbx_g2p3_cls_schema_t *pSchema,
                                 uint32_t uBaseRs,
                                 uint32_t uRc);
PRIVATE
sbStatus_t 
soc_sbx_g2p3_cls_ruleset_commit(soc_sbx_g2p3_state_t *pFe, 
                                soc_sbx_g2p3_cls_schema_t *pPtr,
                                soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory,
                                uint32_t uRs);

PRIVATE
sbStatus_t 
soc_sbx_g2p3_cls_ruleset_relink(soc_sbx_g2p3_state_t *pFe,
                               uint32_t uRs,
                               uint32_t uNextRs,
                               uint32_t uDmaBlockOffset,
                               uint32_t uInstOffset,
                               uint8_t uToLinkList);

int
soc_sbx_g2p3_cls_ruleset_linkage_dump(uint32_t unit,
                                      uint8_t uProgram,
                                      uint8_t uLogProgramNumber,
                                      soc_sbx_g2p3_cls_schema_enum_t sType,
                                      uint32_t uBank);

sbStatus_t
soc_sbx_g2p3_cls_commit_push_to_rc1_dummy(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_schema_t *pSchema,
                            soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            int16_t   uGroupId);
sbStatus_t
soc_sbx_g2p3_cls_commit_push_to_rc1(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_group_t *pClsGroups,
                            soc_sbx_g2p3_cls_schema_enum_t sType,
                            soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            uint32_t uBank,
                            uint32_t  uGroupCount,
                            pfRuleCb pfRuleIter,
                            pfFinalizeCb pfFinalize);
sbStatus_t
soc_sbx_g2p3_cls_commit_push_to_rc0(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_group_t *pClsGroups,
                            soc_sbx_g2p3_cls_schema_enum_t sType,
                            soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            uint32_t uBank,
                            uint32_t   uGroupCount,
                            pfRuleCb pfRuleIter,
                            pfFinalizeCb pfFinalize);

int
soc_sbx_g2p3_cls_commit_group_c2(soc_sbx_g2p3_state_t *pFe,
                          soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                          soc_sbx_g2p3_cls_rule_db_e_t eDb,
                          uint32_t uBank,
                          uint8_t uLogProgNumber,
                          soc_sbx_g2p3_cls_group_t *pClsGroup,
                          pfRuleCb pfRuleIter,
                          pfFinalizeCb pfFinalize,
                          uint32_t uLastCommitRs,
                          int8_t iRc,
                          uint32_t *uFirstRs);

PRIVATE
sbStatus_t
soc_sbx_g2p3_cls_ruleset_alloc(soc_sbx_g2p3_state_t *pFe,
                               uint8_t uProgramNumber,
                               uint8_t uLogProgramNumber,
                               soc_sbx_g2p3_cls_schema_t *pSchema,
                               int16_t uGroupId,
                               int16_t uNextGroupId,
                               uint32_t uBank,
                               uint32_t uNumRs,
                               int8_t   iRcPref,
                               int8_t   *iRcPrefNext,
                               int8_t *uAllocCondition);

PRIVATE
int 
soc_sbx_g2p3_cls_ruleset_rc_alloc(soc_sbx_g2p3_state_t *pFe,
                                uint8_t uProgramNumber, 
                                uint8_t uLogProgramNumber,
                                soc_sbx_g2p3_cls_schema_t *pSchema,
                                int16_t uGroupId,
                                uint32_t uRc,
                                uint32_t uBank,
                                uint32_t *pRs,
                                uint32_t *pOffset); 
PRIVATE
int 
soc_sbx_g2p3_cls_free_ruleset(soc_sbx_g2p3_state_t *pFe,
                                uint32_t uRs); 
PRIVATE
int
soc_sbx_g2p3_cls_ruleset_offset_get(soc_sbx_g2p3_state_t *pFe,
                                        uint32_t uRs,
                                        uint32_t *pRsOffset);

int soc_sbx_g2p3_cls_ruleset_free_count_get(soc_sbx_g2p3_state_t *pFe,
                                            uint32_t uSchemaSize,
                                            uint32_t uBank,
                                            uint32_t *pRc0FreeRsCount,
                                            uint32_t *pRc1FreeRsCount);

int
soc_sbx_g2p3_cls_ruleset_get_free(soc_sbx_g2p3_state_t *pFe, 
                                      uint32_t *pTotalRs);

int
soc_sbx_g2p3_cls_group_ruleset_identify( soc_sbx_g2p3_state_t *pFe,
                                         soc_sbx_g2p3_cls_schema_enum_t sType,
                                         soc_sbx_g2p3_cls_group_t *pClsGroups,
                                         uint8_t uGroupCount,
                                         uint32_t uLastRs);

PRIVATE
sbStatus_t
soc_sbx_g2p3_cls_group_remove(soc_sbx_g2p3_state_t *pFe,
                soc_sbx_g2p3_cls_schema_t *pSchema,
                soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                soc_sbx_g2p3_cls_rule_db_e_t eDb,
                int16_t uGroupId,
                uint32_t uLastRs,
                pfFinalizeCb pfFinalize);

PRIVATE
int 
soc_sbx_g2p3_cls_ruleset_get_last(soc_sbx_g2p3_state_t *pFe,
                                uint8_t uProgramNumber,
                                uint8_t uLogProgramNumber,
                                soc_sbx_g2p3_cls_schema_enum_t sType,
                                uint32_t uBank,
                                int16_t uGroupId,
                                uint32_t *pRs,
                                uint32_t *pTotalRs);
PRIVATE
int
soc_sbx_g2p3_cls_ruleset_get_first_last(soc_sbx_g2p3_state_t *pFe,
                                 uint8_t uProgramNumber,
                                 uint8_t uLogProgramNumber,
                                 soc_sbx_g2p3_cls_schema_enum_t sType,
                                 uint32_t *pFirstRs,
                                 uint32_t *pLastRs,
                                 uint32_t uRcInstance);

sbStatus_t
soc_sbx_g2p3_cls_update_mem_usage(soc_sbx_g2p3_state_t *pFe,
                                  soc_sbx_g2p3_cls_schema_enum_t sType,
                                  soc_sbx_g2p3_cls_rule_db_e_t eDb);

extern sbStatus_t
soc_sbx_g2p3_ace_ruleset_dirty_cb(soc_sbx_g2p3_state_t *pFe,
                               soc_sbx_g2p3_cls_schema_t *pSchema,
                               void** pCookie,
                               uint32_t* pDirty,
                               uint32_t uRuleCount,
                               uint32_t uRuleIndex,
                               uint32_t uRsOffset);

static sbFe2000ClsTemplateField_t soc_sbx_g2p3_cls_c2_g2p3_template[] =
{
    { "Fragment      ", SB_FE2K_CLS_OP_WEQ      , 310, 310,  0, NULL, 
                                    FALSE, TRUE, sbxFieldQualifyIpFrag },
    { "TCP URG       ", SB_FE2K_CLS_OP_WEQ      , 309, 309,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "L4SRC PORT    ", SB_FE2K_CLS_OP_RANGE    , 287, 272,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyL4SrcPort },
    { "LlcDsapSsap   ", SB_FE2K_CLS_OP_RANGE    , 287, 272,  0, NULL,
                                    TRUE , TRUE, sbxFieldQualifySnap },
    { "TCP ACK       ", SB_FE2K_CLS_OP_WEQ      , 308, 308,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP PSH       ", SB_FE2K_CLS_OP_WEQ      , 307, 307,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP RST       ", SB_FE2K_CLS_OP_WEQ      , 306, 306,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "L4DST PORT    ", SB_FE2K_CLS_OP_RANGE    , 271, 256,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyL4DstPort },
    { "LlcCtrlData   ", SB_FE2K_CLS_OP_RANGE    , 271, 256,  0, NULL,
                                    TRUE , TRUE, sbxFieldQualifyLlc },
    { "TCP SYN       ", SB_FE2K_CLS_OP_WEQ      , 305, 305,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP FIN       ", SB_FE2K_CLS_OP_WEQ      , 304, 304,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "Pri           ", SB_FE2K_CLS_OP_WEQ      ,  63,  61,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyVlanPri },
    { "DSCP          ", SB_FE2K_CLS_OP_WEQ      , 303, 298,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyDSCP },
    { "ECN           ", SB_FE2K_CLS_OP_WEQ      , 297, 296,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyECN },
    { "Protocol      ", SB_FE2K_CLS_OP_WEQ      , 295, 288,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyIpProtocol },
    { "IPV4 SA       ", SB_FE2K_CLS_OP_LPM      , 191, 160,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifySrcIp },
    { "IPV4 DA       ", SB_FE2K_CLS_OP_LPM      , 159, 128,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyDstIp },
    { "EtherType     ", SB_FE2K_CLS_OP_WEQ      , 127, 112,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyEtherType },
    { "SMAC_47_0     ", SB_FE2K_CLS_OP_LPM      , 111,  64,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifySrcMac },
    { "VSI           ", SB_FE2K_CLS_OP_WEQ      ,  59,  48,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyVlanId },
    { "DMAC_47_0     ", SB_FE2K_CLS_OP_LPM      ,  47,   0,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyDstMac },
    { "DbType        ", SB_FE2K_CLS_OP_WEQ      , 318, 318,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyDbType },
    { "PortMask      ", SB_FE2K_CLS_OP_MULTIPORT, 317, 312, 50,
               &sbG2FePortArray[0], FALSE, TRUE, sbxFieldQualifyPorts },
};

static sbFe2000ClsTemplateField_t soc_sbx_g2p3_cls_c2_g2p3_v6_template[] =
{
    { "DbType        ", SB_FE2K_CLS_OP_WEQ      , 318, 318,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyDbType },
    { "TC            ", SB_FE2K_CLS_OP_WEQ      , 299, 294,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyDSCP },
    { "TCP URG       ", SB_FE2K_CLS_OP_WEQ      , 293, 293,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP ACK       ", SB_FE2K_CLS_OP_WEQ      , 292, 292,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP PSH       ", SB_FE2K_CLS_OP_WEQ      , 291, 291,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP RST       ", SB_FE2K_CLS_OP_WEQ      , 290, 290,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP SYN       ", SB_FE2K_CLS_OP_WEQ      , 289, 289,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "TCP FIN       ", SB_FE2K_CLS_OP_WEQ      , 288, 288,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyTcpControl },
    { "VSI           ", SB_FE2K_CLS_OP_WEQ      , 311, 300,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyVlanId },
    { "L4SRC PORT    ", SB_FE2K_CLS_OP_RANGE    , 287, 272,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyL4SrcPort },
    { "L4DST PORT    ", SB_FE2K_CLS_OP_RANGE    , 271, 256,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyL4DstPort },
    { "L3 Next Header", SB_FE2K_CLS_OP_RANGE    , 271, 256,  0, NULL,
                                    TRUE , TRUE, sbxFieldQualifyIpProtocol },
    { "IPV6 Da       ", SB_FE2K_CLS_OP_LPM      , 255, 128,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifyDstIp6 },
    { "IPV6 Sa       ", SB_FE2K_CLS_OP_LPM      , 127,   0,  0, NULL,
                                    FALSE, TRUE, sbxFieldQualifySrcIp6 },
    { "PortMask      ", SB_FE2K_CLS_OP_MULTIPORT, 317, 312, 50,
               &sbG2FePortArray[0], FALSE, TRUE, sbxFieldQualifyPorts },
};
#ifdef DYNAMIC_MEMMGR_DEBUG
void soc_sbx_g2p3_cls_dump_rsbd_and_mem(soc_sbx_g2p3_state_t *pFe) {
    uint32_t i;
    soc_sbx_g2p3_cls_t *pClsComp = pFe->pClsComp;
    soc_sbx_g2p3_cls_ruleset_t *pRs = pClsComp->sRsDb;

    /*dump RSDB */
    soc_cm_print("RSDB:\n------------------------------\n");
    for (i=0; i<pClsComp->uMaxRs*SB_FE2000_NUM_RC_INSTANCES; i++) {
        soc_cm_print("  Rs: 0x%x Rc: 0x%x COM: 0x%x alloc: 0x%x BA: 0x%x length: 0x%x prog: 0x%x sType: 0x%x, group: 0x%x, nextRs: 0x%x\n",
             i, pRs[i].uRcInstance, pRs[i].uCommitted, pRs[i].uAlloc, pRs[i].uBaseAddress, pRs[i].uSize, pRs[i].uProgNumber, pRs[i].sType, pRs[i].uGroupId, pRs[i].uNextRs);
    }
    soc_cm_print("RC0 MEM_AVL:\n------------------------------\n");
    shr_mem_avl_free_tree_list(pClsComp->mem_avl_ptr[0][0]);
    shr_mem_avl_list_output(pClsComp->mem_avl_ptr[0][0]);
    soc_cm_print("RC1 MEM_AVL:\n------------------------------\n");
    shr_mem_avl_free_tree_list(pClsComp->mem_avl_ptr[1][0]);
    shr_mem_avl_list_output(pClsComp->mem_avl_ptr[1][0]);
}
#endif /* DYNAMIC_MEMMGR_DEBUG */

#ifdef BCM_WARM_BOOT_SUPPORT
static int
soc_sbx_g2p3_cls_wb_layout_get(int unit, soc_sbx_g2p3_cls_wb_layout_t **layout){
    int rv;
    uint32 size;
    soc_wb_cache_t *wbc;

    *layout = NULL;
    rv = soc_scache_ptr_get(unit, _cls_cache[unit].wb_hdl, 
                            (uint8**)&wbc, &size);
    if (rv != SOC_E_NONE){
        return rv; 
    }
    *layout = (soc_sbx_g2p3_cls_wb_layout_t *)wbc->cache;

    return rv;
}

#endif
int
soc_sbx_g2p3_cls_scache_store(soc_sbx_g2p3_state_t *pFe){
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_cls_wb_layout_t *layout;
    sbStatus_t err;
    soc_sbx_g2p3_cls_wb_ruleset_t *pRs;
    int i;
    int unit = pFe->unit;
    soc_sbx_g2p3_cls_t *pClsComp = pFe->pClsComp;

    err = soc_sbx_g2p3_cls_wb_layout_get(unit, &layout);
    if (err != SB_OK) {
        return err;
    }

    soc_scache_handle_lock(unit, _cls_cache[unit].wb_hdl);

    pRs = &(layout->ruleset[0]);

    for (i=0; i<pClsComp->uMaxRs*SB_FE2000_NUM_RC_INSTANCES; i++) {
        /* TODO: optimize scache store */
        sal_memcpy(&(pRs[i]), &(pClsComp->sRsDb[i]), sizeof(soc_sbx_g2p3_cls_ruleset_t));
    }

    soc_scache_handle_unlock(unit, _cls_cache[unit].wb_hdl);

    return SB_OK;
#else
    return SB_OK;
#endif
}

int
soc_sbx_g2p3_cls_scache_recover(int unit, soc_sbx_g2p3_cls_t *pClsComp){
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_cls_wb_layout_t *layout;
    sbStatus_t err;

    err = soc_sbx_g2p3_cls_wb_layout_get(unit, &layout);
    if (err != SB_OK) {
        return err;
    }

    soc_scache_handle_lock(unit, _cls_cache[unit].wb_hdl);

    soc_scache_handle_unlock(unit, _cls_cache[unit].wb_hdl);

    return SB_OK;
#else
    return SB_OK;
#endif
}

sbStatus_t
soc_sbx_g2p3_cls_wb_ruleset_recover(soc_sbx_g2p3_state_t *pFe, 
                                    soc_sbx_g2p3_cls_schema_t *pSchema,
                                    uint32_t uGroupId,
                                    sbZfFe2000RcSbPattern_t *pPatternMemoryDecode[], 
                                    uint32_t* pDmaBuf, 
                                    uint32_t uRsOffset,
                                    uint32_t *uRuleCount){
    sbZfFe2000RcSbPattern_t *pPatternDecode;
    sbZfC2RcDmaFormat_t zDmaFrameDecode;
    uint32_t uSb, uBlock, uNumDmaBlocks, uId, uInst, uInstIndex, uSchemaInst;
    uint32_t uPatternIdx, uPatternWord;
    uint32_t uBuffer[SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS];
    uint32_t uCount = 0;
    soc_sbx_g2p3_cls_ace_pattern_t aceRule;
    uint32_t uRule, isNull;
    sbStatus_t err;

    uNumDmaBlocks = pSchema->uInstCount / SB_FE2000XT_RC_INST_PER_DMA;

    for (uBlock=0; uBlock<uNumDmaBlocks; uBlock++){
        /* unpack DMA buffer into zDmaFrame */
        sbZfC2RcDmaFormat_InitInstance(&zDmaFrameDecode);
        for (uId=0; uId < SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS; uId++){
            uBuffer[uId] = pDmaBuf[(uBlock*SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS)+uId];
        }
        sbZfC2RcDmaFormat_Unpack(&zDmaFrameDecode, (uint8_t*)uBuffer, SB_ZF_C2RCDMAFORMAT_SIZE_IN_BYTES);
        /* recover pattern information */
        for (uInst=0; uInst < SB_FE2000XT_RC_INST_PER_DMA; uInst++){
            uInstIndex = (uBlock * SB_FE2000XT_RC_INST_PER_DMA) + uInst;
            uSchemaInst = uInstIndex % pSchema->uInstCount;
            /* ignore instructions in DmaFrame */
            for (uPatternIdx=0; uPatternIdx < SB_FE2000XT_RC_WORDS_PER_PATTERN * SB_FE2000XT_RC_NUM_SUPERBLOCKS; uPatternIdx++){
                uSb = uPatternIdx / SB_FE2000XT_RC_WORDS_PER_PATTERN;
                uPatternWord = uPatternIdx % SB_FE2000XT_RC_WORDS_PER_PATTERN;
                pPatternDecode =  &(pPatternMemoryDecode[uSb][uInstIndex]);
                sbZfC2RcDmaFormat_GetPatternWord(&(((uint32_t*)pPatternDecode)[3-uPatternWord]),
                                                 uSb,
                                                 uInst,
                                                 uPatternWord,
                                                 (uint32_t*)(&zDmaFrameDecode));
            }
        }
    }


    for (uRule=0; uRule<G2P3_MAX_CLS_RULES_PER_RULESET_C2; uRule++) {
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
                                               uGroupId,
                                               uRule + uRsOffset,
                                               &aceRule);
        if (err) {
            SB_ASSERT(0);
            return err;
        }
        uCount++;
    }
    *uRuleCount = uCount;

    return SB_OK;

}

sbStatus_t
soc_sbx_g2p3_cls_wb_mem_recover(soc_sbx_g2p3_state_t *pFe) {
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_cls_t *pClsComp = pFe->pClsComp;
    soc_sbx_g2p3_cls_wb_layout_t *layout;
    soc_sbx_g2p3_cls_wb_ruleset_t *pRsDb;
    uint32_t i, uRc;
    sbStatus_t err;

    pClsComp = pFe->pClsComp;
    err = soc_sbx_g2p3_cls_wb_layout_get(pFe->unit, &layout);
    if (err != SB_OK) {
        return err;
    }

    soc_scache_handle_lock(pFe->unit, _cls_cache[pFe->unit].wb_hdl);

    pRsDb = &(layout->ruleset[0]);

    for (i=0; i<pClsComp->uMaxRs*SB_FE2000_NUM_RC_INSTANCES; i++) {
        memcpy(&pClsComp->sRsDb[i], &pRsDb[i], sizeof(soc_sbx_g2p3_cls_ruleset_t));
        if (pRsDb[i].uAlloc) {
            /* reallocate program space memory for ruleset */
            err = shr_mem_avl_realloc(pClsComp->mem_avl_ptr[pRsDb[i].uRcInstance][pRsDb[i].uBank],
                                      pRsDb[i].uSize,
                                      pRsDb[i].uBaseAddress);
            if (err != 0) {
             SB_ASSERT(0);
             return err;
            }
            /* handle SWAP rulesets */
            if ( (pRsDb[i].uProgNumber == -1) && (pRsDb[i].sType == -1) ) {
                uRc = pRsDb[i].uRcInstance;
                pClsComp->usagePerRc[uRc].uMemTotal -= pRsDb[i].uSize;
                SB_ASSERT(pRsDb[i].uSize == pClsComp->uMaxInstCount);
                pClsComp->uSwapRs[uRc] = i;
                SB_ASSERT(pClsComp->isSwapRsUsed[uRc] == 0);
            }

        }
    }

    soc_scache_handle_unlock(pFe->unit, _cls_cache[pFe->unit].wb_hdl);
#endif
    return SB_OK;
}

sbStatus_t
soc_sbx_g2p3_cls_wb_group_recover(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_cls_schema_t *pSchema, soc_sbx_g2p3_cls_rule_db_e_t eDb, uint32_t uGroupId, uint32_t uRuleCount){

#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_wb_layout_t *layout;
    sbStatus_t err;
    soc_sbx_g2p3_cls_wb_ruleset_t *pRsDb;
    uint32_t rc, uLogProg=0;
    int32_t rs, firstRs;
    soc_sbx_g2p3_cls_wb_rc_t *wbData;
    uint32_t uNumDmaBlocks;
    uint32_t uRules;
    uint32_t uCount=0;
    sbZfFe2000RcSbPattern_t *pPatternMemoryDecode[SB_FE2000XT_RC_NUM_SUPERBLOCKS];
    uint32_t uBufSize=0, uSb;
    uint32_t uDmaOffset, *pDmaBuf;
    uint32_t bDealloc = 0;
    sbDmaMemoryHandle_t junk;

    sal_memset(&junk, 0, sizeof(junk));

    pClsComp = pFe->pClsComp;
    err = soc_sbx_g2p3_cls_wb_layout_get(pFe->unit, &layout);
    if (err != SB_OK) {
        return err;
    }

    soc_scache_handle_lock(pFe->unit, _cls_cache[pFe->unit].wb_hdl);

    pRsDb = &(layout->ruleset[0]);

    uNumDmaBlocks = pSchema->uInstCount / SB_FE2000XT_RC_INST_PER_DMA;

    wbData = (soc_sbx_g2p3_cls_wb_rc_t*)(pClsComp->wbData);

    /* determine program */
    switch (pSchema->sType) {
    case SB_G2P3_FE_CLS_G2P3_SCHEMA:
    case SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA:
        uLogProg = G2P3_INGRESS_LOGICAL_PROGRAM_NUMBER;
        break;
    case SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA:
    case SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA:
        uLogProg = G2P3_EGRESS_LOGICAL_PROGRAM_NUMBER;
        break;
    default:
        SB_ASSERT(0);
        return SOC_E_INTERNAL;
    }
    
    /* for each RC...
         get base address of program
         search rsDb to find starting ruleset
         walk ruleset to find first ruleset in this group
         for each ruleset in this group
            decode rules and re-add at ACE
         update memory manager
     */
    firstRs = -1;
    for (rc=0; rc < G2P3_MAX_RC_INSTANCE; rc++) {
        for (rs=0; rs < pClsComp->uMaxRs * SB_FE2000_NUM_RC_INSTANCES; rs++) {
            if ( (pRsDb[rs].uBaseAddress == wbData[rc].prog[uLogProg].uBaseAddress) &&
                 (pRsDb[rs].uProgNumber == uLogProg) &&
                 (pRsDb[rs].uRcInstance == rc)){
                firstRs = rs;
                break;
            }
        }
        if (firstRs != -1)
            break;
    }

    rs = firstRs;
    if (rs > 0) {
        /* allocate memory for a ruleset */
        for (uSb=0; uSb < SB_FE2000XT_RC_NUM_SUPERBLOCKS; uSb++){
            uBufSize = (pSchema->uInstCount)*sizeof(sbZfFe2000RcSbPattern_t);
            for (uSb=0; uSb < SB_FE2000XT_RC_NUM_SUPERBLOCKS; uSb++){
                err = pClsComp->sbmalloc(pClsComp->clientData,
                                         SB_ALLOC_INTERNAL,
                                         uBufSize,
                                         (void*)&pPatternMemoryDecode[uSb], NULL);
                if (err != SB_OK) {
                    SB_ASSERT(0);
                    return err;
                }
                memset(pPatternMemoryDecode[uSb], 0, uBufSize);
            }
        }
        bDealloc = 1;
    }
    while (rs > 0) {
        if (pRsDb[rs].uGroupId == uGroupId) {
            uCount++;
            /* recover ruleset */

            /* set pointer to start of ruleset */
            pDmaBuf = wbData[pRsDb[rs].uRcInstance].dmaBuffer;
            uDmaOffset = pRsDb[rs].uBaseAddress * SB_FE2000XT_RC_INSTRUCTION_WORDS_IN_DMA_BUFFER;

            /* recover rules */
            uRules = 0;
            soc_sbx_g2p3_cls_wb_ruleset_recover(pFe, pSchema, uGroupId,
                                                pPatternMemoryDecode, &pDmaBuf[uDmaOffset],
                                                pClsComp->sRsDb[rs].uRuleOffset,
                                                &uRules);
            uRuleCount -= uRules;
        }
        rs = pRsDb[rs].uNextRs;
    }
    SB_ASSERT(uRuleCount == 0);
    if (bDealloc) {
        for (uSb=0; uSb < SB_FE2000XT_RC_NUM_SUPERBLOCKS; uSb++){
            err = pClsComp->sbfree(pClsComp->clientData,
                                     SB_ALLOC_INTERNAL,
                                     uBufSize,
                                     pPatternMemoryDecode[uSb], junk);
            if (err != SB_OK) {
                SB_ASSERT(0);
                return err;
            }
        }

    }
    soc_scache_handle_unlock(pFe->unit, _cls_cache[pFe->unit].wb_hdl);

    soc_sbx_g2p3_cls_update_mem_usage(pFe, pSchema->sType, eDb);

    return SB_OK;
#else
    return SB_OK;
#endif
}

sbStatus_t
soc_sbx_g2p3_cls_schema_init_c2(soc_sbx_g2p3_state_t *pFe)
{
  int8_t index;
  soc_sbx_g2p3_cls_schema_t *pSchema       = NULL;
  soc_sbx_g2p3_cls_t *pClsCompiler    = pFe->pClsComp;
  sbFe2000ClsTemplate_t *pTemplate;
  uint32_t uBankedMode;
#ifdef BCM_WARM_BOOT_SUPPORT
  int err;
  uint32 size, scache_size;
  soc_wb_cache_t *wbc;
#endif
  pClsCompiler->uMaxInstCount = 0;

  uBankedMode = soc_property_get(pFe->unit, spn_FIELD_ATOMIC_UPDATE, 0);
  /* G2P3 Ingress Schema Initialization  */
  pSchema                       = &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA];
  pSchema->pInstructions        = NULL;
  pSchema->pEncoder             = soc_sbx_g2p3_cls_g2p3_schema_encoder_c2;
  pSchema->pDecoder             = soc_sbx_g2p3_cls_g2p3_schema_decoder_c2;
  pSchema->uInstCount           = 0;
  pSchema->sType                = SB_G2P3_FE_CLS_G2P3_SCHEMA;
  pTemplate = &pSchema->schema;
  pTemplate->bDebug = TRUE;
  pTemplate->ops         = &soc_sbx_g2p3_cls_c2_g2p3_template[0];
  pTemplate->uCount      = sizeof(soc_sbx_g2p3_cls_c2_g2p3_template)/sizeof(sbFe2000ClsTemplateField_t);


  /* set banked mode based on a SOC property */
  pSchema->layout.uBankedMode = uBankedMode;

  sbFe2000ClsGenerateInstructions_c2(pTemplate,
                                  pSchema->sType,
                                  &pSchema->pInstructions,&pSchema->uInstCount,
                                  &pSchema->uInstNoPadCount,TRUE);

  if(pClsCompiler->bDebug) {
    sbFe2000ClsPrintInstructions(pSchema->pInstructions,pSchema->uInstCount, 1);
  }

  /* G2P3 Egress Schema Initialization  */
  pSchema                       = &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA];
  pSchema->pInstructions        = NULL;
  pSchema->pEncoder             = soc_sbx_g2p3_cls_g2p3_schema_encoder_c2;
  pSchema->pDecoder             = soc_sbx_g2p3_cls_g2p3_schema_decoder_c2;
  pSchema->uInstCount           = 0;
  pSchema->sType                = SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA;
  pTemplate = &pSchema->schema;
  pTemplate->bDebug = TRUE;
  pTemplate->ops         = &soc_sbx_g2p3_cls_c2_g2p3_template[0];
  pTemplate->uCount      = sizeof(soc_sbx_g2p3_cls_c2_g2p3_template)/sizeof(sbFe2000ClsTemplateField_t);

  pSchema->layout.uBankedMode = uBankedMode;

  sbFe2000ClsGenerateInstructions_c2(pTemplate,
                                  pSchema->sType,
                                  &pSchema->pInstructions,&pSchema->uInstCount,
                                  &pSchema->uInstNoPadCount,TRUE);

  if(pClsCompiler->bDebug) {
    sbFe2000ClsPrintInstructions(pSchema->pInstructions,pSchema->uInstCount, 1);
  }

  if (pFe->pClsComp->uMaxSchema > SB_G2P3_FE_CLS_MAX_NUM_SCHEMA_C1) {
      /* G2P3 Ingress IPv6 Schema Initialization  */
      pSchema                       = &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA];
      pSchema->pInstructions        = NULL;
      pSchema->pEncoder             = soc_sbx_g2p3_cls_g2p3_v6_schema_encoder_c2;
      pSchema->pDecoder             = soc_sbx_g2p3_cls_g2p3_v6_schema_decoder_c2;
      pSchema->uInstCount           = 0;
      pSchema->sType                = SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA;
      pTemplate = &pSchema->schema;
      pTemplate->bDebug = TRUE;
      pTemplate->ops         = &soc_sbx_g2p3_cls_c2_g2p3_v6_template[0];
      pTemplate->uCount      = sizeof(soc_sbx_g2p3_cls_c2_g2p3_v6_template)/sizeof(sbFe2000ClsTemplateField_t);

      /* set banked mode based on a SOC property */
      pSchema->layout.uBankedMode = uBankedMode;

      sbFe2000ClsGenerateInstructions_c2(pTemplate,
                                  pSchema->sType,
                                  &pSchema->pInstructions,&pSchema->uInstCount,
                                  &pSchema->uInstNoPadCount,TRUE);

      if(pClsCompiler->bDebug) {
          sbFe2000ClsPrintInstructions(pSchema->pInstructions,pSchema->uInstCount, 1);
      }

      /* G2P3 Egress IPv6 Schema Initialization  */
      pSchema                       = &pFe->pClsComp->schemadb[SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA];
      pSchema->pInstructions        = NULL;
      pSchema->pEncoder             = soc_sbx_g2p3_cls_g2p3_v6_schema_encoder_c2;
      pSchema->pDecoder             = soc_sbx_g2p3_cls_g2p3_v6_schema_decoder_c2;
      pSchema->uInstCount           = 0;
      pSchema->sType                = SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA;
      pTemplate = &pSchema->schema;
      pTemplate->bDebug = TRUE;
      pTemplate->ops         = &soc_sbx_g2p3_cls_c2_g2p3_v6_template[0];
      pTemplate->uCount      = sizeof(soc_sbx_g2p3_cls_c2_g2p3_v6_template)/sizeof(sbFe2000ClsTemplateField_t);
  
      sbFe2000ClsGenerateInstructions_c2(pTemplate,
                                      pSchema->sType,
                                      &pSchema->pInstructions,&pSchema->uInstCount,
                                      &pSchema->uInstNoPadCount,TRUE);

      if(pClsCompiler->bDebug) {
          sbFe2000ClsPrintInstructions(pSchema->pInstructions,pSchema->uInstCount, 1);
      }

      /* Generate layout information based on microcode */
      /* set banked mode based on a SOC property */
      pSchema->layout.uBankedMode = uBankedMode;
  }

  for (index =0 ; index < pClsCompiler->uMaxSchema; index++) {
      if(pClsCompiler->uMaxInstCount < pClsCompiler->schemadb[index].uInstCount) {
          pClsCompiler->uMaxInstCount = pClsCompiler->schemadb[index].uInstCount;
      }
  }

  soc_sbx_g2p3_cls_schema_layout_init(pFe, 1);

  soc_sbx_g2p3_rc1_rule_startidx_set(pFe->unit, pClsCompiler->uMaxRs * 1024);

#ifdef BCM_WARM_BOOT_SUPPORT
  /* on cold boot, setup scache */
  SOC_SCACHE_HANDLE_SET(_cls_cache[pFe->unit].wb_hdl, pFe->unit, SOC_SBX_WB_MODULE_COMMON, SOC_SBX_G2P3_WB_CLS);
  /* Is Level 2 recovery even available? */
  err = soc_stable_size_get(pFe->unit, (int*)&size);
  if (err != SOC_E_NONE) {
      return err; 
  }
  
  /* Allocate a new chunk of the scache during a cold boot */
  if (!SOC_WARM_BOOT(pFe->unit) && (size > 0)) {
      scache_size = sizeof(soc_sbx_g2p3_cls_wb_ruleset_t) * pClsCompiler->uMaxRs * SB_FE2000_NUM_RC_INSTANCES;
      scache_size += SOC_WB_SCACHE_CONTROL_SIZE;
      err = soc_scache_alloc(pFe->unit, _cls_cache[pFe->unit].wb_hdl, scache_size);
      if ( (err != SOC_E_NONE) && (err != SOC_E_EXISTS) ){
          return err;
      }
  }

  /* Get the pointer from scache */
  wbc = NULL;
  _cls_cache[pFe->unit].scache_size = 0;
  if (size > 0) {
      err = soc_scache_ptr_get(pFe->unit, _cls_cache[pFe->unit].wb_hdl,
                                  (uint8**)&wbc,
                                  &_cls_cache[pFe->unit].scache_size);
      if (err != SOC_E_NONE) {
          return err; 
      }

  }

  if (wbc){
      if (!SOC_WARM_BOOT(pFe->unit)) {
          wbc->version = G2P3_CLS_WB_CURRENT_VERSION;
      }

      SB_LOG(_SBX_D(pFe->unit, "Obtained scache pointer=0x%08x, %d bytes, "
                      "version=%d.%d\n"),
               (int)wbc->cache, _cls_cache[pFe->unit].scache_size,
               SOC_SCACHE_VERSION_MAJOR(wbc->version),
               SOC_SCACHE_VERSION_MINOR(wbc->version));

      if (wbc->version > G2P3_CLS_WB_CURRENT_VERSION) {
          SB_ERROR(_SBX_D(pFe->unit, "Upgrade scenario not supported.  "
                         "Current version=%d.%d  found %d.%d\n"),
                  SOC_SCACHE_VERSION_MAJOR(G2P3_CLS_WB_CURRENT_VERSION),
                  SOC_SCACHE_VERSION_MINOR(G2P3_CLS_WB_CURRENT_VERSION),
                  SOC_SCACHE_VERSION_MAJOR(wbc->version),
                  SOC_SCACHE_VERSION_MINOR(wbc->version));
          err = SOC_E_CONFIG;

      } else if (wbc->version < G2P3_CLS_WB_CURRENT_VERSION) {
          SB_ERROR(_SBX_D(pFe->unit, "Downgrade scenario not supported.  "
                         "Current version=%d.%d  found %d.%d\n"),
                  SOC_SCACHE_VERSION_MAJOR(G2P3_CLS_WB_CURRENT_VERSION),
                  SOC_SCACHE_VERSION_MINOR(G2P3_CLS_WB_CURRENT_VERSION),
                  SOC_SCACHE_VERSION_MAJOR(wbc->version),
                  SOC_SCACHE_VERSION_MINOR(wbc->version));
          err = SOC_E_CONFIG;
      }

      if (err != SOC_E_NONE) {
          return err;
      }

  }

  /* if warmboot, perform recovery */
  if (SOC_WARM_BOOT(pFe->unit)) {
      /* recover groups */
      err = soc_sbx_g2p3_cls_scache_recover(pFe->unit, pClsCompiler);
      if (err != SB_OK) {
          return err;
      }
  }else{
      err = soc_sbx_g2p3_cls_scache_store(pFe);
      if (err != SB_OK){
          return err;
      }
  }

#endif

  return SB_OK;
}

sbStatus_t
soc_sbx_g2p3_cls_init_c2(soc_sbx_g2p3_state_t *pFe,
                  sbFe2000DmaMgr_t  *pDmaCtxt,
                  sbCommonConfigParams_p_t    cParams,
                  sbIngressConfigParams_t*    iParams,
                  void* pWbData)
{
  void *v;
  int  index=0, ingrSchemaLen=0, egrSchemaLen=0, progNumber=0;
  uint32_t uIngressProgEndAddress = 0;
  sbStatus_t err;
  soc_sbx_g2p3_cls_t *pClsCompiler = NULL;
  soc_sbx_g2p3_cls_prog_attr_t *pProgAttr = NULL;
  uint32_t uRules, uBankedMode, uSize;
  uint8_t v6Enable, uLogProgNumber=0;
  int16_t iRc, iBank;
  soc_sbx_g2p3_cls_wb_rc_t *wbData = pWbData;
#ifdef DYNAMIC_MEMMGR
  int result;
#endif

  /* reset all the data */
  err = cParams->sbmalloc(cParams->clientData,SB_ALLOC_INTERNAL,
                          sizeof(soc_sbx_g2p3_cls_t), &v, NULL);

  if(SB_OK != err){
      return SB_MALLOC_FAILED;
  }

  SB_MEMSET(v, 0, sizeof(soc_sbx_g2p3_cls_t));

  pFe->pClsComp = (soc_sbx_g2p3_cls_t *) v;
  pClsCompiler  = pFe->pClsComp;
  pClsCompiler->bDebug = FALSE;
  pClsCompiler->isC2 = TRUE;
  pClsCompiler->wbData =  wbData;

  /* save the handlers for later use */
  pClsCompiler->sbmalloc   = cParams->sbmalloc;
  pClsCompiler->sbfree     = cParams->sbfree;
  pClsCompiler->clientData = cParams->clientData;

  pClsCompiler->pTmpCtrMem  = thin_host_malloc(
                                sizeof(soc_sbx_g2p3_counter_value_t) *
                                G2P3_MAX_CLS_RULES_PER_RULESET_C2);

  if(!pClsCompiler->pTmpCtrMem){
      return SB_MALLOC_FAILED;
  }

 /* set banked mode based on a SOC property */
  uBankedMode = soc_property_get(pFe->unit, spn_FIELD_ATOMIC_UPDATE, 0);

#ifdef DYNAMIC_MEMMGR
  if (uBankedMode == 1) {
      uSize = RCE_INST_MEM_SIZE / 2;
  } else {
      uSize = RCE_INST_MEM_SIZE;
  }

  iBank = 0;
  for (iRc = 0; iRc < G2P3_MAX_RC_INSTANCE; iRc++) {
      /* Create avl tree for managing RC0 and RC1 instruction memory */
      result = shr_mem_avl_create(&pClsCompiler->mem_avl_ptr[iRc][iBank],
                                   uSize,
                                   0,
                                   64);
      if (result !=0) {
          soc_cm_print("CLS: shr_mem_avl_create failed for RC:%d Bank:%d\n",
                                                              iRc, iBank);
      }
  }

  if (uBankedMode == 1) {
      iBank = 1;
      for (iRc = 0; iRc < G2P3_MAX_RC_INSTANCE; iRc++) {
          /* Create avl tree for managing RC0 and RC1 instruction memory */
          result = shr_mem_avl_create(&pClsCompiler->mem_avl_ptr[iRc][iBank],
                                       uSize,
                                       uSize,
                                       64);
          if (result !=0) {
              soc_cm_print("CLS: shr_mem_avl_create failed for RC:%d Bank:%d\n",
                                                                  iRc, iBank);
          }
      }
   }
#endif


  v6Enable = soc_property_get(pFe->unit, spn_IPV6_ENABLE, 0);
  if(!v6Enable) {
      pClsCompiler->uMaxSchema = SB_G2P3_FE_CLS_MAX_NUM_SCHEMA_C1;
  } else {
      pClsCompiler->uMaxSchema = SB_G2P3_FE_CLS_MAX_NUM_SCHEMA;
  }

  soc_sbx_g2p3_cls_schema_init_c2(pFe);

  /* initialize the dma context cookie */
  pClsCompiler->pDmaCtxt  = pDmaCtxt;

  ingrSchemaLen = \
         pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].uInstCount;
  egrSchemaLen = \
         pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA].uInstCount;

  /* Caladan2 performs updates a single ruleset at a time. */
  /* assumes ingress/egress use the same schema */
  pClsCompiler->uMaxSlabSize = pClsCompiler->uMaxInstCount * (SB_ZF_C2RCDMAFORMAT_SIZE_IN_BYTES / SB_FE2000XT_RC_INST_PER_DMA);

  /* create the dma slabs */
  soc_sbx_g2p3_cls_allocate_dma_slabs(pFe,cParams, pClsCompiler->uMaxSlabSize, 1);

#ifdef RULESET_DEBUG /* CLS_COMP_DEBUG */
  soc_cm_print("RCE ingrSchemaLen: 0x%x\n", ingrSchemaLen);
  soc_cm_print("RCE egrSchemaLen: 0x%x\n", egrSchemaLen);
#endif

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
  uRules = 0;
  for(index=0, progNumber=G2P3_INGRESS_PROGRAM_BASE;\
      index <= 1; index++) {
      pProgAttr               = pClsCompiler->pProgAtrib[progNumber];
      pProgAttr->uNumber      = progNumber++;
      pProgAttr->uActive      = 1;
      pProgAttr->uValid       = 1;

      /* even batch number correspond to ingress schema */
      pProgAttr->sType = SB_G2P3_FE_CLS_G2P3_SCHEMA;
      pProgAttr->uBaseAddress = index * ingrSchemaLen * \
                                pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRs;
      pProgAttr->uLength      = pClsCompiler->uMaxInstCount;

      if (index == 1) {
        pProgAttr->uActive      = 0;
        soc_sbx_g2p3_cls_schema_to_logprogram(pFe, SB_G2P3_FE_CLS_G2P3_SCHEMA, &uLogProgNumber);
        pClsCompiler->sbyProgNumber[uLogProgNumber] = pProgAttr->uNumber;
      }
      uIngressProgEndAddress = pProgAttr->uBaseAddress +\
                                             pProgAttr->uLength * \
                                             pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRs;
  }

  uRules = pClsCompiler->uMaxRs * \
     pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRulesPerRs \
     * SB_FE2000_NUM_RC_INSTANCES;

  if (uRules > 32256) {
    uRules = 32256;
  }


#ifdef RULESET_DEBUG
  soc_cm_print("IRT table size: 0x%x\n", uRules);
#endif
/*  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_irt_table_size_set(pFe->unit, uRules));
 *  */
/* FIX_ME: Is the size uRules good enough? There is no demarcation of rule
 * position between ingress / egress. Don't it needs to be size of ingress
 * rules + egress rules?
 */
  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_rt_mt_table_size_set(pFe->unit, uRules));

  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ertctr_counter_size_set(pFe->unit, 0));
  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_irtctr_counter_size_set(pFe->unit, 0));
  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_rtctr_counter_size_set(pFe->unit, uRules));

  /* Allocate memory for Egress Program and bookmark them */
  uRules = 0;

  for(index=0, progNumber=G2P3_EGRESS_PROGRAM_BASE;\
      index <= 1; index++) {
      pProgAttr               = pClsCompiler->pProgAtrib[progNumber];
      pProgAttr->uNumber      = progNumber++;
      pProgAttr->uActive      = 1;
      pProgAttr->uValid       = 1;

      /* odd batch number correspond to egress schema */
      pProgAttr->sType = SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA;
      pProgAttr->uBaseAddress = uIngressProgEndAddress + \
                                egrSchemaLen * index * pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_SCHEMA].layout.uNumRs;

      pProgAttr->uLength      = pClsCompiler->uMaxInstCount;

      if (index == 1) {
        pProgAttr->uActive      = 0;
        soc_sbx_g2p3_cls_schema_to_logprogram(pFe, SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA, &uLogProgNumber);
        pClsCompiler->sbyProgNumber[uLogProgNumber] = pProgAttr->uNumber;
      }

  }

  uRules = pClsCompiler->uMaxRs * \
            pClsCompiler->schemadb[SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA].layout.uNumRulesPerRs \
            * SB_FE2000_NUM_RC_INSTANCES;

  if (uRules > 32256) {
    uRules = 32256;
  }

#ifdef RULESET_DEBUG
  soc_cm_print("ERT table size: 0x%x\n", uRules);
#endif
/*  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ert_table_size_set(pFe->unit, uRules));
 *  */
/*  SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ertctr_counter_size_set(pFe->unit, uRules));
*/
  soc_sbx_g2p3_cls_rr_init(pFe);

  (void)clsdmatest;
  /* clsdmatest(pFe,cParams);  */
  return SB_OK;
}

PRIVATE
void soc_sbx_g2p3_cls_g2p3_v6_schema_encoder_c2(
    soc_sbx_g2p3_cls_schema_p_t            pSchemaHandler, 
    uint32_t                               uRIndex,
    void                                   *vRule, 
    soc_sbx_g2p3_cls_pattern_memory_p_t    pPatternMemory)
{
    uint32_t uLocation = 0;
    uint32_t uRs    = 0;
    uint32_t uRuleId     = 0;
    uint32_t uRuleSb     = 0;
    uint32_t uRcInstance = 0; 
    sbZfFe2000RcSbPattern_t *pPattern = NULL;
    soc_sbx_g2p3_cls_ace_pattern_ipv6_p_t  pRule = vRule;

    /* get the pattern block that this rule is going to go */
    /* we commit one reulset at a time in C2, so leave uRs and uRc as 0 */
    uRuleId     = uRIndex % pSchemaHandler->layout.uNumRulesPerRs;
    uRuleSb     = uRuleId / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;
  
    pPattern = pPatternMemory->pSbPattern[uRs][uRcInstance][uRuleSb];

    /* Within a super block it's mod 128 */
    uRuleId  = uRuleId % G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;  

#define ENCODE_PATTERN(uOffsetValue, uRuleIdValue, uBitValue) \
do { \
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffsetValue], \
                             uRuleIdValue, \
                             uBitValue);  \
    uOffsetValue++; \
} while (0)

    /* Encode Restart */
    ENCODE_PATTERN(uLocation, uRuleId, 0);

    if (pSchemaHandler->schema.ops[0].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uDbType, 
                               pRule->bDbTypeAny,
                               &pSchemaHandler->schema.ops[0],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }
    if (pSchemaHandler->schema.ops[1].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTC, 
                               pRule->bTCAny,
                               &pSchemaHandler->schema.ops[1],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[2].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagURG, 
                               pRule->bTcpFlagURGAny,
                               &pSchemaHandler->schema.ops[2],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[3].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagACK, 
                               pRule->bTcpFlagACKAny,
                               &pSchemaHandler->schema.ops[3],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[4].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagPSH, 
                               pRule->bTcpFlagPSHAny,
                               &pSchemaHandler->schema.ops[4],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);

    }

    if (pSchemaHandler->schema.ops[5].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagRST, 
                               pRule->bTcpFlagRSTAny,
                               &pSchemaHandler->schema.ops[5],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[6].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagSYN, 
                               pRule->bTcpFlagSYNAny,
                               &pSchemaHandler->schema.ops[6],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[7].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagFIN, 
                               pRule->bTcpFlagFINAny,
                               &pSchemaHandler->schema.ops[7],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);

    }

    if (pSchemaHandler->schema.ops[8].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uVlan, 
                               pRule->bVlanAny,
                               &pSchemaHandler->schema.ops[8],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);

    }

    if (pSchemaHandler->schema.ops[9].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uL4SrcPortLow, 
                               pRule->uL4SrcPortHigh,
                               &pSchemaHandler->schema.ops[9],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if ((pSchemaHandler->schema.ops[10].bEnabled) ||
        (pSchemaHandler->schema.ops[11].bEnabled)) {
        if (!((pRule->uL4SrcPortLow == 0) && (pRule->uL4SrcPortHigh == 0))) {
            sbFe2000XtClsEncodePattern(uRuleId,
                                   pRule->uL4DstPortLow,
                                   pRule->uL4DstPortHigh,
                                   &pSchemaHandler->schema.ops[10],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern, 0);
        }
    }

    if ((pSchemaHandler->schema.ops[10].bEnabled) ||
        (pSchemaHandler->schema.ops[11].bEnabled)) {
        if ((pRule->uL4SrcPortLow == 0) && (pRule->uL4SrcPortHigh == 0)) {
             sbFe2000XtClsEncodePattern(uRuleId,
                                   pRule->uNextHeaderLow, 
                                   pRule->uNextHeaderHigh,
                                   &pSchemaHandler->schema.ops[11],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern, 0);
        }
    }

    if (pSchemaHandler->schema.ops[12].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uuIpv6DaLS, 
                               pRule->uuIpv6DaLen,
                               &pSchemaHandler->schema.ops[12],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               pRule->uuIpv6DaMS); 
    }

    if (pSchemaHandler->schema.ops[13].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uuIpv6SaLS, 
                               pRule->uuIpv6SaLen,
                               &pSchemaHandler->schema.ops[13],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               pRule->uuIpv6SaMS);
    }

    if (pSchemaHandler->schema.ops[14].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uuPbmp, 
                               0,
                               &pSchemaHandler->schema.ops[14],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    while ((uLocation % 8) < 7) {
        ENCODE_PATTERN(uLocation, uRuleId, 0);
    }
    ENCODE_PATTERN(uLocation, uRuleId, 0);
    while ((uLocation % 8) < 7) {
        ENCODE_PATTERN(uLocation, uRuleId, 0);
    }
    ENCODE_PATTERN(uLocation, uRuleId, 0);

    return;
}

PRIVATE
void soc_sbx_g2p3_cls_g2p3_v6_schema_decoder_c2(
    soc_sbx_g2p3_cls_schema_p_t            pSchemaHandler, 
    uint32_t                               uRIndex,
    void                                   *vRule, 
    sbZfFe2000RcSbPattern_t                *pPatternMemory[],
    uint32_t                               *isNull)
{
    uint32_t uLocation = 0;
    uint32_t uRuleId     = 0;
    uint32_t uRuleSb     = 0;
    sbZfFe2000RcSbPattern_t *pPattern = NULL;
    soc_sbx_g2p3_cls_ace_pattern_ipv6_p_t  pRule = vRule;
    uint8_t dummy;
    uint64_t uuField1=0, uuField2=0, uuField3=0;

    /* get the pattern block that this rule is going to go */
    /* we commit one reulset at a time in C2, so leave uRs and uRc as 0 */
    uRuleId     = uRIndex % pSchemaHandler->layout.uNumRulesPerRs;
    uRuleSb     = uRuleId / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;
  
    pPattern = pPatternMemory[uRuleSb];

    /* Within a super block it's mod 128 */
    uRuleId  = uRuleId % G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;  

    *isNull = sbZfFe2000RcSbPatternIsNull(pPattern,
                                          uRuleId,
                                          pSchemaHandler->uInstCount);
    if (*isNull){
        return;
    }

#define DECODE_PATTERN(uOffsetValue, uRuleIdValue, uBitValue) \
do { \
    sbZfFe2000RcSbGetRuleBit(&pPattern[uOffsetValue], \
                             uRuleIdValue, \
                             &uBitValue);  \
    uOffsetValue++; \
} while (0)

    /* Decode Restart */
    DECODE_PATTERN(uLocation, uRuleId, dummy);

    if (pSchemaHandler->schema.ops[0].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[0],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uDbType = uuField1;
        pRule->bDbTypeAny = uuField2;
    }else{
        pRule->bDbTypeAny = 1;
    }
    if (pSchemaHandler->schema.ops[1].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[1],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTC = uuField1;
        pRule->bTCAny = uuField2;
    }else{
        pRule->bTCAny = 1;
    }

    if (pSchemaHandler->schema.ops[2].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[2],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagURG = uuField1;
        pRule->bTcpFlagURGAny = uuField2;
    }else{
        pRule->bTcpFlagURGAny = 1;
    }

    if (pSchemaHandler->schema.ops[3].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[3],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagACK = uuField1;
        pRule->bTcpFlagACKAny = uuField2;
    }else{
        pRule->bTcpFlagACKAny = 1;
    }

    if (pSchemaHandler->schema.ops[4].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[4],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagPSH = uuField1;
        pRule->bTcpFlagPSHAny = uuField2;
    }else{
        pRule->bTcpFlagPSHAny = 1;
    }

    if (pSchemaHandler->schema.ops[5].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[5],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagRST = uuField1;
        pRule->bTcpFlagRSTAny = uuField2;
    }else{
        pRule->bTcpFlagRSTAny = 1;
    }

    if (pSchemaHandler->schema.ops[6].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[6],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagSYN = uuField1;
        pRule->bTcpFlagSYNAny = uuField2;
    }else{
        pRule->bTcpFlagSYNAny = 1;
    }

    if (pSchemaHandler->schema.ops[7].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[7],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagFIN = uuField1;
        pRule->bTcpFlagFINAny = uuField2;
    }else{
        pRule->bTcpFlagFINAny = 1;
    }

    if (pSchemaHandler->schema.ops[8].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[8],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uVlan = uuField1;
        pRule->bVlanAny = uuField2;
    }else{
        pRule->bVlanAny = 1;
    }

    if (pSchemaHandler->schema.ops[9].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[9],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uL4SrcPortLow = uuField1;
        pRule->uL4SrcPortHigh = uuField2;
    }else{
        pRule->uL4SrcPortLow = 0;
        pRule->uL4SrcPortHigh = 0xffff;
    }

   /* Decode either L4 DSTPORT or NextHeader info.  Determine which
    * based on the L2SrcPortLo/High fields.
    */
    pRule->uL4DstPortLow = 0;
    pRule->uL4DstPortHigh = 0xffff;
    if ((pSchemaHandler->schema.ops[10].bEnabled) ||
        (pSchemaHandler->schema.ops[11].bEnabled)) {
        if (!((pRule->uL4SrcPortLow == 0) && (pRule->uL4SrcPortHigh == 0))) {
            sbFe2000XtClsDecodePattern(uRuleId,
                                   &uuField1,
                                   &uuField2,
                                   &pSchemaHandler->schema.ops[10],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern,
                                   &uuField3);
            pRule->uL4DstPortLow = uuField1;
            pRule->uL4DstPortHigh = uuField2;
        }
    }

    pRule->uNextHeaderLow = 0;
    pRule->uNextHeaderHigh = 0xffff;
    if ((pSchemaHandler->schema.ops[10].bEnabled) ||
        (pSchemaHandler->schema.ops[11].bEnabled)) {
        if ((pRule->uL4SrcPortLow == 0) && (pRule->uL4SrcPortHigh == 0)) {
             sbFe2000XtClsDecodePattern(uRuleId,
                                   &uuField1,
                                   &uuField2,
                                   &pSchemaHandler->schema.ops[11],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern,
                                   &uuField3);
            pRule->uNextHeaderLow = uuField1;
            pRule->uNextHeaderHigh = uuField2;
        }
    }

    if (pSchemaHandler->schema.ops[12].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[12],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uuIpv6DaLS = uuField1;
        pRule->uuIpv6DaMS = uuField3;
        pRule->uuIpv6DaLen = uuField2;
    }else{
        pRule->uuIpv6DaLen = 0;
    }

    if (pSchemaHandler->schema.ops[13].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[13],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uuIpv6SaLS = uuField1;
        pRule->uuIpv6SaMS = uuField3;
        pRule->uuIpv6SaLen = uuField2;
    }else{
        pRule->uuIpv6SaLen = 0;
    }

    if (pSchemaHandler->schema.ops[14].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[14],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uuPbmp = uuField1;
        pRule->bPortAny = uuField2;
    }else{
        pRule->bPortAny = 1;
    }
    if (pRule->bPortAny) {
        pRule->uuPbmp = 0;
        pRule->uuPbmp = ~(pRule->uuPbmp);
    }

    return;
}
PRIVATE
sbStatus_t soc_sbx_g2p3_cls_rr_init(soc_sbx_g2p3_state_t *pFe)
{
  uint32_t uRrAddress;
  sbZfC2RcResultResolverEntry_t zfResult;
  uint32_t uOffs, uData, uStride, uRce, uCounter;
  uint32_t uSet, uSbFilter;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
  int32_t uRs, uMaxRs;

  /* Caladan2 only */
  SB_ASSERT(SAND_HAL_IS_FE2KXT(pFe->regSet));

  /* Clear RR counters */
  uData = 0;
  uStride = SAND_HAL_REG_OFFSET(C2, RC_RR1_RULE_CNT_CONFIG) - SAND_HAL_REG_OFFSET(C2, RC_RR0_RULE_CNT_CONFIG);
  for(uRce = 0; uRce < G2P3_MAX_RC_INSTANCE; uRce++) {
    uOffs = (uRce) ? SAND_HAL_REG_OFFSET(C2, RC1_RR0_RULE_CNT_CONFIG) : SAND_HAL_REG_OFFSET(C2, RC0_RR0_RULE_CNT_CONFIG);
    for(uCounter = 0; uCounter < 16; uCounter++) {
      if (uCounter == 15){
        uData = SAND_HAL_MOD_FIELD(C2, RC0_RR1_RULE_CNT_CONFIG, RULE_CNT_DATA, uData, 0);
        uData = SAND_HAL_MOD_FIELD(C2, RC0_RR1_RULE_CNT_CONFIG, RULE_CNT_MASK, uData, 0x8000);
      }else{
        uData = SAND_HAL_MOD_FIELD(C2, RC0_RR0_RULE_CNT_CONFIG, RULE_CNT_DATA, uData, uCounter);
        uData = SAND_HAL_MOD_FIELD(C2, RC0_RR0_RULE_CNT_CONFIG, RULE_CNT_MASK, uData, 0x03ff);
      }
      SAND_HAL_WRITE_OFFS(pFe->regSet, (uOffs + (uStride * uCounter)), uData);
      SAND_HAL_READ_OFFS(pFe->regSet, (uOffs + (uStride * uCounter) + 4));
    }
  }

  /* Set result mode to single-result wide */
  SAND_HAL_WRITE(pFe->regSet, C2, RC0_RR_FORMAT, 0x0);
  SAND_HAL_WRITE(pFe->regSet, C2, RC1_RR_FORMAT, 0x0);

  /* walk rulesets and configure indexes */
  pClsComp = pFe->pClsComp;
  pRsDb = pClsComp->sRsDb;
  uMaxRs = pClsComp->uMaxRs * \
           SB_FE2000_NUM_RC_INSTANCES;
  for (uRs=0; uRs<uMaxRs; uRs++) {
    uSet = uRs % pClsComp->uMaxRs;
    uRrAddress = (((uSet << 10) | 0) << 1);
    zfResult.m_uData = pRsDb[uRs].uRuleOffset;
    for (uSbFilter=0; uSbFilter<G2P3_MAX_CLS_RULES_PER_RULESET_C2; uSbFilter++){
      uRrAddress = (((uSet << 10) | uSbFilter) << 1);
      if (pRsDb[uRs].uRcInstance) {
        sbFe2000XtImfDriver_Rc1ResultResolverWrite((uint32_t)pFe->regSet, uRrAddress, zfResult.m_uData);
        sbFe2000XtImfDriver_Rc1ResultResolverWrite((uint32_t)pFe->regSet, uRrAddress+1, 0);
      } else {
        sbFe2000XtImfDriver_Rc0ResultResolverWrite((uint32_t)pFe->regSet, uRrAddress, zfResult.m_uData);
        sbFe2000XtImfDriver_Rc0ResultResolverWrite((uint32_t)pFe->regSet, uRrAddress+1, 0);
      }
      zfResult.m_uData++;
    }
  }
  return SB_OK;
}

PRIVATE
void soc_sbx_g2p3_cls_g2p3_schema_encoder_c2(
    soc_sbx_g2p3_cls_schema_p_t         pSchemaHandler, 
    uint32_t                            uRIndex,
    void                                *vRule, 
    soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory)
{
    uint32_t uLocation = 0;
    uint32_t uRs    = 0;
    uint32_t uRuleId     = 0;
    uint32_t uRuleSb     = 0;
    uint32_t uRcInstance = 0; 
    sbZfFe2000RcSbPattern_t *pPattern = NULL;
    soc_sbx_g2p3_cls_ace_pattern_p_t    pRule = vRule;

    /* get the pattern block that this rule is going to go */
    /* we commit one reulset at a time in C2, so leave uRs and uRc as 0 */
    uRuleId     = uRIndex % pSchemaHandler->layout.uNumRulesPerRs;
    uRuleSb     = uRuleId / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;
  
    pPattern = pPatternMemory->pSbPattern[uRs][uRcInstance][uRuleSb];

    /* Within a super block it's mod 128 */
    uRuleId  = uRuleId % G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;  

#define ENCODE_PATTERN(uOffsetValue, uRuleIdValue, uBitValue) \
do { \
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffsetValue], \
                             uRuleIdValue, \
                             uBitValue);  \
    uOffsetValue++; \
} while (0)

    /* Encode Restart */
    ENCODE_PATTERN(uLocation, uRuleId, 0);

    if (pSchemaHandler->schema.ops[0].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uFragment, 
                               pRule->bFragmentAny,
                               &pSchemaHandler->schema.ops[0],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[1].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagURG, 
                               pRule->bTcpFlagURGAny,
                               &pSchemaHandler->schema.ops[1],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if ((pSchemaHandler->schema.ops[2].bEnabled) ||
        (pSchemaHandler->schema.ops[3].bEnabled)) {
        if ((pRule->bEtherTypeAny) || (pRule->uEtherType)) {
            sbFe2000XtClsEncodePattern(uRuleId,
                                   pRule->uL4SrcPortLow, 
                                   pRule->uL4SrcPortHigh,
                                   &pSchemaHandler->schema.ops[2],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern, 0);
        }
    }

    if ((pSchemaHandler->schema.ops[2].bEnabled) ||
        (pSchemaHandler->schema.ops[3].bEnabled)) {
        if ((!pRule->bEtherTypeAny) && (!pRule->uEtherType)) {
             sbFe2000XtClsEncodePattern(uRuleId,
                                   pRule->uLlcDsapSsapLow, 
                                   pRule->uLlcDsapSsapHigh,
                                   &pSchemaHandler->schema.ops[3],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern, 0);
        }
    }

    if (pSchemaHandler->schema.ops[4].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagACK, 
                               pRule->bTcpFlagACKAny,
                               &pSchemaHandler->schema.ops[4],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[5].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagPSH, 
                               pRule->bTcpFlagPSHAny,
                               &pSchemaHandler->schema.ops[5],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[6].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagRST, 
                               pRule->bTcpFlagRSTAny,
                               &pSchemaHandler->schema.ops[6],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if ((pSchemaHandler->schema.ops[7].bEnabled) ||
       (pSchemaHandler->schema.ops[8].bEnabled)) {
        if ((pRule->bEtherTypeAny) || (pRule->uEtherType)) {
            sbFe2000XtClsEncodePattern(uRuleId,
                                   pRule->uL4DstPortLow,
                                   pRule->uL4DstPortHigh,
                                   &pSchemaHandler->schema.ops[7],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern, 0);
        }
    }

    if ((pSchemaHandler->schema.ops[7].bEnabled) ||
       (pSchemaHandler->schema.ops[8].bEnabled)) {
        if ((!pRule->bEtherTypeAny) && (!pRule->uEtherType)) {
            sbFe2000XtClsEncodePattern(uRuleId,
                                   pRule->uLlcCtrlDataLow,
                                   pRule->uLlcCtrlDataHigh,
                                   &pSchemaHandler->schema.ops[8],
                                   pSchemaHandler->uInstCount,
                                   &uLocation,
                                   pPattern, 0);
        }
    }

    if (pSchemaHandler->schema.ops[9].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagSYN, 
                               pRule->bTcpFlagSYNAny,
                               &pSchemaHandler->schema.ops[9],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[10].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uTcpFlagFIN, 
                               pRule->bTcpFlagFINAny,
                               &pSchemaHandler->schema.ops[10],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[11].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uPriority, 
                               pRule->bPriorityAny,
                               &pSchemaHandler->schema.ops[11],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[12].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uDscp, 
                               pRule->bDscpAny,
                               &pSchemaHandler->schema.ops[12],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[13].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uEcn, 
                               pRule->bEcnAny,
                               &pSchemaHandler->schema.ops[13],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[14].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uProtocol, 
                               pRule->bProtocolAny,
                               &pSchemaHandler->schema.ops[14],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[15].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uIpv4Sa, 
                               pRule->uIpv4SaLen,
                               &pSchemaHandler->schema.ops[15],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[16].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uIpv4Da, 
                               pRule->uIpv4DaLen,
                               &pSchemaHandler->schema.ops[16],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[17].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uEtherType, 
                               pRule->bEtherTypeAny,
                               &pSchemaHandler->schema.ops[17],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[18].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uuSmac, 
                               pRule->uuSmacLen,
                               &pSchemaHandler->schema.ops[18],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[19].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uVlan, 
                               pRule->bVlanAny,
                               &pSchemaHandler->schema.ops[19],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[20].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uuDmac, 
                               pRule->uuDmacLen,
                               &pSchemaHandler->schema.ops[20],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[21].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uDbType, 
                               pRule->bDbTypeAny,
                               &pSchemaHandler->schema.ops[21],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    if (pSchemaHandler->schema.ops[22].bEnabled) {
        sbFe2000XtClsEncodePattern(uRuleId,
                               pRule->uuPbmp, 
                               0,
                               &pSchemaHandler->schema.ops[22],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 0);
    }

    while ((uLocation % 8) < 7) {
        ENCODE_PATTERN(uLocation, uRuleId, 0);
    }
    ENCODE_PATTERN(uLocation, uRuleId, 0);
    while ((uLocation % 8) < 7) {
        ENCODE_PATTERN(uLocation, uRuleId, 0);
    }
    ENCODE_PATTERN(uLocation, uRuleId, 0);

    return;
}

PRIVATE
void soc_sbx_g2p3_cls_g2p3_schema_decoder_c2(
    soc_sbx_g2p3_cls_schema_p_t         pSchemaHandler, 
    uint32_t                            uRIndex,
    void                                *vRule, 
    sbZfFe2000RcSbPattern_t             *pPatternMemory[],
    uint32_t                            *isNull)
{
    uint32_t uLocationSave, uLocation = 0;
    uint32_t uRuleId     = 0;
    uint32_t uRuleSb     = 0;
    sbZfFe2000RcSbPattern_t *pPattern = NULL;
    soc_sbx_g2p3_cls_ace_pattern_p_t    pRule = vRule;
    uint8_t dummy;
    uint64_t uuField1=0, uuField2=0, uuField3=0;

    /* get the pattern block that this rule is going to go */
    /* we commit one reulset at a time in C2, so leave uRs and uRc as 0 */
    uRuleId     = uRIndex % pSchemaHandler->layout.uNumRulesPerRs;
    uRuleSb     = uRuleId / G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;
  
    pPattern = pPatternMemory[uRuleSb];

    /* Within a super block it's mod 128 */
    uRuleId  = uRuleId % G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET;  

    *isNull = sbZfFe2000RcSbPatternIsNull(pPattern,
                                          uRuleId,
                                          pSchemaHandler->uInstCount);
    if (*isNull){
        return;
    }

#define DECODE_PATTERN(uOffsetValue, uRuleIdValue, uBitValue) \
do { \
    sbZfFe2000RcSbGetRuleBit(&pPattern[uOffsetValue], \
                             uRuleIdValue, \
                             &uBitValue);  \
    uOffsetValue++; \
} while (0)

    /* Decode Restart */
    DECODE_PATTERN(uLocation, uRuleId, dummy);

    if (pSchemaHandler->schema.ops[0].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[0],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 
                               &uuField3);
        pRule->uFragment = uuField1;
        pRule->bFragmentAny = uuField2;
    }else{
        pRule->bFragmentAny = 1;
    }

    if (pSchemaHandler->schema.ops[1].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1, 
                               &uuField2,
                               &pSchemaHandler->schema.ops[1],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern, 
                               &uuField3);
        pRule->uTcpFlagURG = uuField1; 
        pRule->bTcpFlagURGAny = uuField2;
    }else{
        pRule->bTcpFlagURGAny = 1;
    }

   /* Decode both LLC and L4PORT info.  Determine which is valid later
    * based on the Ethertype fields.
    */
    uLocationSave = uLocation;
    if ((pSchemaHandler->schema.ops[2].bEnabled) ||
        (pSchemaHandler->schema.ops[3].bEnabled)) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[2],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uL4SrcPortLow = uuField1;
        pRule->uL4SrcPortHigh = uuField2;
    }else{
        pRule->uL4SrcPortLow = 0;
        pRule->uL4SrcPortHigh = 0xffff;
    }

    if ((pSchemaHandler->schema.ops[2].bEnabled) ||
        (pSchemaHandler->schema.ops[3].bEnabled)) {
         sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[3],
                               pSchemaHandler->uInstCount,
                               &uLocationSave,
                               pPattern,
                               &uuField3);
         pRule->uLlcDsapSsapLow = uuField1;
         pRule->uLlcDsapSsapHigh = uuField2;
    }else{
         pRule->uLlcDsapSsapLow = 0;
         pRule->uLlcDsapSsapHigh = 0xffff;
    }

    SB_ASSERT(uLocation == uLocationSave);

    if (pSchemaHandler->schema.ops[4].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[4],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagACK = uuField1;
        pRule->bTcpFlagACKAny = uuField2;
    }else{
        pRule->bTcpFlagACKAny = 1;
    }

    if (pSchemaHandler->schema.ops[5].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[5],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagPSH = uuField1;
        pRule->bTcpFlagPSHAny = uuField2;
    }else{
        pRule->bTcpFlagPSHAny = 1;
    }

    if (pSchemaHandler->schema.ops[6].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[6],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagRST = uuField1;
        pRule->bTcpFlagRSTAny = uuField2;
    }else{
        pRule->bTcpFlagRSTAny = 1;
    }

   /* Decode both LLC and L4PORT info.  Determine which is valid later
    * based on the Ethertype fields.
    */
    uLocationSave = uLocation;
    if ((pSchemaHandler->schema.ops[7].bEnabled) ||
       (pSchemaHandler->schema.ops[8].bEnabled)) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[7],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uL4DstPortLow = uuField1;
        pRule->uL4DstPortHigh = uuField2;
    }else{
        pRule->uL4DstPortLow = 0;
        pRule->uL4DstPortHigh = 0xffff;
    }

    if ((pSchemaHandler->schema.ops[7].bEnabled) ||
       (pSchemaHandler->schema.ops[8].bEnabled)) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[8],
                               pSchemaHandler->uInstCount,
                               &uLocationSave,
                               pPattern,
                               &uuField3);
        pRule->uLlcCtrlDataLow = uuField1;
        pRule->uLlcCtrlDataHigh = uuField2;
    }else{
        pRule->uLlcCtrlDataLow = 0;
        pRule->uLlcCtrlDataHigh = 0xffff;
    }

    SB_ASSERT(uLocationSave == uLocation);
    if (pSchemaHandler->schema.ops[9].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[9],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagSYN = uuField1;
        pRule->bTcpFlagSYNAny = uuField2;
    }else{
        pRule->bTcpFlagSYNAny = 1;
    }

    if (pSchemaHandler->schema.ops[10].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[10],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uTcpFlagFIN = uuField1;
        pRule->bTcpFlagFINAny = uuField2;
    }else{
        pRule->bTcpFlagFINAny = 1;
    }

    if (pSchemaHandler->schema.ops[11].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[11],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uPriority = uuField1;
        pRule->bPriorityAny = uuField2;
    }else{
        pRule->bPriorityAny = 1;
    }

    if (pSchemaHandler->schema.ops[12].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[12],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uDscp = uuField1;
        pRule->bDscpAny = uuField2;
    }else{
        pRule->bDscpAny = 1;
    }

    if (pSchemaHandler->schema.ops[13].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[13],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uEcn = uuField1;
        pRule->bEcnAny = uuField2;
    }else{
        pRule->bEcnAny = 1;
    }

    if (pSchemaHandler->schema.ops[14].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[14],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uProtocol = uuField1;
        pRule->bProtocolAny = uuField2;
    }else{
        pRule->bProtocolAny = 1;
    }

    if (pSchemaHandler->schema.ops[15].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[15],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uIpv4Sa = uuField1;
        pRule->uIpv4SaLen = uuField2;
    }else{
        pRule->uIpv4SaLen = 0;
    }

    if (pSchemaHandler->schema.ops[16].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[16],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uIpv4Da = uuField1;
        pRule->uIpv4DaLen = uuField2;
    }else{
        pRule->uIpv4DaLen = 0;
    }

    if (pSchemaHandler->schema.ops[17].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[17],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uEtherType = uuField1;
        pRule->bEtherTypeAny = uuField2;
    }else{
        pRule->bEtherTypeAny = 1;
    }

    if (pSchemaHandler->schema.ops[18].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[18],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uuSmac = uuField1;
        pRule->uuSmacLen = uuField2;
    }else{
        pRule->uuSmacLen = 0;
    }

    if (pSchemaHandler->schema.ops[19].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[19],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uVlan = uuField1;
        pRule->bVlanAny = uuField2;
    }else{
        pRule->bVlanAny = 1;
    }

    if (pSchemaHandler->schema.ops[20].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[20],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uuDmac = uuField1;
        pRule->uuDmacLen = uuField2;
    }else{
        pRule->uuDmacLen = 0;
    }

    if (pSchemaHandler->schema.ops[21].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[21],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uDbType = uuField1;
        pRule->bDbTypeAny = uuField2;
    }else{
        pRule->bDbTypeAny = 1;
    }

    if (pSchemaHandler->schema.ops[22].bEnabled) {
        sbFe2000XtClsDecodePattern(uRuleId,
                               &uuField1,
                               &uuField2,
                               &pSchemaHandler->schema.ops[22],
                               pSchemaHandler->uInstCount,
                               &uLocation,
                               pPattern,
                               &uuField3);
        pRule->uuPbmp = uuField1;
        pRule->bPortAny = uuField2;
    }else{
        pRule->bPortAny = 1;
    }
    if (pRule->bPortAny) {
        pRule->uuPbmp = 0;
        pRule->uuPbmp = ~(pRule->uuPbmp);
    }

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

    return;
}
PRIVATE
sbStatus_t
soc_sbx_g2p3_cls_program_adjuster(soc_sbx_g2p3_state_t *pFe,
                soc_sbx_g2p3_cls_schema_t *pSchema,
                uint32_t uBaseRs,
                uint32_t uRc)
{

  uint32_t uPhysicalProgramNumber =0;
  uint8_t  uProgramNumber=0, uLogProgNumber=0;
  soc_sbx_g2p3_cls_prog_attr_t *pNewActProgAttr = NULL;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;

  SB_ASSERT(pFe);

  pClsComp = pFe->pClsComp;


  soc_sbx_g2p3_cls_schema_to_logprogram(pFe, pSchema->sType, &uLogProgNumber);
  soc_sbx_g2p3_cls_schema_to_program(pFe, pSchema->sType, &uProgramNumber);

  uPhysicalProgramNumber    = uProgramNumber;
  pNewActProgAttr           = pClsComp->pProgAtrib[uPhysicalProgramNumber];

    /* this first ruleset in this RC instance, update program mappings */
#ifdef RULESET_DEBUG
    soc_cm_print("After adj schema correction: \n");
    soc_cm_print("Setting RC 0x%x to ruleset 0x%x, program base address 0x%x,",
                  uRc, uBaseRs, pClsComp->sRsDb[uBaseRs].uBaseAddress);
    soc_cm_print(" Phys Prog Num: 0x%x\n", uPhysicalProgramNumber);
#endif

    sbFe2000RcProgramControlSet(pFe->regSet,
                           uRc,
                           uPhysicalProgramNumber,
                           TRUE,
                           0, /* for C2 length argument is unused */
                           pClsComp->sRsDb[uBaseRs].uBaseAddress);

#ifdef CLS_COMP_DEBUG
    soc_cm_print("\n RCE Program Ctrl: Len[%d] Addr[%d] pno[%d] inst[%d]",\
                pNewActProgAttr->uLength,
                pNewActProgAttr->uBaseAddress,
                uPhysicalProgramNumber, uRc);
#endif
    sbFe2000RcMapLogicaltoPhysicalProgram(pFe->regSet, /* user device handle */
                                          uLogProgNumber,
                                          /* logical program number */
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

sbStatus_t
soc_sbx_g2p3_cls_ruleset_commit(soc_sbx_g2p3_state_t *pFe,
                soc_sbx_g2p3_cls_schema_t *pSchema,
                soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory,
                uint32_t uRs)
{

  sbStatus_t status = SB_OK;
  uint32_t uRcInstance=0;
  uint32_t uId;
  uint32_t uBuffer[SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS];
  uint32_t uPhysicalProgramNumber =0;
  uint8_t  uProgramNumber=0, uLogProgNumber = 0;
  uint32_t uDummy, uFirstRs = -1, uLastRs = -1;
  soc_sbx_g2p3_cls_prog_attr_t *pNewActProgAttr = NULL;
  soc_sbx_g2p3_cls_bank_transfer_t *pBankHandler   = NULL;
  uint32_t *pDmaBase                      = NULL;
  sbFe2000DmaOp_t  *pDmaOp                = NULL;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  sbZfFe2000RcSbPattern_t *pPattern       = NULL;
  uint32_t uNumberOfDmaBlocks =  (pSchema->uInstCount) / SB_FE2000XT_RC_INST_PER_DMA;
  sbZfC2RcDmaFormat_t zDmaBlock;
  uint32_t uDmaBlock, uInst, uInstIndex, uSchemaInst, uPatternIdx, uSb, uPatternWord;
  uint32_t uPrevRs = -1, uNextRs = -1, uBaseRs = -1; 
  uint32_t uSetNumber,uJumpTarget;
  uint32_t uDmaBlockJump=0, uAdjDmaBlockJump=0, uAdjSchemaLen;
  uint32_t uJumpInst, uJumpOffset=0, uAdjJumpOffset=0;
  uint8_t bCache=0;
  /* Adjacent schema type in the same program */
  soc_sbx_g2p3_cls_schema_enum_t adjSchemaType;

  SB_ASSERT(pFe);
  /* must be modulo */
  SB_ASSERT((pSchema->uInstCount % SB_FE2000XT_RC_INST_PER_DMA) == 0);


  pClsComp = pFe->pClsComp;

  SB_ASSERT( uNumberOfDmaBlocks <= 80);

  /* obtain the program attributes and switch the states of programs */
  soc_sbx_g2p3_cls_schema_to_program(pFe,
                                    pSchema->sType,
                                    &uProgramNumber);

  uPhysicalProgramNumber    = uProgramNumber;
  pNewActProgAttr           = pClsComp->pProgAtrib[uPhysicalProgramNumber];

  uSetNumber = uRs % pClsComp->uMaxRs;

  if (pNewActProgAttr->sType == SB_G2P3_FE_CLS_G2P3_SCHEMA ||
      pNewActProgAttr->sType == SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA) {
#ifdef RULESET_DEBUG
    soc_cm_print("  Ingress Set: 0x%x\n", uSetNumber);
#endif
  }else{
#ifdef RULESET_DEBUG
    soc_cm_print("  Egress Set: 0x%x\n", uSetNumber);
#endif
  }


  uRcInstance = pClsComp->sRsDb[uRs].uRcInstance;
  uPrevRs = pClsComp->sRsDb[uRs].uPrevRs;
  uNextRs = pClsComp->sRsDb[uRs].uNextRs;
  soc_sbx_g2p3_cls_schema_to_logprogram(pFe, pSchema->sType, &uLogProgNumber);

  if(pClsComp->uMaxSchema > 2) {
      if((uNextRs == -1) || 
        (pClsComp->sRsDb[uNextRs].uRcInstance != uRcInstance)) {
          if( pSchema->sType == SB_G2P3_FE_CLS_G2P3_SCHEMA || 
              pSchema->sType == SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA) {

              adjSchemaType = pSchema->sType + 2;    
              soc_sbx_g2p3_cls_ruleset_get_first_last(pFe,
                                                  uProgramNumber,
                                                  uLogProgNumber,
                                                  adjSchemaType,
                                                  &uFirstRs,
                                                  &uDummy,
                                                  uRcInstance);
          }
      }

      if((uPrevRs == -1) || 
         (pClsComp->sRsDb[uPrevRs].uRcInstance != uRcInstance)) {
          if( pSchema->sType == SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA || 
              pSchema->sType == SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA) {

              adjSchemaType = pSchema->sType - 2;     
              soc_sbx_g2p3_cls_ruleset_get_first_last(pFe,
                                       uProgramNumber,
                                       uLogProgNumber,
                                       adjSchemaType,
                                       &uBaseRs,
                                       &uLastRs,
                                       uRcInstance);
          }
      }
  }

  /* build single DMA buffer */
  /* Caladan2 shares the BankA RC 0 memory allocation */
  pBankHandler   = &pClsComp->bankASlabs[0];
  pDmaOp        = &pBankHandler->dmaop;
  pDmaBase      = (uint32_t *) pBankHandler->pDmaMemory;
  pDmaBase  = pDmaBase;

  for (uDmaBlock=0; uDmaBlock < uNumberOfDmaBlocks; uDmaBlock++) {
      sbZfC2RcDmaFormat_InitInstance(&zDmaBlock);
      for (uInst=0; uInst < SB_FE2000XT_RC_INST_PER_DMA; uInst++) {
          /* Set Instruction */
          uInstIndex = (uDmaBlock * SB_FE2000XT_RC_INST_PER_DMA) + uInst;
          uSchemaInst = uInstIndex % pSchema->uInstCount;
          /* Update Set field of the new rule instruction */
          if (uSchemaInst == (pSchema->uInstCount-1)) {
              sbFe2000ClsUpdateNewRuleSet(&(pSchema->pInstructions[uSchemaInst]), uSetNumber);
          }

          /* overwrite the jump instruction flavor and specify the jump target address
           * if this is the last set we don't have a jump target
           * the jump instruction is offset 24 from the end
           * Also, squirrel away the necessary info for set linking below
           */
          if ( uSchemaInst == (pSchema->uInstCount-25)) {
            bCache = 1;
            uDmaBlockJump = uDmaBlock;
            uJumpOffset = uInst;
            if (pClsComp->sRsDb[uRs].uTerminal) {
              uJumpTarget = -1;
                if (uFirstRs !=-1) {
                    uJumpTarget = pClsComp->sRsDb[uFirstRs].uBaseAddress >> 3;
                }
            } else {
              uNextRs = pClsComp->sRsDb[uRs].uNextRs;
              uJumpTarget = pClsComp->sRsDb[uNextRs].uBaseAddress >> 3;
            }
            sbFe2000ClsUpdateJumpTarget(&(pSchema->pInstructions[uSchemaInst]), uJumpTarget);
            uJumpInst = pSchema->pInstructions[uSchemaInst];
          }
          SB_ZF_C2RCDMAFORMAT_SET_INSTR(uInst,
                                         pSchema->pInstructions[uSchemaInst],
                                         (uint32_t*)(&zDmaBlock));

          for (uPatternIdx=0; uPatternIdx < SB_FE2000XT_RC_WORDS_PER_PATTERN * SB_FE2000XT_RC_NUM_SUPERBLOCKS; uPatternIdx++) {
              /* Set Pattern */
              uSb = uPatternIdx / SB_FE2000XT_RC_WORDS_PER_PATTERN;
              uPatternWord = uPatternIdx % SB_FE2000XT_RC_WORDS_PER_PATTERN;
              /* Extended mode only ever uses a single rule set.  The others are there for legacy and A/B
                 support */
              pPattern = &(pPatternMemory->pSbPattern[0][0][uSb][uInstIndex]);
              sbZfC2RcDmaFormat_SetPatternWord((uint32_t*)(&zDmaBlock),
                                               uSb,
                                               uInst,
                                               uPatternWord,
                                               ((uint32_t*)pPattern)[3 - uPatternWord]);
          }
      }

    /* Pack the DMA buffers */
    sbZfC2RcDmaFormat_Pack(&zDmaBlock, (uint8_t*)uBuffer, SB_ZF_C2RCDMAFORMAT_SIZE_IN_BYTES);
    for(uId=0; uId < SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS; uId++) {
      pDmaBase[(uDmaBlock*SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS)+uId] = uBuffer[uId];
    }

    /* cache block with jump instruction for ruleset swapping */
    if (bCache == 1) {
        SB_MEMCPY(&pClsComp->sRsDb[uRs].zDmaCache, &uBuffer[0], SB_ZF_C2RCDMAFORMAT_SIZE_IN_BYTES);
        bCache = 0; /* reset cache flag */

    }

  }
  /* Dma offset is given by the ProgramOffset*sizeof(rc dmablock)/4 ( # of words) */
  pDmaOp->words       = (pSchema->uInstCount*SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS/SB_FE2000XT_RC_INST_PER_DMA);
  pDmaOp->feAddress   = SB_FE2000_DMA_MAKE_ADDRESS(                   \
                            (uRcInstance == 0) ? SB_FE2000_MEM_CLS_0  \
                                               : SB_FE2000_MEM_CLS_1, \
                             pClsComp->sRsDb[uRs].uBaseAddress * \
                             SB_FE2000XT_RC_INSTRUCTION_WORDS_IN_DMA_BUFFER);

  pDmaOp->hostAddress = pDmaBase;
  pDmaOp->opcode      = SB_FE2000_DMA_OPCODE_WRITE;
  pDmaOp->data        = pFe;

  status = sbFe2000DmaRequest(pClsComp->pDmaCtxt, pFe->DmaCfg.handle,
                              pDmaOp);

  if (status != SB_OK)
    return status;

  pClsComp->sRsDb[uRs].uCommitted = 1;

  /* link prior ruleset to this one...
   * if prior ruleset doesn't exist or is in a different RC, update program mappings
   */
  uPrevRs = pClsComp->sRsDb[uRs].uPrevRs;

  /* Corner case...
   * if this is first ruleset in the program and it is in RC1 need to point RC0 program
   * to a dummy
   * better to pre-allocate a dummy program and set aside the space.  If a dummy is needed,
   * simply point the program map there, but do not allocate a ruleset...
   */
  if ( (uPrevRs == -1) || \
       (pClsComp->sRsDb[uPrevRs].uRcInstance != uRcInstance) ) {
    /* this first ruleset in this RC instance, update program mappings */
#ifdef RULESET_DEBUG
    soc_cm_print("Setting RC 0x%x to ruleset 0x%x,", uRcInstance, uRs);
    soc_cm_print(" program base address 0x%x, Phys Prog Num: 0x%x\n",
                 pClsComp->sRsDb[uRs].uBaseAddress, uPhysicalProgramNumber);
#endif

    if((uBaseRs == -1 ) ||
            !(pSchema->sType == SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA || 
              pSchema->sType == SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA)) {

        uBaseRs = uRs;

    } else {
        uAdjSchemaLen = pClsComp->sRsDb[uLastRs].uSize;
        uAdjDmaBlockJump = (uAdjSchemaLen - 25) / SB_FE2000XT_RC_INST_PER_DMA;
        uAdjJumpOffset = (uAdjSchemaLen - 25) % SB_FE2000XT_RC_INST_PER_DMA;
        soc_sbx_g2p3_cls_ruleset_relink(pFe, 
                                uLastRs,
                                uRs,
                                uAdjDmaBlockJump,
                                uAdjJumpOffset, 0);
    }
  
#ifdef RULESET_DEBUG
    soc_cm_print("After adj schema correction: \n");
    soc_cm_print("Setting RC 0x%x to ruleset 0x%x, program base address 0x%x,",
                  uRcInstance, uBaseRs, pClsComp->sRsDb[uBaseRs].uBaseAddress);
    soc_cm_print(" Phys Prog Num: 0x%x\n", uPhysicalProgramNumber);
#endif


    sbFe2000RcProgramControlSet(pFe->regSet,
                           uRcInstance,
                           uPhysicalProgramNumber,
                           TRUE,
                           0, /* for C2 length argument is unused */
                           pClsComp->sRsDb[uBaseRs].uBaseAddress);

#ifdef CLS_COMP_DEBUG
    soc_cm_print("\n RCE Program Ctrl: Len[%d] Addr[%d] pno[%d] inst[%d]",\
                pNewActProgAttr->uLength,
                pNewActProgAttr->uBaseAddress,
                uPhysicalProgramNumber, uRcInstance);
#endif
    sbFe2000RcMapLogicaltoPhysicalProgram(pFe->regSet, /* user device handle */
                                          uLogProgNumber,
                                          /* logical program number */
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
  }else{
    uPrevRs = pClsComp->sRsDb[uRs].uPrevRs;
    if (pClsComp->sRsDb[uPrevRs].uSize !=
             pClsComp->sRsDb[uRs].uSize) {
      uDmaBlockJump = (pClsComp->sRsDb[uPrevRs].uSize - 25) / SB_FE2000XT_RC_INST_PER_DMA;
      uJumpOffset   = (pClsComp->sRsDb[uPrevRs].uSize - 25) % SB_FE2000XT_RC_INST_PER_DMA;
    }
    /* relink prior ruleset to newly committed */
    soc_sbx_g2p3_cls_ruleset_relink(pFe, 
                                pClsComp->sRsDb[uRs].uPrevRs,
                                uRs,
                                uDmaBlockJump,
                                uJumpOffset, 1);

  }

  return SB_OK;
}
sbStatus_t
soc_sbx_g2p3_cls_commit_c2(soc_sbx_g2p3_state_t *pFe,
                soc_sbx_g2p3_cls_schema_t *pSchema1,
                soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                soc_sbx_g2p3_cls_rule_db_e_t eDb,
                adq_t *pRuleList,
                uint32_t uRuleCount,
                soc_sbx_g2p3_cls_group_t *pClsGroups,
                uint8_t uGroupCount,
                pfRuleCb pfRuleIter,
                pfFinalizeCb pfFinalize)
{
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  adq_t *pRuleElem = NULL;
  uint32_t uNumRs=0;
  sbStatus_t err = SB_OK;
  uint32_t uLastRs, uAllocedRs, uNextRs, uRs = -1, uOldRs;
  uint32_t uRulesInSet;
  uint32_t uOrphRs = -1, uOrphRuleOffset;
  uint32_t uRuleOffset;
  int32_t nRuleIndex, nRs, uGroupRs;
  uint32_t uNewBank = 0, uOldBank = 1, uTemp, uBankedMode = 0;
  uint8_t uProgramNumber;
  uint8_t uLogProgNumber = 0;
  uint32_t uNewBackUpProgramNumber, uPhysicalProgramNumber;
  soc_sbx_g2p3_cls_prog_attr_t *pNewActProgAttr = NULL;
  soc_sbx_g2p3_cls_prog_attr_t *pNewBkupProgAttr = NULL;
  void * dummy;
  soc_sbx_g2p3_cls_ace_pattern_u_t dummy_pattern;
  uint32_t uAdjFirstRs = -1, uAdjLastRs = -1;
  soc_sbx_g2p3_cls_schema_enum_t adjSchemaType;
  int8_t uNextGroupId = -1;
#ifdef DYNAMIC_MEMMGR
  uint8_t uRc;
  adq_t *pRuleElemCache[G2P3_MAX_RC_INSTANCE];
  int16_t uGroupId=0;
  int32_t index;
  int8_t iRcPrefNext = -1;
  int8_t uAllocCondition = 0;
  soc_sbx_g2p3_cls_schema_t *pSchema=pSchema1;
  soc_sbx_g2p3_cls_schema_t *pSchemaSwapRs[G2P3_MAX_RC_INSTANCE];
  uint32_t   uSwapRsRuleCount[G2P3_MAX_RC_INSTANCE];
  uint8_t   bSwapInit[G2P3_MAX_RC_INSTANCE];
  uint32_t uDirty = 0, uRcInstance;
  adq_t *pTmpRuleElem = NULL;
  pRuleElemCache[0] = pRuleElemCache[1] = NULL;
  bSwapInit[0] = bSwapInit[1] = 0;
  pSchemaSwapRs[0] = pSchemaSwapRs[1] = NULL;
  uSwapRsRuleCount[0] = uSwapRsRuleCount[1] = 0;
#endif

  soc_sbx_g2p3_cls_schema_to_logprogram(pFe, pSchema->sType, &uLogProgNumber);
  uBankedMode = pSchema->layout.uBankedMode;
  if (uBankedMode == 0) {
      /* Remove the rulesets of groups with rulecount = 0 */
      for (index = (uGroupCount -1); index >=0; index--) {
          uGroupId = pClsGroups[index].uGroupId;
          if (pClsGroups[index].uRuleCount != 0 ) {
              continue;
          }
          pClsGroups[index].uNumRs = 0;
          err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                              eDb,
                                              uLogProgNumber,
                                              pSchema->sType,
                                              uNewBank,
                                              uGroupId,
                                              &uLastRs,
                                              &uAllocedRs);
          if (err){
              return err;
          }

          if (uLastRs != -1) {
              soc_sbx_g2p3_cls_group_remove(pFe, pSchema, pPatternMemory, 
                                            eDb, uGroupId, uLastRs, pfFinalize);
          }
      }
  }

  err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                          eDb,
                                          uLogProgNumber,
                                          pSchema->sType,
                                          2,
                                          -1,
                                          &uLastRs,
                                          &uAllocedRs);
  if (err){
      return err;
  }

  /* switch banks (if in BankedMode) */
  pClsComp = pFe->pClsComp;
  if ( (uBankedMode == 1) &&
       (uLastRs!= -1) ) {
      if (pClsComp->sRsDb[uLastRs].uCommitted == 1) {
          uOldBank = pClsComp->sRsDb[uLastRs].uBank;
      } else {
          uOldBank = pClsComp->sRsDb[uLastRs].uBank ? 0 : 1;
      }
      uNewBank = uOldBank ? 0 : 1;
      err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                          eDb,
                                          uLogProgNumber,
                                          pSchema->sType,
                                          uNewBank,
                                          -1,
                                          &uLastRs,
                                          &uAllocedRs);
      if (err){
          return err;
      }
  }

  /* Identify the place where the groups could go - in Rc0 / RC1 / both */
  soc_sbx_g2p3_cls_group_ruleset_identify(pFe, pSchema->sType, pClsGroups,
                                          uGroupCount, uLastRs);

  for (index = (uGroupCount - 1); index >= 0; index--) {
      uRuleCount = pClsGroups[index].uRuleCount;
      uGroupId = pClsGroups[index].uGroupId;
#ifdef RULESET_DEBUG
      soc_cm_print("Commit DB: 0x%x, uGroupId: 0x%x, Rule Count: 0x%x\n",
                                               eDb, uGroupId, uRuleCount);
#endif
      if (uRuleCount == 0) {
          pClsGroups[index].uNumRs = 0;
          continue;
      }
      pSchema = pClsGroups[index].pSchemaDesc;

      if (iRcPrefNext == 0) {
          pClsGroups[index].iRc = iRcPrefNext;
      }

      /* Set prior ruleset to the currently allocated rule set for this program.
       * Also get total number of rulesets allocated for this program. */
      err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                              eDb,
                                              uLogProgNumber,
                                              pSchema->sType,
                                              uNewBank,
                                              uGroupId,
                                              &uLastRs,
                                              &uAllocedRs);
      if (err){
          return err;
      }

      /* calculate number of rulesets needed */
      uNumRs = ( (uRuleCount - 1)/ G2P3_MAX_CLS_RULES_PER_RULESET_C2) + 1;
      pClsGroups[index].uNumRs = uNumRs;


#ifdef RULESET_DEBUG
      soc_cm_print("Currently allocated rulesets: 0x%x\n", uAllocedRs);
      soc_sbx_g2p3_cls_ruleset_linkage_dump(pFe->unit, eDb, uLogProgNumber,
                                                    pSchema->sType, uNewBank);
#endif
      /* set the next ruleset index to terminating */
      uNextRs = -1;
      uRs = -1;

      /* allocate additional rulesets if needed */
      if (uNumRs > uAllocedRs) {
#ifdef RULESET_DEBUG
          soc_cm_print("Allocating additional rulesets: 0x%x\n", (uNumRs - uAllocedRs));
#endif
          err = soc_sbx_g2p3_cls_ruleset_alloc(pFe,
                                         eDb,
                                         uLogProgNumber,
                                         pSchema,
                                         uGroupId,
                                         uNextGroupId,
                                         uNewBank,
                                         (uNumRs - uAllocedRs),
                                         pClsGroups[index].iRc,
                                         &iRcPrefNext,
                                         &uAllocCondition );
          if (err){
              return err;
          }

          if (uAllocCondition == 1) {
#ifdef RULESET_DEBUG
              soc_cm_print("\ncalling push_to_rc1_dummy\n");
#endif
              soc_sbx_g2p3_cls_commit_push_to_rc1_dummy(pFe,
                                                  pSchema,
                                                  pPatternMemory,
                                                  eDb,
                                                  uGroupId);
              pClsGroups[index].iRc = -1;
              index++;
              continue;
          } else if (uAllocCondition == 2) {
#ifdef RULESET_DEBUG
              soc_cm_print("\ncalling push_to_rc1\n");
#endif
              /* find the RS at the end of RC0 and move it to RC1 */
              soc_sbx_g2p3_cls_commit_push_to_rc1(pFe,
                                                  pClsGroups,
                                                  pSchema->sType,
                                                  pPatternMemory,
                                                  eDb,
                                                  uNewBank,
                                                  uGroupCount,
                                                  pfRuleIter,
                                                  pfFinalize);
              index++;

          } else if (uAllocCondition == 3) {
#ifdef RULESET_DEBUG
              soc_cm_print("\ncalling push_to_rc0\n");
#endif
             /* find the RS at the start of RC1 and move it to RC0 */
              soc_sbx_g2p3_cls_commit_push_to_rc0(pFe,
                                                  pClsGroups,
                                                  pSchema->sType,
                                                  pPatternMemory,
                                                  eDb,
                                                  uNewBank,
                                                  uGroupCount,
                                                  pfRuleIter,
                                                  pfFinalize);
              index++;
          }

      }
      uNextGroupId = uGroupId;
  }
  /* update final ruleset info */
  err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                          eDb,
                                          uLogProgNumber,
                                          pSchema->sType,
                                          uNewBank,
                                          -1,
                                          &uLastRs,
                                          &uAllocedRs);
  if (err){
      return err;
  }

  /* obtain the program attributes and switch the states of programs */
  soc_sbx_g2p3_cls_schema_to_program(pFe,
                                    pSchema->sType,
                                    &uProgramNumber);

  uNewBackUpProgramNumber      = uProgramNumber;
  uPhysicalProgramNumber    = pClsComp->sbyProgNumber[uLogProgNumber];
  pNewActProgAttr           = pClsComp->pProgAtrib[uPhysicalProgramNumber];
  pNewBkupProgAttr          = pClsComp->pProgAtrib[uNewBackUpProgramNumber];

  /* Set active program info */
  pNewActProgAttr->uActive  = 1;

  /* Set Backup program info */
  pNewBkupProgAttr->uActive = 0;
  pClsComp->sbyProgNumber[uLogProgNumber] = uNewBackUpProgramNumber;

  /* build patterns, update rule table, finalize counters */
  uOldRs = uLastRs;
  uNextRs = -1;
  if (ACEDQ_EMPTY(&pRuleList) == FALSE)
    pRuleElem = ACEDQ_TAIL(&pRuleList);

#ifdef RULESET_DEBUG
  soc_cm_print("linkage prior to commits\n");
  soc_sbx_g2p3_cls_ruleset_linkage_dump(pFe->unit, eDb, uLogProgNumber, 
                                                pSchema->sType, uNewBank);
#endif
  uNumRs = 0;
  for (index = uGroupCount - 1; index >= 0; index--) {
      uNumRs = uNumRs + pClsGroups[index].uNumRs;
      if (pClsGroups[index].uNumRs == 0) {
          continue;
      }
  }

  index = uGroupCount;
  uGroupRs = 0;
  for (nRs=(uNumRs-1); nRs>=0; nRs--) {
      uGroupRs--;
      while (uGroupRs < 0 && index > 0) {
          index--;
          uGroupRs = pClsGroups[index].uNumRs - 1;
          pSchema = pClsGroups[index].pSchemaDesc;
          uGroupId = pClsGroups[index].uGroupId;
      }
#ifdef RULESET_DEBUG
      soc_cm_print("nRs: 0x%x, oldRs: 0x%x\n", nRs, uOldRs);
#endif
      SB_ASSERT(uOldRs != -1);

      /* final rule set may not be full */
      if (uGroupRs == (pClsGroups[index].uNumRs-1)) {
          uRulesInSet = pClsGroups[index].uRuleCount - 
                    (G2P3_MAX_CLS_RULES_PER_RULESET_C2 * (uGroupRs));
      }else{
          uRulesInSet = G2P3_MAX_CLS_RULES_PER_RULESET_C2;
      }

      if (uBankedMode == 0) {
        pTmpRuleElem = pRuleElem;
        soc_sbx_g2p3_ace_ruleset_dirty_cb(pFe,
                                 pSchema,
                                 (void*) (&pRuleElem),
                                 &uDirty,
                                 uRulesInSet,
                                 uRulesInSet-1,
                                 pClsComp->sRsDb[uOldRs].uRuleOffset);
        if (!uDirty) {
          uRcInstance = pClsComp->sRsDb[uOldRs].uRcInstance;
          uNextRs = uOldRs;
          pClsComp->sRsDb[uOldRs].uProgNumber = uLogProgNumber;
          uOldRs = pClsComp->sRsDb[uOldRs].uPrevRs;
          uRcInstance = pClsComp->sRsDb[uNextRs].uRcInstance;

          if ( (uOldRs == -1) ||
             (pClsComp->sRsDb[uOldRs].uRcInstance != uRcInstance) ) {
#ifdef RULESET_DEBUG
              soc_cm_print("Setting RC 0x%x to ruleset 0x%x,",
                                                   uRcInstance, uNextRs);
              soc_cm_print(" program base address 0x%x, Phys Prog Num: 0x%x\n",
                pClsComp->sRsDb[uNextRs].uBaseAddress, uPhysicalProgramNumber);
#endif
              sbFe2000RcProgramControlSet(pFe->regSet,
                                    uRcInstance,
                                    uPhysicalProgramNumber,
                                    TRUE,
                                    0,
                                    pClsComp->sRsDb[uNextRs].uBaseAddress);
            }
            /* if this is the alst ruleset fall thru for swap-set handling
             * and memory management
             */
            if (nRs != 0) {
                continue;
            }
        }

      }

      if (uDirty) {
        if (uBankedMode == 0) {      
              /* obtain a swap rule set */
              err = soc_sbx_g2p3_cls_ruleset_rc_alloc(pFe, 
                                                 eDb,
                                                 uLogProgNumber,
                                                 pSchema,
                                                 uGroupId,
                                                 pClsComp->sRsDb[uOldRs].uRcInstance,
                                                 uNewBank,
                                                 &uRs,
                                                 &uRuleOffset);
              if (err){
                  return err;
              }

#ifdef DYNAMIC_MEMMGR
              uRc =  pClsComp->sRsDb[uOldRs].uRcInstance;
              if (bSwapInit[uRc] == 0 && pClsComp->isSwapRsUsed[uRc] == 1) {
                  pRuleElemCache[uRc] = pRuleElem;
                  pSchemaSwapRs[uRc] = pSchema;
                  uSwapRsRuleCount[uRc]  = uRulesInSet;
                  bSwapInit[uRc] = 1;
              }
#endif

              /* copy links from previous ruleset */
              pClsComp->sRsDb[uRs].uNextRs = pClsComp->sRsDb[uOldRs].uNextRs;
              pClsComp->sRsDb[uRs].uPrevRs = pClsComp->sRsDb[uOldRs].uPrevRs;
              pClsComp->sRsDb[uRs].uTerminal = pClsComp->sRsDb[uOldRs].uTerminal;
              /* update next pointer in previous entry (if any) */
              if (pClsComp->sRsDb[uRs].uPrevRs != -1) {
                pClsComp->sRsDb[pClsComp->sRsDb[uRs].uPrevRs].uNextRs = uRs;
              }
              /* update prev pointer in next entry (if any) */
              if (pClsComp->sRsDb[uRs].uNextRs != -1) {
                pClsComp->sRsDb[pClsComp->sRsDb[uRs].uNextRs].uPrevRs = uRs;
              }
            } else {
              uRs = uOldRs;
              uRuleOffset = pClsComp->sRsDb[uRs].uRuleOffset;
        }

          soc_sbx_g2p3_cls_clear_pattern_memory(pFe, pSchema, pPatternMemory);
          for (nRuleIndex=(uRulesInSet-1); nRuleIndex>=0; nRuleIndex--) {
              pfRuleIter(pFe,
                         pSchema,
                         pPatternMemory,
                         (void*)(&pRuleElem),
                         nRuleIndex,
                         uRuleOffset);
          }
        if (uBankedMode == 0) {
              if (nRs == 0) {
                /* save index of orphaned rule sets */
                uOrphRs = pClsComp->sRsDb[uRs].uPrevRs;
                pClsComp->sRsDb[uRs].uPrevRs = -1;
              }
        }
          /* commit rule set */
          err = soc_sbx_g2p3_cls_ruleset_commit(pFe,
                                         pSchema,
                                         pPatternMemory,
                                         uRs);
          if (err){
              return err;
          }
        if (uBankedMode == 0) {
              /* invoke finalize_cb */
              if (uOldRs != -1) {
                pfFinalize(pFe,
                         eDb,
                         pClsComp->sRsDb[uOldRs].uRuleOffset);
              }

              if (nRs == 0) {
                 /* finalize orphaned rulesets */
                while (uOrphRs != -1) {
#ifdef RULESET_DEBUG
                   soc_cm_print("Freeing orphaned ruleset: 0x%x\n", uOrphRs);
#endif
                   uTemp = pClsComp->sRsDb[uOrphRs].uPrevRs;
                   soc_sbx_g2p3_cls_ruleset_offset_get(pFe,
                                                       uOrphRs,
                                                       &uOrphRuleOffset);
                   soc_sbx_g2p3_cls_free_ruleset(pFe, uOrphRs);

                   /* invoke finalize_cb */
                   pfFinalize(pFe,
                             eDb,
                             uOrphRuleOffset);

                   uOrphRs = uTemp;
                }
              }

              /* free old ruleset */
              uTemp = pClsComp->sRsDb[uOldRs].uPrevRs;
              soc_sbx_g2p3_cls_free_ruleset(pFe, uOldRs);

              /* update indexes */
              uOldRs = uTemp;
              uNextRs = uRs;
            } else {
              uOldRs = pClsComp->sRsDb[uOldRs].uPrevRs;
              uNextRs = uRs;
        }
      } /* if (uDirty) */
  }

  if (uDirty) {
      if (pClsComp->uMaxSchema > 2){
          if (pSchema->sType == SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA ||
              pSchema->sType == SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA) {
              adjSchemaType = pSchema->sType - 2;
          } else {
              adjSchemaType = pSchema->sType + 2;
          }
      }else{
          adjSchemaType = pSchema->sType;
      }

      soc_sbx_g2p3_cls_ruleset_get_first_last(pFe,
                                           uProgramNumber,
                                           uLogProgNumber,
                                           adjSchemaType,
                                           &uAdjFirstRs,
                                           &uAdjLastRs,
                                           0);

      /* check if we need a dummy ruleset in RC0 */
      uNextRs = uRs;
      if (pClsComp->sRsDb[uRs].uRcInstance == 1) { 
          if(uAdjLastRs == -1) {
              /* commit a dummy ruleset */
              soc_sbx_g2p3_cls_clear_pattern_memory(pFe, pSchema, pPatternMemory);
              /* ENHANCEME...create a dummy program and point to it */

              if ((pSchema->sType == SB_G2P3_FE_CLS_G2P3_SCHEMA) || 
                  (pSchema->sType == SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA)) {
                  soc_sbx_g2p3_cls_ace_pattern_init(&dummy_pattern.g2p3);
                  dummy = &dummy_pattern.g2p3;
              } else {
                  soc_sbx_g2p3_cls_ace_pattern_ipv6_init(&dummy_pattern.g2p3_ipv6);
                  dummy = &dummy_pattern.g2p3_ipv6;
              }

              for (nRuleIndex=0; nRuleIndex<G2P3_MAX_CLS_RULES_PER_RULESET_C2; nRuleIndex++) {
                  pSchema->pEncoder(pSchema,
                              nRuleIndex,
                              dummy,
                              pPatternMemory);
              }
              err = soc_sbx_g2p3_cls_ruleset_rc_alloc(pFe, 
                                           eDb,
                                           uLogProgNumber,
                                           pSchema,
                                           uGroupId,
                                           0,
                                           uNewBank,
                                           &uRs,
                                           &uRuleOffset);
              if (err){
                  return err;
              }
#ifdef RULESET_DEBUG
              soc_cm_print("Allocated RC0 dummy ruleset: 0x%x\n", uRs);
#endif
              /* attach to head of program */
              pClsComp->sRsDb[uRs].uNextRs = uNextRs;
              pClsComp->sRsDb[uRs].uPrevRs = -1;
              pClsComp->sRsDb[uNextRs].uPrevRs = uRs;
              pClsComp->sRsDb[uRs].uTerminal = 1;
              pClsComp->sRsDb[uRs].uDummy = 1;
              err = soc_sbx_g2p3_cls_ruleset_commit(pFe,
                                                   pSchema,
                                                   pPatternMemory,
                                                   uRs);
              if (err){
                  return err;
              }
          } else {
              soc_sbx_g2p3_cls_program_adjuster(pFe, pSchema, uAdjFirstRs, 0);
          }
      }
  } /* if (uDirty) */

  if (uBankedMode == 0) {
#ifdef DYNAMIC_MEMMGR
        for (uRc = 0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
            uOldRs = pClsComp->uSwapRs[uRc];
            if (pClsComp->isSwapRsUsed[uRc] == 1) {
#ifdef RULESET_DEBUG
                soc_cm_print("Replacing prealloted swap ruleset for Rc%d\n", uRc);
#endif
                /* final rule set may not be full */
                uRulesInSet = uSwapRsRuleCount[uRc];

                /* obtain a swap rule set */
                err = soc_sbx_g2p3_cls_ruleset_rc_alloc(pFe, 
                                             eDb,
                                             uLogProgNumber,
                                             pSchemaSwapRs[uRc],
                                             pClsComp->sRsDb[uOldRs].uGroupId,
                                             pClsComp->sRsDb[uOldRs].uRcInstance,
                                             uNewBank,
                                             &uRs,
                                             &uRuleOffset);
                if (err){
                    return err;
                }
     
                /* copy links from previous ruleset */
                pClsComp->sRsDb[uRs].uNextRs = pClsComp->sRsDb[uOldRs].uNextRs;
                pClsComp->sRsDb[uRs].uPrevRs = pClsComp->sRsDb[uOldRs].uPrevRs;
                pClsComp->sRsDb[uRs].uTerminal = pClsComp->sRsDb[uOldRs].uTerminal;
                /* update next pointer in previous entry (if any) */
                if (pClsComp->sRsDb[uRs].uPrevRs != -1) {
                    pClsComp->sRsDb[pClsComp->sRsDb[uRs].uPrevRs].uNextRs = uRs;
                }
                /* update prev pointer in next entry (if any) */
                if (pClsComp->sRsDb[uRs].uNextRs != -1) {
                    pClsComp->sRsDb[pClsComp->sRsDb[uRs].uNextRs].uPrevRs = uRs;
                }

                for (nRuleIndex=(uRulesInSet-1); nRuleIndex>=0; nRuleIndex--) {
                    pfRuleIter(pFe,
                               pSchemaSwapRs[uRc],
                               pPatternMemory,
                              (void*)(&pRuleElemCache[uRc]),
                               nRuleIndex,
                               uRuleOffset);
                }

                /* commit rule set */
                err = soc_sbx_g2p3_cls_ruleset_commit(pFe,
                                              pSchemaSwapRs[uRc],
                                              pPatternMemory,
                                              uRs);
                if (err){
                    return err;
                }

                /* invoke finalize_cb */
                if (uOldRs != -1) {
                    pfFinalize(pFe,
                               eDb,
                               pClsComp->sRsDb[uOldRs].uRuleOffset);
                }

                pClsComp->isSwapRsUsed[uRc] = 0;
                pClsComp->sRsDb[uOldRs].sType = -1;
                pClsComp->sRsDb[uOldRs].uProgNumber = -1;
                pClsComp->sRsDb[uOldRs].uCommitted = 0;
            }
        }
  }
#endif

  if (uBankedMode == 1) {
      /* free rulesets in oldbank */
      /* find the last ruleset in old bank */
      err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                              eDb,
                                              uLogProgNumber,
                                              pSchema->sType,
                                              uOldBank,
                                              -1,
                                              &uLastRs,
                                              &uAllocedRs);
      if (err){
          return err;
      }
      /* free the rulesets one by one */
      uOrphRs = uLastRs;
      while (uOrphRs != -1) {
#ifdef RULESET_DEBUG
          soc_cm_print("Freeing orphaned ruleset: 0x%x\n", uOrphRs);
#endif
          uTemp = pClsComp->sRsDb[uOrphRs].uPrevRs;
          soc_sbx_g2p3_cls_ruleset_offset_get(pFe,
                                              uOrphRs,
                                              &uOrphRuleOffset);
          soc_sbx_g2p3_cls_free_ruleset(pFe, uOrphRs);

          /* invoke finalize_cb */
          pfFinalize(pFe,
                     eDb,
                     uOrphRuleOffset);

          uOrphRs = uTemp;
      }
  }

#ifdef RULESET_DEBUG
  soc_cm_print("linkage after commits\n");
  soc_sbx_g2p3_cls_ruleset_linkage_dump(pFe->unit, eDb, uLogProgNumber,
                                               pSchema->sType, uNewBank);
#endif

  soc_sbx_g2p3_cls_update_mem_usage(pFe, pSchema->sType, eDb);

  err = soc_sbx_g2p3_cls_scache_store(pFe);

  return err;
}

sbStatus_t
soc_sbx_g2p3_cls_ruleset_relink(soc_sbx_g2p3_state_t *pFe,
                               uint32_t uRs,
                               uint32_t uNextRs,
                               uint32_t uDmaBlockOffset,
                               uint32_t uInstOffset,
                               uint8_t uToLinkList)
{
  sbStatus_t status = SB_OK;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  sbFe2000DmaOp_t  *pDmaOp                = NULL;
  soc_sbx_g2p3_cls_bank_transfer_t *pBankHandler   = NULL;
  uint32_t *pDmaBase                      = NULL;
  uint32_t uJumpTarget, uJumpInst;
  uint32_t uRcInstance;
  
  pClsComp = pFe->pClsComp;

  uRcInstance = pClsComp->sRsDb[uRs].uRcInstance;
  pBankHandler   = &pClsComp->bankASlabs[0];
  pDmaOp        = &pBankHandler->dmaop;
  pDmaBase      = (uint32_t *) pBankHandler->pDmaMemory;
  pDmaBase  = pDmaBase;

#ifdef RULESET_DEBUG
  soc_cm_print("Linking Rs: 0x%x to Rs: 0x%x\n", uRs, uNextRs);
#endif

  if(uToLinkList == 1) {
      pClsComp->sRsDb[uRs].uNextRs = uNextRs;
      pClsComp->sRsDb[uNextRs].uPrevRs = uRs;
  }

  /* if the ruleset has not yet been committed, all done */
  if (pClsComp->sRsDb[uRs].uCommitted == 0){
    return SOC_E_NONE;
  }

  /* copy cached data into DMA-able memory */
  SB_MEMCPY(pDmaBase, &pClsComp->sRsDb[uRs].zDmaCache, SB_ZF_C2RCDMAFORMAT_SIZE_IN_BYTES);

  /* modify the jump target */
  uJumpTarget = pClsComp->sRsDb[uNextRs].uBaseAddress >> 3;
  uJumpInst = 0xf << 17;  
  /* FIX_ME: Workaround Setting S and M bits */
  uJumpInst = uJumpInst | 0xaa00; 
  sbFe2000ClsUpdateJumpTarget(&uJumpInst, uJumpTarget);
  SB_ZF_C2RCDMAFORMAT_SET_INSTR_DMA(uInstOffset,
                                uJumpInst,
                                (uint8_t*)pDmaBase);

  /* NOTE: no need to update cache value as each subsequent relink will overwrite the stored
   * jump target.  Only the caching of the pattern bits is important */

  /* write modified buffer back */
  pDmaOp->words       = SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS;
  pDmaOp->feAddress   = SB_FE2000_DMA_MAKE_ADDRESS(                   \
                            (uRcInstance == 0) ? SB_FE2000_MEM_CLS_0  \
                                               : SB_FE2000_MEM_CLS_1, \
                             ((pClsComp->sRsDb[uRs].uBaseAddress + \
                              (uDmaBlockOffset * SB_FE2000XT_RC_INST_PER_DMA)) * \
                             SB_FE2000XT_RC_INSTRUCTION_WORDS_IN_DMA_BUFFER));

  pDmaOp->hostAddress = pDmaBase;
  pDmaOp->opcode      = SB_FE2000_DMA_OPCODE_WRITE;
  pDmaOp->data        = pFe;

  status = sbFe2000DmaRequest(pClsComp->pDmaCtxt, pFe->DmaCfg.handle,
                              pDmaOp);

  return status;
}

PRIVATE
int
soc_sbx_g2p3_cls_ruleset_get_first_last(soc_sbx_g2p3_state_t *pFe,
                                 uint8_t uProgramNumber,
                                 uint8_t uLogProgramNumber,
                                 soc_sbx_g2p3_cls_schema_enum_t sType,
                                 uint32_t *pFirstRs,
                                 uint32_t *pLastRs,
                                 uint32_t uRcInstance)
{
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_schema_t *pSchema = NULL;
  soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
  uint32_t uRs, uPrevRs, uNextRs;

  SB_ASSERT(pFe);

  if((G2P3_MAX_CLS_PROGRAMS-1) < uProgramNumber){
    return SOC_E_PARAM;
  }

  pClsComp = pFe->pClsComp;

  pSchema = &pClsComp->schemadb[sType];

  pRsDb = pClsComp->sRsDb;

  *pFirstRs = -1;
  *pLastRs = -1;
  for (uRs=0; uRs<(pClsComp->uMaxRs * pSchema->layout.uNumRcInstance); uRs++) {
    if ( (pRsDb[uRs].uProgNumber == uLogProgramNumber) && \
         (pRsDb[uRs].sType == sType) && \
         (pRsDb[uRs].uAlloc == 1) &&
         (pRsDb[uRs].uRcInstance == uRcInstance) ) {
          uPrevRs = pRsDb[uRs].uPrevRs;
          uNextRs = pRsDb[uRs].uNextRs;
          if (uPrevRs == -1) {
              *pFirstRs = uRs;
          } else if(pRsDb[uPrevRs].uRcInstance != uRcInstance ) {
              *pFirstRs = uRs;
          }

         if (uNextRs == -1) {
             *pLastRs = uRs;
         } else if (pRsDb[uNextRs].uRcInstance != uRcInstance) {
             *pLastRs = uRs;
         }
     }
  }

#ifdef RULESET_DEBUG
  soc_cm_print("Get first last sType[%d], uRc[%d], first[%d], last[%d]\n",
                           sType, uRcInstance, *pFirstRs, *pLastRs);
#endif

  return SOC_E_NONE;
}

PRIVATE
sbStatus_t
soc_sbx_g2p3_cls_group_remove(soc_sbx_g2p3_state_t *pFe,
                soc_sbx_g2p3_cls_schema_t *pSchema,
                soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                soc_sbx_g2p3_cls_rule_db_e_t eDb,
                int16_t uGroupId,
                uint32_t uLastRs,
                pfFinalizeCb pfFinalize)
{
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    sbStatus_t err = SB_OK;
    uint32_t uRs, uPrevRs, uFirstRs, uPrevGroupRs, uNextGroupRs;
    uint32_t uOrphRs = -1, uOrphRuleOffset;
    uint32_t uRuleOffset, uJumpBlock, uJumpOffset;
    int32_t nRuleIndex;
    uint32_t uNewBank = 0, uTemp;
    uint8_t uProgramNumber, uLogProgNumber = 0, bDummy = 0;
    uint32_t uPhysicalProgramNumber;
    void * dummy;
    soc_sbx_g2p3_cls_ace_pattern_u_t dummy_pattern;
    uint32_t uRc0StartAddress = -1, uRc1StartAddress = -1;
    pClsComp = pFe->pClsComp;

    soc_sbx_g2p3_cls_schema_to_logprogram(pFe, pSchema->sType, &uLogProgNumber);
    soc_sbx_g2p3_cls_schema_to_program(pFe, pSchema->sType, &uProgramNumber);
    
    uPhysicalProgramNumber    = uProgramNumber;

    /* Find the first Rs of the Group */
    uPrevRs = pClsComp->sRsDb[uLastRs].uPrevRs;
    uFirstRs = uLastRs;
    while (uPrevRs != -1 && pClsComp->sRsDb[uPrevRs].uGroupId == uGroupId
                         && pClsComp->sRsDb[uPrevRs].uDummy != 1) {
        uFirstRs = uPrevRs;
        uPrevRs = pClsComp->sRsDb[uPrevRs].uPrevRs;
    }

    /* Find the lastRs of previous group and first Rs of next group */    
    uPrevGroupRs = uPrevRs;
    uNextGroupRs = pClsComp->sRsDb[uLastRs].uNextRs;

    if (uNextGroupRs != -1) {
        pClsComp->sRsDb[uNextGroupRs].uPrevRs = uPrevGroupRs;
    }

    if (uPrevGroupRs != -1) {
        /* Link the previous group last Rs to next group first Rs */
        pClsComp->sRsDb[uPrevGroupRs].uNextRs = uNextGroupRs;
        pClsComp->sRsDb[uNextGroupRs].uPrevRs = uPrevGroupRs;

        if (pClsComp->sRsDb[uPrevGroupRs].uRcInstance ==
                pClsComp->sRsDb[uNextGroupRs].uRcInstance) {
              /* if previous and next group are in same Rc, 
               * just relink the rulesets
               */
              uJumpBlock = (pClsComp->sRsDb[uPrevGroupRs].uSize - 25) /
                                             SB_FE2000XT_RC_INST_PER_DMA;
              uJumpOffset = (pClsComp->sRsDb[uPrevGroupRs].uSize - 25) %
                                             SB_FE2000XT_RC_INST_PER_DMA;
              soc_sbx_g2p3_cls_ruleset_relink(pFe,
                                              uPrevGroupRs,
                                              uNextGroupRs,
                                              uJumpBlock,
                                              uJumpOffset, 1);
        } else {
            /* previous and next group are in different Rc,
             * i.e., previous group in Rc0 and next group Rc1
             */
            pClsComp->sRsDb[uPrevGroupRs].uTerminal = 1;
            if (pClsComp->sRsDb[uLastRs].uRcInstance == 1) {
            /* The lastrs of this group is in Rc1. So update the
             * startaddress of the program to first ruleset of next group
             */
                uRc1StartAddress = pClsComp->sRsDb[uNextGroupRs].uBaseAddress;
            }
        }
    } else {
        /* no previous group, i.e., this group is the first group */
        if (pClsComp->sRsDb[uNextGroupRs].uRcInstance == 0) {
            /* next group is in Rc0, update the start address of the program */
            uRc0StartAddress = pClsComp->sRsDb[uNextGroupRs].uBaseAddress;
        } else {
            /* the next group is in Rc1 */
            if (pClsComp->sRsDb[uLastRs].uRcInstance == 1) {
                /* The last rs of this group is in Rc1. So update the start
                 * address of the program to first ruleset of next group */
                uRc1StartAddress = pClsComp->sRsDb[uNextGroupRs].uBaseAddress;
            }
            if (pClsComp->sRsDb[uFirstRs].uRcInstance == 0) {
                /* Next group is in Rc1 && the firstRs of this group is in Rc0
                 * need to create a dummy ruleset */
                bDummy = 1;
            }
        }
    }

    /* free the rulesets belongs to this group */
    pClsComp->sRsDb[uFirstRs].uPrevRs = -1;
    uOrphRs = uLastRs;
    while (uOrphRs != -1) {
        /* Free uLastRs */
        uTemp = pClsComp->sRsDb[uOrphRs].uPrevRs;
        soc_sbx_g2p3_cls_ruleset_offset_get(pFe,
                                            uOrphRs,
                                            &uOrphRuleOffset);
        soc_sbx_g2p3_cls_free_ruleset(pFe, uOrphRs);

        /* invoke finalize_cb */
        pfFinalize(pFe,
                     eDb,
                     uOrphRuleOffset);

        uOrphRs = uTemp;
    }

    /* Write uRc1StartAddress */
    if (uRc1StartAddress != -1) {
#ifdef RULESET_DEBUG
        soc_cm_print("Setting RC1 to program base address 0x%x,",
                                                       uRc1StartAddress);
        soc_cm_print(" Phys Prog Num: 0x%x\n", uPhysicalProgramNumber);
#endif

        sbFe2000RcProgramControlSet(pFe->regSet,
                                 1,
                                 uPhysicalProgramNumber,
                                 TRUE,
                                 0, /* for C2 length argument is unused */
                                 uRc1StartAddress);

    }

    /* Write uRc0 start Address */
    if(uRc0StartAddress != -1) {
#ifdef RULESET_DEBUG
        soc_cm_print("Setting RC0 to program base address 0x%x,",
                                                     uRc0StartAddress);
        soc_cm_print(" Phys Prog Num: 0x%x\n", uPhysicalProgramNumber);
#endif

        sbFe2000RcProgramControlSet(pFe->regSet,
                                 0,
                                 uPhysicalProgramNumber,
                                 TRUE,
                                 0, /* for C2 length argument is unused */
                                 uRc0StartAddress);
    }

    if (bDummy != 0) {
        /* alloc a dummy ruleset and commit */
        soc_sbx_g2p3_cls_clear_pattern_memory(pFe, pSchema, pPatternMemory);
        soc_sbx_g2p3_cls_ace_pattern_init(&dummy_pattern.g2p3);
        dummy = &dummy_pattern.g2p3;

        for (nRuleIndex = 0; nRuleIndex < G2P3_MAX_CLS_RULES_PER_RULESET_C2;
                            nRuleIndex++) {
            pSchema->pEncoder(pSchema,
                              nRuleIndex,
                              dummy,
                              pPatternMemory);
        }
        err = soc_sbx_g2p3_cls_ruleset_rc_alloc(pFe, 
                                       eDb,
                                       uLogProgNumber,
                                       pSchema,
                                       uGroupId,
                                       0,
                                       uNewBank,
                                       &uRs,
                                       &uRuleOffset);
        if (err) {
            return err;
        }
#ifdef RULESET_DEBUG
        soc_cm_print("Allocated RC0 dummy ruleset: 0x%x\n", uRs);
#endif
        /* attach to head of program */
        pClsComp->sRsDb[uRs].uNextRs = uNextGroupRs;
        pClsComp->sRsDb[uRs].uPrevRs = -1;
        pClsComp->sRsDb[uNextGroupRs].uPrevRs = uRs;
        pClsComp->sRsDb[uRs].uTerminal = 1;
        pClsComp->sRsDb[uRs].uDummy = 1;
        err = soc_sbx_g2p3_cls_ruleset_commit(pFe,
                                               pSchema,
                                               pPatternMemory,
                                               uRs);
        if (err) {
            return err;
        }
    }
    return SB_OK;
}


sbStatus_t
soc_sbx_g2p3_cls_update_mem_usage(soc_sbx_g2p3_state_t *pFe,
                                  soc_sbx_g2p3_cls_schema_enum_t sType,
                                  soc_sbx_g2p3_cls_rule_db_e_t eDb)
{
    uint32_t uLastRs, uAllocedRs, uRs;
    uint32_t uRsUsed[G2P3_MAX_RC_INSTANCE], uMemUsed[G2P3_MAX_RC_INSTANCE];
    uint32_t uRsTobeUsed[G2P3_MAX_RC_INSTANCE];
    uint32_t uMemTobeUsed[G2P3_MAX_RC_INSTANCE];
    uint8_t uLogProgNumber = 0, uRc;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    sbStatus_t err = SB_OK;

    SB_ASSERT(pFe);
    pClsComp = pFe->pClsComp;

    soc_sbx_g2p3_cls_schema_to_logprogram(pFe, sType, &uLogProgNumber);
    /* find the number of alloced Rs of this db */
    err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                          eDb,
                                          uLogProgNumber,
                                          sType,
                                          2,
                                          -1,
                                          &uLastRs,
                                          &uAllocedRs);

    /* Init Rs and Mem Used */
    for (uRc = 0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
        uRsUsed[uRc] = 0;
        uMemUsed[uRc] = 0;
    }

    /* find the rulesets & memory used in each Rc for this db */
    uRs = uLastRs;
    while (uRs != -1) {
        uRc = pClsComp->sRsDb[uRs].uRcInstance;
        uRsUsed[uRc]++;
        uMemUsed[uRc] = uMemUsed[uRc] + pClsComp->sRsDb[uRs].uSize;
        uRs = pClsComp->sRsDb[uRs].uPrevRs;
    }

    /* Update the memory usage for this sType*/
    for (uRc = 0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
        pClsComp->usagePerSchema[sType][uRc].uRsUsed = uRsUsed[uRc];
        pClsComp->usagePerSchema[sType][uRc].uMemUsed = uMemUsed[uRc];
        pClsComp->usagePerSchema[sType][uRc].uRsTobeUsed = 0;
        pClsComp->usagePerSchema[sType][uRc].uMemTobeUsed = 0;
    }

    /* Update the total memory usage for each Rc */
    for (uRc = 0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
        uRsUsed[uRc] = 0;
        uRsTobeUsed[uRc] = 0;
        uMemUsed[uRc] = 0;
        uMemTobeUsed[uRc] = 0;
        for (sType = 0; sType < pClsComp->uMaxSchema; sType++) {
            uRsUsed[uRc] = uRsUsed[uRc] +
                         pClsComp->usagePerSchema[sType][uRc].uRsUsed;
            uMemUsed[uRc] = uMemUsed[uRc] +
                         pClsComp->usagePerSchema[sType][uRc].uMemUsed;
            uRsTobeUsed[uRc] = uRsTobeUsed[uRc] +
                      pClsComp->usagePerSchema[sType][uRc].uRsTobeUsed;
            uMemTobeUsed[uRc] = uMemTobeUsed[uRc] +
                     pClsComp->usagePerSchema[sType][uRc].uMemTobeUsed;
        }
        pClsComp->usagePerRc[uRc].uRsUsed = uRsUsed[uRc];
        pClsComp->usagePerRc[uRc].uMemUsed = uMemUsed[uRc];
        pClsComp->usagePerRc[uRc].uRsTobeUsed = uRsTobeUsed[uRc];
        pClsComp->usagePerRc[uRc].uMemTobeUsed = uMemTobeUsed[uRc];
    }
    return SB_OK;
}

sbStatus_t
soc_sbx_g2p3_cls_commit_push_to_rc1_dummy(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_schema_t *pSchema,
                            soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            int16_t   uGroupId)
{
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    sbStatus_t err = SB_OK;
    uint32_t uRs, uRc0FirstRs, uRc0LastRs, uRc1FirstRs;
    uint8_t uProgramNumber, uPhysicalProgramNumber;
    uint8_t uLogProgNumber = 0;
    int32_t nRuleIndex;
    uint32_t uNewBank = 0;
    uint32_t uRuleOffset;
    soc_sbx_g2p3_cls_ace_pattern_u_t dummy_pattern;
    void *dummy;
#ifdef RULESET_DEBUG
    soc_cm_print("\nPUSH TO RC1 DUMMY\n");
#endif
    pClsComp = pFe->pClsComp;
    soc_sbx_g2p3_cls_schema_to_logprogram(pFe, pSchema->sType, &uLogProgNumber);
    soc_sbx_g2p3_cls_schema_to_program(pFe, pSchema->sType, &uProgramNumber);
    
    uPhysicalProgramNumber    = uProgramNumber;

    /* find start Rs of Rc0 */ 
    soc_sbx_g2p3_cls_ruleset_get_first_last(pFe,
                                            uProgramNumber,
                                            uLogProgNumber,
                                            pSchema->sType,
                                            &uRc0FirstRs,
                                            &uRc0LastRs,
                                            0);

   uRc1FirstRs = pClsComp->sRsDb[uRc0LastRs].uNextRs;

    /* Commit a dummy Rs for this Group in RC1, this will write 
       the starting address of the program */
    /* commit a dummy ruleset */
    soc_sbx_g2p3_cls_clear_pattern_memory(pFe, pSchema, pPatternMemory);

    if ((pSchema->sType == SB_G2P3_FE_CLS_G2P3_SCHEMA) || 
          (pSchema->sType == SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA)) {
        soc_sbx_g2p3_cls_ace_pattern_init(&dummy_pattern.g2p3);
        dummy = &dummy_pattern.g2p3;
    } else {
        soc_sbx_g2p3_cls_ace_pattern_ipv6_init(&dummy_pattern.g2p3_ipv6);
        dummy = &dummy_pattern.g2p3_ipv6;
    }

    for (nRuleIndex = 0; nRuleIndex < G2P3_MAX_CLS_RULES_PER_RULESET_C2;
                                                          nRuleIndex++) {
        pSchema->pEncoder(pSchema,
                          nRuleIndex,
                          dummy,
                          pPatternMemory);
    }
    err = soc_sbx_g2p3_cls_ruleset_rc_alloc(pFe, 
                                       eDb,
                                       uLogProgNumber,
                                       pSchema,
                                       uGroupId,
                                       1,
                                       uNewBank,
                                       &uRs,
                                       &uRuleOffset);
    if (err){
        return err;
    }
#ifdef RULESET_DEBUG
    soc_cm_print("Allocated RC1 dummy ruleset - pushing group to RC1: 0x%x\n",
                                                                         uRs);
#endif
    /* attach the ruleset to the ruleset link */
    pClsComp->sRsDb[uRs].uNextRs = uRc1FirstRs;
    pClsComp->sRsDb[uRs].uPrevRs = uRc0LastRs;
    pClsComp->sRsDb[uRc0LastRs].uNextRs = uRs;
    pClsComp->sRsDb[uRc1FirstRs].uPrevRs = uRs;
    /* commit the ruleset */
    err = soc_sbx_g2p3_cls_ruleset_commit(pFe,
                                          pSchema,
                                          pPatternMemory,
                                          uRs);
    if (err){
        return err;
    }

   return SB_OK;
}


sbStatus_t
soc_sbx_g2p3_cls_commit_push_to_rc1(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_group_t *pClsGroups,
                            soc_sbx_g2p3_cls_schema_enum_t sType,
                            soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            uint32_t uBank,
                            uint32_t   uGroupCount,
                            pfRuleCb pfRuleIter,
                            pfFinalizeCb pfFinalize)
{
    uint32_t uRs, uRc0FirstRs, uRc0LastRs, uRc0GroupRsCount = 0;
    uint32_t uRc0FreeRsCount, uRc1FreeRsCount, uNumRs, uFirstRs;
    uint32_t uRcInstance = 0;
    int16_t uGroupId, uNextGroupId, index;
    uint8_t uLogProgNumber = 0, uProgramNumber;
    int8_t uAllocCondition = 0;
    int8_t iRcPrefNext = -1;
    sbStatus_t err = SB_OK;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_schema_t *pSchema;
    

    SB_ASSERT(pFe);

    pClsComp = pFe->pClsComp;
    
    soc_sbx_g2p3_cls_schema_to_logprogram(pFe, sType, &uLogProgNumber);
    soc_sbx_g2p3_cls_schema_to_program(pFe, sType, &uProgramNumber);

    /* find the group to be moved
     * 1. Find the last Rs in Rc0 for the given sType
     * 2. Get the group Id from the rs
     */
    soc_sbx_g2p3_cls_ruleset_get_first_last(pFe,
                                            uProgramNumber,
                                            uLogProgNumber,
                                            sType,
                                            &uRc0FirstRs,
                                            &uRc0LastRs,
                                            uRcInstance);

    uGroupId = pClsComp->sRsDb[uRc0LastRs].uGroupId;

    for (index = 0; index < uGroupCount; index++) {
        if(pClsGroups[index].uGroupId == uGroupId) {
            break;
        }
    }

    pSchema = pClsGroups[index].pSchemaDesc;

    /* find the no of rs free in RC1 */
    soc_sbx_g2p3_cls_ruleset_free_count_get(pFe,
                                            pSchema->uInstCount,
                                            uBank,
                                            &uRc0FreeRsCount,
                                            &uRc1FreeRsCount);

    /* find the no of rs of this group in RC0 */
    uRs = uRc0LastRs;
    while (uRs != -1 && pClsComp->sRsDb[uRs].uGroupId == uGroupId) {
        uRc0GroupRsCount++;
        uRs = pClsComp->sRsDb[uRs].uPrevRs;
    }

    uNextGroupId = -1;

    /* alloc the min(countRc0, countRc1) rulesets in RC1 */
    uNumRs = uRc0GroupRsCount < uRc1FreeRsCount ? uRc0GroupRsCount :
                                                  uRc1FreeRsCount;

                                     
    err = soc_sbx_g2p3_cls_ruleset_alloc(pFe,
                                         eDb,
                                         uLogProgNumber,
                                         pSchema,
                                         uGroupId,
                                         uNextGroupId,
                                         uBank,
                                         uNumRs,
                                         1,
                                         &iRcPrefNext,
                                         &uAllocCondition );
    if (err){
        return err;
    }

    /* commit the group */
    err = soc_sbx_g2p3_cls_commit_group_c2(pFe,
                                           pPatternMemory,
                                           eDb,
                                           uBank,
                                           uLogProgNumber,
                                           &pClsGroups[index],
                                           pfRuleIter,
                                           pfFinalize,
                                           -1,
                                           -1,
                                           &uFirstRs);

    if (err){
        return err;
    }

    /* update Rc0 base address */

   return SB_OK;
}

 

sbStatus_t
soc_sbx_g2p3_cls_commit_push_to_rc0(soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_group_t *pClsGroups,
                            soc_sbx_g2p3_cls_schema_enum_t sType,
                            soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            uint32_t uBank,
                            uint32_t   uGroupCount,
                            pfRuleCb pfRuleIter,
                            pfFinalizeCb pfFinalize)
{
    uint32_t uRs, uRc1FirstRs, uRc1LastRs, uRc1GroupRsCount = 0;
    uint32_t uRc0FreeRsCount, uRc1FreeRsCount, uFirstRs, uAllocedRs;
    uint32_t uRcInstance = 1;
    int32_t uNeededRs, uRsToStayInRc1, uNumRs;
    int16_t uGroupId, uNextGroupId = -1, index, uThisGroupIndex;
    uint8_t uLogProgNumber = 0, uProgramNumber;
    int8_t uAllocCondition = 0;
    int8_t iRcPrefNext = -1;
    int8_t nRuleIndex;
    sbStatus_t err = SB_OK;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_schema_t *pSchema;
    soc_sbx_g2p3_cls_group_t clsGroupTemp;
    adq_t *pRuleElem = NULL;
    

    SB_ASSERT(pFe);

    pClsComp = pFe->pClsComp;
    
    soc_sbx_g2p3_cls_schema_to_logprogram(pFe, sType, &uLogProgNumber);
    soc_sbx_g2p3_cls_schema_to_program(pFe, sType, &uProgramNumber);

    /* find the group to be moved
     * 1. Find the first Rs in Rc1 for the given sType
     * 2. Get the group Id from the rs
     */
    soc_sbx_g2p3_cls_ruleset_get_first_last(pFe,
                                            uProgramNumber,
                                            uLogProgNumber,
                                            sType,
                                            &uRc1FirstRs,
                                            &uRc1LastRs,
                                            uRcInstance);

    uGroupId = pClsComp->sRsDb[uRc1FirstRs].uGroupId;
    for (index = 0; index < uGroupCount; index++) {
        if(pClsGroups[index].uGroupId == uGroupId) {
            break;
        }
    }
    uThisGroupIndex = index;
    pSchema = pClsGroups[uThisGroupIndex].pSchemaDesc;


    /* find the no of rs free in RC0 */
    soc_sbx_g2p3_cls_ruleset_free_count_get(pFe,
                                            pSchema->uInstCount,
                                            uBank,
                                            &uRc0FreeRsCount,
                                            &uRc1FreeRsCount);

   /* find the no of rs of this group in RC1 */
    uRs = uRc1FirstRs;
    while (uRs != -1 && pClsComp->sRsDb[uRs].uGroupId == uGroupId) {
        uRc1GroupRsCount++;
        uRs = pClsComp->sRsDb[uRs].uNextRs;
    }

    /* find the number of alloced Rs of this group */
    err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                          eDb,
                                          uLogProgNumber,
                                          pSchema->sType,
                                          uBank,
                                          uGroupId,
                                          &uRc1LastRs,
                                          &uAllocedRs);
    if (err){
        return err;
    }

    /* find the number of rulesets needed */
    uNumRs = ((pClsGroups[uThisGroupIndex].uRuleCount - 1) /
                   G2P3_MAX_CLS_RULES_PER_RULESET_C2) + 1;
    pClsGroups[uThisGroupIndex].uNumRs = uNumRs;

    uNeededRs = uNumRs - uAllocedRs;
    if (uNeededRs >= uRc0FreeRsCount) {
        /* no space for new rulesets */
        return SOC_E_RESOURCE;
    }

 
    /* alloc the min(Rc0FreeRsCount, (Rc1GroupRsCount + uNeededRs) rs*/
    uNumRs = uRc0FreeRsCount < (uRc1GroupRsCount + uNeededRs) ?
                         uRc0FreeRsCount : (uRc1GroupRsCount + uNeededRs);

    uRs = pClsComp->sRsDb[uRc1LastRs].uNextRs;
    if (uRs != -1) {
        uNextGroupId = pClsComp->sRsDb[uRs].uGroupId;
    }

    if (uNumRs > 0) {
        
        err = soc_sbx_g2p3_cls_ruleset_alloc(pFe,
                                         eDb,
                                         uLogProgNumber,
                                         pSchema,
                                         uGroupId,
                                         uNextGroupId,
                                         uBank,
                                         uNumRs,
                                         0,
                                         &iRcPrefNext,
                                         &uAllocCondition );
        if (err){
            return err;
        }
        /* Calculate no of rulesets to stay in Rc1 */
        uRsToStayInRc1 = (uRc1GroupRsCount + uNeededRs) - uNumRs;    
    } else {
        uRsToStayInRc1 = 0;
    }

    if (uRsToStayInRc1 > 0) {
        clsGroupTemp.uNumRs = pClsGroups[uThisGroupIndex].uNumRs
                                                 - uRsToStayInRc1;
        clsGroupTemp.uRuleCount = clsGroupTemp.uNumRs * 
                                   G2P3_MAX_CLS_RULES_PER_RULESET_C2;

        /* iterate through the rules list to get pass 
         * the rules to be commited in RC1 */
        nRuleIndex = pClsGroups[uThisGroupIndex].uRuleCount
                                           - clsGroupTemp.uRuleCount;
        pRuleElem = pClsGroups[uThisGroupIndex].pRuleList;
        for ( ; nRuleIndex > 0; nRuleIndex--) {
            pfRuleIter(pFe, NULL, NULL, (void*)(&pRuleElem), 0, 0);
        }
        clsGroupTemp.pRuleList = pRuleElem;
    } else {
        clsGroupTemp.uRuleCount = pClsGroups[uThisGroupIndex].uRuleCount;
        clsGroupTemp.uNumRs = pClsGroups[uThisGroupIndex].uNumRs;
        clsGroupTemp.pRuleList = pClsGroups[uThisGroupIndex].pRuleList;
        /* Update the Rc preference for this group and next group */
        if (uNextGroupId != -1) {
             index = uThisGroupIndex + 1;
            while (index < uGroupCount && pClsGroups[index].uRuleCount == 0) {
                index++;
            }
            pClsGroups[index].iRc = -1;
        }
    }
    clsGroupTemp.pSchemaDesc = pClsGroups[uThisGroupIndex].pSchemaDesc;
    clsGroupTemp.uGroupId    = pClsGroups[uThisGroupIndex].uGroupId;

    /* commit the rules starting from (uNumRs - RsStayinRc1) * 768 to 0 */
    soc_sbx_g2p3_cls_ruleset_get_first_last(pFe,
                                            uProgramNumber,
                                            uLogProgNumber,
                                            sType,
                                            &uRc1FirstRs,
                                            &uRc1LastRs,
                                            uRcInstance);
   /* commit the group */
    err = soc_sbx_g2p3_cls_commit_group_c2(pFe,
                                           pPatternMemory,
                                           eDb,
                                           uBank,
                                           uLogProgNumber,
                                           &clsGroupTemp,
                                           pfRuleIter,
                                           pfFinalize,
                                           uRc1LastRs,
                                           0,
                                           &uFirstRs);

    if (err){
        return err;
    }

    if (uRsToStayInRc1 > 0) {
    /* commit the remaing rules in RC1 */
        clsGroupTemp.uRuleCount = pClsGroups[uThisGroupIndex].uRuleCount
                                     - clsGroupTemp.uRuleCount;
        clsGroupTemp.uNumRs = uRsToStayInRc1;
        clsGroupTemp.pRuleList = pClsGroups[uThisGroupIndex].pRuleList;
        /* commit the rulesets in RC1 */
        err = soc_sbx_g2p3_cls_commit_group_c2(pFe,
                                           pPatternMemory,
                                           eDb,
                                           uBank,
                                           uLogProgNumber,
                                           &clsGroupTemp,
                                           pfRuleIter,
                                           pfFinalize,
                                           -1,
                                           1,
                                           &uFirstRs);

        if (err){
            return err;
        }

    }
    /* free the rulesets */
    return SB_OK;
}


sbStatus_t
soc_sbx_g2p3_cls_ruleset_alloc(soc_sbx_g2p3_state_t *pFe,
                               uint8_t uProgramNumber,
                               uint8_t uLogProgramNumber,
                               soc_sbx_g2p3_cls_schema_t *pSchema,
                               int16_t   uGroupId,
                               int16_t   uNextGroupId,
                               uint32_t uBank,
                               uint32_t uNumRs,
                               int8_t   iRcPref,
                               int8_t   *iRcPrefNext,
                               int8_t  *uAllocCondition)
{
  uint32_t uCount = uNumRs, uBankedMode;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_schema_enum_t sType;
  soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
  uint32_t uRs;
  uint32_t uRc0LastRs = -1, uRc0NextRs = -1, uRc1FirstRs = -1, uTempRs;
  uint32_t uFreeRsCount = 0;
  uint32_t uLastRs, uFirstRs, uNextRs, uPrevRs, uGroupLastRs, uDummy;
  uint32_t uNextGroupFirstRs = -1, uPrevGroupLastRs = -1, uStartRs;
  int32_t uFreeRsMemCountRc0 = 0, uFreeRsMemCountRc1 = 0, uRc;
  int32_t uFreeRsCountRc0 = 0, uFreeRsCountRc1 = 0, uFreeCount ;
  int result;
  sbStatus_t err = SB_OK;

  SB_ASSERT(pFe);

  if((G2P3_MAX_CLS_PROGRAMS-1) < uProgramNumber){
    return SOC_E_PARAM;
  }

  pClsComp = pFe->pClsComp;

  sType = pSchema->sType;

  pRsDb = pClsComp->sRsDb;

  *uAllocCondition = 0;

  /* RULESET ALLOCATION ALGORITHM
   * allocate rulesets from RC1 first and link them at the end
   * if needed, allocate additional rulesets from RC0 and link them
   * after the last existing RC0 ruleset in the program (or at the head if none)
   */

  /* get current last ruleset in program */
  err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                          uProgramNumber,
                                          uLogProgramNumber,
                                          sType,
                                          uBank,
                                          uGroupId,
                                          &uGroupLastRs,
                                          &uDummy);
  if (err){
      return err;
  }

  shr_mem_avl_free_count(pClsComp->mem_avl_ptr[1][uBank], pSchema->uInstCount,
                                                         &uFreeRsMemCountRc1);
  shr_mem_avl_free_count(pClsComp->mem_avl_ptr[0][uBank], pSchema->uInstCount,
                                                         &uFreeRsMemCountRc0);

  uBankedMode = pSchema->layout.uBankedMode;
  /* verify space is available */
  for (uRs=0; uRs<(pClsComp->uMaxRs * pSchema->layout.uNumRcInstance); uRs++) {
    if (pRsDb[uRs].uAlloc == 0 && pRsDb[uRs].uBank == uBank){
        if (uBankedMode == 1) {
          if (uBank == 0) {
            uTempRs = uRs + (pClsComp->uMaxRs / 2);
          } else {
            uTempRs = uRs - (pClsComp->uMaxRs / 2);
          }
          if ( !(pRsDb[uTempRs].uAlloc == 0 || pRsDb[uTempRs].sType == sType)) {
            continue;
          }
        }
        uFreeRsCount++;
        if (pRsDb[uRs].uRcInstance == 0) {
            uFreeRsCountRc0++;
        } else {
            uFreeRsCountRc1++;
        }
     }
  }

  *iRcPrefNext = -1;
  uFreeRsCountRc0 = uFreeRsCountRc0 < uFreeRsMemCountRc0 ?
                                 uFreeRsCountRc0 : uFreeRsMemCountRc0;
  uFreeRsCountRc1 = uFreeRsCountRc1 < uFreeRsMemCountRc1 ?
                                 uFreeRsCountRc1 : uFreeRsMemCountRc1;
  uFreeRsCount = uFreeRsCountRc0 + uFreeRsCountRc1;


  if (uGroupLastRs != -1) {
#ifdef RULESET_ALLOC_DEBUG
    soc_cm_print("Ruleset_alloc: Last ruleset for this group (%d) :0x%x\n",
                                                  uGroupId, uGroupLastRs);
#endif
  /* The RC0 rulesets will be inserted between uRc0LastRs and uNextRs */
  /* The RC1 rulesets will be inserted after uGroupLastRs */
      uRs = uGroupLastRs;
      while (uRs != -1 && pRsDb[uRs].uGroupId == uGroupId) {
          uFirstRs = uRs;
          if (pRsDb[uRs].uRcInstance == 0) {
              uRc0LastRs = uRs;
              
              uRc0NextRs = pRsDb[uRs].uNextRs;
              break;
          }
          uRs = pRsDb[uRs].uPrevRs;
      }
      if (uRs != -1 && pRsDb[uRs].uRcInstance == 0) {
          uRc0LastRs = uRs;
          uRc0NextRs = pRsDb[uRs].uNextRs;
      }
  }

  if (uGroupLastRs == -1) {
#ifdef RULESET_ALLOC_DEBUG
    soc_cm_print("Ruleset_alloc: No ruleset for this group (%d) found\n",
                                                               uGroupId);
#endif
      /* get current last ruleset in program */
      err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                          uProgramNumber,
                                          uLogProgramNumber,
                                          sType,
                                          uBank,
                                          uNextGroupId,
                                          &uLastRs,
                                          &uDummy);
      if (err) {
          return err;
      }
      uRs = uLastRs;
      /* Find the previous Groups last Rs */
      while (uRs != -1 && pRsDb[uRs].uGroupId == uNextGroupId
                       && pRsDb[uRs].uDummy != 1) {
           uNextGroupFirstRs = uRs;
           uRs = pRsDb[uRs].uPrevRs;
      }
      uPrevGroupLastRs = uRs;
      if (uPrevGroupLastRs != -1 && 
                      pRsDb[uPrevGroupLastRs].uRcInstance == 0) {
          uRc0LastRs = uPrevGroupLastRs;
          uRc0NextRs = pRsDb[uRs].uNextRs;
      }
  }

  /* Check for both number of RuleSetEntries and FreeSpace */
  if (iRcPref == 0) {
      if (uFreeRsCountRc0 < uNumRs) {
          /* We dont have enough space in RC0 */
          if (uFreeRsCount >= uNumRs) {
          /* there is some space in Rc1, we can push some Rs to Rc1
             and manage */
              if (uRc0NextRs != -1 && 
                     pRsDb[uRc0NextRs].uRcInstance == 1) {
              /* this Group is at end of RC0, its easy to add RS in Rc1 */
                  *uAllocCondition = 1;
#ifdef RULESET_ALLOC_DEBUG
                  soc_cm_print("Ruleset_alloc: Group is at end of Rc0");
                  soc_cm_print(", should push rulesets to Rc1\n");
#endif
                  return SB_OK;
              } else {
              /* this Group is not at end of RC0, 
                 need to push some other group */
#ifdef RULESET_ALLOC_DEBUG
                  soc_cm_print("Ruleset_alloc: Group in Rc0");
                  soc_cm_print(", should push rulesets to Rc1\n");
#endif
                  *uAllocCondition = 2;
                  return SB_OK;
              }
          }
#ifdef RULESET_ALLOC_DEBUG
          soc_cm_print("Ruleset_alloc: Resource not available\n");
#endif
          return SOC_E_RESOURCE;
      }
  } else if (iRcPref == 1) {
      if (uFreeRsCountRc1 < uNumRs) {
          /* We dont have enough space in RC1 */
          if (uFreeRsCount >= uNumRs) {
          /* there is some space in RC0, we can push some Rs to RC0
             and manage */
#ifdef RULESET_ALLOC_DEBUG
             soc_cm_print("Ruleset_alloc: should push rulesets to Rc0\n");
#endif
             *uAllocCondition = 3;
             return SB_OK;
          }
#ifdef RULESET_ALLOC_DEBUG
          soc_cm_print("Ruleset_alloc: Resource not available\n");
#endif
          return SOC_E_RESOURCE;
      }
  } else {
     if (uFreeRsCount < uNumRs) {
#ifdef RULESET_ALLOC_DEBUG
          soc_cm_print("Ruleset_alloc: Resource not available\n");
#endif
          return SOC_E_RESOURCE;
      }
  }

  if (pSchema->layout.uBankedMode) {
      /* Banked Mode */
      
/*      SB_ASSERT(0); */
  }

 
  for (uRc = pSchema->layout.uNumRcInstance - 1; uRc >= 0; uRc--) {
    if ((uRc == 1) && (iRcPref == 0)) {
      continue;
    }
    if ((uRc == 0) && (iRcPref == 1)) {
      continue;
    }
    uStartRs = pClsComp->uMaxRs * (pSchema->layout.uNumRcInstance - uRc - 1);
    if (uRc == 0) {
      uFreeCount = uFreeRsCountRc0;
    } else {
      uFreeCount = uFreeRsCountRc1;
    }
    if (uGroupLastRs == -1) {
      uPrevRs = uPrevGroupLastRs;
      uNextRs = uNextGroupFirstRs;
      if (uRc == 1 && iRcPref == -1) {
          uRc1FirstRs = uNextRs;
      }
      if (uRc == 0 && iRcPref == -1) {
        uNextRs = uRc1FirstRs;
      }
    } else {
      if (uRc == 0) {
        if (uRc0LastRs == -1 && uRc0NextRs == -1) {
#ifdef RULESET_ALLOC_DEBUG
          soc_cm_print("Ruleset_alloc: RC preference is 0, should be 1\n");
#endif
          return -1;
        }
        uPrevRs = uRc0LastRs;
        uNextRs = uRc0NextRs;
      } else {
        if (pRsDb[uGroupLastRs].uRcInstance != 1) {
#ifdef RULESET_ALLOC_DEBUG
          soc_cm_print("Ruleset_alloc: RC preference is 1, should be 0\n");
#endif
          return -1;
        }
        uPrevRs = uGroupLastRs;
        uNextRs = pRsDb[uGroupLastRs].uNextRs;
      }
    }
    for (uRs = uStartRs; uRs < (uStartRs + pClsComp->uMaxRs); uRs++) { 
      if (uFreeRsCount == 0) {
        break;
      }
      if (pRsDb[uRs].uAlloc == 0 && pRsDb[uRs].uBank == uBank) {
        if (uBankedMode == 1) {
          if (uBank == 0) {
            uTempRs = uRs + (pClsComp->uMaxRs / 2);
          } else {
            uTempRs = uRs - (pClsComp->uMaxRs / 2);
          }
          if ( !(pRsDb[uRs].uAlloc == 0 || pRsDb[uRs].sType == sType)) {
            continue;
          }
        }
      /* link in the free ruleset */
#ifdef RULESET_DEBUG
        soc_cm_print ("ALLOC 0x%x\n", uRs);
#endif
#if 0
soc_cm_print("AVL tree prior to MALLOC of RS: 0x%x addr: 0x%x size: 0x%x\n", uRs, pRsDb[uRs].uBaseAddress, pSchema->uInstCount);
shr_mem_avl_free_tree_list(pClsComp->mem_avl_ptr[uRc][pRsDb[uRs].uBank]);
shr_mem_avl_list_output(pClsComp->mem_avl_ptr[uRc][pRsDb[uRs].uBank]);
#endif
        result = shr_mem_avl_malloc(pClsComp->mem_avl_ptr[uRc][uBank], 
                                          pSchema->uInstCount, 
                                          &pRsDb[uRs].uBaseAddress);
        if (result != 0) {
#ifdef RULESET_DEBUG
          soc_cm_print("\nAllocation of ruleset in RC%d failed\n", uRc);
#endif
          break;
        }
        uFreeCount--;
        pRsDb[uRs].uSize = pSchema->uInstCount;
        pRsDb[uRs].uAlloc = 1;
        pRsDb[uRs].uProgNumber = uLogProgramNumber;
        pRsDb[uRs].sType = sType;
        pRsDb[uRs].uGroupId = uGroupId;
        pRsDb[uRs].uCommitted = 0;

#ifdef RULESET_ALLOC_DEBUG
        soc_cm_print("Ruleset_alloc: PrevRs:0x%x AllocRs:0x%x NextRs:0x%x\n",
                                  uPrevRs, uRs, uNextRs);
#endif
        pRsDb[uRs].uNextRs = uNextRs;
        if (uNextRs == -1 || (uRc == 0 && pRsDb[uNextRs].uRcInstance == 1)) {
          pRsDb[uRs].uTerminal = 1;
        }
        if (uNextRs != -1) {
          pRsDb[uNextRs].uPrevRs = uRs;
        }
        pRsDb[uRs].uPrevRs = uPrevRs;
        if (uPrevRs != -1) {
          pRsDb[uPrevRs].uNextRs = uRs;
          pRsDb[uPrevRs].uTerminal = 0;
        }
        if ((uRc == 1) && (uPrevRs == -1 || pRsDb[uPrevRs].uRcInstance == 0)) {
          uRc1FirstRs = uRs;
        }
        uPrevRs = uRs;
        if (uRc == 0) {
          *iRcPrefNext = 0;
        }
        uCount--;
        if (uCount == 0) {
          return SOC_E_NONE;
        }
      }
    }
  }
#ifndef DYNAMIC_MEMMGR
  SB_ASSERT(0);
#endif

  return SOC_E_RESOURCE;
}

int
soc_sbx_g2p3_cls_commit_group_c2(soc_sbx_g2p3_state_t *pFe,
                          soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                          soc_sbx_g2p3_cls_rule_db_e_t eDb,
                          uint32_t uBank,
                          uint8_t uLogProgNumber,
                          soc_sbx_g2p3_cls_group_t *pClsGroup,
                          pfRuleCb pfRuleIter,
                          pfFinalizeCb pfFinalize,
                          uint32_t uLastCommitRs,
                          int8_t iRc,
                          uint32_t *uFirstRs)

{
    sbStatus_t err = SB_OK;
    uint32_t uLastRs, uOldRs, uAllocedRs, uNextRs, uRs, uPrevRs = -1;
    uint32_t uNewBank = 0, uTemp;
    uint32_t uOrphRs = -1, uOrphRuleOffset;
    uint32_t uRuleOffset, uRulesInSet;
    int32_t nRuleIndex, nRs;
    uint8_t uRc;
    uint8_t uNumRs;
    int16_t uGroupId;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_schema_t *pSchema = NULL;
    adq_t *pRuleElem = NULL;
    adq_t *pRuleElemCache[G2P3_MAX_RC_INSTANCE];
    uint32_t uRuleCountCache[G2P3_MAX_RC_INSTANCE];
  
    pRuleElemCache[0]  = pRuleElemCache[1]  = NULL;
    uRuleCountCache[0] = uRuleCountCache[1] = 0;

    pClsComp = pFe->pClsComp;
    pSchema = pClsGroup->pSchemaDesc;
    uNumRs  = pClsGroup->uNumRs;
    uGroupId = pClsGroup->uGroupId;

    if (uLastCommitRs == -1) {
        /* update final ruleset info */
        err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                            eDb,
                                            uLogProgNumber,
                                            pSchema->sType,
                                            uBank,
                                            uGroupId,
                                            &uLastRs,
                                            &uAllocedRs);
        if (err){
            return err;
        }
    } else {
        uLastRs = uLastCommitRs;
    }

    uOldRs = uLastRs;

    pRuleElem = pClsGroup->pRuleList;
#ifdef RULESET_DEBUG
    soc_cm_print("Committing group %d\n", uGroupId);
#endif

    for (nRs = (uNumRs - 1); nRs >=0; nRs--) {
#ifdef RULESET_DEBUG
        soc_cm_print("nRs: 0x%x, oldRs: 0x%x\n", nRs, uOldRs);
#endif
        SB_ASSERT(uOldRs != -1);

        /* final rule set may not be full */
        if (nRs == (uNumRs-1)) {
            uRulesInSet = pClsGroup->uRuleCount - 
                             (G2P3_MAX_CLS_RULES_PER_RULESET_C2 * nRs);
        }else{
            uRulesInSet = G2P3_MAX_CLS_RULES_PER_RULESET_C2;
        }

        /* Cache the rule element at starting of each Rc
         * this is used to replace the pre alloted ruleset when used 
         */ 
        uRc =  pClsComp->sRsDb[uOldRs].uRcInstance;
        if(pRuleElemCache[uRc] == NULL) {
            pRuleElemCache[uRc] = pRuleElem;
            uRuleCountCache[uRc] = uRulesInSet;
        }

        /* obtain a swap rule set */
        err = soc_sbx_g2p3_cls_ruleset_rc_alloc(pFe, 
                                         eDb,
                                         uLogProgNumber,
                                         pSchema,
                                         uGroupId,
                                         pClsComp->sRsDb[uOldRs].uRcInstance,
                                         uNewBank,
                                         &uRs,
                                         &uRuleOffset);
        if (err){
            return err;
        }

        /* copy links from previous ruleset */
        pClsComp->sRsDb[uRs].uNextRs = pClsComp->sRsDb[uOldRs].uNextRs;
        pClsComp->sRsDb[uRs].uPrevRs = pClsComp->sRsDb[uOldRs].uPrevRs;
        pClsComp->sRsDb[uRs].uTerminal = pClsComp->sRsDb[uOldRs].uTerminal;
        /* update next pointer in previous entry (if any) */
        if (pClsComp->sRsDb[uRs].uPrevRs != -1) {
            pClsComp->sRsDb[pClsComp->sRsDb[uRs].uPrevRs].uNextRs = uRs;
        }
        /* update prev pointer in next entry (if any) */
        if (pClsComp->sRsDb[uRs].uNextRs != -1) {
            pClsComp->sRsDb[pClsComp->sRsDb[uRs].uNextRs].uPrevRs = uRs;
        }

        soc_sbx_g2p3_cls_clear_pattern_memory(pFe, pSchema, pPatternMemory);
        for (nRuleIndex=(uRulesInSet-1); nRuleIndex>=0; nRuleIndex--) {
            pfRuleIter(pFe,
                       pSchema,
                       pPatternMemory,
                       (void*)(&pRuleElem),
                       nRuleIndex,
                       uRuleOffset);
        }
        if (nRs == 0) {
            /* save index of orphaned rule sets */
            uOrphRs = pClsComp->sRsDb[uRs].uPrevRs;
            uPrevRs = uOrphRs;
            /* Set uPrevRs = -1, if this is the first Group
             * (or), set uPrevRs to last Rs of previous group
             */
            while (uPrevRs != -1 && 
                   pClsComp->sRsDb[uPrevRs].uGroupId == uGroupId) {
                   if (iRc != -1 && 
                           pClsComp->sRsDb[uPrevRs].uRcInstance != iRc) {
                       break;
                   }
                uPrevRs = pClsComp->sRsDb[uPrevRs].uPrevRs;
            }
            pClsComp->sRsDb[uRs].uPrevRs = uPrevRs;
        }

        /* commit rule set */
        err = soc_sbx_g2p3_cls_ruleset_commit(pFe,
                                     pSchema,
                                     pPatternMemory,
                                     uRs);
        if (err){
            return err;
        }

        /* invoke finalize_cb */
        if (uOldRs != -1) {
            pfFinalize(pFe,
                       eDb,
                       pClsComp->sRsDb[uOldRs].uRuleOffset);
        }

        if (nRs == 0) {
            /* finalize orphaned rulesets */
            while (uOrphRs != -1 && 
                    pClsComp->sRsDb[uOrphRs].uGroupId == uGroupId) {
                if (iRc != -1 && 
                      pClsComp->sRsDb[uPrevRs].uRcInstance != iRc) {
                    break;
                }
#ifdef RULESET_DEBUG
                soc_cm_print("Freeing orphaned ruleset: 0x%x\n", uOrphRs);
#endif
                uTemp = pClsComp->sRsDb[uOrphRs].uPrevRs;
                soc_sbx_g2p3_cls_ruleset_offset_get(pFe,
                                               uOrphRs,
                                               &uOrphRuleOffset);
                soc_sbx_g2p3_cls_free_ruleset(pFe, uOrphRs);

                /* invoke finalize_cb */
                pfFinalize(pFe,
                          eDb,
                          uOrphRuleOffset);

                uOrphRs = uTemp;
            }
        }

        /* free old ruleset */
        uTemp = pClsComp->sRsDb[uOldRs].uPrevRs;
        soc_sbx_g2p3_cls_free_ruleset(pFe, uOldRs);

        /* update indexes */
        uOldRs = uTemp;
        uNextRs = uRs;
    }
    *uFirstRs = uRs;

#ifdef DYNAMIC_MEMMGR
    for (uRc = 0; uRc < G2P3_MAX_RC_INSTANCE; uRc++) {
        uOldRs = pClsComp->uSwapRs[uRc];
        if (pClsComp->isSwapRsUsed[uRc] == 1) {
            uRulesInSet = uRuleCountCache[uRc];

            /* obtain a swap rule set */
            err = soc_sbx_g2p3_cls_ruleset_rc_alloc(pFe, 
                                         eDb,
                                         uLogProgNumber,
                                         pSchema,
                                         pClsComp->sRsDb[uOldRs].uGroupId,
                                         pClsComp->sRsDb[uOldRs].uRcInstance,
                                         uNewBank,
                                         &uRs,
                                         &uRuleOffset);
            if (err){
                return err;
            }
 
            /* copy links from previous ruleset */
            pClsComp->sRsDb[uRs].uNextRs = pClsComp->sRsDb[uOldRs].uNextRs;
            pClsComp->sRsDb[uRs].uPrevRs = pClsComp->sRsDb[uOldRs].uPrevRs;
            pClsComp->sRsDb[uRs].uTerminal = pClsComp->sRsDb[uOldRs].uTerminal;
            /* update next pointer in previous entry (if any) */
            if (pClsComp->sRsDb[uRs].uPrevRs != -1) {
                pClsComp->sRsDb[pClsComp->sRsDb[uRs].uPrevRs].uNextRs = uRs;
            }
            /* update prev pointer in next entry (if any) */
            if (pClsComp->sRsDb[uRs].uNextRs != -1) {
                pClsComp->sRsDb[pClsComp->sRsDb[uRs].uNextRs].uPrevRs = uRs;
            }

            for (nRuleIndex=(uRulesInSet-1); nRuleIndex>=0; nRuleIndex--) {
                pfRuleIter(pFe,
                           pSchema,
                           pPatternMemory,
                          (void*)(&pRuleElemCache[uRc]),
                           nRuleIndex,
                           uRuleOffset);
            }

            /* commit rule set */
            err = soc_sbx_g2p3_cls_ruleset_commit(pFe,
                                          pSchema,
                                          pPatternMemory,
                                          uRs);
            if (err){
                return err;
            }

            /* invoke finalize_cb */
            if (uOldRs != -1) {
                pfFinalize(pFe,
                           eDb,
                           pClsComp->sRsDb[uOldRs].uRuleOffset);
            }

            pClsComp->isSwapRsUsed[uRc] = 0;
            pClsComp->sRsDb[uOldRs].sType = -1;
            pClsComp->sRsDb[uOldRs].uProgNumber = -1;
            pClsComp->sRsDb[uOldRs].uCommitted = 0;
#ifdef RULESET_DEBUG
            soc_cm_print("Swap Ruleset for RC%d freed: ruleset %d\n",
                                                         uRc, uOldRs);
#endif
        }
    }
#endif

    return 0;
}

int soc_sbx_g2p3_cls_ruleset_rc_alloc(soc_sbx_g2p3_state_t *pFe,
                                 uint8_t uProgramNumber,
                                 uint8_t uLogProgramNumber,
                                 soc_sbx_g2p3_cls_schema_t *pSchema,
                                 int16_t  uGroupId,
                                 uint32_t uRc,
                                 uint32_t uBank,
                                 uint32_t *pRs,
                                 uint32_t *pOffset)
{
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_schema_enum_t sType;
  soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
  uint32_t uRs, uRsFreeCount = 0;
#ifdef DYNAMIC_MEMMGR
  int result;
  uint32_t uBaseAddress;
#endif

  SB_ASSERT(pFe);
  SB_ASSERT(pOffset);

  if((G2P3_MAX_CLS_PROGRAMS-1) < uProgramNumber){
    return SOC_E_PARAM;
  }

  pClsComp = pFe->pClsComp;

  sType = pSchema->sType;

  pRsDb = pClsComp->sRsDb;


#ifdef DYNAMIC_MEMMGR
    /* dynamic allocation */
    result = shr_mem_avl_malloc(pClsComp->mem_avl_ptr[uRc][uBank],
                                pSchema->uInstCount,
                                &uBaseAddress);
    for (uRs = 0; uRs < (pClsComp->uMaxRs * pSchema->layout.uNumRcInstance);
                                                                   uRs++) {
        if (pRsDb[uRs].uRcInstance == uRc && pRsDb[uRs].uAlloc == 0
                                         && pRsDb[uRs].uBank == uBank) {
            uRsFreeCount++;
        }
    }
    if (result != 0 || uRsFreeCount == 0) {
        if (result == 0) {
            shr_mem_avl_free(pClsComp->mem_avl_ptr[uRc][uBank],
                             uBaseAddress);
        }

        SB_ASSERT(pClsComp->isSwapRsUsed[uRc]==0);
        uRs = pClsComp->uSwapRs[uRc];
#ifdef RULESET_DEBUG
        soc_cm_print("using preallocated swap Rs 0x%x\n", uRs);
#endif
        pClsComp->isSwapRsUsed[uRc] = 1;
        *pOffset = pRsDb[uRs].uRuleOffset;
        *pRs = uRs;
        pRsDb[uRs].uProgNumber = uLogProgramNumber;
        pRsDb[uRs].sType = sType;
        pRsDb[uRs].uGroupId = uGroupId;
        return SOC_E_NONE;
    }
#endif 

  for (uRs=0; uRs<(pClsComp->uMaxRs * pSchema->layout.uNumRcInstance); uRs++) {
    /* BankedMode allocation (preallocated rulesets) */
    if ( (pRsDb[uRs].uAlloc == 0) &&
         (pRsDb[uRs].uRcInstance == uRc) &&
         (pRsDb[uRs].uBank == uBank) ) {
#ifdef DYNAMIC_MEMMGR
      /* dynamic allocation */
      pRsDb[uRs].uBaseAddress = uBaseAddress;
      pRsDb[uRs].uSize = pSchema->uInstCount;
#endif 
#ifdef RULESET_DEBUG
      soc_cm_print ("ALLOC FOR SWAP 0x%x (%d)\n", uRs, pRsDb[uRs].uBaseAddress);
#endif
      pRsDb[uRs].uAlloc = 1;
      *pOffset = pRsDb[uRs].uRuleOffset;
      *pRs = uRs;
      pRsDb[uRs].uProgNumber = uLogProgramNumber;
      pRsDb[uRs].sType = sType;
      pRsDb[uRs].uGroupId = uGroupId;
      return SOC_E_NONE;
    }

  }

  /* There must always at LEAST one spare ruleset */
  SB_ASSERT(0);

  return SOC_E_NONE;
}

int soc_sbx_g2p3_cls_free_ruleset(soc_sbx_g2p3_state_t *pFe,
                                  uint32_t uRs)
{
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_ruleset_t *pRs = NULL;
  soc_sbx_g2p3_cls_schema_t *pSchema = NULL;
  soc_sbx_g2p3_cls_prog_attr_t *pProgAttr = NULL;
  uint32_t uProgNumber;

  pClsComp = pFe->pClsComp;
  pRs = &pClsComp->sRsDb[uRs];

  SB_ASSERT(pRs->uAlloc == 1);

  uProgNumber = pRs->uProgNumber;

  SB_ASSERT( G2P3_MAX_CLS_PROGRAMS > uProgNumber);

  pProgAttr = pClsComp->pProgAtrib[uProgNumber];
  pSchema = &pClsComp->schemadb[pProgAttr->sType];

  pRs->uNextRs = -1;
  pRs->uPrevRs = -1;
  pRs->uTerminal = 0;
  pRs->uDummy = 0;
  pRs->uCommitted = 0;

  /* preserve program number in banked mode */
/*  if (!pSchema->layout.uBankedMode) { */
    pRs->uProgNumber = -1;
/*  } */
  pRs->uAlloc = 0;

#ifdef RULESET_DEBUG
    soc_cm_print ("FREEING RC%d 0x%x (%d)\n", pRs->uRcInstance, uRs, 
                                                   pRs->uBaseAddress);
#endif
#ifdef DYNAMIC_MEMMGR
  shr_mem_avl_free(pClsComp->mem_avl_ptr[pRs->uRcInstance][pRs->uBank],
                   pRs->uBaseAddress);
  pRs->uBaseAddress=0;
  pRs->uSize = 0;
#endif

  return SOC_E_NONE;
}

int soc_sbx_g2p3_cls_ruleset_offset_get(soc_sbx_g2p3_state_t *pFe,
                                        uint32_t uRs,
                                        uint32_t *pRsOffset)
{
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;

  SB_ASSERT(pFe);

  pClsComp = pFe->pClsComp;

  pRsDb = pClsComp->sRsDb;

  SB_ASSERT(uRs < (pClsComp->uMaxRs * SB_FE2000_NUM_RC_INSTANCES));

  *pRsOffset = pRsDb[uRs].uRuleOffset;

  return SOC_E_NONE;

}

int soc_sbx_g2p3_cls_ruleset_free_count_get(soc_sbx_g2p3_state_t *pFe,
                                            uint32_t uSchemaSize,
                                            uint32_t uBank,
                                            uint32_t *pRc0FreeRsCount,
                                            uint32_t *pRc1FreeRsCount)
{
    int32_t uRc0FreeRsMemCount, uRc1FreeRsMemCount;
    uint32_t uRc0FreeRsCount = 0, uRc1FreeRsCount = 0;
    uint32_t uRs;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;

    SB_ASSERT(pFe);

    pClsComp = pFe->pClsComp;

    shr_mem_avl_free_count(pClsComp->mem_avl_ptr[1][uBank],
                           uSchemaSize, 
                           &uRc1FreeRsMemCount);

    shr_mem_avl_free_count(pClsComp->mem_avl_ptr[0][uBank],
                           uSchemaSize,
                           &uRc0FreeRsMemCount);

    /* verify ruleset space is available */
    for (uRs = 0; uRs < (pClsComp->uMaxRs * G2P3_MAX_RC_INSTANCE); uRs++) {
        if (pClsComp->sRsDb[uRs].uAlloc == 0 && pClsComp->sRsDb[uRs].uBank == uBank){
            if (pClsComp->sRsDb[uRs].uRcInstance == 0) {
                uRc0FreeRsCount++;
            } else {
                uRc1FreeRsCount++;
            }
         }
    }

    uRc0FreeRsCount = uRc0FreeRsCount < uRc0FreeRsMemCount ? 
                           uRc0FreeRsCount : uRc0FreeRsMemCount;
    uRc1FreeRsCount = uRc1FreeRsCount < uRc1FreeRsMemCount ?
                           uRc1FreeRsCount : uRc1FreeRsMemCount;

    *pRc0FreeRsCount = uRc0FreeRsCount;
    *pRc1FreeRsCount = uRc1FreeRsCount;
    return SOC_E_NONE;

}

int soc_sbx_g2p3_cls_ruleset_get_free(soc_sbx_g2p3_state_t *pFe, 
                                      uint32_t *pTotalRs)
{
    uint32_t uRs, uTotalRs=0, uMaxRs=0;
    soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
    soc_sbx_g2p3_cls_schema_t *pSchema = NULL;
 
    pSchema = &pFe->pClsComp->schemadb[0];
    pRsDb = pFe->pClsComp->sRsDb;
    uMaxRs = pFe->pClsComp->uMaxRs * pSchema->layout.uNumRcInstance;

    for (uRs=0; uRs < uMaxRs; uRs++) {
        if ( (pRsDb[uRs].uAlloc != 1) ) {
            uTotalRs++;
        }
    }
    *pTotalRs = uTotalRs;
    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cls_group_ruleset_identify( soc_sbx_g2p3_state_t *pFe,
                                         soc_sbx_g2p3_cls_schema_enum_t sType,
                                         soc_sbx_g2p3_cls_group_t *pClsGroups,
                                         uint8_t uGroupCount,
                                         uint32_t uLastRs)
{
    int16_t uGroupIndex = uGroupCount - 1;
    uint32_t uRs, uPrevRs = -1;
    uint8_t uRc0EndGroup = -1, uRc1StartGroup = -1;
    soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    int8_t iRc;

    SB_ASSERT(pFe);
    uRs = uLastRs;

    pClsComp = pFe->pClsComp;

    pRsDb = pClsComp->sRsDb;


    if (uRs == -1) {
        for (uGroupIndex = uGroupCount - 1; uGroupIndex >=0; uGroupIndex-- ) {
            pClsGroups[uGroupIndex].iRc = -1;
#ifdef RULESET_DEBUG
            soc_cm_print("GroupIndex: %d GroupId : %d  iRc: %d\n", uGroupIndex,
                                               pClsGroups[uGroupIndex].uGroupId,
                                               pClsGroups[uGroupIndex].iRc);
#endif
        }
        return 0;
    }

    /* FIX_ME for two schemas (L2/V4 & V6) in one program */
    
    uPrevRs = pRsDb[uRs].uPrevRs;
    while (uPrevRs != -1 && pRsDb[uPrevRs].sType != sType) {
        uPrevRs = pRsDb[uPrevRs].uPrevRs;
    }

    /* Find the groups at the border RC0 and Rc1 */
    while (uPrevRs != -1) {
        if (pRsDb[uPrevRs].uRcInstance != pRsDb[uRs].uRcInstance) {
            uRc1StartGroup = pRsDb[uRs].uGroupId;
            uRc0EndGroup = pRsDb[uPrevRs].uGroupId;
            break;
        }
        uRs = uPrevRs;
        uPrevRs = pRsDb[uPrevRs].uPrevRs;
        while (uPrevRs != -1 && pRsDb[uPrevRs].sType != sType) {
            uPrevRs = pRsDb[uPrevRs].uPrevRs;
        }
    }

   iRc = 1;
   for ( uGroupIndex = uGroupCount - 1; uGroupIndex >= 0; uGroupIndex-- ) {
       pClsGroups[uGroupIndex].iRc = iRc;
       if (uRc1StartGroup == pClsGroups[uGroupIndex].uGroupId) {
           pClsGroups[uGroupIndex].iRc = -1;
           iRc = 0;
       }
#ifdef DYNAMIC_GROUP_DEBUG
       soc_cm_print("GroupIndex: %d GroupId : %d  iRc: %d\n", uGroupIndex,
                                               pClsGroups[uGroupIndex].uGroupId,
                                               pClsGroups[uGroupIndex].iRc);
#endif
   }

   return 0;
}


int soc_sbx_g2p3_cls_ruleset_get_last(soc_sbx_g2p3_state_t *pFe,
                                 uint8_t uProgramNumber,
                                 uint8_t uLogProgramNumber,
                                 soc_sbx_g2p3_cls_schema_enum_t sType,
                                 uint32_t uBank,
                                 int16_t uGroupId,
                                 uint32_t *pRs,
                                 uint32_t *pTotalRs)
{
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_schema_t *pSchema = NULL;
  soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
  uint32_t uRs, uTotalRs, uNextRs;

  SB_ASSERT(pFe);

  if((G2P3_MAX_CLS_PROGRAMS-1) < uProgramNumber){
    return SOC_E_PARAM;
  }

  pClsComp = pFe->pClsComp;

  pSchema = &pClsComp->schemadb[sType];

  pRsDb = pClsComp->sRsDb;

  uTotalRs = 0;
  *pRs = -1;
  for (uRs=0; uRs<(pClsComp->uMaxRs * pSchema->layout.uNumRcInstance); uRs++) {
    if ( (pRsDb[uRs].uProgNumber == uLogProgramNumber) && \
         (pRsDb[uRs].sType == sType) && \
         (uGroupId == -1 || pRsDb[uRs].uGroupId == uGroupId) && \
         ( 2 <= uBank || pRsDb[uRs].uBank == uBank) && \
         (pRsDb[uRs].uAlloc == 1) ) {
        uTotalRs++;
      if (pRsDb[uRs].uNextRs == -1) {
        *pRs = uRs;
      } else {
        uNextRs = pRsDb[uRs].uNextRs;
        if (uGroupId != -1 && pRsDb[uNextRs].uGroupId != uGroupId) {
            *pRs = uRs;
        }
      }
    }
  }

  *pTotalRs = uTotalRs;

  return SOC_E_NONE;
}

int
soc_sbx_g2p3_cls_ruleset_linkage_dump(uint32_t unit,
                                      uint8_t uProgram,
                                      uint8_t uLogProgramNumber,
                                      soc_sbx_g2p3_cls_schema_enum_t sType,
                                      uint32_t uBank)
{
  sbStatus_t err = SB_OK;
  uint32_t uRs, uDummy;
  soc_sbx_g2p3_cls_t *pClsComp = NULL;
  soc_sbx_g2p3_cls_ruleset_t *pRsDb = NULL;
  soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;


  SB_ASSERT(pFe);

  pClsComp = pFe->pClsComp;
  pRsDb = pClsComp->sRsDb;

  /* find last ruleset and then follow the links */
  err = soc_sbx_g2p3_cls_ruleset_get_last(pFe,
                                    uProgram,
                                    uLogProgramNumber,
                                    sType,
                                    uBank,
                                    -1,
                                    &uRs,
                                    &uDummy);
  if (err){
    return err;
  }

  soc_cm_print ("Program 0x%x\n--------------------------\n", uProgram);
  soc_cm_print ("uRs (RuleOffset) (BaseAddress-Size)(Group)\n");
  while (uRs != -1) {
    soc_cm_print ("0x%x (0x%x) (0x%x-0x%x) (G%d) <--", uRs, pRsDb[uRs].uRuleOffset, pRsDb[uRs].uBaseAddress, pRsDb[uRs].uSize, pRsDb[uRs].uGroupId);
    uRs = pRsDb[uRs].uPrevRs;
  }
  soc_cm_print("\n");
#ifdef DYNAMIC_MEMMGR
#ifdef DYNAMIC_MEMMGR_DEBUG
  shr_mem_avl_list_output(pClsComp->mem_avl_ptr[0][uBank]);
  soc_cm_print("Printing AVL tree List Rc0\n");
  shr_mem_avl_free_tree_list(pClsComp->mem_avl_ptr[0][uBank]);
  shr_mem_avl_list_output(pClsComp->mem_avl_ptr[1][uBank]);
  soc_cm_print("Printing AVL tree List Rc1\n");
  shr_mem_avl_free_tree_list(pClsComp->mem_avl_ptr[1][uBank]);
#endif
#endif
  return SOC_E_NONE;
}

sbStatus_t
soc_sbx_g2p3_cls_wb_init_c2(soc_sbx_g2p3_state_t *pFe, void **wbOut)
{
    uint32_t i, p, dummy, uData, uOffset;
    uint32_t j, uLength, delta;
    sbFe2000DmaOp_t dmaOp;
    soc_sbx_g2p3_cls_wb_rc_t *wbRc;
    soc_sbx_g2p3_cls_wb_prog_t *wbProg;
    sbStatus_t status;

    sal_memset(&dmaOp, 0, sizeof(sbFe2000DmaOp_t));
    status = thin_malloc(0, SB_ALLOC_INTERNAL,
                         sizeof(soc_sbx_g2p3_cls_wb_rc_t) * G2P3_MAX_RC_INSTANCE, (void*)&wbRc, NULL);
    if (status != SOC_E_NONE){
        return status;
    }
    SB_MEMSET(wbRc, 0, sizeof(soc_sbx_g2p3_cls_wb_rc_t) * G2P3_MAX_RC_INSTANCE);

    /* set returned poiner */
    *wbOut = wbRc;


    uOffset = SAND_HAL_REG_OFFSET(C2, LU_RCE_PROGRAM0);
    for (i=0; i < G2P3_MAX_RC_INSTANCE; i++) {

        /* recover program mappings*/
        for (p=0; p < G2P3_MAX_LOGICAL_PROGRAMS; p++) {
            wbProg = &(wbRc[i].prog[p]);
            wbProg->uLogProg = p;
            uData = SAND_HAL_FE2000_READ_OFFS(pFe->regSet, uOffset + (p * 4));
            wbProg->uPhyProg = (uData >> (4 * p)) & 0xf;
            sbFe2000RcProgramControlGet(pFe->regSet, i, wbProg->uPhyProg,
                                        &wbProg->uActive,
                                        &dummy,
                                        &wbProg->uBaseAddress);

        }

        /* DMA out entire memory for RC instance */
        uLength = RCE_INST_MEM_SIZE * SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS / SB_FE2000XT_RC_INST_PER_DMA;

        status = thin_malloc(0, SB_ALLOC_CLS_DMA,
                             uLength * sizeof(uint32_t), (void*)&wbRc[i].dmaBuffer, &wbRc[i].dmaHandle);
        if (status != SB_OK) {
            return status;
        }
        SB_MEMSET(wbRc[i].dmaBuffer, 0, uLength);

        j=0;
        dmaOp.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
        dmaOp.feAddress   = SB_FE2000_DMA_MAKE_ADDRESS(                   \
                                (i == 0) ? SB_FE2000_MEM_CLS_0 : SB_FE2000_MEM_CLS_1, \
                                0);

        dmaOp.hostAddress = wbRc[i].dmaBuffer;
        dmaOp.opcode      = SB_FE2000_DMA_OPCODE_READ;
        dmaOp.data        = pFe;
        delta = SB_ZF_C2RCDMAFORMAT_SIZE_IN_WORDS;
        while (uLength){
            dmaOp.words = 224;
            dmaOp.hostBusAddress = SB_FE2000_DMA_HBA_COMPUTE;
            dmaOp.cb = soc_sbx_g2p3_cls_dma_cb;
            uLength -= (uLength > delta) ? delta : uLength;
            status = sbFe2000DmaRequest(pFe->pDmaMgr, pFe->DmaCfg.handle,
                                        &dmaOp);

            if (status != SB_OK) {
              return status;
            }
            j+=delta;
            dmaOp.feAddress += delta;
            dmaOp.hostAddress += delta;
        }

    }
    return SOC_E_NONE;
}

sbStatus_t
soc_sbx_g2p3_cls_wb_recover_c2(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_cls_schema_t *pSchema)
{
    /* C2 CLS recover is driven by groups through ACE recovey */
    return SOC_E_NONE;
}

sbStatus_t
soc_sbx_g2p3_cls_wb_uninit_c2(soc_sbx_g2p3_state_t *pFe)
{
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_cls_t *pClsComp = NULL;
    soc_sbx_g2p3_cls_wb_rc_t *wbData;
    sbDmaMemoryHandle_t dummy = {0};
    sbStatus_t err = SB_OK;
    pClsComp = pFe->pClsComp;

    /* deallocate RCE program cache */
    wbData = (soc_sbx_g2p3_cls_wb_rc_t*)(pClsComp->wbData);
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
#endif /* BCM_WARM_BOOT_SUPPORT */
    return SB_OK;
}
