/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfG2EplibMvtEntry.hx 1.6.12.4 Broadcom SDK $
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


#ifndef SB_ZF_G2_EPLIB_MVTENTRY_H
#define SB_ZF_G2_EPLIB_MVTENTRY_H

#define SB_ZF_G2_EPLIB_MVTENTRY_SIZE_IN_BYTES 1
#define SB_ZF_G2_EPLIB_MVTENTRY_SIZE 1
#define SB_ZF_G2_EPLIB_MVTENTRY_ULLPORTMASK_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_NTYPE_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_ULMVTDA_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_ULMVTDB_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_BSOURCEKNOCKOUT_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_BENABLECHAINING_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_ULNEXTMCGROUP_BITS "0:0"
#define SB_ZF_G2_EPLIB_MVTENTRY_SET_VLAN(_VLAN_, pZf) (pZf)->ulMvtdA = ((_VLAN_) & 0xfff);  (pZf)->ulMvtdB &= 0x1 
#define SB_ZF_G2_EPLIB_MVTENTRY_GET_VLAN(_VLAN_, pZf) _VLAN_ = ((pZf)->ulMvtdA) & 0xfff);
#define SB_ZF_G2_EPLIB_MVTENTRY_SET_OIX(_IDX_, pZf)  (pZf)->ulMvtdA = ((_IDX_) & 0xfff); (pZf)->ulMvtdA |= ((_IDX_) & 0x18000) >> 3; (pZf)->ulMvtdB =((_IDX_) >> 12) & 0x7 
#define SB_ZF_G2_EPLIB_MVTENTRY_GET_OIX(_IDX_, pZf)  _IDX_ = ((pZf)->ulMvtdA & 0xfff); _IDX_ |= ((pZf)->ulMvtdA << 3) & 0x18000; _IDX_ |= ((pZf)->ulMvtdB & 0x7) <<  12
#define SB_ZF_G2_EPLIB_MVTENTRY_SET_IPMC(_IPMC_, pZf)  ( (pZf)->ulMvtdB = (_IPMC_) & 0x1 )
#define SB_ZF_G2_EPLIB_MVTENTRY_GET_IPMC(_IPMC_, pZf)  _IPMC_ =  (pZf)->ulMvtdB & 0x1



/** @brief  EP MVT Entry

  EP MVT Entry 
*/

typedef struct _sbZfG2EplibMvtEntry {
  uint64_t ullPortMask;
  uint32_t nType;
  uint32_t ulMvtdA;
  uint32_t ulMvtdB;
  uint8_t bSourceKnockout;
  uint8_t bEnableChaining;
  uint32_t ulNextMcGroup;
} sbZfG2EplibMvtEntry_t;

uint32_t
sbZfG2EplibMvtEntry_Pack(sbZfG2EplibMvtEntry_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex);
void
sbZfG2EplibMvtEntry_Unpack(sbZfG2EplibMvtEntry_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex);
void
sbZfG2EplibMvtEntry_InitInstance(sbZfG2EplibMvtEntry_t *pFrame);

#define SB_ZF_G2EPLIBMVTENTRY_SET_PORTMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_MVTDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_MVTDB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_SOURCEKNOCKOUT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_ENABLECHAINING(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_SET_NEXTMCGROUP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_PORTMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_MVTDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_MVTDB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_SOURCEKNOCKOUT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_ENABLECHAINING(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBMVTENTRY_GET_NEXTMCGROUP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
