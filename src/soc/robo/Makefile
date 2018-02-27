# -*- Makefile -*-
# $Id: Makefile 1.19.2.1 Broadcom SDK $
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
# Makefile for SOC driver Library
#
LOCALDIR = src/soc/robo

ifeq (${IMP_PROTECT},1)
CFLAGS += -DIMP_SW_PROTECT
endif

ifeq (${STP_BY_EAP_BLK},1)
CFLAGS += -DSTP_BY_EAP_BLK
endif

include ${SDK}/make/Make.config

ifdef BCM_5324_A0
SUBDIR_TESTAROSSA = bcm5324
endif

ifdef BCM_5324_A1
SUBDIR_TESTAROSSA = bcm5324
endif

ifdef BCM_5396_A0
SUBDIR_DINO16= bcm5396
endif

ifdef BCM_5389_A0
SUBDIR_DINO16 = bcm5396
SUBDIR_DINO8 = bcm5389
endif

ifdef BCM_5398_A0
SUBDIR_DINO16 = bcm5396
SUBDIR_GRIPEN = bcm5398
endif

ifdef BCM_5348_A0
SUBDIR_SILVEROAK = bcm5348
endif

ifdef BCM_5397_A0
SUBDIR_DINO16 = bcm5396
SUBDIR_GRIPEN = bcm5398
endif
ifdef BCM_5347_A0
SUBDIR_SILVEROAK = bcm5348
endif

ifdef BCM_5395_A0
SUBDIR_DINO16 = bcm5396
SUBDIR_FALCON = bcm5395
endif

ifdef BCM_53242_A0
SUBDIR_HARRIER = bcm53242
endif

ifdef BCM_53262_A0
SUBDIR_HARRIER = bcm53242
endif

ifdef BCM_53115_A0
SUBDIR_FALCON = bcm5395
SUBDIR_VULCAN = bcm53115
endif

ifdef BCM_53118_A0
SUBDIR_FALCON = bcm5395
SUBDIR_VULCAN = bcm53115
SUBDIR_BLACKBIRD = bcm53118
endif

ifdef BCM_53280_A0
SUBDIR_THUNDERBOLT = bcm53280
endif

ifdef BCM_53280_B0
SUBDIR_THUNDERBOLT = bcm53280
endif

ifdef BCM_53600_A0
SUBDIR_THUNDERBOLT = bcm53280
SUBDIR_VOYAGER = bcm53600
endif

ifdef BCM_53101_A0
SUBDIR_FALCON = bcm5395
SUBDIR_VULCAN = bcm53115
SUBDIR_LOTUS = bcm53101
endif

ifdef BCM_53125_A0
SUBDIR_FALCON = bcm5395
SUBDIR_VULCAN = bcm53115
SUBDIR_STARFIGHTER = bcm53125
endif

ifdef BCM_53128_A0
SUBDIR_FALCON = bcm5395
SUBDIR_VULCAN = bcm53115
SUBDIR_BLACKBIRD2 = bcm53128
endif


subdirs := \
	$(SUBDIR_TESTAROSSA)\
	$(SUBDIR_DINO16) \
	$(SUBDIR_DINO8) \
	$(SUBDIR_GRIPEN) \
	$(SUBDIR_SILVEROAK) \
        ${SUBDIR_FALCON} \
        ${SUBDIR_HARRIER} \
        ${SUBDIR_VULCAN} \
        ${SUBDIR_BLACKBIRD} \
	${SUBDIR_THUNDERBOLT}\
        ${SUBDIR_LOTUS} \
        ${SUBDIR_STARFIGHTER} \
        ${SUBDIR_BLACKBIRD2} \
        $(SUBDIR_VOYAGER) \
	mcm

CFLAGS += -DBCM47XX_CHOPS -DDMA

include ${SDK}/make/Make.subdirs

lib = libsocrobo
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
