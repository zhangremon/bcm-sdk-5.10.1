/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwRepErrInfo.hx 1.1.28.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_H
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_H

#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_SIZE 4
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_M_NCLIENT_BITS "24:19"
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_M_NTABLEID_BITS "18:15"
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_M_NOFFSET_BITS "14:0"


typedef struct _sbZfFabBm3200BwRepErrInfo {
  int32_t m_nClient;
  int32_t m_nTableId;
  int32_t m_nOffset;
} sbZfFabBm3200BwRepErrInfo_t;

uint32_t
sbZfFabBm3200BwRepErrInfo_Pack(sbZfFabBm3200BwRepErrInfo_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex);
void
sbZfFabBm3200BwRepErrInfo_Unpack(sbZfFabBm3200BwRepErrInfo_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex);
void
sbZfFabBm3200BwRepErrInfo_InitInstance(sbZfFabBm3200BwRepErrInfo_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_SET_CLIENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_TABLE_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 1) & 0x07); \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 8) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (int32_t) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (int32_t) ((pFromData)[1] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[3] ; \
           (nToData) |= (int32_t) ((pFromData)[2] & 0x7f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (int32_t) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (int32_t) ((pFromData)[2] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[0] ; \
           (nToData) |= (int32_t) ((pFromData)[1] & 0x7f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (int32_t) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (int32_t) ((pFromData)[1] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[3] ; \
           (nToData) |= (int32_t) ((pFromData)[2] & 0x7f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWREPERRINFO_GET_CLIENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (int32_t) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_TABLE_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (int32_t) ((pFromData)[2] & 0x07) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWREPERRINFO_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32_t) (pFromData)[0] ; \
           (nToData) |= (int32_t) ((pFromData)[1] & 0x7f) << 8; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwRepErrInfo.hx,v 1.1.28.4 2011/05/22 05:37:52 iakramov Exp $
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
#ifndef SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_CONSOLE_H
#define SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_CONSOLE_H



void
sbZfFabBm3200BwRepErrInfo_Print(sbZfFabBm3200BwRepErrInfo_t *pFromStruct);
int
sbZfFabBm3200BwRepErrInfo_SPrint(sbZfFabBm3200BwRepErrInfo_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfFabBm3200BwRepErrInfo_Validate(sbZfFabBm3200BwRepErrInfo_t *pZf);
int
sbZfFabBm3200BwRepErrInfo_SetField(sbZfFabBm3200BwRepErrInfo_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_BW_REP_ERR_INFO_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
