/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600InaNmPriorityUpdate.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_BM9600_INANMPRIUPDATE_H
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_H

#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_SIZE_IN_BYTES 7
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_SIZE 7
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UINA_BITS "53:47"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_USYSTEMPORT_BITS "46:35"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UESET_BITS "34:25"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UPORTSETADDRESS_BITS "24:17"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UPORTSETOFFSET_BITS "16:13"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BNOCRITICALUPDATE_BITS "12:12"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BCRITICALUPDATE_BITS "11:11"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BMULTICAST_BITS "10:10"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UPRIORITY_BITS "9:6"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BMAXPRIORITY_BITS "5:5"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_UNEXTPRIORITY_BITS "4:1"
#define SB_ZF_FAB_BM9600_INANMPRIUPDATE_M_BNEXTMAXPRIORITY_BITS "0:0"


typedef struct _sbZfFabBm9600InaNmPriorityUpdate {
  uint32_t m_uIna;
  uint32_t m_uSystemPort;
  uint32_t m_uEset;
  uint32_t m_uPortSetAddress;
  uint32_t m_uPortSetOffset;
  uint8_t m_bNoCriticalUpdate;
  uint8_t m_bCriticalUpdate;
  uint8_t m_bMulticast;
  uint32_t m_uPriority;
  uint8_t m_bMaxPriority;
  uint32_t m_uNextPriority;
  uint8_t m_bNextMaxPriority;
} sbZfFabBm9600InaNmPriorityUpdate_t;

uint32_t
sbZfFabBm9600InaNmPriorityUpdate_Pack(sbZfFabBm9600InaNmPriorityUpdate_t *pFrom,
                                      uint8_t *pToData,
                                      uint32_t nMaxToDataIndex);
void
sbZfFabBm9600InaNmPriorityUpdate_Unpack(sbZfFabBm9600InaNmPriorityUpdate_t *pToStruct,
                                        uint8_t *pFromData,
                                        uint32_t nMaxToDataIndex);
void
sbZfFabBm9600InaNmPriorityUpdate_InitInstance(sbZfFabBm9600InaNmPriorityUpdate_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_INA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~ 0x3f) | (((nFromData) >> 1) & 0x3f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_SYSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 5) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_ESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 7) & 0x07); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 7) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PORTOFFS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 3) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NCU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_CU(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_PRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_MAXPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTPRI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_SET_NXTMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_INA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x3f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_SYSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 3) & 0x1f; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x7f) << 5; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_ESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x07) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x01) << 7; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PORTOFFS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x01) << 3; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NCU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_CU(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_PRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x03) << 2; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_MAXPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTPRI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INANMPRIORITYUPDATE_GET_NXTMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
