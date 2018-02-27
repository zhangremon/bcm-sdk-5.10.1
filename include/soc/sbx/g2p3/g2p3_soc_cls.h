#ifndef _SB_G2P3_FE_SOC_CLS_H_
#define _SB_G2P3_FE_SOC_CLS_H_
/*
 * $Id: g2p3_soc_cls.h 1.20.52.1 Broadcom SDK $
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
 * g2p3_soc_cls.h : Classifier Access Control Entry SOC layer interface
 * Used by BCM API
 *
 * --------------------------------------------------------------------------*/
/*
 * Layers on RCE Programming [ BCM Layer --> Soc Layer ]
 *                                              |
 *                             -(not used)---x  ---------------
 *                             |                              |
 *                   Rule Indices Interface        Rule Handle Interface
 *                                                 (exposed to bcm layer) 
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

/* common headers */
#include <soc/sbx/sbDq.h>

typedef void*                   soc_sbx_g2p3_ifp_handle_t;
typedef void*                   soc_sbx_g2p3_efp_handle_t;
typedef void*                   soc_sbx_g2p3_ifp_v6_handle_t;
typedef void*                   soc_sbx_g2p3_efp_v6_handle_t;

/**
 * soc_sbx_g2p3_ifp_max_rule_capacity_get()
 *
 * @brief 
 *
 * This function returns the maximum number of rules that FP
 * supports on the ingress or egress
 * 
 * Sentinel Rule will be reserved on Soc layer (-1)
 *
 * @param unit - FE unit number
 * @param pRuleCount(OUT) - rules supported on this database	
 *
 * @return - SOC_E_NONE on success, error code otherwise
 **/
int 
soc_sbx_g2p3_ifp_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount);

int 
soc_sbx_g2p3_efp_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount);
int 
soc_sbx_g2p3_ifp_v6_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount);

int 
soc_sbx_g2p3_efp_v6_max_rule_capacity_get(uint32_t unit,
                                       uint32_t *pRuleCount);


/**
 * soc_sbx_g2p3_xfp_handle_after_add()
 *
 * @brief 
 *
 * This function adds a rule to a xfp database
 *
 * @param unit - FE unit number
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
                       soc_sbx_g2p3_ifp_handle_t *handle);
int
soc_sbx_g2p3_efp_handle_after_add(uint32_t unit,
                       int16_t uGroupId,	
                       soc_sbx_g2p3_efp_handle_t predecessor, 
                       soc_sbx_g2p3_efp_t *pRule,
                       soc_sbx_g2p3_efp_handle_t *handle);
int
soc_sbx_g2p3_ifp_v6_handle_after_add(uint32_t unit,
                       int16_t uGroupId,	
                       soc_sbx_g2p3_ifp_v6_handle_t predecessor, 
                       soc_sbx_g2p3_ifp_v6_t *pRule,
                       soc_sbx_g2p3_ifp_v6_handle_t *handle);
int
soc_sbx_g2p3_efp_v6_handle_after_add(uint32_t unit,
                       int16_t uGroupId,	
                       soc_sbx_g2p3_efp_v6_handle_t predecessor, 
                       soc_sbx_g2p3_efp_v6_t *pRule,
                       soc_sbx_g2p3_efp_v6_handle_t *handle);

/* Api to aid handle add based on rule priority */
int
soc_sbx_g2p3_ifp_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t *handle);
int
soc_sbx_g2p3_efp_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t *handle);
int
soc_sbx_g2p3_ifp_v6_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_v6_handle_t *handle);
int
soc_sbx_g2p3_efp_v6_handle_head_get(uint32_t unit,
                       soc_sbx_g2p3_efp_v6_handle_t *handle);
int
soc_sbx_g2p3_ifp_handle_tail_get(uint32_t unit,
                       soc_sbx_g2p3_ifp_handle_t *handle);
int
soc_sbx_g2p3_efp_handle_tail_get(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t *handle);

/* mostly similar to iterator functions */
int
soc_sbx_g2p3_ifp_handle_prev_get(uint32_t unit,
		       soc_sbx_g2p3_ifp_handle_t  currentHandle,
                       soc_sbx_g2p3_ifp_handle_t *handle);
int
soc_sbx_g2p3_efp_handle_prev_get(uint32_t unit,
		       soc_sbx_g2p3_efp_handle_t   currentHandle,
                       soc_sbx_g2p3_efp_handle_t *handle);
int
soc_sbx_g2p3_ifp_handle_next_get(uint32_t unit,
		       soc_sbx_g2p3_ifp_handle_t  currentHandle,
                       soc_sbx_g2p3_ifp_handle_t *handle);
int
soc_sbx_g2p3_efp_handle_next_get(uint32_t unit,
		       soc_sbx_g2p3_efp_handle_t   currentHandle,
                       soc_sbx_g2p3_efp_handle_t *handle);
int
soc_sbx_g2p3_ifp_v6_handle_next_get(uint32_t unit,
		       soc_sbx_g2p3_ifp_v6_handle_t  currentHandle,
                       soc_sbx_g2p3_ifp_v6_handle_t *handle);
int
soc_sbx_g2p3_efp_v6_handle_next_get(uint32_t unit,
		       soc_sbx_g2p3_efp_v6_handle_t   currentHandle,
                       soc_sbx_g2p3_efp_v6_handle_t *handle);

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
                       soc_sbx_g2p3_ifp_t *pRule);
int 
soc_sbx_g2p3_efp_handle_update(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t handle,
                       soc_sbx_g2p3_efp_t *pRule);
int 
soc_sbx_g2p3_ifp_v6_handle_update(uint32_t unit,
                       soc_sbx_g2p3_ifp_v6_handle_t handle,
                       soc_sbx_g2p3_ifp_v6_t *pRule);
int 
soc_sbx_g2p3_efp_v6_handle_update(uint32_t unit,
                       soc_sbx_g2p3_efp_v6_handle_t handle,
                       soc_sbx_g2p3_efp_v6_t *pRule);

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
                       soc_sbx_g2p3_ifp_handle_t handle); 
int 
soc_sbx_g2p3_efp_handle_remove(uint32_t unit,
                       soc_sbx_g2p3_efp_handle_t handle); 
int 
soc_sbx_g2p3_ifp_v6_handle_remove(uint32_t unit,
                       soc_sbx_g2p3_ifp_v6_handle_t handle); 
int 
soc_sbx_g2p3_efp_v6_handle_remove(uint32_t unit,
                       soc_sbx_g2p3_efp_v6_handle_t handle); 

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
                            soc_sbx_g2p3_ifp_t *pUserCopy);
int 
soc_sbx_g2p3_efp_rule_from_handle(uint32_t unit,
                            soc_sbx_g2p3_efp_handle_t pHandle, 
                            soc_sbx_g2p3_efp_t *pUserCopy);
int 
soc_sbx_g2p3_ifp_v6_rule_from_handle(uint32_t unit,
                            soc_sbx_g2p3_ifp_v6_handle_t pHandle, 
                            soc_sbx_g2p3_ifp_v6_t *pUserCopy);
int 
soc_sbx_g2p3_efp_v6_rule_from_handle(uint32_t unit,
                            soc_sbx_g2p3_efp_v6_handle_t pHandle, 
                            soc_sbx_g2p3_efp_v6_t *pUserCopy);

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
                        soc_sbx_g2p3_counter_value_t   *pCount,
                        uint8_t bClear);
int
soc_sbx_g2p3_efp_counter_read(uint32_t unit,
                        soc_sbx_g2p3_efp_handle_t pHandle,
                        soc_sbx_g2p3_counter_value_t   *pCount,
                        uint8_t bClear);
int
soc_sbx_g2p3_ifp_v6_counter_read(uint32_t unit,
                        soc_sbx_g2p3_ifp_v6_handle_t pHandle,
                        soc_sbx_g2p3_counter_value_t   *pCount,
                        uint8_t bClear);
int
soc_sbx_g2p3_efp_v6_counter_read(uint32_t unit,
                        soc_sbx_g2p3_efp_v6_handle_t pHandle,
                        soc_sbx_g2p3_counter_value_t   *pCount,
                        uint8_t bClear);

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
soc_sbx_g2p3_ifp_clear_all(uint32_t unit);
int
soc_sbx_g2p3_efp_clear_all(uint32_t unit);
int
soc_sbx_g2p3_ifp_v6_clear_all(uint32_t unit);
int
soc_sbx_g2p3_efp_v6_clear_all(uint32_t unit);


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
soc_sbx_g2p3_ifp_rule_handle_print(int unit, soc_sbx_g2p3_ifp_handle_t *pRuleHandle);
int
soc_sbx_g2p3_efp_rule_handle_print(int unit, soc_sbx_g2p3_efp_handle_t *pRuleHandle);

/**
 * @fn soc_sbx_g2p3_ifp_handle_commit
 *
 * @brief
 * commit function inteded only for IFP handle interface.
 *
 */
int soc_sbx_g2p3_ifp_handle_commit(int unit, int runlength);

/**
 * @fn soc_sbx_g2p3_efp_handle_commit
 *
 * @brief
 * commit function inteded only for EFP handle interface.
 *
 */
int soc_sbx_g2p3_efp_handle_commit(int unit, int runlength);

/**
 * @fn soc_sbx_g2p3_ifp_v6_handle_commit
 *
 * @brief
 * commit function intended only for IFP_V6 handle interface.
 *
 */
int soc_sbx_g2p3_ifp_v6_handle_commit(int unit, int runlength);

/**
 * @fn soc_sbx_g2p3_efp_v6_handle_commit
 *
 * @brief
 * commit function intended only for EFP_V6 handle interface.
 *
 */
int soc_sbx_g2p3_efp_v6_handle_commit(int unit, int runlength);

int soc_sbx_g2p3_max_supported_stages(int unit);
#endif /* _SB_G2P3_FE_SOC_CLS_H_ */
