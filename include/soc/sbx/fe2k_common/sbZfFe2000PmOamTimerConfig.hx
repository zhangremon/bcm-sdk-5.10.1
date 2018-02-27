/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000PmOamTimerConfig.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_FE_2000_PM_OAMTIMERCFG_H
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_H

#define SB_ZF_FE_2000_PM_OAMTIMERCFG_SIZE_IN_BYTES 5
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_SIZE 5
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_URATE_BITS "39:36"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_UDEADLINE_BITS "35:4"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BSTARTED_BITS "3:3"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BSTRICT_BITS "2:2"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BRESET_BITS "1:1"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BINTERRUPT_BITS "0:0"




/** @brief  User Timer Configuration

*/

typedef struct _sbZfFe2000PmOamTimerConfig {
  uint32_t uRate;
  uint32_t uDeadline;
  uint32_t bStarted;
  uint32_t bStrict;
  uint32_t bReset;
  uint32_t bInterrupt;
} sbZfFe2000PmOamTimerConfig_t;

uint32_t
sbZfFe2000PmOamTimerConfig_Pack(sbZfFe2000PmOamTimerConfig_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex);
void
sbZfFe2000PmOamTimerConfig_Unpack(sbZfFe2000PmOamTimerConfig_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex);
void
sbZfFe2000PmOamTimerConfig_InitInstance(sbZfFe2000PmOamTimerConfig_t *pFrame);

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_DEADLINE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 20) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 28) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_STARTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_STRICT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_RESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_INTERRUPT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_DEADLINE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[1] << 4; \
           (nToData) |= (uint32_t) (pFromData)[2] << 12; \
           (nToData) |= (uint32_t) (pFromData)[3] << 20; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x0f) << 28; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_STARTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_STRICT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_RESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_INTERRUPT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
