/*
 * $Id: sbZfFe2000PedDescriptorConsole.c 1.3.36.3 Broadcom SDK $
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
#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFe2000PedDescriptorConsole.hx"



/* Print members in struct */
void
sbZfFe2000PedDescriptor_Print(sbZfFe2000PedDescriptor_t *pFromStruct) {
  SB_LOG("Fe2000PedDescriptor:: cntbyte=0x%02x", (unsigned int)  pFromStruct->m_uContinueByte);
  SB_LOG(" hdrlen=0x%02x", (unsigned int)  pFromStruct->m_uHeaderLength);
  SB_LOG(" resv0=0x%04x", (unsigned int)  pFromStruct->m_uResv0);
  SB_LOG(" strsel=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: buffer=0x%04x", (unsigned int)  pFromStruct->m_uBuffer);
  SB_LOG(" excpidx=0x%02x", (unsigned int)  pFromStruct->m_uExceptionIndex);
  SB_LOG(" squeue=0x%02x", (unsigned int)  pFromStruct->m_uSourceQueue);
  SB_LOG(" resv1=0x%01x", (unsigned int)  pFromStruct->m_uResv1);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: mridx=0x%01x", (unsigned int)  pFromStruct->m_uMirrorIndex);
  SB_LOG(" cpycnt=0x%03x", (unsigned int)  pFromStruct->m_uCopyCount);
  SB_LOG(" crcmode=0x%01x", (unsigned int)  pFromStruct->m_uCrcMode);
  SB_LOG(" dbit=0x%01x", (unsigned int)  pFromStruct->m_uDropBit);
  SB_LOG(" enque=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: dque=0x%02x", (unsigned int)  pFromStruct->m_uDestinationQueue);
  SB_LOG(" xferlen=0x%04x", (unsigned int)  pFromStruct->m_uXferLength);
  SB_LOG(" packmark=0x%01x", (unsigned int)  pFromStruct->m_uPacketMarking);
  SB_LOG(" resv3=0x%01x", (unsigned int)  pFromStruct->m_uResv3);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: srcfrmlen=0x%04x", (unsigned int)  pFromStruct->m_uSourceFrameLength);
  SB_LOG(" pdata0=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte0);
  SB_LOG(" pdata1=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte1);
  SB_LOG(" pdata2=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte2);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata3=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte3);
  SB_LOG(" pdata4=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte4);
  SB_LOG(" pdata5=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte5);
  SB_LOG(" pdata6=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte6);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata7=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte7);
  SB_LOG(" pdata8=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte8);
  SB_LOG(" pdata9=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte9);
  SB_LOG(" pdata10=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte10);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata11=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte11);
  SB_LOG(" pdata12=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte12);
  SB_LOG(" pdata13=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte13);
  SB_LOG(" pdata14=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte14);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata15=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte15);
  SB_LOG(" pdata16=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte16);
  SB_LOG(" pdata17=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte17);
  SB_LOG(" pdata18=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte18);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata19=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte19);
  SB_LOG(" pdata20=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte20);
  SB_LOG(" pdata21=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte21);
  SB_LOG(" pdata22=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte22);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata23=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte23);
  SB_LOG(" pdata24=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte24);
  SB_LOG(" pdata25=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte25);
  SB_LOG(" pdata26=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte26);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata27=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte27);
  SB_LOG(" pdata28=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte28);
  SB_LOG(" pdata29=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte29);
  SB_LOG(" pdata30=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte30);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata31=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte31);
  SB_LOG(" pdata32=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte32);
  SB_LOG(" pdata33=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte33);
  SB_LOG(" pdata34=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte34);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata35=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte35);
  SB_LOG(" pdata36=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte36);
  SB_LOG(" pdata37=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte37);
  SB_LOG(" pdata38=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte38);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata39=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte39);
  SB_LOG(" pdata40=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte40);
  SB_LOG(" pdata41=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte41);
  SB_LOG(" pdata42=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte42);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata43=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte43);
  SB_LOG(" pdata44=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte44);
  SB_LOG(" pdata45=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte45);
  SB_LOG(" pdata46=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte46);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata47=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte47);
  SB_LOG(" pdata48=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte48);
  SB_LOG(" pdata49=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte49);
  SB_LOG(" pdata50=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte50);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata51=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte51);
  SB_LOG(" pdata52=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte52);
  SB_LOG(" pdata53=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte53);
  SB_LOG(" pdata54=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte54);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata55=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte55);
  SB_LOG(" pdata56=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte56);
  SB_LOG(" pdata57=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte57);
  SB_LOG(" pdata58=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte58);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata59=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte59);
  SB_LOG(" pdata60=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte60);
  SB_LOG(" pdata61=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte61);
  SB_LOG(" pdata62=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte62);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata63=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte63);
  SB_LOG(" pdata64=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte64);
  SB_LOG(" pdata65=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte65);
  SB_LOG(" pdata66=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte66);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata67=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte67);
  SB_LOG(" pdata68=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte68);
  SB_LOG(" pdata69=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte69);
  SB_LOG(" pdata70=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte70);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata71=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte71);
  SB_LOG(" pdata72=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte72);
  SB_LOG(" pdata73=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte73);
  SB_LOG(" pdata74=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte74);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata75=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte75);
  SB_LOG(" pdata76=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte76);
  SB_LOG(" pdata77=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte77);
  SB_LOG(" pdata78=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte78);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata79=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte79);
  SB_LOG(" pdata80=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte80);
  SB_LOG(" pdata81=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte81);
  SB_LOG(" pdata82=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte82);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata83=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte83);
  SB_LOG(" pdata84=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte84);
  SB_LOG(" pdata85=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte85);
  SB_LOG(" pdata86=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte86);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata87=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte87);
  SB_LOG(" pdata88=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte88);
  SB_LOG(" pdata89=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte89);
  SB_LOG(" pdata90=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte90);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata91=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte91);
  SB_LOG(" pdata92=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte92);
  SB_LOG(" pdata93=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte93);
  SB_LOG(" pdata94=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte94);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata95=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte95);
  SB_LOG(" pdata96=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte96);
  SB_LOG(" pdata97=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte97);
  SB_LOG(" pdata98=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte98);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata99=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte99);
  SB_LOG(" pdata100=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte100);
  SB_LOG(" pdata101=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte101);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata102=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte102);
  SB_LOG(" pdata103=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte103);
  SB_LOG(" pdata104=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte104);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata105=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte105);
  SB_LOG(" pdata106=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte106);
  SB_LOG(" pdata107=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte107);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata108=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte108);
  SB_LOG(" pdata109=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte109);
  SB_LOG(" pdata110=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte110);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata111=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte111);
  SB_LOG(" pdata112=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte112);
  SB_LOG(" pdata113=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte113);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata114=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte114);
  SB_LOG(" pdata115=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte115);
  SB_LOG(" pdata116=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte116);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata117=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte117);
  SB_LOG(" pdata118=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte118);
  SB_LOG(" pdata119=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte119);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata120=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte120);
  SB_LOG(" pdata121=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte121);
  SB_LOG(" pdata122=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte122);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata123=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte123);
  SB_LOG(" pdata124=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte124);
  SB_LOG(" pdata125=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte125);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata126=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte126);
  SB_LOG(" pdata127=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte127);
  SB_LOG(" pdata128=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte128);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata129=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte129);
  SB_LOG(" pdata130=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte130);
  SB_LOG(" pdata131=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte131);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata132=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte132);
  SB_LOG(" pdata133=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte133);
  SB_LOG(" pdata134=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte134);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata135=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte135);
  SB_LOG(" pdata136=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte136);
  SB_LOG(" pdata137=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte137);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata138=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte138);
  SB_LOG(" pdata139=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte139);
  SB_LOG(" pdata140=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte140);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata141=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte141);
  SB_LOG(" pdata142=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte142);
  SB_LOG(" pdata143=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte143);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata144=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte144);
  SB_LOG(" pdata145=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte145);
  SB_LOG(" pdata146=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte146);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata147=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte147);
  SB_LOG(" pdata148=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte148);
  SB_LOG(" pdata149=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte149);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata150=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte150);
  SB_LOG(" pdata151=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte151);
  SB_LOG(" pdata152=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte152);
  SB_LOG("\n");

  SB_LOG("Fe2000PedDescriptor:: pdata153=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte153);
  SB_LOG(" pdata154=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte154);
  SB_LOG(" pdata155=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte155);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFe2000PedDescriptor_SPrint(sbZfFe2000PedDescriptor_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: cntbyte=0x%02x", (unsigned int)  pFromStruct->m_uContinueByte);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hdrlen=0x%02x", (unsigned int)  pFromStruct->m_uHeaderLength);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv0=0x%04x", (unsigned int)  pFromStruct->m_uResv0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," strsel=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: buffer=0x%04x", (unsigned int)  pFromStruct->m_uBuffer);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," excpidx=0x%02x", (unsigned int)  pFromStruct->m_uExceptionIndex);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," squeue=0x%02x", (unsigned int)  pFromStruct->m_uSourceQueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv1=0x%01x", (unsigned int)  pFromStruct->m_uResv1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: mridx=0x%01x", (unsigned int)  pFromStruct->m_uMirrorIndex);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cpycnt=0x%03x", (unsigned int)  pFromStruct->m_uCopyCount);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," crcmode=0x%01x", (unsigned int)  pFromStruct->m_uCrcMode);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dbit=0x%01x", (unsigned int)  pFromStruct->m_uDropBit);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enque=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: dque=0x%02x", (unsigned int)  pFromStruct->m_uDestinationQueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," xferlen=0x%04x", (unsigned int)  pFromStruct->m_uXferLength);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," packmark=0x%01x", (unsigned int)  pFromStruct->m_uPacketMarking);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv3=0x%01x", (unsigned int)  pFromStruct->m_uResv3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: srcfrmlen=0x%04x", (unsigned int)  pFromStruct->m_uSourceFrameLength);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata0=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata1=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata2=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata3=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata4=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata5=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata6=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata7=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata8=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata9=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata10=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata11=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata12=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata13=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata14=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata15=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata16=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata17=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata18=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata19=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata20=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata21=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata22=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata23=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata24=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata25=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata26=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata27=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata28=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata29=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata30=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata31=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata32=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata33=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata34=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata35=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata36=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata37=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata38=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata39=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata40=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte40);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata41=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte41);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata42=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte42);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata43=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte43);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata44=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte44);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata45=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte45);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata46=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte46);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata47=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte47);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata48=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte48);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata49=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte49);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata50=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte50);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata51=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte51);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata52=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte52);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata53=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte53);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata54=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte54);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata55=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte55);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata56=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte56);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata57=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte57);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata58=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte58);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata59=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte59);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata60=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte60);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata61=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte61);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata62=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte62);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata63=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte63);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata64=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata65=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte65);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata66=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte66);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata67=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte67);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata68=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte68);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata69=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte69);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata70=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte70);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata71=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte71);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata72=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte72);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata73=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte73);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata74=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte74);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata75=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte75);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata76=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte76);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata77=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte77);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata78=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte78);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata79=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte79);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata80=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte80);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata81=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte81);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata82=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte82);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata83=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte83);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata84=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte84);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata85=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte85);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata86=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte86);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata87=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte87);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata88=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte88);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata89=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte89);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata90=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte90);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata91=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte91);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata92=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte92);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata93=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte93);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata94=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte94);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata95=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte95);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata96=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata97=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte97);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata98=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte98);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata99=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte99);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata100=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte100);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata101=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte101);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata102=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte102);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata103=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte103);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata104=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte104);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata105=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte105);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata106=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte106);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata107=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte107);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata108=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte108);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata109=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte109);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata110=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte110);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata111=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte111);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata112=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte112);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata113=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte113);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata114=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte114);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata115=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte115);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata116=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte116);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata117=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte117);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata118=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte118);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata119=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte119);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata120=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte120);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata121=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte121);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata122=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte122);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata123=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte123);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata124=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte124);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata125=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte125);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata126=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte126);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata127=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte127);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata128=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte128);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata129=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte129);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata130=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte130);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata131=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte131);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata132=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte132);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata133=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte133);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata134=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte134);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata135=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte135);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata136=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte136);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata137=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte137);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata138=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte138);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata139=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte139);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata140=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte140);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata141=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte141);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata142=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte142);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata143=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte143);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata144=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte144);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata145=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte145);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata146=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte146);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata147=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte147);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata148=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte148);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata149=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte149);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata150=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte150);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata151=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte151);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata152=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte152);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000PedDescriptor:: pdata153=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte153);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata154=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte154);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pdata155=0x%02x", (unsigned int)  pFromStruct->m_uPacketDataByte155);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFe2000PedDescriptor_Validate(sbZfFe2000PedDescriptor_t *pZf) {

  if (pZf->m_uContinueByte > 0xff) return 0;
  if (pZf->m_uHeaderLength > 0xff) return 0;
  if (pZf->m_uResv0 > 0xffff) return 0;
  if (pZf->m_uStreamSelector > 0x7) return 0;
  if (pZf->m_uBuffer > 0x1fff) return 0;
  if (pZf->m_uExceptionIndex > 0xff) return 0;
  if (pZf->m_uSourceQueue > 0xff) return 0;
  if (pZf->m_uResv1 > 0xf) return 0;
  if (pZf->m_uMirrorIndex > 0xf) return 0;
  if (pZf->m_uCopyCount > 0xfff) return 0;
  if (pZf->m_uCrcMode > 0x3) return 0;
  if (pZf->m_uDropBit > 0x1) return 0;
  if (pZf->m_uEnqueue > 0x1) return 0;
  if (pZf->m_uDestinationQueue > 0xff) return 0;
  if (pZf->m_uXferLength > 0x3fff) return 0;
  if (pZf->m_uPacketMarking > 0x3) return 0;
  if (pZf->m_uResv3 > 0x3) return 0;
  if (pZf->m_uSourceFrameLength > 0x7fff) return 0;
  /* pZf->m_uPacketDataByte0 implicitly masked by data type */
  /* pZf->m_uPacketDataByte1 implicitly masked by data type */
  /* pZf->m_uPacketDataByte2 implicitly masked by data type */
  /* pZf->m_uPacketDataByte3 implicitly masked by data type */
  /* pZf->m_uPacketDataByte4 implicitly masked by data type */
  /* pZf->m_uPacketDataByte5 implicitly masked by data type */
  /* pZf->m_uPacketDataByte6 implicitly masked by data type */
  /* pZf->m_uPacketDataByte7 implicitly masked by data type */
  /* pZf->m_uPacketDataByte8 implicitly masked by data type */
  /* pZf->m_uPacketDataByte9 implicitly masked by data type */
  /* pZf->m_uPacketDataByte10 implicitly masked by data type */
  /* pZf->m_uPacketDataByte11 implicitly masked by data type */
  /* pZf->m_uPacketDataByte12 implicitly masked by data type */
  /* pZf->m_uPacketDataByte13 implicitly masked by data type */
  /* pZf->m_uPacketDataByte14 implicitly masked by data type */
  /* pZf->m_uPacketDataByte15 implicitly masked by data type */
  /* pZf->m_uPacketDataByte16 implicitly masked by data type */
  /* pZf->m_uPacketDataByte17 implicitly masked by data type */
  /* pZf->m_uPacketDataByte18 implicitly masked by data type */
  /* pZf->m_uPacketDataByte19 implicitly masked by data type */
  /* pZf->m_uPacketDataByte20 implicitly masked by data type */
  /* pZf->m_uPacketDataByte21 implicitly masked by data type */
  /* pZf->m_uPacketDataByte22 implicitly masked by data type */
  /* pZf->m_uPacketDataByte23 implicitly masked by data type */
  /* pZf->m_uPacketDataByte24 implicitly masked by data type */
  /* pZf->m_uPacketDataByte25 implicitly masked by data type */
  /* pZf->m_uPacketDataByte26 implicitly masked by data type */
  /* pZf->m_uPacketDataByte27 implicitly masked by data type */
  /* pZf->m_uPacketDataByte28 implicitly masked by data type */
  /* pZf->m_uPacketDataByte29 implicitly masked by data type */
  /* pZf->m_uPacketDataByte30 implicitly masked by data type */
  /* pZf->m_uPacketDataByte31 implicitly masked by data type */
  /* pZf->m_uPacketDataByte32 implicitly masked by data type */
  /* pZf->m_uPacketDataByte33 implicitly masked by data type */
  /* pZf->m_uPacketDataByte34 implicitly masked by data type */
  /* pZf->m_uPacketDataByte35 implicitly masked by data type */
  /* pZf->m_uPacketDataByte36 implicitly masked by data type */
  /* pZf->m_uPacketDataByte37 implicitly masked by data type */
  /* pZf->m_uPacketDataByte38 implicitly masked by data type */
  /* pZf->m_uPacketDataByte39 implicitly masked by data type */
  /* pZf->m_uPacketDataByte40 implicitly masked by data type */
  /* pZf->m_uPacketDataByte41 implicitly masked by data type */
  /* pZf->m_uPacketDataByte42 implicitly masked by data type */
  /* pZf->m_uPacketDataByte43 implicitly masked by data type */
  /* pZf->m_uPacketDataByte44 implicitly masked by data type */
  /* pZf->m_uPacketDataByte45 implicitly masked by data type */
  /* pZf->m_uPacketDataByte46 implicitly masked by data type */
  /* pZf->m_uPacketDataByte47 implicitly masked by data type */
  /* pZf->m_uPacketDataByte48 implicitly masked by data type */
  /* pZf->m_uPacketDataByte49 implicitly masked by data type */
  /* pZf->m_uPacketDataByte50 implicitly masked by data type */
  /* pZf->m_uPacketDataByte51 implicitly masked by data type */
  /* pZf->m_uPacketDataByte52 implicitly masked by data type */
  /* pZf->m_uPacketDataByte53 implicitly masked by data type */
  /* pZf->m_uPacketDataByte54 implicitly masked by data type */
  /* pZf->m_uPacketDataByte55 implicitly masked by data type */
  /* pZf->m_uPacketDataByte56 implicitly masked by data type */
  /* pZf->m_uPacketDataByte57 implicitly masked by data type */
  /* pZf->m_uPacketDataByte58 implicitly masked by data type */
  /* pZf->m_uPacketDataByte59 implicitly masked by data type */
  /* pZf->m_uPacketDataByte60 implicitly masked by data type */
  /* pZf->m_uPacketDataByte61 implicitly masked by data type */
  /* pZf->m_uPacketDataByte62 implicitly masked by data type */
  /* pZf->m_uPacketDataByte63 implicitly masked by data type */
  /* pZf->m_uPacketDataByte64 implicitly masked by data type */
  /* pZf->m_uPacketDataByte65 implicitly masked by data type */
  /* pZf->m_uPacketDataByte66 implicitly masked by data type */
  /* pZf->m_uPacketDataByte67 implicitly masked by data type */
  /* pZf->m_uPacketDataByte68 implicitly masked by data type */
  /* pZf->m_uPacketDataByte69 implicitly masked by data type */
  /* pZf->m_uPacketDataByte70 implicitly masked by data type */
  /* pZf->m_uPacketDataByte71 implicitly masked by data type */
  /* pZf->m_uPacketDataByte72 implicitly masked by data type */
  /* pZf->m_uPacketDataByte73 implicitly masked by data type */
  /* pZf->m_uPacketDataByte74 implicitly masked by data type */
  /* pZf->m_uPacketDataByte75 implicitly masked by data type */
  /* pZf->m_uPacketDataByte76 implicitly masked by data type */
  /* pZf->m_uPacketDataByte77 implicitly masked by data type */
  /* pZf->m_uPacketDataByte78 implicitly masked by data type */
  /* pZf->m_uPacketDataByte79 implicitly masked by data type */
  /* pZf->m_uPacketDataByte80 implicitly masked by data type */
  /* pZf->m_uPacketDataByte81 implicitly masked by data type */
  /* pZf->m_uPacketDataByte82 implicitly masked by data type */
  /* pZf->m_uPacketDataByte83 implicitly masked by data type */
  /* pZf->m_uPacketDataByte84 implicitly masked by data type */
  /* pZf->m_uPacketDataByte85 implicitly masked by data type */
  /* pZf->m_uPacketDataByte86 implicitly masked by data type */
  /* pZf->m_uPacketDataByte87 implicitly masked by data type */
  /* pZf->m_uPacketDataByte88 implicitly masked by data type */
  /* pZf->m_uPacketDataByte89 implicitly masked by data type */
  /* pZf->m_uPacketDataByte90 implicitly masked by data type */
  /* pZf->m_uPacketDataByte91 implicitly masked by data type */
  /* pZf->m_uPacketDataByte92 implicitly masked by data type */
  /* pZf->m_uPacketDataByte93 implicitly masked by data type */
  /* pZf->m_uPacketDataByte94 implicitly masked by data type */
  /* pZf->m_uPacketDataByte95 implicitly masked by data type */
  /* pZf->m_uPacketDataByte96 implicitly masked by data type */
  /* pZf->m_uPacketDataByte97 implicitly masked by data type */
  /* pZf->m_uPacketDataByte98 implicitly masked by data type */
  /* pZf->m_uPacketDataByte99 implicitly masked by data type */
  /* pZf->m_uPacketDataByte100 implicitly masked by data type */
  /* pZf->m_uPacketDataByte101 implicitly masked by data type */
  /* pZf->m_uPacketDataByte102 implicitly masked by data type */
  /* pZf->m_uPacketDataByte103 implicitly masked by data type */
  /* pZf->m_uPacketDataByte104 implicitly masked by data type */
  /* pZf->m_uPacketDataByte105 implicitly masked by data type */
  /* pZf->m_uPacketDataByte106 implicitly masked by data type */
  /* pZf->m_uPacketDataByte107 implicitly masked by data type */
  /* pZf->m_uPacketDataByte108 implicitly masked by data type */
  /* pZf->m_uPacketDataByte109 implicitly masked by data type */
  /* pZf->m_uPacketDataByte110 implicitly masked by data type */
  /* pZf->m_uPacketDataByte111 implicitly masked by data type */
  /* pZf->m_uPacketDataByte112 implicitly masked by data type */
  /* pZf->m_uPacketDataByte113 implicitly masked by data type */
  /* pZf->m_uPacketDataByte114 implicitly masked by data type */
  /* pZf->m_uPacketDataByte115 implicitly masked by data type */
  /* pZf->m_uPacketDataByte116 implicitly masked by data type */
  /* pZf->m_uPacketDataByte117 implicitly masked by data type */
  /* pZf->m_uPacketDataByte118 implicitly masked by data type */
  /* pZf->m_uPacketDataByte119 implicitly masked by data type */
  /* pZf->m_uPacketDataByte120 implicitly masked by data type */
  /* pZf->m_uPacketDataByte121 implicitly masked by data type */
  /* pZf->m_uPacketDataByte122 implicitly masked by data type */
  /* pZf->m_uPacketDataByte123 implicitly masked by data type */
  /* pZf->m_uPacketDataByte124 implicitly masked by data type */
  /* pZf->m_uPacketDataByte125 implicitly masked by data type */
  /* pZf->m_uPacketDataByte126 implicitly masked by data type */
  /* pZf->m_uPacketDataByte127 implicitly masked by data type */
  /* pZf->m_uPacketDataByte128 implicitly masked by data type */
  /* pZf->m_uPacketDataByte129 implicitly masked by data type */
  /* pZf->m_uPacketDataByte130 implicitly masked by data type */
  /* pZf->m_uPacketDataByte131 implicitly masked by data type */
  /* pZf->m_uPacketDataByte132 implicitly masked by data type */
  /* pZf->m_uPacketDataByte133 implicitly masked by data type */
  /* pZf->m_uPacketDataByte134 implicitly masked by data type */
  /* pZf->m_uPacketDataByte135 implicitly masked by data type */
  /* pZf->m_uPacketDataByte136 implicitly masked by data type */
  /* pZf->m_uPacketDataByte137 implicitly masked by data type */
  /* pZf->m_uPacketDataByte138 implicitly masked by data type */
  /* pZf->m_uPacketDataByte139 implicitly masked by data type */
  /* pZf->m_uPacketDataByte140 implicitly masked by data type */
  /* pZf->m_uPacketDataByte141 implicitly masked by data type */
  /* pZf->m_uPacketDataByte142 implicitly masked by data type */
  /* pZf->m_uPacketDataByte143 implicitly masked by data type */
  /* pZf->m_uPacketDataByte144 implicitly masked by data type */
  /* pZf->m_uPacketDataByte145 implicitly masked by data type */
  /* pZf->m_uPacketDataByte146 implicitly masked by data type */
  /* pZf->m_uPacketDataByte147 implicitly masked by data type */
  /* pZf->m_uPacketDataByte148 implicitly masked by data type */
  /* pZf->m_uPacketDataByte149 implicitly masked by data type */
  /* pZf->m_uPacketDataByte150 implicitly masked by data type */
  /* pZf->m_uPacketDataByte151 implicitly masked by data type */
  /* pZf->m_uPacketDataByte152 implicitly masked by data type */
  /* pZf->m_uPacketDataByte153 implicitly masked by data type */
  /* pZf->m_uPacketDataByte154 implicitly masked by data type */
  /* pZf->m_uPacketDataByte155 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFe2000PedDescriptor_SetField(sbZfFe2000PedDescriptor_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ucontinuebyte") == 0) {
    s->m_uContinueByte = value;
  } else if (SB_STRCMP(name, "m_uheaderlength") == 0) {
    s->m_uHeaderLength = value;
  } else if (SB_STRCMP(name, "m_uresv0") == 0) {
    s->m_uResv0 = value;
  } else if (SB_STRCMP(name, "m_ustreamselector") == 0) {
    s->m_uStreamSelector = value;
  } else if (SB_STRCMP(name, "m_ubuffer") == 0) {
    s->m_uBuffer = value;
  } else if (SB_STRCMP(name, "m_uexceptionindex") == 0) {
    s->m_uExceptionIndex = value;
  } else if (SB_STRCMP(name, "m_usourcequeue") == 0) {
    s->m_uSourceQueue = value;
  } else if (SB_STRCMP(name, "m_uresv1") == 0) {
    s->m_uResv1 = value;
  } else if (SB_STRCMP(name, "m_umirrorindex") == 0) {
    s->m_uMirrorIndex = value;
  } else if (SB_STRCMP(name, "m_ucopycount") == 0) {
    s->m_uCopyCount = value;
  } else if (SB_STRCMP(name, "m_ucrcmode") == 0) {
    s->m_uCrcMode = value;
  } else if (SB_STRCMP(name, "m_udropbit") == 0) {
    s->m_uDropBit = value;
  } else if (SB_STRCMP(name, "m_uenqueue") == 0) {
    s->m_uEnqueue = value;
  } else if (SB_STRCMP(name, "m_udestinationqueue") == 0) {
    s->m_uDestinationQueue = value;
  } else if (SB_STRCMP(name, "m_uxferlength") == 0) {
    s->m_uXferLength = value;
  } else if (SB_STRCMP(name, "m_upacketmarking") == 0) {
    s->m_uPacketMarking = value;
  } else if (SB_STRCMP(name, "m_uresv3") == 0) {
    s->m_uResv3 = value;
  } else if (SB_STRCMP(name, "m_usourceframelength") == 0) {
    s->m_uSourceFrameLength = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte0") == 0) {
    s->m_uPacketDataByte0 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte1") == 0) {
    s->m_uPacketDataByte1 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte2") == 0) {
    s->m_uPacketDataByte2 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte3") == 0) {
    s->m_uPacketDataByte3 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte4") == 0) {
    s->m_uPacketDataByte4 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte5") == 0) {
    s->m_uPacketDataByte5 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte6") == 0) {
    s->m_uPacketDataByte6 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte7") == 0) {
    s->m_uPacketDataByte7 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte8") == 0) {
    s->m_uPacketDataByte8 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte9") == 0) {
    s->m_uPacketDataByte9 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte10") == 0) {
    s->m_uPacketDataByte10 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte11") == 0) {
    s->m_uPacketDataByte11 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte12") == 0) {
    s->m_uPacketDataByte12 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte13") == 0) {
    s->m_uPacketDataByte13 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte14") == 0) {
    s->m_uPacketDataByte14 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte15") == 0) {
    s->m_uPacketDataByte15 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte16") == 0) {
    s->m_uPacketDataByte16 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte17") == 0) {
    s->m_uPacketDataByte17 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte18") == 0) {
    s->m_uPacketDataByte18 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte19") == 0) {
    s->m_uPacketDataByte19 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte20") == 0) {
    s->m_uPacketDataByte20 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte21") == 0) {
    s->m_uPacketDataByte21 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte22") == 0) {
    s->m_uPacketDataByte22 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte23") == 0) {
    s->m_uPacketDataByte23 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte24") == 0) {
    s->m_uPacketDataByte24 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte25") == 0) {
    s->m_uPacketDataByte25 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte26") == 0) {
    s->m_uPacketDataByte26 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte27") == 0) {
    s->m_uPacketDataByte27 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte28") == 0) {
    s->m_uPacketDataByte28 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte29") == 0) {
    s->m_uPacketDataByte29 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte30") == 0) {
    s->m_uPacketDataByte30 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte31") == 0) {
    s->m_uPacketDataByte31 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte32") == 0) {
    s->m_uPacketDataByte32 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte33") == 0) {
    s->m_uPacketDataByte33 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte34") == 0) {
    s->m_uPacketDataByte34 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte35") == 0) {
    s->m_uPacketDataByte35 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte36") == 0) {
    s->m_uPacketDataByte36 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte37") == 0) {
    s->m_uPacketDataByte37 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte38") == 0) {
    s->m_uPacketDataByte38 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte39") == 0) {
    s->m_uPacketDataByte39 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte40") == 0) {
    s->m_uPacketDataByte40 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte41") == 0) {
    s->m_uPacketDataByte41 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte42") == 0) {
    s->m_uPacketDataByte42 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte43") == 0) {
    s->m_uPacketDataByte43 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte44") == 0) {
    s->m_uPacketDataByte44 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte45") == 0) {
    s->m_uPacketDataByte45 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte46") == 0) {
    s->m_uPacketDataByte46 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte47") == 0) {
    s->m_uPacketDataByte47 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte48") == 0) {
    s->m_uPacketDataByte48 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte49") == 0) {
    s->m_uPacketDataByte49 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte50") == 0) {
    s->m_uPacketDataByte50 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte51") == 0) {
    s->m_uPacketDataByte51 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte52") == 0) {
    s->m_uPacketDataByte52 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte53") == 0) {
    s->m_uPacketDataByte53 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte54") == 0) {
    s->m_uPacketDataByte54 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte55") == 0) {
    s->m_uPacketDataByte55 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte56") == 0) {
    s->m_uPacketDataByte56 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte57") == 0) {
    s->m_uPacketDataByte57 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte58") == 0) {
    s->m_uPacketDataByte58 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte59") == 0) {
    s->m_uPacketDataByte59 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte60") == 0) {
    s->m_uPacketDataByte60 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte61") == 0) {
    s->m_uPacketDataByte61 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte62") == 0) {
    s->m_uPacketDataByte62 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte63") == 0) {
    s->m_uPacketDataByte63 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte64") == 0) {
    s->m_uPacketDataByte64 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte65") == 0) {
    s->m_uPacketDataByte65 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte66") == 0) {
    s->m_uPacketDataByte66 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte67") == 0) {
    s->m_uPacketDataByte67 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte68") == 0) {
    s->m_uPacketDataByte68 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte69") == 0) {
    s->m_uPacketDataByte69 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte70") == 0) {
    s->m_uPacketDataByte70 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte71") == 0) {
    s->m_uPacketDataByte71 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte72") == 0) {
    s->m_uPacketDataByte72 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte73") == 0) {
    s->m_uPacketDataByte73 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte74") == 0) {
    s->m_uPacketDataByte74 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte75") == 0) {
    s->m_uPacketDataByte75 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte76") == 0) {
    s->m_uPacketDataByte76 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte77") == 0) {
    s->m_uPacketDataByte77 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte78") == 0) {
    s->m_uPacketDataByte78 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte79") == 0) {
    s->m_uPacketDataByte79 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte80") == 0) {
    s->m_uPacketDataByte80 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte81") == 0) {
    s->m_uPacketDataByte81 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte82") == 0) {
    s->m_uPacketDataByte82 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte83") == 0) {
    s->m_uPacketDataByte83 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte84") == 0) {
    s->m_uPacketDataByte84 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte85") == 0) {
    s->m_uPacketDataByte85 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte86") == 0) {
    s->m_uPacketDataByte86 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte87") == 0) {
    s->m_uPacketDataByte87 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte88") == 0) {
    s->m_uPacketDataByte88 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte89") == 0) {
    s->m_uPacketDataByte89 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte90") == 0) {
    s->m_uPacketDataByte90 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte91") == 0) {
    s->m_uPacketDataByte91 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte92") == 0) {
    s->m_uPacketDataByte92 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte93") == 0) {
    s->m_uPacketDataByte93 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte94") == 0) {
    s->m_uPacketDataByte94 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte95") == 0) {
    s->m_uPacketDataByte95 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte96") == 0) {
    s->m_uPacketDataByte96 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte97") == 0) {
    s->m_uPacketDataByte97 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte98") == 0) {
    s->m_uPacketDataByte98 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte99") == 0) {
    s->m_uPacketDataByte99 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte100") == 0) {
    s->m_uPacketDataByte100 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte101") == 0) {
    s->m_uPacketDataByte101 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte102") == 0) {
    s->m_uPacketDataByte102 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte103") == 0) {
    s->m_uPacketDataByte103 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte104") == 0) {
    s->m_uPacketDataByte104 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte105") == 0) {
    s->m_uPacketDataByte105 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte106") == 0) {
    s->m_uPacketDataByte106 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte107") == 0) {
    s->m_uPacketDataByte107 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte108") == 0) {
    s->m_uPacketDataByte108 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte109") == 0) {
    s->m_uPacketDataByte109 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte110") == 0) {
    s->m_uPacketDataByte110 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte111") == 0) {
    s->m_uPacketDataByte111 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte112") == 0) {
    s->m_uPacketDataByte112 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte113") == 0) {
    s->m_uPacketDataByte113 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte114") == 0) {
    s->m_uPacketDataByte114 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte115") == 0) {
    s->m_uPacketDataByte115 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte116") == 0) {
    s->m_uPacketDataByte116 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte117") == 0) {
    s->m_uPacketDataByte117 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte118") == 0) {
    s->m_uPacketDataByte118 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte119") == 0) {
    s->m_uPacketDataByte119 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte120") == 0) {
    s->m_uPacketDataByte120 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte121") == 0) {
    s->m_uPacketDataByte121 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte122") == 0) {
    s->m_uPacketDataByte122 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte123") == 0) {
    s->m_uPacketDataByte123 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte124") == 0) {
    s->m_uPacketDataByte124 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte125") == 0) {
    s->m_uPacketDataByte125 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte126") == 0) {
    s->m_uPacketDataByte126 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte127") == 0) {
    s->m_uPacketDataByte127 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte128") == 0) {
    s->m_uPacketDataByte128 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte129") == 0) {
    s->m_uPacketDataByte129 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte130") == 0) {
    s->m_uPacketDataByte130 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte131") == 0) {
    s->m_uPacketDataByte131 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte132") == 0) {
    s->m_uPacketDataByte132 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte133") == 0) {
    s->m_uPacketDataByte133 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte134") == 0) {
    s->m_uPacketDataByte134 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte135") == 0) {
    s->m_uPacketDataByte135 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte136") == 0) {
    s->m_uPacketDataByte136 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte137") == 0) {
    s->m_uPacketDataByte137 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte138") == 0) {
    s->m_uPacketDataByte138 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte139") == 0) {
    s->m_uPacketDataByte139 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte140") == 0) {
    s->m_uPacketDataByte140 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte141") == 0) {
    s->m_uPacketDataByte141 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte142") == 0) {
    s->m_uPacketDataByte142 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte143") == 0) {
    s->m_uPacketDataByte143 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte144") == 0) {
    s->m_uPacketDataByte144 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte145") == 0) {
    s->m_uPacketDataByte145 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte146") == 0) {
    s->m_uPacketDataByte146 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte147") == 0) {
    s->m_uPacketDataByte147 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte148") == 0) {
    s->m_uPacketDataByte148 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte149") == 0) {
    s->m_uPacketDataByte149 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte150") == 0) {
    s->m_uPacketDataByte150 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte151") == 0) {
    s->m_uPacketDataByte151 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte152") == 0) {
    s->m_uPacketDataByte152 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte153") == 0) {
    s->m_uPacketDataByte153 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte154") == 0) {
    s->m_uPacketDataByte154 = value;
  } else if (SB_STRCMP(name, "m_upacketdatabyte155") == 0) {
    s->m_uPacketDataByte155 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
