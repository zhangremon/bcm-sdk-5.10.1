/*
 * $Id: sbZfQeIncludes.h 1.4 Broadcom SDK $
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
 * This file contains aggregated definitions for Guadalupe 2.x microcode
 */

#ifndef _SOC_SBX_QE2K_INCLUDESX_H
#define _SOC_SBX_QE2K_INCLUDESX_H

#include <soc/sbx/sbTypesGlue.h>

/*
 * include other specific structs defined for zframes
 */
#include "sbZfKaEbMvtAddress.hx"
#include "sbZfKaEbMvtAddressConsole.hx"
#include "sbZfKaEbMvtEntry.hx"
#include "sbZfKaEbMvtEntryConsole.hx"
#include "sbZfKaEgMemFifoControlEntry.hx"
#include "sbZfKaEgMemFifoControlEntryConsole.hx"
#include "sbZfKaEgMemFifoParamEntry.hx"
#include "sbZfKaEgMemFifoParamEntryConsole.hx"
#include "sbZfKaEgMemQCtlEntry.hx"
#include "sbZfKaEgMemQCtlEntryConsole.hx"
#include "sbZfKaEgMemShapingEntry.hx"
#include "sbZfKaEgMemShapingEntryConsole.hx"
#include "sbZfKaEgNotTmePortRemapAddr.hx"
#include "sbZfKaEgNotTmePortRemapAddrConsole.hx"
#include "sbZfKaEgPortRemapEntry.hx"
#include "sbZfKaEgPortRemapEntryConsole.hx"
#include "sbZfKaEgSrcId.hx"
#include "sbZfKaEgSrcIdConsole.hx"
#include "sbZfKaEgTmePortRemapAddr.hx"
#include "sbZfKaEgTmePortRemapAddrConsole.hx"
#include "sbZfKaEiMemDataEntry.hx"
#include "sbZfKaEiMemDataEntryConsole.hx"
#include "sbZfKaEiRawSpiReadEntry.hx"
#include "sbZfKaEiRawSpiReadEntryConsole.hx"
#include "sbZfKaEpBfPriTableAddr.hx"
#include "sbZfKaEpBfPriTableAddrConsole.hx"
#include "sbZfKaEpBfPriTableEntry.hx"
#include "sbZfKaEpBfPriTableEntryConsole.hx"
#include "sbZfKaEpCrTableEntry.hx"
#include "sbZfKaEpCrTableEntryConsole.hx"
#include "sbZfKaEpInstruction.hx"
#include "sbZfKaEpInstructionConsole.hx"
#include "sbZfKaEpIp16BitRewrite.hx"
#include "sbZfKaEpIp16BitRewriteConsole.hx"
#include "sbZfKaEpIp32BitRewrite.hx"
#include "sbZfKaEpIp32BitRewriteConsole.hx"
#include "sbZfKaEpIpCounter.hx"
#include "sbZfKaEpIpCounterConsole.hx"
#include "sbZfKaEpIpFourBitEntry.hx"
#include "sbZfKaEpIpFourBitEntryConsole.hx"
#include "sbZfKaEpIpMplsLabels.hx"
#include "sbZfKaEpIpMplsLabelsConsole.hx"
#include "sbZfKaEpIpOneBitEntry.hx"
#include "sbZfKaEpIpOneBitEntryConsole.hx"
#include "sbZfKaEpIpPortVridSmacTableEntry.hx"
#include "sbZfKaEpIpPortVridSmacTableEntryConsole.hx"
#include "sbZfKaEpIpPrepend.hx"
#include "sbZfKaEpIpPrependConsole.hx"
#include "sbZfKaEpIpPriExpTosRewrite.hx"
#include "sbZfKaEpIpPriExpTosRewriteConsole.hx"
#include "sbZfKaEpIpTciDmac.hx"
#include "sbZfKaEpIpTciDmacConsole.hx"
#include "sbZfKaEpIpTtlRange.hx"
#include "sbZfKaEpIpTtlRangeConsole.hx"
#include "sbZfKaEpIpTwoBitEntry.hx"
#include "sbZfKaEpIpTwoBitEntryConsole.hx"
#include "sbZfKaEpIpV6Tci.hx"
#include "sbZfKaEpIpV6TciConsole.hx"
#include "sbZfKaEpPortTableEntry.hx"
#include "sbZfKaEpPortTableEntryConsole.hx"
#include "sbZfKaEpSlimVlanRecord.hx"
#include "sbZfKaEpSlimVlanRecordConsole.hx"
#include "sbZfKaEpVlanIndRecord.hx"
#include "sbZfKaEpVlanIndRecordConsole.hx"
#include "sbZfKaEpVlanIndTableEntry.hx"
#include "sbZfKaEpVlanIndTableEntryConsole.hx"
#include "sbZfKaPmLastLine.hx"
#include "sbZfKaPmLastLineConsole.hx"
#include "sbZfKaQmBaaCfgTableEntry.hx"
#include "sbZfKaQmBaaCfgTableEntryConsole.hx"
#include "sbZfKaQmDemandCfgDataEntry.hx"
#include "sbZfKaQmDemandCfgDataEntryConsole.hx"
#include "sbZfKaQmFbLine.hx"
#include "sbZfKaQmFbLineConsole.hx"
#include "sbZfKaQmIngressPortEntry.hx"
#include "sbZfKaQmIngressPortEntryConsole.hx"
#include "sbZfKaQmPortBwCfgTableEntry.hx"
#include "sbZfKaQmPortBwCfgTableEntryConsole.hx"
#include "sbZfKaQmQueueAgeEntry.hx"
#include "sbZfKaQmQueueAgeEntryConsole.hx"
#include "sbZfKaQmQueueArrivalsEntry.hx"
#include "sbZfKaQmQueueArrivalsEntryConsole.hx"
#include "sbZfKaQmQueueByteAdjEntry.hx"
#include "sbZfKaQmQueueByteAdjEntryConsole.hx"
#include "sbZfKaQmQueueStateEntry.hx"
#include "sbZfKaQmQueueStateEntryConsole.hx"
#include "sbZfKaQmSlqCntrsEntry.hx"
#include "sbZfKaQmSlqCntrsEntryConsole.hx"
#include "sbZfKaQmWredAvQlenTableEntry.hx"
#include "sbZfKaQmWredAvQlenTableEntryConsole.hx"
#include "sbZfKaQmWredCfgTableEntry.hx"
#include "sbZfKaQmWredCfgTableEntryConsole.hx"
#include "sbZfKaQmWredCurvesTableEntry.hx"
#include "sbZfKaQmWredCurvesTableEntryConsole.hx"
#include "sbZfKaQmWredParamEntry.hx"
#include "sbZfKaQmWredParamEntryConsole.hx"
#include "sbZfKaQsAgeEntry.hx"
#include "sbZfKaQsAgeEntryConsole.hx"
#include "sbZfKaQsAgeThreshLutEntry.hx"
#include "sbZfKaQsAgeThreshLutEntryConsole.hx"
#include "sbZfKaQsDepthHplenEntry.hx"
#include "sbZfKaQsDepthHplenEntryConsole.hx"
#include "sbZfKaQsE2QAddr.hx"
#include "sbZfKaQsE2QAddrConsole.hx"
#include "sbZfKaQsE2QEntry.hx"
#include "sbZfKaQsE2QEntryConsole.hx"
#include "sbZfKaQsLastSentPriAddr.hx"
#include "sbZfKaQsLastSentPriAddrConsole.hx"
#include "sbZfKaQsLastSentPriEntry.hx"
#include "sbZfKaQsLastSentPriEntryConsole.hx"
#include "sbZfKaQsLnaPortRemapEntry.hx"
#include "sbZfKaQsLnaPortRemapEntryConsole.hx"
#include "sbZfKaQsLnaPriEntry.hx"
#include "sbZfKaQsLnaPriEntryConsole.hx"
#include "sbZfKaQsLnaNextPriEntry.hx"
#include "sbZfKaQsLnaNextPriEntryConsole.hx"
#include "sbZfKaQsPriAddr.hx"
#include "sbZfKaQsPriAddrConsole.hx"
#include "sbZfKaQsPriEntry.hx"
#include "sbZfKaQsPriEntryConsole.hx"
#include "sbZfKaQsPriLutAddr.hx"
#include "sbZfKaQsPriLutAddrConsole.hx"
#include "sbZfKaQsPriLutEntry.hx"
#include "sbZfKaQsPriLutEntryConsole.hx"
#include "sbZfKaQsQ2EcEntry.hx"
#include "sbZfKaQsQ2EcEntryConsole.hx"
#include "sbZfKaQsQueueParamEntry.hx"
#include "sbZfKaQsQueueParamEntryConsole.hx"
#include "sbZfKaQsQueueTableEntry.hx"
#include "sbZfKaQsQueueTableEntryConsole.hx"
#include "sbZfKaQsRateTableEntry.hx"
#include "sbZfKaQsRateTableEntryConsole.hx"
#include "sbZfKaQsShapeMaxBurstEntry.hx"
#include "sbZfKaQsShapeMaxBurstEntryConsole.hx"
#include "sbZfKaQsShapeRateEntry.hx"
#include "sbZfKaQsShapeRateEntryConsole.hx"
#include "sbZfKaQsShapeTableEntry.hx"
#include "sbZfKaQsShapeTableEntryConsole.hx"
#include "sbZfKaQmRateDeltaMaxTableEntry.hx"
#include "sbZfKaQmRateDeltaMaxTableEntryConsole.hx"
#include "sbZfKaRbClassDefaultQEntry.hx"
#include "sbZfKaRbClassDefaultQEntryConsole.hx"
#include "sbZfKaRbClassDmacMatchEntry.hx"
#include "sbZfKaRbClassDmacMatchEntryConsole.hx"
#include "sbZfKaRbClassHashIPv4Only.hx"
#include "sbZfKaRbClassHashIPv4OnlyConsole.hx"
#include "sbZfKaRbClassHashIPv6Only.hx"
#include "sbZfKaRbClassHashIPv6OnlyConsole.hx"
#include "sbZfKaRbClassHashInputW0.hx"
#include "sbZfKaRbClassHashInputW0Console.hx"
#include "sbZfKaRbClassHashSVlanIPv4.hx"
#include "sbZfKaRbClassHashSVlanIPv4Console.hx"
#include "sbZfKaRbClassHashSVlanIPv6.hx"
#include "sbZfKaRbClassHashSVlanIPv6Console.hx"
#include "sbZfKaRbClassHashVlanIPv4.hx"
#include "sbZfKaRbClassHashVlanIPv4Console.hx"
#include "sbZfKaRbClassHashVlanIPv6.hx"
#include "sbZfKaRbClassHashVlanIPv6Console.hx"
#include "sbZfKaRbClassIPv4TosEntry.hx"
#include "sbZfKaRbClassIPv4TosEntryConsole.hx"
#include "sbZfKaRbClassIPv6ClassEntry.hx"
#include "sbZfKaRbClassIPv6ClassEntryConsole.hx"
#include "sbZfKaRbClassPortEnablesEntry.hx"
#include "sbZfKaRbClassPortEnablesEntryConsole.hx"
#include "sbZfKaRbClassProtocolEntry.hx"
#include "sbZfKaRbClassProtocolEntryConsole.hx"
#include "sbZfKaRbClassSourceIdEntry.hx"
#include "sbZfKaRbClassSourceIdEntryConsole.hx"
#include "sbZfKaRbPoliceCBSEntry.hx"
#include "sbZfKaRbPoliceCBSEntryConsole.hx"
#include "sbZfKaRbPoliceCfgCtrlEntry.hx"
#include "sbZfKaRbPoliceCfgCtrlEntryConsole.hx"
#include "sbZfKaRbPoliceEBSEntry.hx"
#include "sbZfKaRbPoliceEBSEntryConsole.hx"
#include "sbZfKaRbPolicePortQMapEntry.hx"
#include "sbZfKaRbPolicePortQMapEntryConsole.hx"
#include "sbZfKaSrManualDeskewEntry.hx"
#include "sbZfKaSrManualDeskewEntryConsole.hx"


#endif  /* !_SOC_SBX_QE2K_INCLUDES_H */
