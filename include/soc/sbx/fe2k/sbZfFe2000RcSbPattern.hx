/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000RcSbPattern.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_FE2000RCSBPATTERN_H
#define SB_ZF_FE2000RCSBPATTERN_H

#define SB_ZF_FE2000RCSBPATTERN_SIZE_IN_BYTES 16
#define SB_ZF_FE2000RCSBPATTERN_SIZE 16
#define SB_ZF_FE2000RCSBPATTERN_UPATTERN3_BITS "127:96"
#define SB_ZF_FE2000RCSBPATTERN_UPATTERN2_BITS "95:64"
#define SB_ZF_FE2000RCSBPATTERN_UPATTERN1_BITS "63:32"
#define SB_ZF_FE2000RCSBPATTERN_UPATTERN0_BITS "31:0"



typedef struct
{
  uint32_t *pPattern[4];
} sbZfFe2000RcSbPatternAlias_t;

typedef struct _sbZfFe2000RcSbPattern {
  uint32_t uPattern3;
  uint32_t uPattern2;
  uint32_t uPattern1;
  uint32_t uPattern0;
} sbZfFe2000RcSbPattern_t;

uint32_t
sbZfFe2000RcSbPattern_Pack(sbZfFe2000RcSbPattern_t *pFrom,
                           uint8_t *pToData,
                           uint32_t nMaxToDataIndex);
void
sbZfFe2000RcSbPattern_Unpack(sbZfFe2000RcSbPattern_t *pToStruct,
                             uint8_t *pFromData,
                             uint32_t nMaxToDataIndex);
void
sbZfFe2000RcSbPattern_InitInstance(sbZfFe2000RcSbPattern_t *pFrame);

#define SB_ZF_FE2000RCSBPATTERN_SET_PAT3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000RCSBPATTERN_SET_PAT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000RCSBPATTERN_SET_PAT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000RCSBPATTERN_SET_PAT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000RCSBPATTERN_GET_PAT3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
           (nToData) |= (uint32_t) (pFromData)[13] << 8; \
           (nToData) |= (uint32_t) (pFromData)[14] << 16; \
           (nToData) |= (uint32_t) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_FE2000RCSBPATTERN_GET_PAT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) (pFromData)[10] << 16; \
           (nToData) |= (uint32_t) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_FE2000RCSBPATTERN_GET_PAT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_FE2000RCSBPATTERN_GET_PAT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

void sbZfFe2000RcSbSetRuleBit( sbZfFe2000RcSbPattern_t *pFrame, 
                               uint32_t uRuleId, 
                               uint32_t uValue);
void sbZfFe2000RcSbGetRuleBit( sbZfFe2000RcSbPattern_t *pFrame, 
                               uint32_t uRuleId, 
                               uint8_t *uValue);
void sbZfFe2000RcSbPatternAlias( sbZfFe2000RcSbPattern_t *pFrame, 
                                 sbZfFe2000RcSbPatternAlias_t *pTo);
uint32_t sbZfFe2000RcSbPatternIsNull( sbZfFe2000RcSbPattern_t *pFrameBase,
                                      uint32_t uRuleId,
                                      uint32_t uInstCount);
#endif
