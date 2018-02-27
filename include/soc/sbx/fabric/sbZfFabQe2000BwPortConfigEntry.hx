/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabQe2000BwPortConfigEntry.hx 1.1.48.4 Broadcom SDK $
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


#ifndef SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_H
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_H

#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_SIZE 8
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_M_NLINERATE_BITS "21:0"
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_M_NBASEQUEUE_BITS "35:22"
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_M_NQUEUES_BITS "40:36"
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_M_NSPQUEUES_BITS "45:41"
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_M_NSPARE_BITS "63:46"


/**

 * Copyright (c) Sandburst, Inc. 2005
 * All Rights Reserved.  Unpublished rights reserved under the copyright
 * laws of the United States.
 *
 * The software contained on this media is proprietary to and embodies the
 * confidential technology of Sandburst, Inc. Possession, use, duplication
 * or dissemination of the software and media is authorized only pursuant
 * to a valid written license from Sandburst, Inc.
 *
 * RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure by the U.S.
 * Government is subject to restrictions as set forth in Subparagraph
 * (c) (1) (ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
 */
typedef struct _sbZfFabQe2000BwPortConfigEntry {
  uint32_t m_nLineRate;
  uint32_t m_nBaseQueue;
  uint32_t m_nQueues;
  uint32_t m_nSpQueues;
  uint32_t m_nSpare;
} sbZfFabQe2000BwPortConfigEntry_t;

uint32_t
sbZfFabQe2000BwPortConfigEntry_Pack(sbZfFabQe2000BwPortConfigEntry_t *pFrom,
                                    uint8_t *pToData,
                                    uint32_t nMaxToDataIndex);
void
sbZfFabQe2000BwPortConfigEntry_Unpack(sbZfFabQe2000BwPortConfigEntry_t *pToStruct,
                                      uint8_t *pFromData,
                                      uint32_t nMaxToDataIndex);
void
sbZfFabQe2000BwPortConfigEntry_InitInstance(sbZfFabQe2000BwPortConfigEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SP_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SP_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SP_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SP_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[0] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x01) << 4; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SP_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[5] << 2; \
           (nToData) |= (uint32_t) (pFromData)[4] << 10; \
          } while(0)

#else
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[3] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x01) << 4; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SP_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[6] << 2; \
           (nToData) |= (uint32_t) (pFromData)[7] << 10; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[3] ; \
           (nToData) |= (uint32_t) (pFromData)[2] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[0] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[7] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[6] & 0x01) << 4; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SP_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[6] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[5] << 2; \
           (nToData) |= (uint32_t) (pFromData)[4] << 10; \
          } while(0)

#else
#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[3] << 2; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[5] & 0x01) << 4; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SP_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_FABQE2000BWPORTCONFIGENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[5] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[6] << 2; \
           (nToData) |= (uint32_t) (pFromData)[7] << 10; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabQe2000BwPortConfigEntry.hx,v 1.1.48.4 2011/05/22 05:37:54 iakramov Exp $
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
#ifndef SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_CONSOLE_H
#define SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_CONSOLE_H



void
sbZfFabQe2000BwPortConfigEntry_Print(sbZfFabQe2000BwPortConfigEntry_t *pFromStruct);
int
sbZfFabQe2000BwPortConfigEntry_SPrint(sbZfFabQe2000BwPortConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfFabQe2000BwPortConfigEntry_Validate(sbZfFabQe2000BwPortConfigEntry_t *pZf);
int
sbZfFabQe2000BwPortConfigEntry_SetField(sbZfFabQe2000BwPortConfigEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
