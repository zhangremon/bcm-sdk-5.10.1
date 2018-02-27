/*
 * $Id: sbZfC2PpAggregateHashConfigEntryConsole.hx,v 1.3.16.4 2011/05/22 05:38:30 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfC2PpAggregateHashConfigEntry.hx"
#ifndef SB_ZF_C2PPAGGREGATEHASHCONFIGENTRY_CONSOLE_H
#define SB_ZF_C2PPAGGREGATEHASHCONFIGENTRY_CONSOLE_H



void
sbZfC2PpAggregateHashConfigEntry_Print(sbZfC2PpAggregateHashConfigEntry_t *pFromStruct);
char *
sbZfC2PpAggregateHashConfigEntry_SPrint(sbZfC2PpAggregateHashConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfC2PpAggregateHashConfigEntry_Validate(sbZfC2PpAggregateHashConfigEntry_t *pZf);
int
sbZfC2PpAggregateHashConfigEntry_SetField(sbZfC2PpAggregateHashConfigEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_C2PPAGGREGATEHASHCONFIGENTRY_CONSOLE_H */
