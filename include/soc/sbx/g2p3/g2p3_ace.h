#ifndef _SB_G2P3_FE_CLS_ACE_H_
#define _SB_G2P3_FE_CLS_ACE_H_
/* --------------------------------------------------------------------------
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
 * $Id: g2p3_ace.h 1.36.52.1 Broadcom SDK $
 * soc_sbx_g2p3_cls_ace.h : Classifier Access Control Entry interface
 *
 * --------------------------------------------------------------------------*/
/* common headers */
#include <soc/sbx/g2p3/g2p3_cnt.h>
#include <shared/bitop.h>

/* --------------------------------------------------------------------------
 * Some basic terms to make the comments more coherent :
 * 
 * Pattern - A collection of fields in the packet to perform matching on
 * Action  - The behavior microcode will take as a result of pattern match
 * Rule    - The combination of "Pattern + Action"
 * RuleSet - A collection of related rules, with implied ordering between 
 *           rules. The rules with the RuleSet can be managed and re-arranged
 *           within the RuleSet without affecting other RuleSets. RuleSets can 
 *           be enabled/disabled as a whole. Rulesets have an implied type
 *           (Security, Qos), and are collected into databases.
 * RuleDb  - Rule Database, a collection of RuleSets, with implied ordering
 *           between them. For Gu2k, there are essentially two databases
 *           (Security, Qos).
 * --------------------------------------------------------------------------*/
typedef enum {
  SB_G2P3_CLS_QOS = 0,
  SB_G2P3_CLS_EGRESS,
  SB_G2P3_CLS_MAX_C1,  
  SB_G2P3_CLS_IPV6_QOS = 2,
  SB_G2P3_CLS_IPV6_EGRESS,
  /* leave as last */
  SB_G2P3_CLS_MAX,  
} soc_sbx_g2p3_cls_rule_db_e_t;

#define SB_G2P3_CLS_MAX_STAGES_SUPPORTED 4

typedef enum {
  SB_MAX_USER_GROUPS = 16,
/* XXX_SOC_GROUP_IDX used by unit tests to add rules */
  IFP_SOC_GROUP_IDX = SB_MAX_USER_GROUPS,
  EFP_SOC_GROUP_IDX,
  IFP_V6_SOC_GROUP_IDX,
  EFP_V6_SOC_GROUP_IDX,
/* XXX_DEF_GROUP_IDX used to have sentinel rules */
  IFP_DEF_GROUP_IDX,
  EFP_DEF_GROUP_IDX,
  IFP_V6_DEF_GROUP_IDX,
  EFP_V6_DEF_GROUP_IDX,
  SB_MAX_GROUPS,
} soc_sbx_g2p3_cls_ace_internal_groups_e_t;

typedef enum {
/* XXX_SOC_GROUP_PRI used by unit tests to add rules */
  IFP_SOC_GROUP_PRI = -1,
  EFP_SOC_GROUP_PRI = -2,
  IFP_V6_SOC_GROUP_PRI = -3,
  EFP_V6_SOC_GROUP_PRI = -4,
/* XXX_DEF_GROUP_PRI used to add sentinel rules */
  IFP_DEF_GROUP_PRI = -5,
  EFP_DEF_GROUP_PRI = -6,
  IFP_V6_DEF_GROUP_PRI = -7,
  EFP_V6_DEF_GROUP_PRI = -8,
} soc_sbx_g2p3_cls_ace_internal_groups_pri_e_t;

/*
 * Enumeration of different types of patterns that can be matched against
 */
typedef enum {
  SB_G2P3_CLS_PATTERN =0,
  SB_G2P3_CLS_PATTERN_LLC =1,
  SB_G2P3_CLS_PATTERN_IPV6 =2,
  /* leave as last */
  SB_G2P3_CLS_PATTERN_MAX,
} soc_sbx_g2p3_cls_ace_pattern_types_e_t;

typedef struct  {
  uint8_t bDbTypeAny;
  uint8_t bPortAny;
  uint8_t bPriorityAny;
  uint8_t bFragmentAny;
  uint8_t bTcpFlagURGAny;
  uint8_t bTcpFlagACKAny;
  uint8_t bTcpFlagPSHAny;
  uint8_t bTcpFlagRSTAny;
  uint8_t bTcpFlagSYNAny;
  uint8_t bTcpFlagFINAny;
  uint8_t bDscpAny;
  uint8_t bEcnAny;
  uint8_t bProtocolAny;
  uint8_t bIpv4SaAny;
  uint8_t bIpv4DaAny;
  uint8_t bEtherTypeAny;
  uint8_t bSmacAny;
  uint8_t bVlanAny;
  uint8_t bDmacAny;
  uint32_t uDbType;
  uint32_t uPort;
  uint64_t uuPbmp;
  uint32_t uPriority;
  uint32_t uFragment;
  uint32_t uTcpFlagURG;
  uint32_t uTcpFlagACK;
  uint32_t uTcpFlagPSH;
  uint32_t uTcpFlagRST;
  uint32_t uTcpFlagSYN;
  uint32_t uTcpFlagFIN;
  uint32_t uDscp;
  uint32_t uEcn;
  uint32_t uProtocol;
  uint32_t uL4SrcPortHigh;
  uint32_t uL4SrcPortLow;
  uint32_t uL4DstPortHigh;
  uint32_t uL4DstPortLow;
  /* LLC Fields when Ether Type == 0 */
  uint32_t uLlcSsapHigh;
  uint32_t uLlcSsapLow;
  uint8_t  bSsapAny;
  uint32_t uLlcDsapHigh;
  uint32_t uLlcDsapLow;
  uint8_t  bDsapAny;
  uint32_t uLlcDsapSsapHigh;
  uint32_t uLlcDsapSsapLow;
  uint32_t uLlcCtrlHigh;
  uint32_t uLlcCtrlLow;
  uint8_t  bLlcCtrlAny;
  uint32_t uLlcCtrlDataHigh;
  uint32_t uLlcCtrlDataLow;
  uint32_t uIpv4Sa;
  uint32_t uIpv4SaLen;
  uint32_t uIpv4Da;
  uint32_t uIpv4DaLen;
  uint32_t uEtherType;
  uint64_t uuSmac;
  uint32_t uuSmacLen;
  uint32_t uVlan;
  uint64_t uuDmac;
  uint32_t uuDmacLen;
  uint8_t bEnabled;
} soc_sbx_g2p3_cls_ace_pattern_t;

typedef struct  {
  uint8_t bDbTypeAny;
  uint8_t bPortAny;
  uint8_t bTcpFlagURGAny;
  uint8_t bTcpFlagACKAny;
  uint8_t bTcpFlagPSHAny;
  uint8_t bTcpFlagRSTAny;
  uint8_t bTcpFlagSYNAny;
  uint8_t bTcpFlagFINAny;
  uint8_t bTCAny;
  uint8_t bIpv6SaAny;
  uint8_t bIpv6DaAny;
  uint8_t bEtherTypeAny;
  uint8_t bVlanAny;
  uint32_t uDbType;
  uint32_t uPort;
  uint64_t uuPbmp;
  uint32_t uTcpFlagURG;
  uint32_t uTcpFlagACK;
  uint32_t uTcpFlagPSH;
  uint32_t uTcpFlagRST;
  uint32_t uTcpFlagSYN;
  uint32_t uTcpFlagFIN;
  uint32_t uTC;
  uint32_t uNextHeaderHigh;
  uint32_t uNextHeaderLow;
  uint32_t uL4SrcPortHigh;
  uint32_t uL4SrcPortLow;
  uint32_t uL4DstPortHigh;
  uint32_t uL4DstPortLow;
  uint64_t uuIpv6SaMS;
  uint64_t uuIpv6SaLS;
  uint32_t uuIpv6SaLen;
  uint64_t uuIpv6DaMS;
  uint64_t uuIpv6DaLS;
  uint32_t uuIpv6DaLen;
  uint32_t uVlan;
  uint8_t bEnabled;
} soc_sbx_g2p3_cls_ace_pattern_ipv6_t;

typedef union {
    soc_sbx_g2p3_irt_wrap_t      qos;
    soc_sbx_g2p3_ert_wrap_t   egress;
} soc_sbx_g2p3_ace_action_u_t;

typedef union {
    soc_sbx_g2p3_cls_ace_pattern_t    g2p3;  /* SB_G2P3_CLS_PATTERN */
    soc_sbx_g2p3_cls_ace_pattern_ipv6_t    g2p3_ipv6;  /* SB_G2P3_CLS_PATTERN */
} soc_sbx_g2p3_cls_ace_pattern_u_t;  

typedef struct soc_sbx_g2p3_fp_rule_s {
  soc_sbx_g2p3_cls_ace_pattern_types_e_t  ePatternType;
  soc_sbx_g2p3_cls_ace_pattern_u_t        uPattern; /*union*/
  soc_sbx_g2p3_cls_rule_db_e_t            eActionType;
  soc_sbx_g2p3_ace_action_u_t             uAction;     /*union*/
} soc_sbx_g2p3_ace_rule_t;

typedef enum soc_sbx_g2p3_ace_qualify_e {
    sbxFieldQualifySrcIp6,
    sbxFieldQualifyDstIp6,
    sbxFieldQualifySrcMac,
    sbxFieldQualifyDstMac,
    sbxFieldQualifySnap,
    sbxFieldQualifySrcIp,
    sbxFieldQualifyDstIp,
    sbxFieldQualifyLlc,
    sbxFieldQualifyPorts,
    sbxFieldQualifyVlanId,
    sbxFieldQualifyVlanPri,
    sbxFieldQualifyL4SrcPort,
    sbxFieldQualifyL4DstPort,
    sbxFieldQualifyEtherType,
    sbxFieldQualifyIpProtocol,
    sbxFieldQualifyIp6NextHeader = sbxFieldQualifyIpProtocol,
    sbxFieldQualifyDSCP,
    sbxFieldQualifyTos = sbxFieldQualifyDSCP,
    sbxFieldQualifyECN,
    sbxFieldQualifyTcpControl,
    sbxFieldQualifyIpFrag,
    sbxFieldQualifyDbType,
    sbxFieldQualifyCount
} soc_sbx_g2p3_ace_qualify_e_t;

#define SBX_FIELD_QUALIFY_MAX sbxFieldQualifyCount

typedef struct soc_sbx_g2p3_ace_qset_s {
  SHR_BITDCL w[_SHR_BITDCLSIZE(SBX_FIELD_QUALIFY_MAX)];
} soc_sbx_g2p3_ace_qset_t;

#define SBX_FIELD_QSET_TEST(qset, q) SHR_BITGET(((qset).w), (q))
#define SBX_FIELD_QSET_ADD(qset, q) SHR_BITSET(((qset).w), (q))



#define IFP_2_ACE_HDL_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t)((void*)ptr))
#define EFP_2_ACE_HDL_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t)((void*)ptr))
#define IFP_V6_2_ACE_HDL_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t)((void*)ptr))
#define EFP_V6_2_ACE_HDL_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t)((void*)ptr))

#define ACE_HDL_2_IFP_TYPECAST(ptr) ((soc_sbx_g2p3_ifp_handle_t)((void*)ptr))
#define ACE_HDL_2_EFP_TYPECAST(ptr) ((soc_sbx_g2p3_efp_handle_t)((void*)ptr))
#define ACE_HDL_2_IFP_V6_TYPECAST(ptr) ((soc_sbx_g2p3_ifp_v6_handle_t)((void*)ptr))
#define ACE_HDL_2_EFP_V6_TYPECAST(ptr) ((soc_sbx_g2p3_efp_v6_handle_t)((void*)ptr))

#define IFPP_2_ACE_HDLP_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t*)((void**)ptr))
#define EFPP_2_ACE_HDLP_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t*)((void**)ptr))
#define IFP_V6P_2_ACE_HDLP_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t*)((void**)ptr))
#define EFP_V6P_2_ACE_HDLP_TYPECAST(ptr) ((soc_sbx_g2p3_ace_rule_handle_t*)((void**)ptr))

/* 
 * Forward declaration of a module-local types. This allows use of strongly
 * typed pointers (handles) to the internal structures, while hiding the 
 * implementation inside the .c file. This only permits the use of pointers
 * to reference structures, and implicitly prevents de-referencing the data.
 */
typedef struct soc_sbx_g2p3_ace_mgr_s       soc_sbx_g2p3_ace_mgr_t;
typedef struct soc_sbx_g2p3_ace_mgr_rule_s  soc_sbx_g2p3_ace_mgr_rule_t;
typedef        soc_sbx_g2p3_ace_mgr_rule_t *soc_sbx_g2p3_ace_rule_handle_t;

/**
 * soc_sbx_g2p3_ace_clear_all()
 *
 * This function clears the rules in the database and the user
 * need to call sbG2feClsCommitRuleDb to actually change the state
 * of the RCE hardware.
 *
 * @param pFe - driver control structure
 * @param eDb - Which rule database clear... 
 *           	SB_G2P3_CLS_SECURITY = Clear out Security rules
 *           	SB_G2P3_CLS_QOS      = Clear out QOS rules
 *           	SB_G2P3_CLS_MAX      = Clear out both Security *and* QOS rules.
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_clear_all(soc_sbx_g2p3_state_t *pFe,
                           soc_sbx_g2p3_cls_rule_db_e_t eDb);

/**
 *  soc_sbx_g2p3_ace_rule_count_get()
 *
 * @brief 
 *
 * This  function find the number of rules currently configured in the
 * database given by eDb
 *
 * @param pFe - driver control structure
 * @param eDb - Which rule database clear... 
 *           	SB_G2P3_CLS_SECURITY = Clear out Security rules
 *           	SB_G2P3_CLS_QOS      = Clear out QOS rules
 *           	SB_G2P3_CLS_MAX      = Clear out both Security *and* QOS rules.
 * @param pCount -- Count of the number of Rules in this database
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_count_get(soc_sbx_g2p3_state_t *pFe,
                                soc_sbx_g2p3_cls_rule_db_e_t eDb,
                                uint32_t *pCount);
/**
 * soc_sbx_g2p3_ace_rule_addHead()
 *
 * @brief 
 *
 * This function adds a rule to a database, inserting it at the
 * head of the list. This function returns a handle to the newly 
 * created rule.
 *
 * @param pFe 	       - Driver control structure
 * @param eDb          - Rule database to modify
 * @param pRule        - Rule pattern/action to add
 * @param ppRuleHandle - Returns handle to new rule
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_addHead(soc_sbx_g2p3_state_t *pFe, 
                              soc_sbx_g2p3_cls_rule_db_e_t eDb,
                              soc_sbx_g2p3_ace_rule_t *pRule,
                              soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle);

/**
 * soc_sbx_g2p3_ace_rule_modify()
 *
 * @brief 
 *
 * This function modifies a already existing rule in the 
 * database identified by the rule handle and updates with 
 * the latest update rule information. The big difference
 * between a rule add (see above ) vs this function sis that
 * this one preserves the rule counts and carries them forward.
 * The user MUST call soc_sbx_g2p3_cls_commitRuleDb to make the rule
 * modification into effect.
 *
 * @param pFe 	       - Driver control structure
 * @param eDb          - Rule database to modify
 * @param pRuleHandle  - Returns handle to new rule
 * @param pRule        - Rule pattern/action to add
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_modify(soc_sbx_g2p3_state_t *pFe, 
                       soc_sbx_g2p3_cls_rule_db_e_t eDb,
                       soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
                       soc_sbx_g2p3_ace_rule_t *pRule );

/**
 * soc_sbx_g2p3_ace_rule_addTail()
 *
 * @brief 
 *
 * This function adds a rule to a rule-set, inserting it at the
 * tail of the list. This function returns a handle to the newly 
 * created rule.
 *
 * @param pFe 	       - Driver control structure
 * @param eDb          - Rule database to modify
 * @param pRule        - Rule pattern/action to add
 * @param ppRuleHandle - Returns handle to new rule
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_addTail(soc_sbx_g2p3_state_t *pFe, 
                        soc_sbx_g2p3_cls_rule_db_e_t eDb,
                        soc_sbx_g2p3_ace_rule_t *pRule,
                        soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle);


/**
 * soc_sbx_g2p3_ace_rule_insert_after()
 *
 * @brief 
 *
 * This function adds a rule to a rule-set, inserting it after
 * another (specified) element in the list. This function returns
 * a handle to the newly created rule.
 *
 * @param pFe 	       - Driver control structure
 * @param eDb          - Rule database to modify
 * @param pRule        - Rule pattern/action to add
 * @param ppRuleHandle - Returns handle to new rule
 * @param pRefRule     - Handle to rule to insert after in list
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_insert_after(soc_sbx_g2p3_state_t *pFe, 
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            soc_sbx_g2p3_ace_rule_t *pRule,
                            soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle,
                            soc_sbx_g2p3_ace_rule_handle_t  pRefRule);

/**
 * soc_sbx_g2p3_ace_rule_next_get()
 *
 * @brief 
 *
 * This function returns the next rule following the rule given by 
 * the position of the rule handle passed in.
 *
 * @param pFe 	       - Driver control structure
 * @param eDb          - Rule database to modify
 * @param pRefRule     - Handle to rule of the reference pointer
 * @param ppRuleHandle - Returns handle to the rule next to reference rule
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_next_get(soc_sbx_g2p3_state_t *pFe, 
                        soc_sbx_g2p3_cls_rule_db_e_t eDb,
                        soc_sbx_g2p3_ace_rule_handle_t  pRefRule,
                        soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle);

/**
 * soc_sbx_g2p3_ace_rule_prev_get()
 *
 * @brief 
 *
 * This function returns the  rule previous to the rule given by 
 * the position of the rule handle passed in.
 *
 * @param pFe 	       - Driver control structure
 * @param eDb          - Rule database to modify
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
                        soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle);

/**
 * soc_sbx_g2p3_ace_rule_remove()
 *
 * @brief 
 *
 * Remove a rule/action pair from a set, given a pointer to the rule.
 *
 * @param pFe         - Driver control structure
 * @param pRuleHandle - Reference to the rule to be removed
 *
 * @return - SB_OK on success, SB_IN_PROGRESS for async, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_remove(soc_sbx_g2p3_state_t *pFe, 
		               soc_sbx_g2p3_ace_rule_handle_t pRuleHandle);

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
soc_sbx_g2p3_ace_rule_handle_print(soc_sbx_g2p3_state_t *pFe, soc_sbx_g2p3_ace_mgr_rule_t *pRuleHandle);

/**
 * @fn soc_sbx_g2p3_ace_rule_print()
 *
 * @brief
 *
 * Print a rule/action pair from a set, given a pointer to the rule.
 *
 * @param pRule - Reference to the rule to be printed
 * @param pRule        - Rule pattern/action to print
 *
 * @return - SB_OK on success, SB_IN_PROGRESS for async, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_rule_print(soc_sbx_g2p3_ace_rule_t *pRule);

/**
 * soc_sbx_g2p3_ace_rule_head_get()
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
	         	soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle);

/**
 * soc_sbx_g2p3_cls_aceTailGet()
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
			       soc_sbx_g2p3_ace_rule_handle_t *ppRuleHandle);

/**
 * soc_sbx_g2p3_ace_commit_rule_db()
 * 
 * @brief 
 *
 * Commit a rule database to the hardware. This will pack the patterns into
 * the binary format needed by the hardware and transfer them to the pattern
 * memories inside the classifier. In addition, it will handle the transition
 * of banks from inactive to active state.
 *
 * NOTE:  The databases SB_G2P3_FE_CLS_SECURITY and SB_G2P3_FE_CLS_QOS are
 * commited at the same time always.
 * 
 * @param pFe - Driver control structure
 * @param eDb - Which rule database to commit to hardware (Security, Qos)
 *
 * @return - SB_OK on success, SB_IN_PROGRESS for async, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_commit_rule_db(soc_sbx_g2p3_state_t *pFe, 
			        soc_sbx_g2p3_cls_rule_db_e_t eDb);

sbStatus_t
soc_sbx_g2p3_ace_commit_rule_db_c2(soc_sbx_g2p3_state_t *pFe, 
			        soc_sbx_g2p3_cls_rule_db_e_t eDb);

/**
 * soc_sbx_g2p3_ace_rule_from_handle()
 * 
 * @brief 
 *
 * Given a classifier handle, returns the copy of the rule 
 * given the user provided structure. 
 *
 * @param pFe       -- Driver control structure
 * @param eDb       -- database enumeration.
 * @param pHandle   -- rule handle
 * @param pUserCopy -- pointer to the user memory 
 *
 * @return - pointer to rule described by handle
 **/
sbStatus_t 
soc_sbx_g2p3_ace_rule_from_handle( soc_sbx_g2p3_state_t *pFe,
                            soc_sbx_g2p3_cls_rule_db_e_t eDb,
                            soc_sbx_g2p3_ace_rule_handle_t pHandle, 
                            soc_sbx_g2p3_ace_rule_t *pUserCopy);
/**
 *  soc_sbx_g2p3_ace_counter_read()
 *
 * @brief 
 *
 * This function reads the hit count of the rule given by the 
 * Rule handle. This function finalizes the counters for the rule
 * handle and retuns the accumlated count. The bClear argument 
 * clears the current accumlated count.
 *
 * @param pFe       -- Driver control structure
 * @param eDb       -- database enumeration.
 * @param pHandle   -- rule handle
 * @param pCount    -- Pointer to the soc_sbx_g2p3_rule_count_t structure.
 * @param bClear    -- Force clear of the rule counts accumulation
 *
 * @return - SB_OK on success, error code otherwise
 **/
sbStatus_t
soc_sbx_g2p3_ace_counter_read(soc_sbx_g2p3_state_t *pFe,
                        soc_sbx_g2p3_cls_rule_db_e_t eDb,
                        soc_sbx_g2p3_ace_rule_handle_t pHandle,
                        soc_sbx_g2p3_count_t        *pCount,
                        uint8_t bClear);



/*sbStatus_t soc_sbx_g2p3_ace_rule_edit_uDsapSsap   (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle, uint32_t uDsapSsapLow, uint32_t uDsapSsapHigh);*/
/*sbStatus_t soc_sbx_g2p3_ace_rule_edit_uCtrl       (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle, uint32_t uCtrlByteLow, uint32_t uCtrlByteHigh);*/

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uPort       (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uPort,
		uint8_t bPortAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uPriority   (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uPriority,
		uint8_t bPriorityAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uFragment   (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uFragment,
		uint8_t bFragmentAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uTcpFlagURG (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uTcpFlagURG,
		uint8_t bTcpFlagURGAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uTcpFlagACK (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uTcpFlagACK,
		uint8_t bTcpFlagACKAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uTcpFlagPSH (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uTcpFlagPSH,
		uint8_t bTcpFlagPSHAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uTcpFlagRST (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uTcpFlagRST,
		uint8_t bTcpFlagRSTAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uTcpFlagSYN (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uTcpFlagSYN,
		uint8_t bTcpFlagSYNAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uTcpFlagFIN (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uTcpFlagFIN,
		uint8_t bTcpFlagFINAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uDscp       (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uDscp,
		uint8_t bDscpAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uEcn        (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uEcn,
		uint8_t bEcnAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uProtocol   (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uProtocol,
		uint8_t bProtocolAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uL4SrcPort  (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uL4SrcPortHigh,
		uint32_t uL4SrcPortLow);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uL4DstPort  (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uL4DstPortHigh,
		uint32_t uL4DstPortLow);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uIpv4Sa     (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uIpv4Sa,
		uint8_t bIpv4SaAny,
		uint32_t uIpv4SaLen);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uIpv4Da     (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uIpv4Da,
		uint8_t bIpv4DaAny,
		uint32_t uIpv4DaLen);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uEtherType  (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uEtherType,
		uint8_t bEtherTypeAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uuSmac      (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint64_t uuSmac,
		uint8_t bSmacAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uVlan       (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint32_t uVlan,
		uint8_t bVlanAny);

sbStatus_t
soc_sbx_g2p3_ace_rule_edit_uuDmac      (soc_sbx_g2p3_ace_rule_handle_t pRuleHandle,
		uint64_t uuDmac,
		uint8_t bDmacAny);

sbStatus_t
soc_sbx_g2p3_ace_wildcard_rule_addTail(soc_sbx_g2p3_state_t *pFe,
                                       soc_sbx_g2p3_cls_rule_db_e_t eDb,
                                       soc_sbx_g2p3_ace_mgr_rule_t **ppRuleHandle);

extern void
soc_sbx_g2p3_cls_ace_pattern_init(soc_sbx_g2p3_cls_ace_pattern_t *pFromStruct);

extern void
soc_sbx_g2p3_cls_ace_pattern_print(soc_sbx_g2p3_cls_ace_pattern_t *pFromStruct);

extern int
soc_sbx_g2p3_cls_ace_pattern_validate(soc_sbx_g2p3_cls_ace_pattern_t *pZf);

extern int
soc_sbx_g2p3_cls_ace_pattern_set(soc_sbx_g2p3_cls_ace_pattern_t *s,
                                 char* name, int value);

extern void
soc_sbx_g2p3_cls_ace_pattern_ipv6_init(soc_sbx_g2p3_cls_ace_pattern_ipv6_t *pFromStruct);

/*
extern void
soc_sbx_g2p3_cls_ace_pattern_print(soc_sbx_g2p3_cls_ace_pattern_t *pFromStruct);
*/

extern int
soc_sbx_g2p3_cls_ace_pattern_ipv6_validate(soc_sbx_g2p3_cls_ace_pattern_ipv6_t *pZf);

extern int
soc_sbx_g2p3_cls_ace_pattern_ipv6_set(soc_sbx_g2p3_cls_ace_pattern_ipv6_t *s,
                                 char* name, int value);

int
soc_sbx_g2p3_ace_nth_handle(int unit, soc_sbx_g2p3_cls_rule_db_e_t db,
                                int sentinelok, int irule,
                                soc_sbx_g2p3_ace_rule_handle_t *h);

sbStatus_t
soc_sbx_g2p3_ace_group_create(int unit, 
                              soc_sbx_g2p3_cls_rule_db_e_t eDb,
                              int16_t uGroupIndex,
                              int32_t uPriority,
                              soc_sbx_g2p3_ace_qset_t qset,
                              uint8_t bAllQualifers);

sbStatus_t
soc_sbx_g2p3_ace_group_del(int unit,
                           soc_sbx_g2p3_cls_rule_db_e_t eDb,
                           int16_t uGroupIndex);

#endif /* _SB_G2P3_FE_CLS_ACE_H_ */
