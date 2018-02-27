/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgMemFifoControlEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_H
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_H

#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_SIZE 8
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NTAILPAGE_BITS "35:26"
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NTAILOFFSET_BITS "25:21"
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NCURRDEPTH_BITS "20:5"
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_M_NHEADOFFSET_BITS "4:0"


typedef struct _sbZfKaEgMemFifoControlEntry {
  uint32_t m_nTailPage;
  uint32_t m_nTailOffset;
  uint32_t m_nCurrDepth;
  uint32_t m_nHeadOffset;
} sbZfKaEgMemFifoControlEntry_t;

uint32_t
sbZfKaEgMemFifoControlEntry_Pack(sbZfKaEgMemFifoControlEntry_t *pFrom,
                                 uint8_t *pToData,
                                 uint32_t nMaxToDataIndex);
void
sbZfKaEgMemFifoControlEntry_Unpack(sbZfKaEgMemFifoControlEntry_t *pToStruct,
                                   uint8_t *pFromData,
                                   uint32_t nMaxToDataIndex);
void
sbZfKaEgMemFifoControlEntry_InitInstance(sbZfKaEgMemFifoControlEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILPAGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_TAILOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_CURRDEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x1f) | (((nFromData) >> 11) & 0x1f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_SET_HEADOFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[1] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILPAGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_TAILOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_CURRDEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[1] << 3; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x1f) << 11; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOCONTROLENTRY_GET_HEADOFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#endif
