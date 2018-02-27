/*
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
 * $Id: g2p3_cc.c 1.1.184.1 Broadcom SDK $
 *
 *-----------------------------------------------------------------------------*/
#include <soc/error.h>
#include <soc/sbx/g2p3/g2p3_cc.h>
#include "soc/sbx/sbx_drv.h"
#include <soc/sbx/fe2k_common/sbFe2000CommonDriver.h>
#include <soc/sbx/fe2kxt/sbFe2000XtInitUtils.h>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntryFirst.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntryNonFirst.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcCamRamNonLastEntry.hx>

#define PUBLIC
#define PRIVATE static

#define G2P3_CC_MAX_PORTS 12

/* CC TCAM index management */
#define G2P3_CC_L2CP_INDEX 0
#define G2P3_CC_VLAN_TPID_INDEX 1
#define G2P3_CC_VLAN_TPID_ENTRIES 4
#define G2P3_CC_VLAN_PRIMAP_INDEX (G2P3_CC_VLAN_TPID_INDEX + G2P3_CC_VLAN_TPID_ENTRIES)
#define G2P3_CC_VLAN_PRIMAP_ENTRIES 8

#define G2P3_CC_PORT_DEFAULT_INDEX SB_FE2000XT_PR_CC_LAST_VALID_RULE

/* CC TCAM states */
#define G2P3_CC_TCAM_STATE_TPID_MATCH 1

int
soc_sbx_g2p3_cc_l2cp_set(int unit,
                         uint32_t instance,
                         uint32_t portmask,
                         g2p3_cc_de_t de)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    uint32_t port;
    sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
    sbZfC2PrCcTcamConfigEntryFirst_t zTcamConfigEntryFirst;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (portmask >= (1 << G2P3_CC_MAX_PORTS)) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
    sbZfC2PrCcTcamConfigEntryFirst_InitInstance(&zTcamConfigEntryFirst);

    if (C2PrCcCamRuleRead(sbx->sbhdl, instance, G2P3_CC_L2CP_INDEX, uCfgBuffer, uCfgRamBuffer) != TRUE) {
        return SOC_E_FAIL;
    }

    sbZfC2PrCcTcamConfigEntryFirst_Unpack(&zTcamConfigEntryFirst, (uint8_t *) &uCfgBuffer[0],
                             SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
    sbZfC2PrCcCamRamLastEntry_Unpack(&zLastEntryRam, (uint8_t *) &uCfgRamBuffer[0],
                                SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);

    zTcamConfigEntryFirst.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;
    /* match on l2cp mac */
    zTcamConfigEntryFirst.m_uPacket0_31 = 0x0180c200;
    zTcamConfigEntryFirst.m_uPacket32_63 = 0x00000000;
    zTcamConfigEntryFirst.m_uMask0_31= 0xffffffff;
    zTcamConfigEntryFirst.m_uMask32_63= 0xff000000;

    /* use port-based default queues */
    zLastEntryRam.m_uQueueAction = SB_ZF_C2PRCCCAMRAMLASTENTRY_QUEUE_ACT_DEFAULT_ONLY;
    zLastEntryRam.m_uQueue = 0;
    zLastEntryRam.m_uDrop = (de == G2P3_CC_DE_DROP) ? 1 : 0;
    zLastEntryRam.m_uLast = 1;

    /* loop over all ports and set de for each port */
    for (port = 0; port < G2P3_CC_MAX_PORTS; port++) {
        if ( (1 << port) & portmask) {
            switch (port) {
            case 0: zLastEntryRam.m_uDEPort0 = de; break;
            case 1: zLastEntryRam.m_uDEPort1 = de; break;
            case 2: zLastEntryRam.m_uDEPort2 = de; break;
            case 3: zLastEntryRam.m_uDEPort3 = de; break;
            case 4: zLastEntryRam.m_uDEPort4 = de; break;
            case 5: zLastEntryRam.m_uDEPort5 = de; break;
            case 6: zLastEntryRam.m_uDEPort6 = de; break;
            case 7: zLastEntryRam.m_uDEPort7 = de; break;
            case 8: zLastEntryRam.m_uDEPort8 = de; break;
            case 9: zLastEntryRam.m_uDEPort9 = de; break;
            case 10: zLastEntryRam.m_uDEPort10 = de; break;
            case 11: zLastEntryRam.m_uDEPort11 = de; break;
            default:
              return SOC_E_PARAM;
            }
        }
    }

    sbZfC2PrCcTcamConfigEntryFirst_Pack(&zTcamConfigEntryFirst, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
    sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

    if (C2PrCcCamRuleWrite(sbx->sbhdl, instance, G2P3_CC_L2CP_INDEX, uCfgBuffer, uCfgRamBuffer, 1) != TRUE) {
        return SB_FAILED;
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cc_l2cp_get(int unit,
                         uint32_t instance,
                         uint32_t port,
                         g2p3_cc_de_t *de)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
    sbZfC2PrCcTcamConfigEntryFirst_t zTcamConfigEntryFirst;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (port >= (1 << G2P3_CC_MAX_PORTS)) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
    sbZfC2PrCcTcamConfigEntryFirst_InitInstance(&zTcamConfigEntryFirst);

    if (C2PrCcCamRuleRead(sbx->sbhdl, instance, G2P3_CC_L2CP_INDEX, uCfgBuffer, uCfgRamBuffer) != TRUE) {
        return SOC_E_FAIL;
    }

    sbZfC2PrCcTcamConfigEntryFirst_Unpack(&zTcamConfigEntryFirst, (uint8_t *) &uCfgBuffer[0],
                             SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
    sbZfC2PrCcCamRamLastEntry_Unpack(&zLastEntryRam, (uint8_t *) &uCfgRamBuffer[0],
                                SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);

    if (zTcamConfigEntryFirst.m_uValid != SB_FE2000XT_PR_CC_KEY_VALID) {
        return SOC_E_FAIL;
    }
    
    switch (port) {
    case 0: *de = zLastEntryRam.m_uDEPort0; break;
    case 1: *de = zLastEntryRam.m_uDEPort1; break;
    case 2: *de = zLastEntryRam.m_uDEPort2; break;
    case 3: *de = zLastEntryRam.m_uDEPort3; break;
    case 4: *de = zLastEntryRam.m_uDEPort4; break;
    case 5: *de = zLastEntryRam.m_uDEPort5; break;
    case 6: *de = zLastEntryRam.m_uDEPort6; break;
    case 7: *de = zLastEntryRam.m_uDEPort7; break;
    case 8: *de = zLastEntryRam.m_uDEPort8; break;
    case 9: *de = zLastEntryRam.m_uDEPort9; break;
    case 10: *de = zLastEntryRam.m_uDEPort10; break;
    case 11: *de = zLastEntryRam.m_uDEPort11; break;
    default:
      return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cc_tpid_set(int unit,
                         uint32_t instance,
                         uint32_t tpid_index,
                         uint32_t tpid)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbZfC2PrCcCamRamNonLastEntry_t zNonLastEntryRam;
    sbZfC2PrCcTcamConfigEntryFirst_t zTcamConfigEntryFirst;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (tpid_index >= G2P3_CC_VLAN_TPID_ENTRIES) {
        return SOC_E_PARAM;
    }

    if (tpid > 0xffff) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamNonLastEntry_InitInstance(&zNonLastEntryRam);
    sbZfC2PrCcTcamConfigEntryFirst_InitInstance(&zTcamConfigEntryFirst);

    zTcamConfigEntryFirst.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;

    /* match on TPID of first vlan tag */
    zTcamConfigEntryFirst.m_uPacket0_31 = 0x0;
    zTcamConfigEntryFirst.m_uPacket32_63 = 0x0;
    zTcamConfigEntryFirst.m_uPacket64_95 = 0x0;
    zTcamConfigEntryFirst.m_uPacket96_127 = (tpid & 0xffff) << 16;
    zTcamConfigEntryFirst.m_uMask0_31= 0x0;
    zTcamConfigEntryFirst.m_uMask32_63= 0x0;
    zTcamConfigEntryFirst.m_uMask64_95 = 0x0;
    zTcamConfigEntryFirst.m_uMask96_127 = 0xffff0000;

    /* use port-based default queues */
    zNonLastEntryRam.m_uNextState = G2P3_CC_TCAM_STATE_TPID_MATCH;
    zNonLastEntryRam.m_uShift = 14;

    sbZfC2PrCcTcamConfigEntryFirst_Pack(&zTcamConfigEntryFirst, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
    sbZfC2PrCcCamRamNonLastEntry_Pack(&zNonLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

    if (C2PrCcCamRuleWrite(sbx->sbhdl, instance, G2P3_CC_VLAN_TPID_INDEX + tpid_index, uCfgBuffer, uCfgRamBuffer, 1) != TRUE) {
        return SB_FAILED;
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cc_tpid_get(int unit,
                         uint32_t instance,
                         uint32_t tpid_index,
                         uint32_t *tpid)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbZfC2PrCcCamRamNonLastEntry_t zNonLastEntryRam;
    sbZfC2PrCcTcamConfigEntryFirst_t zTcamConfigEntryFirst;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (tpid_index >= G2P3_CC_VLAN_TPID_ENTRIES) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamNonLastEntry_InitInstance(&zNonLastEntryRam);
    sbZfC2PrCcTcamConfigEntryFirst_InitInstance(&zTcamConfigEntryFirst);

    if (C2PrCcCamRuleRead(sbx->sbhdl, instance, G2P3_CC_VLAN_TPID_INDEX + tpid_index, uCfgBuffer, uCfgRamBuffer) != TRUE) {
        return SOC_E_FAIL;
    }

    sbZfC2PrCcTcamConfigEntryFirst_Unpack(&zTcamConfigEntryFirst, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
    sbZfC2PrCcCamRamNonLastEntry_Unpack(&zNonLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

    if (zTcamConfigEntryFirst.m_uValid != SB_FE2000XT_PR_CC_KEY_VALID) {
        return SOC_E_FAIL;
    }

    *tpid = (zTcamConfigEntryFirst.m_uPacket96_127 >> 16) && 0xffff;

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cc_primap_set(int unit,
                           uint32_t instance,
                           uint32_t portmask,
                           uint32_t pri,
                           g2p3_cc_de_t de)
{
    /* load pri rule, update DEs for selected ports, write back */
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    uint32_t port;
    sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
    sbZfC2PrCcTcamConfigEntryNonFirst_t zTcamConfigEntryNonFirst;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (portmask >= (1 << G2P3_CC_MAX_PORTS)) {
        return SOC_E_PARAM;
    }
    if (pri > 7) {
        return SOC_E_PARAM;
    }
    if (de > 3) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
    sbZfC2PrCcTcamConfigEntryNonFirst_InitInstance(&zTcamConfigEntryNonFirst);

    if (C2PrCcCamRuleRead(sbx->sbhdl, instance, G2P3_CC_VLAN_PRIMAP_INDEX + pri , uCfgBuffer, uCfgRamBuffer) != TRUE) {
        return SOC_E_FAIL;
    }

    sbZfC2PrCcTcamConfigEntryNonFirst_Unpack(&zTcamConfigEntryNonFirst, (uint8_t *) &uCfgBuffer[0],
                             SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
    sbZfC2PrCcCamRamLastEntry_Unpack(&zLastEntryRam, (uint8_t *) &uCfgRamBuffer[0],
                                SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);

    zTcamConfigEntryNonFirst.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;
        
    /* match on state and pri */
    zTcamConfigEntryNonFirst.m_uPacket0_31 = (pri & 0x7) << 29;
    zTcamConfigEntryNonFirst.m_uMask0_31= 7 << 29;
    zTcamConfigEntryNonFirst.m_uState = G2P3_CC_TCAM_STATE_TPID_MATCH;

    /* use port-based default queues */
    zLastEntryRam.m_uQueueAction = SB_ZF_C2PRCCCAMRAMLASTENTRY_QUEUE_ACT_DEFAULT_ONLY;
    zLastEntryRam.m_uQueue = 0;
    zLastEntryRam.m_uDrop = (de == G2P3_CC_DE_DROP) ? 1 : 0;
    zLastEntryRam.m_uLast = 1;

    /* loop over all ports and set de for each port */
    for (port = 0; port < G2P3_CC_MAX_PORTS; port++) {
        if ( (1 << port) & portmask) {
            switch (port) {
            case 0: zLastEntryRam.m_uDEPort0 = de; break;
            case 1: zLastEntryRam.m_uDEPort1 = de; break;
            case 2: zLastEntryRam.m_uDEPort2 = de; break;
            case 3: zLastEntryRam.m_uDEPort3 = de; break;
            case 4: zLastEntryRam.m_uDEPort4 = de; break;
            case 5: zLastEntryRam.m_uDEPort5 = de; break;
            case 6: zLastEntryRam.m_uDEPort6 = de; break;
            case 7: zLastEntryRam.m_uDEPort7 = de; break;
            case 8: zLastEntryRam.m_uDEPort8 = de; break;
            case 9: zLastEntryRam.m_uDEPort9 = de; break;
            case 10: zLastEntryRam.m_uDEPort10 = de; break;
            case 11: zLastEntryRam.m_uDEPort11 = de; break;
            default:
              return SOC_E_PARAM;
            }
        }
    }

    sbZfC2PrCcTcamConfigEntryNonFirst_Pack(&zTcamConfigEntryNonFirst, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
    sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

    if (C2PrCcCamRuleWrite(sbx->sbhdl, instance, G2P3_CC_VLAN_PRIMAP_INDEX + pri, uCfgBuffer, uCfgRamBuffer, 1) != TRUE) {
        return SB_FAILED;
    }

    return SOC_E_NONE;
}
int
soc_sbx_g2p3_cc_primap_get(int unit,
                           uint32_t instance,
                           uint32_t port,
                           uint32_t pri,
                           g2p3_cc_de_t *de)
{
    /* load pri rule, update DEs for selected ports, write back */
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
    sbZfC2PrCcTcamConfigEntryNonFirst_t zTcamConfigEntryNonFirst;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (port >= (1 << G2P3_CC_MAX_PORTS)) {
        return SOC_E_PARAM;
    }
    if (pri > 7) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
    sbZfC2PrCcTcamConfigEntryNonFirst_InitInstance(&zTcamConfigEntryNonFirst);

    if (C2PrCcCamRuleRead(sbx->sbhdl, instance, G2P3_CC_VLAN_PRIMAP_INDEX + pri , uCfgBuffer, uCfgRamBuffer) != TRUE) {
        return SOC_E_FAIL;
    }

    sbZfC2PrCcTcamConfigEntryNonFirst_Unpack(&zTcamConfigEntryNonFirst, (uint8_t *) &uCfgBuffer[0],
                             SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
    sbZfC2PrCcCamRamLastEntry_Unpack(&zLastEntryRam, (uint8_t *) &uCfgRamBuffer[0],
                                SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);

    zTcamConfigEntryNonFirst.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;
        
    switch (port) {
    case 0: *de = zLastEntryRam.m_uDEPort0; break;
    case 1: *de = zLastEntryRam.m_uDEPort1; break;
    case 2: *de = zLastEntryRam.m_uDEPort2; break;
    case 3: *de = zLastEntryRam.m_uDEPort3; break;
    case 4: *de = zLastEntryRam.m_uDEPort4; break;
    case 5: *de = zLastEntryRam.m_uDEPort5; break;
    case 6: *de = zLastEntryRam.m_uDEPort6; break;
    case 7: *de = zLastEntryRam.m_uDEPort7; break;
    case 8: *de = zLastEntryRam.m_uDEPort8; break;
    case 9: *de = zLastEntryRam.m_uDEPort9; break;
    case 10: *de = zLastEntryRam.m_uDEPort10; break;
    case 11: *de = zLastEntryRam.m_uDEPort11; break;
    default:
      return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cc_port_default_de_set(int unit,
                             uint32_t instance,
                             uint32_t portmask,
                             g2p3_cc_de_t de)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    uint32_t port;
    sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
    sbZfC2PrCcTcamConfigEntry_t zTcamConfigEntry;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (portmask >= (1 << G2P3_CC_MAX_PORTS)) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
    sbZfC2PrCcTcamConfigEntry_InitInstance(&zTcamConfigEntry);

    if (C2PrCcCamRuleRead(sbx->sbhdl, instance, G2P3_CC_PORT_DEFAULT_INDEX, uCfgBuffer, uCfgRamBuffer) != TRUE) {
        return SOC_E_FAIL;
    }

    sbZfC2PrCcTcamConfigEntry_Unpack(&zTcamConfigEntry, (uint8_t *) &uCfgBuffer[0],
                             SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
    sbZfC2PrCcCamRamLastEntry_Unpack(&zLastEntryRam, (uint8_t *) &uCfgRamBuffer[0],
                                SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);

    zTcamConfigEntry.m_uValid = SB_FE2000XT_PR_CC_KEY_VALID;

    /* use port-based default queues */
    zLastEntryRam.m_uQueueAction = SB_ZF_C2PRCCCAMRAMLASTENTRY_QUEUE_ACT_DEFAULT_ONLY;
    zLastEntryRam.m_uQueue = 0;
    zLastEntryRam.m_uDrop = (de == G2P3_CC_DE_DROP) ? 1 : 0;
    zLastEntryRam.m_uLast = 1;

    /* loop over all ports and set de for each port */
    for (port = 0; port < G2P3_CC_MAX_PORTS; port++) {
        if ( (1 << port) & portmask) {
            switch (port) {
            case 0: zLastEntryRam.m_uDEPort0 = de; break;
            case 1: zLastEntryRam.m_uDEPort1 = de; break;
            case 2: zLastEntryRam.m_uDEPort2 = de; break;
            case 3: zLastEntryRam.m_uDEPort3 = de; break;
            case 4: zLastEntryRam.m_uDEPort4 = de; break;
            case 5: zLastEntryRam.m_uDEPort5 = de; break;
            case 6: zLastEntryRam.m_uDEPort6 = de; break;
            case 7: zLastEntryRam.m_uDEPort7 = de; break;
            case 8: zLastEntryRam.m_uDEPort8 = de; break;
            case 9: zLastEntryRam.m_uDEPort9 = de; break;
            case 10: zLastEntryRam.m_uDEPort10 = de; break;
            case 11: zLastEntryRam.m_uDEPort11 = de; break;
            default:
              return SOC_E_PARAM;
            }
        }
    }

    sbZfC2PrCcTcamConfigEntry_Pack(&zTcamConfigEntry, (uint8_t*)uCfgBuffer,
                           SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_BYTES);
    sbZfC2PrCcCamRamLastEntry_Pack(&zLastEntryRam, (uint8_t*)uCfgRamBuffer,
                              SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES);

    if (C2PrCcCamRuleWrite(sbx->sbhdl, instance, G2P3_CC_PORT_DEFAULT_INDEX, uCfgBuffer, uCfgRamBuffer, 1) != TRUE) {
        return SB_FAILED;
    }

    return SOC_E_NONE;
}

int
soc_sbx_g2p3_cc_port_default_de_get(int unit,
                             uint32_t instance,
                             uint32_t port,
                             g2p3_cc_de_t *de)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbZfC2PrCcCamRamLastEntry_t zLastEntryRam;
    sbZfC2PrCcTcamConfigEntry_t zTcamConfigEntry;
    uint32_t uCfgBuffer[SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS];
    uint32_t uCfgRamBuffer[SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS];

    if (port >= (1 << G2P3_CC_MAX_PORTS)) {
        return SOC_E_PARAM;
    }

    sbZfC2PrCcCamRamLastEntry_InitInstance(&zLastEntryRam);
    sbZfC2PrCcTcamConfigEntry_InitInstance(&zTcamConfigEntry);

    if (C2PrCcCamRuleRead(sbx->sbhdl, instance, G2P3_CC_PORT_DEFAULT_INDEX, uCfgBuffer, uCfgRamBuffer) != TRUE) {
        return SOC_E_FAIL;
    }

    sbZfC2PrCcTcamConfigEntry_Unpack(&zTcamConfigEntry, (uint8_t *) &uCfgBuffer[0],
                             SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
    sbZfC2PrCcCamRamLastEntry_Unpack(&zLastEntryRam, (uint8_t *) &uCfgRamBuffer[0],
                                SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);

    if (zTcamConfigEntry.m_uValid != SB_FE2000XT_PR_CC_KEY_VALID) {
        return SOC_E_FAIL;
    }

    switch (port) {
    case 0: *de = zLastEntryRam.m_uDEPort0; break;
    case 1: *de = zLastEntryRam.m_uDEPort1; break;
    case 2: *de = zLastEntryRam.m_uDEPort2; break;
    case 3: *de = zLastEntryRam.m_uDEPort3; break;
    case 4: *de = zLastEntryRam.m_uDEPort4; break;
    case 5: *de = zLastEntryRam.m_uDEPort5; break;
    case 6: *de = zLastEntryRam.m_uDEPort6; break;
    case 7: *de = zLastEntryRam.m_uDEPort7; break;
    case 8: *de = zLastEntryRam.m_uDEPort8; break;
    case 9: *de = zLastEntryRam.m_uDEPort9; break;
    case 10: *de = zLastEntryRam.m_uDEPort10; break;
    case 11: *de = zLastEntryRam.m_uDEPort11; break;
    default:
      return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}
