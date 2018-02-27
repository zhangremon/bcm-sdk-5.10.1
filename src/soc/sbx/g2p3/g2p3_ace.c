/* $Id: g2p3_ace.c 1.126.42.3 Broadcom SDK $
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
 * soc_sbx_g2p3_cls_ace.c : Classifier Access Control Entry interface
 *
 * --------------------------------------------------------------------------*/
#include <soc/sbx/g2p3/g2p3_int.h>
#include <soc/sbx/g2p3/g2p3_ace.h>
#include <soc/sbx/g2p3/g2p3_ace_internal.h>
#include <bcm_int/sbx/error.h>

extern int soc_sbx_g2p3_cls_scache_store(soc_sbx_g2p3_state_t *pFe);

#define SOC_SBX_G2P3_ACE_GROUPID_INVALID (SB_MAX_GROUPS + 1)
/*
 * Layers on RCE Programming [ BCM Layer --> Soc Layer ]
 *                                              |
 *                             --------------------------------
 *                             |                              |
 *                   Rule Indices Interface        Rule Handle Interface
 *                             |                              |
 *                             --------------------------------
 *                                              |
 *                                           Ace Layer
 * The Soc layer and Ace layer are implemented on this file.
 * SOC layer provides two interface for programming. One interface uses
 * Rule Indices where the SOC layer maintains and manipulates Rule Indices.
 * This interface is primarily focused to be used for Unit testing.
 * The Second interfaces uses Rule Handles. BCM layer uses the Handle interface.
 */

/*----------------------------------------------------------------------------
 * Forward declare module-local types
 *----------------------------------------------------------------------------*/

typedef enum {
  SB_G2P3_FE_CLS_BANK_A = 0,
  SB_G2P3_FE_CLS_BANK_B = 1,
} soc_sbx_g2p3_ace_bank_t;


struct soc_sbx_g2p3_ace_mgr_rule_s {
  adq_t               listElem;
  soc_sbx_g2p3_ace_rule_t Rule;
  soc_sbx_g2p3_cls_rule_db_e_t eDb;
  int16_t            uGroupId;
  uint32_t           ulMemChunkId;
  uint8_t            bEnable;
  uint8_t            bcommitted;
  uint32_t           uRuleIdx;
  uint32_t           uRuleOffset;
  uint32_t           uOldRuleIdx;
  uint32_t           uOldRuleOffset;
  soc_sbx_g2p3_count_t    counts;
};

typedef struct soc_sbx_g2p3_ace_group_s {
    adq_t                  listElem;
    soc_sbx_g2p3_cls_schema_t schemaDesc;
    int32_t               uPriority;
    uint8_t               uFirstRc;
    soc_sbx_g2p3_cls_rule_db_e_t eDbType;
    int16_t               uGroupId;
    uint32_t              uRuleCount;   /* counts only enabled rules */
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_ace_qset_t qset;
    uint8_t                 bAllQualifiers;
#endif

} soc_sbx_g2p3_ace_group_t;

typedef struct soc_sbx_g2p3_ace_db_s {
    adq_t                               listElem;
    soc_sbx_g2p3_ace_mgr_rule_t        *listRules;
    soc_sbx_g2p3_ace_group_t          *listGroups;
    soc_sbx_g2p3_cls_schema_t         *pSchemaDesc;
    uint8_t                            bEnable;
    soc_sbx_g2p3_cls_rule_db_e_t       eDbType;
    soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory;
    uint8_t                            bBank;
    int                                ctrsPerBank;
    uint8_t                            uProgramNumber;
} soc_sbx_g2p3_ace_db_t;

#define MAX_MEM_CHUNKS 100

struct soc_sbx_g2p3_ace_mgr_s {
  soc_sbx_g2p3_ace_db_t      *listRuleDbs;
  soc_sbx_g2p3_ace_mgr_rule_t  *listFreeRules;
  soc_sbx_g2p3_ace_group_t     *pGroups;
  void           *aMemChunks[MAX_MEM_CHUNKS];
  uint32_t        ulChunkCount;
  uint32_t        ulRulesPerChunk;
  uint8_t         bCommitting;
  soc_sbx_g2p3_cls_group_t  *pTmpMem;
};

#ifdef BCM_WARM_BOOT_SUPPORT
typedef struct soc_sbx_g2p3_ace_wb_group_s{
    soc_sbx_g2p3_ace_qset_t     qset;
    uint8_t                     bAllQualifiers;
    uint8_t                     uFirstRc;
    int16_t                     uGroupId;
    int32_t                     uPriority;
    soc_sbx_g2p3_cls_rule_db_e_t eDbType;
    uint32_t                    uRuleCount;
}soc_sbx_g2p3_ace_wb_group_t;

/* Warmboot support */
typedef struct soc_sbx_g2p3_ace_wb_layout_s{
    soc_sbx_g2p3_ace_wb_group_t group[SB_MAX_GROUPS];
}soc_sbx_g2p3_ace_wb_layout_t;

typedef struct soc_sbx_g2p3_ace_cache_s {
    uint32_t                wb_hdl;
    uint32_t                scache_size;
} soc_sbx_g2p3_ace_cache_t;

soc_sbx_g2p3_ace_cache_t _ace_cache[BCM_MAX_NUM_UNITS];

#define G2P3_ACE_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define G2P3_ACE_WB_CURRENT_VERSION            G2P3_ACE_WB_VERSION_1_0

/* Is Level 2 warmboot configured - */
#define L2CACHE_WB_L2_CONFIGURED(unit) (_ace_cache[unit].scache_size != 0)

#endif
/*----------------------------------------------------------------------------
 * Forward declare module-local functions
 *----------------------------------------------------------------------------*/
static int
soc_sbx_g2p3_fpctr_get_ext(int unit, soc_sbx_g2p3_cls_rule_db_e_t db,
                           int clear, int cnum,
                           soc_sbx_g2p3_counter_value_t *v);

static sbStatus_t
soc_sbx_g2p3_ace_rule_add(soc_sbx_g2p3_state_t *pFe,
                    soc_sbx_g2p3_cls_rule_db_e_t eDb,
                    soc_sbx_g2p3_ace_rule_t *pRule,
                    soc_sbx_g2p3_ace_mgr_rule_t **ppRuleHandle,
                    soc_sbx_g2p3_ace_mgr_rule_t *pRefRule,
                    uint8_t bBefore,
                    uint8_t bAddHead,
                    int16_t uGroupId);

static sbStatus_t
soc_sbx_g2p3_ace_rule_validate(soc_sbx_g2p3_state_t *pFe,
                         soc_sbx_g2p3_cls_rule_db_e_t eDb,
                         soc_sbx_g2p3_ace_rule_t *pRule );

static sbStatus_t
soc_sbx_g2p3_ace_bind_schema_to_rule_db(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_db_t *pDb,
                   soc_sbx_g2p3_cls_schema_enum_t eType);

static sbStatus_t
soc_sbx_g2p3_ace_alloc_rset_pattern_memory(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_db_t *pDb);
static sbStatus_t
soc_sbx_g2p3_ace_free_rset_pattern_memory(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_db_t *pDb);

static sbStatus_t
soc_sbx_g2p3_ace_rt_qos_action_add(soc_sbx_g2p3_state_t *pFe , uint8_t bBank, uint32_t uIndex,
               soc_sbx_g2p3_irt_wrap_t *pAction);

static sbStatus_t
soc_sbx_g2p3_ace_rt_egr_action_add(soc_sbx_g2p3_state_t *pFe , uint8_t bBank, uint32_t uIndex,
                         soc_sbx_g2p3_ert_wrap_t *pAction);

static sbStatus_t
soc_sbx_g2p3_ace_alloc_mem_chunk(soc_sbx_g2p3_ace_mgr_t *pClsAce);

static sbStatus_t
soc_sbx_g2p3_ace_alloc_rule(soc_sbx_g2p3_ace_mgr_t *pClsAce, soc_sbx_g2p3_ace_mgr_rule_t **pRule);

static sbStatus_t
soc_sbx_g2p3_ace_free_rule(soc_sbx_g2p3_ace_mgr_t *pClsAce, soc_sbx_g2p3_ace_mgr_rule_t *pRule);

static
int soc_sbx_g2p3_ifp_add_sentinel(int unit, int irule, soc_sbx_g2p3_ifp_t *e);
static
int soc_sbx_g2p3_ifp_v6_add_sentinel(int unit, int irule, soc_sbx_g2p3_ifp_v6_t *e);
static
int soc_sbx_g2p3_efp_add_sentinel(int unit, int erule, soc_sbx_g2p3_efp_t *e);
static
int soc_sbx_g2p3_efp_v6_add_sentinel(int unit, int erule, soc_sbx_g2p3_efp_v6_t *e);

int soc_sbx_g2p3_ace_is_rule_enabled(soc_sbx_g2p3_ace_rule_t *pRule);

void *
soc_sbx_g2p3_ace_rule_pattern_get(soc_sbx_g2p3_ace_rule_t *pRule);

#ifdef BCM_WARM_BOOT_SUPPORT
sbStatus_t
soc_sbx_g2p3_ace_wb_rule_recover(soc_sbx_g2p3_state_t *pFe,
                                       soc_sbx_g2p3_cls_schema_t *pSchema,
                                       uint32_t uGroupId,
                                       uint32_t uRule,
                                       soc_sbx_g2p3_cls_ace_pattern_t *pPattern);
static int
soc_sbx_g2p3_ace_wb_layout_get(int unit, soc_sbx_g2p3_ace_wb_layout_t **layout){
    int rv;
    uint32 size;
    soc_wb_cache_t *wbc;

    *layout = NULL;
    rv = soc_scache_ptr_get(unit, _ace_cache[unit].wb_hdl, 
                            (uint8**)&wbc, &size);
    if (rv != SOC_E_NONE) {
        return rv; 
    }
    *layout = (soc_sbx_g2p3_ace_wb_layout_t *)wbc->cache;
    return rv;
}

#endif
int
soc_sbx_g2p3_ace_scache_store(soc_sbx_g2p3_state_t *pFe){
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_ace_wb_layout_t *layout;
    soc_sbx_g2p3_ace_db_t *pDb=NULL;
    soc_sbx_g2p3_ace_wb_group_t *group;
    soc_sbx_g2p3_ace_group_t *pGroup;
    sbStatus_t err;
    uint32_t i, unit;
    soc_sbx_g2p3_ace_mgr_t *pClsAce;

    unit = pFe->unit;
    pClsAce = pFe->pClsAce;

    err = soc_sbx_g2p3_ace_wb_layout_get(unit, &layout);
    if (err != SB_OK) {
        return err;
    }

    soc_scache_handle_lock(unit, _ace_cache[unit].wb_hdl);

    group = layout->group;

    i=0;
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
        if (pDb->listGroups != NULL) {
            ACEDQ_TRAVERSE(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t) {
                group[i].qset = pGroup->qset;
                group[i].bAllQualifiers = pGroup->bAllQualifiers;
                group[i].uFirstRc = pGroup->uFirstRc;
                group[i].uGroupId  = pGroup->uGroupId;
                group[i].uPriority = pGroup->uPriority;
                group[i].eDbType = pGroup->eDbType;
                group[i].uRuleCount = pGroup->uRuleCount;
                i++;
            } ACEDQ_TRAVERSE_END(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t);
        }
        SB_ASSERT(i <= SB_MAX_GROUPS);
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* mark remaing groups invalid */
    while (i<SB_MAX_GROUPS) {
        group[i].uGroupId = SOC_SBX_G2P3_ACE_GROUPID_INVALID;
        i++;
    }

    soc_scache_handle_unlock(unit, _ace_cache[unit].wb_hdl);

    return SB_OK;
#else
    return SB_OK;
#endif
}

int
soc_sbx_g2p3_ace_scache_recover(soc_sbx_g2p3_state_t *pFe){
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_sbx_g2p3_ace_wb_layout_t *layout;
    soc_sbx_g2p3_ace_wb_group_t *group;
    soc_sbx_g2p3_ace_group_t *pGroup;
    soc_sbx_g2p3_ace_db_t *pDb=NULL;
    sbStatus_t err;
    uint32_t i, unit;
    soc_sbx_g2p3_ace_mgr_t *pClsAce;
    uint32_t uRuleCount[SB_MAX_GROUPS];

    unit = pFe->unit;
    pClsAce = pFe->pClsAce;
    err = soc_sbx_g2p3_ace_wb_layout_get(unit, &layout);
    if (err != SB_OK) {
        return err;
    }

    soc_scache_handle_lock(unit, _ace_cache[unit].wb_hdl);

    group = layout->group;

    for (i=0; i<SB_MAX_GROUPS; i++) {
        if (group[i].uGroupId != SOC_SBX_G2P3_ACE_GROUPID_INVALID) {
            /* recreate groups */
            err = soc_sbx_g2p3_ace_group_create(unit,
                                                group[i].eDbType,
                                                group[i].uGroupId,
                                                group[i].uPriority,
                                                group[i].qset,
                                                group[i].bAllQualifiers);
            if (err !=  SB_OK) {
                SB_ERROR(_SBX_D(unit, "ACE group 0x%x recovery failed (0x%x)"),
                    i, err);
                return err;
            }
            /* save rulecount */
            uRuleCount[group[i].uGroupId] = group[i].uRuleCount;
        }
    }

    soc_scache_handle_unlock(unit, _ace_cache[unit].wb_hdl);

    /* recover CLS layer memory */
    soc_sbx_g2p3_cls_wb_mem_recover(pFe);

    /* walk groups and invoke recovery at CLS layer */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
        ACEDQ_TRAVERSE(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t) {
            if (pGroup->uGroupId >= IFP_DEF_GROUP_IDX) {
                err = soc_sbx_g2p3_ace_wb_rule_recover(pFe,
                                                       &pGroup->schemaDesc,
                                                       pGroup->uGroupId,
                                                       0,
                                                       NULL);
                if (err != SB_OK) {
                    SB_ERROR(_SBX_D(unit, "ACE rule recovery for group 0x%x recovery failed (0x%x)"),
                        pGroup->uGroupId, err);
                    return err;
                }
            }else{
                if (uRuleCount[pGroup->uGroupId]) {
                    err = soc_sbx_g2p3_cls_wb_group_recover(pFe, &pGroup->schemaDesc, pGroup->eDbType, pGroup->uGroupId, uRuleCount[pGroup->uGroupId]);
                    if (err != SB_OK) {
                        SB_ERROR(_SBX_D(unit, "ACE rule recovery for group 0x%x recovery failed (0x%x)"),
                            pGroup->uGroupId, err);
                        return err;
                    }
                }
            }
        } ACEDQ_TRAVERSE_END(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t);
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* cleanup */
    soc_sbx_g2p3_cls_wb_uninit(pFe);
#if 0
soc_sbx_g2p3_ace_mgr_rule_t *pRule       = NULL;
soc_cm_print("DUMPING all recovered rules in ORDER!\n");
ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
    ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
      soc_sbx_g2p3_ace_rule_handle_print(pFe, pRule);
    } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);
} ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);
#endif
    return SB_OK;
#else
    return SB_OK;
#endif
}

/*************************/
/********* ACE ***********/
/*************************/
/**
 * @fn soc_sbx_g2p3_ace_init()
 *
 * @brief
 *
 * This function sets up the classifier driver for use. It prepares
 * the structures that are used for handling sets, and allocates all
 * the required resources for use. It does NOT touch the hardware and
 * therefore issues no DMA transactions affecting the classifier
 * state. This function should always complete synchronously.
 *
 * @param fe    - driver control structure
 * @param pInit - initialization configuration structure
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_init(soc_sbx_g2p3_state_t *pFe,
                 sbCommonConfigParams_p_t cParams,
                 sbIngressConfigParams_p_t iParams)
{
  soc_sbx_g2p3_ace_mgr_t     *pClsAce;
  soc_sbx_g2p3_ace_db_t      *pDb = 0;
  soc_sbx_g2p3_ace_mgr_rule_t *pRule = 0;
  soc_sbx_g2p3_ifp_t ifp;
  soc_sbx_g2p3_efp_t efp;
  soc_sbx_g2p3_ifp_v6_t ifp_v6;
  soc_sbx_g2p3_efp_v6_t efp_v6;
  void              *vp;
  sbStatus_t         err = 0;
  uint32_t           db, i;
  int                rv;
  uint32_t           uNumRules = 0;
  uint32_t           uSchema, uSchemaRules = 0;
  soc_sbx_g2p3_counter_params_t cp;
  uint32_t isC2 = SAND_HAL_IS_FE2KXT(pFe->regSet);
  uint32_t uMaxDb;
  uint8_t v6Enable;
  soc_sbx_g2p3_ace_qset_t qset, qset_dummy;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 size, scache_size;
    soc_wb_cache_t *wbc;
#endif

  /* assumption: list elem struct at front of ref struct */
  SB_ASSERT((void*)pDb == (void*)&pDb->listElem);
  SB_ASSERT((void*)pRule == (void*)&pRule->listElem);

  vp = thin_host_malloc(sizeof(*pClsAce));
  if (!vp)
    return SB_MALLOC_FAILED;

  pFe->pClsAce = pClsAce = vp;
  if (isC2) {
#ifdef BCM_WARM_BOOT_SUPPORT
    /* on cold boot, setup scache */
    SOC_SCACHE_HANDLE_SET(_ace_cache[pFe->unit].wb_hdl, pFe->unit, SOC_SBX_WB_MODULE_COMMON, SOC_SBX_G2P3_WB_ACE);
    /* Is Level 2 recovery even available? */
    rv = soc_stable_size_get(pFe->unit, (int*)&size);
    if (rv != SOC_E_NONE) {
        return rv; 
    }
    
    /* Allocate a new chunk of the scache during a cold boot */
    if (!SOC_WARM_BOOT(pFe->unit) && (size > 0)) {
        scache_size = sizeof(soc_sbx_g2p3_ace_wb_group_t) * SB_MAX_GROUPS;
        scache_size += SOC_WB_SCACHE_CONTROL_SIZE;
        rv = soc_scache_alloc(pFe->unit, _ace_cache[pFe->unit].wb_hdl, scache_size);
        if ( (rv != SOC_E_NONE) && (rv != SOC_E_EXISTS) ){
            return rv;
        }
    }

    /* Get the pointer from scache */
    wbc = NULL;
    _ace_cache[pFe->unit].scache_size = 0;
    if (size > 0) {
        rv = soc_scache_ptr_get(pFe->unit, _ace_cache[pFe->unit].wb_hdl,
                                    (uint8**)&wbc,
                                    &_ace_cache[pFe->unit].scache_size);
        if (rv != SOC_E_NONE) {
            return rv; 
        }

    }

    if (wbc){
        if (!SOC_WARM_BOOT(pFe->unit)) {
            wbc->version = G2P3_ACE_WB_CURRENT_VERSION;
        }

        SB_LOGV1(_SBX_D(pFe->unit, "Obtained scache pointer=0x%08x, %d bytes, "
                        "version=%d.%d\n"),
                 (int)wbc->cache, _ace_cache[pFe->unit].scache_size,
                 SOC_SCACHE_VERSION_MAJOR(wbc->version),
                 SOC_SCACHE_VERSION_MINOR(wbc->version));

        if (wbc->version > G2P3_ACE_WB_CURRENT_VERSION) {
            SB_ERROR(_SBX_D(pFe->unit, "Upgrade scenario not supported.  "
                           "Current version=%d.%d  found %d.%d\n"),
                    SOC_SCACHE_VERSION_MAJOR(G2P3_ACE_WB_CURRENT_VERSION),
                    SOC_SCACHE_VERSION_MINOR(G2P3_ACE_WB_CURRENT_VERSION),
                    SOC_SCACHE_VERSION_MAJOR(wbc->version),
                    SOC_SCACHE_VERSION_MINOR(wbc->version));
            rv = SOC_E_CONFIG;

        } else if (wbc->version < G2P3_ACE_WB_CURRENT_VERSION) {
            SB_ERROR(_SBX_D(pFe->unit, "Downgrade scenario not supported.  "
                           "Current version=%d.%d  found %d.%d\n"),
                    SOC_SCACHE_VERSION_MAJOR(G2P3_ACE_WB_CURRENT_VERSION),
                    SOC_SCACHE_VERSION_MINOR(G2P3_ACE_WB_CURRENT_VERSION),
                    SOC_SCACHE_VERSION_MAJOR(wbc->version),
                    SOC_SCACHE_VERSION_MINOR(wbc->version));
            rv = SOC_E_CONFIG;
        }

        if (rv != BCM_E_NONE) {
            return rv;
        }

    }
#endif

    pClsAce->pGroups = thin_host_malloc(sizeof(soc_sbx_g2p3_ace_group_t) * SB_MAX_GROUPS);
    if (pClsAce->pGroups == NULL) {
      return SB_MALLOC_FAILED;
    }
    for (i=0; i<SB_MAX_GROUPS; i++){
        pClsAce->pGroups[i].uGroupId = SOC_SBX_G2P3_ACE_GROUPID_INVALID;
    }

    pClsAce->pTmpMem = thin_host_malloc(sizeof(soc_sbx_g2p3_cls_group_t) * SB_MAX_GROUPS);
    if (pClsAce->pTmpMem == NULL) {
      return SB_MALLOC_FAILED;
    }

  } else {
    pClsAce->pGroups = NULL;
    pClsAce->pTmpMem = NULL;
  }

  /* calculate mamximum number of rules */
  for (uSchema = 0; uSchema < pFe->pClsComp->uMaxSchema; uSchema++) {
    err = soc_sbx_g2p3_cls_rule_capacity_get(pFe->unit, uSchema, &uSchemaRules);
    if (err != SB_OK) {
        return err;
    }
    uNumRules += uSchemaRules;
  }

/* Ingress Rules and IPv6 Ingress rules share the same space and similary
 * egress rules
 */
  if(pFe->pClsComp->uMaxSchema > SB_G2P3_FE_CLS_MAX_NUM_SCHEMA_C1) {
      uNumRules = uNumRules / 2;
  }

  SB_MEMSET(pClsAce->aMemChunks, 0, sizeof(pClsAce->aMemChunks));
  pClsAce->ulChunkCount = 0;
  if ( SAND_HAL_IS_FE2KXT(pFe->regSet) ) {
    pClsAce->ulRulesPerChunk = (uNumRules / MAX_MEM_CHUNKS) + 1;
  }else{
    pClsAce->ulRulesPerChunk = 100;
  }
/*FIX_ME: Check the uNumRules. */
#ifdef RULE_ADD_DBG
  soc_cm_print("MAX_RULES = 0x%x\n", uNumRules);
  soc_cm_print("RULE PER CHUNK: 0x%x, CHUNKS: 0x%x\n", pClsAce->ulRulesPerChunk, MAX_MEM_CHUNKS);
#endif

  
  pClsAce->listFreeRules = NULL;
  pClsAce->listRuleDbs = NULL;

  v6Enable = soc_property_get(pFe->unit, spn_IPV6_ENABLE, 0);

  if(isC2 && v6Enable) {
    uMaxDb = SB_G2P3_CLS_MAX;
  } else {
    uMaxDb = SB_G2P3_CLS_MAX_C1;
  }

  /* allocate rule databases */
  for (db = 0; db < uMaxDb; db++) {
    /* allocate database */
    pDb= (soc_sbx_g2p3_ace_db_t*)thin_host_malloc(sizeof(soc_sbx_g2p3_ace_db_t));
    if (!pDb)
      return SB_MALLOC_FAILED;

    ACEDQ_INIT(pDb);

    /* init database */
    pDb->bEnable = 1;
    pDb->pSchemaDesc = NULL;
    pDb->pPatternMemory = NULL;
    pDb->eDbType = (soc_sbx_g2p3_cls_rule_db_e_t) db;
    pDb->bBank   = SB_G2P3_FE_CLS_BANK_A;
    pDb->listRules = NULL;
    pDb->listGroups = NULL;

    /* bind schema to database */
    switch(db) {
    case SB_G2P3_CLS_QOS:
      {
          if (isC2) {
              rv = soc_sbx_g2p3_rtctr_counter_params_get(pFe->unit, &cp);
          } else {
              rv = soc_sbx_g2p3_irtctr_counter_params_get(pFe->unit, &cp);
          }
          if (rv != SOC_E_NONE) {
              return SB_OTHER_ERR_CODE;
          }

          pDb->ctrsPerBank = cp.size;

          pDb->uProgramNumber = SB_G2P3_FE_CLS_G2P3_SCHEMA_PROGRAM_NUMBER;

          err = soc_sbx_g2p3_ace_bind_schema_to_rule_db(pFe, pDb, \
                                                        SB_G2P3_FE_CLS_G2P3_SCHEMA);
          if (err != SB_OK)
            return err;

          /* allocate RSET pattern memory */
          err = soc_sbx_g2p3_ace_alloc_rset_pattern_memory(pFe, pDb);
          if (err != SB_OK)
            return err;
      }
      break;
    case SB_G2P3_CLS_EGRESS:
      {
          if(isC2) {
              rv = soc_sbx_g2p3_rtctr_counter_params_get(pFe->unit, &cp);
          } else {
              rv = soc_sbx_g2p3_ertctr_counter_params_get(pFe->unit, &cp);
          }
          if (rv != SOC_E_NONE) {
              return SB_OTHER_ERR_CODE;
          }

          pDb->uProgramNumber = SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA_PROGRAM_NUMBER;

          err = soc_sbx_g2p3_ace_bind_schema_to_rule_db(pFe, pDb, \
                                                        SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA);
          if (err != SB_OK)
            return err;

          /* allocate RSET pattern memory */
          err = soc_sbx_g2p3_ace_alloc_rset_pattern_memory(pFe, pDb);
          if (err != SB_OK)
            return err;
      }
      break;
   case SB_G2P3_CLS_IPV6_QOS:
      {
          rv = soc_sbx_g2p3_rtctr_counter_params_get(pFe->unit, &cp);
          if (rv != SOC_E_NONE) {
              return SB_OTHER_ERR_CODE;
          }

          pDb->ctrsPerBank = cp.size;

          pDb->uProgramNumber = SB_G2P3_FE_CLS_G2P3_SCHEMA_PROGRAM_NUMBER;

          err = soc_sbx_g2p3_ace_bind_schema_to_rule_db(pFe, pDb, \
                                                        SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA);
          if (err != SB_OK)
            return err;

          /* allocate RSET pattern memory */
          err = soc_sbx_g2p3_ace_alloc_rset_pattern_memory(pFe, pDb);
          if (err != SB_OK)
            return err;
      }
      break;

   case SB_G2P3_CLS_IPV6_EGRESS:
      {
          rv = soc_sbx_g2p3_rtctr_counter_params_get(pFe->unit, &cp);
          if (rv != SOC_E_NONE) {
              return SB_OTHER_ERR_CODE;
          }

          pDb->ctrsPerBank = cp.size;

          pDb->uProgramNumber = SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA_PROGRAM_NUMBER;

          err = soc_sbx_g2p3_ace_bind_schema_to_rule_db(pFe, pDb, \
                                                        SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA);
          if (err != SB_OK)
            return err;

          /* allocate RSET pattern memory */
          err = soc_sbx_g2p3_ace_alloc_rset_pattern_memory(pFe, pDb);
          if (err != SB_OK)
            return err;
      }
      break;

    default:
      break;
    }

    /* add database to list of databases */
    ACEDQ_INSERT_TAIL(&pClsAce->listRuleDbs, pDb);
  }

  /* allocate memory chunk to the free list */
  soc_sbx_g2p3_ace_alloc_mem_chunk(pClsAce);

  /* if warmboot, perform recovery */
  if (SOC_WARM_BOOT(pFe->unit) && (isC2)) {
      /* recover groups */
      err = soc_sbx_g2p3_ace_scache_recover(pFe);
      if (err != SB_OK) {
          return err;
      }
  }else if (isC2) {
      sal_memset(&qset, 0, sizeof(soc_sbx_g2p3_ace_qset_t));
      SBX_FIELD_QSET_ADD(qset, sbxFieldQualifyDbType); 
      SBX_FIELD_QSET_ADD(qset, sbxFieldQualifyPorts); 

      /* Create Groups to be used when rules added via soc apis */
      /* Note:
       * The order/position of rules of different group is managed at
       * BCM layer. The groups XXX_SOC_GROUP_IDX are strategically created
       * just before XXX_DEF_GROUP_IDX which has only the sentinel rule.
       * So the rules of XXX_SOC_GROUP are created before the sentinel
       * by default
       */
      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                    SB_G2P3_CLS_QOS,
                                    IFP_SOC_GROUP_IDX,
                                    IFP_SOC_GROUP_PRI,
                                    qset_dummy,
                                    TRUE);

      /* Create Group to add sentinel rule */
      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                  SB_G2P3_CLS_QOS,
                                  IFP_DEF_GROUP_IDX,
                                  IFP_DEF_GROUP_PRI,
                                  qset,
                                  FALSE);

      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                    SB_G2P3_CLS_EGRESS,
                                    EFP_SOC_GROUP_IDX,
                                    EFP_SOC_GROUP_PRI,
                                    qset_dummy,
                                    TRUE);

      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                    SB_G2P3_CLS_EGRESS,
                                    EFP_DEF_GROUP_IDX,
                                    EFP_DEF_GROUP_PRI,
                                    qset,
                                    FALSE);
  }else if SOC_WARM_BOOT(pFe->unit) {
      /* recover each db for CA */
      ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
          soc_sbx_g2p3_cls_wb_recover(pFe, pDb->pSchemaDesc);
      } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);
      soc_sbx_g2p3_cls_wb_uninit(pFe);
  }

  if (!SOC_WARM_BOOT(pFe->unit)) {

      /* Add the sentinel rule & commit */
      soc_sbx_g2p3_ifp_t_init(&ifp);
      rv = soc_sbx_g2p3_ifp_add_sentinel (pFe->unit, 0, &ifp);
      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }
      soc_sbx_g2p3_efp_t_init(&efp);
      rv = soc_sbx_g2p3_efp_add_sentinel(pFe->unit, 0, &efp);
      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }

      rv = soc_sbx_g2p3_ifp_handle_commit(pFe->unit, 0xffffffff);
      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }

      rv = soc_sbx_g2p3_efp_handle_commit(pFe->unit, 0xffffffff);
      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }
  }

  if ( (!SOC_WARM_BOOT(pFe->unit)) && (uMaxDb > 2) && (isC2 == 1)) {

      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                    SB_G2P3_CLS_IPV6_QOS,
                                    IFP_V6_SOC_GROUP_IDX,
                                    IFP_V6_SOC_GROUP_PRI,
                                    qset_dummy,
                                    TRUE);

      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                    SB_G2P3_CLS_IPV6_QOS,
                                    IFP_V6_DEF_GROUP_IDX,
                                    IFP_V6_DEF_GROUP_PRI,
                                    qset,
                                   FALSE);

      /* Add the sentinel rule & commit */
      soc_sbx_g2p3_ifp_v6_t_init(&ifp_v6);
      rv = soc_sbx_g2p3_ifp_v6_add_sentinel(pFe->unit, 0, 
                                          &ifp_v6);

      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }
      rv = soc_sbx_g2p3_ifp_v6_handle_commit(pFe->unit, 0xffffffff);
      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }
      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                    SB_G2P3_CLS_IPV6_EGRESS,
                                    EFP_V6_SOC_GROUP_IDX,
                                    EFP_V6_SOC_GROUP_PRI,
                                    qset_dummy,
                                    TRUE);

      soc_sbx_g2p3_ace_group_create(pFe->unit,
                                    SB_G2P3_CLS_IPV6_EGRESS,
                                    EFP_V6_DEF_GROUP_IDX,
                                    EFP_V6_DEF_GROUP_PRI,
                                    qset,
                                    FALSE);

     /* Add the sentinel rule & commit */
      soc_sbx_g2p3_efp_v6_t_init(&efp_v6);
      rv = soc_sbx_g2p3_efp_v6_add_sentinel(pFe->unit, 0, &efp_v6);

      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }
      rv = soc_sbx_g2p3_efp_v6_handle_commit(pFe->unit, 0xffffffff);
      if (rv != SOC_E_NONE) {
          return SB_OTHER_ERR_CODE;
      }
  }

#ifdef BCM_WARM_BOOT_SUPPORT
  if (isC2) {
      err = soc_sbx_g2p3_ace_scache_store(pFe);
      if (err != SB_OK){
          return err;
      }
  }
#endif
  return err;
}

/**
 * @fn soc_sbx_g2p3_ace_un_init()
 *
 * @brief
 *
 * This function breaks down the classifier driver, and returns all its
 * resources to the system. It does NOT affect the current state of the
 * classifier hardware. This allows the driver to be restarted on failure.
 * To nullify any existing rules in the classifier memory, a call should be
 * made to soc_sbx_g2p3_ace_clear_all().
 *
 * @param pFe - driver control structure
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_un_init(soc_sbx_g2p3_state_t *pFe)
{
  soc_sbx_g2p3_ace_mgr_t  *pClsAce;
  soc_sbx_g2p3_ace_db_t   *pDbElem = NULL;
  soc_sbx_g2p3_ace_db_t   *pDb;
  int i;
  sbStatus_t err = SB_OK;
  soc_sbx_g2p3_ace_group_t *pCurrGroup;

  SB_ASSERT(pFe);

  /* protect double-free */
  if (!pFe->pClsAce)
    return SB_OK;

  pClsAce = pFe->pClsAce;

  /* free allocated rule memory */
  for (i = pClsAce->ulChunkCount - 1; i >= 0; i--)
    thin_host_free(pClsAce->aMemChunks[i]);

  /* free list of databases, and lists of rulesets beneath */
  if (ACEDQ_EMPTY(&pClsAce->listRuleDbs) == FALSE)
    ACEDQ_REMOVE_HEAD(&pClsAce->listRuleDbs, &pDbElem, soc_sbx_g2p3_ace_db_t);

  while (pDbElem) {
    pDb = (soc_sbx_g2p3_ace_db_t *) pDbElem;
    
    err = soc_sbx_g2p3_ace_free_rset_pattern_memory(pFe, pDb);
    if (err != SB_OK){
        /* ignore error */
    }
    if (SAND_HAL_IS_FE2KXT(pFe->regSet)) {
        pCurrGroup = pDb->listGroups;
        ACEDQ_TRAVERSE(&pDb->listGroups, pCurrGroup, soc_sbx_g2p3_ace_group_t) {
            thin_host_free(pCurrGroup->schemaDesc.pInstructions);
            pCurrGroup->schemaDesc.pInstructions = NULL;
            thin_host_free(pCurrGroup->schemaDesc.schema.ops);
            pCurrGroup->schemaDesc.schema.ops = NULL;
        } ACEDQ_TRAVERSE_END(&pDb->listGroups, pCurrGroup, \
                                   soc_sbx_g2p3_ace_group_t);
    }
    thin_host_free(pDb);
    if (ACEDQ_EMPTY(&pClsAce->listRuleDbs) == FALSE)
      ACEDQ_REMOVE_HEAD(&pClsAce->listRuleDbs, &pDbElem, soc_sbx_g2p3_ace_db_t);
    else
      pDbElem = NULL;

  }

  if (SAND_HAL_IS_FE2KXT(pFe->regSet)){
#ifndef BCM_WARM_BOOT_SUPPORT
    /* free group memory */
    thin_host_free(pClsAce->pGroups);
    pClsAce->pGroups = 0;
#endif
    thin_host_free(pClsAce->pTmpMem);
    pClsAce->pTmpMem = 0;
  }

  /* free control structure */
  pFe->pClsAce = 0;
  thin_host_free(pClsAce);
  return err;
}

sbStatus_t
soc_sbx_g2p3_ace_group_create(int unit, 
                              soc_sbx_g2p3_cls_rule_db_e_t eDb,
                              int16_t uGroupIndex,
                              int32_t uPriority,
                              soc_sbx_g2p3_ace_qset_t qset,
                              uint8_t bAllQualifiers)
{
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_mgr_t *pClsAce;
    soc_sbx_g2p3_ace_db_t *pDb = NULL;
    soc_sbx_g2p3_cls_schema_t *pSchema;
    soc_sbx_g2p3_ace_group_t  *pGroup = NULL, *pCurrGroup, *pPrevGroup = NULL;
    uint32_t isC2 = SAND_HAL_IS_FE2KXT(pFe->regSet);
    uint8_t bEndOfList;

    SB_ASSERT(pFe);
    SB_ASSERT(pFe->pClsAce);

    pClsAce = pFe->pClsAce;

    if (isC2 == 0) {
        return SB_OK;
    }

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
        if (eDb == pDb->eDbType) {
            break;
        }
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* database not found */
    if (!pDb) {
        return SB_BAD_ARGUMENT_ERR_CODE;
    }

    /* Get group from pClsAce */
    pGroup = &pClsAce->pGroups[uGroupIndex];
    /* Initialize group */
    pGroup->uGroupId = uGroupIndex;
    pGroup->eDbType = eDb;
    pGroup->uRuleCount = 0;

#ifdef BCM_WARM_BOOT_SUPPORT
    pGroup->qset = qset;
    pGroup->bAllQualifiers = bAllQualifiers;
#endif
    pSchema = &pGroup->schemaDesc;
    memcpy(pSchema, pDb->pSchemaDesc, sizeof(soc_sbx_g2p3_cls_schema_t));
    pSchema->pInstructions = NULL;

    /* Add group to database group list*/
    ACEDQ_INIT(pGroup);
    pGroup->uPriority = uPriority;
    if (pDb->listGroups == NULL) {
        pDb->listGroups = pGroup;
    } else {
        bEndOfList = 1;
        pCurrGroup = pDb->listGroups;
        ACEDQ_TRAVERSE(&pDb->listGroups, pCurrGroup, soc_sbx_g2p3_ace_group_t) {
            if( pCurrGroup->uPriority < uPriority) {
                bEndOfList = 0;
                break;
            }
            pPrevGroup = pCurrGroup;
        } ACEDQ_TRAVERSE_END(&pDb->listGroups, pCurrGroup, \
                                   soc_sbx_g2p3_ace_group_t);
      
        if (bEndOfList == 0) {
            ACEDQ_INSERT_PREV(&pDb->listGroups, pCurrGroup, pGroup);
        } else {
            ACEDQ_INSERT_NEXT(&pDb->listGroups, pPrevGroup, pGroup);
        }
    }

    /* alloc memory for template */
    pSchema->schema.ops = NULL;
    pSchema->schema.ops = (sbFe2000ClsTemplateField_t *)
                            thin_host_malloc(sizeof(sbFe2000ClsTemplateField_t)
                                  * pSchema->schema.uCount);

    if (pSchema->schema.ops == NULL) {
        ACEDQ_REMOVE(&pDb->listGroups, pGroup);
        return SB_MALLOC_FAILED;
    }
    /* copy database schema template to group schema template */
    memcpy(pSchema->schema.ops, pDb->pSchemaDesc->schema.ops,
                sizeof(sbFe2000ClsTemplateField_t) * pSchema->schema.uCount);

    /* enable the template fields depending on qualifiers in the qset */
    soc_sbx_g2p3_cls_template_fields_enable(&pSchema->schema,
                                             qset,
                                             bAllQualifiers);

    /* Generate the instructions */
    sbFe2000ClsGenerateInstructions_c2(&pSchema->schema,
                                        pSchema->sType,
                                       &pSchema->pInstructions,
                                       &pSchema->uInstCount,
                                       &pSchema->uInstNoPadCount,
                                       TRUE);
#ifdef RULE_ADD_DBG
    soc_cm_print("\n Group created : RuleSet Size %d\n", pSchema->uInstCount);
#endif

    return SB_OK;
}

sbStatus_t
soc_sbx_g2p3_ace_group_del(int unit,
                           soc_sbx_g2p3_cls_rule_db_e_t eDb,
                           int16_t uGroupIndex)
{
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_mgr_t *pClsAce = pFe->pClsAce;
    soc_sbx_g2p3_ace_db_t *pDb;
    soc_sbx_g2p3_ace_group_t *pGroup;
    soc_sbx_g2p3_cls_schema_t *pSchema;
    uint32_t isC2 = SAND_HAL_IS_FE2KXT(pFe->regSet);

    if (isC2 == 0) {
        return SB_OK;
    }

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
        if (eDb == pDb->eDbType) {
            break;
        }
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* database not found */
    if (!pDb) {
        return SB_BAD_ARGUMENT_ERR_CODE;
    }

    /* Get the group */
    pGroup = &pClsAce->pGroups[uGroupIndex];
    pSchema = &pGroup->schemaDesc;
    /* free instruction memory and template memory */
    if (pSchema->pInstructions) {
        thin_host_free(pSchema->pInstructions);
        pSchema->pInstructions = NULL;
    }
    if (pSchema->schema.ops) {
        thin_host_free(pSchema->schema.ops);
        pSchema->schema.ops = NULL;
    }
    pGroup->uGroupId = SOC_SBX_G2P3_ACE_GROUPID_INVALID;

    /* remove group from database group list */
    ACEDQ_REMOVE(&pDb->listGroups, pGroup);

    return SB_OK;
}

/*
 * UTG functions
 */

int
soc_sbx_g2p3_ace_nth_handle(int unit, soc_sbx_g2p3_cls_rule_db_e_t db,
                                int sentinelok, int irule,
                                soc_sbx_g2p3_ace_rule_handle_t *h)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t sentinel;
    int i;
    sbStatus_t s;
    
    *h = NULL;

    s = soc_sbx_g2p3_ace_rule_tail_get(fe, db, &sentinel);
    if (s != SB_OK) {
        return SOC_E_FAIL;
    }
    s = soc_sbx_g2p3_ace_rule_head_get(fe, db, h);
    if (s != SB_OK) {
        return SOC_E_FAIL;
    }

    for (i = 0; *h && i < irule; i++) {
        s = soc_sbx_g2p3_ace_rule_next_get(fe, db, *h, h);
        if (*h == sentinel)
            break;
    }

    if ( (*h == sentinel) && (!sentinelok) ) {
        *h = NULL;
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

/**
 * @fn soc_sbx_g2p3_ace_rule_add()
 *
 * @brief
 *
 * This function adds a rule to a rule-set, and returns a handle
 * to the new rule. This function is MODULE LOCAL and allows the
 * user to insert the rule in one of four manners.
 *
 *  - Insert Before Another Element In List
 *  - Insert After Another Element In List
 *  - Insert At Head of List
 *  - Insert At Tail of List
 *
 * @param pFe          - Driver control structure
 * @param pRule        - Rule pattern/action to add
 * @param ppRuleHandle - Returns handle to new rule
 * @param pRefRule     - Pointer to another rule in list. If this
 *                       is NON-NULL we insert before/after this
 *                       rule. If it IS NULL, then we insert the
 *                       rule at head/tail of the list.
 * @param bBefore      - If pRefRule is valid, this determines if
 *                       we insert before/after the rule in the list
 * @param bBefore      - If pRefRule is NULL, this determines if
 *                       we insert the rule at HEAD/TAIL of list
 *
 * @return - SB_OK on success, error code otherwise
 **/
static sbStatus_t
soc_sbx_g2p3_ace_rule_add(soc_sbx_g2p3_state_t *pFe,
                    soc_sbx_g2p3_cls_rule_db_e_t eDb,
                    soc_sbx_g2p3_ace_rule_t *pRule,
                    soc_sbx_g2p3_ace_mgr_rule_t **ppRuleHandle,
                    soc_sbx_g2p3_ace_mgr_rule_t *pRefRule,
                    uint8_t bBefore,
                    uint8_t bAddHead,
                    int16_t uGroupId)
{
  soc_sbx_g2p3_ace_mgr_t *pClsAce;
  soc_sbx_g2p3_ace_mgr_rule_t *pRuleI = NULL;
  soc_sbx_g2p3_ace_db_t *pDb = NULL;
  uint32_t uCapacity, uLength = 0, isC2;
  uint8_t bAvailable = 0;

  sbStatus_t status;
  SB_ASSERT(pFe);
  SB_ASSERT(pFe->pClsAce);

  pClsAce = pFe->pClsAce;

  isC2 = SAND_HAL_IS_FE2KXT(pFe->regSet);

  /* find database */
  ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
    if (eDb == pDb->eDbType)
      break;
  } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

  /* database not found */
  if (!pDb)
    return SB_BAD_ARGUMENT_ERR_CODE;

  /* validate capacity */
  if (!isC2) {
      if (eDb == SB_G2P3_CLS_QOS) {
          status = soc_sbx_g2p3_ifp_max_rule_capacity_get(pFe->unit,
                                                          &uCapacity);
      }else if (eDb == SB_G2P3_CLS_EGRESS) {
          status = soc_sbx_g2p3_efp_max_rule_capacity_get(pFe->unit,
                                                          &uCapacity);
      }else if (eDb == SB_G2P3_CLS_IPV6_QOS) {
          status = soc_sbx_g2p3_ifp_v6_max_rule_capacity_get(pFe->unit,
                                                             &uCapacity);
      }else if (eDb == SB_G2P3_CLS_IPV6_EGRESS) {
          status = soc_sbx_g2p3_efp_v6_max_rule_capacity_get(pFe->unit,
                                                            &uCapacity);
      }else{
          return SB_BAD_ARGUMENT_ERR_CODE;
      }
      if (status != SB_OK) {
          return SB_OTHER_ERR_CODE;
      }

      ACEDQ_LENGTH(&pDb->listRules, uLength);
      if ( uLength >= uCapacity) {
#ifdef RULE_ADD_DBG
          soc_cm_print("\n Capacity: 0x%x reached,", uCapacity);
          soc_cm_print(" cannot add additional rules\n");
#endif
         return SB_ACE_TOO_MANY_ACES;
      }
  } else {
      if (pDb->listRules == NULL) {
          uLength = 0;
      } else {
          ACEDQ_TRAVERSE(&pDb->listRules, pRuleI, soc_sbx_g2p3_ace_mgr_rule_t) {
              if (pRuleI) {
                  if (pRuleI->uGroupId == uGroupId) {
                      uLength++;
                  }
              }  
          } ACEDQ_TRAVERSE_END(&pDb->listRules, pRuleI, \
                                 soc_sbx_g2p3_ace_mgr_rule_t);
      }
      /* check whether there is space to add one more rule */
      soc_sbx_g2p3_cls_is_rule_capacity_available(pFe->unit, 
                                       &pClsAce->pGroups[uGroupId].schemaDesc,
                                       uLength,
                                       &bAvailable);

      if (bAvailable == 0) {
#ifdef RULE_ADD_DBG
          soc_cm_print("\n Capacity reached: Rule count 0x%x\n", uLength);
#endif
          return SB_ACE_TOO_MANY_ACES;
      }
  }
  /* validate the rule */
  status = soc_sbx_g2p3_ace_rule_validate(pFe,eDb,pRule);
  if(status != SB_OK) {
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  /* get a rule from the internal free-list */
  pRuleI = NULL;
  if (SB_OK != (status = soc_sbx_g2p3_ace_alloc_rule(pClsAce, &pRuleI)))
    return status;

  /* copy the rule pattern information, and define order */
  SB_MEMCPY(&pRuleI->Rule, pRule, sizeof(pRuleI->Rule));

  /* pass back reference to internal rule */
  *ppRuleHandle = pRuleI;
  /*pRuleI->bEnable   = 1;*/
  pRuleI->bcommitted = FALSE;

  /* clear the rule counts */
  if (soc_sbx_g2p3_ace_is_rule_enabled(&pRuleI->Rule)) {
    COMPILER_64_ZERO(pRuleI->counts.packets);
    COMPILER_64_ZERO(pRuleI->counts.bytes);
  }

  /* associated with database...this will be needed for
   * list management at time of deletion.
   */
  pRuleI->eDb = eDb;

  pRuleI->uGroupId = uGroupId;

  /* if we have a rule reference, then we are using it to
   * insert either before or after. If it is null, then we
   * are inserting either at the head or tail. */
  if (pRefRule) {
    if (bBefore) {
      ACEDQ_INSERT_PREV(&pDb->listRules, pRefRule, pRuleI);
    } else {
      ACEDQ_INSERT_NEXT(&pDb->listRules, pRefRule, pRuleI);
    }
  } else  {
    if (bAddHead != 0){
      ACEDQ_INSERT_HEAD(&pDb->listRules, pRuleI);
    }else{
      ACEDQ_INSERT_TAIL(&pDb->listRules, pRuleI);
    }
  }
  return SB_OK;
}

/**
 * @fn soc_sbx_g2p3_ace_rule_modify()
 *
 * @brief
 *
 * This function modifies a already existing rule in the
 * database identified by the rule handle and updates with
 * the latest update rule information. Here the db parameter
 * is used to verify the data structure integrity.
 *
 * @param pFe          - Driver control structure
 * @param pRule        - Rule pattern/action to add
 * @param ppRuleHandle - Returns handle to new rule
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_modify(soc_sbx_g2p3_state_t *pFe,
                       soc_sbx_g2p3_cls_rule_db_e_t eDb,
                       soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
                       soc_sbx_g2p3_ace_rule_t *pRule )
{
  sbStatus_t status = SB_OK;
  soc_sbx_g2p3_ace_rule_t *pSrcRule = &pRuleHandle->Rule;

  /* validate the rule */
  status = soc_sbx_g2p3_ace_rule_validate(pFe,eDb,pRule);
  if(status != SB_OK) {
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  SB_MEMCPY(pSrcRule, pRule, sizeof(soc_sbx_g2p3_ace_rule_t));

  pRuleHandle->bcommitted = 0;
  return status;
}


/**
 * @fn soc_sbx_g2p3_ace_rule_remove()
 *
 * @brief
 *
 * Remove a rule/action pair from a set, given a pointer to the rule.
 *
 * @param pFe         - Driver control structure
 * @param eDb         - Which rule database to remove from (Security, Qos)
 * @param ulRs   - Identifier of set this rule should be deleted from
 * @param pRuleHandle - Reference to the rule to be removed
 *
 * @return - SB_OK on success, SB_IN_PROGRESS for async, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_remove(soc_sbx_g2p3_state_t *pFe,
                       soc_sbx_g2p3_ace_mgr_rule_t *pRuleHandle)
{
  soc_sbx_g2p3_ace_mgr_t *pClsAce;
  soc_sbx_g2p3_ace_db_t *pDb = NULL;

  pClsAce = pFe->pClsAce;

  /* find database */
  ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
    if (pRuleHandle->eDb == pDb->eDbType)
      break;
  } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

  /* database not found */
  if (!pDb)
    return SB_BAD_ARGUMENT_ERR_CODE;

  ACEDQ_REMOVE(&pDb->listRules, pRuleHandle);

  return soc_sbx_g2p3_ace_free_rule(pFe->pClsAce, pRuleHandle);
}

/**
 * @fn soc_sbx_g2p3_ace_rule_handle_print()
 *
 * @brief
 *
 * Print a rule handle
 *
 * @param pRuleHandle - Reference to the rule handle to be printed
 *
 * @return - SB_OK
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_handle_print(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_mgr_rule_t *pRuleHandle)
{
  SB_LOG("RuleHandle = %p\n", (void *)pRuleHandle);
  SB_LOG("RuleHandle.ulMemChunkId    = 0x%x\n", pRuleHandle->ulMemChunkId);
  SB_LOG("RuleHandle.bEnable         = 0x%x\n", pRuleHandle->bEnable     );
  SB_LOG("RuleHandle.bcommitted       = 0x%x\n", pRuleHandle->bcommitted   );
  SB_LOG("RuleHandle.uRuleIdx        = 0x%x\n", pRuleHandle->uRuleIdx    );
  SB_LOG("RuleHandle.uRuleOffset     = 0x%x\n", pRuleHandle->uRuleOffset );
  SB_LOG("RuleHandle.uOldRuleIdx     = 0x%x\n", pRuleHandle->uOldRuleIdx    );
  SB_LOG("RuleHandle.uOldRuleOffset  = 0x%x\n", pRuleHandle->uOldRuleOffset );
  SB_LOG("RuleHandle.uNumPktHit   = 0x%x\n", (pRuleHandle->counts).packets    );
  /* print pattern */
  soc_sbx_g2p3_cls_ace_pattern_print(&pRuleHandle->Rule.uPattern.g2p3);
  /* print action */
  if (pRuleHandle->eDb == SB_G2P3_CLS_QOS) {
      soc_sbx_g2p3_irt_wrap_print(pFe->unit, &pRuleHandle->Rule.uAction.qos);
  }else{
      soc_sbx_g2p3_ert_wrap_print(pFe->unit, &pRuleHandle->Rule.uAction.egress);
  }

  return SB_OK;
}


/**
 * @fn soc_sbx_g2p3_ace_head_get()
 *
 * @brief
 *
 * Return a handle to first rule in database
 *
 * @param pFe          - Driver control structure
 * @param eDb          - Which rule database to modify (Security, Qos)
 * @param ppRuleHandle  - Reference to the rule to be modified
 *
 * @return - SB_OK on success, SB_IN_PROGRESS for async, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_head_get(soc_sbx_g2p3_state_t *pFe,
                               soc_sbx_g2p3_cls_rule_db_e_t eDb,
                               soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle)
{
  soc_sbx_g2p3_ace_db_t *pDb                    = NULL;
  soc_sbx_g2p3_ace_mgr_t *pClsAce               = pFe->pClsAce;

  /* find database */
  ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
    if (eDb == pDb->eDbType)
      break;
  } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

  /* database not found */
  if (!pDb)
    return SB_BAD_ARGUMENT_ERR_CODE;

  *ppRuleHandle = (soc_sbx_g2p3_ace_mgr_rule_t*)ACEDQ_HEAD(&pDb->listRules);
  return SB_OK;
}

/**
 * @fn soc_sbx_g2p3_ace_rule_tail_get()
 *
 * @brief
 *
 * Return a handle to last rule in database
 *
 * @param pFe          - Driver control structure
 * @param eDb          - Which rule database to modify (Security, Qos)
 * @param ppRuleHandle  - Reference to the rule to be modified
 *
 * @return - SB_OK on success, SB_IN_PROGRESS for async, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_tail_get(soc_sbx_g2p3_state_t *pFe,
                               soc_sbx_g2p3_cls_rule_db_e_t eDb,
                               soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle)
{
  soc_sbx_g2p3_ace_db_t *pDb                    = NULL;
  soc_sbx_g2p3_ace_mgr_t *pClsAce               = pFe->pClsAce;

  /* find database */
  ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
    if ( eDb == pDb->eDbType )
      break;
  } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

  /* database not found */
  if (!pDb)
    return SB_BAD_ARGUMENT_ERR_CODE;

  *ppRuleHandle = (soc_sbx_g2p3_ace_rule_handle_t) ACEDQ_TAIL(&pDb->listRules);

  return SB_OK;
}


/**
 * @fn soc_sbx_g2p3_ace_rule_next_get()
 *
 * @brief
 *
 * This function returns the next rule following the rule given by
 * the position of the rule handle passed in.
 *
 * @param pFe          - Driver control structure
 * @param pRefRule     - Handle to rule of the reference pointer
 * @param ppRuleHandle - Returns handle to the rule next to reference rule
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_next_get(soc_sbx_g2p3_state_t *pFe,
                        soc_sbx_g2p3_cls_rule_db_e_t eDb,
                        soc_sbx_g2p3_ace_rule_handle_t  pRefRule,
                        soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle)
{
  *ppRuleHandle = (soc_sbx_g2p3_ace_rule_handle_t )ACEDQ_NEXT(pRefRule);
  return SB_OK;
}

/**
 * @fn soc_sbx_g2p3_ace_rule_prev_get()
 *
 * @brief
 *
 * This function returns the  rule previous to the rule given by
 * the position of the rule handle passed in.
 *
 * @param pFe          - Driver control structure
 * @param pRefRule     - Handle to rule of the reference pointer
 * @param ppRuleHandle - Returns handle to the previous rule reference to the
 *                       reference rule.
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_prev_get(soc_sbx_g2p3_state_t *pFe,
                        soc_sbx_g2p3_cls_rule_db_e_t eDb,
                        soc_sbx_g2p3_ace_rule_handle_t  pRefRule,
                        soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle)
{
  *ppRuleHandle = (soc_sbx_g2p3_ace_rule_handle_t )ACEDQ_PREV(pRefRule);
  return SB_OK;
}


/**
 * @fn soc_sbx_g2p3_ace_rule_count()
 *
 * @brief
 *
 * This  function find the number of rules currently configured in the
 * database given by eDb
 *
 * @param pFe - driver control structure
 * @param eDb - Which rule database clear...
 *              SB_G2P3_CLS_QOS      = Clear out QOS rules
 *              SB_G2P3_CLS_EGRESS   = Clear out Security rules
 *              SB_G2P3_CLS_MAX      = Clear out both Security *and* QOS rules.
 * @param pCount -- Count of the number of Rules in this database
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_count_get(soc_sbx_g2p3_state_t *pFe,
                                soc_sbx_g2p3_cls_rule_db_e_t eDb,
                                uint32_t *pCount)
{
  soc_sbx_g2p3_ace_db_t *pDb                    = NULL;
  soc_sbx_g2p3_ace_mgr_t *pClsAce               = pFe->pClsAce;

  /* find database */
  ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
    if (eDb == pDb->eDbType)
      break;
  } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

  /* database not found */
  if (!pDb)
    return SB_BAD_ARGUMENT_ERR_CODE;

  ACEDQ_LENGTH(&pDb->listRules, *pCount);
  return SB_OK;
}

/**
 * @fn soc_sbx_g2p3_ace_commit_rule_db()
 *
 * @brief
 *
 * Commit a rule database to the hardware. This will pack the patterns into
 * the binary format needed by the hardware and transfer them to the pattern
 * memories inside the classifier. In addition, it will handle the transition
 * of banks from inactive to active state.
 *
 * @param pFe - Driver control structure
 * @param eDb - Which rule database to commit to hardware (Security, Qos)
 *
 * @return - SB_OK on success, SB_IN_PROGRESS for async, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_commit_rule_db(soc_sbx_g2p3_state_t *pFe,
                                soc_sbx_g2p3_cls_rule_db_e_t eDb)
{
    soc_sbx_g2p3_ace_db_t *pDb                    = NULL;
    soc_sbx_g2p3_cls_schema_t *pSchemaHandler        = NULL;
    soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory = NULL;
    soc_sbx_g2p3_ace_mgr_rule_t *pRule       = NULL;
    uint8_t bRuleEnabled                     = FALSE;
    soc_sbx_g2p3_counter_value_t cv;

    uint32_t uRuleCount = 0, uRSet=0; /*uRcInstance=0,*/
    uint32_t uRuleIdx = 0;
    uint32_t uIndex = 0;
    uint32_t oldRuleOffset = 0;
    uint8_t  uProgNumber = 0;
    sbStatus_t err = SB_OK;
    soc_sbx_g2p3_ace_mgr_t *pClsAce  = pFe->pClsAce;
    uint32_t isC2 = SAND_HAL_IS_FE2KXT(pFe->regSet);

    if (isC2) {
        return soc_sbx_g2p3_ace_commit_rule_db_c2(pFe, eDb);
    }

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* database not found */
    if (!pDb)
      return SB_BAD_ARGUMENT_ERR_CODE;

    /* get the schema hadnlers */
    pSchemaHandler = (soc_sbx_g2p3_cls_schema_t *) pDb->pSchemaDesc;

    /* get the pattern memory for this db */
    pPatternMemory = (soc_sbx_g2p3_cls_pattern_memory_t *) pDb->pPatternMemory;

    /* clear the pattern memory */
    soc_sbx_g2p3_cls_clear_pattern_memory(pFe, pSchemaHandler, pPatternMemory);

    if(SOC_E_NONE != soc_sbx_g2p3_cls_get_stby_prog_number(pFe, pSchemaHandler, &uProgNumber)){
        return SB_BAD_ARGUMENT_ERR_CODE;
    }

    /* obtain old rule offset for RT and counter accumulation */
    soc_sbx_g2p3_cls_get_ctr_offset(pFe, uProgNumber, &oldRuleOffset);

    /*
     * This function needs to be fixed once the interrupts
     * are working and we have async completes for rule
     * action, counter async completes.
     */
    /* iterate through rules */
    ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
            if (pRule) {
                bRuleEnabled = FALSE;
                if (pRule->Rule.uPattern.g2p3.bEnabled) {
                    bRuleEnabled = TRUE;
                    pSchemaHandler->pEncoder(pSchemaHandler,
                                             uRuleCount,
                                             &pRule->Rule.uPattern.g2p3,
                                             pPatternMemory);
                }

                if(bRuleEnabled) {

                    /* assign the index. */
                    uRuleIdx = uRuleCount;

                    pRule->uOldRuleIdx    = pRule->uRuleIdx;
                    pRule->uOldRuleOffset = pRule->uRuleOffset;
                    pRule->uRuleIdx       = uRuleIdx;
                    pRule->uRuleOffset    = oldRuleOffset;

                    /* add the rule action to the table */
                    err = SB_FAILED;
                    while (err!=SB_OK) {
                        if(pRule->Rule.eActionType == SB_G2P3_CLS_QOS) {
                            err = soc_sbx_g2p3_ace_rt_qos_action_add(pFe,pDb->bBank,
                                                                     uRuleIdx + oldRuleOffset,
                                                                     &pRule->Rule.uAction.qos);
                        } else if(pRule->Rule.eActionType == SB_G2P3_CLS_EGRESS) {
                            err = soc_sbx_g2p3_ace_rt_egr_action_add(pFe,pDb->bBank,
                                                                     uRuleIdx + oldRuleOffset,
                                                                     &pRule->Rule.uAction.egress);
                        } else {
                            SB_LOG("ERROR: Wrong ActionType\n");
                        }
                        if (err != SB_OK) {
                            SB_LOG("WARNING: Adding rule to table failed, err is %d\n", err);
                            return err;
                        }
                    }

                    uRSet = uRuleCount/pSchemaHandler->layout.uNumRulesPerRs;

                    uRuleCount++;

                }
            }
    } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);

    /*
     * every result will be returned via lower part of CRR register
     * we're not using upper part of CRR at all, in g2p3
     */
    for(uIndex=0; uIndex < G2P3_MAX_CLS_RULESETS_PER_SCHEMA; uIndex++) {
        pPatternMemory->uSaveResult[uIndex][0] = 0;
        pPatternMemory->uSaveResult[uIndex][1] = 0;
    }
    err = soc_sbx_g2p3_cls_commit(pFe,
                                  pDb->bBank,
                                  pSchemaHandler,
                                  pPatternMemory,
                                  &pDb->uProgramNumber);

    if (err){
        return err;
    }

    /* Carry the rule counts forward */
    uRuleCount = 0;
    ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
        if (pRule && pRule->Rule.uPattern.g2p3.bEnabled) {
            if(pRule->bcommitted) {
                uRuleCount = pRule->uOldRuleIdx + pRule->uOldRuleOffset;
#ifdef COUNTER_DEBUG
                soc_cm_print("\n%% Ace Commit Rcount[%d] Ridx[%d] Offs[%d]", \
                             uRuleCount, pRule->uRuleIdx, oldRuleOffset);
#endif
                err = SOC_E_PARAM;
                if(pRule->Rule.eActionType == SB_G2P3_CLS_QOS) {
                    err = soc_sbx_g2p3_irtctr_get(pFe->unit, 1, uRuleCount, &cv);
                } else if(pRule->Rule.eActionType == SB_G2P3_CLS_EGRESS) {
                    err = soc_sbx_g2p3_ertctr_get(pFe->unit, 1, uRuleCount, &cv);
                }
                
                if (!err){
#ifdef COUNTER_DEBUG
                    soc_cm_print("\n Present Rule Count pkt[%qx] byte[%qx]", \
                                 pRule->counts.packets, pRule->counts.bytes);
                    soc_cm_print("\n Counts got pkt[%qx] byte[%qx]", cv.packets, cv.bytes);
#endif
                    COMPILER_64_ADD_64(pRule->counts.packets, cv.packets);
                    COMPILER_64_ADD_64(pRule->counts.bytes, cv.bytes);
                }
            } else if(bRuleEnabled) {
                COMPILER_64_ZERO(pRule->counts.packets);
                COMPILER_64_ZERO(pRule->counts.bytes);
                pRule->bcommitted = TRUE;
            }
            uRuleCount++;
        }
    } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);

    return err;
}

sbStatus_t
soc_sbx_g2p3_ace_finalize_cb(soc_sbx_g2p3_state_t *pFe,
                             soc_sbx_g2p3_cls_rule_db_e_t eDb,
                             uint32_t uRsOffset)
{
    soc_sbx_g2p3_ace_db_t *pDb                    = NULL;
    soc_sbx_g2p3_ace_mgr_t *pClsAce  = pFe->pClsAce;
    soc_sbx_g2p3_ace_mgr_rule_t *pRule             = NULL;
    uint32_t uRuleCount;
    int err = SB_OK;
    soc_sbx_g2p3_counter_value_t * pCV;
    soc_sbx_g2p3_counter_params_t p;

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* database not found */
    if (!pDb)
      return SB_BAD_ARGUMENT_ERR_CODE;

    /* flush all the counters for the ruleset */
    err = soc_sbx_g2p3_rtctr_counter_params_get(pFe->unit, &p);

    pCV = (soc_sbx_g2p3_counter_value_t *) pFe->pClsComp->pTmpCtrMem;
    if (!err) {
        err = sbFe2000CmuCounterRead(pFe->pCmuMgr, p.bank, p.base + uRsOffset, G2P3_MAX_CLS_RULES_PER_RULESET_C2, (void*)pCV, 1, 1);
    }

    /* aggregate counts */
    if (!err) {
        ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
            if (pRule->uOldRuleOffset == uRsOffset) {
                uRuleCount = pRule->uOldRuleIdx + uRsOffset;
#ifdef COUNTER_DEBUG
                soc_cm_print("\n%% Ace Commit Rcount[%d] Ridx[%d] Offs[%d]", \
                             uRuleCount, pRule->uRuleIdx, uRsOffset);
#endif
                COMPILER_64_ADD_64(pRule->counts.packets, pCV[pRule->uOldRuleIdx].packets);
                COMPILER_64_ADD_64(pRule->counts.bytes, pCV[pRule->uOldRuleIdx].bytes);
#ifdef COUNTER_DEBUG
                soc_cm_print("\n Present Rule Count pkt[%qx] byte[%qx]", \
                             pRule->counts.packets, pRule->counts.bytes);
                soc_cm_print("\n Counts got pkt[%qx] byte[%qx]", pCV[pRule->uOldRuleIdx].packets, pCV[pRule->uOldRuleIdx].bytes);
#endif
            }
        } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);

    }

    return err;

}

sbStatus_t
soc_sbx_g2p3_ace_commit_rule_cb(soc_sbx_g2p3_state_t *pFe,
                               soc_sbx_g2p3_cls_schema_t *pSchema,
                               soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory,
                               void** pCookie,
                               uint32_t uRuleIndex,
                               uint32_t uRsOffset)
{
    soc_sbx_g2p3_ace_mgr_rule_t *pRule       = (soc_sbx_g2p3_ace_mgr_rule_t *) (*pCookie);
    sbStatus_t err = SB_OK;
    soc_sbx_g2p3_ace_mgr_t *pClsAce;
    soc_sbx_g2p3_ace_db_t *pDb = NULL;

    pClsAce = pFe->pClsAce;

    if (pRule == NULL){
        SB_ASSERT(0);
        return SB_FAILED;
    }
    
    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (pRule->eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* find next active rule, iterating backwards */
    if (pRule) {
        while ((pRule != NULL) && !(soc_sbx_g2p3_ace_is_rule_enabled(&pRule->Rule))) {
            pRule = (soc_sbx_g2p3_ace_mgr_rule_t *)ACEDQ_PREV(pRule);
            /* terminate when we pass head of list */
            if (ACEDQ_IS_TAIL(&pDb->listRules, pRule)){
                pRule = NULL;
                SB_ASSERT(0);
            }
        }
    } else {
        SB_LOG("ERROR: unexpectedly NULL rule pointer\n");
        SB_ASSERT(0);
    }

    /* when pSchema && pPattern Memory are null
     * this call back is used to traverse through the rules
     */
    if (pSchema == NULL && pPatternMemory == NULL) {
        /* move iterator */
        pRule = (soc_sbx_g2p3_ace_mgr_rule_t*) ACEDQ_PREV(pRule);
        if (ACEDQ_IS_TAIL(&pDb->listRules, pRule)){
            /* terminate when we pass head of list */
            pRule = NULL;
        }
        *pCookie = pRule;   
        return SB_OK;
    }

    SB_ASSERT(pSchema != NULL);
    SB_ASSERT(pPatternMemory != NULL);

    if (pRule) {
        pSchema->pEncoder(pSchema,
                          uRuleIndex,
                          soc_sbx_g2p3_ace_rule_pattern_get(&pRule->Rule),
                          pPatternMemory);
        /* save info for carrying counts forward */
        if (pRule->bcommitted){
            pRule->uOldRuleOffset = pRule->uRuleOffset;
            pRule->uOldRuleIdx = pRule->uRuleIdx;
        }else{
            /* set old values to sentinel values */
            pRule->uOldRuleOffset = -1;
            pRule->uOldRuleIdx = -1;
            pRule->bcommitted = 1;
        }

        pRule->uRuleIdx = uRuleIndex;
        pRule->uRuleOffset = uRsOffset;
#if 0
soc_cm_print("encoding rule 0x%x\n", uRuleIndex + pRule->uRuleOffset);
soc_sbx_g2p3_cls_ace_pattern_print( soc_sbx_g2p3_ace_rule_pattern_get(&pRule->Rule));
#endif
        /* update action table */
        err = SB_FAILED;
        while (err!=SB_OK) {
            if(pRule->Rule.eActionType == SB_G2P3_CLS_QOS) {
                err = soc_sbx_g2p3_ace_rt_qos_action_add(pFe, 0,
                                                         pRule->uRuleIdx + uRsOffset,
                                                         &pRule->Rule.uAction.qos);
            } else if(pRule->Rule.eActionType == SB_G2P3_CLS_EGRESS) {
                err = soc_sbx_g2p3_ace_rt_egr_action_add(pFe, 0,
                                                         pRule->uRuleIdx + uRsOffset,
                                                         &pRule->Rule.uAction.egress);
            } else {
                SB_LOG("ERROR: Wrong ActionType\n");
            }
            if (err != SB_OK) {
                SB_LOG("WARNING: Adding rule to table failed, err is %d\n", err);
                return err;
            }
        }
    }else{
        return SOC_E_NOT_FOUND;
    }

    /* move iterator */
    pRule = (soc_sbx_g2p3_ace_mgr_rule_t*) ACEDQ_PREV(pRule);
    if (ACEDQ_IS_TAIL(&pDb->listRules, pRule)){
        /* terminate when we pass head of list */
        pRule = NULL;
    }

    *pCookie = pRule;

    return err;
}

/*
 * Determine if any targeted rules for this ruleset are dirty.  Rules are 
 * considered dirty if:
 *   -ruleoffset with ruleset has changed
 *   -rule has not been previously committed
 * If the set of rules is not dirty, the rule pointer is advanced to the
 * end of the set.  If the set is dirty, the rule pointer is unchanged.
 */
sbStatus_t
soc_sbx_g2p3_ace_ruleset_dirty_cb(soc_sbx_g2p3_state_t *pFe,
                               soc_sbx_g2p3_cls_schema_t *pSchema,
                               void** pCookie,
                               uint32_t* pDirty,
                               uint32_t uRuleCount,
                               uint32_t uRuleIndex,
                               uint32_t uRsOffset)
{
    soc_sbx_g2p3_ace_mgr_rule_t *pRule       = (soc_sbx_g2p3_ace_mgr_rule_t *) (*pCookie);
    sbStatus_t err = SB_OK;
    soc_sbx_g2p3_ace_mgr_t *pClsAce;
    soc_sbx_g2p3_ace_db_t *pDb = NULL;
    uint32_t count = 0;

    *pDirty = 0;

    pClsAce = pFe->pClsAce;

    if (pRule == NULL){
        SB_ASSERT(0);
        return SB_FAILED;
    }

    if (uRuleCount == 0) {
        return SB_OK;
    }
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (pRule->eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* iterate thru active rules */
    while (count < uRuleCount) {
        /* find next active rule, iterating backwards */
        if (pRule) {
            while ((pRule != NULL) && !(soc_sbx_g2p3_ace_is_rule_enabled(&pRule->Rule))) {
                pRule = (soc_sbx_g2p3_ace_mgr_rule_t *)ACEDQ_PREV(pRule);
                /* terminate when we pass head of list */
                if (ACEDQ_IS_TAIL(&pDb->listRules, pRule)){
                    pRule = NULL;
                    SB_ASSERT(0);
                }
            }
        } else {
            SB_LOG("ERROR: unexpectedly NULL rule pointer\n");
            SB_ASSERT(0);
        }

        if (!pRule) {
            return SB_FAILED;
        }

        /* check for dirty rule */
        if (!pRule->bcommitted) {
            *pDirty = 1;
        }else if (pRule->uRuleIdx != uRuleIndex) {
            *pDirty = 1;
        }else if (pRule->uRuleOffset != uRsOffset) {
            *pDirty = 1;
        }

        if (*pDirty) {
            return err;
        }

        count++;
        uRuleIndex--;

        pRule = (soc_sbx_g2p3_ace_mgr_rule_t*) ACEDQ_PREV(pRule);
        if (ACEDQ_IS_TAIL(&pDb->listRules, pRule)){
            /* terminate when we pass head of list */
            pRule = NULL;
        }
    }

    /* ruleset clean, move iterator */
    *pCookie = pRule;

    return err;
}

sbStatus_t
soc_sbx_g2p3_ace_commit_rule_db_c2(soc_sbx_g2p3_state_t *pFe,
                                soc_sbx_g2p3_cls_rule_db_e_t eDb)
{
    soc_sbx_g2p3_ace_db_t *pDb                    = NULL;
    soc_sbx_g2p3_cls_schema_t *pSchemaHandler        = NULL;
    soc_sbx_g2p3_cls_pattern_memory_t *pPatternMemory = NULL;
    soc_sbx_g2p3_ace_mgr_rule_t *pRule             = NULL;
    uint32_t uRuleCount = 0;
    sbStatus_t err = SB_OK;
    soc_sbx_g2p3_ace_mgr_t *pClsAce  = pFe->pClsAce;
    int16_t uGroupIndex = -1;
    soc_sbx_g2p3_cls_group_t *clsGroup = pClsAce->pTmpMem;
    uint8_t uGroupCount = 0; /*, uLastGroupWithRule = -1; */
    soc_sbx_g2p3_ace_group_t *pGroup;

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    /* database not found */
    if (!pDb) {
      return SB_BAD_ARGUMENT_ERR_CODE;
    }

    /* get the schema handlers */
    pSchemaHandler = (soc_sbx_g2p3_cls_schema_t *) &pDb->listGroups->schemaDesc;

    /* count the number of groups */
    ACEDQ_TRAVERSE(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t) {
        clsGroup[uGroupCount].uGroupId = pGroup->uGroupId;
        clsGroup[uGroupCount].pSchemaDesc = &pGroup->schemaDesc;
        clsGroup[uGroupCount].uRuleCount = 0;
        uGroupCount++;
    } ACEDQ_TRAVERSE_END(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t);

    /* calculate number of rules in each group */
    uGroupIndex = 0;

    ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
        if (pRule) {
            if (pRule->bcommitted) {
                pRule->uOldRuleOffset = pRule->uRuleOffset;
                pRule->uOldRuleIdx = pRule->uRuleIdx;
            } else {
                pRule->uOldRuleOffset = -1;
                pRule->uOldRuleIdx = -1;
            }
            if (soc_sbx_g2p3_ace_is_rule_enabled(&pRule->Rule)) {
                if(pRule->uGroupId == clsGroup[uGroupIndex].uGroupId) {
                    clsGroup[uGroupIndex].uRuleCount++;
                    clsGroup[uGroupIndex].pRuleList = (adq_p_t) pRule;
                } else {
                    do {
                        if(clsGroup[uGroupIndex].uRuleCount == 0) {
                            clsGroup[uGroupIndex].pRuleList = NULL;
                        }
                        uGroupIndex++;
                    }while (clsGroup[uGroupIndex].uGroupId != pRule->uGroupId && uGroupIndex <= uGroupCount);
                    if (uGroupIndex <= uGroupCount) {
                        clsGroup[uGroupIndex].pRuleList = (adq_p_t) pRule;
                        clsGroup[uGroupIndex].uRuleCount++;
                    }
                }
            }
        }
    } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);

    /* get the pattern memory for this db */
    pPatternMemory = (soc_sbx_g2p3_cls_pattern_memory_t *) pDb->pPatternMemory;

    /* clear the pattern memory */
    soc_sbx_g2p3_cls_clear_pattern_memory(pFe, pSchemaHandler, pPatternMemory);
    err = soc_sbx_g2p3_cls_commit_c2(pFe,
                                       pSchemaHandler,
                                       pPatternMemory,
                                       eDb,
                                       (adq_p_t)(pDb->listRules),
                                       uRuleCount,
                                       &clsGroup[0],
                                       uGroupCount,
                                       soc_sbx_g2p3_ace_commit_rule_cb,
                                       soc_sbx_g2p3_ace_finalize_cb);
    if (err != SB_OK){
        return err;
    }

    /* update rulecounts */
    uGroupIndex = 0;
    ACEDQ_TRAVERSE(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t) {
        pGroup->uRuleCount = clsGroup[uGroupIndex++].uRuleCount;
    } ACEDQ_TRAVERSE_END(&pDb->listGroups, pGroup, soc_sbx_g2p3_ace_group_t);

    /* update scache */
    err = soc_sbx_g2p3_ace_scache_store(pFe);
    if (err != SB_OK){
        return err;
    }
    return err;
}

static sbStatus_t
soc_sbx_g2p3_ace_rule_validate(soc_sbx_g2p3_state_t *pFe,
                         soc_sbx_g2p3_cls_rule_db_e_t eDb,
                         soc_sbx_g2p3_ace_rule_t *pRule )
{

    switch(pRule->ePatternType) {
        case SB_G2P3_CLS_PATTERN:
        case SB_G2P3_CLS_PATTERN_LLC:
            if(!soc_sbx_g2p3_cls_ace_pattern_validate(&pRule->uPattern.g2p3) ) {
                return SB_BAD_ARGUMENT_ERR_CODE;
            }
            break;
        case SB_G2P3_CLS_PATTERN_IPV6:
            if(!soc_sbx_g2p3_cls_ace_pattern_ipv6_validate(&pRule->uPattern.g2p3_ipv6) ) {
                return SB_BAD_ARGUMENT_ERR_CODE;
            }
            break;
        default:
            break;
    }
    return SB_OK;
}

/**
 * @fn soc_sbx_g2p3_ace_mem_compact()
 *
 * @brief
 *
 * This function can be used to reclaim memory that was allocated to the
 * classification database for storing rules. When rules are allocated, if
 * there is not enough memory to hold the new rules it will request memory
 * from the system in "chunks". Later, when this memory is no longer in use
 * the entries can be coalesced into fewer "chunks", and the free memory can
 * be returned to the system
 *
 * @param pFe - Driver control structure
 *
 * @return - Number of memory 'chunks' that were reclaimed
 **/
uint32_t
soc_sbx_g2p3_ace_mem_compact(soc_sbx_g2p3_state_t *pFe)
{
  uint32_t               ulLen, ulChunkTodo, ulChunkId, ulWorkChunk, ulListLen;
  soc_sbx_g2p3_ace_mgr_t     *pClsAce;
  soc_sbx_g2p3_ace_db_t      *pDb;
  soc_sbx_g2p3_ace_mgr_rule_t *pRule, *pNew, *pNext;
  adq_p_t   holdList;
  sbStatus_t             sts;

  SB_ASSERT(pFe);
  SB_ASSERT(pFe->pClsAce);

  pClsAce = pFe->pClsAce;

  /* are enough rules free, to eliminate a chunk? */
  ACEDQ_LENGTH(&pClsAce->listFreeRules, ulLen);
  ulChunkTodo = ulLen / pClsAce->ulRulesPerChunk;
  if (!ulChunkTodo)
    return 0;

  /* eliminate as many chunks as possible */
  for (ulWorkChunk = 0; ulWorkChunk < ulChunkTodo; ulWorkChunk++) {
    holdList = NULL;

    /* we will eliminate the last allocated chunk the
     * reference count is 1-based, arrays are 0-based */
    ulChunkId = pClsAce->ulChunkCount - 1;

    ACEDQ_TRAVERSE(&pClsAce->listFreeRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
      if (pRule->ulMemChunkId == ulChunkId) {
        ACEDQ_REMOVE(&pClsAce->listFreeRules, pRule);
        ACEDQ_INSERT_TAIL(&holdList, pRule);
      }
    } ACEDQ_TRAVERSE_END(&pClsAce->listFreeRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);

    /* iterate through database list */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      /* iterate through ruleset */
      ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
        if (pRule->ulMemChunkId == ulChunkId) {
          pNext = (soc_sbx_g2p3_ace_mgr_rule_t*)ACEDQ_NEXT(pRule);
          if (ACEDQ_IS_HEAD(&pDb->listRules, pNext)){
              /* terminate when we pass the tail */
              pNext = NULL;
          }
          sts = soc_sbx_g2p3_ace_alloc_rule(pClsAce, &pNew);
          if (sts != SB_OK)
            return sts;
          ACEDQ_REMOVE(&pDb->listRules, pRule);
          SB_MEMCPY(&pNew->Rule, &pRule->Rule, sizeof(pNew->Rule));
          pNew->bEnable = pRule->bEnable;
          ACEDQ_INSERT_TAIL(&holdList, pRule);
          if (pNext) {
            ACEDQ_INSERT_PREV(&pDb->listRules, pNext, pNew);
          }else{
            ACEDQ_INSERT_TAIL(&pDb->listRules, pNew);
          }
        }
      } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

#if 1 /* xxx todo -- remove me at release. sanity check */
    ACEDQ_LENGTH(&holdList, ulListLen);
    SB_ASSERT(pClsAce->ulRulesPerChunk == ulListLen);
#endif

    /* free memory, decrement chunk count */
    thin_host_free(pClsAce->aMemChunks[pClsAce->ulChunkCount]);
    pClsAce->ulChunkCount--;
  }

  return ulChunkTodo;
}

/**
 * @fn soc_sbx_g2p3_ace_bind_schema_to_rule_db()
 *
 * @brief - Binds the Rule Database with a scehma
 *
 * @param pFe   -- Pointer to the FE context
 * @param pDb   -- Pointer to the Classifier Rule database
 * @param eType -- Schema Type
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_bind_schema_to_rule_db(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_db_t *pDb, soc_sbx_g2p3_cls_schema_enum_t eType)
{
  /* once the header files are separated we can do strict pointers */
  pDb->pSchemaDesc = (void *) soc_sbx_g2p3_cls_schema_handler_get(pFe, eType);

  if (pDb->pSchemaDesc == NULL)
    return SB_INVALID_HANDLE;
  return SB_OK;
}

/**
 * @fn AllocRsPatternMemory()
 *
 * @brief - creates a pattern memory pool for give set with a given schema
 *
 * @param pFe   -- Pointer to the FE context
 * @param pDb   -- Pointer to the Classifier Rule database
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_alloc_rset_pattern_memory(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_db_t *pDb)
{
  return soc_sbx_g2p3_cls_alloc_pattern_memory(pFe, pDb->pSchemaDesc, &pDb->pPatternMemory);
}

/**
 * @fn soc_sbx_g2p3_ace_free_rset_pattern_memory()
 *
 * @brief - free ruleset memory
 *
 * @param pFe   -- Pointer to the FE context
 * @param pDb   -- Pointer to the Classifier Rule database
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_free_rset_pattern_memory(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_db_t *pDb)
{
  return soc_sbx_g2p3_cls_free_pattern_memory(pFe, pDb->pSchemaDesc, pDb->pPatternMemory);
}

/**
 * @fn soc_sbx_g2p3_ace_rt_qos_action_add()
 *
 * @brief - Switchs the bank id of the RuleDb to be used the next time.
 *
 * @param pFe   -- Pointer to the FE context
 * @param bBank -- Bank Id
 * @param uIndex -- rule Index
 * @param pAction -- QOS rule Action
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rt_qos_action_add(soc_sbx_g2p3_state_t *pFe, uint8_t bBank, uint32_t uIndex,
               soc_sbx_g2p3_irt_wrap_t *pAction)
{
  return soc_sbx_g2p3_irt_wrap_set(pFe->unit, (int)uIndex, pAction);
}

/**
 * @fn soc_sbx_g2p3_ace_rt_egr_action_add()
 *
 * @brief - Adds the security action to the database
 *
 * @param pFe   -- Pointer to the FE context
 * @param bBank -- Bank Id
 * @param uIndex -- rule Index
 * @param pAction -- Security rule Action
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rt_egr_action_add(soc_sbx_g2p3_state_t *pFe , uint8_t bBank, uint32_t uIndex,
                         soc_sbx_g2p3_ert_wrap_t *pAction)
{
  return soc_sbx_g2p3_ert_wrap_set(pFe->unit, (int) uIndex, pAction);
}


/**
 * @fn soc_sbx_g2p3_ace_alloc_mem_chunk()
 *
 * @brief - Allocate a chunk of memory, and split it into rules that
 *          are then added onto the free list.
 *
 * @param pClsAce - Pointer to classifier control structure
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_alloc_mem_chunk(soc_sbx_g2p3_ace_mgr_t *pClsAce)
{
  soc_sbx_g2p3_ace_mgr_rule_t *pRule;
  uint32_t               i;
  void                  *vp;

  SB_ASSERT(pClsAce);

  if (MAX_MEM_CHUNKS <= pClsAce->ulChunkCount)
    return SB_MALLOC_FAILED;

  /* allocate chunk of elements */
  vp = thin_host_malloc(pClsAce->ulRulesPerChunk * sizeof(soc_sbx_g2p3_ace_mgr_rule_t));
  if (!vp)
    return SB_MALLOC_FAILED;

  /* array of rules */
  pRule = (soc_sbx_g2p3_ace_mgr_rule_t *) vp;

  /* go through and them individually to free list */
  for (i = 0; i < pClsAce->ulRulesPerChunk; i++) {
    /* init element struct */
    ACEDQ_INIT(&pRule[i]);

    /* init rule struct */
    pRule[i].ulMemChunkId = pClsAce->ulChunkCount;
    pRule[i].bEnable = 1;
    SB_MEMSET(&pRule[i].Rule, 0x0, sizeof(pRule[i].Rule));

    /* push onto free list */
    ACEDQ_INSERT_TAIL(&pClsAce->listFreeRules, &pRule[i]);
  }

  /* keep track of how many chunks have been allocated */
  pClsAce->aMemChunks[pClsAce->ulChunkCount++] = vp;

  return SB_OK;
}

/**
 * @fn soc_sbx_g2p3_ace_alloc_rule()
 *
 * @brief
 *
 * Get rule from free list. If we're out of rules, allocate another memory
 * chunk to satisfy the request. Add remaining memory to the free-list
 *
 * @param pClsAce - Pointer to classifier control structure
 * @param pRule - Rule pointer (passed by reference, returns pointer to rule)
 *
 * @return SB_OK on success, error code otherwise
 **/
static sbStatus_t
soc_sbx_g2p3_ace_alloc_rule(soc_sbx_g2p3_ace_mgr_t *pClsAce, soc_sbx_g2p3_ace_mgr_rule_t **pRule)
{
  sbStatus_t status;

  /* initialize pointer to NULL */
  *pRule = NULL;

  /* get a rule off free list */
  if (ACEDQ_EMPTY(&pClsAce->listFreeRules) == FALSE)
  {
    ACEDQ_REMOVE_HEAD(&pClsAce->listFreeRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);
  }

  /* if successful, return it */
  if (*pRule) {
    return SB_OK;
  }

  /* no rule avail free list, so allocate another memory chunk */
  status = soc_sbx_g2p3_ace_alloc_mem_chunk(pClsAce);
  if (status != SB_OK)
    return status;

  /* no try and get a rule again off 'refreshed' free list */
  if (ACEDQ_EMPTY(&pClsAce->listFreeRules) == FALSE)
    ACEDQ_REMOVE_HEAD(&pClsAce->listFreeRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);
  else 
    pRule = NULL;

  /* if successful, return it */
  if (pRule) {
    return SB_OK;
  }

  /* unreachable, should have failed soc_sbx_g2p3_ace_alloc_mem_chunk() earlier */
  SB_ASSERT(0);

  return SB_MALLOC_FAILED;
}

/**
 * @fn soc_sbx_g2p3_ace_free_rule()
 *
 * @brief - Return a rule to the free-list. This will allow the rule to be
 *          reused later by later allocations.
 *
 * @param pClsAce - Pointer to classifier control structure
 * @param pRule - Rule pointer
 *
 * @return SB_OK on success, error code otherwise
 **/
static sbStatus_t
soc_sbx_g2p3_ace_free_rule(soc_sbx_g2p3_ace_mgr_t *pClsAce, soc_sbx_g2p3_ace_mgr_rule_t *pRule)
{
  ACEDQ_INSERT_HEAD(&pClsAce->listFreeRules, pRule);
  return SB_OK;
}

/**
 * @fn soc_sbx_g2p3_ace_rule_from_handle()
 *
 * @brief
 *
 * Given a classifier handle, return pointer to associated rule
 *
 * @param pHandle - rule handle
 *
 * @return - pointer to rule described by handle
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_from_handle(soc_sbx_g2p3_state_t *pFe,
                           soc_sbx_g2p3_cls_rule_db_e_t eDb,
                           soc_sbx_g2p3_ace_mgr_rule_t *pHandle,
                           soc_sbx_g2p3_ace_rule_t *pUserCopy)
{
  /* TODO: validate the handle w.r.t database type */
  SB_MEMCPY(pUserCopy, &pHandle->Rule, sizeof(soc_sbx_g2p3_ace_rule_t));
  return SB_OK;
}

/*
 * RCE
 */

void
soc_sbx_g2p3_cls_ace_pattern_print(soc_sbx_g2p3_cls_ace_pattern_t *pFromStruct)
{
  SB_LOG("G2P3FeClsAcePattern:: dbtypeany=0x%01x", (unsigned int)  pFromStruct->bDbTypeAny);
  SB_LOG(" portany=0x%01x", (unsigned int)  pFromStruct->bPortAny);
  SB_LOG(" priorityany=0x%01x", (unsigned int)  pFromStruct->bPriorityAny);
  SB_LOG(" fragmentany=0x%01x", (unsigned int)  pFromStruct->bFragmentAny);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: tcpflagurgany=0x%01x", (unsigned int)  pFromStruct->bTcpFlagURGAny);
  SB_LOG(" tcpflagackany=0x%01x", (unsigned int)  pFromStruct->bTcpFlagACKAny);
  SB_LOG(" tcpflagpshany=0x%01x", (unsigned int)  pFromStruct->bTcpFlagPSHAny);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: tcpflagrstany=0x%01x", (unsigned int)  pFromStruct->bTcpFlagRSTAny);
  SB_LOG(" tcpflagsynany=0x%01x", (unsigned int)  pFromStruct->bTcpFlagSYNAny);
  SB_LOG(" tcpflagfinany=0x%01x", (unsigned int)  pFromStruct->bTcpFlagFINAny);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: dscpany=0x%01x", (unsigned int)  pFromStruct->bDscpAny);
  SB_LOG(" ecnany=0x%01x", (unsigned int)  pFromStruct->bEcnAny);
  SB_LOG(" protocolany=0x%01x", (unsigned int)  pFromStruct->bProtocolAny);
  SB_LOG(" ipv4saany=0x%01x", (unsigned int)  pFromStruct->bIpv4SaAny);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: ipv4daany=0x%01x", (unsigned int)  pFromStruct->bIpv4DaAny);
  SB_LOG(" ethertypeany=0x%01x", (unsigned int)  pFromStruct->bEtherTypeAny);
  SB_LOG(" smacany=0x%01x", (unsigned int)  pFromStruct->bSmacAny);
  SB_LOG(" vlanany=0x%01x", (unsigned int)  pFromStruct->bVlanAny);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: dmacany=0x%01x", (unsigned int)  pFromStruct->bDmacAny);
  SB_LOG(" dbtype=0x%02x", (unsigned int)  pFromStruct->uDbType);
  SB_LOG(" port=0x%02x", (unsigned int)  pFromStruct->uPort);
  SB_LOG(" priority=0x%01x", (unsigned int)  pFromStruct->uPriority);
  SB_LOG(" fragment=0x%01x", (unsigned int)  pFromStruct->uFragment);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: tcpflagurg=0x%01x", (unsigned int)  pFromStruct->uTcpFlagURG);
  SB_LOG(" tcpflagack=0x%01x", (unsigned int)  pFromStruct->uTcpFlagACK);
  SB_LOG(" tcpflagpsh=0x%01x", (unsigned int)  pFromStruct->uTcpFlagPSH);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: tcpflagrst=0x%01x", (unsigned int)  pFromStruct->uTcpFlagRST);
  SB_LOG(" tcpflagsyn=0x%01x", (unsigned int)  pFromStruct->uTcpFlagSYN);
  SB_LOG(" tcpflagfin=0x%01x", (unsigned int)  pFromStruct->uTcpFlagFIN);
  SB_LOG(" dscp=0x%02x", (unsigned int)  pFromStruct->uDscp);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: ecn=0x%01x", (unsigned int)  pFromStruct->uEcn);
  SB_LOG(" protocol=0x%02x", (unsigned int)  pFromStruct->uProtocol);
  SB_LOG(" l4srcporthigh=0x%04x", (unsigned int)  pFromStruct->uL4SrcPortHigh);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: l4srcportlow=0x%04x", (unsigned int)  pFromStruct->uL4SrcPortLow);
  SB_LOG(" l4dstporthigh=0x%04x", (unsigned int)  pFromStruct->uL4DstPortHigh);
  SB_LOG(" l4dstportlow=0x%04x", (unsigned int)  pFromStruct->uL4DstPortLow);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: bdsapany=0x%01x", (unsigned int) pFromStruct->bDsapAny);
  SB_LOG(" llcdsapssaphigh=0x%04x", (unsigned int)  pFromStruct->uLlcDsapSsapHigh);
  SB_LOG(" llcdsapssaplow=0x%08x", (unsigned int)  pFromStruct->uLlcDsapSsapLow);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: bllcctrlany=0x%01x", (unsigned int) pFromStruct->bLlcCtrlAny);
  SB_LOG(" llcctrldatahigh=0x%04x", (unsigned int)  pFromStruct->uLlcCtrlDataHigh);
  SB_LOG(" llcctrldatalow=0x%04x", (unsigned int)  pFromStruct->uLlcCtrlDataLow);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: ipv4sa=0x%08x", (unsigned int)  pFromStruct->uIpv4Sa);
  SB_LOG(" ipv4salen=0x%02x", (unsigned int)  pFromStruct->uIpv4SaLen);
  SB_LOG("\n");
  SB_LOG("G2P3FeClsAcePattern:: ipv4da=0x%08x", (unsigned int)  pFromStruct->uIpv4Da);
  SB_LOG(" ipv4dalen=0x%02x", (unsigned int)  pFromStruct->uIpv4DaLen);
  SB_LOG(" ethertype=0x%04x", (unsigned int)  pFromStruct->uEtherType);
  SB_LOG("\n");

  SB_LOG("G2P3FeClsAcePattern:: smac=0x%012llx", (uint64_t)  pFromStruct->uuSmac);
  SB_LOG(" smaclen=0x%02x", (unsigned int)  pFromStruct->uuSmacLen);
  SB_LOG(" vlan=0x%04x", (unsigned int)  pFromStruct->uVlan);
  SB_LOG(" dmac=0x%012llx", (uint64_t)  pFromStruct->uuDmac);
  SB_LOG("\n");
  SB_LOG("G2P3FeClsAcePattern:: dmaclen=0x%02x", (unsigned int)  pFromStruct->uuDmacLen);
  SB_LOG(" pbmp=0x%013llx", (uint64_t)  pFromStruct->uuPbmp);
  SB_LOG(" enabled=0x%01x", (unsigned int)  pFromStruct->bEnabled);
  SB_LOG("\n");
}

int
soc_sbx_g2p3_cls_ace_pattern_validate(soc_sbx_g2p3_cls_ace_pattern_t *pZf)
{
  if (pZf->bDbTypeAny > 0x1) return 0;
  if (pZf->bPortAny > 0x1) return 0;
  if (pZf->bPriorityAny > 0x1) return 0;
  if (pZf->bFragmentAny > 0x1) return 0;
  if (pZf->bTcpFlagURGAny > 0x1) return 0;
  if (pZf->bTcpFlagACKAny > 0x1) return 0;
  if (pZf->bTcpFlagPSHAny > 0x1) return 0;
  if (pZf->bTcpFlagRSTAny > 0x1) return 0;
  if (pZf->bTcpFlagSYNAny > 0x1) return 0;
  if (pZf->bTcpFlagFINAny > 0x1) return 0;
  if (pZf->bDscpAny > 0x1) return 0;
  if (pZf->bEcnAny > 0x1) return 0;
  if (pZf->bProtocolAny > 0x1) return 0;
  if (pZf->bIpv4SaAny > 0x1) return 0;
  if (pZf->bIpv4DaAny > 0x1) return 0;
  if (pZf->bEtherTypeAny > 0x1) return 0;
  if (pZf->bSmacAny > 0x1) return 0;
  if (pZf->bVlanAny > 0x1) return 0;
  if (pZf->bDmacAny > 0x1) return 0;
  if (pZf->uDbType > 0x3) return 0;
  if (pZf->uPort > 0x3f) return 0;
  if (pZf->uPriority > 0x7) return 0;
  if (pZf->uFragment > 0x1) return 0;
  if (pZf->uTcpFlagURG > 0x1) return 0;
  if (pZf->uTcpFlagACK > 0x1) return 0;
  if (pZf->uTcpFlagPSH > 0x1) return 0;
  if (pZf->uTcpFlagRST > 0x1) return 0;
  if (pZf->uTcpFlagSYN > 0x1) return 0;
  if (pZf->uTcpFlagFIN > 0x1) return 0;
  if (pZf->uDscp > 0x3f) return 0;
  if (pZf->uEcn > 0x3) return 0;
  if (pZf->uProtocol > 0xff) return 0;
  /* If Ether Type is 0(LLC) validate LLC fields
   * else validate Dix or SNAP fields */
  if(!pZf->bEtherTypeAny && !pZf->uEtherType) {
      if (pZf->uLlcSsapHigh > 0xff) return 0;
      if (pZf->uLlcSsapLow > 0xff) return 0;
      if (pZf->uLlcDsapHigh > 0xff) return 0;
      if (pZf->uLlcDsapLow > 0xff) return 0;
      if (pZf->uLlcCtrlHigh > 0xff) return 0;
      if (pZf->uLlcCtrlLow > 0xff) return 0;
  } else {
      if (pZf->uL4SrcPortHigh > 0xffff) return 0;
      if (pZf->uL4SrcPortLow > 0xffff) return 0;
      if (pZf->uL4DstPortHigh > 0xffff) return 0;
      if (pZf->uL4DstPortLow > 0xffff) return 0;
      if (pZf->uEtherType > 0xffff) return 0;
  }
  /* pZf->uIpv4Sa implicitly masked by data type */
  if (pZf->uIpv4SaLen > 0x3f) return 0;
  /* pZf->uIpv4Da implicitly masked by data type */
  if (pZf->uIpv4DaLen > 0x3f) return 0;
  if (pZf->uuSmac > 0xFFFFFFFFFFFFULL) return 0;
  if (pZf->uuSmacLen > 0x30) return 0;
  if (pZf->uVlan > 0x3fff) return 0;
  if (pZf->uuDmac > 0xFFFFFFFFFFFFULL) return 0;
  if (pZf->uuDmacLen > 0x30) return 0;
  if (pZf->bEnabled > 0x1) return 0;

  return 1; /* success */
}

int
soc_sbx_g2p3_cls_ace_pattern_set(soc_sbx_g2p3_cls_ace_pattern_t *s, char* name, int value)
{
  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "dbtypeany") == 0) {
    s->bDbTypeAny = value;
  } else if (SB_STRCMP(name, "portany") == 0) {
    s->bPortAny = value;
  } else if (SB_STRCMP(name, "priorityany") == 0) {
    s->bPriorityAny = value;
  } else if (SB_STRCMP(name, "fragmentany") == 0) {
    s->bFragmentAny = value;
  } else if (SB_STRCMP(name, "tcpflagurgany") == 0) {
    s->bTcpFlagURGAny = value;
  } else if (SB_STRCMP(name, "tcpflagackany") == 0) {
    s->bTcpFlagACKAny = value;
  } else if (SB_STRCMP(name, "tcpflagpshany") == 0) {
    s->bTcpFlagPSHAny = value;
  } else if (SB_STRCMP(name, "tcpflagrstany") == 0) {
    s->bTcpFlagRSTAny = value;
  } else if (SB_STRCMP(name, "tcpflagsynany") == 0) {
    s->bTcpFlagSYNAny = value;
  } else if (SB_STRCMP(name, "tcpflagfinany") == 0) {
    s->bTcpFlagFINAny = value;
  } else if (SB_STRCMP(name, "dscpany") == 0) {
    s->bDscpAny = value;
  } else if (SB_STRCMP(name, "ecnany") == 0) {
    s->bEcnAny = value;
  } else if (SB_STRCMP(name, "protocolany") == 0) {
    s->bProtocolAny = value;
  } else if (SB_STRCMP(name, "ipv4saany") == 0) {
    s->bIpv4SaAny = value;
  } else if (SB_STRCMP(name, "ipv4daany") == 0) {
    s->bIpv4DaAny = value;
  } else if (SB_STRCMP(name, "ethertypeany") == 0) {
    s->bEtherTypeAny = value;
  } else if (SB_STRCMP(name, "smacany") == 0) {
    s->bSmacAny = value;
  } else if (SB_STRCMP(name, "vlanany") == 0) {
    s->bVlanAny = value;
  } else if (SB_STRCMP(name, "dmacany") == 0) {
    s->bDmacAny = value;
  } else if (SB_STRCMP(name, "udbtype") == 0) {
    s->uDbType = value;
  } else if (SB_STRCMP(name, "uport") == 0) {
    s->uPort = value;
  } else if (SB_STRCMP(name, "upriority") == 0) {
    s->uPriority = value;
  } else if (SB_STRCMP(name, "ufragment") == 0) {
    s->uFragment = value;
  } else if (SB_STRCMP(name, "utcpflagurg") == 0) {
    s->uTcpFlagURG = value;
  } else if (SB_STRCMP(name, "utcpflagack") == 0) {
    s->uTcpFlagACK = value;
  } else if (SB_STRCMP(name, "utcpflagpsh") == 0) {
    s->uTcpFlagPSH = value;
  } else if (SB_STRCMP(name, "utcpflagrst") == 0) {
    s->uTcpFlagRST = value;
  } else if (SB_STRCMP(name, "utcpflagsyn") == 0) {
    s->uTcpFlagSYN = value;
  } else if (SB_STRCMP(name, "utcpflagfin") == 0) {
    s->uTcpFlagFIN = value;
  } else if (SB_STRCMP(name, "udscp") == 0) {
    s->uDscp = value;
  } else if (SB_STRCMP(name, "uecn") == 0) {
    s->uEcn = value;
  } else if (SB_STRCMP(name, "uprotocol") == 0) {
    s->uProtocol = value;
  } else if (SB_STRCMP(name, "ul4srcporthigh") == 0) {
    s->uL4SrcPortHigh = value;
  } else if (SB_STRCMP(name, "ul4srcportlow") == 0) {
    s->uL4SrcPortLow = value;
  } else if (SB_STRCMP(name, "ul4dstporthigh") == 0) {
    s->uL4DstPortHigh = value;
  } else if (SB_STRCMP(name, "ul4dstportlow") == 0) {
    s->uL4DstPortLow = value;
  } else if (SB_STRCMP(name, "uipv4sa") == 0) {
    s->uIpv4Sa = value;
  } else if (SB_STRCMP(name, "uipv4salen") == 0) {
    s->uIpv4SaLen = value;
  } else if (SB_STRCMP(name, "uipv4da") == 0) {
    s->uIpv4Da = value;
  } else if (SB_STRCMP(name, "uipv4dalen") == 0) {
    s->uIpv4DaLen = value;
  } else if (SB_STRCMP(name, "uethertype") == 0) {
    s->uEtherType = value;
  } else if (SB_STRCMP(name, "uusmac") == 0) {
    s->uuSmac = value;
  } else if (SB_STRCMP(name, "uusmaclen") == 0) {
    s->uuSmacLen = value;
  } else if (SB_STRCMP(name, "uvlan") == 0) {
    s->uVlan = value;
  } else if (SB_STRCMP(name, "uudmac") == 0) {
    s->uuDmac = value;
  } else if (SB_STRCMP(name, "uudmaclen") == 0) {
    s->uuDmacLen = value;
  } else if (SB_STRCMP(name, "uupbmp") == 0) {
    s->uuPbmp = value;
  } else if (SB_STRCMP(name, "enabled") == 0) {
    s->bEnabled = value;
  } else if (SB_STRCMP(name, "ussaphigh") == 0) {
    s->uLlcSsapHigh = value;
  } else if (SB_STRCMP(name, "ussaplow") == 0) {
    s->uLlcSsapLow = value;
  } else if (SB_STRCMP(name, "udsaphigh") == 0) {
    s->uLlcDsapHigh = value;
  }else if (SB_STRCMP(name, "udsaplow") == 0) {
    s->uLlcDsapLow = value;
  } else if (SB_STRCMP(name, "ullcctrlhigh") == 0) {
    s->uLlcCtrlHigh = value;
  } else if (SB_STRCMP(name, "ullcctrllow") == 0) {
    s->uLlcCtrlLow = value;
  }  else {
    /* string failed to match any field--ignored */
    return -1;
  }

  s->uLlcDsapSsapHigh = (s->uLlcDsapHigh << 8) | s->uLlcSsapHigh;
  s->uLlcDsapSsapLow  = (s->uLlcDsapLow  << 8) | s->uLlcSsapLow ;
  s->uLlcCtrlDataHigh = (s->uLlcCtrlHigh << 8) | 0xff;
  s->uLlcCtrlDataLow  = (s->uLlcCtrlLow  << 8) | 0x00;

  return(0);
}

void
soc_sbx_g2p3_cls_ace_pattern_init(soc_sbx_g2p3_cls_ace_pattern_t *pFrame)
{
  pFrame->bDbTypeAny =  (unsigned int)  0;
  pFrame->bPortAny =  (unsigned int)  0;
  pFrame->bPriorityAny =  (unsigned int)  0;
  pFrame->bFragmentAny =  (unsigned int)  0;
  pFrame->bTcpFlagURGAny =  (unsigned int)  0;
  pFrame->bTcpFlagACKAny =  (unsigned int)  0;
  pFrame->bTcpFlagPSHAny =  (unsigned int)  0;
  pFrame->bTcpFlagRSTAny =  (unsigned int)  0;
  pFrame->bTcpFlagSYNAny =  (unsigned int)  0;
  pFrame->bTcpFlagFINAny =  (unsigned int)  0;
  pFrame->bDscpAny =  (unsigned int)  0;
  pFrame->bEcnAny =  (unsigned int)  0;
  pFrame->bProtocolAny =  (unsigned int)  0;
  pFrame->bIpv4SaAny =  (unsigned int)  0;
  pFrame->bIpv4DaAny =  (unsigned int)  0;
  pFrame->bEtherTypeAny =  (unsigned int)  0;
  pFrame->bSmacAny =  (unsigned int)  0;
  pFrame->bVlanAny =  (unsigned int)  0;
  pFrame->bDmacAny =  (unsigned int)  0;
  pFrame->uDbType =  (unsigned int)  0;
  pFrame->uPort =  (unsigned int)  0;
  pFrame->uPriority =  (unsigned int)  0;
  pFrame->uFragment =  (unsigned int)  0;
  pFrame->uTcpFlagURG =  (unsigned int)  0;
  pFrame->uTcpFlagACK =  (unsigned int)  0;
  pFrame->uTcpFlagPSH =  (unsigned int)  0;
  pFrame->uTcpFlagRST =  (unsigned int)  0;
  pFrame->uTcpFlagSYN =  (unsigned int)  0;
  pFrame->uTcpFlagFIN =  (unsigned int)  0;
  pFrame->uDscp =  (unsigned int)  0;
  pFrame->uEcn =  (unsigned int)  0;
  pFrame->uProtocol =  (unsigned int)  0;
  pFrame->uL4SrcPortHigh =  (unsigned int)  0;
  pFrame->uL4SrcPortLow =  (unsigned int)  0;
  pFrame->uL4DstPortHigh =  (unsigned int)  0;
  pFrame->uL4DstPortLow =  (unsigned int)  0;
  pFrame->uIpv4Sa =  (unsigned int)  0;
  pFrame->uIpv4SaLen =  (unsigned int)  0;
  pFrame->uIpv4Da =  (unsigned int)  0;
  pFrame->uIpv4DaLen =  (unsigned int)  0;
  pFrame->uLlcDsapHigh =  (unsigned int)  0;
  pFrame->uLlcDsapLow =  (unsigned int)  0;
  pFrame->uLlcSsapHigh =  (unsigned int)  0;
  pFrame->uLlcSsapLow =  (unsigned int)  0;
  pFrame->bSsapAny =  (unsigned int)  0;
  pFrame->bDsapAny =  (unsigned int)  0;
  pFrame->uLlcCtrlHigh =  (unsigned int)  0;
  pFrame->uLlcCtrlLow =  (unsigned int)  0;
  pFrame->bLlcCtrlAny =  (unsigned int)  0;
  pFrame->uEtherType =  (unsigned int)  0;
  pFrame->uuSmac =  (uint64_t)  0;
  pFrame->uuSmacLen =  (unsigned int)  0;
  pFrame->uVlan =  (unsigned int)  0;
  pFrame->uuDmac =  (uint64_t)  0;
  pFrame->uuDmacLen =  (unsigned int)  0;
  pFrame->uuPbmp =  (uint64_t)  0;
  pFrame->bEnabled =  (unsigned int)  0;

  pFrame->uLlcDsapSsapHigh = (unsigned int)  0;
  pFrame->uLlcDsapSsapLow  = (unsigned int) 0;
  pFrame->uLlcCtrlDataHigh = (unsigned int) 0;
  pFrame->uLlcCtrlDataLow  = (unsigned int) 0;
}

int
soc_sbx_g2p3_cls_ace_pattern_ipv6_validate(soc_sbx_g2p3_cls_ace_pattern_ipv6_t *pZf)
{
  if (pZf->bDbTypeAny > 0x1) return 0;
  if (pZf->bPortAny > 0x1) return 0;
  if (pZf->bTcpFlagURGAny > 0x1) return 0;
  if (pZf->bTcpFlagACKAny > 0x1) return 0;
  if (pZf->bTcpFlagPSHAny > 0x1) return 0;
  if (pZf->bTcpFlagRSTAny > 0x1) return 0;
  if (pZf->bTcpFlagSYNAny > 0x1) return 0;
  if (pZf->bTcpFlagFINAny > 0x1) return 0;
  if (pZf->bTCAny > 0x1) return 0;
  if (pZf->bVlanAny > 0x1) return 0;
  if (pZf->uPort > 0x3f) return 0;
  if (pZf->uDbType > 0x3) return 0;
  if (pZf->uTcpFlagURG > 0x1) return 0;
  if (pZf->uTcpFlagACK > 0x1) return 0;
  if (pZf->uTcpFlagPSH > 0x1) return 0;
  if (pZf->uTcpFlagRST > 0x1) return 0;
  if (pZf->uTcpFlagSYN > 0x1) return 0;
  if (pZf->uTcpFlagFIN > 0x1) return 0;
  if (pZf->uTC > 0x3f) return 0;
 /* if (pZf->uNextHeader > 0xff) return 0; */
  if (pZf->uL4SrcPortHigh > 0xffff) return 0;
  if (pZf->uL4SrcPortLow > 0xffff) return 0;
  if (pZf->uL4DstPortHigh > 0xffff) return 0;
  if (pZf->uL4DstPortLow > 0xffff) return 0;
  if (pZf->uVlan > 0x3fff) return 0;
  if (pZf->bEnabled > 0x1) return 0;

  return 1; /* success */
}

int
soc_sbx_g2p3_cls_ace_pattern_ipv6_set(soc_sbx_g2p3_cls_ace_pattern_ipv6_t *s, char* name, int value)
{
  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "dbtypeany") == 0) {
    s->bDbTypeAny = value;
  } else if (SB_STRCMP(name, "portany") == 0) {
    s->bPortAny = value;
  } else if (SB_STRCMP(name, "tcpflagurgany") == 0) {
    s->bTcpFlagURGAny = value;
  } else if (SB_STRCMP(name, "tcpflagackany") == 0) {
    s->bTcpFlagACKAny = value;
  } else if (SB_STRCMP(name, "tcpflagpshany") == 0) {
    s->bTcpFlagPSHAny = value;
  } else if (SB_STRCMP(name, "tcpflagrstany") == 0) {
    s->bTcpFlagRSTAny = value;
  } else if (SB_STRCMP(name, "tcpflagsynany") == 0) {
    s->bTcpFlagSYNAny = value;
  } else if (SB_STRCMP(name, "tcpflagfinany") == 0) {
    s->bTcpFlagFINAny = value;
  } else if (SB_STRCMP(name, "tcany") == 0) {
    s->bTCAny = value;
  } else if (SB_STRCMP(name, "ipv6saany") == 0) {
    s->bIpv6SaAny = value;
  } else if (SB_STRCMP(name, "ipv6daany") == 0) {
    s->bIpv6DaAny = value;
  } else if (SB_STRCMP(name, "vlanany") == 0) {
    s->bVlanAny = value;
  } else if (SB_STRCMP(name, "udbtype") == 0) {
    s->uDbType = value;
  } else if (SB_STRCMP(name, "uport") == 0) {
    s->uPort = value;
  } else if (SB_STRCMP(name, "utcpflagurg") == 0) {
    s->uTcpFlagURG = value;
  } else if (SB_STRCMP(name, "utcpflagack") == 0) {
    s->uTcpFlagACK = value;
  } else if (SB_STRCMP(name, "utcpflagpsh") == 0) {
    s->uTcpFlagPSH = value;
  } else if (SB_STRCMP(name, "utcpflagrst") == 0) {
    s->uTcpFlagRST = value;
  } else if (SB_STRCMP(name, "utcpflagsyn") == 0) {
    s->uTcpFlagSYN = value;
  } else if (SB_STRCMP(name, "utcpflagfin") == 0) {
    s->uTcpFlagFIN = value;
  } else if (SB_STRCMP(name, "utc") == 0) {
    s->uTC = value;
  } else if (SB_STRCMP(name, "unextheaderhigh") == 0) {
    s->uNextHeaderHigh = value;
  } else if (SB_STRCMP(name, "unextheaderlow") == 0) {
    s->uNextHeaderLow = value;
  } else if (SB_STRCMP(name, "ul4srcporthigh") == 0) {
    s->uL4SrcPortHigh = value;
  } else if (SB_STRCMP(name, "ul4srcportlow") == 0) {
    s->uL4SrcPortLow = value;
  } else if (SB_STRCMP(name, "ul4dstporthigh") == 0) {
    s->uL4DstPortHigh = value;
  } else if (SB_STRCMP(name, "ul4dstportlow") == 0) {
    s->uL4DstPortLow = value;
  } else if (SB_STRCMP(name, "uuipv6sams") == 0) {
    s->uuIpv6SaMS = value;
  } else if (SB_STRCMP(name, "uuipv6sals") == 0) {
    s->uuIpv6SaLS = value;
  } else if (SB_STRCMP(name, "uuipv6salen") == 0) {
    s->uuIpv6SaLen = value;
  } else if (SB_STRCMP(name, "uuipv6dams") == 0) {
    s->uuIpv6DaMS = value;
  } else if (SB_STRCMP(name, "uuipv6dals") == 0) {
    s->uuIpv6DaLS = value;
  } else if (SB_STRCMP(name, "uuipv6dalen") == 0) {
    s->uuIpv6DaLen = value;
  } else if (SB_STRCMP(name, "uvlan") == 0) {
    s->uVlan = value;
  } else if (SB_STRCMP(name, "uupbmp") == 0) {
    s->uuPbmp = value;
  } else if (SB_STRCMP(name, "enabled") == 0) {
    s->bEnabled = value;
  }  else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);
}

void
soc_sbx_g2p3_cls_ace_pattern_ipv6_init(soc_sbx_g2p3_cls_ace_pattern_ipv6_t *pFrame)
{
  pFrame->bDbTypeAny =  (unsigned int)  0;
  pFrame->bPortAny =  (unsigned int)  0;
  pFrame->bTcpFlagURGAny =  (unsigned int)  0;
  pFrame->bTcpFlagACKAny =  (unsigned int)  0;
  pFrame->bTcpFlagPSHAny =  (unsigned int)  0;
  pFrame->bTcpFlagRSTAny =  (unsigned int)  0;
  pFrame->bTcpFlagSYNAny =  (unsigned int)  0;
  pFrame->bTcpFlagFINAny =  (unsigned int)  0;
  pFrame->bTCAny =  (unsigned int)  0;
  pFrame->bVlanAny =  (unsigned int)  0;
  pFrame->uDbType =  (unsigned int)  0;
  pFrame->uPort =  (unsigned int)  0;
  pFrame->uTcpFlagURG =  (unsigned int)  0;
  pFrame->uTcpFlagACK =  (unsigned int)  0;
  pFrame->uTcpFlagPSH =  (unsigned int)  0;
  pFrame->uTcpFlagRST =  (unsigned int)  0;
  pFrame->uTcpFlagSYN =  (unsigned int)  0;
  pFrame->uTcpFlagFIN =  (unsigned int)  0;
  pFrame->uTC =  (unsigned int)  0;
  pFrame->uNextHeaderHigh =  (unsigned int)  0;
  pFrame->uNextHeaderLow =  (unsigned int)  0;
  pFrame->uL4SrcPortHigh =  (unsigned int)  0;
  pFrame->uL4SrcPortLow =  (unsigned int)  0;
  pFrame->uL4DstPortHigh =  (unsigned int)  0;
  pFrame->uL4DstPortLow =  (unsigned int)  0;
  pFrame->uuIpv6SaMS = (unsigned int) 0;
  pFrame->uuIpv6SaLS = (unsigned int) 0;
  pFrame->uuIpv6SaLen = (unsigned int) 0;
  pFrame->uuIpv6DaMS = (unsigned int) 0;
  pFrame->uuIpv6DaLS = (unsigned int) 0;
  pFrame->uuIpv6DaLen = (unsigned int) 0;
  pFrame->uVlan =  (unsigned int)  0;
  pFrame->uuPbmp =  (unsigned int)  0;
  pFrame->bEnabled =  (unsigned int)  0;
}

/***********************/
/*** SOC INTERFACE *****/
/***********************/

/*** Static SOC functions ****/
static int
soc_sbx_g2p3_ace_translate_from_ifp(soc_sbx_g2p3_ifp_t *e,
                                    soc_sbx_g2p3_ace_rule_t *r)
{
    int i;

    r->eActionType = SB_G2P3_CLS_QOS;
    r->ePatternType = SB_G2P3_CLS_PATTERN;
    if (soc_sbx_g2p3_max_supported_stages(1) == SB_G2P3_CLS_MAX_C1) {
        r->uPattern.g2p3.uDbType = 0;
        r->uPattern.g2p3.bDbTypeAny = 1;
    } else {
        r->uPattern.g2p3.uDbType = 0;
        r->uPattern.g2p3.bDbTypeAny = 0;
    }

    r->uPattern.g2p3.uPort = e->port;
    r->uPattern.g2p3.bPortAny = !e->useport;
    r->uPattern.g2p3.uPriority = e->pri;
    r->uPattern.g2p3.bPriorityAny = !e->usepri;
    r->uPattern.g2p3.uFragment = e->fragment;
    r->uPattern.g2p3.bFragmentAny = !e->usefragment;
    r->uPattern.g2p3.uTcpFlagURG = e->urg;
    r->uPattern.g2p3.bTcpFlagURGAny = !e->useurg;
    r->uPattern.g2p3.uTcpFlagACK = e->ack;
    r->uPattern.g2p3.bTcpFlagACKAny = !e->useack;
    r->uPattern.g2p3.uTcpFlagPSH = e->psh;
    r->uPattern.g2p3.bTcpFlagPSHAny = !e->usepsh;
    r->uPattern.g2p3.uTcpFlagRST = e->rst;
    r->uPattern.g2p3.bTcpFlagRSTAny = !e->userst;
    r->uPattern.g2p3.uTcpFlagSYN = e->syn;
    r->uPattern.g2p3.bTcpFlagSYNAny = !e->usesyn;
    r->uPattern.g2p3.uTcpFlagFIN = e->fin;
    r->uPattern.g2p3.bTcpFlagFINAny = !e->usefin;
    r->uPattern.g2p3.uDscp = e->dscp;
    r->uPattern.g2p3.bDscpAny = !e->usedscp;
    r->uPattern.g2p3.uEcn = e->ecn;
    r->uPattern.g2p3.bEcnAny = !e->useecn;
    r->uPattern.g2p3.uProtocol = e->proto;
    r->uPattern.g2p3.bProtocolAny = !e->useproto;
    r->uPattern.g2p3.uIpv4Sa = e->sa;
    r->uPattern.g2p3.uIpv4SaLen = e->sawidth;
    r->uPattern.g2p3.bIpv4SaAny = 0;
    r->uPattern.g2p3.uIpv4Da = e->da;
    r->uPattern.g2p3.uIpv4DaLen = e->dawidth;
    r->uPattern.g2p3.bIpv4DaAny = 0;
    r->uPattern.g2p3.uL4SrcPortHigh = e->sporthi;
    r->uPattern.g2p3.uL4SrcPortLow = e->sportlo;
    r->uPattern.g2p3.uL4DstPortHigh = e->dporthi;
    r->uPattern.g2p3.uL4DstPortLow = e->dportlo;
    r->uPattern.g2p3.uEtherType = e->etype;
    /* LLC Fields when Ether Type == 0 */
    r->uPattern.g2p3.uLlcSsapHigh = e->ssaphi;
    r->uPattern.g2p3.uLlcSsapLow = e->ssaplo;
    r->uPattern.g2p3.bSsapAny = !e->usessap;
    r->uPattern.g2p3.uLlcDsapHigh = e->dsaphi;
    r->uPattern.g2p3.uLlcDsapLow = e->dsaplo;
    r->uPattern.g2p3.bDsapAny = !e->usedsap;
    r->uPattern.g2p3.uLlcDsapSsapHigh = (e->dsaphi << 8) | e->ssaphi;
    r->uPattern.g2p3.uLlcDsapSsapLow  = (e->dsaplo << 8) | e->ssaplo;
    r->uPattern.g2p3.uLlcCtrlHigh = e->llcctrlhi;
    r->uPattern.g2p3.uLlcCtrlLow = e->llcctrllo;
    r->uPattern.g2p3.bLlcCtrlAny = !e->usellcctrl;
    r->uPattern.g2p3.bEtherTypeAny = !e->useetype;
    r->uPattern.g2p3.uLlcCtrlDataHigh = (e->llcctrlhi << 8) | 0xff;
    r->uPattern.g2p3.uLlcCtrlDataLow  = (e->llcctrllo << 8) | 0x00;
    r->uPattern.g2p3.uuSmac = 0;
    for (i = 0; i < 6; i++) {
        r->uPattern.g2p3.uuSmac <<= 8;
        r->uPattern.g2p3.uuSmac |= e->smac[i];
    }
    r->uPattern.g2p3.bSmacAny = 0;
    r->uPattern.g2p3.uuSmacLen = e->smacwidth;
    r->uPattern.g2p3.uVlan = e->vid;
    r->uPattern.g2p3.bVlanAny = !e->usevid;
    r->uPattern.g2p3.uuDmac = 0;
    for (i = 0; i < 6; i++) {
        r->uPattern.g2p3.uuDmac <<= 8;
        r->uPattern.g2p3.uuDmac |= e->dmac[i];
    }
    r->uPattern.g2p3.bDmacAny = 0;
    r->uPattern.g2p3.uuDmacLen = e->dmacwidth;

    r->uPattern.g2p3.uuPbmp = 0;
    for (i = 0; i < 7; i++) {
        r->uPattern.g2p3.uuPbmp <<= 8;
        r->uPattern.g2p3.uuPbmp |= 0xFF;
        r->uPattern.g2p3.uuPbmp &= ~(e->pbmpn[i]);
    }
    r->uPattern.g2p3.bEnabled = e->enable;

    r->uAction.qos.vlan = e->vlan;
    r->uAction.qos.ftidx = e->ftidx;
    r->uAction.qos.mirror = e->mirror;
    r->uAction.qos.usevlan = e->usevlan;
    r->uAction.qos.useftidx = e->useftidx;
    r->uAction.qos.copy = e->copy;
    r->uAction.qos.dp = e->dp;
    r->uAction.qos.cos = e->cos;
    r->uAction.qos.fcos = e->fcos;
    r->uAction.qos.usedp = e->usedp;
    r->uAction.qos.usecos = e->usecos;
    r->uAction.qos.policer = e->policer;
    r->uAction.qos.typedpolice = e->typedpolice;
    r->uAction.qos.mefcos = e->mefcos;
    r->uAction.qos.mef = e->mef;

    return SOC_E_NONE;
}

static int
soc_sbx_g2p3_ace_translate_to_ifp(soc_sbx_g2p3_ifp_t *e,
                                  soc_sbx_g2p3_ace_rule_t *r)
{
    int i;

    e->dbtype = r->uPattern.g2p3.uDbType;
    e->usedbtype = !r->uPattern.g2p3.bDbTypeAny;
    e->port = r->uPattern.g2p3.uPort;
    e->useport = !r->uPattern.g2p3.bPortAny;
    e->pri = r->uPattern.g2p3.uPriority;
    e->usepri = !r->uPattern.g2p3.bPriorityAny;
    e->fragment = r->uPattern.g2p3.uFragment;
    e->usefragment = !r->uPattern.g2p3.bFragmentAny;
    e->urg = r->uPattern.g2p3.uTcpFlagURG;
    e->useurg = !r->uPattern.g2p3.bTcpFlagURGAny;
    e->ack = r->uPattern.g2p3.uTcpFlagACK;
    e->useack = !r->uPattern.g2p3.bTcpFlagACKAny;
    e->psh = r->uPattern.g2p3.uTcpFlagPSH;
    e->usepsh = !r->uPattern.g2p3.bTcpFlagPSHAny;
    e->rst = r->uPattern.g2p3.uTcpFlagRST;
    e->userst = !r->uPattern.g2p3.bTcpFlagRSTAny;
    e->syn = r->uPattern.g2p3.uTcpFlagSYN;
    e->usesyn = !r->uPattern.g2p3.bTcpFlagSYNAny;
    e->fin = r->uPattern.g2p3.uTcpFlagFIN;
    e->usefin = !r->uPattern.g2p3.bTcpFlagFINAny;
    e->dscp = r->uPattern.g2p3.uDscp;
    e->usedscp = !r->uPattern.g2p3.bDscpAny;
    e->ecn = r->uPattern.g2p3.uEcn;
    e->useecn = !r->uPattern.g2p3.bEcnAny;
    e->proto = r->uPattern.g2p3.uProtocol;
    e->useproto = !r->uPattern.g2p3.bProtocolAny;
    e->sa = r->uPattern.g2p3.uIpv4Sa;
    e->sawidth = r->uPattern.g2p3.uIpv4SaLen;
    e->da = r->uPattern.g2p3.uIpv4Da;
    e->dawidth = r->uPattern.g2p3.uIpv4DaLen;
    e->sporthi = r->uPattern.g2p3.uL4SrcPortHigh;
    e->sportlo = r->uPattern.g2p3.uL4SrcPortLow;
    e->dporthi = r->uPattern.g2p3.uL4DstPortHigh;
    e->dportlo = r->uPattern.g2p3.uL4DstPortLow;
    e->etype = r->uPattern.g2p3.uEtherType;
    e->useetype = !r->uPattern.g2p3.bEtherTypeAny;
    /* LLC Fields when Ether Type == 0 */
    e->ssaphi    = r->uPattern.g2p3.uLlcSsapHigh;
    e->ssaplo    = r->uPattern.g2p3.uLlcSsapLow;
    e->usessap = !r->uPattern.g2p3.bSsapAny;
    e->dsaphi    = r->uPattern.g2p3.uLlcDsapHigh;
    e->dsaplo    = r->uPattern.g2p3.uLlcDsapLow;
    e->usedsap = !r->uPattern.g2p3.bDsapAny;
    e->llcctrlhi = r->uPattern.g2p3.uLlcCtrlHigh;
    e->llcctrllo = r->uPattern.g2p3.uLlcCtrlLow;
    e->usellcctrl = !r->uPattern.g2p3.bLlcCtrlAny;
    for (i = 0; i < 6; i++) {
        e->smac[i] = r->uPattern.g2p3.uuSmac >> (8 * (5 - i));
    }
    for (i = 0; i < 6; i++) {
        e->dmac[i] = r->uPattern.g2p3.uuDmac >> (8 * (5 - i));
    }
    e->smacwidth = r->uPattern.g2p3.uuSmacLen;
    e->vid = r->uPattern.g2p3.uVlan;
    e->usevid = !r->uPattern.g2p3.bVlanAny;
    for (i = 0; i < 6; i++) {
        e->dmac[i] = r->uPattern.g2p3.uuDmac >> (8 * (5 - i));
    }
    e->dmacwidth = r->uPattern.g2p3.uuDmacLen;
    for (i = 0; i < 7; i++) {
        e->pbmpn[i] = ~(r->uPattern.g2p3.uuPbmp >> (8 * (6 - i)));
    }
    e->enable = r->uPattern.g2p3.bEnabled;

    e->vlan = r->uAction.qos.vlan;
    e->ftidx = r->uAction.qos.ftidx;
    e->mirror = r->uAction.qos.mirror;
    e->usevlan = r->uAction.qos.usevlan;
    e->useftidx = r->uAction.qos.useftidx;
    e->copy = r->uAction.qos.copy;
    e->dp = r->uAction.qos.dp;
    e->cos = r->uAction.qos.cos;
    e->fcos = r->uAction.qos.fcos;
    e->usedp = r->uAction.qos.usedp;
    e->usecos = r->uAction.qos.usecos;
    e->policer = r->uAction.qos.policer;
    e->typedpolice = r->uAction.qos.typedpolice;
    e->mefcos = r->uAction.qos.mefcos;
    e->mef = r->uAction.qos.mef;

    return SOC_E_NONE;
}



/*
 * efp
 */
static int
soc_sbx_g2p3_ace_translate_from_efp(soc_sbx_g2p3_efp_t *e,
                                    soc_sbx_g2p3_ace_rule_t *r)
{
    int i;

    r->eActionType = SB_G2P3_CLS_EGRESS;
    r->ePatternType = SB_G2P3_CLS_PATTERN;
    if (soc_sbx_g2p3_max_supported_stages(1) == SB_G2P3_CLS_MAX_C1) {
        r->uPattern.g2p3.uDbType = 0;
        r->uPattern.g2p3.bDbTypeAny = 1;
    } else {
        r->uPattern.g2p3.uDbType = 0;
        r->uPattern.g2p3.bDbTypeAny = 0;
    }
    r->uPattern.g2p3.uPort = e->port;
    r->uPattern.g2p3.bPortAny = !e->useport;
    r->uPattern.g2p3.uPriority = e->pri;
    r->uPattern.g2p3.bPriorityAny = !e->usepri;
    r->uPattern.g2p3.uFragment = e->fragment;
    r->uPattern.g2p3.bFragmentAny = !e->usefragment;
    r->uPattern.g2p3.uTcpFlagURG = e->urg;
    r->uPattern.g2p3.bTcpFlagURGAny = !e->useurg;
    r->uPattern.g2p3.uTcpFlagACK = e->ack;
    r->uPattern.g2p3.bTcpFlagACKAny = !e->useack;
    r->uPattern.g2p3.uTcpFlagPSH = e->psh;
    r->uPattern.g2p3.bTcpFlagPSHAny = !e->usepsh;
    r->uPattern.g2p3.uTcpFlagRST = e->rst;
    r->uPattern.g2p3.bTcpFlagRSTAny = !e->userst;
    r->uPattern.g2p3.uTcpFlagSYN = e->syn;
    r->uPattern.g2p3.bTcpFlagSYNAny = !e->usesyn;
    r->uPattern.g2p3.uTcpFlagFIN = e->fin;
    r->uPattern.g2p3.bTcpFlagFINAny = !e->usefin;
    r->uPattern.g2p3.uDscp = e->dscp;
    r->uPattern.g2p3.bDscpAny = !e->usedscp;
    r->uPattern.g2p3.uEcn = e->ecn;
    r->uPattern.g2p3.bEcnAny = !e->useecn;
    r->uPattern.g2p3.uProtocol = e->proto;
    r->uPattern.g2p3.bProtocolAny = !e->useproto;
    r->uPattern.g2p3.uIpv4Sa = e->sa;
    r->uPattern.g2p3.uIpv4SaLen = e->sawidth;
    r->uPattern.g2p3.bIpv4SaAny = 0;
    r->uPattern.g2p3.uIpv4Da = e->da;
    r->uPattern.g2p3.uIpv4DaLen = e->dawidth;
    r->uPattern.g2p3.bIpv4DaAny = 0;
    r->uPattern.g2p3.uL4SrcPortHigh = e->sporthi;
    r->uPattern.g2p3.uL4SrcPortLow = e->sportlo;
    r->uPattern.g2p3.uL4DstPortHigh = e->dporthi;
    r->uPattern.g2p3.uL4DstPortLow = e->dportlo;
    r->uPattern.g2p3.uEtherType = e->etype;
    r->uPattern.g2p3.bEtherTypeAny = !e->useetype;
    /* LLC Fields when Ether Type == 0 */
    r->uPattern.g2p3.uLlcSsapHigh = e->ssaphi;
    r->uPattern.g2p3.uLlcSsapLow = e->ssaplo;
    r->uPattern.g2p3.bSsapAny = !e->usessap;
    r->uPattern.g2p3.uLlcDsapHigh = e->dsaphi;
    r->uPattern.g2p3.uLlcDsapLow = e->dsaplo;
    r->uPattern.g2p3.bDsapAny = !e->usedsap;
    r->uPattern.g2p3.uLlcDsapSsapHigh = (e->dsaphi << 8) | e->ssaphi;
    r->uPattern.g2p3.uLlcDsapSsapLow  = (e->dsaplo << 8) | e->ssaplo;
    r->uPattern.g2p3.uLlcCtrlHigh = e->llcctrlhi;
    r->uPattern.g2p3.uLlcCtrlLow = e->llcctrllo;
    r->uPattern.g2p3.bLlcCtrlAny = !e->usellcctrl;
    r->uPattern.g2p3.uLlcCtrlDataHigh = (e->llcctrlhi << 8) | 0xff;
    r->uPattern.g2p3.uLlcCtrlDataLow  = (e->llcctrllo << 8) | 0x00;
    r->uPattern.g2p3.uuSmac = 0;
    for (i = 0; i < 6; i++) {
        r->uPattern.g2p3.uuSmac <<= 8;
        r->uPattern.g2p3.uuSmac |= e->smac[i];
    }
    r->uPattern.g2p3.bSmacAny = 0;
    r->uPattern.g2p3.uuSmacLen = e->smacwidth;
    r->uPattern.g2p3.uVlan = e->pvlan;
    r->uPattern.g2p3.bVlanAny = !e->usepvlan;
    r->uPattern.g2p3.uuDmac = 0;
    for (i = 0; i < 6; i++) {
        r->uPattern.g2p3.uuDmac <<= 8;
        r->uPattern.g2p3.uuDmac |= e->dmac[i];
    }
    r->uPattern.g2p3.bDmacAny = 0;
    r->uPattern.g2p3.uuDmacLen = e->dmacwidth;
    r->uPattern.g2p3.uuPbmp = 0;
    for (i = 0; i < 7; i++) {
        r->uPattern.g2p3.uuPbmp <<= 8;
        r->uPattern.g2p3.uuPbmp |= 0xFF;
        r->uPattern.g2p3.uuPbmp &= ~(e->pbmpn[i]);
    }
    r->uPattern.g2p3.bEnabled = e->enable;

    r->uAction.egress.mirror = e->mirror;
    r->uAction.egress.drop = e->drop;

    return SOC_E_NONE;
}

static int
soc_sbx_g2p3_ace_translate_to_efp(soc_sbx_g2p3_efp_t *e,
                                  soc_sbx_g2p3_ace_rule_t *r)
{
    int i;

    e->dbtype = r->uPattern.g2p3.uDbType;
    e->usedbtype = !r->uPattern.g2p3.bDbTypeAny;
    e->port = r->uPattern.g2p3.uPort;
    e->useport = !r->uPattern.g2p3.bPortAny;
    e->pri = r->uPattern.g2p3.uPriority;
    e->usepri = !r->uPattern.g2p3.bPriorityAny;
    e->fragment = r->uPattern.g2p3.uFragment;
    e->usefragment = !r->uPattern.g2p3.bFragmentAny;
    e->urg = r->uPattern.g2p3.uTcpFlagURG;
    e->useurg = !r->uPattern.g2p3.bTcpFlagURGAny;
    e->ack = r->uPattern.g2p3.uTcpFlagACK;
    e->useack = !r->uPattern.g2p3.bTcpFlagACKAny;
    e->psh = r->uPattern.g2p3.uTcpFlagPSH;
    e->usepsh = !r->uPattern.g2p3.bTcpFlagPSHAny;
    e->rst = r->uPattern.g2p3.uTcpFlagRST;
    e->userst = !r->uPattern.g2p3.bTcpFlagRSTAny;
    e->syn = r->uPattern.g2p3.uTcpFlagSYN;
    e->usesyn = !r->uPattern.g2p3.bTcpFlagSYNAny;
    e->fin = r->uPattern.g2p3.uTcpFlagFIN;
    e->usefin = !r->uPattern.g2p3.bTcpFlagFINAny;
    e->dscp = r->uPattern.g2p3.uDscp;
    e->usedscp = !r->uPattern.g2p3.bDscpAny;
    e->ecn = r->uPattern.g2p3.uEcn;
    e->useecn = !r->uPattern.g2p3.bEcnAny;
    e->proto = r->uPattern.g2p3.uProtocol;
    e->useproto = !r->uPattern.g2p3.bProtocolAny;
    e->sa = r->uPattern.g2p3.uIpv4Sa;
    e->sawidth = r->uPattern.g2p3.uIpv4SaLen;
    e->da = r->uPattern.g2p3.uIpv4Da;
    e->dawidth = r->uPattern.g2p3.uIpv4DaLen;
    e->sporthi = r->uPattern.g2p3.uL4SrcPortHigh;
    e->sportlo = r->uPattern.g2p3.uL4SrcPortLow;
    e->dporthi = r->uPattern.g2p3.uL4DstPortHigh;
    e->dportlo = r->uPattern.g2p3.uL4DstPortLow;
    e->etype = r->uPattern.g2p3.uEtherType;
    e->useetype = !r->uPattern.g2p3.bEtherTypeAny;
    /* LLC Fields when Ether Type == 0 */
    e->ssaphi    = r->uPattern.g2p3.uLlcSsapHigh;
    e->ssaplo    = r->uPattern.g2p3.uLlcSsapLow;
    e->usessap = !r->uPattern.g2p3.bSsapAny;
    e->dsaphi    = r->uPattern.g2p3.uLlcDsapHigh;
    e->dsaplo    = r->uPattern.g2p3.uLlcDsapLow;
    e->usedsap = !r->uPattern.g2p3.bDsapAny;
    e->llcctrlhi = r->uPattern.g2p3.uLlcCtrlHigh;
    e->llcctrllo = r->uPattern.g2p3.uLlcCtrlLow;
    e->usellcctrl = !r->uPattern.g2p3.bLlcCtrlAny;
    for (i = 0; i < 6; i++) {
        e->smac[i] = r->uPattern.g2p3.uuSmac >> (8 * (5 - i));
    }
    for (i = 0; i < 6; i++) {
        e->dmac[i] = r->uPattern.g2p3.uuDmac >> (8 * (5 - i));
    }
    e->smacwidth = r->uPattern.g2p3.uuSmacLen;
    e->pvlan = r->uPattern.g2p3.uVlan;
    e->usepvlan = !r->uPattern.g2p3.bVlanAny;
    for (i = 0; i < 6; i++) {
        e->dmac[i] = r->uPattern.g2p3.uuDmac >> (8 * (5 - i));
    }
    e->dmacwidth = r->uPattern.g2p3.uuDmacLen;
    for (i = 0; i < 7; i++) {
        e->pbmpn[i] = ~(r->uPattern.g2p3.uuPbmp >> (8 * (6 - i)));
    }
    e->enable = r->uPattern.g2p3.bEnabled;

    e->mirror = r->uAction.egress.mirror;
    e->drop = r->uAction.egress.drop;

    return SOC_E_NONE;
}

static int
soc_sbx_g2p3_ace_translate_from_ifp_v6(soc_sbx_g2p3_ifp_v6_t *e,
                                    soc_sbx_g2p3_ace_rule_t *r)
{
    int8_t i;
    r->eActionType = SB_G2P3_CLS_QOS;
    r->ePatternType = SB_G2P3_CLS_PATTERN_IPV6;
#if 0
    r->uPattern.g2p3_ipv6.uDbType = e->dbtype;
    r->uPattern.g2p3_ipv6.bDbTypeAny = !e->usedbtype;
#else
    r->uPattern.g2p3_ipv6.uDbType = 1;
    r->uPattern.g2p3_ipv6.bDbTypeAny = 0;
#endif
    r->uPattern.g2p3_ipv6.uPort = e->port;
    r->uPattern.g2p3_ipv6.bPortAny = !e->useport;
    r->uPattern.g2p3_ipv6.uTcpFlagURG = e->urg;
    r->uPattern.g2p3_ipv6.bTcpFlagURGAny = !e->useurg;
    r->uPattern.g2p3_ipv6.uTcpFlagACK = e->ack;
    r->uPattern.g2p3_ipv6.bTcpFlagACKAny = !e->useack;
    r->uPattern.g2p3_ipv6.uTcpFlagPSH = e->psh;
    r->uPattern.g2p3_ipv6.bTcpFlagPSHAny = !e->usepsh;
    r->uPattern.g2p3_ipv6.uTcpFlagRST = e->rst;
    r->uPattern.g2p3_ipv6.bTcpFlagRSTAny = !e->userst;
    r->uPattern.g2p3_ipv6.uTcpFlagSYN = e->syn;
    r->uPattern.g2p3_ipv6.bTcpFlagSYNAny = !e->usesyn;
    r->uPattern.g2p3_ipv6.uTcpFlagFIN = e->fin;
    r->uPattern.g2p3_ipv6.bTcpFlagFINAny = !e->usefin;
    r->uPattern.g2p3_ipv6.uTC = e->TC;
    r->uPattern.g2p3_ipv6.bTCAny = !e->useTC;
    r->uPattern.g2p3_ipv6.uNextHeaderHigh = e->nextheaderhi;
    r->uPattern.g2p3_ipv6.uNextHeaderLow = e->nextheaderlo;
    r->uPattern.g2p3_ipv6.uL4SrcPortHigh = e->sporthi;
    r->uPattern.g2p3_ipv6.uL4SrcPortLow = e->sportlo;
    r->uPattern.g2p3_ipv6.uL4DstPortHigh = e->dporthi;
    r->uPattern.g2p3_ipv6.uL4DstPortLow = e->dportlo;

    for (i = 0; i < 8; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6SaMS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6SaMS |= e->sa[i];
    }
    for (i = 8; i < 16; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6SaLS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6SaLS |= e->sa[i];
    }
    r->uPattern.g2p3_ipv6.uuIpv6SaLen = e->sawidth;

    for (i = 0; i < 8; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6DaMS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6DaMS |= e->da[i];
    }
    for (i = 8; i < 16; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6DaLS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6DaLS |= e->da[i];
    }
    r->uPattern.g2p3_ipv6.uuIpv6DaLen = e->dawidth;

    r->uPattern.g2p3_ipv6.uVlan = e->vid;
    r->uPattern.g2p3_ipv6.bVlanAny = !e->usevid;

    r->uPattern.g2p3_ipv6.uuPbmp = 0;
    for (i = 0; i < 7; i++) {
        r->uPattern.g2p3_ipv6.uuPbmp <<= 8;
        r->uPattern.g2p3_ipv6.uuPbmp |= 0xFF;
        r->uPattern.g2p3_ipv6.uuPbmp &= ~(e->pbmpn[i]);
    }

    r->uPattern.g2p3_ipv6.bEnabled = e->enable;

    r->uAction.qos.vlan = e->vlan;
    r->uAction.qos.ftidx = e->ftidx;
    r->uAction.qos.mirror = e->mirror;
    r->uAction.qos.usevlan = e->usevlan;
    r->uAction.qos.useftidx = e->useftidx;
    r->uAction.qos.copy = e->copy;
    r->uAction.qos.dp = e->dp;
    r->uAction.qos.cos = e->cos;
    r->uAction.qos.fcos = e->fcos;
    r->uAction.qos.usedp = e->usedp;
    r->uAction.qos.usecos = e->usecos;
    r->uAction.qos.policer = e->policer;
    r->uAction.qos.typedpolice = e->typedpolice;
    r->uAction.qos.mefcos = e->mefcos;
    r->uAction.qos.mef = e->mef;

    return SOC_E_NONE;
}

static int
soc_sbx_g2p3_ace_translate_to_ifp_v6(soc_sbx_g2p3_ifp_v6_t *e,
                                  soc_sbx_g2p3_ace_rule_t *r)
{
    int8_t i;
    e->dbtype = r->uPattern.g2p3_ipv6.uDbType;
    e->usedbtype = !r->uPattern.g2p3_ipv6.bDbTypeAny;
    e->port = r->uPattern.g2p3_ipv6.uPort;
    e->useport = !r->uPattern.g2p3_ipv6.bPortAny;
    e->urg = r->uPattern.g2p3_ipv6.uTcpFlagURG;
    e->useurg = !r->uPattern.g2p3_ipv6.bTcpFlagURGAny;
    e->ack = r->uPattern.g2p3_ipv6.uTcpFlagACK;
    e->useack = !r->uPattern.g2p3_ipv6.bTcpFlagACKAny;
    e->psh = r->uPattern.g2p3_ipv6.uTcpFlagPSH;
    e->usepsh = !r->uPattern.g2p3_ipv6.bTcpFlagPSHAny;
    e->rst = r->uPattern.g2p3_ipv6.uTcpFlagRST;
    e->userst = !r->uPattern.g2p3_ipv6.bTcpFlagRSTAny;
    e->syn = r->uPattern.g2p3_ipv6.uTcpFlagSYN;
    e->usesyn = !r->uPattern.g2p3_ipv6.bTcpFlagSYNAny;
    e->fin = r->uPattern.g2p3_ipv6.uTcpFlagFIN;
    e->usefin = !r->uPattern.g2p3_ipv6.bTcpFlagFINAny;
    e->TC = r->uPattern.g2p3_ipv6.uTC;
    e->useTC = !r->uPattern.g2p3_ipv6.bTCAny;
    e->nextheaderhi = r->uPattern.g2p3_ipv6.uNextHeaderHigh;
    e->nextheaderlo = r->uPattern.g2p3_ipv6.uNextHeaderLow;
    e->sporthi = r->uPattern.g2p3_ipv6.uL4SrcPortHigh;
    e->sportlo = r->uPattern.g2p3_ipv6.uL4SrcPortLow;
    e->dporthi = r->uPattern.g2p3_ipv6.uL4DstPortHigh;
    e->dportlo = r->uPattern.g2p3_ipv6.uL4DstPortLow;

    for (i = 0; i < 8; i++) {
        e->sa[i] = r->uPattern.g2p3_ipv6.uuIpv6SaMS >> (8 * (7 - i));
    }
    for (i = 8; i < 16; i++) {
        e->sa[i] = r->uPattern.g2p3_ipv6.uuIpv6SaLS >> (8 * (15 - i));
    }
    e->sawidth = r->uPattern.g2p3_ipv6.uuIpv6SaLen;

    for (i = 0; i < 8; i++) {
        e->da[i] = r->uPattern.g2p3_ipv6.uuIpv6DaMS >> (8 * (7 - i));
    }
    for (i = 8; i < 16; i++) {
        e->da[i] = r->uPattern.g2p3_ipv6.uuIpv6DaLS >> (8 * (15 - i));
    }
    e->dawidth = r->uPattern.g2p3_ipv6.uuIpv6DaLen;

    e->vid = r->uPattern.g2p3_ipv6.uVlan;
    e->usevid = !r->uPattern.g2p3_ipv6.bVlanAny;

    for (i = 0; i < 7; i++) {
        e->pbmpn[i] = ~(r->uPattern.g2p3_ipv6.uuPbmp >> (8 * (6 - i)));
        /*e->pbmpn[i] = r->uPattern.g2p3_ipv6.uuPbmp >> (8 * i);*/
    }
    e->enable = r->uPattern.g2p3_ipv6.bEnabled;

    e->vlan = r->uAction.qos.vlan;
    e->ftidx = r->uAction.qos.ftidx;
    e->mirror = r->uAction.qos.mirror;
    e->usevlan = r->uAction.qos.usevlan;
    e->useftidx = r->uAction.qos.useftidx;
    e->copy = r->uAction.qos.copy;
    e->dp = r->uAction.qos.dp;
    e->cos = r->uAction.qos.cos;
    e->fcos = r->uAction.qos.fcos;
    e->usedp = r->uAction.qos.usedp;
    e->usecos = r->uAction.qos.usecos;
    e->policer = r->uAction.qos.policer;
    e->typedpolice = r->uAction.qos.typedpolice;
    e->mefcos = r->uAction.qos.mefcos;
    e->mef = r->uAction.qos.mef;

    return SOC_E_NONE;
}

static int
soc_sbx_g2p3_ace_translate_from_efp_v6(soc_sbx_g2p3_efp_v6_t *e,
                                    soc_sbx_g2p3_ace_rule_t *r)
{
    int8_t i;
    r->eActionType = SB_G2P3_CLS_EGRESS;
    r->ePatternType = SB_G2P3_CLS_PATTERN_IPV6;
#if 0
    r->uPattern.g2p3_ipv6.uDbType = e->dbtype;
    r->uPattern.g2p3_ipv6.bDbTypeAny = !e->usedbtype;
#else
    r->uPattern.g2p3_ipv6.uDbType = 1;
    r->uPattern.g2p3_ipv6.bDbTypeAny = 0;
#endif
    r->uPattern.g2p3_ipv6.uPort = e->port;
    r->uPattern.g2p3_ipv6.bPortAny = !e->useport;
    r->uPattern.g2p3_ipv6.uTcpFlagURG = e->urg;
    r->uPattern.g2p3_ipv6.bTcpFlagURGAny = !e->useurg;
    r->uPattern.g2p3_ipv6.uTcpFlagACK = e->ack;
    r->uPattern.g2p3_ipv6.bTcpFlagACKAny = !e->useack;
    r->uPattern.g2p3_ipv6.uTcpFlagPSH = e->psh;
    r->uPattern.g2p3_ipv6.bTcpFlagPSHAny = !e->usepsh;
    r->uPattern.g2p3_ipv6.uTcpFlagRST = e->rst;
    r->uPattern.g2p3_ipv6.bTcpFlagRSTAny = !e->userst;
    r->uPattern.g2p3_ipv6.uTcpFlagSYN = e->syn;
    r->uPattern.g2p3_ipv6.bTcpFlagSYNAny = !e->usesyn;
    r->uPattern.g2p3_ipv6.uTcpFlagFIN = e->fin;
    r->uPattern.g2p3_ipv6.bTcpFlagFINAny = !e->usefin;
    r->uPattern.g2p3_ipv6.uTC = e->TC;
    r->uPattern.g2p3_ipv6.bTCAny = !e->useTC;
    r->uPattern.g2p3_ipv6.uNextHeaderHigh = e->nextheaderhi;
    r->uPattern.g2p3_ipv6.uNextHeaderLow = e->nextheaderlo;
    r->uPattern.g2p3_ipv6.uL4SrcPortHigh = e->sporthi;
    r->uPattern.g2p3_ipv6.uL4SrcPortLow = e->sportlo;
    r->uPattern.g2p3_ipv6.uL4DstPortHigh = e->dporthi;
    r->uPattern.g2p3_ipv6.uL4DstPortLow = e->dportlo;
    r->uPattern.g2p3_ipv6.uVlan = e->vid;
    r->uPattern.g2p3_ipv6.bVlanAny = !e->usevid;

    r->uPattern.g2p3_ipv6.uuPbmp = 0;
    for (i = 0; i < 7; i++) {
        r->uPattern.g2p3_ipv6.uuPbmp <<= 8;
        r->uPattern.g2p3_ipv6.uuPbmp |= 0xFF;
        r->uPattern.g2p3_ipv6.uuPbmp &= ~(e->pbmpn[i]);
    }

    for (i = 0; i < 8; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6SaMS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6SaMS |= e->sa[i];
    }
    for (i = 8; i < 16; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6SaLS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6SaLS |= e->sa[i];
    }
    r->uPattern.g2p3_ipv6.uuIpv6SaLen = e->sawidth;

    for (i = 0; i < 8; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6DaMS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6DaMS |= e->da[i];
    }
    for (i = 8; i < 16; i++) {
        r->uPattern.g2p3_ipv6.uuIpv6DaLS <<= 8;
        r->uPattern.g2p3_ipv6.uuIpv6DaLS |= e->da[i];
    }
    r->uPattern.g2p3_ipv6.uuIpv6DaLen = e->dawidth;

    r->uPattern.g2p3_ipv6.bEnabled = e->enable;

    r->uAction.egress.mirror = e->mirror;
    r->uAction.egress.drop = e->drop; 

    return SOC_E_NONE;
}

static int
soc_sbx_g2p3_ace_translate_to_efp_v6(soc_sbx_g2p3_efp_v6_t *e,
                                  soc_sbx_g2p3_ace_rule_t *r)
{
    int8_t i;

    e->dbtype = r->uPattern.g2p3_ipv6.uDbType;
    e->usedbtype = !r->uPattern.g2p3_ipv6.bDbTypeAny;
    e->port = r->uPattern.g2p3_ipv6.uPort;
    e->useport = !r->uPattern.g2p3_ipv6.bPortAny;
    e->urg = r->uPattern.g2p3_ipv6.uTcpFlagURG;
    e->useurg = !r->uPattern.g2p3_ipv6.bTcpFlagURGAny;
    e->ack = r->uPattern.g2p3_ipv6.uTcpFlagACK;
    e->useack = !r->uPattern.g2p3_ipv6.bTcpFlagACKAny;
    e->psh = r->uPattern.g2p3_ipv6.uTcpFlagPSH;
    e->usepsh = !r->uPattern.g2p3_ipv6.bTcpFlagPSHAny;
    e->rst = r->uPattern.g2p3_ipv6.uTcpFlagRST;
    e->userst = !r->uPattern.g2p3_ipv6.bTcpFlagRSTAny;
    e->syn = r->uPattern.g2p3_ipv6.uTcpFlagSYN;
    e->usesyn = !r->uPattern.g2p3_ipv6.bTcpFlagSYNAny;
    e->fin = r->uPattern.g2p3_ipv6.uTcpFlagFIN;
    e->usefin = !r->uPattern.g2p3_ipv6.bTcpFlagFINAny;
    e->TC = r->uPattern.g2p3_ipv6.uTC;
    e->useTC = !r->uPattern.g2p3_ipv6.bTCAny;
    e->nextheaderhi = r->uPattern.g2p3_ipv6.uNextHeaderHigh;
    e->nextheaderlo = r->uPattern.g2p3_ipv6.uNextHeaderLow;
    e->sporthi = r->uPattern.g2p3_ipv6.uL4SrcPortHigh;
    e->sportlo = r->uPattern.g2p3_ipv6.uL4SrcPortLow;
    e->dporthi = r->uPattern.g2p3_ipv6.uL4DstPortHigh;
    e->dportlo = r->uPattern.g2p3_ipv6.uL4DstPortLow;
    e->vid = r->uPattern.g2p3_ipv6.uVlan;
    e->usevid = !r->uPattern.g2p3_ipv6.bVlanAny;

    for (i = 0; i < 8; i++) {
        e->sa[i] = r->uPattern.g2p3_ipv6.uuIpv6SaMS >> (8 * (7 - i));
    }
    for (i = 8; i < 16; i++) {
        e->sa[i] = r->uPattern.g2p3_ipv6.uuIpv6SaLS >> (8 * (15 - i));
    }
    e->sawidth = r->uPattern.g2p3_ipv6.uuIpv6SaLen;

    for (i = 0; i < 8; i++) {
        e->da[i] = r->uPattern.g2p3_ipv6.uuIpv6DaMS >> (8 * (7 - i));
    }
    for (i = 8; i < 16; i++) {
        e->da[i] = r->uPattern.g2p3_ipv6.uuIpv6DaLS >> (8 * (15 - i));
    }
    e->dawidth = r->uPattern.g2p3_ipv6.uuIpv6DaLen;

    for (i = 0; i < 7; i++) {
        e->pbmpn[i] = ~(r->uPattern.g2p3_ipv6.uuPbmp >> (8 * (6 - i)));
        /*e->pbmpn[i] = r->uPattern.g2p3_ipv6.uuPbmp >> (8 * i);*/
    }
    e->enable = r->uPattern.g2p3_ipv6.bEnabled;

    e->mirror = r->uAction.egress.mirror;
    e->drop = r->uAction.egress.drop; 

    return SOC_E_NONE;
}


/****************************************/
/* RCE SOC layer Interface using Indices*/
/****************************************/
int soc_sbx_g2p3_efp_set_ext(int unit, int erule, soc_sbx_g2p3_efp_t *e)
{
    soc_sbx_g2p3_ace_rule_handle_t h;
    int rv;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS, 0, erule, &h);
    if (rv == SOC_E_NONE) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_update(unit, erule, e));
    } else if (rv == SOC_E_NOT_FOUND) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_add(unit, erule, e));
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_commit(unit, 0xffffffff));
        rv = SOC_E_NONE;
    }

    return rv;
}

int soc_sbx_g2p3_efp_add_ext(int unit, int erule, soc_sbx_g2p3_efp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS, 1, erule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_efp(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_EGRESS,
                          &r, &h0, h, 1, 1,
                          EFP_SOC_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}
static
int soc_sbx_g2p3_efp_add_sentinel(int unit, int erule, soc_sbx_g2p3_efp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS, 1, erule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_efp(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_EGRESS,
                          &r, &h0, 0, 0, 0,
                          EFP_DEF_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_efp_get_ext(int unit, int erule, soc_sbx_g2p3_efp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS,
                                                    0, erule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_from_handle(fe, SB_G2P3_CLS_EGRESS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_to_efp(e, &r));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_efp_update_ext(int unit, int erule, soc_sbx_g2p3_efp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS,
                                                    0, erule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_efp(e, &r));

    s = soc_sbx_g2p3_ace_rule_modify(fe, SB_G2P3_CLS_EGRESS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_commit(unit, 0xffffffff));

    return SOC_E_NONE;

}

int soc_sbx_g2p3_efp_delete_ext(int unit, int erule)
{
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_remove(unit, erule));
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_commit(unit, 0xffffffff));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_efp_remove_ext(int unit, int erule)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS,
                                                    0, erule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_remove(fe, h);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_efp_commit_ext(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_EGRESS);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int
soc_sbx_g2p3_efp_first_ext(int unit, int *nerule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS, 0, 0, &h);
    if (rv == SOC_E_NONE) {
        *nerule = 0;
    }
    return rv;
}

int
soc_sbx_g2p3_efp_next_ext(int unit, int erule, int *nerule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_EGRESS, 0, erule + 1, &h);
    if (rv == SOC_E_NONE) {
        *nerule = erule + 1;
    }
    return rv;
}

static int
soc_sbx_g2p3_fpctr_get_ext(int unit, soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            int clear, int cnum,
                            soc_sbx_g2p3_counter_value_t *v)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_mgr_t *pClsAce = fe->pClsAce;
    soc_sbx_g2p3_ace_db_t *pDb = NULL;
    soc_sbx_g2p3_ace_mgr_rule_t *pRule = NULL;
    soc_sbx_g2p3_counter_value_t cv;
    int rv;
    uint32_t oldRuleOffset = 0, ruleCount = 0, rulePresent = 0;

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    if (!pDb) {
        return SOC_E_INTERNAL;
    }

    ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
        /* !ACEDQ_IS_TAIL to avoid SENTINEL rule */
        if (ruleCount == cnum && !ACEDQ_IS_TAIL(&pDb->listRules, pRule)) {
            rulePresent = 1;
            break;
        }
        ruleCount++;
    } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);

    /* obtain counter offset for accumulation */
    if (SAND_HAL_IS_FE2KXT(fe->regSet)){
        oldRuleOffset = pRule->uRuleOffset;
    }else{
        soc_sbx_g2p3_cls_get_ctr_offset(fe, pDb->uProgramNumber,
                                    &oldRuleOffset);
    }

    COMPILER_64_ZERO(v->packets);
    COMPILER_64_ZERO(v->bytes);

    if (rulePresent == 0 || !soc_sbx_g2p3_ace_is_rule_enabled(&pRule->Rule)) {
        return SOC_E_NOT_FOUND;
    }

    if (pRule->bcommitted) {
        cnum = oldRuleOffset + pRule->uRuleIdx;
#ifdef COUNTER_DEBUG
        soc_cm_print("\n$$FP Rcount[%d] Ridx[%d] Offs[%d]", \
                     cnum, pRule->uRuleIdx,
                     oldRuleOffset);
#endif

    if (SAND_HAL_IS_FE2KXT(fe->regSet)){
        rv = soc_sbx_g2p3_rtctr_get(fe->unit, 1, cnum, &cv);
    } else {
        rv = (eDb == SB_G2P3_CLS_EGRESS)
           ? soc_sbx_g2p3_ertctr_get(fe->unit, 1, cnum, &cv)
           : soc_sbx_g2p3_irtctr_get(fe->unit, 1, cnum, &cv);
    }

        SB_ASSERT(rv == 0);

#ifdef COUNTER_DEBUG
        soc_cm_print("\n $$FP Present Rule Count pkt[%qx] byte[%qx]", \
                    pRule->counts.packets, pRule->counts.bytes);
        soc_cm_print("\n$$FP Counts got pkt[%qx] byte[%qx]",
                     cv.packets, cv.bytes);
#endif

        COMPILER_64_ADD_64(pRule->counts.packets, cv.packets);
        COMPILER_64_ADD_64(pRule->counts.bytes, cv.bytes);
        v->packets = pRule->counts.packets;
        v->bytes = pRule->counts.bytes;
        if (clear) {
            COMPILER_64_ZERO(pRule->counts.packets);
            COMPILER_64_ZERO(pRule->counts.bytes);
        }
    }

    return SOC_E_NONE;
}

static int
soc_sbx_g2p3_fpctr_handle_get_ext(int unit, soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            int clear, soc_sbx_g2p3_ace_mgr_rule_t *pRule,
                            soc_sbx_g2p3_counter_value_t *v)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_mgr_t *pClsAce = fe->pClsAce;
    soc_sbx_g2p3_ace_db_t *pDb = NULL;
    soc_sbx_g2p3_counter_value_t cv;
    int rv, ruleIdx=0;
    uint32_t oldRuleOffset = 0;

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    if (!pDb) {
        return SOC_E_INTERNAL;
    }

    COMPILER_64_ZERO(v->packets);
    COMPILER_64_ZERO(v->bytes);
    ruleIdx = pRule->uRuleIdx;

    if (!soc_sbx_g2p3_ace_is_rule_enabled(&pRule->Rule)) {
        return SOC_E_NOT_FOUND;
    }

    if (!SOC_IS_SBX_FE2KXT(unit)){
        /* obtain counter offset for accumulation */
        soc_sbx_g2p3_cls_get_ctr_offset(fe, pDb->uProgramNumber,
                                        &oldRuleOffset);
    }else{
        oldRuleOffset = pRule->uRuleOffset;
    }

    COMPILER_64_ZERO(v->packets);
    COMPILER_64_ZERO(v->bytes);

    if (pRule->bcommitted) {
        ruleIdx += oldRuleOffset;

        if (SOC_IS_SBX_FE2KXT(unit)){
            rv = soc_sbx_g2p3_rtctr_get(fe->unit, 1, ruleIdx, &cv);
        } else {
            if (eDb == SB_G2P3_CLS_QOS) {
                rv = soc_sbx_g2p3_irtctr_get(fe->unit, 1, ruleIdx, &cv);
            }else if (eDb == SB_G2P3_CLS_EGRESS){
                rv = soc_sbx_g2p3_ertctr_get(fe->unit, 1, ruleIdx, &cv);
            }else {
                SB_ASSERT(0);
                return SOC_E_INTERNAL;
            }
        }
        COMPILER_64_ADD_64(pRule->counts.packets, cv.packets);
        COMPILER_64_ADD_64(pRule->counts.bytes, cv.bytes);
        v->packets = pRule->counts.packets;
        v->bytes = pRule->counts.bytes;
        if (clear) {
            COMPILER_64_ZERO(pRule->counts.packets);
            COMPILER_64_ZERO(pRule->counts.bytes);
        }
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_ifpctr_get_ext(int unit, int clear, int cnum,
                            soc_sbx_g2p3_counter_value_t *v)
{
    return soc_sbx_g2p3_fpctr_get_ext(unit, SB_G2P3_CLS_QOS, clear, cnum, v);
}

int
soc_sbx_g2p3_efpctr_get_ext(int unit, int clear, int cnum,
                            soc_sbx_g2p3_counter_value_t *v)
{
    return soc_sbx_g2p3_fpctr_get_ext(unit, SB_G2P3_CLS_EGRESS, clear, cnum,
                                      v);
}

int
soc_sbx_g2p3_ifp_v6ctr_get_ext(int unit, int clear, int cnum,
                            soc_sbx_g2p3_counter_value_t *v)
{
    return soc_sbx_g2p3_fpctr_get_ext(unit, SB_G2P3_CLS_IPV6_QOS, clear, cnum, v);
}

int
soc_sbx_g2p3_efp_v6ctr_get_ext(int unit, int clear, int cnum,
                            soc_sbx_g2p3_counter_value_t *v)
{
    return soc_sbx_g2p3_fpctr_get_ext(unit, SB_G2P3_CLS_IPV6_EGRESS, clear, cnum,
                                      v);
}

int soc_sbx_g2p3_ifp_set_ext(int unit, int irule, soc_sbx_g2p3_ifp_t *e)
{
    soc_sbx_g2p3_ace_rule_handle_t h;
    int rv;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS, 0, irule, &h);
    if (rv == SOC_E_NONE) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_update(unit, irule, e));
    } else if (rv == SOC_E_NOT_FOUND) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_add(unit, irule, e));
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_commit(unit, 0xffffffff));
        rv = SOC_E_NONE;
    }

    return rv;
}


int soc_sbx_g2p3_ifp_add_ext(int unit, int irule, soc_sbx_g2p3_ifp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS, 1, irule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_ifp(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_QOS,
                             &r, &h0, h, 1, 1,
                             IFP_SOC_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}
static
int soc_sbx_g2p3_ifp_add_sentinel(int unit, int irule, soc_sbx_g2p3_ifp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS, 1, irule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_ifp(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_QOS,
                             &r, &h0, 0, 0, 0,
                             IFP_DEF_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_ifp_get_ext(int unit, int irule, soc_sbx_g2p3_ifp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS,
                                                    0, irule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_from_handle(fe, SB_G2P3_CLS_QOS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_to_ifp(e, &r));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_ifp_update_ext(int unit, int irule, soc_sbx_g2p3_ifp_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS,
                                                    0, irule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_ifp(e, &r));

    s = soc_sbx_g2p3_ace_rule_modify(fe, SB_G2P3_CLS_QOS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_commit(unit, 0xffffffff));

    return SOC_E_NONE;

}
int soc_sbx_g2p3_ifp_delete_ext(int unit, int irule)
{
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_remove(unit, irule));
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_commit(unit, 0xffffffff));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_ifp_remove_ext(int unit, int irule)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS,
                                                    0, irule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_remove(fe, h);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_ifp_commit_ext(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_QOS);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int
soc_sbx_g2p3_ifp_first_ext(int unit, int *nirule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS, 0, 0, &h);
    if (rv == SOC_E_NONE) {
        *nirule = 0;
    }
    return rv;
}

int
soc_sbx_g2p3_ifp_next_ext(int unit, int irule, int *nirule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_QOS, 0, irule + 1, &h);
    if (rv == SOC_E_NONE) {
        *nirule = irule + 1;
    }
    return rv;
}

/****************************************/
/* RCE SOC layer Interface using Handles*/
/****************************************/


/* Api to aid handle add based on rule priority */
static int
soc_sbx_g2p3_handle_get(uint32_t unit,
                       soc_sbx_g2p3_ace_rule_handle_t  *handle,
                       soc_sbx_g2p3_ace_rule_handle_t  predecessor,
                       soc_sbx_g2p3_cls_rule_db_e_t eDb,
                       uint8_t bHead, uint8_t bBefore)
{
    sbStatus_t     sb_status = SB_OK;
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);

    if(!handle){
        return SOC_E_PARAM;
    }

    if(!predecessor){
        if(bHead){
            sb_status = soc_sbx_g2p3_ace_rule_head_get(pFe, eDb, handle);
        } else {
            sb_status = soc_sbx_g2p3_ace_rule_tail_get(pFe, eDb, handle);
        }
    } else {
        if(bBefore){
            sb_status = soc_sbx_g2p3_ace_rule_prev_get(pFe, eDb,
                                                       predecessor,
                                                       handle);
        } else {
            sb_status = soc_sbx_g2p3_ace_rule_next_get(pFe, eDb,
                                                       predecessor,
                                                       handle);
        }
    }
    return(translate_sbx_result(sb_status));
}


static int
soc_sbx_g2p3_rule_modify(uint32_t unit,
                       soc_sbx_g2p3_ace_rule_handle_t  handle,
                       soc_sbx_g2p3_ace_rule_t         *pRule,
                       soc_sbx_g2p3_cls_rule_db_e_t eDb)
{
    sbStatus_t sb_status = SB_OK;
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);
    if(!pRule){
        return SOC_E_PARAM;
    }
    if(!handle){
        return SOC_E_PARAM;
    }

    sb_status = soc_sbx_g2p3_ace_rule_modify(pFe, eDb, handle, pRule);
    return(translate_sbx_result(sb_status));
}

static int
soc_sbx_g2p3_rule_from_handle(uint32_t unit,
                              soc_sbx_g2p3_cls_rule_db_e_t eDb,
                              soc_sbx_g2p3_ace_rule_handle_t handle,
                              soc_sbx_g2p3_ace_rule_t *pUserCopy)
{
    sbStatus_t sb_status = SB_OK;
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);
    if(!pUserCopy){
        return SOC_E_PARAM;
    }
    if(!handle){
        return SOC_E_PARAM;
    }

    sb_status =  soc_sbx_g2p3_ace_rule_from_handle(pFe, eDb, handle, pUserCopy);
    return(translate_sbx_result(sb_status));
}

static
int soc_sbx_g2p3_fp_clear_all_ext(uint32_t unit,
                                  soc_sbx_g2p3_cls_rule_db_e_t eDb)
{
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_mgr_t *pClsAce = pFe->pClsAce;
    soc_sbx_g2p3_ace_db_t *pDb = NULL;
    soc_sbx_g2p3_ace_mgr_rule_t *pRule = NULL;
    int  rv;

    /* find database */
    ACEDQ_TRAVERSE(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t) {
      if (eDb == pDb->eDbType)
        break;
    } ACEDQ_TRAVERSE_END(&pClsAce->listRuleDbs, pDb, soc_sbx_g2p3_ace_db_t);

    if (!pDb) {
        return SOC_E_INTERNAL;
    }

    /* Verify if the rule entry has to be MEMSET to 0 */
    ACEDQ_TRAVERSE(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t) {
         soc_sbx_g2p3_ace_rule_remove(pFe, pRule);
    } ACEDQ_TRAVERSE_END(&pDb->listRules, pRule, soc_sbx_g2p3_ace_mgr_rule_t);

    if(eDb == SB_G2P3_CLS_QOS){
      soc_sbx_g2p3_ifp_t ifp;
      soc_sbx_g2p3_ifp_t_init(&ifp);
      rv = soc_sbx_g2p3_ifp_add_sentinel(pFe->unit, 0, &ifp);
      if (rv != SOC_E_NONE) {
        return SB_OTHER_ERR_CODE;
      }
    } else if(eDb == SB_G2P3_CLS_EGRESS){
      soc_sbx_g2p3_efp_t efp;
      soc_sbx_g2p3_efp_t_init(&efp);
      rv = soc_sbx_g2p3_efp_add_sentinel(pFe->unit, 0, &efp);
      if (rv != SOC_E_NONE) {
        return SB_OTHER_ERR_CODE;
      }
    } else if(eDb == SB_G2P3_CLS_IPV6_QOS){
      soc_sbx_g2p3_ifp_v6_t ifp_v6;
      soc_sbx_g2p3_ifp_v6_t_init(&ifp_v6);
      rv = soc_sbx_g2p3_ifp_v6_add_sentinel(pFe->unit, 0, &ifp_v6);
      if (rv != SOC_E_NONE) {
        return SB_OTHER_ERR_CODE;
      }
    } else if(eDb == SB_G2P3_CLS_IPV6_EGRESS){
      soc_sbx_g2p3_efp_v6_t efp_v6;
      soc_sbx_g2p3_efp_v6_t_init(&efp_v6);
      rv = soc_sbx_g2p3_efp_v6_add_sentinel(pFe->unit, 0, &efp_v6);
      if (rv != SOC_E_NONE) {
        return SB_OTHER_ERR_CODE;
      }
    } else {
        SB_ASSERT(0);
    }

    /* Add sentinel rule back */
    return SOC_E_NONE;
}


/**
 * soc_sbx_g2p3_xfp_max_rule_capacity_get()
 *
 * @brief
 *
 * This function returns the maximum number of rules that FP
 * supports on the ingress or egress.
 * Sentinel Rule will be reserved on Soc layer (-1) on each program
 *
 * @param unit - FE unit number
 * @param eDb          - Rule database
 * @param pRuleCount(OUT) - rules supported on this database
 *
 * @return - SOC_E_NONE on success, error code otherwise
 **/
int
soc_sbx_g2p3_ifp_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount)
{

    /* G2P3 has only 1 Ruleset support in one RC instance */
    return (soc_sbx_g2p3_cls_rule_capacity_get(unit,
                                               SB_G2P3_FE_CLS_G2P3_SCHEMA,
                                               pRuleCount));
}

int
soc_sbx_g2p3_efp_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount)
{
    return (soc_sbx_g2p3_cls_rule_capacity_get(unit,
                                               SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA,
                                               pRuleCount));
}

int
soc_sbx_g2p3_ifp_v6_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount)
{

    /* G2P3 has only 1 Ruleset support in one RC instance */
    return (soc_sbx_g2p3_cls_rule_capacity_get(unit,
                                               SB_G2P3_FE_CLS_G2P3_IPV6_SCHEMA,
                                               pRuleCount));
}

int
soc_sbx_g2p3_efp_v6_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount)
{
    return (soc_sbx_g2p3_cls_rule_capacity_get(unit,
                                               SB_G2P3_FE_CLS_G2P3_IPV6_EGR_SCHEMA,
                                               pRuleCount));
}

/**
 * soc_sbx_g2p3_xfp_handle_after_add()
 *
 * @brief
 *
 * This function adds a rule to a xfp database
 *
 * @param unit - FE unit number
 * @param groupId - group identifier
 * @param predecessor handle and Rule Element - NULL will add to Head
 * @outparam - inserted rule handle
 *
 * @return - SOC_E_NONE on success, error code otherwise
 **/
int
soc_sbx_g2p3_ifp_handle_after_add(uint32_t unit,
                       int16_t uGroupId,
                       soc_sbx_g2p3_ifp_handle_t predecessor,
                       soc_sbx_g2p3_ifp_t *pRule,
                       soc_sbx_g2p3_ifp_handle_t *handle)
{
    uint8_t bAddHead = 0;
    soc_sbx_g2p3_ace_rule_t     aceRule;
    soc_sbx_g2p3_ace_rule_handle_t pIntHandle = NULL;
    soc_sbx_g2p3_ace_rule_handle_t pRefRuleHandle = NULL;
    sbStatus_t     sb_status = SB_OK;
    int            status = SOC_E_NONE;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);
    if(!pRule){
        return SOC_E_PARAM;
    }
    if(!handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_ifp(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    if(!predecessor){
        bAddHead = 1;
    } else {
        pRefRuleHandle = IFP_2_ACE_HDL_TYPECAST(predecessor);
    }

    sb_status = soc_sbx_g2p3_ace_rule_add(pFe, SB_G2P3_CLS_QOS, &aceRule,
                                          &pIntHandle, pRefRuleHandle,
                                          0, bAddHead, uGroupId);
    if(SB_OK == sb_status){

        *handle = ACE_HDL_2_IFP_TYPECAST(pIntHandle);
    }
#ifdef RULE_ADD_DBG
    soc_cm_print("\n Adding Ingress Rule Handle %p", handle);
#endif
    return(translate_sbx_result(sb_status));
}

int
soc_sbx_g2p3_efp_handle_after_add(uint32_t unit,
                       int16_t uGroupId,
                       soc_sbx_g2p3_efp_handle_t predecessor,
                       soc_sbx_g2p3_efp_t *pRule,
                       soc_sbx_g2p3_efp_handle_t *handle)
{
    uint8_t bAddHead = 0;
    soc_sbx_g2p3_ace_rule_t     aceRule;
    soc_sbx_g2p3_ace_rule_handle_t pIntHandle = NULL;
    soc_sbx_g2p3_ace_rule_handle_t pRefRuleHandle = NULL;
    sbStatus_t     sb_status = SB_OK;
    int            status = SOC_E_NONE;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);
    if(!pRule){
        return SOC_E_PARAM;
    }
    if(!handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_efp(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    if(!predecessor){
        bAddHead = 1;
    } else {
        pRefRuleHandle = EFP_2_ACE_HDL_TYPECAST(predecessor);
    }

    sb_status = soc_sbx_g2p3_ace_rule_add(pFe, SB_G2P3_CLS_EGRESS, &aceRule,
                                          &pIntHandle, pRefRuleHandle,
                                          0, bAddHead, uGroupId);
    if(SB_OK == sb_status){
        *handle = ACE_HDL_2_EFP_TYPECAST(pIntHandle);
    }
#ifdef RULE_ADD_DBG
    soc_cm_print("\n Adding Egress Rule Handle %p", handle);
#endif
    return(translate_sbx_result(sb_status));
}

int
soc_sbx_g2p3_ifp_v6_handle_after_add(uint32_t unit,
                       int16_t uGroupId,
                       soc_sbx_g2p3_ifp_v6_handle_t predecessor,
                       soc_sbx_g2p3_ifp_v6_t *pRule,
                       soc_sbx_g2p3_ifp_v6_handle_t *handle)

{
    uint8_t bAddHead = 0;
    soc_sbx_g2p3_ace_rule_t     aceRule;
    soc_sbx_g2p3_ace_rule_handle_t pIntHandle = NULL;
    soc_sbx_g2p3_ace_rule_handle_t pRefRuleHandle = NULL;
    sbStatus_t     sb_status = SB_OK;
    int            status = SOC_E_NONE;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);
    if(!pRule){
        return SOC_E_PARAM;
    }
    if(!handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_ifp_v6(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    if(!predecessor){
        bAddHead = 1;
    } else {
        pRefRuleHandle = IFP_V6_2_ACE_HDL_TYPECAST(predecessor);
    }

    sb_status = soc_sbx_g2p3_ace_rule_add(pFe, SB_G2P3_CLS_IPV6_QOS, &aceRule,
                                          &pIntHandle, pRefRuleHandle,
                                          0, bAddHead, uGroupId);
    if(SB_OK == sb_status){

        *handle = ACE_HDL_2_IFP_V6_TYPECAST(pIntHandle);
    }
#ifdef RULE_ADD_DBG
    soc_cm_print("\n Adding Ingress v6 Rule Handle %p", handle);
#endif
    return(translate_sbx_result(sb_status));
}

int
soc_sbx_g2p3_efp_v6_handle_after_add(uint32_t unit,
                       int16_t uGroupId,
                       soc_sbx_g2p3_efp_v6_handle_t predecessor,
                       soc_sbx_g2p3_efp_v6_t *pRule,
                       soc_sbx_g2p3_efp_v6_handle_t *handle)
{
    uint8_t bAddHead = 0;
    soc_sbx_g2p3_ace_rule_t     aceRule;
    soc_sbx_g2p3_ace_rule_handle_t pIntHandle = NULL;
    soc_sbx_g2p3_ace_rule_handle_t pRefRuleHandle = NULL;
    sbStatus_t     sb_status = SB_OK;
    int            status = SOC_E_NONE;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);
    if(!pRule){
        return SOC_E_PARAM;
    }
    if(!handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_efp_v6(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    if(!predecessor){
        bAddHead = 1;
    } else {
        pRefRuleHandle = EFP_2_ACE_HDL_TYPECAST(predecessor);
    }

    sb_status = soc_sbx_g2p3_ace_rule_add(pFe, SB_G2P3_CLS_IPV6_EGRESS,
                                          &aceRule,
                                          &pIntHandle, pRefRuleHandle,
                                          0, bAddHead, uGroupId);
    if(SB_OK == sb_status){
        *handle = ACE_HDL_2_EFP_V6_TYPECAST(pIntHandle);
    }
#ifdef RULE_ADD_DBG
    soc_cm_print("\n Adding Egress v6 Rule Handle %p", handle);
#endif
    return(translate_sbx_result(sb_status));
}

int
soc_sbx_g2p3_ifp_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   IFPP_2_ACE_HDLP_TYPECAST(handle), NULL,
                                   SB_G2P3_CLS_QOS, 1, 0));
}

int
soc_sbx_g2p3_efp_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   EFPP_2_ACE_HDLP_TYPECAST(handle), NULL,
                                   SB_G2P3_CLS_EGRESS, 1, 0));
}

int
soc_sbx_g2p3_ifp_v6_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_v6_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   IFP_V6P_2_ACE_HDLP_TYPECAST(handle), NULL,
                                   SB_G2P3_CLS_IPV6_QOS, 1, 0));
}

int
soc_sbx_g2p3_efp_v6_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_efp_v6_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   EFP_V6P_2_ACE_HDLP_TYPECAST(handle), NULL,
                                   SB_G2P3_CLS_IPV6_EGRESS, 1, 0));
}

int
soc_sbx_g2p3_ifp_handle_tail_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   IFPP_2_ACE_HDLP_TYPECAST(handle), NULL,
                                   SB_G2P3_CLS_QOS, 0, 0));
}

int
soc_sbx_g2p3_efp_handle_tail_get(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   EFPP_2_ACE_HDLP_TYPECAST(handle), NULL,
                                   SB_G2P3_CLS_EGRESS, 0, 0));
}


/* mostly similar to iterator functions */
int
soc_sbx_g2p3_ifp_handle_prev_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t  currentHandle,
                       soc_sbx_g2p3_ifp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   IFPP_2_ACE_HDLP_TYPECAST(handle), currentHandle,
                                   SB_G2P3_CLS_QOS, 0, 1));
}

int
soc_sbx_g2p3_efp_handle_prev_get(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t   currentHandle,
                       soc_sbx_g2p3_efp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   EFPP_2_ACE_HDLP_TYPECAST(handle), currentHandle,
                                   SB_G2P3_CLS_EGRESS, 0, 1));
}

int
soc_sbx_g2p3_ifp_handle_next_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t  currentHandle,
                       soc_sbx_g2p3_ifp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   IFPP_2_ACE_HDLP_TYPECAST(handle), currentHandle,
                                   SB_G2P3_CLS_QOS, 0, 0));
}

int
soc_sbx_g2p3_efp_handle_next_get(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t   currentHandle,
                       soc_sbx_g2p3_efp_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   EFPP_2_ACE_HDLP_TYPECAST(handle), currentHandle,
                                   SB_G2P3_CLS_EGRESS, 0, 0));
}

int
soc_sbx_g2p3_ifp_v6_handle_next_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_v6_handle_t  currentHandle,
                       soc_sbx_g2p3_ifp_v6_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   IFP_V6P_2_ACE_HDLP_TYPECAST(handle), currentHandle,
                                   SB_G2P3_CLS_IPV6_QOS, 0, 0));
}

int
soc_sbx_g2p3_efp_v6_handle_next_get(uint32_t unit,
                       soc_sbx_g2p3_efp_v6_handle_t   currentHandle,
                       soc_sbx_g2p3_efp_v6_handle_t *handle)
{
    return(soc_sbx_g2p3_handle_get(unit,
                                   EFP_V6P_2_ACE_HDLP_TYPECAST(handle), currentHandle,
                                   SB_G2P3_CLS_IPV6_EGRESS, 0, 0));
}

/**
 * soc_sbx_g2p3_xfp_rule_modify()
 *
 * @brief
 *
 * This function modifies a already existing rule in the
 * database identified by the rule handle and updates with
 * the latest update rule information. The big difference
 * between a rule add (see above ) vs this function is that
 * this one preserves the rule counts and carries them forward.
 * The user MUST call soc_sbx_g2p3_xfp_cls_commitRuleDb to make the rule
 * modification into effect.
 *
 * @param unit - FE unit number
 * @param rule handle and Rule Element
 *
 * @return - SOC_E_NONE on success, error code otherwise
 **/
int
soc_sbx_g2p3_ifp_handle_update(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t handle,
                       soc_sbx_g2p3_ifp_t *pRule)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pRule || !handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_ifp(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    return(soc_sbx_g2p3_rule_modify(unit,
                                    IFP_2_ACE_HDL_TYPECAST(handle),
                                    &aceRule,
                                    SB_G2P3_CLS_QOS));
}

int
soc_sbx_g2p3_efp_handle_update(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t handle,
                       soc_sbx_g2p3_efp_t *pRule)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pRule || !handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_efp(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    return(soc_sbx_g2p3_rule_modify(unit,
                                    EFP_2_ACE_HDL_TYPECAST(handle),
                                    &aceRule,
                                    SB_G2P3_CLS_EGRESS));
}

int
soc_sbx_g2p3_ifp_v6_handle_update(uint32_t unit,
                       soc_sbx_g2p3_ifp_v6_handle_t handle,
                       soc_sbx_g2p3_ifp_v6_t *pRule)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pRule || !handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_ifp_v6(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    return(soc_sbx_g2p3_rule_modify(unit,
                                    IFP_V6_2_ACE_HDL_TYPECAST(handle),
                                    &aceRule,
                                    SB_G2P3_CLS_IPV6_QOS));
}

int
soc_sbx_g2p3_efp_v6_handle_update(uint32_t unit,
                       soc_sbx_g2p3_efp_v6_handle_t handle,
                       soc_sbx_g2p3_efp_v6_t *pRule)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pRule || !handle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_ace_translate_from_efp_v6(pRule, &aceRule);
    if(SOC_E_NONE != status){
        return status;
    }

    return(soc_sbx_g2p3_rule_modify(unit,
                                    EFP_V6_2_ACE_HDL_TYPECAST(handle),
                                    &aceRule,
                                    SB_G2P3_CLS_IPV6_EGRESS));
}

/**
 * soc_sbx_g2p3_xfp_handle_remove()
 *
 * @brief
 *
 * Remove a rule/action pair from a set, given a pointer to the rule.
 *
 * @param unit - FE unit number
 * @param pRuleHandle - Reference to the rule to be removed
 *
 * @return - SOC_E_NONE on success,  error code otherwise
 **/
int
soc_sbx_g2p3_ifp_handle_remove(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t handle)
{
    sbStatus_t sb_status = SB_OK;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);

    if(!handle){
        return SOC_E_PARAM;
    }

#ifdef RULE_ADD_DBG
    soc_cm_print("\n Removing Ingress Rule Handle %p", handle);
#endif
    sb_status = soc_sbx_g2p3_ace_rule_remove( pFe, IFP_2_ACE_HDL_TYPECAST(handle));
    return(translate_sbx_result(sb_status));
}

int
soc_sbx_g2p3_efp_handle_remove(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t handle)
{
    sbStatus_t sb_status = SB_OK;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);

    if(!handle){
        return SOC_E_PARAM;
    }

#ifdef RULE_ADD_DBG
    soc_cm_print("\n Removing Egress Rule Handle %p", handle);
#endif
    sb_status = soc_sbx_g2p3_ace_rule_remove( pFe, EFP_2_ACE_HDL_TYPECAST(handle));
    return(translate_sbx_result(sb_status));
}

int
soc_sbx_g2p3_ifp_v6_handle_remove(uint32_t unit,
                       soc_sbx_g2p3_ifp_v6_handle_t handle)
{
    sbStatus_t sb_status = SB_OK;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);

    if(!handle){
        return SOC_E_PARAM;
    }

#ifdef RULE_ADD_DBG
    soc_cm_print("\n Removing Ingress V6 Rule Handle %p", handle);
#endif
    sb_status = soc_sbx_g2p3_ace_rule_remove( pFe, IFP_V6_2_ACE_HDL_TYPECAST(handle));
    return(translate_sbx_result(sb_status));
}

int
soc_sbx_g2p3_efp_v6_handle_remove(uint32_t unit,
                       soc_sbx_g2p3_efp_v6_handle_t handle)
{
    sbStatus_t sb_status = SB_OK;

    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    SB_ASSERT(pFe);

    if(!handle){
        return SOC_E_PARAM;
    }

#ifdef RULE_ADD_DBG
    soc_cm_print("\n Removing Egress V6 Rule Handle %p", handle);
#endif
    sb_status = soc_sbx_g2p3_ace_rule_remove( pFe, EFP_V6_2_ACE_HDL_TYPECAST(handle));
    return(translate_sbx_result(sb_status));
}

/**
 * soc_sbx_g2p3_xfp_rule_from_handle()
 *
 * @brief
 *
 * Given a classifier handle, returns the copy of the rule
 * given the user provided structure.
 *
 * @param unit - FE unit number
 * @param pHandle   -- rule handle
 * @param pUserCopy -- pointer to the user memory
 *
 * @return - pointer to rule described by handle
 **/
int
soc_sbx_g2p3_ifp_rule_from_handle(uint32_t unit,
                            soc_sbx_g2p3_ifp_handle_t pHandle,
                            soc_sbx_g2p3_ifp_t *pUserCopy)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pUserCopy || !pHandle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_rule_from_handle(unit,
                                         SB_G2P3_CLS_QOS,
                                         IFP_2_ACE_HDL_TYPECAST(pHandle),
                                         &aceRule);
    if(SOC_E_NONE == status){
        status = soc_sbx_g2p3_ace_translate_to_ifp(pUserCopy, &aceRule);
    }
    return status;
}

int
soc_sbx_g2p3_efp_rule_from_handle(uint32_t unit,
                            soc_sbx_g2p3_efp_handle_t pHandle,
                            soc_sbx_g2p3_efp_t *pUserCopy)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pUserCopy || !pHandle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_rule_from_handle(unit,
                                         SB_G2P3_CLS_EGRESS,
                                         EFP_2_ACE_HDL_TYPECAST(pHandle),
                                         &aceRule);
    if(SOC_E_NONE == status){
        status = soc_sbx_g2p3_ace_translate_to_efp(pUserCopy, &aceRule);
    }
    return status;

}

int
soc_sbx_g2p3_ifp_v6_rule_from_handle(uint32_t unit,
                            soc_sbx_g2p3_ifp_v6_handle_t pHandle,
                            soc_sbx_g2p3_ifp_v6_t *pUserCopy)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pUserCopy || !pHandle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_rule_from_handle(unit,
                                         SB_G2P3_CLS_IPV6_QOS,
                                         IFP_V6_2_ACE_HDL_TYPECAST(pHandle),
                                         &aceRule);
    if(SOC_E_NONE == status){
        status = soc_sbx_g2p3_ace_translate_to_ifp_v6(pUserCopy, &aceRule);
    }
    return status;
}

int
soc_sbx_g2p3_efp_v6_rule_from_handle(uint32_t unit,
                            soc_sbx_g2p3_efp_v6_handle_t pHandle,
                            soc_sbx_g2p3_efp_v6_t *pUserCopy)
{
    int            status = SOC_E_NONE;
    soc_sbx_g2p3_ace_rule_t     aceRule;

    if(!pUserCopy || !pHandle){
        return SOC_E_PARAM;
    }

    status = soc_sbx_g2p3_rule_from_handle(unit,
                                         SB_G2P3_CLS_IPV6_EGRESS,
                                         EFP_V6_2_ACE_HDL_TYPECAST(pHandle),
                                         &aceRule);
    if(SOC_E_NONE == status){
        status = soc_sbx_g2p3_ace_translate_to_efp_v6(pUserCopy, &aceRule);
    }
    return status;

}


/**
 *  soc_sbx_g2p3_xfp_counter_read()
 *
 * @brief
 *
 * This function reads the hit count of the rule given by the
 * Rule handle. This function finalizes the counters for the rule
 * handle and retuns the accumlated count. The bClear argument
 * clears the current accumlated count.
 *
 * @param unit - FE unit number
 * @param pHandle   -- rule handle
 * @param pCount    -- Pointer to the soc_sbx_g2p3_rule_count_t structure.
 * @param bClear    -- Force clear of the rule counts accumulation
 *
 * @return - SOC_E_NONE on success, error code otherwise
 **/
int
soc_sbx_g2p3_ifp_counter_read(uint32_t unit,
                        soc_sbx_g2p3_ifp_handle_t pHandle,
                        soc_sbx_g2p3_counter_value_t *pCount,
                        uint8_t bClear)
{
    soc_sbx_g2p3_ace_rule_handle_t pAceHandle = NULL;

    if(!pHandle){
        return SOC_E_PARAM;
    }

    pAceHandle = IFP_2_ACE_HDL_TYPECAST(pHandle);
    return soc_sbx_g2p3_fpctr_handle_get_ext(unit, SB_G2P3_CLS_QOS, bClear,
                                             pAceHandle, pCount);
}

int
soc_sbx_g2p3_efp_counter_read(uint32_t unit,
                        soc_sbx_g2p3_efp_handle_t pHandle,
                        soc_sbx_g2p3_counter_value_t *pCount,
                        uint8_t bClear)
{
    soc_sbx_g2p3_ace_rule_handle_t pAceHandle = NULL;

    if(!pHandle){
        return SOC_E_PARAM;
    }

    pAceHandle = EFP_2_ACE_HDL_TYPECAST(pHandle);
    return soc_sbx_g2p3_fpctr_handle_get_ext(unit, SB_G2P3_CLS_EGRESS, bClear,
                                             pAceHandle, pCount);
}

int
soc_sbx_g2p3_ifp_v6_counter_read(uint32_t unit,
                        soc_sbx_g2p3_ifp_v6_handle_t pHandle,
                        soc_sbx_g2p3_counter_value_t *pCount,
                        uint8_t bClear)
{
    soc_sbx_g2p3_ace_rule_handle_t pAceHandle = NULL;

    if(!pHandle){
        return SOC_E_PARAM;
    }

    pAceHandle = IFP_V6_2_ACE_HDL_TYPECAST(pHandle);
    return soc_sbx_g2p3_fpctr_handle_get_ext(unit, SB_G2P3_CLS_IPV6_QOS, bClear,
                                             pAceHandle, pCount);
}

int
soc_sbx_g2p3_efp_v6_counter_read(uint32_t unit,
                        soc_sbx_g2p3_efp_v6_handle_t pHandle,
                        soc_sbx_g2p3_counter_value_t *pCount,
                        uint8_t bClear)
{
    soc_sbx_g2p3_ace_rule_handle_t pAceHandle = NULL;

    if(!pHandle){
        return SOC_E_PARAM;
    }

    pAceHandle = EFP_V6_2_ACE_HDL_TYPECAST(pHandle);
    return soc_sbx_g2p3_fpctr_handle_get_ext(unit, SB_G2P3_CLS_IPV6_EGRESS, bClear,
                                             pAceHandle, pCount);
}

/**
 * soc_sbx_g2p3_xfp_clear_all()
 *
 * This function clears the rules in the database and the user
 * need to call CommitRuleDb to actually change the state
 * of the RCE hardware.
 *
 * @param unit - FE unit number
 * @return - SOC_E_NONE on success, error code otherwise
 **/
int
soc_sbx_g2p3_ifp_clear_all(uint32_t unit)
{

#ifdef RULE_ADD_DBG
    soc_cm_print("\n Clearing all Ingress rule");
#endif
    return soc_sbx_g2p3_fp_clear_all_ext(unit,SB_G2P3_CLS_QOS);
}

int
soc_sbx_g2p3_efp_clear_all(uint32_t unit)
{
#ifdef RULE_ADD_DBG
    soc_cm_print("\n Clearing all Egress rule");
#endif
    return soc_sbx_g2p3_fp_clear_all_ext(unit, SB_G2P3_CLS_EGRESS);
}

int
soc_sbx_g2p3_ifp_v6_clear_all(uint32_t unit)
{

#ifdef RULE_ADD_DBG
    soc_cm_print("\n Clearing all Ingress v6 rule");
#endif
    return soc_sbx_g2p3_fp_clear_all_ext(unit,SB_G2P3_CLS_IPV6_QOS);
}

int
soc_sbx_g2p3_efp_v6_clear_all(uint32_t unit)
{
#ifdef RULE_ADD_DBG
    soc_cm_print("\n Clearing all Egress v6 rule");
#endif
    return soc_sbx_g2p3_fp_clear_all_ext(unit, SB_G2P3_CLS_IPV6_EGRESS);
}


/**
 * @fn soc_sbx_g2p3_xfp_rule_handle_print()
 *
 * @brief
 *
 * Print a rule handle
 *
 * @param pRuleHandle - Reference to the rule handle to be printed
 *
 * @return - SOC_E_NONE on success, error code otherwise
 **/
int
soc_sbx_g2p3_ifp_rule_handle_print(int unit, soc_sbx_g2p3_ifp_handle_t *pRuleHandle)
{
  soc_sbx_control_t *sbx;
  sbx = SOC_SBX_CONTROL(unit);
  if (sbx == NULL) {
      return SOC_E_INIT;
  }
  soc_sbx_g2p3_ace_rule_handle_print(sbx->drv, IFP_2_ACE_HDL_TYPECAST(pRuleHandle));
  return SOC_E_NONE;
}

int
soc_sbx_g2p3_efp_rule_handle_print(int unit, soc_sbx_g2p3_efp_handle_t *pRuleHandle)
{
  soc_sbx_control_t *sbx;
  sbx = SOC_SBX_CONTROL(unit);
  if (sbx == NULL) {
      return SOC_E_INIT;
  }
  soc_sbx_g2p3_ace_rule_handle_print(sbx->drv, EFP_2_ACE_HDL_TYPECAST(pRuleHandle));
  return SOC_E_NONE;
}

/**
 * @fn soc_sbx_g2p3_ifp_handle_commit
 *
 * @brief
 * commit function intended only for IFP handle interface.
 *
 */
int soc_sbx_g2p3_ifp_handle_commit(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

#ifdef RULE_ADD_DBG
    soc_cm_print("\nCommit ifp ");
#endif

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_QOS);
    rv = soc_sbx_translate_status(s);
    return rv;
}


/**
 * @fn soc_sbx_g2p3_efp_handle_commit
 *
 * @brief
 * commit function inteded only for EFP handle interface.
 *
 */
int soc_sbx_g2p3_efp_handle_commit(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

#ifdef RULE_ADD_DBG
    soc_cm_print("\nCommit efp");
#endif

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_EGRESS);
    rv = soc_sbx_translate_status(s);
    return rv;
}

/**
 * @fn soc_sbx_g2p3_ifp_v6_handle_commit
 *
 * @brief
 * commit function inteded only for IFP_V6 handle interface.
 *
 */
int soc_sbx_g2p3_ifp_v6_handle_commit(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

#ifdef RULE_ADD_DBG
    soc_cm_print("\nCommit ifp ");
#endif

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_IPV6_QOS);
    rv = soc_sbx_translate_status(s);
    return rv;
}

/**
 * @fn soc_sbx_g2p3_efp_v6_handle_commit
 *
 * @brief
 * commit function inteded only for EFP_V6 handle interface.
 *
 */
int soc_sbx_g2p3_efp_v6_handle_commit(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

#ifdef RULE_ADD_DBG
    soc_cm_print("\nCommit efp_v6 ");
#endif

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_IPV6_EGRESS);
    rv = soc_sbx_translate_status(s);
    return rv;
}

int soc_sbx_g2p3_ifp_v6_set_ext(int unit, int irule, soc_sbx_g2p3_ifp_v6_t *e)
{
    soc_sbx_g2p3_ace_rule_handle_t h;
    int rv;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS, 0, irule, &h);
    if (rv == SOC_E_NONE) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_v6_update(unit, irule, e));
    } else if (rv == SOC_E_NOT_FOUND) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_v6_add(unit, irule, e));
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_v6_commit(unit, 0xffffffff));
        rv = SOC_E_NONE;
    }

    return rv;
}


int soc_sbx_g2p3_ifp_v6_add_ext(int unit, int irule, soc_sbx_g2p3_ifp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS, 1, irule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_ifp_v6(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_IPV6_QOS,
                             &r, &h0, h, 1, 1,
                             IFP_V6_SOC_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}
static
int soc_sbx_g2p3_ifp_v6_add_sentinel(int unit, int irule, soc_sbx_g2p3_ifp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS, 1, irule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_ifp_v6(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_IPV6_QOS,
                             &r, &h0, 0, 0, 0,
                             IFP_V6_DEF_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_ifp_v6_get_ext(int unit, int irule, soc_sbx_g2p3_ifp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS,
                                                    0, irule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_from_handle(fe, SB_G2P3_CLS_IPV6_QOS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_to_ifp_v6(e, &r));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_ifp_v6_update_ext(int unit, int irule, soc_sbx_g2p3_ifp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS,
                                                    0, irule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_ifp_v6(e, &r));

    s = soc_sbx_g2p3_ace_rule_modify(fe, SB_G2P3_CLS_IPV6_QOS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_v6_commit(unit, 0xffffffff));

    return SOC_E_NONE;

}

int soc_sbx_g2p3_ifp_v6_delete_ext(int unit, int irule)
{
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_v6_remove(unit, irule));
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ifp_v6_commit(unit, 0xffffffff));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_ifp_v6_remove_ext(int unit, int irule)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS,
                                                    0, irule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_remove(fe, h);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_ifp_v6_commit_ext(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_IPV6_QOS);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int
soc_sbx_g2p3_ifp_v6_first_ext(int unit, int *nirule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS, 0, 0, &h);
    if (rv == SOC_E_NONE) {
        *nirule = 0;
    }
    return rv;
}

int
soc_sbx_g2p3_ifp_v6_next_ext(int unit, int irule, int *nirule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_QOS, 0, irule + 1, &h);
    if (rv == SOC_E_NONE) {
        *nirule = irule + 1;
    }
    return rv;
}

int soc_sbx_g2p3_efp_v6_set_ext(int unit, int erule, soc_sbx_g2p3_efp_v6_t *e)
{
    soc_sbx_g2p3_ace_rule_handle_t h;
    int rv;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS, 0, erule, &h);
    if (rv == SOC_E_NONE) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_v6_update(unit, erule, e));
    } else if (rv == SOC_E_NOT_FOUND) {
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_v6_add(unit, erule, e));
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_v6_commit(unit, 0xffffffff));
        rv = SOC_E_NONE;
    }

    return rv;
}

int soc_sbx_g2p3_efp_v6_add_ext(int unit, int erule, soc_sbx_g2p3_efp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS, 1, erule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_efp_v6(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_IPV6_EGRESS,
                          &r, &h0, h, 1, 1,
                          EFP_V6_SOC_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}
static
int soc_sbx_g2p3_efp_v6_add_sentinel(int unit, int erule, soc_sbx_g2p3_efp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h, h0;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS, 1, erule, &h));

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_efp_v6(e, &r));

    s = soc_sbx_g2p3_ace_rule_add(fe, SB_G2P3_CLS_IPV6_EGRESS,
                          &r, &h0, 0, 0, 0,
                          EFP_V6_DEF_GROUP_IDX);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_efp_v6_get_ext(int unit, int erule, soc_sbx_g2p3_efp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS,
                                                    0, erule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_from_handle(fe, SB_G2P3_CLS_IPV6_EGRESS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_to_efp_v6(e, &r));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_efp_v6_update_ext(int unit, int erule, soc_sbx_g2p3_efp_v6_t *e)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS,
                                                    0, erule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_translate_from_efp_v6(e, &r));

    s = soc_sbx_g2p3_ace_rule_modify(fe, SB_G2P3_CLS_IPV6_EGRESS, h, &r);
    rv = soc_sbx_translate_status(s);
    if (rv != SOC_E_NONE) {
        return rv;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_v6_commit(unit, 0xffffffff));

    return SOC_E_NONE;

}

int soc_sbx_g2p3_efp_v6_delete_ext(int unit, int erule)
{
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_v6_remove(unit, erule));
    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_efp_v6_commit(unit, 0xffffffff));

    return SOC_E_NONE;
}

int soc_sbx_g2p3_efp_v6_remove_ext(int unit, int erule)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_ace_rule_handle_t h;
    sbStatus_t s;
    int rv;

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS,
                                                    0, erule, &h));
    if (!h) {
        return SOC_E_NOT_FOUND;
    }

    s = soc_sbx_g2p3_ace_rule_remove(fe, h);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int soc_sbx_g2p3_efp_v6_commit_ext(int unit, int runlength)
{
    soc_sbx_g2p3_state_t *fe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    sbStatus_t s;
    int rv;

    s = soc_sbx_g2p3_ace_commit_rule_db(fe, SB_G2P3_CLS_IPV6_EGRESS);
    rv = soc_sbx_translate_status(s);

    return rv;
}

int
soc_sbx_g2p3_efp_v6_first_ext(int unit, int *nerule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS, 0, 0, &h);
    if (rv == SOC_E_NONE) {
        *nerule = 0;
    }
    return rv;
}

int
soc_sbx_g2p3_efp_v6_next_ext(int unit, int erule, int *nerule)
{
    int rv;
    soc_sbx_g2p3_ace_rule_handle_t h;

    rv = soc_sbx_g2p3_ace_nth_handle(unit, SB_G2P3_CLS_IPV6_EGRESS, 0, erule + 1, &h);
    if (rv == SOC_E_NONE) {
        *nerule = erule + 1;
    }
    return rv;
}

int soc_sbx_g2p3_ert_wrap_set_ext(int unit,
                    int irule,
                    soc_sbx_g2p3_ert_wrap_t *e)
{
    soc_sbx_g2p3_rt_rr0_t rt_rr0;
    soc_sbx_g2p3_rt_rr1_t rt_rr1;
    soc_sbx_g2p3_ert_t    ert;
    int iset, ifilter, iruleentry;
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;

    pClsComp = pFe->pClsComp;

    iset = irule / G2P3_MAX_CLS_RULES_PER_RULESET_C2;
    ifilter = irule % G2P3_MAX_CLS_RULES_PER_RULESET_C2;

    iruleentry = iset * 1024 + ifilter;

    if (SAND_HAL_IS_FE2KXT(unit)) {
        if ( irule < (pClsComp->uMaxRs * G2P3_MAX_CLS_RULES_PER_RULESET_C2) ) {
            soc_sbx_g2p3_rt_rr0_t_init(&rt_rr0);
            rt_rr0.emirror = e->mirror;
            rt_rr0.drop   = e->drop;
            rt_rr0.rtidx  = irule;
            return soc_sbx_g2p3_rt_rr0_set(unit, iruleentry, &rt_rr0);
        } else {
            soc_sbx_g2p3_rt_rr1_t_init(&rt_rr1);
            rt_rr1.emirror = e->mirror;
            rt_rr1.drop   = e->drop;
            rt_rr1.rtidx  = irule;
            iruleentry = iruleentry - (1024 * pClsComp->uMaxRs);
            return soc_sbx_g2p3_rt_rr1_set(unit, iruleentry, &rt_rr1);
        }
    } else {
        soc_sbx_g2p3_ert_t_init(&ert);
        ert.mirror = e->mirror;
        ert.drop   = e->drop;
        return soc_sbx_g2p3_ert_set(unit, irule, &ert);
    }
}
     
int soc_sbx_g2p3_ert_wrap_get_ext(int unit,
                    int irule,
                    soc_sbx_g2p3_ert_wrap_t *e)
{
    soc_sbx_g2p3_rt_rr0_t rt_rr0;
    soc_sbx_g2p3_rt_rr1_t rt_rr1;
    soc_sbx_g2p3_ert_t    ert;
    int iset, ifilter, iruleentry;
    int ret;
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;

    pClsComp = pFe->pClsComp;

    iset = irule / G2P3_MAX_CLS_RULES_PER_RULESET_C2;
    ifilter = irule % G2P3_MAX_CLS_RULES_PER_RULESET_C2;
    iruleentry = iset * 1024 + ifilter;

    if (SAND_HAL_IS_FE2KXT(unit)) {
        if ( irule < (pClsComp->uMaxRs * G2P3_MAX_CLS_RULES_PER_RULESET_C2)) {
            soc_sbx_g2p3_rt_rr0_t_init(&rt_rr0);
            ret = soc_sbx_g2p3_rt_rr0_get(unit, iruleentry, &rt_rr0);
            if (ret !=0) return ret;

            e->mirror = rt_rr0.emirror;
            e->drop   = rt_rr0.drop;
            return ret;
        } else {
            soc_sbx_g2p3_rt_rr1_t_init(&rt_rr1);
            iruleentry = iruleentry - (1024 * pClsComp->uMaxRs);
            ret = soc_sbx_g2p3_rt_rr1_get(unit, iruleentry, &rt_rr1);
            if (ret !=0) return ret;

            e->mirror = rt_rr1.emirror;
            e->drop   = rt_rr1.drop;
            return ret;
        }
    } else {
        soc_sbx_g2p3_ert_t_init(&ert);
        ret = soc_sbx_g2p3_ert_get(unit, irule, &ert);

        e->mirror = ert.mirror;
        e->drop   = ert.drop;
        return ret;
    }
}
     
    

int soc_sbx_g2p3_irt_wrap_set_ext(int unit,
                    int irule,
                    soc_sbx_g2p3_irt_wrap_t *e)
{
    soc_sbx_g2p3_rt_rr0_t rt_rr0;
    soc_sbx_g2p3_rt_rr1_t rt_rr1;
    soc_sbx_g2p3_rt_mt_t rt_mt;
    soc_sbx_g2p3_irt_t irt;
    int irule1, iset, ifilter, iruleentry;
    int ret;
    soc_sbx_g2p3_state_t *pFe =
        (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;

    pClsComp = pFe->pClsComp;

    iset = irule / G2P3_MAX_CLS_RULES_PER_RULESET_C2;
    ifilter = irule % G2P3_MAX_CLS_RULES_PER_RULESET_C2;

    iruleentry = iset * 1024 + ifilter;

    if (SAND_HAL_IS_FE2KXT(unit)) {
        if ( irule < (pClsComp->uMaxRs * G2P3_MAX_CLS_RULES_PER_RULESET_C2)) {
            soc_sbx_g2p3_rt_rr0_t_init(&rt_rr0);
            rt_rr0.vlan     = e->vlan;
            rt_rr0.ftidx    = e->ftidx;
            rt_rr0.mirror   = e->mirror; 
            rt_rr0.usevlan  = e->usevlan;
            rt_rr0.useftidx = e->useftidx;
            rt_rr0.dp =e->dp;
            rt_rr0.usedp =e->usedp;
            rt_rr0.copy     = e->copy;

            rt_rr0.typedpolice  = e->typedpolice;
            rt_rr0.mefcos   = e->mefcos;
            rt_rr0.mef      = e->mef; 
            rt_rr0.cos  = e->cos;
            rt_rr0.fcos  = e->fcos;
            rt_rr0.usecos = e->usecos;

            ret = soc_sbx_g2p3_rt_rr0_set(unit, iruleentry, &rt_rr0);
            if (ret !=0) return ret;
        } else {
           soc_sbx_g2p3_rt_rr1_t_init(&rt_rr1);
            rt_rr1.vlan     = e->vlan;
            rt_rr1.ftidx    = e->ftidx;
            rt_rr1.mirror   = e->mirror; 
            rt_rr1.usevlan  = e->usevlan;
            rt_rr1.useftidx = e->useftidx;
            rt_rr1.copy     = e->copy;
            rt_rr1.dp =e->dp;
            rt_rr1.usedp =e->usedp;

            rt_rr1.typedpolice  = e->typedpolice;
            rt_rr1.mefcos   = e->mefcos;
            rt_rr1.mef      = e->mef;
            rt_rr1.cos  = e->cos;
            rt_rr1.fcos  = e->fcos;
            rt_rr1.usecos = e->usecos;

            /* use 1024 instead of the 768 entries per ruleset to account for 
              Set-SB-FilterSet indexing into the Results Resolver. */
            irule1 = iruleentry - (pClsComp->uMaxRs * 1024); 
            ret = soc_sbx_g2p3_rt_rr1_set(unit, irule1, &rt_rr1);
            if (ret !=0) return ret;
        }

        soc_sbx_g2p3_rt_mt_t_init(&rt_mt);
        rt_mt.policer = e->policer;
        ret = soc_sbx_g2p3_rt_mt_set(unit, irule, &rt_mt);
        if (ret !=0 ) return ret;
    } else {

        soc_sbx_g2p3_irt_t_init(&irt);
        irt.vlan     = e->vlan;
        irt.ftidx    = e->ftidx;
        irt.mirror   = e->mirror;
        irt.usevlan  = e->usevlan;
        irt.useftidx = e->useftidx;
        irt.copy     = e->copy;
        irt.dp       = e->dp;
        irt.cos      = e->cos;
        irt.fcos     = e->fcos;
        irt.usedp    = e->usedp;
        irt.usecos   = e->usecos;
        irt.policer  = e->policer;
        irt.typedpolice  = e->typedpolice;
        irt.mefcos   = e->mefcos;
        irt.mef      = e->mef;
        ret = soc_sbx_g2p3_irt_set(unit, irule, &irt);
        if (ret != 0) return ret;
    }
     return 0;
}

int soc_sbx_g2p3_irt_wrap_get_ext(int unit,
                    int irule,
                    soc_sbx_g2p3_irt_wrap_t *e)
{
    soc_sbx_g2p3_rt_rr0_t rt_rr0;
    soc_sbx_g2p3_rt_rr1_t rt_rr1;
    soc_sbx_g2p3_rt_mt_t rt_mt;
    soc_sbx_g2p3_irt_t irt;
    int irule1, iset, ifilter, iruleentry;
    int ret;
    soc_sbx_g2p3_state_t *pFe =
             (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
    soc_sbx_g2p3_cls_t *pClsComp = NULL;

    pClsComp = pFe->pClsComp;

    iset = irule / G2P3_MAX_CLS_RULES_PER_RULESET_C2;
    ifilter = irule % G2P3_MAX_CLS_RULES_PER_RULESET_C2;

    iruleentry = iset * 1024 + ifilter;

   if (SAND_HAL_IS_FE2KXT(unit)) {
        if ( irule < (pClsComp->uMaxRs * G2P3_MAX_CLS_RULES_PER_RULESET_C2)) {
            soc_sbx_g2p3_rt_rr0_t_init(&rt_rr0);
            ret = soc_sbx_g2p3_rt_rr0_get(unit, iruleentry, &rt_rr0);
            if (ret !=0) return ret;
            
            e->vlan        = rt_rr0.vlan;
            e->ftidx       = rt_rr0.ftidx;
            e->mirror      = rt_rr0.mirror; 
            e->usevlan     = rt_rr0.usevlan;
            e->useftidx    = rt_rr0.useftidx;
            e->copy        = rt_rr0.copy;
            e->typedpolice = rt_rr0.typedpolice;
            e->mefcos      = rt_rr0.mefcos;
            e->mef         = rt_rr0.mef;
        e->dp     = rt_rr0.dp;
        e->cos    = rt_rr0.cos;
        e->fcos    = rt_rr0.fcos;
        e->usedp  = rt_rr0.usedp;
        e->usecos = rt_rr0.usecos;
        } else {
            soc_sbx_g2p3_rt_rr1_t_init(&rt_rr1);
            /* use 1024 instead of the 768 entries per ruleset to account for 
              Set-SB-FilterSet indexing into the Results Resolver. */
            irule1 = iruleentry - (pClsComp->uMaxRs * 1024); 
            ret = soc_sbx_g2p3_rt_rr1_get(unit, irule1, &rt_rr1);
            if (ret !=0) return ret;
            
            e->vlan       = rt_rr1.vlan;
            e->ftidx      = rt_rr1.ftidx;
            e->mirror     = rt_rr1.mirror; 
            e->usevlan    = rt_rr1.usevlan;
            e->useftidx   = rt_rr1.useftidx;
            e->copy       = rt_rr1.copy;
            e->typedpolice = rt_rr1.typedpolice;
            e->mefcos     = rt_rr1.mefcos;
            e->mef        = rt_rr1.mef;

        e->dp     = rt_rr1.dp;
        e->cos    = rt_rr1.cos;
        e->fcos    = rt_rr1.fcos;
        e->usedp  = rt_rr1.usedp;
        e->usecos = rt_rr1.usecos;
        }

        soc_sbx_g2p3_rt_mt_t_init(&rt_mt);
        ret = soc_sbx_g2p3_rt_mt_get(unit, irule, &rt_mt);
        if (ret !=0) return ret;
        e->policer = rt_mt.policer;
    } else {

        soc_sbx_g2p3_irt_t_init(&irt);
        ret = soc_sbx_g2p3_irt_get(unit, irule, &irt);
        if (ret !=0) return ret;

        e->vlan        = irt.vlan;
        e->ftidx       = irt.ftidx;
        e->mirror      = irt.mirror;
        e->usevlan     = irt.usevlan;
        e->useftidx    = irt.useftidx;
        e->copy        = irt.copy;
        e->policer     = irt.policer;
        e->typedpolice = irt.typedpolice;
        e->mefcos      = irt.mefcos;
        e->mef         = irt.mef;
        e->dp          = irt.dp;
        e->cos         = irt.cos;
        e->fcos        = irt.fcos;
        e->usedp       = irt.usedp;
        e->usecos      = irt.usecos;
    }
    return ret;
}

int
soc_sbx_g2p3_ace_is_rule_enabled(soc_sbx_g2p3_ace_rule_t *pRule)
{
    if (pRule == NULL) {
        return 0;
    }
    switch(pRule->ePatternType) {
        case SB_G2P3_CLS_PATTERN:
        case SB_G2P3_CLS_PATTERN_LLC:
            return pRule->uPattern.g2p3.bEnabled;
        case SB_G2P3_CLS_PATTERN_IPV6:
            return pRule->uPattern.g2p3_ipv6.bEnabled;
        default:
            break;
    }
    return 0;
}

void *
soc_sbx_g2p3_ace_rule_pattern_get(soc_sbx_g2p3_ace_rule_t *pRule)
{
    if (pRule == NULL) {
        return NULL;
    }
    switch(pRule->ePatternType) {
        case SB_G2P3_CLS_PATTERN:
        case SB_G2P3_CLS_PATTERN_LLC:
            return (void *) &pRule->uPattern.g2p3;
        case SB_G2P3_CLS_PATTERN_IPV6:
            return (void *) &pRule->uPattern.g2p3_ipv6;
        default:
            break;
    }
    return NULL;
}

int soc_sbx_g2p3_max_supported_stages(int unit)
{
  uint8_t v6Enable;
  v6Enable = soc_property_get(unit, spn_IPV6_ENABLE, 0);
  if(!v6Enable) {
      return SB_G2P3_CLS_MAX_C1;
  } else {
      return SB_G2P3_CLS_MAX;
  }

}

/* ENHANCE_ME: separate CA/C2 rule recovery */
sbStatus_t
soc_sbx_g2p3_ace_wb_rule_recover(soc_sbx_g2p3_state_t *pFe,
                                       soc_sbx_g2p3_cls_schema_t *pSchema,
                                       uint32_t uGroupId,
                                       uint32_t uRule,
                                       soc_sbx_g2p3_cls_ace_pattern_t *pPattern)
{
    soc_sbx_g2p3_ace_rule_handle_t h;
    soc_sbx_g2p3_ace_rule_t r;
    int err = SB_OK;
    soc_sbx_g2p3_ifp_t ifp;
    soc_sbx_g2p3_efp_t efp;
    soc_sbx_g2p3_ifp_v6_t ifp_v6;
    soc_sbx_g2p3_efp_v6_t efp_v6;

    /* handle sentinel rules */
    switch (uGroupId) {
    case IFP_DEF_GROUP_IDX:
        soc_sbx_g2p3_ifp_t_init(&ifp);
        err = soc_sbx_g2p3_ifp_add_sentinel(pFe->unit, 0, &ifp);
        return err;
        break;
    case EFP_DEF_GROUP_IDX:
        soc_sbx_g2p3_efp_t_init(&efp);
        err = soc_sbx_g2p3_efp_add_sentinel(pFe->unit, 0, &efp);
        return err;
        break;
    case IFP_V6_DEF_GROUP_IDX:
        soc_sbx_g2p3_ifp_v6_t_init(&ifp_v6);
        err = soc_sbx_g2p3_ifp_v6_add_sentinel(pFe->unit, 0, &ifp_v6);
        return err;
        break;
    case EFP_V6_DEF_GROUP_IDX:
        soc_sbx_g2p3_efp_v6_t_init(&efp_v6);
        err = soc_sbx_g2p3_efp_v6_add_sentinel(pFe->unit, 0, &efp_v6);
        return err;
        break;
    default:
        /* fall thru */
        break;
    }

    sal_memset(&r, 0, sizeof(soc_sbx_g2p3_ace_rule_t));

    sal_memcpy(&r.uPattern.g2p3, pPattern, sizeof(soc_sbx_g2p3_cls_ace_pattern_t));

    if ( !(r.uPattern.g2p3.bEtherTypeAny) &&
         !(r.uPattern.g2p3.uEtherType) ) {
        r.ePatternType = SB_G2P3_CLS_PATTERN_LLC;
    }else{
        r.ePatternType = SB_G2P3_CLS_PATTERN;
    }

    if ( !SAND_HAL_IS_FE2KXT(pFe->regSet) ) {
        if (pSchema->sType == SB_G2P3_FE_CLS_G2P3_SCHEMA) {
            uGroupId = IFP_SOC_GROUP_IDX;
        }else{
            uGroupId = EFP_SOC_GROUP_IDX;
        }
    }

    switch (pSchema->sType) {
    case SB_G2P3_FE_CLS_G2P3_SCHEMA:
        r.eActionType = SB_G2P3_CLS_QOS;
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_irt_wrap_get(pFe->unit, uRule, &r.uAction.qos));
        err = soc_sbx_g2p3_ace_rule_add(pFe, SB_G2P3_CLS_QOS,
                                 &r, &h, NULL, 0, 0,
                                 uGroupId);
        break;
    case SB_G2P3_FE_CLS_G2P3_EGR_SCHEMA:
        r.eActionType = SB_G2P3_CLS_EGRESS;
        SOC_IF_ERROR_RETURN
            (soc_sbx_g2p3_ert_wrap_get(pFe->unit, uRule, &r.uAction.egress));
        err = soc_sbx_g2p3_ace_rule_add(pFe, SB_G2P3_CLS_EGRESS,
                                 &r, &h, NULL, 0, 0,
                                 uGroupId);
        break;
    default:
        SB_ASSERT(0);
        return SOC_E_INTERNAL;
    }


#if 0
soc_cm_print("Recovered rule 0x%x group 0x%x\n", uRule, uGroupId);
    if (err == SB_OK) {
        soc_sbx_g2p3_ace_rule_handle_print(pFe, h);
    }
#endif
    return err;
}
