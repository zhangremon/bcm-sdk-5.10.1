/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
 * $Id: sbFe2000ClsComp.h 1.12.52.1 Broadcom SDK $
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
 *
 * sbFe2000ClsComp.h  : FE2000 Classifier Instruction Compiler
 * 
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE2K_CLS_COMP_H_
#define _SB_FE2K_CLS_COMP_H_

#include "glue.h"
#include "sbTypes.h"
#include <soc/sbx/fe2k/sbZfFe2000RcSbPattern.hx>

typedef enum sbFe2000ClsOperation_s
{
  SB_FE2K_CLS_OP_EQ                           = 0,
  SB_FE2K_CLS_OP_LEQ                          = 1,
  SB_FE2K_CLS_OP_LT                           = 2,
  SB_FE2K_CLS_OP_GT                           = 3,
  SB_FE2K_CLS_OP_GEQ                          = 4,
  SB_FE2K_CLS_OP_LPM                          = 5,
  SB_FE2K_CLS_OP_RANGE                        = 6,
  SB_FE2K_CLS_OP_ONE_OF                       = 7,
  SB_FE2K_CLS_OP_MULTIPORT                    = 8,
  SB_FE2K_CLS_OP_WEQ                          = 9,
  SB_FE2K_CLS_OP_SUSPEND_IFNOT_PATTERN        = 10,
  SB_FE2K_CLS_MAX_OPS                         = 11
} sbFe2000ClsOperation_t;

typedef struct sbFe2000ClsTemplateField_s
{
  char                       sFieldName[128];
  sbFe2000ClsOperation_t     eOperation;
  int32_t                    uStart;
  int32_t                    uEnd;
  uint32_t                   uWidth;
  uint8_t                   *pAValArray; /* only 8 bits */
  uint8_t                    bOverlaid;
  uint8_t                    bEnabled;
  uint8_t                    uQualifier;
} sbFe2000ClsTemplateField_t;


/* @struct sbFe2000ClsTemplate_t
 */
typedef struct sbFe2000ClsTemplate_s
{
  char                               sName[128];
  uint32_t                           uCount;
  sbFe2000ClsTemplateField_t         *ops;
  uint8_t                            bDebug;
} sbFe2000ClsTemplate_t;

sbStatus_t 
sbFe2000ClsGenerateInstructions( sbFe2000ClsTemplate_t *pTemplate, 
                                 uint32_t **ppInstructions, 
                                 uint32_t *pCount, 
                                 uint32_t *pRealCount,
                                 uint32_t bResultToLowerHalf,
                                 uint32_t isC2 );

sbStatus_t
sbFe2000ClsGenerateInstructions_c2( sbFe2000ClsTemplate_t *pTemplate,
                                    uint32_t sType,
                                    uint32_t **ppInstructions,
                                    uint32_t *pCount,
                                    uint32_t *pRealCount,
                                    uint32_t bResultToLowerHalf);

sbStatus_t
sbFe2000XtClsEncodePattern(uint32_t uRuleId,
                           uint64_t uuField1,
                           uint64_t uuField2,
                           sbFe2000ClsTemplateField_t *pOperation,
                           uint32_t uProgramSize,
                           uint32_t * pLocation,
                           sbZfFe2000RcSbPattern_t * pPattern,
                           uint64_t uuField3);
sbStatus_t
sbFe2000XtClsDecodePattern(uint32_t uRuleId,
                           uint64_t *uuField1,
                           uint64_t *uuField2,
                           sbFe2000ClsTemplateField_t *pOperation,
                           uint32_t uProgramSize,
                           uint32_t * pLocation,
                           sbZfFe2000RcSbPattern_t * pPattern,
                           uint64_t *uuField3);

void 
sbFe2000ClsPrintTemplate(sbFe2000ClsTemplate_t *pTemplate);

void 
sbFe2000ClsPrintInstructions(uint32_t *pInstructions, uint32_t uCount, uint32_t isC2);

void 
sbFe2000ClsUpdateNewRuleSet(uint32_t *pInstruction, uint32_t set);

void
sbFe2000ClsUpdateJumpTarget(uint32_t *pInstruction, uint32_t jumpTarget);


#endif
