/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000PmProfileTimerMemory.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_FE_2000_PM_PROFTIMERMEMORY_H
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_H

#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES 8
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE 8
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_URESV0_BITS "63:63"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_UTYPE_BITS "62:62"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_UPROFMODE_BITS "61:60"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_URESV1_BITS "59:32"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_URESV2_BITS "31:24"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_BINTERRUPT_BITS "23:23"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_BRESET_BITS "22:22"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_BSTRICT_BITS "21:21"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_BSTARTED_BITS "20:20"
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_UDEADLINE_BITS "19:0"


#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_WORDS   ((SB_ZF_FE_2000_PM_PROFTIMERMEMORY_SIZE_IN_BYTES+3)/4)


/*
 * sbG2FePmProfileType_e

typedef enum sbG2FePmProfileType_e {
  SB_FE_2000_PM_PTYPE_TIMER = 0,
  SB_FE_2000_PM_PTYPE_SEQGEN,
  SB_FE_2000_PM_PTYPE_SEMAPHORE,
} sbG2FePmProfileType_t;
 */


/** @brief  PM Profile Timer memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfFe2000PmProfileTimerMemory {
  uint32_t uResv0;
  uint32_t uType;
  uint32_t uProfMode;
  uint32_t uResv1;
  uint32_t uResv2;
  uint32_t bInterrupt;
  uint32_t bReset;
  uint32_t bStrict;
  uint32_t bStarted;
  uint32_t uDeadline;
} sbZfFe2000PmProfileTimerMemory_t;

uint32_t
sbZfFe2000PmProfileTimerMemory_Pack(sbZfFe2000PmProfileTimerMemory_t *pFrom,
                                    uint8_t *pToData,
                                    uint32_t nMaxToDataIndex);
void
sbZfFe2000PmProfileTimerMemory_Unpack(sbZfFe2000PmProfileTimerMemory_t *pToStruct,
                                      uint8_t *pFromData,
                                      uint32_t nMaxToDataIndex);
void
sbZfFe2000PmProfileTimerMemory_InitInstance(sbZfFe2000PmProfileTimerMemory_t *pFrame);

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_PROFMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_RESV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x0f) | (((nFromData) >> 24) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_RESV2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_BINTERRUPT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_BRESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_BSTRICT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_BSTARTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_SET_DEADLINE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_PROFMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_RESV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) (pFromData)[1] << 16; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x0f) << 24; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_RESV2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_BINTERRUPT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_BRESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_BSTRICT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_BSTARTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILETIMERMEMORY_GET_DEADLINE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
           (nToData) |= (uint32_t) (pFromData)[6] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x0f) << 16; \
          } while(0)

/**
 * Return 1 if same else 0.
 */
uint8_t sbZfFe2000PmProfileTimerMemory_Compare( sbZfFe2000PmProfileTimerMemory_t *pProfile1,
                                              sbZfFe2000PmProfileTimerMemory_t *pProfile2);

void sbZfFe2000PmProfileTimerMemory_Copy( sbZfFe2000PmProfileTimerMemory_t *pSource,
                                        sbZfFe2000PmProfileTimerMemory_t *pDest);

uint32_t sbZfFe2000PmProfileTimerMemory_Pack32( sbZfFe2000PmProfileTimerMemory_t *pFrom, 
                                              uint32_t *pToData, 
                                              uint32_t nMaxToDataIndex);

uint32_t sbZfFe2000PmProfileTimerMemory_Unpack32(sbZfFe2000PmProfileTimerMemory_t *pToData,
                                               uint32_t *pFrom,
                                               uint32_t nMaxToDataIndex);
#endif
