/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaQmBufferState1NonHeaderEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_H
#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_H

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_SIZE_IN_BYTES 4
#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_SIZE 4
#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_M_URESERVED_BITS "30:26"
#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_M_UREPLICATIONSTATE_BITS "25:15"
#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_M_UDROP_BITS "14:14"
#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_M_UPACKETLENGTH_BITS "13:0"


typedef struct _sbZfCaQmBufferState1NonHeaderEntry {
  uint32_t m_uReserved;
  uint32_t m_uReplicationState;
  uint32_t m_uDrop;
  uint32_t m_uPacketLength;
} sbZfCaQmBufferState1NonHeaderEntry_t;

uint32_t
sbZfCaQmBufferState1NonHeaderEntry_Pack(sbZfCaQmBufferState1NonHeaderEntry_t *pFrom,
                                        uint8_t *pToData,
                                        uint32_t nMaxToDataIndex);
void
sbZfCaQmBufferState1NonHeaderEntry_Unpack(sbZfCaQmBufferState1NonHeaderEntry_t *pToStruct,
                                          uint8_t *pFromData,
                                          uint32_t nMaxToDataIndex);
void
sbZfCaQmBufferState1NonHeaderEntry_InitInstance(sbZfCaQmBufferState1NonHeaderEntry_t *pFrame);

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_SET_REPLICATIONSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 9) & 0x03); \
          } while(0)

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_SET_DROP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_SET_PACKETLENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_GET_REPLICATIONSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[2] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 9; \
          } while(0)

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_GET_DROP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_CAQMBUFFERSTATE1NONHEADERENTRY_GET_PACKETLENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
