/* -*- Mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/* $Id: sbFe2000ClsComp.c 1.33.52.3 Broadcom SDK $
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
 * sbFe2000ClsComp.c : FE2000 Classifier Instruction Generator 
 *
 *-----------------------------------------------------------------------------*/

#include "sbTypes.h"
#include "sbStatus.h"
#include "sbWrappers.h"
#include "sbFe2000ClsComp.h"
#include "sbZfFe2000ClsInstruction.hx"


typedef enum 
{
  SB_FE2K_CLS_SM_PATTERN_INVERSE                   = 0,   /* result = !pattern */
  SB_FE2K_CLS_SM_KEY_GT_PATTERN                    = 1,   /* result = (key > pattern) */
  SB_FE2K_CLS_SM_KEY_NAND_PATTERN                  = 2,   /* result = nand(key, pattern ) */
  SB_FE2K_CLS_SM_KEY_NEQ_PATTERN                   = 3,   /* result = xor(key, pattern) = (key !=pattern) */
  SB_FE2K_CLS_SM_KEY_NOR_PATTERN                   = 4,   /* result = nor(key, pattern) */
  SB_FE2K_CLS_SM_FUNCTION_ZERO                     = 5,   /* Function Zero */
  SB_FE2K_CLS_SM_KEY_INVERSE                       = 6,   /* result = !key */
  SB_FE2K_CLS_SM_KEY_LT_PATTERN                    = 7,   /* result = (key < pattern) */
  SB_FE2K_CLS_SM_MASK_PATTERN_INVERSE              = 8,   /* result = !(!key & pattern) = (key or !pattern) */
  SB_FE2K_CLS_SM_KEY_BIT                           = 9,   /* result = key */
  SB_FE2K_CLS_SM_FUNCTION_ONE                      = 10,  /* Function One */
  SB_FE2K_CLS_SM_KEY_OR_PATTERN                    = 11,  /* result = (key | pattern ) */
  SB_FE2K_CLS_SM_KEY_EQUAL_PATTERN                 = 12,  /* reuslt = (key == pattern) */
  SB_FE2K_CLS_SM_KEY_AND_PATTERN                   = 13,  /* reuslt = (key & pattern) */
  SB_FE2K_CLS_SM_MASK_INVERTED_KEY                 = 14,  /* reuslt = !(k && !p) = !k | pattern */
  SB_FE2K_CLS_SM_PATTERN_BIT                       = 15,  /* reuslt = pattern */
  SB_FE2K_CLS_SM_MAX                               = 16
} sbFe2000ClsSMOpCode_t;

typedef enum 
{
  SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION = 0,
  SB_FE2K_CLS_OPCODE_MSB_RESUME             = 1,
  SB_FE2K_CLS_OPCODE_MSB_NEW_RULE           = 2,
  SB_FE2K_CLS_OPCODE_MSB_RESTART            = 3
} sbFe2000ClsOpcodeMsb_t;

typedef enum 
{
  SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY        = 0,
  SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_RESULT     = 1,
  SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_LPM        = 2,
  SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_MULTIPORT  = 3
} sbFe2000ClsOpcodeLsb_t;

/* For c2 the opcodes are documented as a 4 bit field, without the msb/lsb split */

typedef enum 
{
  SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY        = 0,
  SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_RESULT     = 1,
  SB_FE2K_CLS_OPCODE_C2_LPM                  = 2,
  SB_FE2K_CLS_OPCODE_C2_MULTIPORT            = 3,
  SB_FE2K_CLS_OPCODE_C2_RESUME               = 4,
  SB_FE2K_CLS_OPCODE_C2_NEW_RULE_RES_SLOT0   = 8,
  SB_FE2K_CLS_OPCODE_C2_NEW_RULE_RES_SLOT1   = 9,
  SB_FE2K_CLS_OPCODE_C2_RESTART              = 12,
  SB_FE2K_CLS_OPCODE_C2_JUMP_NOT_END         = 14,
  SB_FE2K_CLS_OPCODE_C2_JUMP_END_NEXT_NR     = 15,


} sbFe2000ClsOpcodeC2_t;

static const char *RcOpNames[SB_FE2K_CLS_MAX_OPS] = 
{
  "EQ              ",
  "LEQ             ",
  "LT              ",
  "GT              ",
  "GEQ             ",
  "LPM             ",
  "RANGE           ",
  "ONE_OF          ",
  "MULTIPORT       ",
  "WEQ             ",
  "SUSP_IFNOT_PATT "
};

static const char *RcInstructionNames[SB_FE2K_CLS_SM_MAX] =
{
  "PATTERN_INVERSE     ",   
  "KEY_GT_PATTERN      ",
  "KEY_NAND_PATTERN    ",
  "KEY_NEQ_PATTERN     ",
  "KEY_NOR_PATTERN     ",
  "FUNCTION_ZERO       ",
  "KEY_INVERSE         ",
  "KEY_LT_PATTERN      ",
  "MASK_PATTERN_INVERSE",
  "KEY_BIT             ",
  "FUNCTION_ONE        ",
  "KEY_OR_PATTERN      ",
  "KEY_EQUAL_PATTERN   ",
  "KEY_AND_PATTERN     ",
  "MASK_INVERTED_KEY   ",
  "PATTERN_BIT         "
};

static const char *RcOpCodeMsbNames[4] = 
{
  "normal_instruction",
  "resume",
  "new_rule",
  "restart",
};

static const char *RcOpCodeLsbNames[4] = 
{
  "a_is_key",
  "a_is_result",
  "a_is_lpm",
  "a_is_multiport"
};

static const char *RcOpCodeNamesC2[16] =
{
  "Norm - A is key",
  "Norm - A is res",
  "LPM            ",
  "Multiport      ",
  "Resume         ",
  "Illegal5       ",
  "Illegal6       ",
  "Illegal7       ",
  "NewRule rs sl 0",
  "NewRule rs sl 1",
  "Illegal10      ",
  "Illegal11      ",
  "Restart        ",
  "Illegal13      ",
  "Jump not eop   ",
  "Jump eop nxt nr"
};

typedef struct {
  uint32_t uOpCodeMsb;
  uint32_t uOpCodeLsb;
} c1_opcode_t;

typedef struct {
  uint32_t uOpCode; /* don't do msb/lsb for c2 */
} c2_opcode_t;


typedef struct 
{
   union c1_or_c2_union {
    c1_opcode_t c1_op;
    c2_opcode_t c2_op;
  } c1c2;
  uint32_t uSField;
  uint32_t uMField;
  uint32_t uAField;
} sbFe2000ClsInstructionFormat_t;


/* The macros CLS_xxx are not setup correctly for C2 since fields have changed. Make c2 specific versions */


#define CLS_OPMSB(INSTR)       ( (INSTR >> 18) & 0x3) 
#define CLS_OPLSB(INSTR)       ( (INSTR >> 16) & 0x3) 
#define CLS_SFIELD(INSTR)      ( (INSTR >> 12) & 0xF) 
#define CLS_MFIELD(INSTR)      ( (INSTR >> 8)  & 0xF) 
#define CLS_AFIELD(INSTR)      ( (INSTR)   & 0xFF)
 
#define CLS_OPFIELD_C2(INSTR)     ( (INSTR >> 17) & 0xF) /* c2 has straight 4 bit op field */
#define CLS_SFIELD_C2(INSTR)      ( (INSTR >> 13) & 0xF) 
#define CLS_MFIELD_C2(INSTR)      ( (INSTR >> 9)  & 0xF) 
#define CLS_AFIELD_C2(INSTR)      ( (INSTR)   & 0x1FF)   /* 'A' field gets extra bit */
 
/* this macro is the c2 version of SB_FE2K_MAKE_CLS_INSTRUCTION(x)  */

#define SB_FE2K_MAKE_CLS_INSTRUCTION_C2(x) ((x.c1c2.c2_op.uOpCode << 17) | \
                                            (x.uSField    << 13) | \
                                            (x.uMField    << 9 ) | \
                                            (x.uAField) )

#define SB_FE2K_MAKE_CLS_INSTRUCTION_C1(x)   ( (x.c1c2.c1_op.uOpCodeMsb << 18) | \
                                            (x.c1c2.c1_op.uOpCodeLsb << 16) | \
                                            (x.uSField    << 12) | \
                                            (x.uMField    << 8 ) | \
                                            (x.uAField) )



#define CLS_SAVE_TO_LOWER_SWORD           ( (1 << 2) )
#define CLS_SAVE_TO_HIGHER_SWORD          ( (1 << 2) | 0x1 )

static void
sbFe2000ClsStuffRuleSet(sbFe2000ClsTemplate_t *pTemplate, 
                        uint32_t *pInstructions,
                        uint32_t bResultToLowerHalf,
                        uint32_t *pProgramSize,
                        uint32_t *pPadding,
                        uint32_t isC2);

void 
sbFe2000ClsPrintTemplate(sbFe2000ClsTemplate_t *pTemplate)
{
  int i;
  sbFe2000ClsTemplateField_t  *pOperation = NULL;

  SB_LOG("total number of operations count %d\n", pTemplate->uCount);
  for(i=0; i < pTemplate->uCount ; i++) {
    pOperation = &pTemplate->ops[i];
    SB_LOG("FName: %s  OpName: %s StartBit: %3d  EndBit: %3d OpWidth: %3d \n", 
                                pOperation->sFieldName,
                                RcOpNames[pOperation->eOperation],
                                pOperation->uStart,
                                pOperation->uEnd ,
                                pOperation->uWidth);
  }
}

void sbFe2000ClsPrintInstructions(uint32_t *pInstructions, uint32_t uCount, uint32_t isC2)
{
  int i=0;
  uint32_t uInstruction =0 ;
  SB_LOG("total number of instructions %d\n", uCount);
  for(i=0; i < uCount; i++) {
    uInstruction = pInstructions[i];
    if(isC2) {
      SB_LOG("I 0x%x\t%s\tS:%s\tM:%s\t%3d\n",
             i,
             RcOpCodeNamesC2[CLS_OPFIELD_C2(uInstruction)],
             RcInstructionNames[CLS_SFIELD_C2(uInstruction)], 
             RcInstructionNames[CLS_MFIELD_C2(uInstruction)], 
             CLS_AFIELD_C2(uInstruction)  );

    } else {
      SB_LOG("%s\t%s\t%s\t%s\t%3d\n",
                RcOpCodeMsbNames[CLS_OPMSB(uInstruction)],
                RcOpCodeLsbNames[CLS_OPLSB(uInstruction)],
                RcInstructionNames[CLS_SFIELD(uInstruction)], 
                RcInstructionNames[CLS_MFIELD(uInstruction)], 
                CLS_AFIELD(uInstruction)  );
    }
  }
}

/*
 * Print a block of 4 pattern bits that corresponds with 1 instruction
*/
void sbFe2000ClsPrintPattern(uint32_t *pPattern)
{
  SB_LOG("P[31:00]  = 0x%08x\n", *pPattern);
  SB_LOG("P[63:32]  = 0x%08x\n", *(pPattern+1) );
  SB_LOG("P[95:64]  = 0x%08x\n", *(pPattern+2) );
  SB_LOG("P[127:96] = 0x%08x\n", *(pPattern+3) );
}

void
sbFe2000ClsUpdateNewRuleSet(uint32_t *pInstruction, uint32_t set)
{
  SB_ASSERT( ((CLS_OPFIELD_C2(*pInstruction)==SB_FE2K_CLS_OPCODE_C2_NEW_RULE_RES_SLOT0) || \
             (CLS_OPFIELD_C2(*pInstruction)==SB_FE2K_CLS_OPCODE_C2_NEW_RULE_RES_SLOT1)) );

  *pInstruction = *pInstruction & (~CLS_AFIELD_C2(0xFFFFFFFF));
  *pInstruction = *pInstruction | (CLS_AFIELD_C2(set));

}
/*
 * sbFe2000ClsUpdateJumpTarget 
 * This function sets up the correct flavor of jump instruction according to the jumpTarget parameter passed. 
 * If its zero its the terminating flavor. 
*/

#define MASK_S_AND_M 0x0001FE00

void
sbFe2000ClsUpdateJumpTarget(uint32_t *pInstruction, uint32_t jumpTarget)
{
  /* we should be overwriting a flavor of jump instruction */
  SB_ASSERT( ((CLS_OPFIELD_C2(*pInstruction)==SB_FE2K_CLS_OPCODE_C2_JUMP_NOT_END) || \
             (CLS_OPFIELD_C2(*pInstruction)==SB_FE2K_CLS_OPCODE_C2_JUMP_END_NEXT_NR)) );

  *pInstruction &= MASK_S_AND_M; /* clear everything except s + m bits */

  if (jumpTarget < 4096) {
    *pInstruction |= SB_FE2K_CLS_OPCODE_C2_JUMP_NOT_END << 17 |
                    CLS_AFIELD_C2(jumpTarget);
  } else {
    *pInstruction |= SB_FE2K_CLS_OPCODE_C2_JUMP_END_NEXT_NR << 17;
  }
}

static int32_t
sbFe2000XtClsEncodeInstructions(sbFe2000ClsTemplate_t *pTemplate,
                                uint32_t *pInstructions,
                                uint32_t bResultToLowerHalf,
                                uint32_t *pProgramSize,
                                uint32_t *pPadding)
{
    sbFe2000ClsInstructionFormat_t zInstruction;
    uint32_t uInstructionCount, uFieldId, uWidth, uNumPorts, temp;
    uint32_t uOffset;
    sbFe2000ClsTemplateField_t *pOperation;
    uint32_t uJumpInstructionNo = -1; /* uSize = -1, uPadding; */
    
#define BUILD_INST(uOpcodeValue, uSFieldValue, uMFieldValue, uAFieldValue) \
do { \
    sal_memset(&zInstruction, 0, sizeof(&zInstruction)); \
    zInstruction.c1c2.c2_op.uOpCode = (uOpcodeValue); \
    zInstruction.uSField            = (uSFieldValue); \
    zInstruction.uMField            = (uMFieldValue); \
    zInstruction.uAField            = (uAFieldValue); \
    if(pInstructions != NULL) { \
        pInstructions[uInstructionCount] = \
                               SB_FE2K_MAKE_CLS_INSTRUCTION_C2(zInstruction); \
    } \
    uInstructionCount++; \
    if (uInstructionCount == uJumpInstructionNo) { \
        sal_memset(&zInstruction, 0, sizeof(&zInstruction)); \
        zInstruction.c1c2.c2_op.uOpCode = \
                                SB_FE2K_CLS_OPCODE_C2_JUMP_END_NEXT_NR; \
        zInstruction.uSField            = SB_FE2K_CLS_SM_FUNCTION_ZERO; \
        zInstruction.uMField            = SB_FE2K_CLS_SM_FUNCTION_ZERO; \
        zInstruction.uAField            = 0; \
        if(pInstructions != NULL) { \
            pInstructions[uInstructionCount] = \
                               SB_FE2K_MAKE_CLS_INSTRUCTION_C2(zInstruction); \
        } \
        uInstructionCount++; \
    } \
} while (0)

    if ( pInstructions !=NULL) {
        if (*pProgramSize == -1) {
            sbFe2000XtClsEncodeInstructions(pTemplate, NULL, 0, 
                                            pProgramSize, pPadding);
        }
        /* Jump is @ 25th from the last */
        uJumpInstructionNo = *pProgramSize - *pPadding - 25;
    }

    uInstructionCount = 0;
  
    /* Restart Instruction */
    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_RESTART, \
               SB_FE2K_CLS_SM_PATTERN_INVERSE, \
               SB_FE2K_CLS_SM_PATTERN_INVERSE, \
               0);

    /* Walk through the template and encode the opcodes */
    for (uFieldId = 0; uFieldId < pTemplate->uCount; uFieldId++) {
        if (pTemplate->ops[uFieldId].bEnabled == 0 || 
                         pTemplate->ops[uFieldId].bOverlaid) {
            continue;
        }

        pOperation = &pTemplate->ops[uFieldId];

        uWidth = pOperation->uStart - pOperation->uEnd + 1;

        switch(pOperation->eOperation) {
        case SB_FE2K_CLS_OP_MULTIPORT:  /* MPT */
            /* Ensure the field size >= 6 bits */
            uNumPorts = pOperation->uWidth;
            if (uWidth < 6) {
               /* Print Error */
               return -1;
            }
            /* Pad until module 6 
                  while((InstNum + numPorts + 2) % 8 < 7)*/
            while (((uInstructionCount + uNumPorts + 2) % 8) < 7) {
                /* Encode No Operation */
                BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                           SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                           SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                           0);
            }

            temp = 0;
            while (temp < uNumPorts) {
                uOffset = uNumPorts - temp - 1;
                if (uOffset == 4) {
                    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_RESUME, \
                               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                               uOffset);
                }
                if (uOffset == 0) {
                    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_MULTIPORT, \
                               SB_FE2K_CLS_SM_KEY_AND_PATTERN, \
                               SB_FE2K_CLS_SM_KEY_NAND_PATTERN, \
                               uOffset);
                } else {
                    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_MULTIPORT, \
                               SB_FE2K_CLS_SM_KEY_AND_PATTERN, \
                               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                               uOffset);
                }
                temp++;
            }
            break;
            
        case SB_FE2K_CLS_OP_RANGE:      /* RNG */
            if (uWidth < 6) {
               /* Print Error */
               return -1;
            }

            while ((uInstructionCount % 8) != 5) {
                /* Encode No Operation */
                BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                           SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                           SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                           0);
            }

            /* check for master or slave */

            temp = pOperation->uStart;

            /* Encode Greater Than */
            while (temp >= pOperation->uEnd) {
                uOffset = temp - pOperation->uEnd + 1;
            
                if (uOffset == 5) {
                    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_RESUME, \
                               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                               temp);
                }

                BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                           SB_FE2K_CLS_SM_KEY_GT_PATTERN, \
                           SB_FE2K_CLS_SM_KEY_LT_PATTERN, \
                           temp);
                temp--;
            }

            /* Encode Less Than */
            temp = pOperation->uStart;
            while (temp >= pOperation->uEnd) {
                uOffset = temp - pOperation->uEnd + 1;
            
                if (uOffset == 5) {
                    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_RESUME, \
                               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                               temp);
                }

                BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                           SB_FE2K_CLS_SM_KEY_LT_PATTERN, \
                           SB_FE2K_CLS_SM_KEY_GT_PATTERN, \
                           temp);
                temp--;
            }
            break;
        case SB_FE2K_CLS_OP_WEQ:        /* WEQ */

            BUILD_INST(SB_FE2K_CLS_OPCODE_C2_LPM, \
                       SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                       SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                       pOperation->uStart);
            
            temp = pOperation->uStart;
            uOffset = uWidth;
            while (temp >= pOperation->uEnd) {
                uOffset--;
                uWidth = pOperation->uStart - temp;
                BUILD_INST(SB_FE2K_CLS_OPCODE_C2_LPM, \
                           SB_FE2K_CLS_SM_PATTERN_INVERSE, \
                           SB_FE2K_CLS_SM_PATTERN_INVERSE, \
                           temp);
                temp--;
            }
            break;

        case SB_FE2K_CLS_OP_LPM:        /* LPM */

            BUILD_INST(SB_FE2K_CLS_OPCODE_C2_LPM, \
                       SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                       SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                       pOperation->uEnd);
            
            temp = pOperation->uEnd;
            uOffset = 0;
            while (pOperation->uStart >= temp) {
                uWidth = pOperation->uStart - temp;
                BUILD_INST(SB_FE2K_CLS_OPCODE_C2_LPM, \
                           SB_FE2K_CLS_SM_PATTERN_INVERSE, \
                           SB_FE2K_CLS_SM_PATTERN_INVERSE, \
                           temp);
                temp++;
                uOffset++;
            }
            break;
            
        default:
            break;
        }
    }

   /* Ensure module 8 */
    while ((uInstructionCount % 8) < 7) {
        BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   0);
    }

   /* Place the jump instruction */
    if( uJumpInstructionNo == -1) {
        BUILD_INST(SB_FE2K_CLS_OPCODE_C2_JUMP_END_NEXT_NR, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   0);
    } else {
        BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   0);
    }

   /* Add 7 Nops */
    while ((uInstructionCount % 8) < 7) {
        BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   0);
    }

    while (uInstructionCount < 63) {
        BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   SB_FE2K_CLS_SM_FUNCTION_ZERO, \
                   0);
    }

   /* New Rule */
    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_NEW_RULE_RES_SLOT0, \
               SB_FE2K_CLS_SM_PATTERN_INVERSE, \
               SB_FE2K_CLS_SM_PATTERN_INVERSE, \
               0);

   /* Add Nop */
    BUILD_INST(SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY, \
               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
               SB_FE2K_CLS_SM_FUNCTION_ZERO, \
               0);
   *pProgramSize = uInstructionCount;
   *pPadding = 1;
   return 0;
}


sbStatus_t
sbFe2000XtClsEncodePattern(uint32_t uRuleId,
                           uint64_t uuField1,
                           uint64_t uuField2,
                           sbFe2000ClsTemplateField_t *pOperation,
                           uint32_t uProgramSize,
                           uint32_t * pLocation,
                           sbZfFe2000RcSbPattern_t * pPattern,
                           uint64_t uuField3)
{
    uint32_t uWidth, uNumPorts, temp;
    uint32_t uOffset , uPatternBit, uLocation;
    uint32_t uJumpInstructionNo = -1;
    
#define ENCODE_PATTERN(uOffsetValue, uRuleIdValue, uBitValue) \
do { \
    sbZfFe2000RcSbSetRuleBit(&pPattern[uOffsetValue], \
                             uRuleIdValue, \
                             uBitValue);  \
    uOffsetValue++; \
    if (uOffsetValue == uJumpInstructionNo) { \
        sbZfFe2000RcSbSetRuleBit(&pPattern[uOffsetValue], \
                                 uRuleIdValue, \
                                 uBitValue);  \
        uOffsetValue++; \
    } \
} while (0)


    /* Jump is @ 25th from the last */
    uJumpInstructionNo = uProgramSize - 25;

    /* Pattern for Restart should be encoded in calling function */
    if (*pLocation == 0) {
        return -1;
    }

    uLocation = *pLocation;
    uWidth = pOperation->uStart - pOperation->uEnd + 1;

    switch(pOperation->eOperation) {
    case SB_FE2K_CLS_OP_MULTIPORT:  /* MPT */
        /* Ensure the field size >= 6 bits */
        uNumPorts = pOperation->uWidth;
        if (uWidth < 6) {
           /* Print Error */
           return -1;
        }

        /* Pad until module 6 
           while((InstNum + numPorts + 2) % 8 < 7)*/
        while (((uLocation + uNumPorts + 2) % 8) < 7) {
            /* Encode No Operation */
            ENCODE_PATTERN(uLocation, uRuleId, 0);
        }

        temp = 0;
        while (temp < uNumPorts) {
            uOffset = uNumPorts - temp - 1;
            uPatternBit = (((uuField1 >> uOffset) & 0x1) ? 1: 0);
            if (uOffset == 4) {
                ENCODE_PATTERN(uLocation, uRuleId, 0);
            }
            ENCODE_PATTERN(uLocation, uRuleId, uPatternBit);
            temp++;
        }
        break;
            
    case SB_FE2K_CLS_OP_RANGE:      /* RNG */
        if (uWidth < 6) {
           /* Print Error */
           return -1;
        }

        while ((uLocation % 8) != 5) {
                /* Encode No Operation */
            ENCODE_PATTERN(uLocation, uRuleId, 0);
        }

        temp = pOperation->uStart;

        /* Encode Greater Than */
        while (temp >= pOperation->uEnd) {
            uOffset = temp - pOperation->uEnd;
            /* uuField1 has the Low Value of the Range */
            uPatternBit = (((uuField1 >> uOffset) & 0x1) ? 1: 0);
            
            if (uOffset == 4) {
                ENCODE_PATTERN(uLocation, uRuleId, 0);
            }
            ENCODE_PATTERN(uLocation, uRuleId, uPatternBit);
            temp--;
        }

        /* Encode Less Than */
        temp = pOperation->uStart;
        while (temp >= pOperation->uEnd) {
            uOffset = temp - pOperation->uEnd;
            /* uuField2 has the High Value of the Range */
            uPatternBit = (((uuField2 >> uOffset) & 0x1) ? 1: 0);
            
            if (uOffset == 4) {
                ENCODE_PATTERN(uLocation, uRuleId, 0);
            }
            ENCODE_PATTERN(uLocation, uRuleId, uPatternBit);
            temp--;
        }
        break;

    case SB_FE2K_CLS_OP_WEQ:        /* WEQ */
        /* uuField2 has the anybit for WEQ */
        uPatternBit = ((uuField2 & 0x1) ? 1 : 0);
        ENCODE_PATTERN(uLocation, uRuleId, uPatternBit);

        temp = pOperation->uStart;
        uOffset = uWidth;
        while (temp >= pOperation->uEnd) {
            uOffset--;
            uWidth = pOperation->uStart - temp;
            uPatternBit = ((uuField2 & 0x1) ? 1 : 
                           (((uuField1 >> uOffset) & 0x1) ? 1: 0));
            ENCODE_PATTERN(uLocation, uRuleId, uPatternBit);
            temp--;
        }
        break;

    case SB_FE2K_CLS_OP_LPM:        /* LPM */
        /* uuField2 has the length for LPM */
        uPatternBit = ((uuField2 < uWidth) ? 1 : 0);
        ENCODE_PATTERN(uLocation, uRuleId, uPatternBit);

        temp = pOperation->uEnd;
        uOffset = 0;
        while (pOperation->uStart >= temp) {
            uWidth = pOperation->uStart - temp;
            if (uOffset < 64) {
                uPatternBit = ((uuField2 < uWidth) ? 1 : 
                               ((uuField2 == uWidth) ? 0 : 
                                ((( uuField1 >> uOffset) & 0x1) ? 1 : 0)));
            } else {
                uPatternBit = ((uuField2 < uWidth) ? 1 : 
                               ((uuField2 == uWidth) ? 0 : 
                                ((( uuField3 >> (uOffset -64)) & 0x1) ? 1 : 0)
                               ));
            }
            ENCODE_PATTERN(uLocation, uRuleId, uPatternBit);
            temp++;
            uOffset++;
        }
        break;
            
    default:
        break;
    }

    *pLocation = uLocation;
    return 0;
}

sbStatus_t
sbFe2000XtClsDecodePattern(uint32_t uRuleId,
                           uint64_t *uuField1,
                           uint64_t *uuField2,
                           sbFe2000ClsTemplateField_t *pOperation,
                           uint32_t uProgramSize,
                           uint32_t * pLocation,
                           sbZfFe2000RcSbPattern_t * pPattern,
                           uint64_t *uuField3)
{
    uint32_t uWidth, uNumPorts, temp;
    uint32_t uOffset, uLocation, uDontCareLength;
    uint32_t uJumpInstructionNo = -1;
    uint8_t dummy, uPatternBit;
    uint64_t uuTemp;
    
#define DECODE_PATTERN(uOffsetValue, uRuleIdValue, uBitValue) \
do { \
    sbZfFe2000RcSbGetRuleBit(&pPattern[uOffsetValue], \
                             uRuleIdValue, \
                             &uBitValue);  \
    uOffsetValue++; \
    if (uOffsetValue == uJumpInstructionNo) { \
        uOffsetValue++; \
    } \
} while (0)

    /* Jump is @ 25th from the last */
    uJumpInstructionNo = uProgramSize - 25;

    /* Pattern for Restart should be encoded in calling function */
    if (*pLocation == 0) {
        return -1;
    }

    uLocation = *pLocation;
    uWidth = pOperation->uStart - pOperation->uEnd + 1;

    switch(pOperation->eOperation) {
    case SB_FE2K_CLS_OP_MULTIPORT:  /* MPT */
        /* Ensure the field size >= 6 bits */
        uNumPorts = pOperation->uWidth;
        if (uWidth < 6) {
           /* Print Error */
           return -1;
        }

        /* Pad until module 6 
           while((InstNum + numPorts + 2) % 8 < 7)*/
        while (((uLocation + uNumPorts + 2) % 8) < 7) {
            /* Encode No Operation */
            DECODE_PATTERN(uLocation, uRuleId, dummy);
        }

        temp = 0;
        *uuField1 = 0;
        *uuField2 = 1;
        while (temp < uNumPorts) {
            uOffset = uNumPorts - temp - 1;
            if (uOffset == 4) {
              DECODE_PATTERN(uLocation, uRuleId, dummy);
            }
            DECODE_PATTERN(uLocation, uRuleId, uPatternBit);
            *uuField1 <<= 1;
            *uuField1 |= uPatternBit;
            *uuField2 &= uPatternBit;
            temp++;
        }
        break;

    case SB_FE2K_CLS_OP_RANGE:      /* RNG */
        if (uWidth < 6) {
           /* Print Error */
           return -1;
        }

        while ((uLocation % 8) != 5) {
                /* Encode No Operation */
            DECODE_PATTERN(uLocation, uRuleId, dummy);
        }


        /* Encode Greater Than */
        *uuField1 = 0;
        temp = pOperation->uStart;
        while (temp >= pOperation->uEnd) {
            uOffset = temp - pOperation->uEnd;
            if (uOffset == 4) {
                DECODE_PATTERN(uLocation, uRuleId, dummy);
            }
            DECODE_PATTERN(uLocation, uRuleId, uPatternBit);
            /* uuField1 has the Low Value of the Range */
            *uuField1 <<= 1;
            *uuField1 |= uPatternBit;
            temp--;
        }

        /* Encode Less Than */
        *uuField2 = 0;
        temp = pOperation->uStart;
        while (temp >= pOperation->uEnd) {
            uOffset = temp - pOperation->uEnd;
            if (uOffset == 4) {
                DECODE_PATTERN(uLocation, uRuleId, dummy);
            }
            DECODE_PATTERN(uLocation, uRuleId, uPatternBit);
            /* uuField2 has the High Value of the Range */
            *uuField2 <<= 1;
            *uuField2 |= uPatternBit;
            temp--;
        }
        break;
    case SB_FE2K_CLS_OP_WEQ:        /* WEQ */
        /* uuField2 has the anybit for WEQ */
        DECODE_PATTERN(uLocation, uRuleId, uPatternBit);
        *uuField2 = uPatternBit;

        temp = pOperation->uStart;
        *uuField1 = 0;
        while (temp >= pOperation->uEnd) {
            DECODE_PATTERN(uLocation, uRuleId, uPatternBit);
            *uuField1 <<= 1;
            *uuField1 |= ((*uuField2) ? 0 : uPatternBit);
            temp--;
        }
        break;

    case SB_FE2K_CLS_OP_LPM:        /* LPM */
        /* uuField2 has the length for LPM */
        uWidth = pOperation->uStart - pOperation->uEnd + 1;

        temp = pOperation->uEnd;
        uOffset = 0;
        *uuField1 = 0;
        *uuField3 = 0;
        /* decode don't care bit */
        uDontCareLength = 0;
        uPatternBit = 1;
        while (uPatternBit) {
            DECODE_PATTERN(uLocation, uRuleId, uPatternBit);
            uDontCareLength++;
            if (uDontCareLength > uWidth){
              break;
            }
        }
        /* don't count transition bit */
        uDontCareLength--;
        /* uuField2 has the length for the LPM */
        *uuField2 = uWidth - uDontCareLength;

        /* decode prefix */
        for (uOffset=*uuField2; uOffset > 0; uOffset--) {
          DECODE_PATTERN(uLocation, uRuleId, uPatternBit);
          uuTemp = uPatternBit;
          if (uOffset <= 64) {
            uuTemp <<= *uuField2 - uOffset + uDontCareLength;
            *uuField1 |= uuTemp;
          }else{
            *uuField3 <<= 1;
            *uuField3 |= uPatternBit;
            uuTemp <<= *uuField2 - (uOffset - 64) + uDontCareLength;
            *uuField3 |= uuTemp;
          }
        }
        break;

    default:
        break;
    }

    *pLocation = uLocation;

    return 0;
}
static void
sbFe2000ClsStuffRuleSet(sbFe2000ClsTemplate_t *pTemplate, 
                        uint32_t *pInstructions,
                        uint32_t bResultToLowerHalf,
                        uint32_t *pProgramSize,
                        uint32_t *pPadding,
                        uint32_t isC2)
{
  sbFe2000ClsInstructionFormat_t  zInstruction;
  uint32_t uInstructionCount;
  sbFe2000ClsTemplateField_t  *pOperation;
  uint32_t uId, uOpId, uWidth, uIndex, uUnpaddedInstructionCount, uPaddingModulus;

  if (isC2) {
    return;
  }

#define STUFF \
do { \
  if (pInstructions) { \
    pInstructions[uInstructionCount] = SB_FE2K_MAKE_CLS_INSTRUCTION_C1(zInstruction); \
  } \
  uInstructionCount++; \
} while (0)

  uInstructionCount = 0;
  uPaddingModulus = 4;

  sal_memset(&zInstruction, 0, sizeof(&zInstruction));
  zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_RESTART;
  zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 1100 */

  zInstruction.uSField    = SB_FE2K_CLS_SM_PATTERN_INVERSE;
  zInstruction.uMField    = SB_FE2K_CLS_SM_PATTERN_INVERSE;
  zInstruction.uAField    = 0;
  STUFF;

  /* walk each row of the template encoding the opcodes for each operation type */
  /* eg. Wildcard equate, range, lpm ( that seems to be all we are using for now ) */

  for(uOpId = 0; uOpId < pTemplate->uCount; uOpId++) {
    sal_memset(&zInstruction, 0, sizeof(zInstruction));

    /* get the current field operation */
    pOperation = &pTemplate->ops[uOpId];

    /* +1 is for total count of bits including the ends */
    uWidth = pOperation->uStart-pOperation->uEnd +1;

    switch(pOperation->eOperation)  {
    case SB_FE2K_CLS_OP_SUSPEND_IFNOT_PATTERN:
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uMField    = SB_FE2K_CLS_SM_PATTERN_INVERSE;
      zInstruction.uAField    =  0;
      STUFF;
      break;
    case SB_FE2K_CLS_OP_MULTIPORT:
      SB_ASSERT(pOperation->pAValArray);

      uId = 0;
      for(uIndex=0; uIndex < pOperation->uWidth-1; uIndex++)  {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_MULTIPORT; /* => 0011 */
        /* here the Suspend operation is really with (key[255:248] == AValue) and pattern */
        zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_AND_PATTERN;
        zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
        zInstruction.uAField    = pOperation->pAValArray[uId++]; /* user needs to fill in this value */
        STUFF;
      }
      uIndex = pOperation->uWidth;
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_MULTIPORT; /* => 0011 */
      /* here the Suspend operation is really with (key[255:248] == AValue) and pattern */
      zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_AND_PATTERN;
      zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_NAND_PATTERN;
      zInstruction.uAField    = pOperation->pAValArray[uId++]; /* user needs to fill in this value */
      STUFF;
      break;
    case SB_FE2K_CLS_OP_WEQ:
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_PATTERN_BIT;
      zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uAField    = 0;
      STUFF;

      for(uIndex=pOperation->uStart; uIndex > pOperation->uEnd; uIndex--) {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
        zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_NEQ_PATTERN;
        zInstruction.uAField    = uIndex;
        STUFF;
      }

      uIndex = pOperation->uEnd;
      zInstruction.c1c2.c2_op.uOpCode = SB_FE2K_CLS_OPCODE_C2_AFIELD_IS_KEY;
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_EQUAL_PATTERN;
      zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_NEQ_PATTERN;
      zInstruction.uAField    = uIndex;
      STUFF;

      break;
    case SB_FE2K_CLS_OP_EQ:
      for(uIndex=pOperation->uStart; uIndex > pOperation->uEnd; uIndex--) {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
        zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_NEQ_PATTERN;
        zInstruction.uAField    = uIndex;
        STUFF;
      }
      uIndex = pOperation->uEnd;
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_EQUAL_PATTERN;
      zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_NEQ_PATTERN;
      zInstruction.uAField    = uIndex;
      STUFF;
      break;
    case SB_FE2K_CLS_OP_LT:
      for(uIndex=pOperation->uStart; uIndex >= pOperation->uEnd;uIndex--) {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_LT_PATTERN;
        zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_GT_PATTERN;
        zInstruction.uAField    = uIndex;
        STUFF;
      }
      break;
    case SB_FE2K_CLS_OP_GT:
      for(uIndex=pOperation->uStart; uIndex >= pOperation->uEnd;uIndex--)  {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_GT_PATTERN;
        zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_LT_PATTERN;
        zInstruction.uAField    = uIndex;
        STUFF;
      }
      break;
    case SB_FE2K_CLS_OP_ONE_OF:
      for(uIndex=pOperation->uStart; uIndex > pOperation->uEnd;uIndex--) {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_AND_PATTERN;
        zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
        zInstruction.uAField    = uIndex;
        STUFF;
      }

      uIndex = pOperation->uEnd;
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_AND_PATTERN;
      zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_NAND_PATTERN;
      zInstruction.uAField    = uIndex;
      STUFF;
      break;
    case SB_FE2K_CLS_OP_RANGE:
      /* Greater than Part */
      for(uIndex=pOperation->uStart; uIndex >= pOperation->uEnd;uIndex--) {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_GT_PATTERN;
        zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_LT_PATTERN;
        zInstruction.uAField    = uIndex;
        STUFF;
      }

      /* resume in the middle */
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_RESUME;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0100 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uAField    = uIndex;
      STUFF;
     

      /* less than part */
      for(uIndex=pOperation->uStart; uIndex >= pOperation->uEnd;uIndex--) {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_KEY_LT_PATTERN;
        zInstruction.uMField    = SB_FE2K_CLS_SM_KEY_GT_PATTERN;
        zInstruction.uAField    = uIndex;
        STUFF;
      }
      break;
    case SB_FE2K_CLS_OP_LPM:
      /* burn the start of the lpm signature */
      uIndex = pOperation->uEnd;
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_LPM; /* => 0010 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uAField    = uIndex;
      STUFF;

      /* run through the lpm instructions */
      for(uIndex=pOperation->uEnd; uIndex < pOperation->uStart;uIndex++) {
        zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
        zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_LPM; /* => 0010 */
        zInstruction.uSField    = SB_FE2K_CLS_SM_PATTERN_INVERSE;
        zInstruction.uMField    = SB_FE2K_CLS_SM_PATTERN_INVERSE;
        zInstruction.uAField    = uIndex;
        STUFF;
      }

      /* burn the end of the lpm signature */
      uIndex = pOperation->uStart;
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_LPM; /* => 0010 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ONE;
      zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ONE;
      zInstruction.uAField    = uIndex;
      STUFF;
      break;
    default:
      SB_LOG("unexpected classifier opcode: 0x%x\n", 
             pOperation->eOperation);
      break;
    } /* end switch on operation */

    /* each rule / operation is followed by a resume instruction */

    if(uOpId < pTemplate->uCount-1) { /* if not at last row of template */
      zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_RESUME;
      zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0100 */
      zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
      zInstruction.uAField    = 0;
      STUFF;

    } /* end if not at end of template */
  } /* end for walking all the rows in the template */

  /* Now we've finished the filter set, encode a "new rule" instruction as a filter set separator 
   */
  zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NEW_RULE;
  zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 1000 */
  zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
  zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
  zInstruction.uAField    =
    bResultToLowerHalf ? CLS_SAVE_TO_LOWER_SWORD: CLS_SAVE_TO_HIGHER_SWORD;
  STUFF;

  uUnpaddedInstructionCount = uInstructionCount;
  /* what follows is no-op padding to take us to a mod 4 boundary for c1 or mod 8 for c2 */


  while (uInstructionCount % uPaddingModulus) {
    zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NORMAL_INSTRUCTION;
    zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY; /* => 0000 */
    zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
    zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
    zInstruction.uAField    = 0;
    STUFF;
  }

  /*
   * This instruction is not executed `in-place' here.  It is
   * copied over the `real' NEW_RULE instruction (generated above
   * before the padding) if the result is supposed to be delivered to
   * the other half of the classifier RCE.
   */
  zInstruction.c1c2.c1_op.uOpCodeMsb = SB_FE2K_CLS_OPCODE_MSB_NEW_RULE;
  zInstruction.c1c2.c1_op.uOpCodeLsb = SB_FE2K_CLS_OPCODE_LSB_AFIELD_IS_KEY;
  zInstruction.uSField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
  zInstruction.uMField    = SB_FE2K_CLS_SM_FUNCTION_ZERO;
  zInstruction.uAField    =
    bResultToLowerHalf ? CLS_SAVE_TO_HIGHER_SWORD : CLS_SAVE_TO_LOWER_SWORD;
  STUFF;

  *pProgramSize = uInstructionCount;
  *pPadding = uInstructionCount - uUnpaddedInstructionCount;
#undef STUFF
}


sbStatus_t 
sbFe2000ClsGenerateInstructions( sbFe2000ClsTemplate_t *pTemplate, 
                                 uint32_t **ppInstructions, 
                                 uint32_t *pCount,
                                 uint32_t *pRealCount,
                                 uint32_t bResultToLowerHalf,
                                 uint32_t isC2)
{
  uint32_t *pInstructions;
  uint32_t uProgramSize, uPadding;
  sbStatus_t status;

  /* The first call to sbFe2000ClsStuffRuleSet with the NULL pInstructions doesn't generate
   * any instructions, its sole purpose is to figure out the program size so we can
   * malloc the right amount of memory
   */
  sbFe2000ClsStuffRuleSet(pTemplate, NULL, bResultToLowerHalf, &uProgramSize,
                          &uPadding, isC2);

  if(pTemplate->bDebug) {
    SB_LOGV3("Program Size %d\n", uProgramSize); 
  }

  status = thin_malloc(0, SB_ALLOC_INTERNAL,
                       uProgramSize * sizeof(uint32_t), 
                       (void **) &pInstructions, 0);

  if (status != SB_OK) {
    return status;
  }

  SB_MEMSET(pInstructions, 0, uProgramSize * sizeof(uint32_t));


  sbFe2000ClsStuffRuleSet(pTemplate, pInstructions, bResultToLowerHalf,
                          &uProgramSize, &uPadding, isC2 );

  /*
   * The returned pCount doesn't include the `extra' NEW_RULE instruction
   * generated  above by StuffRuleSet.  Again, really stupid argument
   * passing convention (confusing), but some day this will all be replaced
   * by a robot script.  There's some jobs you don't mind eliminating.
   */
  *pCount         = uProgramSize - 1;
  *ppInstructions = pInstructions;
  *pRealCount = uProgramSize - uPadding;

  /* 
   * from the program size we should be able to calculate the number of
   * rules. from the following calculation given rtl verifcation team.
   * For Max. Instruction Size is found through Waves.  It was first calcuated 
   * by epoch length * 4 since the LR would do a context switch that would take a 
   * full epoch. Then subtract the load and unload time for the RCE.  There is 
   * also load and unload time for the LR and so the lenght was shorten from 
   * 1612 (403 cycles) to 1088 (272 cycles).
   *
   * LR Key call to LR sends Keys to RCE - 20 Cycles (LR fills the Key buffer in LR)
   * LR sends Key to LR sends start RCE program - 47 Cycles (resource shadow before running RC)
   * LR sends start program to RCE start program - 56 Cycles (48 clocks to load Key Buffer and more)
   * RCE Start Program to RCE done program - 272 cycles (running program)
   * RCE done program to RCE results out - 14 cycles
   * RCE results out to LR read result registers CRR - 8 cycles
   * Total cycles = 407
   * Epoch = 403
   * End of Switch to using CRR register (4 nops).
   * 
   * uRuleSets = 1088/# of instructions
   */

  if(pTemplate->bDebug) {
    SB_LOGV1("CLS Program Size After Adjustment %d\n", uProgramSize - 1);
  }

  return SB_OK;
}


sbStatus_t
sbFe2000ClsGenerateInstructions_c2( sbFe2000ClsTemplate_t *pTemplate,
                                    uint32_t sType,
                                    uint32_t **ppInstructions,
                                    uint32_t *pCount,
                                    uint32_t *pRealCount,
                                    uint32_t bResultToLowerHalf)
{
  uint32_t *pInstructions;
  uint32_t uProgramSize=-1, uPadding;
  sbStatus_t status;

  sbFe2000XtClsEncodeInstructions(pTemplate, NULL, bResultToLowerHalf, &uProgramSize, &uPadding);

  status = thin_malloc(0, SB_ALLOC_INTERNAL,
                       uProgramSize * sizeof(uint32_t), 
                       (void **) &pInstructions, 0);

  if (status != SB_OK) {
    return status;
  }

  SB_MEMSET(pInstructions, 0, uProgramSize * sizeof(uint32_t));

  sbFe2000XtClsEncodeInstructions(pTemplate, pInstructions, bResultToLowerHalf, &uProgramSize, &uPadding);

  *pCount         = uProgramSize - 1;
  *ppInstructions = pInstructions;
  *pRealCount = uProgramSize - uPadding;

  return SB_OK;
}  
