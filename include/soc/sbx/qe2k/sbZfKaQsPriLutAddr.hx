/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsPriLutAddr.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAQSPRILUTADDR_H
#define SB_ZF_ZFKAQSPRILUTADDR_H

#define SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSPRILUTADDR_SIZE 4
#define SB_ZF_ZFKAQSPRILUTADDR_M_NRESERVED_BITS "31:13"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NSHAPED_BITS "12:12"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NDEPTH_BITS "11:9"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NANEMICAGED_BITS "8:8"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NQTYPE_BITS "7:4"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NEFAGED_BITS "3:3"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NCREDITLEVEL_BITS "2:2"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NHOLDTS_BITS "1:1"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NPKTLEN_BITS "0:0"


typedef struct _sbZfKaQsPriLutAddr {
  uint32_t m_nReserved;
  uint8_t m_nShaped;
  uint32_t m_nDepth;
  uint8_t m_nAnemicAged;
  uint32_t m_nQType;
  uint8_t m_nEfAged;
  uint32_t m_nCreditLevel;
  uint8_t m_nHoldTs;
  uint32_t m_nPktLen;
} sbZfKaQsPriLutAddr_t;

uint32_t
sbZfKaQsPriLutAddr_Pack(sbZfKaQsPriLutAddr_t *pFrom,
                        uint8_t *pToData,
                        uint32_t nMaxToDataIndex);
void
sbZfKaQsPriLutAddr_Unpack(sbZfKaQsPriLutAddr_t *pToStruct,
                          uint8_t *pFromData,
                          uint32_t nMaxToDataIndex);
void
sbZfKaQsPriLutAddr_InitInstance(sbZfKaQsPriLutAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[1] << 3; \
           (nToData) |= (uint32_t) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[1] << 3; \
           (nToData) |= (uint32_t) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) (pFromData)[2] << 3; \
           (nToData) |= (uint32_t) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
