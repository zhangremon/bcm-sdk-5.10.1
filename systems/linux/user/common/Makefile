# -*- Makefile -*-
# $Id: Makefile 1.66.4.1 Broadcom SDK $
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

override-target=unix-user

ifeq ($(LINUX_MAKE_SHARED_LIB),1)
libext = so.$(SHAREDLIBVER)
else
libext = a
endif

ifeq (,$(platform))
$(error Internal error: platform variable is not set)
endif

include $(SDK)/make/Make.config
LOCALDIR=systems/linux/user/common
include $(SDK)/make/Make.depend
kernel-override=linux-$(platform)
ifdef BLDCONFIG
KERN_BLDROOT=${SDK}/${SDKBUILD}/${BLDCONFIG}/$(kernel-override)$(bldroot_suffix)
else
KERN_BLDROOT=${SDK}/${SDKBUILD}/$(kernel-override)$(bldroot_suffix)
endif

ifeq (,$(kernel_version))
kernel_version=2_4
endif

ifeq ($(kernel_version),2_6)
ADD_TO_CFLAGS += -DLKM_2_6
KOBJ=ko
else
ADD_TO_CFLAGS += -DLKM_2_4
KOBJ=o
endif

ifneq (,$(TOOLS_DIR))
override PATH := $(TOOLS_DIR):$(PATH)
endif

ifeq ($(DEST_DIR),)
DEST_DIR=${BLDDIR}
endif

BCM_LOCAL :=bcm.$(targetplat)$(all_suffix)
BCM = $(DEST_DIR)/$(BCM_LOCAL)

KERNEL_BDE_LOCAL :=linux-kernel-bde.$(KOBJ)
KERNEL_BDE :=$(DEST_DIR)/$(KERNEL_BDE_LOCAL)

USER_BDE_LOCAL :=linux-user-bde.$(KOBJ)
USER_BDE=$(DEST_DIR)/$(USER_BDE_LOCAL)

UK_PROXY_LOCAL :=linux-uk-proxy.$(KOBJ)
BCM_KNET_LOCAL :=linux-bcm-knet.$(KOBJ)

UK_PROXY=$(DEST_DIR)/$(UK_PROXY_LOCAL)
BCM_KNET=$(DEST_DIR)/$(BCM_KNET_LOCAL)

NETSERVE_LOCAL :=netserve
NETSERVE = $(DEST_DIR)/$(NETSERVE_LOCAL)

ifeq (,$(findstring DELIVER,$(MAKECMDGOALS)))
.DEFAULT_GOAL := all
all_targets := kernel_modules user_libs $(KERNEL_BDE) $(USER_BDE) $(BCM) $(NETSERVE)

LOCAL_TARGETS :=

ifeq ($(NO_LOCAL_TARGETS),)
LOCAL_TARGETS +=$(patsubst %,$(realpath ..)/$(platform)/%,$(KERNEL_BDE_LOCAL) $(USER_BDE_LOCAL) $(BCM_LOCAL) $(BCM_LOCAL).dbg $(NETSERVE_LOCAL))

all_targets +=$(LOCAL_TARGETS)
endif

ifdef BUILD_KNET
# Kernel network support
all_targets += $(UK_PROXY) $(BCM_KNET)

ifeq ($(NO_LOCAL_TARGETS),)
LOCAL_TARGETS +=$(patsubst %,../$(platform)/%,$(UK_PROXY_LOCAL) $(BCM_KNET_LOCAL))
all_targets +=$(LOCAL_TARGETS)
endif

ADD_TO_CFLAGS += -I$(SDK)/systems/linux/kernel/modules/include
COND_KNET_LIBS = libuser.$(libext)
endif


all: $(BLDDIR)/.tree $(all_targets)

ifeq ($(NO_LOCAL_TARGETS),)
define LOCAL_TARGET_DEF
$(1) : $(DEST_DIR)/$(notdir $(1))
	cp $$? $$@
endef
endif

# Target specific
ADD_TO_CFLAGS += -DLINUX_SAL_DMA_ALLOC_OVERRIDE
ADD_TO_CFLAGS += -DLINUX_SAL_USLEEP_OVERRIDE -DLINUX_SAL_SEM_OVERRIDE
ADD_TO_CFLAGS += -DLINUX_SAL_UDELAY_OVERRIDE

# User BDE libraries
ADD_TO_CFLAGS += -I$(SDK)/systems/bde/linux/include

CFLAGS += $(ADD_TO_CFLAGS)

ifdef ESW_CHIPS
ESW_DIAG_LIBS = libdiag_esw.$(libext) 
endif

ifdef ROBO_CHIPS
ROBO_DIAG_LIBS = libdiagrobo.$(libext)
endif

ifdef SBX_CHIPS
SBX_DIAG_LIBS = \
	libdiag_sbx.$(libext)
endif

ifdef EA_CHIPS
EA_DIAG_LIBS = \
	libdiag_ea.$(libext)
endif

DIAG_LIBS = \
        libdiag.$(libext)       \
        ${ESW_DIAG_LIBS} \
        ${ROBO_DIAG_LIBS} \
        ${SBX_DIAG_LIBS} \
        ${EA_DIAG_LIBS} \
	$(COND_LIB_EAV_APPL) \
	${COND_LIB_BCMX_DIAG} \
        ${COND_LIB_TEST} \
	$(COND_LIB_TCL) \
	$(APPL_LIBS) \
	$(COND_LIB_EDITLINE) \
	$(COND_LIB_WBDM) \
	$(COND_LIB_SMB) \
	$(COND_LIB_PHY_DIAGNOSTICS) \
	$(COND_KNET_LIBS) \
	libsal_appl.$(libext) libsal_appl_plat.$(libext) liblubde.$(libext)

ACL_LIB = $(COND_LIB_ACL)
BCM_LIBS = $(DIAG_LIBS) $(DRV_LIBS) $(ACL_LIB) $(COND_LIB_ETH_DRV)
BCM_LIBS_BLD = $(addprefix $(BLDDIR)/,$(BCM_LIBS))

BCM_LIBS_BLD += ${EXTERNAL_LIBS}

kernel_modules:
	$(MAKE) -C $(SDK)/systems/bde/linux/kernel kernel_version=$(kernel_version)
	$(MAKE) -C $(SDK)/systems/bde/linux/user/kernel kernel_version=$(kernel_version)
ifdef BUILD_KNET
	$(MAKE) -C $(SDK)/systems/linux/kernel/modules kernel_version=$(kernel_version) \
		subdirs="shared uk-proxy bcm-knet" override-target=linux-$(platform)
endif

user_libs:
ifdef CUSTOMER_SOURCE_DIR
	$(MAKE) -C $(CUSTOMER_SOURCE_DIR) CFLAGS="$(CFLAGS)"
endif
	$(MAKE) -C $(SDK)/systems/bde/linux/user CFLAGS="$(CFLAGS)"
	$(MAKE) -C $(SDK)/src CFLAGS="$(ADD_TO_CFLAGS)"
	$(MAKE) -C $(SDK)/systems/drv CFLAGS="$(CFLAGS)"
ifdef BUILD_KNET
	$(MAKE) -C $(SDK)/systems/linux/kernel/user/shared CFLAGS="$(CFLAGS)"
endif
ifdef BUILD_MACSEC
	@${ECHO} "Building MACSEC libraries ...."
	$(MAKE) -C ${MACSEC_HOME} MACSEC_CC="${CC}" MACSEC_LD="${LD}" MACSEC_AR="$(AR)" MACSEC_CFLAGS="${MACSEC_BUILD_FLAG}" MACSEC_TARGET="${targetbase}/${targetplat}/${platform}"
endif

# Broadcom Command Monitor

# Build platform_defines.c to compile in config.bcm
#
ifndef NO_SAL_APPL
PLATFORM_DEFINES_OBJ=${BLDDIR}/platform_defines.o
${BLDDIR}/platform_defines.c: ${SDK}/rc/config.bcm
	$(PERL) ${SDK}/tools/bcm2c_linux.pl ${SDK}/rc/config.bcm >$@
${BLDDIR}/platform_defines.o: ${BLDDIR}/platform_defines.c
	${CC} ${CFLAGS} -c $< -o $@
else
PLATFORM_DEFINES_OBJ=
endif


$(BCM): $(BCM).dbg
	mkdir -p $(@D)
	$(OBJCOPY) --strip-debug $< $@

$(BCM).dbg: $(BLDDIR)/socdiag.o $(BLDDIR)/version.o \
	${PLATFORM_DEFINES_OBJ} $(BCM_LIBS_BLD) 
	@rm -f $@	# Prevent core dump if currently running
	@echo Linking $@ # This is needed for error parsing
	$(CC) $(CFLAGS) -o $@ $(BLDDIR)/socdiag.o $(BLDDIR)/version.o \
	-Wl,--start-group ${PLATFORM_DEFINES_OBJ} $(BCM_LIBS_BLD)  -Wl,--end-group $(LDFLAGS)


$(KERNEL_BDE): $(KERN_BLDROOT)/linux-kernel-bde.$(KOBJ)
	mkdir -p $(@D)
	$(OBJCOPY) --strip-debug $< $@

$(USER_BDE): $(KERN_BLDROOT)/linux-user-bde.$(KOBJ)
	$(OBJCOPY) --strip-debug $< $@

$(UK_PROXY): $(KERN_BLDROOT)/linux-uk-proxy.$(KOBJ)
	$(OBJCOPY) --strip-debug $< $@

$(BCM_KNET): $(KERN_BLDROOT)/linux-bcm-knet.$(KOBJ)
	$(OBJCOPY) --strip-debug $< $@

$(NETSERVE): $(BLDDIR)/netserve.o
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $(BLDDIR)/netserve.o $(LDFLAGS) -lutil
	$(OBJCOPY) --strip-debug $@

ifeq ($(NO_LOCAL_TARGETS),)
$(foreach targ,$(LOCAL_TARGETS),$(eval $(call LOCAL_TARGET_DEF,$(targ))))
endif

# Build Version
$(BLDDIR)/version.c: $(BLDDIR)/socdiag.o $(BCM_LIBS_BLD)
	$(PERL) $(SDK)/tools/version.pl > $@
$(BLDDIR)/version.o: $(BLDDIR)/version.c
	$(CC) $(CFLAGS) -c $< -o $@

clean::
	$(MAKE) -C $(SDK)/systems/bde/linux/kernel $@
	$(MAKE) -C $(SDK)/systems/bde/linux/user/kernel $@
	$(MAKE) -C $(SDK)/systems/bde/linux/user CFLAGS="$(CFLAGS)" $@
	$(MAKE) -C $(SDK)/src CFLAGS="$(CFLAGS)" $@
	$(MAKE) -C $(SDK)/systems/linux/kernel/user/shared $@
	$(MAKE) -C $(SDK)/systems/linux/kernel/modules \
		subdirs="shared uk-proxy bcm-knet" \
		override-target=linux-$(platform) $@
	$(RM) $(BLDDIR)/version.c $(BLDDIR)/version.o
	$(RM) $(BLDDIR)/platform_defines.c $(BLDDIR)/platform_defines.o
	$(RM) $(BLDDIR)/socdiag.o
	$(RM) $(KERNEL_BDE) $(USER_BDE) $(BCM) $(BCM).dbg
	$(RM) $(UK_PROXY) $(BCM_KNET)
	$(RM) $(KERN_BLDROOT)/linux-kernel-bde.$(KOBJ)
	$(RM) $(KERN_BLDROOT)/linux-user-bde.$(KOBJ)
	$(RM) $(KERN_BLDROOT)/linux-uk-proxy.$(KOBJ)
	$(RM) $(KERN_BLDROOT)/linux-bcm-knet.$(KOBJ)
	$(RM) $(BLDDIR)/netserve.o
	$(RM) $(NETSERVE) $(LOCAL_TARGETS)

distclean:: clean

.PHONY: variable

#
# Echo variable values used for configuration
# usage: make VAR=CC variable
#
variable::
	@echo $($(VAR))

else
ifndef DELIVERY
$(error DELIVERY required)
endif

DELIVERABLES := $(KERNEL_BDE) $(USER_BDE) $(BCM) $(BCM).dbg $(NETSERVE)
# 1= source
define DELIVERY_RULE
_DEL_TARG += $(DELIVERY)/$(notdir $(1))
$(DELIVERY)/$(notdir $(1)) : $(1)
	@mkdir -p $(DELIVERY)
ifeq ($(filter $(notdir $(BCM)) $(notdir $(BCM).dbg),$(notdir $1)),)
	$(OBJCOPY) --strip-debug $$< $$@
else
	$(CP) -p $$< $$@
endif
endef

$(foreach f,$(DELIVERABLES),$(eval $(call DELIVERY_RULE,$(f))))

DELIVER: $(_DEL_TARG)
endif

# Make.config defines remaining phony targets
.PHONY: build kernel_modules user_libs DELIVER
