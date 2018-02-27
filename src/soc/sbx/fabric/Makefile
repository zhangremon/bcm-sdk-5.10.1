# $Id: Makefile 1.12.26.2 Broadcom SDK $
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

SDK :=$(shell if [ -n "$$SDK" ] ; then\
	echo $$SDK;\
	else\
	cd $(dir $(lastword $(MAKEFILE_LIST))); while /usr/bin/test ! -e RELEASE ; do \
	dir=`cd ../;pwd`;			\
	if [ "$$dir" = "/" ] ; then		\
	   echo Cannot find SDK in $(lastword $(MAKEFILE_LIST)) 1>&2;		\
	   exit 1;				\
	fi ;					\
	cd $$dir;				\
	done ;					\
	pwd;					\
	fi)

ifeq ($(SDK),)					
$(error Please run this in a tree)		
endif 				

export SDK

LOCALDIR = src/soc/sbx/fabric

include ${SDK}/make/Make.config

SBX_FABRIC_INC  = ${SDK}/include/soc/sbx/fabric

ZFS	= $(wildcard *.zf)
ZFCS	= $(addsuffix .c, $(basename ${ZFS})) \
		$(addsuffix Console.c, $(basename ${ZFS}))
ZFHXS	= $(addprefix ${SBX_FABRIC_INC}/, $(addsuffix .hx, $(basename ${ZFS}))\
		$(addsuffix Console.hx, $(basename ${ZFS})))
ZFOS	= $(addsuffix .o, $(basename ${ZFS}))

# not every .zf file generates a %Console.c file 
# by adding them to the object files list they will be
# linked into the library
ZFOS += \
sbZfFabBm9600BwAllocCfgBaseEntryConsole.o \
sbZfFabBm9600BwAllocRateEntryConsole.o \
sbZfFabBm9600BwFetchDataEntryConsole.o \
sbZfFabBm9600BwFetchSumEntryConsole.o \
sbZfFabBm9600BwFetchValidEntryConsole.o \
sbZfFabBm9600BwR0BagEntryConsole.o \
sbZfFabBm9600BwR0BwpEntryConsole.o \
sbZfFabBm9600BwR0WdtEntryConsole.o \
sbZfFabBm9600BwR1BagEntryConsole.o \
sbZfFabBm9600BwR1Wct0AEntryConsole.o \
sbZfFabBm9600BwR1Wct0BEntryConsole.o \
sbZfFabBm9600BwR1Wct1AEntryConsole.o \
sbZfFabBm9600BwR1Wct1BEntryConsole.o \
sbZfFabBm9600BwR1Wct2AEntryConsole.o \
sbZfFabBm9600BwR1Wct2BEntryConsole.o \
sbZfFabBm9600BwR1WstEntryConsole.o \
sbZfFabBm9600BwWredCfgBaseEntryConsole.o \
sbZfFabBm9600BwWredDropNPart1EntryConsole.o \
sbZfFabBm9600BwWredDropNPart2EntryConsole.o \
sbZfFabBm9600FoLinkStateTableEntryConsole.o \
sbZfFabBm9600FoTestInfoConsole.o \
sbZfFabBm9600InaEsetPriEntryConsole.o \
sbZfFabBm9600InaHi1Selected_0EntryConsole.o \
sbZfFabBm9600InaHi1Selected_1EntryConsole.o \
sbZfFabBm9600InaHi2Selected_0EntryConsole.o \
sbZfFabBm9600InaHi2Selected_1EntryConsole.o \
sbZfFabBm9600InaHi3Selected_0EntryConsole.o \
sbZfFabBm9600InaHi3Selected_1EntryConsole.o \
sbZfFabBm9600InaHi4Selected_0EntryConsole.o \
sbZfFabBm9600InaHi4Selected_1EntryConsole.o \
sbZfFabBm9600InaNmPriorityUpdateConsole.o \
sbZfFabBm9600InaPortPriEntryConsole.o \
sbZfFabBm9600InaRandomNumGenEntryConsole.o \
sbZfFabBm9600InaSysportMapEntryConsole.o \
sbZfFabBm9600LinkFailureInfoConsole.o \
sbZfFabBm9600NmEgressRankerEntryConsole.o \
sbZfFabBm9600NmEmtEntryConsole.o \
sbZfFabBm9600NmEmt_0EntryConsole.o \
sbZfFabBm9600NmEmt_0_1EntryConsole.o \
sbZfFabBm9600NmEmt_1EntryConsole.o \
sbZfFabBm9600NmEmtdebugbank0EntryConsole.o \
sbZfFabBm9600NmEmtdebugbank1EntryConsole.o \
sbZfFabBm9600NmIngressRankerEntryConsole.o \
sbZfFabBm9600NmPortsetInfoEntryConsole.o \
sbZfFabBm9600NmPortsetLinkEntryConsole.o \
sbZfFabBm9600NmRandomNumGenEntryConsole.o \
sbZfFabBm9600NmSysportArrayEntryConsole.o \
sbZfFabBm9600XbXcfgRemapEntryConsole.o \
sbZfFabBm9600NmFullStatusEntryConsole.o


lib = lib_sbx_fabric

include $(SDK)/make/Make.lib

include ${SDK}/make/Make.depend
