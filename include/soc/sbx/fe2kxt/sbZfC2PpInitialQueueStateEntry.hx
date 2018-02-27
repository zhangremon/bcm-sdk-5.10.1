/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PpInitialQueueStateEntry.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_C2PPINITIALQUEUESTATEENTRY_H
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_H

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SIZE_IN_BYTES 18
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SIZE 18
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETSMACMCAST_BITS "141:141"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETVLAN0EQ3FF_BITS "140:140"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETVLAN0EQZERO_BITS "139:139"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_USYNCSPORTLT1024_BITS "138:138"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UUNKMPLS0LBL_BITS "137:137"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UUNKMPLS1LBL_BITS "136:136"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UUNKMPLS2LBL_BITS "135:135"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETVLAN2EQZERO_BITS "134:134"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETVLAN2EQ3FF_BITS "133:133"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV6SAORDAMATCH_BITS "132:132"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV6SAORDALPBK_BITS "131:131"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4SAORDAMATCH_BITS "130:130"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4SAORDAMARTIAN_BITS "129:129"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UUDPSPEQDP_BITS "128:128"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UTCPTINYFRAG_BITS "127:127"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UTCPSYNFIN_BITS "126:126"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UTCPXMASSCAN_BITS "125:125"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UTCPNULLSCAN_BITS "124:124"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UTCPSPEQDP_BITS "123:123"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETTYPEVALUE_BITS "122:122"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETMACEQZERO_BITS "121:121"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UINVPPPADDRCTRL_BITS "120:120"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UINVPPPPID_BITS "119:119"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETVLAN1EQZERO_BITS "118:118"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETVLAN1EQ3FF_BITS "117:117"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UENETSMACEQDMAC_BITS "116:116"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UINVGRERES0_BITS "115:115"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4SAORDALPBK_BITS "114:114"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4RUNTPKT_BITS "113:113"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4OPTIONS_BITS "112:112"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UINVIPV4CHECKSUM_BITS "111:111"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UINVIPV4VERSION_BITS "110:110"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UINVIPV4RUNTHDR_BITS "109:109"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4LENERROR_BITS "108:108"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4PKTLENERR_BITS "107:107"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4INVSA_BITS "106:106"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4INVDA_BITS "105:105"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV4SAEQDA_BITS "104:104"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV6RUNTPKT_BITS "103:103"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UINVIPV6VER_BITS "102:102"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV6PKTLENERR_BITS "101:101"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV6INVSA_BITS "100:100"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV6INVDA_BITS "99:99"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UIPV6SAEQDA_BITS "98:98"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UQUEUEHASH_BITS "97:66"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UHEADERSHIFT_BITS "65:60"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UHEADERTYPE_BITS "59:56"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_USTATE_BITS "55:32"
#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_M_UVARIABLE_BITS "31:0"


typedef struct _sbZfC2PpInitialQueueStateEntry {
  uint32_t m_uEnetSmacMcast;
  uint32_t m_uEnetVlan0Eq3FF;
  uint32_t m_uEnetVlan0EqZero;
  uint32_t m_uSyncSPortLt1024;
  uint32_t m_uUnkMpls0Lbl;
  uint32_t m_uUnkMpls1Lbl;
  uint32_t m_uUnkMpls2Lbl;
  uint32_t m_uEnetVlan2EqZero;
  uint32_t m_uEnetVlan2Eq3FF;
  uint32_t m_uIpv6SaOrDaMatch;
  uint32_t m_uIpv6SaOrDaLpbk;
  uint32_t m_uIpv4SaOrDaMatch;
  uint32_t m_uIpv4SaOrDaMartian;
  uint32_t m_uUdpSpEqDp;
  uint32_t m_uTcpTinyFrag;
  uint32_t m_uTcpSynFin;
  uint32_t m_uTcpXmasScan;
  uint32_t m_uTcpNullScan;
  uint32_t m_uTcpSpEqDp;
  uint32_t m_uEnetTypeValue;
  uint32_t m_uEnetMacEqZero;
  uint32_t m_uInvPppAddrCtrl;
  uint32_t m_uInvPppPid;
  uint32_t m_uEnetVlan1EqZero;
  uint32_t m_uEnetVlan1Eq3FF;
  uint32_t m_uEnetSmacEqDmac;
  uint32_t m_uInvGreRes0;
  uint32_t m_uIpv4SaOrDaLpbk;
  uint32_t m_uIpv4RuntPkt;
  uint32_t m_uIpv4Options;
  uint32_t m_uInvIpv4Checksum;
  uint32_t m_uInvIpv4Version;
  uint32_t m_uInvIpv4RuntHdr;
  uint32_t m_uIpv4LenError;
  uint32_t m_uIpv4PktLenErr;
  uint32_t m_uIpv4InvSa;
  uint32_t m_uIpv4InvDa;
  uint32_t m_uIpv4SaEqDa;
  uint32_t m_uIpv6RuntPkt;
  uint32_t m_uInvIpv6Ver;
  uint32_t m_uIpv6PktLenErr;
  uint32_t m_uIpv6InvSa;
  uint32_t m_uIpv6InvDa;
  uint32_t m_uIpv6SaEqDa;
  uint32_t m_uQueueHash;
  uint32_t m_uHeaderShift;
  uint32_t m_uHeaderType;
  uint32_t m_uState;
  uint32_t m_uVariable;
} sbZfC2PpInitialQueueStateEntry_t;

uint32_t
sbZfC2PpInitialQueueStateEntry_Pack(sbZfC2PpInitialQueueStateEntry_t *pFrom,
                                    uint8_t *pToData,
                                    uint32_t nMaxToDataIndex);
void
sbZfC2PpInitialQueueStateEntry_Unpack(sbZfC2PpInitialQueueStateEntry_t *pToStruct,
                                      uint8_t *pFromData,
                                      uint32_t nMaxToDataIndex);
void
sbZfC2PpInitialQueueStateEntry_InitInstance(sbZfC2PpInitialQueueStateEntry_t *pFrame);

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETSMACMCAST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETVLAN0EQ3FF(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETVLAN0EQZERO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_SYNCSPORTLT1024(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_UNKMPLS0LBL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_UNKMPLS1LBL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_UNKMPLS2LBL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETVLAN2EQZERO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETVLAN2EQ3FF(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV6SAORDAMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV6SAORDALPBK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4SAORDAMATCH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4SAORDAMARTIAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_UDPSPEQDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_TCPTINYFRAG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_TCPSYNFIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_TCPXMASSCAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_TCPNULLSCAN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_TCPSPEQDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETTYPEVALUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETMACEQZERO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_INVPPPADDRCTRL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_INVPPPPID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETVLAN1EQZERO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETVLAN1EQ3FF(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_ENETSMACEQDMAC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_INVGRERES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4SAORDALPBK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4RUNTPKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4OPTIONS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_INVIPV4CHECKSUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_INVIPV4VERSION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_INVIPV4RUNTHDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4LENERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4PKTLENERR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4INVSA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4INVDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV4SAEQDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV6RUNTPKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_INVIPV6VER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV6PKTLENERR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV6INVSA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV6INVDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_IPV6SAEQDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_QUEUEHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_HEADERSHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_HEADERTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_STATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_SET_VARIABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETSMACMCAST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETVLAN0EQ3FF(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETVLAN0EQZERO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_SYNCSPORTLT1024(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_UNKMPLS0LBL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_UNKMPLS1LBL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[17]) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_UNKMPLS2LBL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETVLAN2EQZERO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETVLAN2EQ3FF(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV6SAORDAMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV6SAORDALPBK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4SAORDAMATCH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4SAORDAMARTIAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_UDPSPEQDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[16]) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_TCPTINYFRAG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_TCPSYNFIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_TCPXMASSCAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_TCPNULLSCAN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_TCPSPEQDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETTYPEVALUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETMACEQZERO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_INVPPPADDRCTRL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[15]) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_INVPPPPID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETVLAN1EQZERO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETVLAN1EQ3FF(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_ENETSMACEQDMAC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_INVGRERES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4SAORDALPBK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4RUNTPKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4OPTIONS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[14]) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_INVIPV4CHECKSUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_INVIPV4VERSION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_INVIPV4RUNTHDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4LENERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4PKTLENERR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4INVSA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4INVDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV4SAEQDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[13]) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV6RUNTPKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_INVIPV6VER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV6PKTLENERR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV6INVSA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV6INVDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_IPV6SAEQDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[12] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_QUEUEHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[8] >> 2) & 0x3f; \
           (nToData) |= (uint32_t) (pFromData)[9] << 6; \
           (nToData) |= (uint32_t) (pFromData)[10] << 14; \
           (nToData) |= (uint32_t) (pFromData)[11] << 22; \
           (nToData) |= (uint32_t) ((pFromData)[12] & 0x03) << 30; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_HEADERSHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_HEADERTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_STATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[4] ; \
           (nToData) |= (uint32_t) (pFromData)[5] << 8; \
           (nToData) |= (uint32_t) (pFromData)[6] << 16; \
          } while(0)

#define SB_ZF_C2PPINITIALQUEUESTATEENTRY_GET_VARIABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[0] ; \
           (nToData) |= (uint32_t) (pFromData)[1] << 8; \
           (nToData) |= (uint32_t) (pFromData)[2] << 16; \
           (nToData) |= (uint32_t) (pFromData)[3] << 24; \
          } while(0)

#endif
