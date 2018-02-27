# $Id: Makefile 1.10 Broadcom SDK $
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
# Makefile for BCM driver Library
#

LOCALDIR = src/bcm/sbx

include ${SDK}/make/Make.config
CFGFLAGS += -DBCM_CES_SDK -DAG_GCC -DAG_STAM 
CFLAGS += -I${SDK}/include/soc/ces/nemo_driver -I${SDK}/include/soc/ces/include_bcm -I${SDK}/include/soc/ces/include_int -I${SDK}/include/soc/ces/include_ext -I${SDK}/include/soc/ces/clsbuilder

ifdef BCM_QE2000_A0
SUBDIR_QE2000 = qe2000
endif

ifdef BCM_BME3200_A0
SUBDIR_BME3200 = bme3200
endif

ifdef BCM_BME3200_B0
SUBDIR_BME3200 = bme3200
endif

ifdef BCM_BM9600_A0
SUBDIR_BM9600 = bm9600
endif

ifdef BCM_BM9600_A1
SUBDIR_BM9600 = bm9600
endif

ifdef BCM_BM9600_B0
SUBDIR_BM9600 = bm9600
endif

ifdef BCM_FE2000_A0
SUBDIR_FE2000 = fe2000
endif

ifdef BCM_88230_A0
SUBDIR_SIRIUS = sirius
endif

ifdef BCM_88230_B0
SUBDIR_SIRIUS = sirius
endif

ifdef BCM_88230_C0
SUBDIR_SIRIUS = sirius
endif

subdirs := \
	common \
	$(SUBDIR_QE2000) \
	$(SUBDIR_BME3200) \
	$(SUBDIR_BM9600) \
	$(SUBDIR_FE2000) \
	$(SUBDIR_SIRIUS) \
	$(SUBDIR_NONE)

include ${SDK}/make/Make.subdirs

lib = libbcm_sbx
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
