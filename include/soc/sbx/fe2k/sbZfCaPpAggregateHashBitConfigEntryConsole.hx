/*
 * $Id: sbZfCaPpAggregateHashBitConfigEntryConsole.hx,v 1.3.36.4 2011/05/22 05:38:03 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfCaPpAggregateHashBitConfigEntry.hx"
#ifndef SB_ZF_CAPPAGGREGATEHASHBITCONFIGENTRY_CONSOLE_H
#define SB_ZF_CAPPAGGREGATEHASHBITCONFIGENTRY_CONSOLE_H



void
sbZfCaPpAggregateHashBitConfigEntry_Print(sbZfCaPpAggregateHashBitConfigEntry_t *pFromStruct);
char *
sbZfCaPpAggregateHashBitConfigEntry_SPrint(sbZfCaPpAggregateHashBitConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfCaPpAggregateHashBitConfigEntry_Validate(sbZfCaPpAggregateHashBitConfigEntry_t *pZf);
int
sbZfCaPpAggregateHashBitConfigEntry_SetField(sbZfCaPpAggregateHashBitConfigEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_CAPPAGGREGATEHASHBITCONFIGENTRY_CONSOLE_H */
