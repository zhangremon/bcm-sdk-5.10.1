/*
 * $Id: g2p3_incl.h 1.4 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * This file contains top level includes for g2p3 microcode

 * Mostly, this file is here to break a circularity in inclusion
 * and type definition for the soc_sbx_g2p3_state_t, but
 * it includes files that are indepenent of that definition to
 * allow other files (esp. the generated table definitions) to include
 * only it.
 */
#ifndef _SOC_SBX_G2P3_INCL_H
#define _SOC_SBX_G2P3_INCL_H
#ifdef BCM_SBX_SUPPORT
#include <sal/types.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/glue.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_common.h>
#include <soc/sbx/fe2k_common/sbFe2000DmaMgr.h>
#include <soc/sbx/fe2k_common/sbFe2000CmuMgr.h>
#include <soc/sbx/fe2k_common/sbFe2000Pm.h>
#include <soc/sbx/fe2k_common/sbFe2000UcodeLoad.h>
#include <soc/sbx/fe2k/sbFe2000ClsComp.h>
#include <soc/sbx/fe2k_common/sbFeISupport.h>
#include <soc/sbx/g2p3/g2p3_zframes.h>

/* typedef struct soc_sbx_g2p3_state_s soc_sbx_g2p3_state_t; */

typedef uint32_t sramAddr;

#endif
#endif
