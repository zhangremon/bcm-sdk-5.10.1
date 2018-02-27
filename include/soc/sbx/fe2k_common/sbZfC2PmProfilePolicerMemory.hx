/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PmProfilePolicerMemory.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2_PM_PROFPOLICERMEMORY_H
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_H

#define SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES 12
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE 12
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UTYPE_BITS "74:74"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_ULENGTHSHIFT_BITS "73:71"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BCOUPLINGFLAG_BITS "70:70"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BRFC2698MODE_BITS "69:69"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UBKTCSIZE_BITS "68:64"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BBLIND_BITS "63:63"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BBKTENODECREMENT_BITS "62:62"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UEIR_BITS "61:44"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UEBS_BITS "43:32"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BDROPONRED_BITS "31:31"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BBKTCNODECREMENT_BITS "30:30"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UCIR_BITS "29:12"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UCBS_BITS "11:0"


#define SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_WORDS   ((SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES+3)/4)



/** @brief  PM Profile Policer memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfC2PmProfilePolicerMemory {
  uint32_t uType;
  uint32_t uLengthShift;
  uint32_t bCouplingFlag;
  uint32_t bRFC2698Mode;
  uint32_t uBktCSize;
  uint32_t bBlind;
  uint32_t bBktENoDecrement;
  uint32_t uEIR;
  uint32_t uEBS;
  uint32_t bDropOnRed;
  uint32_t bBktCNoDecrement;
  uint32_t uCIR;
  uint32_t uCBS;
} sbZfC2PmProfilePolicerMemory_t;

uint32_t
sbZfC2PmProfilePolicerMemory_Pack(sbZfC2PmProfilePolicerMemory_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex);
void
sbZfC2PmProfilePolicerMemory_Unpack(sbZfC2PmProfilePolicerMemory_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex);
void
sbZfC2PmProfilePolicerMemory_InitInstance(sbZfC2PmProfilePolicerMemory_t *pFrame);

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_LENSHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_CFLAG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_R2698(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BCSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BENODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_EIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x3f) | (((nFromData) >> 12) & 0x3f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_EBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_DRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BCNODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_CIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~ 0x3f) | (((nFromData) >> 12) & 0x3f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_CBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_LENSHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_CFLAG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_R2698(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BCSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x1f; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BENODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_EIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[5] << 4; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x3f) << 12; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_EBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_DRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BCNODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_CIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[9] << 4; \
           (nToData) |= (uint32_t) ((pFromData)[8] & 0x3f) << 12; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_CBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[11] ; \
           (nToData) |= (uint32_t) ((pFromData)[10] & 0x0f) << 8; \
          } while(0)

/**
 * Return 1 if same else 0.
 */
uint8_t sbZfC2PmProfilePolicerMemory_Compare( sbZfC2PmProfilePolicerMemory_t *pProfile1,
                                                sbZfC2PmProfilePolicerMemory_t *pProfile2);
void sbZfC2PmProfilePolicerMemory_Copy( sbZfC2PmProfilePolicerMemory_t *pSource,
                                          sbZfC2PmProfilePolicerMemory_t *pDest);

uint32_t sbZfC2PmProfilePolicerMemory_Pack32( sbZfC2PmProfilePolicerMemory_t *pFrom, 
                                                uint32_t *pToData, 
                                                uint32_t nMaxToDataIndex);

uint32_t sbZfC2PmProfilePolicerMemory_Unpack32(sbZfC2PmProfilePolicerMemory_t *pToData,
                                                 uint32_t *pFrom,
                                                 uint32_t nMaxToDataIndex);
#endif
