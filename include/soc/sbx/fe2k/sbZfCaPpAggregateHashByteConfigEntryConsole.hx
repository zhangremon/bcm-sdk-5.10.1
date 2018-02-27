/*
 * $Id: sbZfCaPpAggregateHashByteConfigEntryConsole.hx,v 1.3.36.4 2011/05/22 05:38:03 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfCaPpAggregateHashByteConfigEntry.hx"
#ifndef SB_ZF_CAPPAGGREGATEHASHBYTECONFIGENTRY_CONSOLE_H
#define SB_ZF_CAPPAGGREGATEHASHBYTECONFIGENTRY_CONSOLE_H



void
sbZfCaPpAggregateHashByteConfigEntry_Print(sbZfCaPpAggregateHashByteConfigEntry_t *pFromStruct);
char *
sbZfCaPpAggregateHashByteConfigEntry_SPrint(sbZfCaPpAggregateHashByteConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfCaPpAggregateHashByteConfigEntry_Validate(sbZfCaPpAggregateHashByteConfigEntry_t *pZf);
int
sbZfCaPpAggregateHashByteConfigEntry_SetField(sbZfCaPpAggregateHashByteConfigEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_CAPPAGGREGATEHASHBYTECONFIGENTRY_CONSOLE_H */
