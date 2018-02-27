/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200NmEmapEntry.hx 1.1.28.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_BM3200_EMAP_ENTRY_H
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_H

#define SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE_IN_BYTES 17
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE 17
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_M_NEMAPX_BITS "131:128"
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_M_NEMAPD_BITS "127:96"
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_M_NEMAPC_BITS "95:64"
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_M_NEMAPB_BITS "63:32"
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_M_NEMAPA_BITS "31:0"


typedef struct _sbZfFabBm3200NmEmapEntry {
  uint32_t m_nEmapx;
  uint32_t m_nEmapd;
  uint32_t m_nEmapc;
  uint32_t m_nEmapb;
  uint32_t m_nEmapa;
} sbZfFabBm3200NmEmapEntry_t;

uint32_t
sbZfFabBm3200NmEmapEntry_Pack(sbZfFabBm3200NmEmapEntry_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex);
void
sbZfFabBm3200NmEmapEntry_Unpack(sbZfFabBm3200NmEmapEntry_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex);
void
sbZfFabBm3200NmEmapEntry_InitInstance(sbZfFabBm3200NmEmapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_SET_EMAPA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[19]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[15] ; \
           (nToData) |= (uint32_t) (pFromData)[14] << 8; \
           (nToData) |= (uint32_t) (pFromData)[13] << 16; \
           (nToData) |= (uint32_t) (pFromData)[12] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[11] ; \
           (nToData) |= (uint32_t) (pFromData)[10] << 8; \
           (nToData) |= (uint32_t) (pFromData)[9] << 16; \
           (nToData) |= (uint32_t) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
           (nToData) |= (uint32_t) (pFromData)[6] << 8; \
           (nToData) |= (uint32_t) (pFromData)[5] << 16; \
           (nToData) |= (uint32_t) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) (pFromData)[1] << 16; \
           (nToData) |= (uint32_t) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
           (nToData) |= (uint32_t) (pFromData)[13] << 8; \
           (nToData) |= (uint32_t) (pFromData)[14] << 16; \
           (nToData) |= (uint32_t) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) (pFromData)[10] << 16; \
           (nToData) |= (uint32_t) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[19]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[15] ; \
           (nToData) |= (uint32_t) (pFromData)[14] << 8; \
           (nToData) |= (uint32_t) (pFromData)[13] << 16; \
           (nToData) |= (uint32_t) (pFromData)[12] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[11] ; \
           (nToData) |= (uint32_t) (pFromData)[10] << 8; \
           (nToData) |= (uint32_t) (pFromData)[9] << 16; \
           (nToData) |= (uint32_t) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
           (nToData) |= (uint32_t) (pFromData)[6] << 8; \
           (nToData) |= (uint32_t) (pFromData)[5] << 16; \
           (nToData) |= (uint32_t) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) (pFromData)[1] << 16; \
           (nToData) |= (uint32_t) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[12] ; \
           (nToData) |= (uint32_t) (pFromData)[13] << 8; \
           (nToData) |= (uint32_t) (pFromData)[14] << 16; \
           (nToData) |= (uint32_t) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[8] ; \
           (nToData) |= (uint32_t) (pFromData)[9] << 8; \
           (nToData) |= (uint32_t) (pFromData)[10] << 16; \
           (nToData) |= (uint32_t) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
           (nToData) |= (uint32_t) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_FABBM3200NMEMAPENTRY_GET_EMAPA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200NmEmapEntry.hx,v 1.1.28.4 2011/05/22 05:37:52 iakramov Exp $
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



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_EMAP_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_EMAP_ENTRY_CONSOLE_H



void
sbZfFabBm3200NmEmapEntry_Print(sbZfFabBm3200NmEmapEntry_t *pFromStruct);
int
sbZfFabBm3200NmEmapEntry_SPrint(sbZfFabBm3200NmEmapEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfFabBm3200NmEmapEntry_Validate(sbZfFabBm3200NmEmapEntry_t *pZf);
int
sbZfFabBm3200NmEmapEntry_SetField(sbZfFabBm3200NmEmapEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_EMAP_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
