/*
 * $Id: sbZfC2PpAggregateHashBitConfigEntryConsole.hx,v 1.3.16.4 2011/05/22 05:38:30 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfC2PpAggregateHashBitConfigEntry.hx"
#ifndef SB_ZF_C2PPAGGREGATEHASHBITCONFIGENTRY_CONSOLE_H
#define SB_ZF_C2PPAGGREGATEHASHBITCONFIGENTRY_CONSOLE_H



void
sbZfC2PpAggregateHashBitConfigEntry_Print(sbZfC2PpAggregateHashBitConfigEntry_t *pFromStruct);
char *
sbZfC2PpAggregateHashBitConfigEntry_SPrint(sbZfC2PpAggregateHashBitConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfC2PpAggregateHashBitConfigEntry_Validate(sbZfC2PpAggregateHashBitConfigEntry_t *pZf);
int
sbZfC2PpAggregateHashBitConfigEntry_SetField(sbZfC2PpAggregateHashBitConfigEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_C2PPAGGREGATEHASHBITCONFIGENTRY_CONSOLE_H */
