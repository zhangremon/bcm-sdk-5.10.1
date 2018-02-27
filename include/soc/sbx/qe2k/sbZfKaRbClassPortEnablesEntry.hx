/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassPortEnablesEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKARBCLASSPORTENABLESENTRY_H
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_H

#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE 4
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NRESERVE_BITS "31:23"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEUSER1_BITS "22:22"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEUSER0_BITS "21:21"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEVLANPRI_BITS "20:20"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEHIPRIVLAN_BITS "19:19"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEDMACMATCH_BITS "18:18"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSELAYER4_BITS "17:17"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NFLOWHASHENABLE_BITS "16:16"
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_M_NUSEHASHCOS_BITS "15:0"


typedef struct _sbZfKaRbClassPortEnablesEntry {
  uint32_t m_nReserve;
  uint8_t m_nUseUser1;
  uint8_t m_nUseUser0;
  uint8_t m_nUseVlanPri;
  uint8_t m_nUseHiPriVlan;
  uint8_t m_nUseDmacMatch;
  uint8_t m_nUseLayer4;
  uint8_t m_nFlowHashEnable;
  uint32_t m_nUseHashCos;
} sbZfKaRbClassPortEnablesEntry_t;

uint32_t
sbZfKaRbClassPortEnablesEntry_Pack(sbZfKaRbClassPortEnablesEntry_t *pFrom,
                                   uint8_t *pToData,
                                   uint32_t nMaxToDataIndex);
void
sbZfKaRbClassPortEnablesEntry_Unpack(sbZfKaRbClassPortEnablesEntry_t *pToStruct,
                                     uint8_t *pFromData,
                                     uint32_t nMaxToDataIndex);
void
sbZfKaRbClassPortEnablesEntry_InitInstance(sbZfKaRbClassPortEnablesEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_RESERVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEUSER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEVLANPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHIPRIVLAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEDMACMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USELAYER4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_FLOWHASHENB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_SET_USEHASHCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[0] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[3] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[0] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_RESERVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[3] << 1; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEUSER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEVLANPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHIPRIVLAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEDMACMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USELAYER4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_FLOWHASHENB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPORTENABLESENTRY_GET_USEHASHCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
