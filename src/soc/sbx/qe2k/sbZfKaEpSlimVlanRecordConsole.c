/*
 * $Id: sbZfKaEpSlimVlanRecordConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEpSlimVlanRecordConsole.hx"



/* Print members in struct */
void
sbZfKaEpSlimVlanRecord_Print(sbZfKaEpSlimVlanRecord_t *pFromStruct) {
  SB_LOG("KaEpSlimVlanRecord:: vid0=0x%03x", (unsigned int)  pFromStruct->m_nVid0);
  SB_LOG(" vid1=0x%03x", (unsigned int)  pFromStruct->m_nVid1);
  SB_LOG(" offset=0x%01x", (unsigned int)  pFromStruct->m_nOffset);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: vlan_packet_counter=0x%08x", (unsigned int)  pFromStruct->m_nVlanPacketCounter);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: vlan_byte_counter=0x%09llx", (uint64_t)  pFromStruct->m_nnVlanByteCounter);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: drop_packet_counter=0x%08x", (unsigned int)  pFromStruct->m_nDropPacketCounter);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: drop_byte_counter=0x%09llx", (uint64_t)  pFromStruct->m_nnDropByteCounter);
  SB_LOG(" port_state_0=0x%01x", (unsigned int)  pFromStruct->m_nPortState_0);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_1=0x%01x", (unsigned int)  pFromStruct->m_nPortState_1);
  SB_LOG(" port_state_2=0x%01x", (unsigned int)  pFromStruct->m_nPortState_2);
  SB_LOG(" port_state_3=0x%01x", (unsigned int)  pFromStruct->m_nPortState_3);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_4=0x%01x", (unsigned int)  pFromStruct->m_nPortState_4);
  SB_LOG(" port_state_5=0x%01x", (unsigned int)  pFromStruct->m_nPortState_5);
  SB_LOG(" port_state_6=0x%01x", (unsigned int)  pFromStruct->m_nPortState_6);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_7=0x%01x", (unsigned int)  pFromStruct->m_nPortState_7);
  SB_LOG(" port_state_8=0x%01x", (unsigned int)  pFromStruct->m_nPortState_8);
  SB_LOG(" port_state_9=0x%01x", (unsigned int)  pFromStruct->m_nPortState_9);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_10=0x%01x", (unsigned int)  pFromStruct->m_nPortState_10);
  SB_LOG(" port_state_11=0x%01x", (unsigned int)  pFromStruct->m_nPortState_11);
  SB_LOG(" port_state_12=0x%01x", (unsigned int)  pFromStruct->m_nPortState_12);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_13=0x%01x", (unsigned int)  pFromStruct->m_nPortState_13);
  SB_LOG(" port_state_14=0x%01x", (unsigned int)  pFromStruct->m_nPortState_14);
  SB_LOG(" port_state_15=0x%01x", (unsigned int)  pFromStruct->m_nPortState_15);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_16=0x%01x", (unsigned int)  pFromStruct->m_nPortState_16);
  SB_LOG(" port_state_17=0x%01x", (unsigned int)  pFromStruct->m_nPortState_17);
  SB_LOG(" port_state_18=0x%01x", (unsigned int)  pFromStruct->m_nPortState_18);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_19=0x%01x", (unsigned int)  pFromStruct->m_nPortState_19);
  SB_LOG(" port_state_20=0x%01x", (unsigned int)  pFromStruct->m_nPortState_20);
  SB_LOG(" port_state_21=0x%01x", (unsigned int)  pFromStruct->m_nPortState_21);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_22=0x%01x", (unsigned int)  pFromStruct->m_nPortState_22);
  SB_LOG(" port_state_23=0x%01x", (unsigned int)  pFromStruct->m_nPortState_23);
  SB_LOG(" port_state_24=0x%01x", (unsigned int)  pFromStruct->m_nPortState_24);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_25=0x%01x", (unsigned int)  pFromStruct->m_nPortState_25);
  SB_LOG(" port_state_26=0x%01x", (unsigned int)  pFromStruct->m_nPortState_26);
  SB_LOG(" port_state_27=0x%01x", (unsigned int)  pFromStruct->m_nPortState_27);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_28=0x%01x", (unsigned int)  pFromStruct->m_nPortState_28);
  SB_LOG(" port_state_29=0x%01x", (unsigned int)  pFromStruct->m_nPortState_29);
  SB_LOG(" port_state_30=0x%01x", (unsigned int)  pFromStruct->m_nPortState_30);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_31=0x%01x", (unsigned int)  pFromStruct->m_nPortState_31);
  SB_LOG(" port_state_32=0x%01x", (unsigned int)  pFromStruct->m_nPortState_32);
  SB_LOG(" port_state_33=0x%01x", (unsigned int)  pFromStruct->m_nPortState_33);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_34=0x%01x", (unsigned int)  pFromStruct->m_nPortState_34);
  SB_LOG(" port_state_35=0x%01x", (unsigned int)  pFromStruct->m_nPortState_35);
  SB_LOG(" port_state_36=0x%01x", (unsigned int)  pFromStruct->m_nPortState_36);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_37=0x%01x", (unsigned int)  pFromStruct->m_nPortState_37);
  SB_LOG(" port_state_38=0x%01x", (unsigned int)  pFromStruct->m_nPortState_38);
  SB_LOG(" port_state_39=0x%01x", (unsigned int)  pFromStruct->m_nPortState_39);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_40=0x%01x", (unsigned int)  pFromStruct->m_nPortState_40);
  SB_LOG(" port_state_41=0x%01x", (unsigned int)  pFromStruct->m_nPortState_41);
  SB_LOG(" port_state_42=0x%01x", (unsigned int)  pFromStruct->m_nPortState_42);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_43=0x%01x", (unsigned int)  pFromStruct->m_nPortState_43);
  SB_LOG(" port_state_44=0x%01x", (unsigned int)  pFromStruct->m_nPortState_44);
  SB_LOG(" port_state_45=0x%01x", (unsigned int)  pFromStruct->m_nPortState_45);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_46=0x%01x", (unsigned int)  pFromStruct->m_nPortState_46);
  SB_LOG(" port_state_47=0x%01x", (unsigned int)  pFromStruct->m_nPortState_47);
  SB_LOG(" port_state_48=0x%01x", (unsigned int)  pFromStruct->m_nPortState_48);
  SB_LOG("\n");

  SB_LOG("KaEpSlimVlanRecord:: port_state_49=0x%01x", (unsigned int)  pFromStruct->m_nPortState_49);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEpSlimVlanRecord_SPrint(sbZfKaEpSlimVlanRecord_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: vid0=0x%03x", (unsigned int)  pFromStruct->m_nVid0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," vid1=0x%03x", (unsigned int)  pFromStruct->m_nVid1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," offset=0x%01x", (unsigned int)  pFromStruct->m_nOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: vlan_packet_counter=0x%08x", (unsigned int)  pFromStruct->m_nVlanPacketCounter);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: vlan_byte_counter=0x%09llx", (uint64_t)  pFromStruct->m_nnVlanByteCounter);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: drop_packet_counter=0x%08x", (unsigned int)  pFromStruct->m_nDropPacketCounter);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: drop_byte_counter=0x%09llx", (uint64_t)  pFromStruct->m_nnDropByteCounter);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_0=0x%01x", (unsigned int)  pFromStruct->m_nPortState_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_1=0x%01x", (unsigned int)  pFromStruct->m_nPortState_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_2=0x%01x", (unsigned int)  pFromStruct->m_nPortState_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_3=0x%01x", (unsigned int)  pFromStruct->m_nPortState_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_4=0x%01x", (unsigned int)  pFromStruct->m_nPortState_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_5=0x%01x", (unsigned int)  pFromStruct->m_nPortState_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_6=0x%01x", (unsigned int)  pFromStruct->m_nPortState_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_7=0x%01x", (unsigned int)  pFromStruct->m_nPortState_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_8=0x%01x", (unsigned int)  pFromStruct->m_nPortState_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_9=0x%01x", (unsigned int)  pFromStruct->m_nPortState_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_10=0x%01x", (unsigned int)  pFromStruct->m_nPortState_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_11=0x%01x", (unsigned int)  pFromStruct->m_nPortState_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_12=0x%01x", (unsigned int)  pFromStruct->m_nPortState_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_13=0x%01x", (unsigned int)  pFromStruct->m_nPortState_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_14=0x%01x", (unsigned int)  pFromStruct->m_nPortState_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_15=0x%01x", (unsigned int)  pFromStruct->m_nPortState_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_16=0x%01x", (unsigned int)  pFromStruct->m_nPortState_16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_17=0x%01x", (unsigned int)  pFromStruct->m_nPortState_17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_18=0x%01x", (unsigned int)  pFromStruct->m_nPortState_18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_19=0x%01x", (unsigned int)  pFromStruct->m_nPortState_19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_20=0x%01x", (unsigned int)  pFromStruct->m_nPortState_20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_21=0x%01x", (unsigned int)  pFromStruct->m_nPortState_21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_22=0x%01x", (unsigned int)  pFromStruct->m_nPortState_22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_23=0x%01x", (unsigned int)  pFromStruct->m_nPortState_23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_24=0x%01x", (unsigned int)  pFromStruct->m_nPortState_24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_25=0x%01x", (unsigned int)  pFromStruct->m_nPortState_25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_26=0x%01x", (unsigned int)  pFromStruct->m_nPortState_26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_27=0x%01x", (unsigned int)  pFromStruct->m_nPortState_27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_28=0x%01x", (unsigned int)  pFromStruct->m_nPortState_28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_29=0x%01x", (unsigned int)  pFromStruct->m_nPortState_29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_30=0x%01x", (unsigned int)  pFromStruct->m_nPortState_30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_31=0x%01x", (unsigned int)  pFromStruct->m_nPortState_31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_32=0x%01x", (unsigned int)  pFromStruct->m_nPortState_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_33=0x%01x", (unsigned int)  pFromStruct->m_nPortState_33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_34=0x%01x", (unsigned int)  pFromStruct->m_nPortState_34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_35=0x%01x", (unsigned int)  pFromStruct->m_nPortState_35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_36=0x%01x", (unsigned int)  pFromStruct->m_nPortState_36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_37=0x%01x", (unsigned int)  pFromStruct->m_nPortState_37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_38=0x%01x", (unsigned int)  pFromStruct->m_nPortState_38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_39=0x%01x", (unsigned int)  pFromStruct->m_nPortState_39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_40=0x%01x", (unsigned int)  pFromStruct->m_nPortState_40);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_41=0x%01x", (unsigned int)  pFromStruct->m_nPortState_41);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_42=0x%01x", (unsigned int)  pFromStruct->m_nPortState_42);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_43=0x%01x", (unsigned int)  pFromStruct->m_nPortState_43);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_44=0x%01x", (unsigned int)  pFromStruct->m_nPortState_44);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_45=0x%01x", (unsigned int)  pFromStruct->m_nPortState_45);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_46=0x%01x", (unsigned int)  pFromStruct->m_nPortState_46);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_47=0x%01x", (unsigned int)  pFromStruct->m_nPortState_47);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_48=0x%01x", (unsigned int)  pFromStruct->m_nPortState_48);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEpSlimVlanRecord:: port_state_49=0x%01x", (unsigned int)  pFromStruct->m_nPortState_49);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEpSlimVlanRecord_Validate(sbZfKaEpSlimVlanRecord_t *pZf) {

  if (pZf->m_nVid0 > 0xfff) return 0;
  if (pZf->m_nVid1 > 0xfff) return 0;
  if (pZf->m_nOffset > 0xf) return 0;
  if (pZf->m_nVlanPacketCounter > 0x1fffffff) return 0;
  if (pZf->m_nnVlanByteCounter > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_nDropPacketCounter > 0x1fffffff) return 0;
  if (pZf->m_nnDropByteCounter > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_nPortState_0 > 0x3) return 0;
  if (pZf->m_nPortState_1 > 0x3) return 0;
  if (pZf->m_nPortState_2 > 0x3) return 0;
  if (pZf->m_nPortState_3 > 0x3) return 0;
  if (pZf->m_nPortState_4 > 0x3) return 0;
  if (pZf->m_nPortState_5 > 0x3) return 0;
  if (pZf->m_nPortState_6 > 0x3) return 0;
  if (pZf->m_nPortState_7 > 0x3) return 0;
  if (pZf->m_nPortState_8 > 0x3) return 0;
  if (pZf->m_nPortState_9 > 0x3) return 0;
  if (pZf->m_nPortState_10 > 0x3) return 0;
  if (pZf->m_nPortState_11 > 0x3) return 0;
  if (pZf->m_nPortState_12 > 0x3) return 0;
  if (pZf->m_nPortState_13 > 0x3) return 0;
  if (pZf->m_nPortState_14 > 0x3) return 0;
  if (pZf->m_nPortState_15 > 0x3) return 0;
  if (pZf->m_nPortState_16 > 0x3) return 0;
  if (pZf->m_nPortState_17 > 0x3) return 0;
  if (pZf->m_nPortState_18 > 0x3) return 0;
  if (pZf->m_nPortState_19 > 0x3) return 0;
  if (pZf->m_nPortState_20 > 0x3) return 0;
  if (pZf->m_nPortState_21 > 0x3) return 0;
  if (pZf->m_nPortState_22 > 0x3) return 0;
  if (pZf->m_nPortState_23 > 0x3) return 0;
  if (pZf->m_nPortState_24 > 0x3) return 0;
  if (pZf->m_nPortState_25 > 0x3) return 0;
  if (pZf->m_nPortState_26 > 0x3) return 0;
  if (pZf->m_nPortState_27 > 0x3) return 0;
  if (pZf->m_nPortState_28 > 0x3) return 0;
  if (pZf->m_nPortState_29 > 0x3) return 0;
  if (pZf->m_nPortState_30 > 0x3) return 0;
  if (pZf->m_nPortState_31 > 0x3) return 0;
  if (pZf->m_nPortState_32 > 0x3) return 0;
  if (pZf->m_nPortState_33 > 0x3) return 0;
  if (pZf->m_nPortState_34 > 0x3) return 0;
  if (pZf->m_nPortState_35 > 0x3) return 0;
  if (pZf->m_nPortState_36 > 0x3) return 0;
  if (pZf->m_nPortState_37 > 0x3) return 0;
  if (pZf->m_nPortState_38 > 0x3) return 0;
  if (pZf->m_nPortState_39 > 0x3) return 0;
  if (pZf->m_nPortState_40 > 0x3) return 0;
  if (pZf->m_nPortState_41 > 0x3) return 0;
  if (pZf->m_nPortState_42 > 0x3) return 0;
  if (pZf->m_nPortState_43 > 0x3) return 0;
  if (pZf->m_nPortState_44 > 0x3) return 0;
  if (pZf->m_nPortState_45 > 0x3) return 0;
  if (pZf->m_nPortState_46 > 0x3) return 0;
  if (pZf->m_nPortState_47 > 0x3) return 0;
  if (pZf->m_nPortState_48 > 0x3) return 0;
  if (pZf->m_nPortState_49 > 0x3) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEpSlimVlanRecord_SetField(sbZfKaEpSlimVlanRecord_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nvid0") == 0) {
    s->m_nVid0 = value;
  } else if (SB_STRCMP(name, "m_nvid1") == 0) {
    s->m_nVid1 = value;
  } else if (SB_STRCMP(name, "m_noffset") == 0) {
    s->m_nOffset = value;
  } else if (SB_STRCMP(name, "m_nvlanpacketcounter") == 0) {
    s->m_nVlanPacketCounter = value;
  } else if (SB_STRCMP(name, "m_nnvlanbytecounter") == 0) {
    s->m_nnVlanByteCounter = value;
  } else if (SB_STRCMP(name, "m_ndroppacketcounter") == 0) {
    s->m_nDropPacketCounter = value;
  } else if (SB_STRCMP(name, "m_nndropbytecounter") == 0) {
    s->m_nnDropByteCounter = value;
  } else if (SB_STRCMP(name, "m_nportstate_0") == 0) {
    s->m_nPortState_0 = value;
  } else if (SB_STRCMP(name, "m_nportstate_1") == 0) {
    s->m_nPortState_1 = value;
  } else if (SB_STRCMP(name, "m_nportstate_2") == 0) {
    s->m_nPortState_2 = value;
  } else if (SB_STRCMP(name, "m_nportstate_3") == 0) {
    s->m_nPortState_3 = value;
  } else if (SB_STRCMP(name, "m_nportstate_4") == 0) {
    s->m_nPortState_4 = value;
  } else if (SB_STRCMP(name, "m_nportstate_5") == 0) {
    s->m_nPortState_5 = value;
  } else if (SB_STRCMP(name, "m_nportstate_6") == 0) {
    s->m_nPortState_6 = value;
  } else if (SB_STRCMP(name, "m_nportstate_7") == 0) {
    s->m_nPortState_7 = value;
  } else if (SB_STRCMP(name, "m_nportstate_8") == 0) {
    s->m_nPortState_8 = value;
  } else if (SB_STRCMP(name, "m_nportstate_9") == 0) {
    s->m_nPortState_9 = value;
  } else if (SB_STRCMP(name, "m_nportstate_10") == 0) {
    s->m_nPortState_10 = value;
  } else if (SB_STRCMP(name, "m_nportstate_11") == 0) {
    s->m_nPortState_11 = value;
  } else if (SB_STRCMP(name, "m_nportstate_12") == 0) {
    s->m_nPortState_12 = value;
  } else if (SB_STRCMP(name, "m_nportstate_13") == 0) {
    s->m_nPortState_13 = value;
  } else if (SB_STRCMP(name, "m_nportstate_14") == 0) {
    s->m_nPortState_14 = value;
  } else if (SB_STRCMP(name, "m_nportstate_15") == 0) {
    s->m_nPortState_15 = value;
  } else if (SB_STRCMP(name, "m_nportstate_16") == 0) {
    s->m_nPortState_16 = value;
  } else if (SB_STRCMP(name, "m_nportstate_17") == 0) {
    s->m_nPortState_17 = value;
  } else if (SB_STRCMP(name, "m_nportstate_18") == 0) {
    s->m_nPortState_18 = value;
  } else if (SB_STRCMP(name, "m_nportstate_19") == 0) {
    s->m_nPortState_19 = value;
  } else if (SB_STRCMP(name, "m_nportstate_20") == 0) {
    s->m_nPortState_20 = value;
  } else if (SB_STRCMP(name, "m_nportstate_21") == 0) {
    s->m_nPortState_21 = value;
  } else if (SB_STRCMP(name, "m_nportstate_22") == 0) {
    s->m_nPortState_22 = value;
  } else if (SB_STRCMP(name, "m_nportstate_23") == 0) {
    s->m_nPortState_23 = value;
  } else if (SB_STRCMP(name, "m_nportstate_24") == 0) {
    s->m_nPortState_24 = value;
  } else if (SB_STRCMP(name, "m_nportstate_25") == 0) {
    s->m_nPortState_25 = value;
  } else if (SB_STRCMP(name, "m_nportstate_26") == 0) {
    s->m_nPortState_26 = value;
  } else if (SB_STRCMP(name, "m_nportstate_27") == 0) {
    s->m_nPortState_27 = value;
  } else if (SB_STRCMP(name, "m_nportstate_28") == 0) {
    s->m_nPortState_28 = value;
  } else if (SB_STRCMP(name, "m_nportstate_29") == 0) {
    s->m_nPortState_29 = value;
  } else if (SB_STRCMP(name, "m_nportstate_30") == 0) {
    s->m_nPortState_30 = value;
  } else if (SB_STRCMP(name, "m_nportstate_31") == 0) {
    s->m_nPortState_31 = value;
  } else if (SB_STRCMP(name, "m_nportstate_32") == 0) {
    s->m_nPortState_32 = value;
  } else if (SB_STRCMP(name, "m_nportstate_33") == 0) {
    s->m_nPortState_33 = value;
  } else if (SB_STRCMP(name, "m_nportstate_34") == 0) {
    s->m_nPortState_34 = value;
  } else if (SB_STRCMP(name, "m_nportstate_35") == 0) {
    s->m_nPortState_35 = value;
  } else if (SB_STRCMP(name, "m_nportstate_36") == 0) {
    s->m_nPortState_36 = value;
  } else if (SB_STRCMP(name, "m_nportstate_37") == 0) {
    s->m_nPortState_37 = value;
  } else if (SB_STRCMP(name, "m_nportstate_38") == 0) {
    s->m_nPortState_38 = value;
  } else if (SB_STRCMP(name, "m_nportstate_39") == 0) {
    s->m_nPortState_39 = value;
  } else if (SB_STRCMP(name, "m_nportstate_40") == 0) {
    s->m_nPortState_40 = value;
  } else if (SB_STRCMP(name, "m_nportstate_41") == 0) {
    s->m_nPortState_41 = value;
  } else if (SB_STRCMP(name, "m_nportstate_42") == 0) {
    s->m_nPortState_42 = value;
  } else if (SB_STRCMP(name, "m_nportstate_43") == 0) {
    s->m_nPortState_43 = value;
  } else if (SB_STRCMP(name, "m_nportstate_44") == 0) {
    s->m_nPortState_44 = value;
  } else if (SB_STRCMP(name, "m_nportstate_45") == 0) {
    s->m_nPortState_45 = value;
  } else if (SB_STRCMP(name, "m_nportstate_46") == 0) {
    s->m_nPortState_46 = value;
  } else if (SB_STRCMP(name, "m_nportstate_47") == 0) {
    s->m_nPortState_47 = value;
  } else if (SB_STRCMP(name, "m_nportstate_48") == 0) {
    s->m_nPortState_48 = value;
  } else if (SB_STRCMP(name, "m_nportstate_49") == 0) {
    s->m_nPortState_49 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
