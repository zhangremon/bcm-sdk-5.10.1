# $Id: Makefile 1.24.44.1 Broadcom SDK $
# $Copyright: Copyright 2011 Broadcom Corporation.
# This program is the proprietary software of Broadcom Corporation
# and/or its licensors, and may only be used, duplicated, modified
# or distributed pursuant to the terms and conditions of a separate,
# written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized
# License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software
# and all intellectual property rights therein.  IF YOU HAVE
# NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
# IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
# ALL USE OF THE SOFTWARE.  
#  
# Except as expressly set forth in the Authorized License,
#  
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of
# Broadcom integrated circuit products.
#  
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
# PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
# OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
# INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
# ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
# TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
# THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
# ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
#
# Makefile for CINT library
#

ifneq (,$(findstring cint,$(MAKECMDGOALS)))
LBUILD=1
endif

ifneq (,$(findstring utest,$(MAKECMDGOALS)))
LBUILD=1
endif

ifndef LBUILD

# Build as part of the SDK
LOCALDIR = src/appl/cint
include ${SDK}/make/Make.config
lib = libcint
include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend

else

# Build standalone interpreter application

HDRS = $(wildcard *.h)
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

$(OBJS): $(HDRS) Makefile

CFLAGS = -g -DCINT_CONFIG_INCLUDE_STDLIB=1 -DCINT_CONFIG_INCLUDE_LOAD=1 -DCINT_CONFIG_INCLUDE_MAIN=1 -DCINT_CONFIG_INCLUDE_TEST_DATA=1 -Wall -Werror -DYYDEBUG

cint: $(OBJS)
	gcc -o $@ $+ -ldl -lrt -lreadline -lcurses

utest: cint
	rm -f *.code *.actual *.expected
	perl utest.pl utests

endif 

# Cancel implicit rules
%.c: %.y
%.c: %.l

ifdef CINT_PARSER_REGEN

# Regenerate scanner and parser headersg and sources first
$(HDRS): cint_c.lex.c cint_c.tab.c

FLEX ?= /projects/ntsw-tools/bin/flex-2.5.31
PATCH=patch
REPLACESTRING=/projects/ntsw-tools/bin/replacestring

cint_c.tab.c: cint_grammar.y
	yacc -dv -o $@ $<
# Avoid compiler warnings
	$(REPLACESTRING) -a "YYSTYPE yylval;" "YYSTYPE yylval = 0;" $@
	$(REPLACESTRING) -a "YYLTYPE yylloc;" "YYLTYPE yylloc = {0,0,0,0};" $@
cint_c.lex.c: cint_scanner.l lex.patch
	$(FLEX) -o $@ $<
	$(PATCH) < lex.patch
endif
