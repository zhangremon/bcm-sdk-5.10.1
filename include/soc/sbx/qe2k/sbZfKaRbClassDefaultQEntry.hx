/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassDefaultQEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKARBCLASSDEFAULTQENTRY_H
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_H

#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_SIZE 4
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NSPARE1_BITS "31:18"
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NDEFAULTDP_BITS "17:16"
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NSPARE0_BITS "15:14"
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NDEFAULTQ_BITS "13:0"


typedef struct _sbZfKaRbClassDefaultQEntry {
  uint32_t m_nSpare1;
  uint32_t m_nDefaultDp;
  uint32_t m_nSpare0;
  uint32_t m_nDefaultQ;
} sbZfKaRbClassDefaultQEntry_t;

uint32_t
sbZfKaRbClassDefaultQEntry_Pack(sbZfKaRbClassDefaultQEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex);
void
sbZfKaRbClassDefaultQEntry_Unpack(sbZfKaRbClassDefaultQEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex);
void
sbZfKaRbClassDefaultQEntry_InitInstance(sbZfKaRbClassDefaultQEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[0] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[3] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[0] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[3] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#endif
