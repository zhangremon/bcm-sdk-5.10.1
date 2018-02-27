/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpPortTableEntry.hx 1.1.44.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFKAEPPORTTABLEENTRY_H
#define SB_ZF_ZFKAEPPORTTABLEENTRY_H

#define SB_ZF_ZFKAEPPORTTABLEENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEPPORTTABLEENTRY_SIZE 8
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE15_BITS "63:63"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE14_BITS "62:62"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE13_BITS "61:61"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE12_BITS "60:60"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE11_BITS "59:59"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE10_BITS "58:58"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE9_BITS "57:57"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE8_BITS "56:56"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE7_BITS "55:55"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE6_BITS "54:54"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE5_BITS "53:53"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE4_BITS "52:52"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE3_BITS "51:51"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE2_BITS "50:50"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE1_BITS "49:49"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NENABLE0_BITS "48:48"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NRESERVED0_BITS "47:45"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NCOUNTTRANS_BITS "44:40"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NRESERVED1_BITS "39:33"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_NPREPEND_BITS "32:32"
#define SB_ZF_ZFKAEPPORTTABLEENTRY_M_INSTRUCTION_BITS "31:0"


typedef struct _sbZfKaEpPortTableEntry {
  uint8_t m_nEnable15;
  uint8_t m_nEnable14;
  uint8_t m_nEnable13;
  uint8_t m_nEnable12;
  uint8_t m_nEnable11;
  uint8_t m_nEnable10;
  uint8_t m_nEnable9;
  uint8_t m_nEnable8;
  uint8_t m_nEnable7;
  uint8_t m_nEnable6;
  uint8_t m_nEnable5;
  uint8_t m_nEnable4;
  uint8_t m_nEnable3;
  uint8_t m_nEnable2;
  uint8_t m_nEnable1;
  uint8_t m_nEnable0;
  int32_t m_nReserved0;
  int32_t m_nCountTrans;
  int32_t m_nReserved1;
  int32_t m_nPrepend;
  uint32_t m_Instruction;
} sbZfKaEpPortTableEntry_t;

uint32_t
sbZfKaEpPortTableEntry_Pack(sbZfKaEpPortTableEntry_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex);
void
sbZfKaEpPortTableEntry_Unpack(sbZfKaEpPortTableEntry_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex);
void
sbZfKaEpPortTableEntry_InitInstance(sbZfKaEpPortTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_TRANS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_PREPEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_INST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_TRANS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_PREPEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_INST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_TRANS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_PREPEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_INST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_ENABLE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_TRANS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_RESRV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_PREPEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_SET_INST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[6] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_TRANS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[6]) & 0x1f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[7] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_PREPEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[7]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_INST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) (pFromData)[1] << 16; \
           (nToData) |= (uint32_t) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[5] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_TRANS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[5]) & 0x1f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[4] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_PREPEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_INST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[5]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[6] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_TRANS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[6]) & 0x1f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[7] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_PREPEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[7]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_INST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) (pFromData)[1] << 16; \
           (nToData) |= (uint32_t) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[7]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_ENABLE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[6]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[5] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_TRANS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[5]) & 0x1f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_RESRV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[4] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_PREPEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPPORTTABLEENTRY_GET_INST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
#endif
