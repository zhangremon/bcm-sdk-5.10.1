/*
 * $Id: sbZfCaPpAggregateHashByteConfigEntryConsole.c,v 1.1.48.3 2011/05/22 05:39:26 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfCaPpAggregateHashByteConfigEntryConsole.hx"



/* Print members in struct */
void
sbZfCaPpAggregateHashByteConfigEntry_Print(sbZfCaPpAggregateHashByteConfigEntry_t *pFromStruct) {
  SB_LOG("CaPpAggregateHashByteConfigEntry:: data1=0x%02x", (unsigned int)  pFromStruct->m_uData1);
  SB_LOG(" data0=0x%08x", (unsigned int)  pFromStruct->m_uData0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPpAggregateHashByteConfigEntry_SPrint(sbZfCaPpAggregateHashByteConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpAggregateHashByteConfigEntry:: data1=0x%02x", (unsigned int)  pFromStruct->m_uData1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data0=0x%08x", (unsigned int)  pFromStruct->m_uData0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPpAggregateHashByteConfigEntry_Validate(sbZfCaPpAggregateHashByteConfigEntry_t *pZf) {

  if (pZf->m_uData1 > 0xff) return 0;
  /* pZf->m_uData0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPpAggregateHashByteConfigEntry_SetField(sbZfCaPpAggregateHashByteConfigEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
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
