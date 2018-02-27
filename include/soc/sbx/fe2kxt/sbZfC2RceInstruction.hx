/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2RceInstruction.hx 1.3.16.4 Broadcom SDK $
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
 */


#ifndef SB_ZF_ZFC2RCEINSTRUCTION_H
#define SB_ZF_ZFC2RCEINSTRUCTION_H

#define SB_ZF_ZFC2RCEINSTRUCTION_SIZE_IN_BYTES 3
#define SB_ZF_ZFC2RCEINSTRUCTION_SIZE 3
#define SB_ZF_ZFC2RCEINSTRUCTION_M_UOPCODE_BITS "20:17"
#define SB_ZF_ZFC2RCEINSTRUCTION_M_UOPCODESUSPENSION_BITS "16:13"
#define SB_ZF_ZFC2RCEINSTRUCTION_M_UOPCODEMISMATCH_BITS "12:9"
#define SB_ZF_ZFC2RCEINSTRUCTION_M_UOPERANDA_BITS "8:0"
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_NORMAL_INSTR_A_KEY_FIELD 0
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_NORMAL_INSTR_A_RESULT    1   // OPRA Bit 7 select Result Register 4:0 selects bits
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_LPM                      2
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_MULTIPORT                3   // Directly compare 255:248 with OPRA
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_NEW_RULE_SLOT0           8   // OPRA 5:0 indicates set number A[3:2]/A[1:0] 00:Cx=0, 01:Cx=1, 10:Cx=1 if Filter equal, 11:inclusive  
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_NEW_RULE_SLOT1           9   // OPRA 5:0 indicates set number
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_RESUME                   4   // Limited to modulo 8 instructions
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_RESTART                 12
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_JUMP_NOT_END            14
#define SB_ZF_ZFC2RCEINSTRUCTION_OP_JUMP_END_NEXT           15
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_ONE           10
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_ZERO          5
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_XNOR          12
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_XOR           3
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_GREATER_THAN  1
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_LESS_THAN     7
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_OR            11
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_NOR           4
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_AND           13
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_NAND          2
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_KEY_INV       6
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_KEY           9
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_PAT           15
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_PAT_INV       0
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_MASK_INV_KEY  14
#define SB_ZF_ZFC2RCEINSTRUCTION_SM_MASK_INV_PAT  8
#define SB_ZF_ZFC2RCEINSTRUCTION_NEW_RULE_S0    0 //20'h8_000X --OP_NEW_RULE_SLOT0,  SM_PAT_INV, SM_PAT_INV,   X (reg X)
#define SB_ZF_ZFC2RCEINSTRUCTION_NEW_RULE_S1   48 //20'h9_000X --OP_NEW_RULE_SLOT1,  SM_PAT_INV, SM_PAT_INV,   X (reg X)
#define SB_ZF_ZFC2RCEINSTRUCTION_RESTART        1 //20'hC_0000 --  OP_RESTART,   SM_PAT_INV, SM_PAT_INV,    0
#define SB_ZF_ZFC2RCEINSTRUCTION_LPM            2 //20'h2_0000 --  OP_LPM,       SM_PAT_INV, SM_PAT_INV,    0
#define SB_ZF_ZFC2RCEINSTRUCTION_RESUME         3 //20'h4_5500 --  OP_RESUME,    SM_ZERO,    SM_ZERO,       0
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT      4 // 20'h3_5600    OP_MULTIPORT  SM_ZERO,    SM_KEY_INV,    0
#define SB_ZF_ZFC2RCEINSTRUCTION_JUMP          49 // 4'hE,5,0      OP_JUMP_NOT_END  SM_ZERO, SM_PAT_INV     0
#define SB_ZF_ZFC2RCEINSTRUCTION_JUMP_END_NEXT 50 // 4'hF,5,0      OP_JUMP_END_NEXT SM_ZERO, SM_PAT_INV     0
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_PAT      6 //     OP_NORMAL_INSTR_A_KEY_FIELD,  SM_PAT,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_KEY      7 //     OP_NORMAL_INSTR_A_KEY_FIELD,  SM_KEY,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_NAND     8 //     OP_NORMAL_INSTR_A_KEY_FIELD,  SM_NAND, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_AND      9 //     OP_NORMAL_INSTR_A_KEY_FIELD,  SM_AND,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_EQ       10 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_ZERO, SM_XOR,  user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_NEQ      11 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_XOR,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_KEYINV   12 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_KEY_INV, SM_ZERO, user_a[7:0] 
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_PATINV   13 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_KEY_INV, SM_ZERO, user_a[7:0] 
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_MASKINVK 14 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_MASK_INV_KEY, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_MASKINVP 15 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_MASK_INV_PAT, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_SUSPEND  16 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_ONE,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_MISMATCH 17 //    OP_NORMAL_INSTR_A_KEY_FIELD,  SM_ZERO,  SM_ONE, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_LESS_THAN 18//   OP_NORMAL_INSTR_A_KEY_FIELD,  SM_LESS_THAN,   SM_GREATER_THAN,   user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_KEY_GREATER_THAN  19// OP_NORMAL_INSTR_A_KEY_FIELD,  SM_GREATER_THAN,   SM_LESS_THAN,   user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_PAT      20// OP_NORMAL_INSTR_A_RESULT, SM_PAT,     SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_KEY      21// OP_NORMAL_INSTR_A_RESULT, SM_KEY,     SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_NAND     22// OP_NORMAL_INSTR_A_RESULT, SM_NAND,    SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_AND      23// OP_NORMAL_INSTR_A_RESULT, SM_ZERO,    SM_AND,  user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_EQ       24// OP_NORMAL_INSTR_A_RESULT, SM_ZERO,    SM_XOR,  user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_NEQ      25// OP_NORMAL_INSTR_A_RESULT, SM_XOR,     SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_KEYINV   26// OP_NORMAL_INSTR_A_RESULT, SM_KEY_INV, SM_ZERO, user_a[7:0] 
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_PATINV   27// OP_NORMAL_INSTR_A_RESULT, SM_KEY_INV, SM_ZERO, user_a[7:0] 
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_MASKINVK 28// OP_NORMAL_INSTR_A_RESULT, SM_MASK_INV_KEY, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_MASKINVP 29// OP_NORMAL_INSTR_A_RESULT, SM_MASK_INV_PAT, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_SUSPEND  30// OP_NORMAL_INSTR_A_RESULT, SM_ONE,     SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_MISMATCH 31// OP_NORMAL_INSTR_A_RESULT, SM_ZERO,    SM_ONE, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_LESS_THAN  32// OP_NORMAL_INSTR_A_RESULT  SM_LESS_THAN,   SM_GREATER_THAN,   user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_RESULT_GREATER_THAN      33// OP_NORMAL_INSTR_A_RESULT  SM_GREATER_THAN,   SM_LESS_THAN,   user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_PAT      34// OP_MULTIPORT,  SM_PAT,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_KEY      35// OP_MULTIPORT,  SM_KEY,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_NAND     36// OP_MULTIPORT,  SM_NAND, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_AND      37// OP_MULTIPORT,  SM_ZERO, SM_AND,  user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_EQ       38// OP_MULTIPORT,  SM_ZERO, SM_XOR,  user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_NEQ      39// OP_MULTIPORT,  SM_XOR,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_KEYINV   40// OP_MULTIPORT,  SM_KEY_INV, SM_ZERO, user_a[7:0] 
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_PATINV   41// OP_MULTIPORT,  SM_KEY_INV, SM_ZERO, user_a[7:0] 
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_MASKINVK 42// OP_MULTIPORT,  SM_MASK_INV_KEY, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_MASKINVP 43// OP_MULTIPORT,  SM_MASK_INV_PAT, SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_SUSPEND  44// OP_MULTIPORT,  SM_ONE,  SM_ZERO, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_MISMATCH 45// OP_MULTIPORT,  SM_ZERO,  SM_ONE, user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_LESS_THAN  46// OP_MULTIPORT,  SM_LESS_THAN,   SM_GREATER_THAN,   user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_MULTIPORT_GREATER_THAN      47// OP_MULTIPORT,  SM_GREATER_THAN,   SM_LESS_THAN,   user_a[7:0]
#define SB_ZF_ZFC2RCEINSTRUCTION_USE_SPRINT


typedef struct _sbZfC2RceInstruction {
  uint32_t m_uOpcode;
  uint32_t m_uOpcodeSuspension;
  uint32_t m_uOpcodeMismatch;
  uint32_t m_uOperandA;
} sbZfC2RceInstruction_t;

uint32_t
sbZfC2RceInstruction_Pack(sbZfC2RceInstruction_t *pFrom,
                          uint8_t *pToData,
                          uint32_t nMaxToDataIndex);
void
sbZfC2RceInstruction_Unpack(sbZfC2RceInstruction_t *pToStruct,
                            uint8_t *pFromData,
                            uint32_t nMaxToDataIndex);
void
sbZfC2RceInstruction_InitInstance(sbZfC2RceInstruction_t *pFrame);

#define SB_ZF_C2RCEINSTRUCTION_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_C2RCEINSTRUCTION_SET_OPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_C2RCEINSTRUCTION_SET_OPM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_C2RCEINSTRUCTION_SET_OPRA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_C2RCEINSTRUCTION_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_C2RCEINSTRUCTION_GET_OPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 3; \
          } while(0)

#define SB_ZF_C2RCEINSTRUCTION_GET_OPM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_C2RCEINSTRUCTION_GET_OPRA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 8; \
          } while(0)

#endif
