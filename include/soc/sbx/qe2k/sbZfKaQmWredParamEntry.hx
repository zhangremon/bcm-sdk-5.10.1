/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmWredParamEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAQMWREDPARAMENTRY_H
#define SB_ZF_ZFKAQMWREDPARAMENTRY_H

#define SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE 8
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NTMAXEXCEEDED2_BITS "35:35"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NECNEXCEEDED2_BITS "34:34"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NPDROP2_BITS "33:24"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NTMAXEXCEEDED1_BITS "23:23"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NECNEXCEEDED1_BITS "22:22"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NPDROP1_BITS "21:12"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NTMAXEXCEEDED0_BITS "11:11"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NECNEXCEEDED0_BITS "10:10"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NPDROP0_BITS "9:0"


typedef struct _sbZfKaQmWredParamEntry {
  uint8_t m_nTMaxExceeded2;
  uint8_t m_nEcnExceeded2;
  uint32_t m_nPDrop2;
  uint8_t m_nTMaxExceeded1;
  uint8_t m_nEcnExceeded1;
  uint32_t m_nPDrop1;
  uint8_t m_nTMaxExceeded0;
  uint8_t m_nEcnExceeded0;
  uint32_t m_nPDrop0;
} sbZfKaQmWredParamEntry_t;

uint32_t
sbZfKaQmWredParamEntry_Pack(sbZfKaQmWredParamEntry_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex);
void
sbZfKaQmWredParamEntry_Unpack(sbZfKaQmWredParamEntry_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex);
void
sbZfKaQmWredParamEntry_InitInstance(sbZfKaQmWredParamEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
