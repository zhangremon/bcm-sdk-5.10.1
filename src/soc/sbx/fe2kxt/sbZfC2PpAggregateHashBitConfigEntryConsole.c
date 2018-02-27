/*
 * $Id: sbZfC2PpAggregateHashBitConfigEntryConsole.c,v 1.1.34.3 2011/05/22 05:39:54 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfC2PpAggregateHashBitConfigEntryConsole.hx"



/* Print members in struct */
void
sbZfC2PpAggregateHashBitConfigEntry_Print(sbZfC2PpAggregateHashBitConfigEntry_t *pFromStruct) {
  SB_LOG("C2PpAggregateHashBitConfigEntry:: data9=0x%08x", (unsigned int)  pFromStruct->m_uData9);
  SB_LOG(" data8=0x%08x", (unsigned int)  pFromStruct->m_uData8);
  SB_LOG("\n");

  SB_LOG("C2PpAggregateHashBitConfigEntry:: data7=0x%08x", (unsigned int)  pFromStruct->m_uData7);
  SB_LOG(" data6=0x%08x", (unsigned int)  pFromStruct->m_uData6);
  SB_LOG("\n");

  SB_LOG("C2PpAggregateHashBitConfigEntry:: data5=0x%08x", (unsigned int)  pFromStruct->m_uData5);
  SB_LOG(" data4=0x%08x", (unsigned int)  pFromStruct->m_uData4);
  SB_LOG("\n");

  SB_LOG("C2PpAggregateHashBitConfigEntry:: data3=0x%08x", (unsigned int)  pFromStruct->m_uData3);
  SB_LOG(" data2=0x%08x", (unsigned int)  pFromStruct->m_uData2);
  SB_LOG("\n");

  SB_LOG("C2PpAggregateHashBitConfigEntry:: data1=0x%08x", (unsigned int)  pFromStruct->m_uData1);
  SB_LOG(" data0=0x%08x", (unsigned int)  pFromStruct->m_uData0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PpAggregateHashBitConfigEntry_SPrint(sbZfC2PpAggregateHashBitConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpAggregateHashBitConfigEntry:: data9=0x%08x", (unsigned int)  pFromStruct->m_uData9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data8=0x%08x", (unsigned int)  pFromStruct->m_uData8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpAggregateHashBitConfigEntry:: data7=0x%08x", (unsigned int)  pFromStruct->m_uData7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data6=0x%08x", (unsigned int)  pFromStruct->m_uData6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpAggregateHashBitConfigEntry:: data5=0x%08x", (unsigned int)  pFromStruct->m_uData5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data4=0x%08x", (unsigned int)  pFromStruct->m_uData4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpAggregateHashBitConfigEntry:: data3=0x%08x", (unsigned int)  pFromStruct->m_uData3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data2=0x%08x", (unsigned int)  pFromStruct->m_uData2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpAggregateHashBitConfigEntry:: data1=0x%08x", (unsigned int)  pFromStruct->m_uData1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data0=0x%08x", (unsigned int)  pFromStruct->m_uData0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PpAggregateHashBitConfigEntry_Validate(sbZfC2PpAggregateHashBitConfigEntry_t *pZf) {

  /* pZf->m_uData9 implicitly masked by data type */
  /* pZf->m_uData8 implicitly masked by data type */
  /* pZf->m_uData7 implicitly masked by data type */
  /* pZf->m_uData6 implicitly masked by data type */
  /* pZf->m_uData5 implicitly masked by data type */
  /* pZf->m_uData4 implicitly masked by data type */
  /* pZf->m_uData3 implicitly masked by data type */
  /* pZf->m_uData2 implicitly masked by data type */
  /* pZf->m_uData1 implicitly masked by data type */
  /* pZf->m_uData0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PpAggregateHashBitConfigEntry_SetField(sbZfC2PpAggregateHashBitConfigEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_udata9") == 0) {
    s->m_uData9 = value;
  } else if (SB_STRCMP(name, "m_udata8") == 0) {
    s->m_uData8 = value;
  } else if (SB_STRCMP(name, "m_udata7") == 0) {
    s->m_uData7 = value;
  } else if (SB_STRCMP(name, "m_udata6") == 0) {
    s->m_uData6 = value;
  } else if (SB_STRCMP(name, "m_udata5") == 0) {
    s->m_uData5 = value;
  } else if (SB_STRCMP(name, "m_udata4") == 0) {
    s->m_uData4 = value;
  } else if (SB_STRCMP(name, "m_udata3") == 0) {
    s->m_uData3 = value;
  } else if (SB_STRCMP(name, "m_udata2") == 0) {
    s->m_uData2 = value;
  } else if (SB_STRCMP(name, "m_udata1") == 0) {
    s->m_uData1 = value;
  } else if (SB_STRCMP(name, "m_udata0") == 0) {
    s->m_uData0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
