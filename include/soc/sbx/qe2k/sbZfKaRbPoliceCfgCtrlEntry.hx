/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbPoliceCfgCtrlEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKARBPOLCFGCTRLENTRY_H
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_H

#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE 4
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NRESERVED_BITS "31:27"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NENABLE_BITS "26:26"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NNOTBLIND_BITS "25:25"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NDROPONRED_BITS "24:24"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NENABLEMON_BITS "23:23"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NMONCNTID_BITS "22:20"
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_M_NINCRATE_BITS "19:0"


typedef struct _sbZfKaRbPoliceCfgCtrlEntry {
  uint32_t m_nReserved;
  uint8_t m_nEnable;
  uint8_t m_nNotBlind;
  uint8_t m_nDropOnRed;
  uint8_t m_nEnableMon;
  uint32_t m_nMonCntId;
  uint32_t m_nIncRate;
} sbZfKaRbPoliceCfgCtrlEntry_t;

uint32_t
sbZfKaRbPoliceCfgCtrlEntry_Pack(sbZfKaRbPoliceCfgCtrlEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex);
void
sbZfKaRbPoliceCfgCtrlEntry_Unpack(sbZfKaRbPoliceCfgCtrlEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex);
void
sbZfKaRbPoliceCfgCtrlEntry_InitInstance(sbZfKaRbPoliceCfgCtrlEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_NOTBLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_DROPONRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_ENABLEMON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_MONCNTID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_SET_INCRATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x0f) << 16; \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x0f) << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x0f) << 16; \
          } while(0)

#else
#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_NOTBLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_DROPONRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_ENABLEMON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_MONCNTID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KARBPOLICECFGCTRLENTRY_GET_INCRATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x0f) << 16; \
          } while(0)

#endif
#endif
