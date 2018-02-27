/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgMemQCtlEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAEGMEMQCTLENTRY_H
#define SB_ZF_ZFKAEGMEMQCTLENTRY_H

#define SB_ZF_ZFKAEGMEMQCTLENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEGMEMQCTLENTRY_SIZE 8
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NDROPONFULL_BITS "25:25"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NWPTR_BITS "24:19"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NRPTR_BITS "18:13"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NSIZE_BITS "12:10"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NBASE_BITS "9:0"


typedef struct _sbZfKaEgMemQCtlEntry {
  uint32_t m_nDropOnFull;
  uint32_t m_nWptr;
  uint32_t m_nRptr;
  uint32_t m_nSize;
  uint32_t m_nBase;
} sbZfKaEgMemQCtlEntry_t;

uint32_t
sbZfKaEgMemQCtlEntry_Pack(sbZfKaEgMemQCtlEntry_t *pFrom,
                          uint8_t *pToData,
                          uint32_t nMaxToDataIndex);
void
sbZfKaEgMemQCtlEntry_Unpack(sbZfKaEgMemQCtlEntry_t *pToStruct,
                            uint8_t *pFromData,
                            uint32_t nMaxToDataIndex);
void
sbZfKaEgMemQCtlEntry_InitInstance(sbZfKaEgMemQCtlEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
