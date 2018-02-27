/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000PtMirrorIndexEntry.hx 1.3.36.4 Broadcom SDK $
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


#ifndef SB_ZF_FE2000PTMIRRORINDEXENTRY_H
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_H

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SIZE 4
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_M_USPARE_BITS "31:12"
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_M_UOVERWRITEHDR_BITS "11:11"
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_M_UHIGHGIGHDR_BITS "10:10"
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_M_UDROPSRCPKT_BITS "9:9"
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_M_UHDRPRESENT_BITS "8:8"
#define SB_ZF_FE2000PTMIRRORINDEXENTRY_M_UMIRRORQUEUE_BITS "7:0"


typedef struct _sbZfFe2000PtMirrorIndexEntry {
  uint32_t m_uSpare;
  uint32_t m_uOverWriteHdr;
  uint32_t m_uHighGigHdr;
  uint32_t m_uDropSrcPkt;
  uint32_t m_uHdrPresent;
  uint32_t m_uMirrorQueue;
} sbZfFe2000PtMirrorIndexEntry_t;

uint32_t
sbZfFe2000PtMirrorIndexEntry_Pack(sbZfFe2000PtMirrorIndexEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex);
void
sbZfFe2000PtMirrorIndexEntry_Unpack(sbZfFe2000PtMirrorIndexEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex);
void
sbZfFe2000PtMirrorIndexEntry_InitInstance(sbZfFe2000PtMirrorIndexEntry_t *pFrame);

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SET_OVERWRITEHDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SET_HIGHGIGHDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SET_DROPSRCPKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SET_HDRPRESENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_SET_MIRRORQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[2] << 4; \
           (nToData) |= (uint32_t) (pFromData)[3] << 12; \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_GET_OVERWRITEHDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_GET_HIGHGIGHDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_GET_DROPSRCPKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_GET_HDRPRESENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PTMIRRORINDEXENTRY_GET_MIRRORQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
          } while(0)

#endif
