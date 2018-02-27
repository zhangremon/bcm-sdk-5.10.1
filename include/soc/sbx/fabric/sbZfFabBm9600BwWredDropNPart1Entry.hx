/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwWredDropNPart1Entry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_H
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_H

#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE 4
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UDP1_BITS "31:31"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UECN1_BITS "30:30"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_URESERVED1_BITS "29:26"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UPBDE1_BITS "25:16"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UDP0_BITS "15:15"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UECN0_BITS "14:14"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_URESERVED0_BITS "13:10"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UPBDE0_BITS "9:0"


typedef struct _sbZfFabBm9600BwWredDropNPart1Entry {
  uint32_t m_uDp1;
  uint32_t m_uEcn1;
  uint32_t m_uReserved1;
  uint32_t m_uPbDe1;
  uint32_t m_uDp0;
  uint32_t m_uEcn0;
  uint32_t m_uReserved0;
  uint32_t m_uPbDe0;
} sbZfFabBm9600BwWredDropNPart1Entry_t;

uint32_t
sbZfFabBm9600BwWredDropNPart1Entry_Pack(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrom,
                                        uint8_t *pToData,
                                        uint32_t nMaxToDataIndex);
void
sbZfFabBm9600BwWredDropNPart1Entry_Unpack(sbZfFabBm9600BwWredDropNPart1Entry_t *pToStruct,
                                          uint8_t *pFromData,
                                          uint32_t nMaxToDataIndex);
void
sbZfFabBm9600BwWredDropNPart1Entry_InitInstance(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[1] ; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[2] ; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
