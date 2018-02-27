/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */

#ifndef _SB_FE_I_SUPPORT_H_
#define _SB_FE_I_SUPPORT_H_

/******************************************************************************
 *
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
 *
 * sbFeISupport.h
 *
 * $Id: sbFeISupport.h 1.5.196.1 Broadcom SDK $
 *
 *****************************************************************************/

#include <soc/sbx/sbStatus.h>
typedef sbStatus_t (*sbMalloc_f_t)
     (void *chipUserToken, sbFeMallocType_t type, uint32_t size,
      void **memoryP, sbDmaMemoryHandle_t *dmaHandleP);

typedef sbStatus_t (*sbFree_f_t)
     (void *chipUserToken, sbFeMallocType_t type, uint32_t size,
      void *memoryP, sbDmaMemoryHandle_t dmaHandleP);

typedef sbSyncToken_t (*sbIsrSync_f_t)(void *chipUserToken);

typedef void (*sbIsrUnsync_f_t)(void *chipUserToken,
                                       sbSyncToken_t syncToken);

typedef enum sbFeHashGeneration_e {
  SB_FE_HASH_OVER_L2  = (1 << 0), 
  SB_FE_HASH_OVER_L3  = (1 << 1),
  SB_FE_HASH_OVER_L4  = (1 << 2),
  SB_FE_HASH_OVER_VID = (1 << 3)
}sbFeHashGeneration_t;

typedef struct sbCommonConfigParams_s {
  uint32_t maximumContiguousDmaMalloc;
  sbMalloc_f_t sbmalloc;
  sbFree_f_t sbfree;
  void *clientData;
  sbIsrSync_f_t isrSync;
  sbIsrUnsync_f_t isrUnsync;
  sbFeAsyncCallback_f_t asyncCallback;
  uint32_t uHashConfig;
} sbCommonConfigParams_t, *sbCommonConfigParams_p_t;

typedef struct sbIngressConfigParams_s {
  uint32_t ipv4McMaxIps;  
  uint32_t ipv4McSlabSize;
  uint32_t ipv4McMaxDirty;
  uint32_t payloadMax;
  uint32_t l2MaxMacs;
  uint32_t l2slabSize;
  uint32_t clsMaxL2Keys;
  uint32_t clsMaxRules;
  uint32_t svidCompSlabSize;
  uint32_t agingMaxDma;
  uint32_t ipv4MaxRoutes;
  uint32_t ipv4SlabSize;
  uint32_t aceRuleDbs;
  uint32_t aceRuleSetsPerDb;
  uint32_t aceRulesPerChunk;
  uint32_t aceMaxMemChunks;
  uint32_t ipv6EMKeys;
  uint32_t ipv6MaxRoutes;
  uint32_t ipv6SlabSize;
  uint32_t ipv6McMaxIps;
  uint32_t ipv6McSlabSize;
  uint32_t ipv4McGSlabSize;
  uint32_t ipv4McGMaxDirty;
  /* (*,g) related */
  uint32_t ipv4mcgSlabSize;
  uint32_t ipv4mcgMaxDirty;
  /* (s,g) related */
  uint32_t ipv4mcsgSlabSize;
  uint32_t ipv4mcsgMaxDirty;
} sbIngressConfigParams_t, *sbIngressConfigParams_p_t;

typedef struct sbIngressUcodeParams_s {
  uint8_t * ucodePackage;
  uint32_t payloadStartSrc;
  uint32_t payloadSizeSrc;
  uint32_t payloadIpmcGStartSrc;
  uint32_t payloadIpmcGSizeSrc;
  uint32_t payloadl2StartSrc;
  uint32_t payloadl2SizeSrc;
  uint32_t payloadl2StartDst;
  uint32_t payloadl2SizeDst;
  uint32_t payloadipv6StartSrc;
  uint32_t payloadipv6SizeSrc;
  uint32_t payloadipv6StartDst;
  uint32_t payloadipv6SizeDst;
  uint32_t l2Table1Bits;
  uint32_t l2SrcTable1Start;
  uint32_t l2SrcTable1Bank;
  uint32_t l2DstTable1Start;
  uint32_t l2DstTable1Bank;
  uint32_t l2MacTable1Start;
  uint32_t l2MacTable1Bank;
  uint32_t l2MacPayloadStart;
  uint32_t l2MacPayloadSize;
  uint32_t l2MacPayloadBank1;
  uint32_t l2MacPayloadBank2;
  uint32_t l2MacPayloadBank3;
  uint32_t sVid2EtcLeftBase;
  uint32_t sVid2EtcRightBase;
  uint32_t sVid2EtcPaylBase;
  uint32_t sVid2EtcPaylBaseRight;
  uint32_t sVid2EtcSeedLoc;
  uint32_t sVid2EtcLeftBank;
  uint32_t sVid2EtcRightBank;
  uint32_t sVid2EtcPaylBankLeft;
  uint32_t sVid2EtcPaylBankRight;
  uint32_t sVid2EtcTopOff;
  uint32_t sVid2EtcTableBits;
  uint32_t smacAgeBankBase;
  uint32_t ipv4McTable1Bits;
  uint32_t ipv4McTable1Base;
  /* For (*,G) Lookup. */
  uint32_t ipv4McGTable1Bits;
  uint32_t ipv4McGTable1Base;
  uint32_t ipv4DstTable1Start;
  uint32_t ipv4DstTable1Size;
  uint32_t ipv4DstTable1Bank;

  uint32_t ipv4SrcTable1Start;
  uint32_t ipv4SrcTable1Size;
  uint32_t ipv4SrcTable1Bank;
  uint32_t ipv4SrcTable2Start;
  uint32_t ipv4SrcTable2Size;
  uint32_t ipv4SrcTable2BankA;
  uint32_t ipv4SrcTable2BankB;
  uint32_t ipv4SrcTable2StartB;
  uint32_t ipv4SrcTable2SizeB;

  uint32_t ipv4DstTable2BankA;
  uint32_t ipv4DstTable2Start;
  uint32_t ipv4DstTable2Size;
  uint32_t ipv4DstTable2BankB;
  uint32_t ipv4DstTable2StartB;
  uint32_t ipv4DstTable2SizeB;
  uint32_t ipv4McTbl1Start;
  uint32_t ipv4McTbl1Size;
  /* For (*,G) Lookup. */
  uint32_t ipv4McGTbl1Start;
  uint32_t ipv4McGTbl1Size;
  uint32_t ipv6SaEmTable1Bank;
  uint32_t ipv6DaEmTable1Bank;
  uint32_t ipv6SaEmTable1Base;
  uint32_t ipv6DaEmTable1Base;
  uint32_t ipv6SaEmTable1Bits;
  uint32_t ipv6DaEmTable1Bits;
  uint32_t ipv6TableSize1;
  uint32_t ipv6SrcTableInSramA;
  uint32_t ipv6SrcTableStart1;
  uint32_t ipv6SrcTableSize1;
  uint32_t ipv6DstTableStart1;
  uint32_t ipv6DstTableSize1;
  uint32_t ipv6SrcTableBank2;
  uint32_t ipv6SrcTableStart2;
  uint32_t ipv6SrcTableSize2;
  uint32_t ipv6DstTableBank2;
  uint32_t ipv6DstTableStart2;
  uint32_t ipv6DstTableSize2;
  uint32_t ipv6McTable1Bits;
  uint32_t ipv6McTable1Base;
  uint32_t payloadipv6McStart;
  uint32_t payloadipv6McSize;
  /* Similar to already existing fields - deprecate the previous ones for lack of clear naming */
  /* (s,g) lookup */
  uint32_t ipv4mcsgTable1Bits;
  uint32_t ipv4mcsgTable1Base;
  uint32_t ipv4mcsgTable1Bank;
  uint32_t ipv4mcsgTable1Start;
  uint32_t payloadipv4mcsgStart;
  uint32_t payloadipv4mcsgSize;
  uint32_t payloadipv4mcsgBank;
  uint32_t payloadipv4mcsgBank2;
  uint32_t payloadipv4mcsgBank3;
  /* (*,g) lookup */
  uint32_t ipv4mcgTable1Bits;
  uint32_t ipv4mcgTable1Base;
  uint32_t ipv4mcgTable1Bank;
  uint32_t ipv4mcgTable1Start;
  uint32_t payloadipv4mcgStart;
  uint32_t payloadipv4mcgSize;
  uint32_t payloadipv4mcgBank;
  uint32_t payloadipv4mcgBank2;
} sbIngressUcodeParams_t, *sbIngressUcodeParams_p_t;

/* SWS and PPE ucode Port Configuration Information */
#define SBX_SWS_QUEUES    128
typedef struct sbFeSwsConfig_s {
  uint32_t  qid2PortId[SBX_SWS_QUEUES];
  uint32_t  port2QidId[SBX_SWS_QUEUES/2];
}sbFeSwsConfig_t;

/**
 * Type codes for the asynchronous event callback
 */
typedef enum sbG2FeAsyncType_e {
  SB_FE_ASYNC_INIT_DONE,     
  SB_FE_L2_ASYNC_COMMIT_DONE,
  SB_FE_L2_ASYNC_SMAC_UPDATE_DONE,
  SB_FE_L2_ASYNC_DMAC_UPDATE_DONE,
  SB_FE_L2_ASYNC_SMAC_GET_DONE,  
  SB_FE_L2_ASYNC_DMAC_GET_DONE, 
  SB_FE_ASYNC_SVID_COMMIT_DONE,
  SB_FE_ASYNC_SMAC_AGE_SET_DONE,
  SB_FE_ASYNC_SMAC_AGING_DONE,
  SB_FE_ASYNC_OLD_SMAC,
  SB_FE_ASYNC_IPV4MC_COMMIT_DONE,
  SB_FE_ASYNC_IPV4MC_GET_DONE,
  SB_FE_ASYNC_DMAC_UPDATE_DONE,
  SB_FE_ASYNC_SMAC_UPDATE_DONE,
  SB_FE_ASYNC_IPV4_COMMIT_DONE,
  SB_FE_ASYNC_IPV6EM_COMMIT_DONE,
  SB_FE_ASYNC_IPV6SAEM_UPDATE_DONE,
  SB_FE_ASYNC_IPV6DAEM_UPDATE_DONE,
  SB_FE_ASYNC_IPV6_COMMIT_DONE,
  SB_FE_ASYNC_IPV6MC_COMMIT_DONE,
  SB_FE_ASYNC_IPV6MC_UPDATE_DONE,
  SB_FE_ASYNC_IPV6MC_GET_DONE,
  SB_FE_ASYNC_EGR_LAG_DONE,
  SB_FE_ASYNC_EGR_SHIM_HEADER_EXPROC_DONE,
  SB_FE_ASYNC_EGR_SHIM_HEADER_MC_DONE,
  SB_FE_ASYNC_EGR_SHIM_HEADER_UC_DONE,
  SB_FE_ASYNC_ETE_RAW_DONE,
  SB_FE_ASYNC_ETE_L2_DONE,
  SB_FE_ASYNC_ETE_MPLS_DONE,
  SB_FE_ASYNC_ETE_IPV4_DONE,
  SB_FE_ASYNC_ETE_IPV4ENCAP_DONE,
  SB_FE_ASYNC_ETE_IPV6_DONE,
  SB_FE_ASYNC_ETE_L2ENCAP_DONE,
  SB_FE_ASYNC_ETE_MIM_DONE,
  SB_FE_ASYNC_EGR_REMARK_DONE,
  SB_FE_ASYNC_FTE_DONE,
  SB_FE_ASYNC_ING_LAG_DONE,
  SB_FE_ASYNC_PORT_SID_DROP_DONE,
  SB_FE_ASYNC_ISID2ETC_DONE,
  SB_FE_ASYNC_L2PROTO2PORT_DONE,
  SB_FE_ASYNC_LABEL2ETC_DONE,
  SB_FE_ASYNC_LABELVID2ETC_DONE,
  SB_FE_ASYNC_OUT_HDRINDEX2ETC_DONE,
  SB_FE_ASYNC_EGR_VLANPORT2ETC_DONE,
  SB_FE_ASYNC_PORT2ETC_DONE,
  SB_FE_ASYNC_PORT2SID_DONE,
  SB_FE_ASYNC_PORTPROTO2VLAN_DONE,
  SB_FE_ASYNC_PSTACKEDVID2ETC_DONE,
  SB_FE_ASYNC_PSTACKEDVIDSEED_DONE,
  SB_FE_ASYNC_PVID2ETC_DONE,
  SB_FE_ASYNC_MCPORT2ETC_DONE,
  SB_FE_ASYNC_PROT_DONE,
  SB_FE_ASYNC_EGRPVID2ETC_DONE,
  SB_FE_ASYNC_QIDRR_DONE,
  SB_FE_ASYNC_QOSPROFILE_DONE,
  SB_FE_ASYNC_QOS_MAP_DONE,
  SB_FE_ASYNC_REPSETENTRY_DONE,
  SB_FE_ASYNC_SMAC_IDX2SMAC_DONE,
  SB_FE_ASYNC_TUNNEL2ETC_DONE,
  SB_FE_ASYNC_VLAN2ETC_DONE,
  SB_FE_ASYNC_ROUTED_VLAN2ETC_DONE,
  SB_FE_ASYNC_RP2ETC_DONE,
  SB_FE_ASYNC_RULETABLE_DONE,
  SB_FE_ASYNC_EGRRULES_DONE,
  SB_FE_ASYNC_VRF2ETC_DONE,
  SB_FE_ASYNC_EXCEPTION_DONE,
  SB_FE_ASYNC_ING_PROT_DONE,
  SB_FE_ASYNC_L1SA_DONE,
  SB_FE_ASYNC_L1DA_DONE,
  SB_FE_ASYNC_TPID_DONE,
  SB_FE_ASYNC_CLS_COMMIT_DONE,
  SB_FE_ASYNC_OAMTIMERCALENDAR_DONE,
  SB_FE_ASYNC_OAMINSTANCE2ETC_DONE,
  SB_FE_ASYNC_OAMPORTMDLEVEL2ETC_DONE,
  SB_FE_ASYNC_OAMPORT2ETC_DONE,
  SB_FE_ASYNC_OAMENDPOINTRECORD_DONE,
  SB_FE_ASYNC_OAMENDPOINT2PEER_DONE,
  SB_FE_ASYNC_POLICER_DONE,
  SB_FE_ASYNC_INGMIRROR_DONE,
  SB_FE_ASYNC_EGRMIRROR_DONE,
  SB_FE_ASYNC_PORTL2CP2ETC_DONE,
  SB_FE_ASYNC_DMAC_IDX2DMAC_DONE,
  SB_FE_ASYNC_L2MAC_UPDATE_DONE,
  SB_FE_ASYNC_L2MAC_GET_DONE,
  SB_FE_ASYNC_L2MAC_COMMIT_DONE,
  SB_FE_ASYNC_FABRICCOSREMAP_DONE,
  SB_FE_ASYNC_TYPES_MAX
} sbG2FeAsyncType_t;


#endif
