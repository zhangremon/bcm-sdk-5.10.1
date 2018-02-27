/*
 * $Id: sbZfC2PpAggregateHashFormatConsole.hx,v 1.3.16.4 2011/05/22 05:38:30 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfC2PpAggregateHashFormat.hx"
#ifndef SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_CONSOLE_H
#define SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_CONSOLE_H



void
sbZfC2PpAggregateHashFormat_Print(sbZfC2PpAggregateHashFormat_t *pFromStruct);
char *
sbZfC2PpAggregateHashFormat_SPrint(sbZfC2PpAggregateHashFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfC2PpAggregateHashFormat_Validate(sbZfC2PpAggregateHashFormat_t *pZf);
int
sbZfC2PpAggregateHashFormat_SetField(sbZfC2PpAggregateHashFormat_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFC2PPAGGREGATEHASHFORMAT_CONSOLE_H */
