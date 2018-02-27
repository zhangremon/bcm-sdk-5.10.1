/*
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
 *$Id: sbFe2000Pp.h 1.2.224.1 Broadcom SDK $
 *-----------------------------------------------------------------------------*/
#ifndef _SB_FE_2000_PP_H_
#define _SB_FE_2000_PP_H_

#include <sal/types.h>

typedef enum sbFe2000PpExcType_e
{
  SB_FE2000_INV_PPP_ADDR_CTL          = 0x10, /**< PPP Invalid Address or Control */
  SB_FE2000_INV_PPP_PID               = 0x11, /**< PPP Invalid PID field */
  SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG1  = 0x15, /**< Ethernet VLAN= 0x3FF for Tag 1 */
  SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG2  = 0x16, /**< Ethernet VLAN= 0x3FF for Tag 2 */
  SB_FE2000_ENET_VLAN_EQUAL_3FF_TAG3  = 0x17, /**< Ethernet VLAN= 0x3FF for Tag 3 */
  SB_FE2000_ENET_SMAC_EQ_DMAC         = 0x18, /**< Ethernet SMAC equals DMAC */
  SB_FE2000_INV_GRE_RES0              = 0x19, /**< Invalid GRE Reserved0 field */
  SB_FE2000_UNK_MPLS_LBL_LABEL0       = 0x1A, /**< MPLS Label out of range for label 0 */
  SB_FE2000_UNK_MPLS_LBL_LABEL1       = 0x1B, /**< MPLS Label out of range for label 1 */
  SB_FE2000_UNK_MPLS_LBL_LABEL2       = 0x1C, /**< MPLS Label out of range for label 2 */
  SB_FE2000_ENET_SMAC_EQ_DMAC_ZERO    = 0x1D, /**< Ethernet SMAC or DMAC equal zero */
  SB_FE2000_ENET_TYPE_BETWEEN_1501_AND_1536 = 0x1E, /**< Ethernet Type value is
                                                        between [1501 and 1536] */
  SB_FE2000_ENET_SMAC_EQ_MULTICAST    = 0x1F, /**< SMAC is multicast */
  SB_FE2000_IPV4_RUNT_PKT             = 0x20, /**< Packet too small for an IPv4 packet */
  SB_FE2000_IPV4_OPTIONS              = 0x21, /**< IPv4 options detected */
  SB_FE2000_INV_IPV4_CHECKSUM         = 0x22, /**< Invalid IPv4 header checksum */
  SB_FE2000_INV_IPV4_VER              = 0x23, /**< Invalid IPv4 version */
  SB_FE2000_INV_IPV4_RUNT_HDR         = 0x24, /**< IPv4 header length less than 5 */
  SB_FE2000_INV_IPV4_LEN_ERR          = 0x25, /**< IPv4 Length must be greater than 20 bytes */
  SB_FE2000_INV_IPV4_PKT_LEN_ERR      = 0x26, /**< IPv4 Length must be smaller than
                                                  the packet length */
  SB_FE2000_INV_IPV4_SA               = 0x27, /**< Invalid IPv4 Source Address */
  SB_FE2000_INV_IPV4_DA               = 0x28, /**< Invalid IPv4 Destination Address */
  SB_FE2000_INV_IPV4_SA_EQ_DA         = 0x29, /**< IPv4 Source Address equals
                                                  Destination Address */
  SB_FE2000_INV_IPV4_SA_OR_DA_IS_LOOPBACK     = 0x2A, /**< IPv4 Source Address or
                                                          Destination Address is Loopback */
  SB_FE2000_INV_IPV4_SA_OR_DA_MARTIN_ADDRESS  = 0x2B, /**< IPv4 Martian address check */
  SB_FE2000_IPV4_USR_ADDR_0           = 0x30, /**< IPv4 Filter 0 */
  SB_FE2000_IPV4_USR_ADDR_1           = 0x31, /**< IPv4 Filter 1 */
  SB_FE2000_IPV4_USR_ADDR_2           = 0x32, /**< IPv4 Filter 2 */
  SB_FE2000_IPV4_USR_ADDR_3           = 0x33, /**< IPv4 Filter 3 */
  SB_FE2000_IPV4_USR_ADDR_4           = 0x34, /**< IPv4 Filter 4 */
  SB_FE2000_IPV4_USR_ADDR_5           = 0x35, /**< IPv4 Filter 5 */
  SB_FE2000_IPV4_USR_ADDR_6           = 0x36, /**< IPv4 Filter 6 */
  SB_FE2000_IPV4_USR_ADDR_7           = 0x37, /**< IPv4 Filter 7 */
  SB_FE2000_IPV4_FRAG_ICMP_PROTOCOL   = 0x38, /**< IPv4 ICMP packet is fragmented */
  SB_FE2000_IPV6_RUNT_PKT             = 0x40, /**< Packet too small for an IPv6 packet */
  SB_FE2000_INV_IPV6_VER              = 0x41, /**< Invalid IPv6 version */
  SB_FE2000_IPV6_PKT_LEN_ERR          = 0x42, /**< IPv6 Length must be smaller than
                                                  the packet length */
  SB_FE2000_INV_IPV6_SA               = 0x43, /**< Invalid IPv6 Source Address */
  SB_FE2000_INV_IPV6_DA               = 0x44, /**< Invalid IPv6 Destination Address */
  SB_FE2000_IPV6_SA_EQ_DA             = 0x45, /**< IPv6 Source Address equals
                                                  Destination Address */
  SB_FE2000_IPV6_SA_OR_DA_LOOPBACK    = 0x46, /**< IPv6 Source Address or Destination
                                                  Address is Loopback */
  SB_FE2000_IPV6_USR_ADDR_0           = 0x48, /**< IPv6 Filter 0 */
  SB_FE2000_IPV6_USR_ADDR_1           = 0x49, /**< IPv6 Filter 1 */
  SB_FE2000_IPV6_USR_ADDR_2           = 0x4A, /**< IPv6 Filter 2 */
  SB_FE2000_IPV6_USR_ADDR_3           = 0x4B, /**< IPv6 Filter 3 */
  SB_FE2000_TCP_OR_UDP_DP_EQUAL_SP    = 0x50, /**< TCP/UDP SP=DP */
  SB_FE2000_TCP_SQ_EQ_ZERO_AND_FLAG_ZERO         = 0x51, /**< TCP Sequence number is 0
                                                             and the Flag is 0 */
  SB_FE2000_TCP_SQ_EQ_ZERO_AND_FIN_URG_PSH_ZERO  = 0x52, /**< If the TCP sequence number is 0
                                                             and the FIN, URG & PSH is 0 */
  SB_FE2000_TCP_SYN_AND_FIN_BOTH_SET = 0x53, /**< If the TCP SYN and FIN bits are both set */
  SB_FE2000_L4_TINY_FRAG             = 0x54, /**< If FO=0 and PROTOCOL=TCP and TRANSPORTLEN
                                                 PP_XXXX.TFRAG_MIN or if FO=1 and PROTOCOL=TCP */
  SB_FE2000_L4_SYN_SPORT_LT_1024     = 0x55, /**< Protocol is TCP, SYN bit is set and SP < 1024 */
  SB_FE2000_MAX_HW_EXC
} sbFe2000PpExcType_t;

#endif /* _SB_FE_2000_PP_H_ */
