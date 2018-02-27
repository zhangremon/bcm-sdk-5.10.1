/*
 * $Id: sbZfCaPpAggregateHashFormatConsole.hx,v 1.3.36.4 2011/05/22 05:38:04 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfCaPpAggregateHashFormat.hx"
#ifndef SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_CONSOLE_H
#define SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_CONSOLE_H



void
sbZfCaPpAggregateHashFormat_Print(sbZfCaPpAggregateHashFormat_t *pFromStruct);
char *
sbZfCaPpAggregateHashFormat_SPrint(sbZfCaPpAggregateHashFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfCaPpAggregateHashFormat_Validate(sbZfCaPpAggregateHashFormat_t *pZf);
int
sbZfCaPpAggregateHashFormat_SetField(sbZfCaPpAggregateHashFormat_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_CONSOLE_H */
