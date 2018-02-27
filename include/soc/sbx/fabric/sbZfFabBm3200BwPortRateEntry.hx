/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwPortRateEntry.hx 1.1.48.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_H
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_H

#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE 8
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_M_NRESERVED_BITS "31:26"
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_M_NSPGROUPS_BITS "25:21"
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_M_NGROUPS_BITS "20:16"
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_M_NGROUP_BITS "15:0"
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_M_NLINERATE_BITS "54:32"
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_M_NRESERVED1_BITS "63:55"


typedef struct _sbZfFabBm3200BwPortRateEntry {
  uint32_t m_nReserved;
  uint32_t m_nSpGroups;
  uint32_t m_nGroups;
  uint32_t m_nGroup;
  uint32_t m_nLineRate;
  uint32_t m_nReserved1;
} sbZfFabBm3200BwPortRateEntry_t;

uint32_t
sbZfFabBm3200BwPortRateEntry_Pack(sbZfFabBm3200BwPortRateEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex);
void
sbZfFabBm3200BwPortRateEntry_Unpack(sbZfFabBm3200BwPortRateEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex);
void
sbZfFabBm3200BwPortRateEntry_InitInstance(sbZfFabBm3200BwPortRateEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_SP_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_LINE_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_SP_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_LINE_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_SP_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_LINE_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_SP_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUPS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_GROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_LINE_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_SP_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_LINE_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
           (nToData) |= (uint32_t) (pFromData)[6] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[4] << 1; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_SP_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_LINE_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[7] << 1; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_SP_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[0] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_LINE_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[7] ; \
           (nToData) |= (uint32_t) (pFromData)[6] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[4] << 1; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[3] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_SP_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32_t) ((pFromData)[3] & 0x03) << 3; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUPS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_GROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_LINE_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_FABBM3200BWPORTRATEENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[7] << 1; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwPortRateEntry.hx,v 1.1.48.4 2011/05/22 05:37:52 iakramov Exp $
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
#ifndef SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwPortRateEntry_Print(sbZfFabBm3200BwPortRateEntry_t *pFromStruct);
int
sbZfFabBm3200BwPortRateEntry_SPrint(sbZfFabBm3200BwPortRateEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfFabBm3200BwPortRateEntry_Validate(sbZfFabBm3200BwPortRateEntry_t *pZf);
int
sbZfFabBm3200BwPortRateEntry_SetField(sbZfFabBm3200BwPortRateEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
