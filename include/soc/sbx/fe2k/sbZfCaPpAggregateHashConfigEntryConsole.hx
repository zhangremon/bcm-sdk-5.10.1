/*
 * $Id: sbZfCaPpAggregateHashConfigEntryConsole.hx,v 1.3.36.4 2011/05/22 05:38:04 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbZfCaPpAggregateHashConfigEntry.hx"
#ifndef SB_ZF_CAPPAGGREGATEHASHCONFIGENTRY_CONSOLE_H
#define SB_ZF_CAPPAGGREGATEHASHCONFIGENTRY_CONSOLE_H



void
sbZfCaPpAggregateHashConfigEntry_Print(sbZfCaPpAggregateHashConfigEntry_t *pFromStruct);
char *
sbZfCaPpAggregateHashConfigEntry_SPrint(sbZfCaPpAggregateHashConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize);
int
sbZfCaPpAggregateHashConfigEntry_Validate(sbZfCaPpAggregateHashConfigEntry_t *pZf);
int
sbZfCaPpAggregateHashConfigEntry_SetField(sbZfCaPpAggregateHashConfigEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_CAPPAGGREGATEHASHCONFIGENTRY_CONSOLE_H */
