/*
 * $Id: sbZfC2PpInitialQueueStateEntryConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfC2PpInitialQueueStateEntryConsole.hx"



/* Print members in struct */
void
sbZfC2PpInitialQueueStateEntry_Print(sbZfC2PpInitialQueueStateEntry_t *pFromStruct) {
  SB_LOG("C2PpInitialQueueStateEntry:: enetsmacmcast=0x%01x", (unsigned int)  pFromStruct->m_uEnetSmacMcast);
  SB_LOG(" enetvlan0eq3ff=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan0Eq3FF);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: enetvlan0eqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan0EqZero);
  SB_LOG(" syncsportlt1024=0x%01x", (unsigned int)  pFromStruct->m_uSyncSPortLt1024);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: unkmpls0lbl=0x%01x", (unsigned int)  pFromStruct->m_uUnkMpls0Lbl);
  SB_LOG(" unkmpls1lbl=0x%01x", (unsigned int)  pFromStruct->m_uUnkMpls1Lbl);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: unkmpls2lbl=0x%01x", (unsigned int)  pFromStruct->m_uUnkMpls2Lbl);
  SB_LOG(" enetvlan2eqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan2EqZero);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: enetvlan2eq3ff=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan2Eq3FF);
  SB_LOG(" ipv6saordamatch=0x%01x", (unsigned int)  pFromStruct->m_uIpv6SaOrDaMatch);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv6saordalpbk=0x%01x", (unsigned int)  pFromStruct->m_uIpv6SaOrDaLpbk);
  SB_LOG(" ipv4saordamatch=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaOrDaMatch);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv4saordamartian=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaOrDaMartian);
  SB_LOG(" udpspeqdp=0x%01x", (unsigned int)  pFromStruct->m_uUdpSpEqDp);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: tcptinyfrag=0x%01x", (unsigned int)  pFromStruct->m_uTcpTinyFrag);
  SB_LOG(" tcpsynfin=0x%01x", (unsigned int)  pFromStruct->m_uTcpSynFin);
  SB_LOG(" tcpxmasscan=0x%01x", (unsigned int)  pFromStruct->m_uTcpXmasScan);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: tcpnullscan=0x%01x", (unsigned int)  pFromStruct->m_uTcpNullScan);
  SB_LOG(" tcpspeqdp=0x%01x", (unsigned int)  pFromStruct->m_uTcpSpEqDp);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: enettypevalue=0x%01x", (unsigned int)  pFromStruct->m_uEnetTypeValue);
  SB_LOG(" enetmaceqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetMacEqZero);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: invpppaddrctrl=0x%01x", (unsigned int)  pFromStruct->m_uInvPppAddrCtrl);
  SB_LOG(" invppppid=0x%01x", (unsigned int)  pFromStruct->m_uInvPppPid);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: enetvlan1eqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan1EqZero);
  SB_LOG(" enetvlan1eq3ff=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan1Eq3FF);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: enetsmaceqdmac=0x%01x", (unsigned int)  pFromStruct->m_uEnetSmacEqDmac);
  SB_LOG(" invgreres0=0x%01x", (unsigned int)  pFromStruct->m_uInvGreRes0);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv4saordalpbk=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaOrDaLpbk);
  SB_LOG(" ipv4runtpkt=0x%01x", (unsigned int)  pFromStruct->m_uIpv4RuntPkt);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv4options=0x%01x", (unsigned int)  pFromStruct->m_uIpv4Options);
  SB_LOG(" invipv4checksum=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv4Checksum);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: invipv4version=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv4Version);
  SB_LOG(" invipv4runthdr=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv4RuntHdr);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv4lenerror=0x%01x", (unsigned int)  pFromStruct->m_uIpv4LenError);
  SB_LOG(" ipv4pktlenerr=0x%01x", (unsigned int)  pFromStruct->m_uIpv4PktLenErr);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv4invsa=0x%01x", (unsigned int)  pFromStruct->m_uIpv4InvSa);
  SB_LOG(" ipv4invda=0x%01x", (unsigned int)  pFromStruct->m_uIpv4InvDa);
  SB_LOG(" ipv4saeqda=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaEqDa);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv6runtpkt=0x%01x", (unsigned int)  pFromStruct->m_uIpv6RuntPkt);
  SB_LOG(" invipv6ver=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv6Ver);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv6pktlenerr=0x%01x", (unsigned int)  pFromStruct->m_uIpv6PktLenErr);
  SB_LOG(" ipv6invsa=0x%01x", (unsigned int)  pFromStruct->m_uIpv6InvSa);
  SB_LOG(" ipv6invda=0x%01x", (unsigned int)  pFromStruct->m_uIpv6InvDa);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: ipv6saeqda=0x%01x", (unsigned int)  pFromStruct->m_uIpv6SaEqDa);
  SB_LOG(" queuehash=0x%08x", (unsigned int)  pFromStruct->m_uQueueHash);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: headershift=0x%02x", (unsigned int)  pFromStruct->m_uHeaderShift);
  SB_LOG(" headertype=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType);
  SB_LOG(" state=0x%06x", (unsigned int)  pFromStruct->m_uState);
  SB_LOG("\n");

  SB_LOG("C2PpInitialQueueStateEntry:: variable=0x%08x", (unsigned int)  pFromStruct->m_uVariable);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PpInitialQueueStateEntry_SPrint(sbZfC2PpInitialQueueStateEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: enetsmacmcast=0x%01x", (unsigned int)  pFromStruct->m_uEnetSmacMcast);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enetvlan0eq3ff=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan0Eq3FF);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: enetvlan0eqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan0EqZero);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," syncsportlt1024=0x%01x", (unsigned int)  pFromStruct->m_uSyncSPortLt1024);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: unkmpls0lbl=0x%01x", (unsigned int)  pFromStruct->m_uUnkMpls0Lbl);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," unkmpls1lbl=0x%01x", (unsigned int)  pFromStruct->m_uUnkMpls1Lbl);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: unkmpls2lbl=0x%01x", (unsigned int)  pFromStruct->m_uUnkMpls2Lbl);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enetvlan2eqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan2EqZero);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: enetvlan2eq3ff=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan2Eq3FF);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv6saordamatch=0x%01x", (unsigned int)  pFromStruct->m_uIpv6SaOrDaMatch);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv6saordalpbk=0x%01x", (unsigned int)  pFromStruct->m_uIpv6SaOrDaLpbk);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv4saordamatch=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaOrDaMatch);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv4saordamartian=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaOrDaMartian);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," udpspeqdp=0x%01x", (unsigned int)  pFromStruct->m_uUdpSpEqDp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: tcptinyfrag=0x%01x", (unsigned int)  pFromStruct->m_uTcpTinyFrag);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tcpsynfin=0x%01x", (unsigned int)  pFromStruct->m_uTcpSynFin);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tcpxmasscan=0x%01x", (unsigned int)  pFromStruct->m_uTcpXmasScan);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: tcpnullscan=0x%01x", (unsigned int)  pFromStruct->m_uTcpNullScan);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tcpspeqdp=0x%01x", (unsigned int)  pFromStruct->m_uTcpSpEqDp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: enettypevalue=0x%01x", (unsigned int)  pFromStruct->m_uEnetTypeValue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enetmaceqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetMacEqZero);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: invpppaddrctrl=0x%01x", (unsigned int)  pFromStruct->m_uInvPppAddrCtrl);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," invppppid=0x%01x", (unsigned int)  pFromStruct->m_uInvPppPid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: enetvlan1eqzero=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan1EqZero);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enetvlan1eq3ff=0x%01x", (unsigned int)  pFromStruct->m_uEnetVlan1Eq3FF);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: enetsmaceqdmac=0x%01x", (unsigned int)  pFromStruct->m_uEnetSmacEqDmac);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," invgreres0=0x%01x", (unsigned int)  pFromStruct->m_uInvGreRes0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv4saordalpbk=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaOrDaLpbk);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv4runtpkt=0x%01x", (unsigned int)  pFromStruct->m_uIpv4RuntPkt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv4options=0x%01x", (unsigned int)  pFromStruct->m_uIpv4Options);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," invipv4checksum=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv4Checksum);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: invipv4version=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv4Version);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," invipv4runthdr=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv4RuntHdr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv4lenerror=0x%01x", (unsigned int)  pFromStruct->m_uIpv4LenError);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv4pktlenerr=0x%01x", (unsigned int)  pFromStruct->m_uIpv4PktLenErr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv4invsa=0x%01x", (unsigned int)  pFromStruct->m_uIpv4InvSa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv4invda=0x%01x", (unsigned int)  pFromStruct->m_uIpv4InvDa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv4saeqda=0x%01x", (unsigned int)  pFromStruct->m_uIpv4SaEqDa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv6runtpkt=0x%01x", (unsigned int)  pFromStruct->m_uIpv6RuntPkt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," invipv6ver=0x%01x", (unsigned int)  pFromStruct->m_uInvIpv6Ver);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv6pktlenerr=0x%01x", (unsigned int)  pFromStruct->m_uIpv6PktLenErr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv6invsa=0x%01x", (unsigned int)  pFromStruct->m_uIpv6InvSa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ipv6invda=0x%01x", (unsigned int)  pFromStruct->m_uIpv6InvDa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: ipv6saeqda=0x%01x", (unsigned int)  pFromStruct->m_uIpv6SaEqDa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," queuehash=0x%08x", (unsigned int)  pFromStruct->m_uQueueHash);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: headershift=0x%02x", (unsigned int)  pFromStruct->m_uHeaderShift);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," headertype=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," state=0x%06x", (unsigned int)  pFromStruct->m_uState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpInitialQueueStateEntry:: variable=0x%08x", (unsigned int)  pFromStruct->m_uVariable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PpInitialQueueStateEntry_Validate(sbZfC2PpInitialQueueStateEntry_t *pZf) {

  if (pZf->m_uEnetSmacMcast > 0x1) return 0;
  if (pZf->m_uEnetVlan0Eq3FF > 0x1) return 0;
  if (pZf->m_uEnetVlan0EqZero > 0x1) return 0;
  if (pZf->m_uSyncSPortLt1024 > 0x1) return 0;
  if (pZf->m_uUnkMpls0Lbl > 0x1) return 0;
  if (pZf->m_uUnkMpls1Lbl > 0x1) return 0;
  if (pZf->m_uUnkMpls2Lbl > 0x1) return 0;
  if (pZf->m_uEnetVlan2EqZero > 0x1) return 0;
  if (pZf->m_uEnetVlan2Eq3FF > 0x1) return 0;
  if (pZf->m_uIpv6SaOrDaMatch > 0x1) return 0;
  if (pZf->m_uIpv6SaOrDaLpbk > 0x1) return 0;
  if (pZf->m_uIpv4SaOrDaMatch > 0x1) return 0;
  if (pZf->m_uIpv4SaOrDaMartian > 0x1) return 0;
  if (pZf->m_uUdpSpEqDp > 0x1) return 0;
  if (pZf->m_uTcpTinyFrag > 0x1) return 0;
  if (pZf->m_uTcpSynFin > 0x1) return 0;
  if (pZf->m_uTcpXmasScan > 0x1) return 0;
  if (pZf->m_uTcpNullScan > 0x1) return 0;
  if (pZf->m_uTcpSpEqDp > 0x1) return 0;
  if (pZf->m_uEnetTypeValue > 0x1) return 0;
  if (pZf->m_uEnetMacEqZero > 0x1) return 0;
  if (pZf->m_uInvPppAddrCtrl > 0x1) return 0;
  if (pZf->m_uInvPppPid > 0x1) return 0;
  if (pZf->m_uEnetVlan1EqZero > 0x1) return 0;
  if (pZf->m_uEnetVlan1Eq3FF > 0x1) return 0;
  if (pZf->m_uEnetSmacEqDmac > 0x1) return 0;
  if (pZf->m_uInvGreRes0 > 0x1) return 0;
  if (pZf->m_uIpv4SaOrDaLpbk > 0x1) return 0;
  if (pZf->m_uIpv4RuntPkt > 0x1) return 0;
  if (pZf->m_uIpv4Options > 0x1) return 0;
  if (pZf->m_uInvIpv4Checksum > 0x1) return 0;
  if (pZf->m_uInvIpv4Version > 0x1) return 0;
  if (pZf->m_uInvIpv4RuntHdr > 0x1) return 0;
  if (pZf->m_uIpv4LenError > 0x1) return 0;
  if (pZf->m_uIpv4PktLenErr > 0x1) return 0;
  if (pZf->m_uIpv4InvSa > 0x1) return 0;
  if (pZf->m_uIpv4InvDa > 0x1) return 0;
  if (pZf->m_uIpv4SaEqDa > 0x1) return 0;
  if (pZf->m_uIpv6RuntPkt > 0x1) return 0;
  if (pZf->m_uInvIpv6Ver > 0x1) return 0;
  if (pZf->m_uIpv6PktLenErr > 0x1) return 0;
  if (pZf->m_uIpv6InvSa > 0x1) return 0;
  if (pZf->m_uIpv6InvDa > 0x1) return 0;
  if (pZf->m_uIpv6SaEqDa > 0x1) return 0;
  /* pZf->m_uQueueHash implicitly masked by data type */
  if (pZf->m_uHeaderShift > 0x3f) return 0;
  if (pZf->m_uHeaderType > 0xf) return 0;
  if (pZf->m_uState > 0xffffff) return 0;
  /* pZf->m_uVariable implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PpInitialQueueStateEntry_SetField(sbZfC2PpInitialQueueStateEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uenetsmacmcast") == 0) {
    s->m_uEnetSmacMcast = value;
  } else if (SB_STRCMP(name, "m_uenetvlan0eq3ff") == 0) {
    s->m_uEnetVlan0Eq3FF = value;
  } else if (SB_STRCMP(name, "m_uenetvlan0eqzero") == 0) {
    s->m_uEnetVlan0EqZero = value;
  } else if (SB_STRCMP(name, "m_usyncsportlt1024") == 0) {
    s->m_uSyncSPortLt1024 = value;
  } else if (SB_STRCMP(name, "m_uunkmpls0lbl") == 0) {
    s->m_uUnkMpls0Lbl = value;
  } else if (SB_STRCMP(name, "m_uunkmpls1lbl") == 0) {
    s->m_uUnkMpls1Lbl = value;
  } else if (SB_STRCMP(name, "m_uunkmpls2lbl") == 0) {
    s->m_uUnkMpls2Lbl = value;
  } else if (SB_STRCMP(name, "m_uenetvlan2eqzero") == 0) {
    s->m_uEnetVlan2EqZero = value;
  } else if (SB_STRCMP(name, "m_uenetvlan2eq3ff") == 0) {
    s->m_uEnetVlan2Eq3FF = value;
  } else if (SB_STRCMP(name, "m_uipv6saordamatch") == 0) {
    s->m_uIpv6SaOrDaMatch = value;
  } else if (SB_STRCMP(name, "m_uipv6saordalpbk") == 0) {
    s->m_uIpv6SaOrDaLpbk = value;
  } else if (SB_STRCMP(name, "m_uipv4saordamatch") == 0) {
    s->m_uIpv4SaOrDaMatch = value;
  } else if (SB_STRCMP(name, "m_uipv4saordamartian") == 0) {
    s->m_uIpv4SaOrDaMartian = value;
  } else if (SB_STRCMP(name, "m_uudpspeqdp") == 0) {
    s->m_uUdpSpEqDp = value;
  } else if (SB_STRCMP(name, "m_utcptinyfrag") == 0) {
    s->m_uTcpTinyFrag = value;
  } else if (SB_STRCMP(name, "m_utcpsynfin") == 0) {
    s->m_uTcpSynFin = value;
  } else if (SB_STRCMP(name, "m_utcpxmasscan") == 0) {
    s->m_uTcpXmasScan = value;
  } else if (SB_STRCMP(name, "m_utcpnullscan") == 0) {
    s->m_uTcpNullScan = value;
  } else if (SB_STRCMP(name, "m_utcpspeqdp") == 0) {
    s->m_uTcpSpEqDp = value;
  } else if (SB_STRCMP(name, "m_uenettypevalue") == 0) {
    s->m_uEnetTypeValue = value;
  } else if (SB_STRCMP(name, "m_uenetmaceqzero") == 0) {
    s->m_uEnetMacEqZero = value;
  } else if (SB_STRCMP(name, "m_uinvpppaddrctrl") == 0) {
    s->m_uInvPppAddrCtrl = value;
  } else if (SB_STRCMP(name, "m_uinvppppid") == 0) {
    s->m_uInvPppPid = value;
  } else if (SB_STRCMP(name, "m_uenetvlan1eqzero") == 0) {
    s->m_uEnetVlan1EqZero = value;
  } else if (SB_STRCMP(name, "m_uenetvlan1eq3ff") == 0) {
    s->m_uEnetVlan1Eq3FF = value;
  } else if (SB_STRCMP(name, "m_uenetsmaceqdmac") == 0) {
    s->m_uEnetSmacEqDmac = value;
  } else if (SB_STRCMP(name, "m_uinvgreres0") == 0) {
    s->m_uInvGreRes0 = value;
  } else if (SB_STRCMP(name, "m_uipv4saordalpbk") == 0) {
    s->m_uIpv4SaOrDaLpbk = value;
  } else if (SB_STRCMP(name, "m_uipv4runtpkt") == 0) {
    s->m_uIpv4RuntPkt = value;
  } else if (SB_STRCMP(name, "m_uipv4options") == 0) {
    s->m_uIpv4Options = value;
  } else if (SB_STRCMP(name, "m_uinvipv4checksum") == 0) {
    s->m_uInvIpv4Checksum = value;
  } else if (SB_STRCMP(name, "m_uinvipv4version") == 0) {
    s->m_uInvIpv4Version = value;
  } else if (SB_STRCMP(name, "m_uinvipv4runthdr") == 0) {
    s->m_uInvIpv4RuntHdr = value;
  } else if (SB_STRCMP(name, "m_uipv4lenerror") == 0) {
    s->m_uIpv4LenError = value;
  } else if (SB_STRCMP(name, "m_uipv4pktlenerr") == 0) {
    s->m_uIpv4PktLenErr = value;
  } else if (SB_STRCMP(name, "m_uipv4invsa") == 0) {
    s->m_uIpv4InvSa = value;
  } else if (SB_STRCMP(name, "m_uipv4invda") == 0) {
    s->m_uIpv4InvDa = value;
  } else if (SB_STRCMP(name, "m_uipv4saeqda") == 0) {
    s->m_uIpv4SaEqDa = value;
  } else if (SB_STRCMP(name, "m_uipv6runtpkt") == 0) {
    s->m_uIpv6RuntPkt = value;
  } else if (SB_STRCMP(name, "m_uinvipv6ver") == 0) {
    s->m_uInvIpv6Ver = value;
  } else if (SB_STRCMP(name, "m_uipv6pktlenerr") == 0) {
    s->m_uIpv6PktLenErr = value;
  } else if (SB_STRCMP(name, "m_uipv6invsa") == 0) {
    s->m_uIpv6InvSa = value;
  } else if (SB_STRCMP(name, "m_uipv6invda") == 0) {
    s->m_uIpv6InvDa = value;
  } else if (SB_STRCMP(name, "m_uipv6saeqda") == 0) {
    s->m_uIpv6SaEqDa = value;
  } else if (SB_STRCMP(name, "m_uqueuehash") == 0) {
    s->m_uQueueHash = value;
  } else if (SB_STRCMP(name, "m_uheadershift") == 0) {
    s->m_uHeaderShift = value;
  } else if (SB_STRCMP(name, "m_uheadertype") == 0) {
    s->m_uHeaderType = value;
  } else if (SB_STRCMP(name, "m_ustate") == 0) {
    s->m_uState = value;
  } else if (SB_STRCMP(name, "m_uvariable") == 0) {
    s->m_uVariable = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
