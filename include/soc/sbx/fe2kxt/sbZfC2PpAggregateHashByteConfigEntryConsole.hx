/*
 * $Id: sbZfC2PpAggregateHashByteConfigEntryConsole.hx,v 1.3.16.4 2011/05/22 05:38:30 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfC2PpAggregateHashByteConfigEntry.hx"
#ifndef SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_CONSOLE_H
#define SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_CONSOLE_H



void
sbZfC2PpAggregateHashByteConfigEntry_Print(sbZfC2PpAggregateHashByteConfigEntry_t *pFromStruct);
char *
sbZfC2PpAggregateHashByteConfigEntry_SPrint(sbZfC2PpAggregateHashByteConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfC2PpAggregateHashByteConfigEntry_Validate(sbZfC2PpAggregateHashByteConfigEntry_t *pZf);
int
sbZfC2PpAggregateHashByteConfigEntry_SetField(sbZfC2PpAggregateHashByteConfigEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_C2PPAGGREGATEHASHBYTECONFIGENTRY_CONSOLE_H */
