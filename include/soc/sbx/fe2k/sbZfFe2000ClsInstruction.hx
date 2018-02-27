/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000ClsInstruction.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_FE2000CLSINSTRUCTION_H
#define SB_ZF_FE2000CLSINSTRUCTION_H

#define SB_ZF_FE2000CLSINSTRUCTION_SIZE_IN_BYTES 3
#define SB_ZF_FE2000CLSINSTRUCTION_SIZE 3
#define SB_ZF_FE2000CLSINSTRUCTION_UOPCODEMSB_BITS "19:18"
#define SB_ZF_FE2000CLSINSTRUCTION_UOPCODELSB_BITS "17:16"
#define SB_ZF_FE2000CLSINSTRUCTION_USFIELD_BITS "15:12"
#define SB_ZF_FE2000CLSINSTRUCTION_UMFIELD_BITS "11:8"
#define SB_ZF_FE2000CLSINSTRUCTION_UAFIELD_BITS "7:0"



#define SB_FE2K_MAKE_CLS_INSTRUCTION(x)   ( (x.uOpCodeMsb << 18) | \
                                            (x.uOpCodeLsb << 16) | \
                                            (x.uSField    << 12) | \
                                            (x.uMField    << 8 ) | \
                                            (x.uAField) )

typedef struct _sbZfFe2000ClsInstruction {
  uint32_t uOpCodeMsb;
  uint32_t uOpCodeLsb;
  uint32_t uSField;
  uint32_t uMField;
  uint32_t uAField;
} sbZfFe2000ClsInstruction_t;

uint32_t
sbZfFe2000ClsInstruction_Pack(sbZfFe2000ClsInstruction_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex);
void
sbZfFe2000ClsInstruction_Unpack(sbZfFe2000ClsInstruction_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex);
void
sbZfFe2000ClsInstruction_InitInstance(sbZfFe2000ClsInstruction_t *pFrame);

#define SB_ZF_FE2000CLSINSTRUCTION_SET_OPMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 2)) | (((nFromData) & 0x03) << 2); \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_SET_OPLSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_SET_SFLD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_SET_MFLD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_SET_AFLD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_GET_OPMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x03; \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_GET_OPLSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_GET_SFLD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_GET_MFLD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000CLSINSTRUCTION_GET_AFLD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
