/*
 * $Id: sbZfC2PrCcTcamConfigEntryFirstConsole.c 1.1.22.3 Broadcom SDK $
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
#include "sbZfC2PrCcTcamConfigEntryFirstConsole.hx"



/* Print members in struct */
void
sbZfC2PrCcTcamConfigEntryFirst_Print(sbZfC2PrCcTcamConfigEntryFirst_t *pFromStruct) {
  SB_LOG("C2PrCcTcamConfigEntryFirst:: mask0_31=0x%08x", (unsigned int)  pFromStruct->m_uMask0_31);
  SB_LOG(" mask32_63=0x%08x", (unsigned int)  pFromStruct->m_uMask32_63);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: mask64_95=0x%08x", (unsigned int)  pFromStruct->m_uMask64_95);
  SB_LOG(" mask96_127=0x%08x", (unsigned int)  pFromStruct->m_uMask96_127);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: mask128_159=0x%08x", (unsigned int)  pFromStruct->m_uMask128_159);
  SB_LOG(" mask160_191=0x%08x", (unsigned int)  pFromStruct->m_uMask160_191);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: mask192_199=0x%02x", (unsigned int)  pFromStruct->m_uMask192_199);
  SB_LOG(" maskreserved=0x%03x", (unsigned int)  pFromStruct->m_uMaskReserved);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: maskport=0x%02x", (unsigned int)  pFromStruct->m_uMaskPort);
  SB_LOG(" packet0_31=0x%08x", (unsigned int)  pFromStruct->m_uPacket0_31);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: packet32_63=0x%08x", (unsigned int)  pFromStruct->m_uPacket32_63);
  SB_LOG(" packet64_95=0x%08x", (unsigned int)  pFromStruct->m_uPacket64_95);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: packet96_127=0x%08x", (unsigned int)  pFromStruct->m_uPacket96_127);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: packet128_159=0x%08x", (unsigned int)  pFromStruct->m_uPacket128_159);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: packet160_191=0x%08x", (unsigned int)  pFromStruct->m_uPacket160_191);
  SB_LOG(" packet192_199=0x%02x", (unsigned int)  pFromStruct->m_uPacket192_199);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntryFirst:: reserved=0x%03x", (unsigned int)  pFromStruct->m_uReserved);
  SB_LOG(" port=0x%02x", (unsigned int)  pFromStruct->m_uPort);
  SB_LOG(" valid=0x%01x", (unsigned int)  pFromStruct->m_uValid);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PrCcTcamConfigEntryFirst_SPrint(sbZfC2PrCcTcamConfigEntryFirst_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: mask0_31=0x%08x", (unsigned int)  pFromStruct->m_uMask0_31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mask32_63=0x%08x", (unsigned int)  pFromStruct->m_uMask32_63);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: mask64_95=0x%08x", (unsigned int)  pFromStruct->m_uMask64_95);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mask96_127=0x%08x", (unsigned int)  pFromStruct->m_uMask96_127);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: mask128_159=0x%08x", (unsigned int)  pFromStruct->m_uMask128_159);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mask160_191=0x%08x", (unsigned int)  pFromStruct->m_uMask160_191);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: mask192_199=0x%02x", (unsigned int)  pFromStruct->m_uMask192_199);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maskreserved=0x%03x", (unsigned int)  pFromStruct->m_uMaskReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: maskport=0x%02x", (unsigned int)  pFromStruct->m_uMaskPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," packet0_31=0x%08x", (unsigned int)  pFromStruct->m_uPacket0_31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: packet32_63=0x%08x", (unsigned int)  pFromStruct->m_uPacket32_63);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," packet64_95=0x%08x", (unsigned int)  pFromStruct->m_uPacket64_95);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: packet96_127=0x%08x", (unsigned int)  pFromStruct->m_uPacket96_127);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: packet128_159=0x%08x", (unsigned int)  pFromStruct->m_uPacket128_159);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: packet160_191=0x%08x", (unsigned int)  pFromStruct->m_uPacket160_191);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," packet192_199=0x%02x", (unsigned int)  pFromStruct->m_uPacket192_199);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntryFirst:: reserved=0x%03x", (unsigned int)  pFromStruct->m_uReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port=0x%02x", (unsigned int)  pFromStruct->m_uPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," valid=0x%01x", (unsigned int)  pFromStruct->m_uValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PrCcTcamConfigEntryFirst_Validate(sbZfC2PrCcTcamConfigEntryFirst_t *pZf) {

  /* pZf->m_uMask0_31 implicitly masked by data type */
  /* pZf->m_uMask32_63 implicitly masked by data type */
  /* pZf->m_uMask64_95 implicitly masked by data type */
  /* pZf->m_uMask96_127 implicitly masked by data type */
  /* pZf->m_uMask128_159 implicitly masked by data type */
  /* pZf->m_uMask160_191 implicitly masked by data type */
  if (pZf->m_uMask192_199 > 0xff) return 0;
  if (pZf->m_uMaskReserved > 0x1ff) return 0;
  if (pZf->m_uMaskPort > 0x1f) return 0;
  /* pZf->m_uPacket0_31 implicitly masked by data type */
  /* pZf->m_uPacket32_63 implicitly masked by data type */
  /* pZf->m_uPacket64_95 implicitly masked by data type */
  /* pZf->m_uPacket96_127 implicitly masked by data type */
  /* pZf->m_uPacket128_159 implicitly masked by data type */
  /* pZf->m_uPacket160_191 implicitly masked by data type */
  if (pZf->m_uPacket192_199 > 0xff) return 0;
  if (pZf->m_uReserved > 0x1ff) return 0;
  if (pZf->m_uPort > 0x1f) return 0;
  if (pZf->m_uValid > 0x3) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PrCcTcamConfigEntryFirst_SetField(sbZfC2PrCcTcamConfigEntryFirst_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_umask0_31") == 0) {
    s->m_uMask0_31 = value;
  } else if (SB_STRCMP(name, "m_umask32_63") == 0) {
    s->m_uMask32_63 = value;
  } else if (SB_STRCMP(name, "m_umask64_95") == 0) {
    s->m_uMask64_95 = value;
  } else if (SB_STRCMP(name, "m_umask96_127") == 0) {
    s->m_uMask96_127 = value;
  } else if (SB_STRCMP(name, "m_umask128_159") == 0) {
    s->m_uMask128_159 = value;
  } else if (SB_STRCMP(name, "m_umask160_191") == 0) {
    s->m_uMask160_191 = value;
  } else if (SB_STRCMP(name, "m_umask192_199") == 0) {
    s->m_uMask192_199 = value;
  } else if (SB_STRCMP(name, "m_umaskreserved") == 0) {
    s->m_uMaskReserved = value;
  } else if (SB_STRCMP(name, "m_umaskport") == 0) {
    s->m_uMaskPort = value;
  } else if (SB_STRCMP(name, "m_upacket0_31") == 0) {
    s->m_uPacket0_31 = value;
  } else if (SB_STRCMP(name, "m_upacket32_63") == 0) {
    s->m_uPacket32_63 = value;
  } else if (SB_STRCMP(name, "m_upacket64_95") == 0) {
    s->m_uPacket64_95 = value;
  } else if (SB_STRCMP(name, "m_upacket96_127") == 0) {
    s->m_uPacket96_127 = value;
  } else if (SB_STRCMP(name, "m_upacket128_159") == 0) {
    s->m_uPacket128_159 = value;
  } else if (SB_STRCMP(name, "m_upacket160_191") == 0) {
    s->m_uPacket160_191 = value;
  } else if (SB_STRCMP(name, "m_upacket192_199") == 0) {
    s->m_uPacket192_199 = value;
  } else if (SB_STRCMP(name, "m_ureserved") == 0) {
    s->m_uReserved = value;
  } else if (SB_STRCMP(name, "m_uport") == 0) {
    s->m_uPort = value;
  } else if (SB_STRCMP(name, "m_uvalid") == 0) {
    s->m_uValid = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
