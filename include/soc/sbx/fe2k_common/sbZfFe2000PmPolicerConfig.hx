/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFe2000PmPolicerConfig.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_FE_2000_PM_POLICERCFG_H
#define SB_ZF_FE_2000_PM_POLICERCFG_H

#define SB_ZF_FE_2000_PM_POLICERCFG_SIZE_IN_BYTES 22
#define SB_ZF_FE_2000_PM_POLICERCFG_SIZE 22
#define SB_ZF_FE_2000_PM_POLICERCFG_URFCMODE_BITS "170:169"
#define SB_ZF_FE_2000_PM_POLICERCFG_ULENSHIFT_BITS "168:166"
#define SB_ZF_FE_2000_PM_POLICERCFG_URATE_BITS "165:134"
#define SB_ZF_FE_2000_PM_POLICERCFG_UCBS_BITS "133:102"
#define SB_ZF_FE_2000_PM_POLICERCFG_UCIR_BITS "101:70"
#define SB_ZF_FE_2000_PM_POLICERCFG_UEBS_BITS "69:38"
#define SB_ZF_FE_2000_PM_POLICERCFG_UEIR_BITS "37:6"
#define SB_ZF_FE_2000_PM_POLICERCFG_URSVD_BITS "5:5"
#define SB_ZF_FE_2000_PM_POLICERCFG_BBLINDMODE_BITS "4:4"
#define SB_ZF_FE_2000_PM_POLICERCFG_BDROPONRED_BITS "3:3"
#define SB_ZF_FE_2000_PM_POLICERCFG_BCOUPLING_BITS "2:2"
#define SB_ZF_FE_2000_PM_POLICERCFG_BCBSNODECREMENT_BITS "1:1"
#define SB_ZF_FE_2000_PM_POLICERCFG_BEBSNODECREMENT_BITS "0:0"



/**
 * sbFe2000FePmPolicerMode_e
 * 
 * This enumeration defines the Policer Mode of Operation.
 * see RFC 2697, 2698, 4115 and MEF specficiations for a 
 * detailed description of these policer operations.
 */

typedef enum sbFe2000FePmPolicerMode_e {
  SB_FE_2000_PM_PMODE_RFC_2697 = 0,
  SB_FE_2000_PM_PMODE_RFC_2698,
  SB_FE_2000_PM_PMODE_RFC_4115,
  SB_FE_2000_PM_PMODE_RFC_MEF
} sbFe2000FePmPolicerMode_t;



/** @brief  User Policer Configuration

*/

typedef struct _sbZfFe2000PmPolicerConfig {
/** @brief <p> Must be one of the modes of operation</p> */
/** @brief <p> listed in sbFe2000FePmPolicerMode_t</p> */

  uint32_t uRfcMode;
/** @brief <p> ulLengthShift</p> */
/** @brief <p> +ve Shifts PktLen to right by "LenShift"( ignores low-order bits ) </p> */
/** @brief <p> -ve Accuracy tradeoff for longer burst length</p> */

  uint32_t uLenShift;
/** @brief <p> CIR & EIR  units</p> */
/** @brief <p> see sbFe2000FePm.h for the rate units enums.</p> */

  uint32_t uRate;
/** @brief <p> Committed Burst Size of the Policer</p> */
/** @brief <p> Must be atleast MTU of the incomging stream</p> */
/** @brief <p> Units: In Bytes</p> */

  uint32_t uCBS;
/** @brief <p> Commited Information Rate of the Policer</p> */
/** @brief <p> Peak Rate that's accepted</p> */
/** @brief <p> Units: In uRate Per Second</p> */

  uint32_t uCIR;
/** @brief <p> Excess  Burst Size of the Policer</p> */
/** @brief <p> Units: In Bytes</p> */

  uint32_t uEBS;
/** @brief <p> Excess  Information Rate of the Policer</p> */
/** @brief <p> Units: In uRate Per Second</p> */

  uint32_t uEIR;
/** @brief <p> Reserved</p> */

  uint32_t uRsvd;
/** @brief <p> Policer Color aware or unaware</p> */

  uint32_t bBlindMode;
/** @brief <p> Policer policy on the RED packets.</p> */
/** @brief <p> if bDropOnRed is set the packets will be dropped.</p> */

  uint32_t bDropOnRed;
/** @brief <p> This coupling flags dictates if the tokens</p> */
/** @brief <p> being added to the CBS bucket should be diverted</p> */
/** @brief <p> to EBS bucket or not. Not a valid thing for RFC2697 mode</p> */
/** @brief <p> of operation.</p> */

  uint32_t bCoupling;
/** @brief <p> If set, tokens are never decremented from the </p> */
/** @brief <p> CBS bucket even when the packet is GREEN.</p> */

  uint32_t bCBSNoDecrement;
/** @brief <p> If set, tokens are never decremented from the </p> */
/** @brief <p> EBS bucket even when the packet is GREEN/YELLOW</p> */

  uint32_t bEBSNoDecrement;
} sbZfFe2000PmPolicerConfig_t;

uint32_t
sbZfFe2000PmPolicerConfig_Pack(sbZfFe2000PmPolicerConfig_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex);
void
sbZfFe2000PmPolicerConfig_Unpack(sbZfFe2000PmPolicerConfig_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex);
void
sbZfFe2000PmPolicerConfig_InitInstance(sbZfFe2000PmPolicerConfig_t *pFrame);

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_RFCMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_LENSHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((pToData)[20] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[21] = ((pToData)[21] & ~ 0x01) | (((nFromData) >> 2) & 0x01); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_RUNITS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_CBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_CIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_EBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_EIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x3f) | (((nFromData) >> 26) & 0x3f); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_BMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_DRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_COUPLING(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_CBSNODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_SET_EBSNODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_RFCMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[21] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_LENSHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[20] >> 6) & 0x03; \
           (nToData) |= (uint32_t) ((pFromData)[21] & 0x01) << 2; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_RUNITS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[17] << 2; \
           (nToData) |= (uint32_t) (pFromData)[18] << 10; \
           (nToData) |= (uint32_t) (pFromData)[19] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[20] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_CBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[13] << 2; \
           (nToData) |= (uint32_t) (pFromData)[14] << 10; \
           (nToData) |= (uint32_t) (pFromData)[15] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[16] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_CIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[9] << 2; \
           (nToData) |= (uint32_t) (pFromData)[10] << 10; \
           (nToData) |= (uint32_t) (pFromData)[11] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[12] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_EBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[4] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[5] << 2; \
           (nToData) |= (uint32_t) (pFromData)[6] << 10; \
           (nToData) |= (uint32_t) (pFromData)[7] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[8] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_EIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[1] << 2; \
           (nToData) |= (uint32_t) (pFromData)[2] << 10; \
           (nToData) |= (uint32_t) (pFromData)[3] << 18; \
           (nToData) |= (uint32_t) ((pFromData)[4] & 0x3f) << 26; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_BMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_DRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_COUPLING(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_CBSNODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERCONFIG_GET_EBSNODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
