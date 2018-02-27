/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
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
 * $Id: g2p3_cls.h 1.44.52.1 Broadcom SDK $
 *-----------------------------------------------------------------------------*/


#ifndef _SB_G2P3_FE_CLS_COMP_H_
#define _SB_G2P3_FE_CLS_COMP_H_

#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include <soc/sbx/fe2k/sbFe2000ClsComp.h>
#include <soc/sbx/fe2k/sbZfFe2000RcSbPattern.hx>
#include <soc/sbx/fe2kxt/sbFe2000Xt.h>
#include <shared/mem_avl.h>
#include <soc/sbx/g2p3/g2p3_ace_internal.h>


enum { 
  SB_G2P3_FE_RC0 = 0, 
  SB_G2P3_FE_RC1 = 1 
};

/* This are the logical program numbers from the ucode. 
 * Ingress uses the program numbers from 0-7 and 
 * the egress uses the program number from 8-14 . as we 
 * want to use the program 1 for the egress we have to use
 * number 9 as the logical program number. so the ingress
 * can't use the program number 1 as it is going to use it.
 */
enum {
  SB_G2P3_FE_CLS_G2_SCHEMA_PROGRAM_NUMBER       = 0,
  SB_G2P3_FE_CLS_G2_EGR_SCHEMA_PROGRAM_NUMBER   = 9,
  SB_G2P3_FE_CLS_G2P3_SCHEMA_PROGRAM_NUMBER     = 0,
  SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA_PROGRAM_NUMBER = 9,
};

enum {
  G2P3_INGRESS_LOGICAL_PROGRAM_NUMBER,
  G2P3_EGRESS_LOGICAL_PROGRAM_NUMBER,
  G2P3_MAX_LOGICAL_PROGRAMS,
};

#define G2P3_MAX_CLS_RULES_PER_RULESET  (256)
#define G2P3_MAX_CLS_RULES_PER_RULESET_C2  (768)
#define G2P3_MAX_CLS_RULES_PER_SUPER_BLOCK_RULESET  (128)
#define G2P3_MAX_RC_INSTANCE   (2)
#define G2P3_DEFAULT_NUM_BATCH (1)

/* For Caladan, hardware supports up to 4 rulesets per schema, but
 * these are limited by the number of instructions
 * available to classification:
 *   ((EPOCH_LEN - (2*48) - 35) * 4)
 * For 20g linerate this is 1088 instructions.
 */
#define G2P3_MAX_CLS_RULESETS_PER_SCHEMA  (4)
#define G2P3_CA_MAX_INST_PER_RC_20G       (1088)

/* For Caladan2 the limitation is based on the program length */
#define G2P3_MAX_CLS_RULESETS_PER_RC_C2 (24)

#define G2P3_MAX_CLS_PROGRAMS             (16)  

#define G2P3_INGRESS_PROGRAM_BASE         (0)
#define G2P3_EGRESS_PROGRAM_BASE          (8)

#define G2P3_DEFAULT_INGRESS_BATCH        (0)
#define G2P3_DEFAULT_EGRESS_BATCH         (1)

#define RCE_INST_MEM_SIZE  (4096)
/* 
 * @enum soc_sbx_g2p3_cls_schema_t
 * For now we are going to start iwth independent schema for 
 * different class of acls. And we will integrate everything
 * in the master schema SB_G2P3_FE_CLS_G2_SCHEMA.
 */

typedef enum {
  SB_G2P3_FE_CLS_G2P3_SCHEMA,
  SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA,
  SB_G2P3_FE_CLS_MAX_NUM_SCHEMA_C1,  
  SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA=2,
  SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA,
  SB_G2P3_FE_CLS_MAX_NUM_SCHEMA  
} soc_sbx_g2p3_cls_schema_enum_t;

/* Program Attribute Structure */
typedef struct soc_sbx_g2p3_cls_program_attribute_s {
    uint8_t     uNumber;        /* physical program number                    */
    uint8_t     uActive;        /* if 1 the program is active or its a backup */
    uint32_t    uBaseAddress;   /* Base Address of this program               */
    uint32_t    uLength;        /* Length of this program                     */
    soc_sbx_g2p3_cls_schema_enum_t sType; /* Schema type ingress or egress    */    
    uint8_t     uValid;         /* if 1 the program is valid                  */
} soc_sbx_g2p3_cls_prog_attr_t;

/* Schema Layout Configuration Structure */
typedef struct soc_sbx_g2p3_schema_layout_s {
    uint8_t     uNumRs;   /* number of rules sets this schema must support */
    uint32_t    uNumRulesPerRs; /* number of rules per rule set */
    uint8_t     uNumRcInstance; /* number of RCE instance this schema must use   */
    uint8_t     uBaseRcInstance;/* Base rc instance to start with. ZERO Based    */
    uint32_t    uBankedMode;
} soc_sbx_g2p3_schema_layout_t;

/* forward declarations */
typedef  soc_sbx_g2p3_cls_ace_pattern_t         *soc_sbx_g2p3_cls_ace_pattern_p_t;
typedef  soc_sbx_g2p3_cls_ace_pattern_ipv6_t         *soc_sbx_g2p3_cls_ace_pattern_ipv6_p_t;
typedef struct soc_sbx_g2p3_cls_schema_s        *soc_sbx_g2p3_cls_schema_p_t;
typedef struct soc_sbx_g2p3_cls_comp_s          *soc_sbx_g2p3_cls_comp_p_t;
typedef struct soc_sbx_g2p3_cls_pattern_memory_s *soc_sbx_g2p3_cls_pattern_memory_p_t;

/* cls encoders */
typedef void (*soc_sbx_g2p3_cls_encoder_p_t) (soc_sbx_g2p3_cls_schema_p_t         pSchemaHandler, 
                                              uint32_t                            uRuleId,
                                              void *    pRule, 
                                              soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory);
typedef void (*soc_sbx_g2p3_cls_decoder_p_t) (soc_sbx_g2p3_cls_schema_p_t         pSchemaHandler, 
                                              uint32_t                            uRuleId,
                                              void *    pRule, 
                                              sbZfFe2000RcSbPattern_t *pPatternMemory[],
                                              uint32_t *isNull);

typedef uint8_t soc_sbx_g2p3_bit_vector_t;
/**
 * @struct for the cls schema 
 */
typedef struct soc_sbx_g2p3_cls_schema_s
{
  sbFe2000ClsTemplate_t        schema;
  uint32_t                    *pInstructions;
  uint32_t                     uInstCount;
  uint32_t                     uInstNoPadCount;
  uint32_t                     uOffset;
  soc_sbx_g2p3_cls_encoder_p_t pEncoder;
  soc_sbx_g2p3_cls_decoder_p_t pDecoder;
  soc_sbx_g2p3_schema_layout_t layout;
  soc_sbx_g2p3_cls_schema_enum_t sType;  
} soc_sbx_g2p3_cls_schema_t;

/*
 * struct for passing group information during commit
 */
typedef struct soc_sbx_g2p3_cls_group_s
{
  int16_t                    uGroupId;
  soc_sbx_g2p3_cls_schema_t *pSchemaDesc;
  uint8_t                    uPriority;
  uint32_t                   uRuleCount;
  adq_t                     *pRuleList;
  uint32_t                   uNumRs;
  int8_t                     iRc;
} soc_sbx_g2p3_cls_group_t;

/*
 * struct for maintaining memory and ruleset usage
 */
typedef struct soc_sbx_g2p3_cls_capacity_mgr_s
{
  uint32_t uMemTotal; /* Total memory per Rc */
  uint32_t uRsMax;   /* Maximum rulesets per Rc */
  uint32_t uMemUsed; /* Committed memory Usage */
  uint32_t uRsUsed;  /* Committed Rs Usage */
  uint32_t uMemTobeUsed; /* Extra Memory needed for next commit */
  uint32_t uRsTobeUsed;  /* Extra Rs needed for next commit */
} soc_sbx_g2p3_cls_capacity_mgr_t;

/**
 * @struct for the cls Pattern Memory  
 */
typedef struct soc_sbx_g2p3_cls_pattern_memory_s
{
  uint32_t                  uCount;
  sbZfFe2000RcSbPattern_t  *pSbPattern[G2P3_MAX_CLS_RULESETS_PER_SCHEMA]\
                                      [G2P3_MAX_RC_INSTANCE]\
                                      [SB_FE2000XT_RC_NUM_SUPERBLOCKS];
  uint32_t                  uSaveResult[G2P3_MAX_CLS_RULESETS_PER_SCHEMA]\
                                       [G2P3_MAX_RC_INSTANCE];
  soc_sbx_g2p3_bit_vector_t *pVector;
} soc_sbx_g2p3_cls_pattern_memory_t;

/**
 * @enum enumeration for the DMA states
 */
typedef enum
{
  SB_G2P3_FE_DMA_STATE_IDLE=0,
  SB_G2P3_FE_DMA_STATE_BUSY
} soc_sbx_g2p3_cls_dma_state_t;

/** 
 * @struct for the DMA bank transfers
 */
typedef struct soc_sbx_g2p3_cls_bank_transfer_s
{
  uint32_t            uRcDb;
  uint32_t            uSize;
  void                *pDmaMemory;
  soc_sbx_g2p3_cls_dma_state_t state;
  sbFe2000DmaOp_t     dmaop;
} soc_sbx_g2p3_cls_bank_transfer_t;


typedef struct soc_sbx_g2p3_cls_ruleset_s soc_sbx_g2p3_cls_ruleset_t;

/**
 * @struct core classifier compiler engine state
 */
typedef struct soc_sbx_g2p3_cls_comp_s
{

  /* data base  and dma management */
  sbFe2000DmaMgr_t                *pDmaCtxt;
  uint32_t                         uMaxSlabSize;
  soc_sbx_g2p3_cls_bank_transfer_t bankASlabs[G2P3_MAX_RC_INSTANCE]; 
  soc_sbx_g2p3_cls_bank_transfer_t bankBSlabs[G2P3_MAX_RC_INSTANCE]; 
  soc_sbx_g2p3_cls_schema_t        schemadb[SB_G2P3_FE_CLS_MAX_NUM_SCHEMA];
  soc_sbx_g2p3_cls_capacity_mgr_t
                                usagePerSchema[SB_G2P3_FE_CLS_MAX_NUM_SCHEMA]\
                                                 [G2P3_MAX_RC_INSTANCE];
  soc_sbx_g2p3_cls_capacity_mgr_t  usagePerRc[G2P3_MAX_RC_INSTANCE];
  uint32_t                         uMaxInstCount;
  uint8_t                          uMaxSchema;
  soc_sbx_g2p3_cls_ruleset_t       *sRsDb;
  uint8_t                          uMaxRs;
  
  /* Array of Program Attributes */
  soc_sbx_g2p3_cls_prog_attr_t     *pProgAtrib[G2P3_MAX_CLS_PROGRAMS];
  
  /* index by schema returns current standby program number */
  uint8_t                          sbyProgNumber[SB_G2P3_FE_CLS_MAX_NUM_SCHEMA];
        
  /* pure management */
  sbMalloc_f_t              sbmalloc;
  sbFree_f_t                sbfree;
  void                      *clientData;
  uint32_t                  bDebug;
  uint32_t                  isC2; /* Caladan 2 runtime differentiation */
  void                      *pTmpCtrMem; /* Used in fetching counters in block */
  void                      *wbData;
  /* Used to manage RC instruction memory */
  shr_mem_avl_t *           mem_avl_ptr[G2P3_MAX_RC_INSTANCE][2]; 
  uint32_t                  uSwapRs[G2P3_MAX_RC_INSTANCE];
  uint8_t                   isSwapRsUsed[G2P3_MAX_RC_INSTANCE];
} soc_sbx_g2p3_cls_t;

/*
 * This is called during the initialization procedure
 * by the state machine before the ACE initialization.
 * @param pFe         Pointer to the soc_sbx_g2p3_state_t struct
 * @param pDmaCtxt    Dma Context 
 * @param cParams     FE common configuartion parameters.
 * @param iParams     FE ingress configuartion parameters.
 */
sbStatus_t soc_sbx_g2p3_cls_init(soc_sbx_g2p3_state_t *pFe,
                             sbFe2000DmaMgr_t  *pDmaCtxt,
                             sbCommonConfigParams_p_t    cParams,
                             sbIngressConfigParams_p_t   iParams,
                             void *pWbData);

sbStatus_t soc_sbx_g2p3_cls_init_c2(soc_sbx_g2p3_state_t *pFe,
                             sbFe2000DmaMgr_t  *pDmaCtxt,
                             sbCommonConfigParams_p_t    cParams,
                             sbIngressConfigParams_p_t   iParams,
                             void* pWbData);
/* 
 * This function frees up all the memory allocated during the
 * initialization process
 * @param pFe         Pointer to the soc_sbx_g2p3_state_t struct
 */
sbStatus_t soc_sbx_g2p3_cls_un_init(soc_sbx_g2p3_state_t *pFe);

/*
 * This function provides a way to get the schema description 
 * structure from the enumeration
 * @param pFe         Pointer to the soc_sbx_g2p3_state_t struct
 * @param eName       Name of the Schema format
 */
soc_sbx_g2p3_cls_schema_t *soc_sbx_g2p3_cls_schema_handler_get(soc_sbx_g2p3_state_t *pFe,
                                             soc_sbx_g2p3_cls_schema_enum_t eName);
/*
 * This function provides a way to create the
 * PatternMemory for a given schema.
 * @param pFe         Pointer to the soc_sbx_g2p3_state_t struct
 * @param pSchema     Pointer to the Schema Description
 * @param ppPatternMemory Pointer to Pointer for the Pattern Memory
 */

sbStatus_t soc_sbx_g2p3_cls_alloc_pattern_memory( soc_sbx_g2p3_state_t *pFe,
                                        soc_sbx_g2p3_cls_schema_t *pSchema,
                                        soc_sbx_g2p3_cls_pattern_memory_t **ppPatterMemory);

/*
 * This function provides a way to free the
 * PatternMemory for a given schema.
 * @param pFe         Pointer to the soc_sbx_g2p3_state_t struct
 * @param pSchema     Pointer to the Schema Description
 * @param pPatternMemory  Pointer for the Pattern Memory
 */

sbStatus_t soc_sbx_g2p3_cls_free_pattern_memory(soc_sbx_g2p3_state_t *pFe, 
                                      soc_sbx_g2p3_cls_schema_t *pSchema, 
                                      soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory);
/*
 * This function provides a way to clear the
 * PatternMemory for a given schema.
 * @param pFe         Pointer to the soc_sbx_g2p3_state_t struct
 * @param pSchema     Pointer to the Schema Description
 * @param pPatternMemory  Pointer for the Pattern Memory
 */

sbStatus_t soc_sbx_g2p3_cls_clear_pattern_memory(soc_sbx_g2p3_state_t *pFe, 
                                      soc_sbx_g2p3_cls_schema_t *pSchema, 
                                      soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory);
/* 
 * This function provides a way to commit the cls patterns
 */
sbStatus_t soc_sbx_g2p3_cls_commit(soc_sbx_g2p3_state_t *pFe, 
                           uint8_t  uBankId,
                           soc_sbx_g2p3_cls_schema_t *pPtr,
                           soc_sbx_g2p3_cls_pattern_memory_p_t pPatternMemory,
                           uint8_t *pProgramNumber);
typedef sbStatus_t (*pfFinalizeCb)(soc_sbx_g2p3_state_t* pFe,
                              soc_sbx_g2p3_cls_rule_db_e_t eDb,
                              uint32_t uRsOffset);
typedef sbStatus_t (*pfRuleCb)(soc_sbx_g2p3_state_t* pFe,
                                   soc_sbx_g2p3_cls_schema_t *pSchema,
                                   soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                                   void **pCookie,
                                   uint32_t uRuleIndex,
                                   uint32_t uRsOffset);

sbStatus_t
soc_sbx_g2p3_cls_commit_c2(soc_sbx_g2p3_state_t *pFe,
                soc_sbx_g2p3_cls_schema_t *pSchema,
                soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                soc_sbx_g2p3_cls_rule_db_e_t eDb,
                adq_t *pRuleList,
                uint32_t uRuleCount,
                soc_sbx_g2p3_cls_group_t *pClsGroup,
                uint8_t uGroupCount,
                pfRuleCb pfRuleIter,
                pfFinalizeCb pfFinalize);

int 
soc_sbx_g2p3_cls_rule_capacity_get(uint32_t unit,
                               soc_sbx_g2p3_cls_schema_enum_t sType,
                                   uint32_t *pRuleCount);
int
soc_sbx_g2p3_cls_schema_to_program(soc_sbx_g2p3_state_t *pFe,
                                  soc_sbx_g2p3_cls_schema_enum_t sType,
                                  uint8_t *pProgNumber);

int 
soc_sbx_g2p3_cls_get_ctr_offset(soc_sbx_g2p3_state_t *pFe,
                                uint8_t uProgramNumber, 
                                uint32_t *pOffset); 
int
soc_sbx_g2p3_cls_get_stby_prog_number(soc_sbx_g2p3_state_t *pFe,
                                      soc_sbx_g2p3_cls_schema_t *pSchema,
                                      uint8_t *pProgramNumber);

int
soc_sbx_g2p3_cls_print_ruleset(soc_sbx_g2p3_state_t *pFe);


int
soc_sbx_g2p3_cls_is_rule_capacity_available(uint32_t unit,
                               soc_sbx_g2p3_cls_schema_t *pSchema,
                               uint32_t uRuleCount,
                               uint8_t *bAvailable);

int
soc_sbx_g2p3_cls_rule_usable_capacity_get(uint32_t unit,
                               soc_sbx_g2p3_cls_schema_enum_t sType,
                               uint32_t *pRuleCount);

int
soc_sbx_g2p3_cls_template_fields_enable(
                               sbFe2000ClsTemplate_t *pTemplate,
                               soc_sbx_g2p3_ace_qset_t qset, uint8_t bEnableAll);

sbStatus_t
soc_sbx_g2p3_cls_wb_init(soc_sbx_g2p3_state_t *pFe, void **wbOut);
sbStatus_t
soc_sbx_g2p3_cls_wb_mem_recover(soc_sbx_g2p3_state_t *pFe);
sbStatus_t 
soc_sbx_g2p3_cls_wb_recover(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_cls_schema_t *pSchema);
sbStatus_t
soc_sbx_g2p3_cls_wb_uninit(soc_sbx_g2p3_state_t *pFe);
sbStatus_t
soc_sbx_g2p3_cls_wb_group_recover(soc_sbx_g2p3_state_t *pFe, 
                                  soc_sbx_g2p3_cls_schema_t *pSchema, 
                                  soc_sbx_g2p3_cls_rule_db_e_t eDb, 
                                  uint32_t uGroupId, 
                                  uint32_t uRuleCount);


#endif
