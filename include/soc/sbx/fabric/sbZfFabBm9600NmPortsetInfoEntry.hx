/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600NmPortsetInfoEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_H
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_H

#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_SIZE_IN_BYTES 3
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_SIZE 3
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_UVIRTUALPORT_BITS "18:18"
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_UVPORTEOPP_BITS "17:15"
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_USTARTPORT_BITS "14:7"
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_UEGNODE_BITS "6:0"


typedef struct _sbZfFabBm9600NmPortsetInfoEntry {
  uint32_t m_uVirtualPort;
  uint32_t m_uVportEopp;
  uint32_t m_uStartPort;
  uint32_t m_uEgNode;
} sbZfFabBm9600NmPortsetInfoEntry_t;

uint32_t
sbZfFabBm9600NmPortsetInfoEntry_Pack(sbZfFabBm9600NmPortsetInfoEntry_t *pFrom,
                                     uint8_t *pToData,
                                     uint32_t nMaxToDataIndex);
void
sbZfFabBm9600NmPortsetInfoEntry_Unpack(sbZfFabBm9600NmPortsetInfoEntry_t *pToStruct,
                                       uint8_t *pFromData,
                                       uint32_t nMaxToDataIndex);
void
sbZfFabBm9600NmPortsetInfoEntry_InitInstance(sbZfFabBm9600NmPortsetInfoEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
