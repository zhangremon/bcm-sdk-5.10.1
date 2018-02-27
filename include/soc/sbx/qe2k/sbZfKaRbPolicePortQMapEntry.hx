/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbPolicePortQMapEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKARBPOLPORTQMAPENTRY_H
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_H

#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_SIZE 4
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NRESERVED2_BITS "31:25"
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NODDMETER_BITS "24:16"
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NRESERVED1_BITS "15:9"
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NEVENMETER_BITS "8:0"


typedef struct _sbZfKaRbPolicePortQMapEntry {
  uint32_t m_nReserved2;
  uint32_t m_nOddMeter;
  uint32_t m_nReserved1;
  uint32_t m_nEvenMeter;
} sbZfKaRbPolicePortQMapEntry_t;

uint32_t
sbZfKaRbPolicePortQMapEntry_Pack(sbZfKaRbPolicePortQMapEntry_t *pFrom,
                                 uint8_t *pToData,
                                 uint32_t nMaxToDataIndex);
void
sbZfKaRbPolicePortQMapEntry_Unpack(sbZfKaRbPolicePortQMapEntry_t *pToStruct,
                                   uint8_t *pFromData,
                                   uint32_t nMaxToDataIndex);
void
sbZfKaRbPolicePortQMapEntry_InitInstance(sbZfKaRbPolicePortQMapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 8; \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 8; \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 8; \
          } while(0)

#endif
#endif
