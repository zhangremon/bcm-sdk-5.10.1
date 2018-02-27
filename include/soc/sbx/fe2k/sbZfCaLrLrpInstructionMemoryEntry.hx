/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaLrLrpInstructionMemoryEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_H
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_H

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SIZE_IN_BYTES 12
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SIZE 12
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_UTHREAD_BITS "95:93"
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_UOPCODE_BITS "92:84"
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_URA_BITS "83:83"
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_URB_BITS "82:82"
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_UTARGET_BITS "81:64"
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_USOURCEA_BITS "63:32"
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_USOURCEB_BITS "31:16"
#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_M_URESERVED_BITS "15:0"


typedef struct _sbZfCaLrLrpInstructionMemoryEntry {
  uint32_t m_uThread;
  uint32_t m_uOpcode;
  uint32_t m_uRa;
  uint32_t m_uRb;
  uint32_t m_uTarget;
  uint32_t m_uSourceA;
  uint32_t m_uSourceB;
  uint32_t m_uReserved;
} sbZfCaLrLrpInstructionMemoryEntry_t;

uint32_t
sbZfCaLrLrpInstructionMemoryEntry_Pack(sbZfCaLrLrpInstructionMemoryEntry_t *pFrom,
                                       uint8_t *pToData,
                                       uint32_t nMaxToDataIndex);
void
sbZfCaLrLrpInstructionMemoryEntry_Unpack(sbZfCaLrLrpInstructionMemoryEntry_t *pToStruct,
                                         uint8_t *pFromData,
                                         uint32_t nMaxToDataIndex);
void
sbZfCaLrLrpInstructionMemoryEntry_InitInstance(sbZfCaLrLrpInstructionMemoryEntry_t *pFrame);

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_THREAD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_OPCODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x1f) | (((nFromData) >> 4) & 0x1f); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_RA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_RB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_TARGET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x03) | (((nFromData) >> 16) & 0x03); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_SOURCEA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_SOURCEB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_THREAD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[11] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_OPCODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[11] & 0x1f) << 4; \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_RA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_RB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_TARGET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x03) << 16; \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_SOURCEA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_SOURCEB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_CALRLRPINSTRUCTIONMEMORYENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
          } while(0)

#endif
