/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfCaPpRxPortDataPpDescriptorFormat.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_H
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_H

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SIZE_IN_BYTES 16
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SIZE 16
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_M_URXPORTMASK_57_50_BITS "121:114"
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_M_URXPORTMASK_49_32_BITS "113:96"
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_M_URXPORTMASK_31_0_BITS "95:64"
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_M_UPAD_BITS "63:58"
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_M_URXPORTDATA_57_50_BITS "57:50"
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_M_URXPORTDATA_49_32_BITS "49:32"
#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_M_URXPORTDATA_31_0_BITS "31:0"


typedef struct _sbZfCaPpRxPortDataPpDescriptorFormat {
  uint32_t m_uRxPortMask_57_50;
  uint32_t m_uRxPortMask_49_32;
  uint32_t m_uRxPortMask_31_0;
  uint32_t m_uPad;
  uint32_t m_uRxPortData_57_50;
  uint32_t m_uRxPortData_49_32;
  uint32_t m_uRxPortData_31_0;
} sbZfCaPpRxPortDataPpDescriptorFormat_t;

uint32_t
sbZfCaPpRxPortDataPpDescriptorFormat_Pack(sbZfCaPpRxPortDataPpDescriptorFormat_t *pFrom,
                                          uint8_t *pToData,
                                          uint32_t nMaxToDataIndex);
void
sbZfCaPpRxPortDataPpDescriptorFormat_Unpack(sbZfCaPpRxPortDataPpDescriptorFormat_t *pToStruct,
                                            uint8_t *pFromData,
                                            uint32_t nMaxToDataIndex);
void
sbZfCaPpRxPortDataPpDescriptorFormat_InitInstance(sbZfCaPpRxPortDataPpDescriptorFormat_t *pFrame);

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SET_RXPORTDATA3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[15] = ((pToData)[15] & ~ 0x03) | (((nFromData) >> 6) & 0x03); \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SET_RXPORTDATA1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 16) & 0x03); \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SET_RXPORTDATA0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SET_RXPORTPAD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SET_RXPORTMASK3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 6) & 0x03); \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SET_RXPORTMASK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 16) & 0x03); \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_SET_RXPORTMASK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_GET_RXPORTDATA3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[15] & 0x03) << 6; \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_GET_RXPORTDATA1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
           (nToData) |= (uint32_t) (pFromData)[13] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[14] & 0x03) << 16; \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_GET_RXPORTDATA0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) (pFromData)[10] << 16; \
           (nToData) |= (uint32_t) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_GET_RXPORTPAD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_GET_RXPORTMASK3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x03) << 6; \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_GET_RXPORTMASK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x03) << 16; \
          } while(0)

#define SB_ZF_CAPPRXPORTDATAPPDESCRIPTORFORMAT_GET_RXPORTMASK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
