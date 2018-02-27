/*
 * $Id: sbZfC2PpInitialQueueStateEntry.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2PpInitialQueueStateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2PpInitialQueueStateEntry_Pack(sbZfC2PpInitialQueueStateEntry_t *pFrom,
                                    uint8_t *pToData,
                                    uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2PPINITIALQUEUESTATEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uEnetSmacMcast */
  (pToData)[17] |= ((pFrom)->m_uEnetSmacMcast & 0x01) <<5;

  /* Pack Member: m_uEnetVlan0Eq3FF */
  (pToData)[17] |= ((pFrom)->m_uEnetVlan0Eq3FF & 0x01) <<4;

  /* Pack Member: m_uEnetVlan0EqZero */
  (pToData)[17] |= ((pFrom)->m_uEnetVlan0EqZero & 0x01) <<3;

  /* Pack Member: m_uSyncSPortLt1024 */
  (pToData)[17] |= ((pFrom)->m_uSyncSPortLt1024 & 0x01) <<2;

  /* Pack Member: m_uUnkMpls0Lbl */
  (pToData)[17] |= ((pFrom)->m_uUnkMpls0Lbl & 0x01) <<1;

  /* Pack Member: m_uUnkMpls1Lbl */
  (pToData)[17] |= ((pFrom)->m_uUnkMpls1Lbl & 0x01);

  /* Pack Member: m_uUnkMpls2Lbl */
  (pToData)[16] |= ((pFrom)->m_uUnkMpls2Lbl & 0x01) <<7;

  /* Pack Member: m_uEnetVlan2EqZero */
  (pToData)[16] |= ((pFrom)->m_uEnetVlan2EqZero & 0x01) <<6;

  /* Pack Member: m_uEnetVlan2Eq3FF */
  (pToData)[16] |= ((pFrom)->m_uEnetVlan2Eq3FF & 0x01) <<5;

  /* Pack Member: m_uIpv6SaOrDaMatch */
  (pToData)[16] |= ((pFrom)->m_uIpv6SaOrDaMatch & 0x01) <<4;

  /* Pack Member: m_uIpv6SaOrDaLpbk */
  (pToData)[16] |= ((pFrom)->m_uIpv6SaOrDaLpbk & 0x01) <<3;

  /* Pack Member: m_uIpv4SaOrDaMatch */
  (pToData)[16] |= ((pFrom)->m_uIpv4SaOrDaMatch & 0x01) <<2;

  /* Pack Member: m_uIpv4SaOrDaMartian */
  (pToData)[16] |= ((pFrom)->m_uIpv4SaOrDaMartian & 0x01) <<1;

  /* Pack Member: m_uUdpSpEqDp */
  (pToData)[16] |= ((pFrom)->m_uUdpSpEqDp & 0x01);

  /* Pack Member: m_uTcpTinyFrag */
  (pToData)[15] |= ((pFrom)->m_uTcpTinyFrag & 0x01) <<7;

  /* Pack Member: m_uTcpSynFin */
  (pToData)[15] |= ((pFrom)->m_uTcpSynFin & 0x01) <<6;

  /* Pack Member: m_uTcpXmasScan */
  (pToData)[15] |= ((pFrom)->m_uTcpXmasScan & 0x01) <<5;

  /* Pack Member: m_uTcpNullScan */
  (pToData)[15] |= ((pFrom)->m_uTcpNullScan & 0x01) <<4;

  /* Pack Member: m_uTcpSpEqDp */
  (pToData)[15] |= ((pFrom)->m_uTcpSpEqDp & 0x01) <<3;

  /* Pack Member: m_uEnetTypeValue */
  (pToData)[15] |= ((pFrom)->m_uEnetTypeValue & 0x01) <<2;

  /* Pack Member: m_uEnetMacEqZero */
  (pToData)[15] |= ((pFrom)->m_uEnetMacEqZero & 0x01) <<1;

  /* Pack Member: m_uInvPppAddrCtrl */
  (pToData)[15] |= ((pFrom)->m_uInvPppAddrCtrl & 0x01);

  /* Pack Member: m_uInvPppPid */
  (pToData)[14] |= ((pFrom)->m_uInvPppPid & 0x01) <<7;

  /* Pack Member: m_uEnetVlan1EqZero */
  (pToData)[14] |= ((pFrom)->m_uEnetVlan1EqZero & 0x01) <<6;

  /* Pack Member: m_uEnetVlan1Eq3FF */
  (pToData)[14] |= ((pFrom)->m_uEnetVlan1Eq3FF & 0x01) <<5;

  /* Pack Member: m_uEnetSmacEqDmac */
  (pToData)[14] |= ((pFrom)->m_uEnetSmacEqDmac & 0x01) <<4;

  /* Pack Member: m_uInvGreRes0 */
  (pToData)[14] |= ((pFrom)->m_uInvGreRes0 & 0x01) <<3;

  /* Pack Member: m_uIpv4SaOrDaLpbk */
  (pToData)[14] |= ((pFrom)->m_uIpv4SaOrDaLpbk & 0x01) <<2;

  /* Pack Member: m_uIpv4RuntPkt */
  (pToData)[14] |= ((pFrom)->m_uIpv4RuntPkt & 0x01) <<1;

  /* Pack Member: m_uIpv4Options */
  (pToData)[14] |= ((pFrom)->m_uIpv4Options & 0x01);

  /* Pack Member: m_uInvIpv4Checksum */
  (pToData)[13] |= ((pFrom)->m_uInvIpv4Checksum & 0x01) <<7;

  /* Pack Member: m_uInvIpv4Version */
  (pToData)[13] |= ((pFrom)->m_uInvIpv4Version & 0x01) <<6;

  /* Pack Member: m_uInvIpv4RuntHdr */
  (pToData)[13] |= ((pFrom)->m_uInvIpv4RuntHdr & 0x01) <<5;

  /* Pack Member: m_uIpv4LenError */
  (pToData)[13] |= ((pFrom)->m_uIpv4LenError & 0x01) <<4;

  /* Pack Member: m_uIpv4PktLenErr */
  (pToData)[13] |= ((pFrom)->m_uIpv4PktLenErr & 0x01) <<3;

  /* Pack Member: m_uIpv4InvSa */
  (pToData)[13] |= ((pFrom)->m_uIpv4InvSa & 0x01) <<2;

  /* Pack Member: m_uIpv4InvDa */
  (pToData)[13] |= ((pFrom)->m_uIpv4InvDa & 0x01) <<1;

  /* Pack Member: m_uIpv4SaEqDa */
  (pToData)[13] |= ((pFrom)->m_uIpv4SaEqDa & 0x01);

  /* Pack Member: m_uIpv6RuntPkt */
  (pToData)[12] |= ((pFrom)->m_uIpv6RuntPkt & 0x01) <<7;

  /* Pack Member: m_uInvIpv6Ver */
  (pToData)[12] |= ((pFrom)->m_uInvIpv6Ver & 0x01) <<6;

  /* Pack Member: m_uIpv6PktLenErr */
  (pToData)[12] |= ((pFrom)->m_uIpv6PktLenErr & 0x01) <<5;

  /* Pack Member: m_uIpv6InvSa */
  (pToData)[12] |= ((pFrom)->m_uIpv6InvSa & 0x01) <<4;

  /* Pack Member: m_uIpv6InvDa */
  (pToData)[12] |= ((pFrom)->m_uIpv6InvDa & 0x01) <<3;

  /* Pack Member: m_uIpv6SaEqDa */
  (pToData)[12] |= ((pFrom)->m_uIpv6SaEqDa & 0x01) <<2;

  /* Pack Member: m_uQueueHash */
  (pToData)[8] |= ((pFrom)->m_uQueueHash & 0x3f) <<2;
  (pToData)[9] |= ((pFrom)->m_uQueueHash >> 6) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uQueueHash >> 14) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uQueueHash >> 22) &0xFF;
  (pToData)[12] |= ((pFrom)->m_uQueueHash >> 30) & 0x03;

  /* Pack Member: m_uHeaderShift */
  (pToData)[7] |= ((pFrom)->m_uHeaderShift & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_uHeaderShift >> 4) & 0x03;

  /* Pack Member: m_uHeaderType */
  (pToData)[7] |= ((pFrom)->m_uHeaderType & 0x0f);

  /* Pack Member: m_uState */
  (pToData)[4] |= ((pFrom)->m_uState) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uState >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uState >> 16) &0xFF;

  /* Pack Member: m_uVariable */
  (pToData)[0] |= ((pFrom)->m_uVariable) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uVariable >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uVariable >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uVariable >> 24) &0xFF;

  return SB_ZF_C2PPINITIALQUEUESTATEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2PpInitialQueueStateEntry_Unpack(sbZfC2PpInitialQueueStateEntry_t *pToStruct,
                                      uint8_t *pFromData,
                                      uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uEnetSmacMcast */
  (pToStruct)->m_uEnetSmacMcast =  (uint32_t)  ((pFromData)[17] >> 5) & 0x01;

  /* Unpack Member: m_uEnetVlan0Eq3FF */
  (pToStruct)->m_uEnetVlan0Eq3FF =  (uint32_t)  ((pFromData)[17] >> 4) & 0x01;

  /* Unpack Member: m_uEnetVlan0EqZero */
  (pToStruct)->m_uEnetVlan0EqZero =  (uint32_t)  ((pFromData)[17] >> 3) & 0x01;

  /* Unpack Member: m_uSyncSPortLt1024 */
  (pToStruct)->m_uSyncSPortLt1024 =  (uint32_t)  ((pFromData)[17] >> 2) & 0x01;

  /* Unpack Member: m_uUnkMpls0Lbl */
  (pToStruct)->m_uUnkMpls0Lbl =  (uint32_t)  ((pFromData)[17] >> 1) & 0x01;

  /* Unpack Member: m_uUnkMpls1Lbl */
  (pToStruct)->m_uUnkMpls1Lbl =  (uint32_t)  ((pFromData)[17] ) & 0x01;

  /* Unpack Member: m_uUnkMpls2Lbl */
  (pToStruct)->m_uUnkMpls2Lbl =  (uint32_t)  ((pFromData)[16] >> 7) & 0x01;

  /* Unpack Member: m_uEnetVlan2EqZero */
  (pToStruct)->m_uEnetVlan2EqZero =  (uint32_t)  ((pFromData)[16] >> 6) & 0x01;

  /* Unpack Member: m_uEnetVlan2Eq3FF */
  (pToStruct)->m_uEnetVlan2Eq3FF =  (uint32_t)  ((pFromData)[16] >> 5) & 0x01;

  /* Unpack Member: m_uIpv6SaOrDaMatch */
  (pToStruct)->m_uIpv6SaOrDaMatch =  (uint32_t)  ((pFromData)[16] >> 4) & 0x01;

  /* Unpack Member: m_uIpv6SaOrDaLpbk */
  (pToStruct)->m_uIpv6SaOrDaLpbk =  (uint32_t)  ((pFromData)[16] >> 3) & 0x01;

  /* Unpack Member: m_uIpv4SaOrDaMatch */
  (pToStruct)->m_uIpv4SaOrDaMatch =  (uint32_t)  ((pFromData)[16] >> 2) & 0x01;

  /* Unpack Member: m_uIpv4SaOrDaMartian */
  (pToStruct)->m_uIpv4SaOrDaMartian =  (uint32_t)  ((pFromData)[16] >> 1) & 0x01;

  /* Unpack Member: m_uUdpSpEqDp */
  (pToStruct)->m_uUdpSpEqDp =  (uint32_t)  ((pFromData)[16] ) & 0x01;

  /* Unpack Member: m_uTcpTinyFrag */
  (pToStruct)->m_uTcpTinyFrag =  (uint32_t)  ((pFromData)[15] >> 7) & 0x01;

  /* Unpack Member: m_uTcpSynFin */
  (pToStruct)->m_uTcpSynFin =  (uint32_t)  ((pFromData)[15] >> 6) & 0x01;

  /* Unpack Member: m_uTcpXmasScan */
  (pToStruct)->m_uTcpXmasScan =  (uint32_t)  ((pFromData)[15] >> 5) & 0x01;

  /* Unpack Member: m_uTcpNullScan */
  (pToStruct)->m_uTcpNullScan =  (uint32_t)  ((pFromData)[15] >> 4) & 0x01;

  /* Unpack Member: m_uTcpSpEqDp */
  (pToStruct)->m_uTcpSpEqDp =  (uint32_t)  ((pFromData)[15] >> 3) & 0x01;

  /* Unpack Member: m_uEnetTypeValue */
  (pToStruct)->m_uEnetTypeValue =  (uint32_t)  ((pFromData)[15] >> 2) & 0x01;

  /* Unpack Member: m_uEnetMacEqZero */
  (pToStruct)->m_uEnetMacEqZero =  (uint32_t)  ((pFromData)[15] >> 1) & 0x01;

  /* Unpack Member: m_uInvPppAddrCtrl */
  (pToStruct)->m_uInvPppAddrCtrl =  (uint32_t)  ((pFromData)[15] ) & 0x01;

  /* Unpack Member: m_uInvPppPid */
  (pToStruct)->m_uInvPppPid =  (uint32_t)  ((pFromData)[14] >> 7) & 0x01;

  /* Unpack Member: m_uEnetVlan1EqZero */
  (pToStruct)->m_uEnetVlan1EqZero =  (uint32_t)  ((pFromData)[14] >> 6) & 0x01;

  /* Unpack Member: m_uEnetVlan1Eq3FF */
  (pToStruct)->m_uEnetVlan1Eq3FF =  (uint32_t)  ((pFromData)[14] >> 5) & 0x01;

  /* Unpack Member: m_uEnetSmacEqDmac */
  (pToStruct)->m_uEnetSmacEqDmac =  (uint32_t)  ((pFromData)[14] >> 4) & 0x01;

  /* Unpack Member: m_uInvGreRes0 */
  (pToStruct)->m_uInvGreRes0 =  (uint32_t)  ((pFromData)[14] >> 3) & 0x01;

  /* Unpack Member: m_uIpv4SaOrDaLpbk */
  (pToStruct)->m_uIpv4SaOrDaLpbk =  (uint32_t)  ((pFromData)[14] >> 2) & 0x01;

  /* Unpack Member: m_uIpv4RuntPkt */
  (pToStruct)->m_uIpv4RuntPkt =  (uint32_t)  ((pFromData)[14] >> 1) & 0x01;

  /* Unpack Member: m_uIpv4Options */
  (pToStruct)->m_uIpv4Options =  (uint32_t)  ((pFromData)[14] ) & 0x01;

  /* Unpack Member: m_uInvIpv4Checksum */
  (pToStruct)->m_uInvIpv4Checksum =  (uint32_t)  ((pFromData)[13] >> 7) & 0x01;

  /* Unpack Member: m_uInvIpv4Version */
  (pToStruct)->m_uInvIpv4Version =  (uint32_t)  ((pFromData)[13] >> 6) & 0x01;

  /* Unpack Member: m_uInvIpv4RuntHdr */
  (pToStruct)->m_uInvIpv4RuntHdr =  (uint32_t)  ((pFromData)[13] >> 5) & 0x01;

  /* Unpack Member: m_uIpv4LenError */
  (pToStruct)->m_uIpv4LenError =  (uint32_t)  ((pFromData)[13] >> 4) & 0x01;

  /* Unpack Member: m_uIpv4PktLenErr */
  (pToStruct)->m_uIpv4PktLenErr =  (uint32_t)  ((pFromData)[13] >> 3) & 0x01;

  /* Unpack Member: m_uIpv4InvSa */
  (pToStruct)->m_uIpv4InvSa =  (uint32_t)  ((pFromData)[13] >> 2) & 0x01;

  /* Unpack Member: m_uIpv4InvDa */
  (pToStruct)->m_uIpv4InvDa =  (uint32_t)  ((pFromData)[13] >> 1) & 0x01;

  /* Unpack Member: m_uIpv4SaEqDa */
  (pToStruct)->m_uIpv4SaEqDa =  (uint32_t)  ((pFromData)[13] ) & 0x01;

  /* Unpack Member: m_uIpv6RuntPkt */
  (pToStruct)->m_uIpv6RuntPkt =  (uint32_t)  ((pFromData)[12] >> 7) & 0x01;

  /* Unpack Member: m_uInvIpv6Ver */
  (pToStruct)->m_uInvIpv6Ver =  (uint32_t)  ((pFromData)[12] >> 6) & 0x01;

  /* Unpack Member: m_uIpv6PktLenErr */
  (pToStruct)->m_uIpv6PktLenErr =  (uint32_t)  ((pFromData)[12] >> 5) & 0x01;

  /* Unpack Member: m_uIpv6InvSa */
  (pToStruct)->m_uIpv6InvSa =  (uint32_t)  ((pFromData)[12] >> 4) & 0x01;

  /* Unpack Member: m_uIpv6InvDa */
  (pToStruct)->m_uIpv6InvDa =  (uint32_t)  ((pFromData)[12] >> 3) & 0x01;

  /* Unpack Member: m_uIpv6SaEqDa */
  (pToStruct)->m_uIpv6SaEqDa =  (uint32_t)  ((pFromData)[12] >> 2) & 0x01;

  /* Unpack Member: m_uQueueHash */
  (pToStruct)->m_uQueueHash =  (uint32_t)  ((pFromData)[8] >> 2) & 0x3f;
  (pToStruct)->m_uQueueHash |=  (uint32_t)  (pFromData)[9] << 6;
  (pToStruct)->m_uQueueHash |=  (uint32_t)  (pFromData)[10] << 14;
  (pToStruct)->m_uQueueHash |=  (uint32_t)  (pFromData)[11] << 22;
  (pToStruct)->m_uQueueHash |=  (uint32_t)  ((pFromData)[12] & 0x03) << 30;

  /* Unpack Member: m_uHeaderShift */
  (pToStruct)->m_uHeaderShift =  (uint32_t)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_uHeaderShift |=  (uint32_t)  ((pFromData)[8] & 0x03) << 4;

  /* Unpack Member: m_uHeaderType */
  (pToStruct)->m_uHeaderType =  (uint32_t)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_uState */
  (pToStruct)->m_uState =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uState |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uState |=  (uint32_t)  (pFromData)[6] << 16;

  /* Unpack Member: m_uVariable */
  (pToStruct)->m_uVariable =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uVariable |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uVariable |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uVariable |=  (uint32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfC2PpInitialQueueStateEntry_InitInstance(sbZfC2PpInitialQueueStateEntry_t *pFrame) {

  pFrame->m_uEnetSmacMcast =  (unsigned int)  0;
  pFrame->m_uEnetVlan0Eq3FF =  (unsigned int)  0;
  pFrame->m_uEnetVlan0EqZero =  (unsigned int)  0;
  pFrame->m_uSyncSPortLt1024 =  (unsigned int)  0;
  pFrame->m_uUnkMpls0Lbl =  (unsigned int)  0;
  pFrame->m_uUnkMpls1Lbl =  (unsigned int)  0;
  pFrame->m_uUnkMpls2Lbl =  (unsigned int)  0;
  pFrame->m_uEnetVlan2EqZero =  (unsigned int)  0;
  pFrame->m_uEnetVlan2Eq3FF =  (unsigned int)  0;
  pFrame->m_uIpv6SaOrDaMatch =  (unsigned int)  0;
  pFrame->m_uIpv6SaOrDaLpbk =  (unsigned int)  0;
  pFrame->m_uIpv4SaOrDaMatch =  (unsigned int)  0;
  pFrame->m_uIpv4SaOrDaMartian =  (unsigned int)  0;
  pFrame->m_uUdpSpEqDp =  (unsigned int)  0;
  pFrame->m_uTcpTinyFrag =  (unsigned int)  0;
  pFrame->m_uTcpSynFin =  (unsigned int)  0;
  pFrame->m_uTcpXmasScan =  (unsigned int)  0;
  pFrame->m_uTcpNullScan =  (unsigned int)  0;
  pFrame->m_uTcpSpEqDp =  (unsigned int)  0;
  pFrame->m_uEnetTypeValue =  (unsigned int)  0;
  pFrame->m_uEnetMacEqZero =  (unsigned int)  0;
  pFrame->m_uInvPppAddrCtrl =  (unsigned int)  0;
  pFrame->m_uInvPppPid =  (unsigned int)  0;
  pFrame->m_uEnetVlan1EqZero =  (unsigned int)  0;
  pFrame->m_uEnetVlan1Eq3FF =  (unsigned int)  0;
  pFrame->m_uEnetSmacEqDmac =  (unsigned int)  0;
  pFrame->m_uInvGreRes0 =  (unsigned int)  0;
  pFrame->m_uIpv4SaOrDaLpbk =  (unsigned int)  0;
  pFrame->m_uIpv4RuntPkt =  (unsigned int)  0;
  pFrame->m_uIpv4Options =  (unsigned int)  0;
  pFrame->m_uInvIpv4Checksum =  (unsigned int)  0;
  pFrame->m_uInvIpv4Version =  (unsigned int)  0;
  pFrame->m_uInvIpv4RuntHdr =  (unsigned int)  0;
  pFrame->m_uIpv4LenError =  (unsigned int)  0;
  pFrame->m_uIpv4PktLenErr =  (unsigned int)  0;
  pFrame->m_uIpv4InvSa =  (unsigned int)  0;
  pFrame->m_uIpv4InvDa =  (unsigned int)  0;
  pFrame->m_uIpv4SaEqDa =  (unsigned int)  0;
  pFrame->m_uIpv6RuntPkt =  (unsigned int)  0;
  pFrame->m_uInvIpv6Ver =  (unsigned int)  0;
  pFrame->m_uIpv6PktLenErr =  (unsigned int)  0;
  pFrame->m_uIpv6InvSa =  (unsigned int)  0;
  pFrame->m_uIpv6InvDa =  (unsigned int)  0;
  pFrame->m_uIpv6SaEqDa =  (unsigned int)  0;
  pFrame->m_uQueueHash =  (unsigned int)  0;
  pFrame->m_uHeaderShift =  (unsigned int)  0;
  pFrame->m_uHeaderType =  (unsigned int)  0;
  pFrame->m_uState =  (unsigned int)  0;
  pFrame->m_uVariable =  (unsigned int)  0;

}
