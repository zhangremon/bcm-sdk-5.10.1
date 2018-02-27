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
# $Id: Makefile 1.9.20.2 Broadcom SDK $
LOCALDIR = src/soc/sbx/g2p3

include ${SDK}/make/Make.config

G2_INC  = ${SDK}/include/soc/sbx/g2p3

# commented out with PPE Tools 2.0 \
GENERATE_C_FILES=1
GEN_INCS = $(G2_INC)

export GENERATE_C_FILES
export GEN_INCS

ZFS      = $(wildcard *.zf)
ZFCS     = $(addsuffix .c, $(basename ${ZFS})) \
	 	$(addsuffix Console.c, $(basename ${ZFS}))
ZFHXS    = $(addprefix ${G2_INC}/, $(addsuffix .hx, $(basename ${ZFS})) \
			 $(addsuffix Console.hx, $(basename ${ZFS})))
ZFOS     = $(addsuffix .o, $(basename ${ZFS}))

# not every .zf file generates a %Console.c file 
# by adding them to the object files list they will be
# linked into the library

# commented out with PPE Tools 2.0 \
ZFOS += \
sbZfG2P3FePpCamConfigConsole.o \
sbZfG2P3FePpHashTemplateConsole.o \
sbZfG2P3FeRxPortDataConsole.o \
sbZfG2P3FePpCamRamConfigConsole.o \
sbZfG2P3FePpInitialQueueStateConsole.o \
sbZfG2P3FePpCamStateConsole.o \
sbZfG2P3FePpStationMatchConsole.o

BOBJS    += $(addprefix ${BLDDIR}/, ${ZFOS})

# compile without intraprocedural optimizations
$(BLDDIR)/g2p3_pp_rule_encode.o: CFLAGS := $(subst -O2,-O0,$(CFLAGS))

lib = libsoc_g2p3_sbx

include ${SDK}/make/Make.lib

clean::
	$Q$(RM) $(ZFHXS) $(ZFCS)

.SECONDARY:: ${ZFCS} ${ZFHXS} ${MMAPHX}

include ${SDK}/make/Make.depend
