/*
 * $Id: mem.c 1.112.2.14 Broadcom SDK $
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
 * SOC Memory (Table) Utilities
 */

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>

#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/register.h>
#include <soc/drv.h>
#include <soc/enet.h>
#include <soc/hash.h>
#include <soc/l2x.h>
#include <soc/l3x.h>
#include <soc/lpm.h>
#include <soc/vlan.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#ifdef BCM_SBUSDMA_SUPPORT
#include <soc/sbusdma.h>
#endif
#if defined(BCM_DRACO_SUPPORT) 
#include <soc/draco.h>
#endif
#if defined(BCM_HERCULES_SUPPORT)
#include <soc/hercules.h>
#endif
#if defined(BCM_TUCANA_SUPPORT)
#include <soc/tucana.h>
#endif
#if defined(BCM_EASYRIDER_SUPPORT)
#include <soc/er_cmdmem.h>
#include <soc/easyrider.h>
#endif
#if defined(BCM_TRIUMPH_SUPPORT)
#include <soc/triumph.h>
#include <soc/er_tcam.h>
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <soc/triumph2.h>
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
#include <soc/trident.h>
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_KATANA_SUPPORT) && defined(BCM_CMICM_SUPPORT)
#include <soc/katana.h>
#endif /* BCM_KATANA_SUPPORT */

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)

/* extra delay for quickturn */
uint32 soc_mem_fifo_delay_value = 15 * MILLISECOND_USEC;

/*
 * Empty (null) table entries
 */
uint32	_soc_mem_entry_null_zeroes[SOC_MAX_MEM_WORDS];
        /* First word f's, rest 0's */
uint32	_soc_mem_entry_null_word0[SOC_MAX_MEM_WORDS] = { ~0 };

#define SOC_MEM_COMPARE_RETURN(a, b) {		\
        if ((a) < (b)) { return -1; }		\
        if ((a) > (b)) { return  1; }		\
}

#ifdef BCM_FILTER_SUPPORT
int
_soc_mem_cmp_rule(int unit, void *ent_a, void *ent_b)
{
    uint32 filt_a[SOC_MAX_MEM_FIELD_WORDS], filt_b[SOC_MAX_MEM_FIELD_WORDS];
    uint32 fsel_a, fsel_b;
    uint32 eport_a, eport_b;
    int i, irule_words;
    int extended = soc_feature(unit, soc_feature_filter_extended);
    int xgs = soc_feature(unit, soc_feature_filter_xgs);
    int	rulemem;

    /*
     * This needs to be done based on the block that that is being
     * referenced, but we don't have that info at hand.
     */
    if (NUM_FE_PORT(unit) > 0) {
        rulemem = FILTER_IRULEm;
    } else {
        rulemem = GFILTER_IRULEm;
    }

    fsel_a = soc_mem_field32_get(unit, rulemem, ent_a, FSELf);
    fsel_b = soc_mem_field32_get(unit, rulemem, ent_b, FSELf);
    SOC_MEM_COMPARE_RETURN(fsel_a, fsel_b);

    if (extended || xgs) {
        uint32 egrsmod_a, egrsmod_b;

        egrsmod_a = soc_mem_field32_get(unit, rulemem, ent_a, EGRSMODf);
        egrsmod_b = soc_mem_field32_get(unit, rulemem, ent_b, EGRSMODf);
        SOC_MEM_COMPARE_RETURN(egrsmod_a, egrsmod_b);
    }

    if (!xgs) {
        uint32 iport_a, iport_b;

        iport_a = soc_mem_field32_get(unit, rulemem, ent_a, IPORTf);
        iport_b = soc_mem_field32_get(unit, rulemem, ent_b, IPORTf);
        SOC_MEM_COMPARE_RETURN(iport_a, iport_b);
    }

    eport_a = soc_mem_field32_get(unit, rulemem, ent_a, EPORTf);
    eport_b = soc_mem_field32_get(unit, rulemem, ent_b, EPORTf);
    SOC_MEM_COMPARE_RETURN(eport_a, eport_b);

#if defined(BCM_XGS_SWITCH_SUPPORT)
    if (extended || xgs) {
        uint32 pktformat_a, pktformat_b;

        pktformat_a = soc_mem_field32_get(unit, rulemem, ent_a, PKTFORMATf);
        pktformat_b = soc_mem_field32_get(unit, rulemem, ent_b, PKTFORMATf);
        SOC_MEM_COMPARE_RETURN(pktformat_a, pktformat_b);
    }
#endif /* BCM_XGS_SWITCH_SUPPORT */

    soc_mem_field_get(unit, rulemem, ent_a, FILTERf, filt_a);
    soc_mem_field_get(unit, rulemem, ent_b, FILTERf, filt_b);
    irule_words = soc_mem_field_length(unit, rulemem, FILTERf) / 32;

    for (i = irule_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(filt_a[i], filt_b[i]);
    }

    return 0;
}

#if defined(BCM_5665)

int
_soc_mem_cmp_rule_5665(int unit, void *ent_a, void *ent_b)
{
    uint32 filt_a[SOC_MAX_MEM_FIELD_WORDS], filt_b[SOC_MAX_MEM_FIELD_WORDS];
    int i, irule_words;
    int	rulemem;
    uint32 val_a, val_b;

    rulemem = FILTER_IRULEm;

    val_a = soc_mem_field32_get(unit, rulemem, ent_a, FSELf);
    val_b = soc_mem_field32_get(unit, rulemem, ent_b, FSELf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    if (!soc_mem_field32_get(unit, rulemem, ent_a, IPORT_MODEf)) {
        /* Non-bitmap mode; check IPORT */
        val_a = soc_mem_field32_get(unit, rulemem, ent_a, IPORTf);
        val_b = soc_mem_field32_get(unit, rulemem, ent_b, IPORTf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);
    } /* Else, match in a group */

    val_a = soc_mem_field32_get(unit, rulemem, ent_a, EGRSMODf);
    val_b = soc_mem_field32_get(unit, rulemem, ent_b, EGRSMODf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_mem_field32_get(unit, rulemem, ent_a, EPORTf);
    val_b = soc_mem_field32_get(unit, rulemem, ent_b, EPORTf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_mem_field32_get(unit, rulemem, ent_a, PKTFORMATf);
    val_b = soc_mem_field32_get(unit, rulemem, ent_b, PKTFORMATf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    soc_mem_field_get(unit, rulemem, ent_a, FILTERf, filt_a);
    soc_mem_field_get(unit, rulemem, ent_b, FILTERf, filt_b);
    irule_words = soc_mem_field_length(unit, rulemem, FILTERf) / 32;

    for (i = irule_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(filt_a[i], filt_b[i]);
    }

    return 0;
}
#endif
#endif /* BCM_FILTER_SUPPORT */

int
_soc_mem_cmp_word0(int unit, void *ent_a, void *ent_b)
{
    COMPILER_REFERENCE(unit);

    /*
     * Good for l3_def_ip_entry_t and l3_l3_entry_t.
     * The first uint32 (IP address) is the sort key.
     */

    SOC_MEM_COMPARE_RETURN(*(uint32 *)ent_a, *(uint32 *)ent_b);

    return 0;
}

int
_soc_mem_cmp_undef(int unit, void *ent_a, void *ent_b)
{
    COMPILER_REFERENCE(ent_a);
    COMPILER_REFERENCE(ent_b);

    soc_cm_print("soc_mem_cmp: cannot compare entries of this type\n");

    assert(0);

    return 0;
}

#ifdef BCM_XGS_SWITCH_SUPPORT

int
_soc_mem_cmp_l2x(int unit, void *ent_a, void *ent_b)
{
    sal_mac_addr_t mac_a, mac_b;
    vlan_id_t vlan_a, vlan_b;

    vlan_a = soc_L2Xm_field32_get(unit, ent_a, VLAN_IDf);
    vlan_b = soc_L2Xm_field32_get(unit, ent_b, VLAN_IDf);
    SOC_MEM_COMPARE_RETURN(vlan_a, vlan_b);

    soc_L2Xm_mac_addr_get(unit, ent_a, MAC_ADDRf, mac_a);
    soc_L2Xm_mac_addr_get(unit, ent_b, MAC_ADDRf, mac_b);

    return ENET_CMP_MACADDR(mac_a, mac_b);
}

int
_soc_mem_cmp_l2x_sync(int unit, void *ent_a, void *ent_b)
{
    l2x_entry_t     *l2x1 = (l2x_entry_t *)ent_a;
    l2x_entry_t     *l2x2 = (l2x_entry_t *)ent_b;

    /* force bits that may be updated by HW to be the same */
    if (SOC_IS_XGS12_SWITCH(unit) || SOC_IS_XGS12_FABRIC(unit)) {
        soc_L2Xm_field32_set(unit, l2x1, HIT_BITf, 0);
        soc_L2Xm_field32_set(unit, l2x2, HIT_BITf, 0);
    }

    if (SOC_IS_XGS3_SWITCH(unit) || SOC_IS_XGS3_FABRIC(unit)) {
        if (SOC_IS_EASYRIDER(unit)) {
                soc_mem_field32_set(unit, L2_ENTRY_INTERNALm, l2x1, TIMESTAMPf, 0);
                soc_mem_field32_set(unit, L2_ENTRY_INTERNALm, l2x2, TIMESTAMPf, 0);			
        } else {
            soc_L2Xm_field32_set(unit, l2x1, HITSAf, 0);
            soc_L2Xm_field32_set(unit, l2x2, HITSAf, 0);
            soc_L2Xm_field32_set(unit, l2x1, HITDAf, 0);
            soc_L2Xm_field32_set(unit, l2x2, HITDAf, 0);
            if (SOC_MEM_FIELD_VALID(unit, L2Xm, LOCAL_SAf)) {
                soc_L2Xm_field32_set(unit, l2x1, LOCAL_SAf, 0);
                soc_L2Xm_field32_set(unit, l2x2, LOCAL_SAf, 0);
            }
            if (SOC_MEM_FIELD_VALID(unit, L2Xm, EVEN_PARITYf)) {
                soc_L2Xm_field32_set(unit, l2x1, EVEN_PARITYf, 0);
                soc_L2Xm_field32_set(unit, l2x2, EVEN_PARITYf, 0);
            }
            if (SOC_MEM_FIELD_VALID(unit, L2Xm, ODD_PARITYf)) {
                soc_L2Xm_field32_set(unit, l2x1, ODD_PARITYf, 0);
                soc_L2Xm_field32_set(unit, l2x2, ODD_PARITYf, 0);
            }
        }
    }

    return sal_memcmp(ent_a, ent_b, sizeof(l2x_entry_t));
}


#ifdef BCM_FIREBOLT_SUPPORT
int
_soc_mem_cmp_l2x2(int unit, void *ent_a, void *ent_b)
{
    sal_mac_addr_t mac_a, mac_b;
    uint32 val_a, val_b;

    if (SOC_MEM_FIELD_VALID(unit, L2Xm, ENTRY_TYPEf)) {
        val_a = soc_L2Xm_field32_get(unit, ent_a, KEY_TYPEf);
        val_b = soc_L2Xm_field32_get(unit, ent_b, KEY_TYPEf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        switch (val_a) {
        case 0: /* BRIDGE */
            val_a = soc_L2Xm_field32_get(unit, ent_a, VLAN_IDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, VLAN_IDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            soc_L2Xm_mac_addr_get(unit, ent_a, MAC_ADDRf, mac_a);
            soc_L2Xm_mac_addr_get(unit, ent_b, MAC_ADDRf, mac_b);
            return ENET_CMP_MACADDR(mac_a, mac_b);

        case 1: /* SINGLE_CROSS_CONNECT */
            val_a = soc_L2Xm_field32_get(unit, ent_a, OVIDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, OVIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;
        case 2: /* DOUBLE_CROSS_CONNECT */
            val_a = soc_L2Xm_field32_get(unit, ent_a, OVIDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, OVIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_L2Xm_field32_get(unit, ent_a, IVIDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, IVIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;

        case 3: /* VFI */
            val_a = soc_L2Xm_field32_get(unit, ent_a, VFIf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, VFIf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            soc_L2Xm_mac_addr_get(unit, ent_a, MAC_ADDRf, mac_a);
            soc_L2Xm_mac_addr_get(unit, ent_b, MAC_ADDRf, mac_b);
            return ENET_CMP_MACADDR(mac_a, mac_b);

        case 4: /* VIF */
            val_a = soc_L2Xm_field32_get(unit, ent_a, VIF__NAMESPACEf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, VIF__NAMESPACEf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_L2Xm_field32_get(unit, ent_a, VIF__DST_VIFf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, VIF__DST_VIFf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_L2Xm_field32_get(unit, ent_a, VIF__Pf);
            val_b = soc_L2Xm_field32_get(unit, ent_b, VIF__Pf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;

        case 5: /* TRILL_NONUC_ACCESS */
            val_a = soc_L2Xm_field32_get(unit, ent_a,
                                         TRILL_NONUC_ACCESS__VLAN_IDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b,
                                         TRILL_NONUC_ACCESS__VLAN_IDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            soc_L2Xm_mac_addr_get(unit, ent_a, TRILL_NONUC_ACCESS__MAC_ADDRf,
                                  mac_a);
            soc_L2Xm_mac_addr_get(unit, ent_b, TRILL_NONUC_ACCESS__MAC_ADDRf,
                                  mac_b);
            return ENET_CMP_MACADDR(mac_a, mac_b);

        case 6: /* TRILL_NONUC_NETWORK_LONG */
            val_a = soc_L2Xm_field32_get(unit, ent_a,
                                         TRILL_NONUC_NETWORK_LONG__VLAN_IDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b,
                                         TRILL_NONUC_NETWORK_LONG__VLAN_IDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_L2Xm_field32_get(unit, ent_a,
                                         TRILL_NONUC_NETWORK_LONG__TREE_IDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b,
                                         TRILL_NONUC_NETWORK_LONG__TREE_IDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            soc_L2Xm_mac_addr_get
                (unit, ent_a, TRILL_NONUC_NETWORK_LONG__MAC_ADDRESSf, mac_a);
            soc_L2Xm_mac_addr_get
                (unit, ent_b, TRILL_NONUC_NETWORK_LONG__MAC_ADDRESSf, mac_b);
            return ENET_CMP_MACADDR(mac_a, mac_b);

        case 7: /* TRILL_NONUC_NETWORK_SHORT */
            val_a = soc_L2Xm_field32_get(unit, ent_a,
                                         TRILL_NONUC_NETWORK_SHORT__VLAN_IDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b,
                                         TRILL_NONUC_NETWORK_SHORT__VLAN_IDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_L2Xm_field32_get(unit, ent_a,
                                         TRILL_NONUC_NETWORK_SHORT__TREE_IDf);
            val_b = soc_L2Xm_field32_get(unit, ent_b,
                                         TRILL_NONUC_NETWORK_SHORT__TREE_IDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;
        default:
            return 1;
        }
    }

    val_a = soc_L2Xm_field32_get(unit, ent_a, VLAN_IDf);
    val_b = soc_L2Xm_field32_get(unit, ent_b, VLAN_IDf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    soc_L2Xm_mac_addr_get(unit, ent_a, MAC_ADDRf, mac_a);
    soc_L2Xm_mac_addr_get(unit, ent_b, MAC_ADDRf, mac_b);
    return ENET_CMP_MACADDR(mac_a, mac_b);
}

int
_soc_mem_cmp_l3x2(int unit, void *ent_a, void *ent_b)
{
    uint32 val_a, val_b;

    if (!SOC_MEM_FIELD_VALID(unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {
        return 0;
    }

    val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, KEY_TYPEf);
    val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, KEY_TYPEf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    switch (val_a) {
    case 0: /* IPV4_UNICAST */
        return _soc_mem_cmp_l3x2_ip4ucast(unit, ent_a, ent_b);
    case 1: /* IPV4_MULTIICAST */
        return _soc_mem_cmp_l3x2_ip4mcast(unit, ent_a, ent_b); 
    case 2: /* IPV6_UNICAST */
        return _soc_mem_cmp_l3x2_ip6ucast(unit, ent_a, ent_b);
    case 3: /* IPV6_MULTIICAST */
        return _soc_mem_cmp_l3x2_ip6mcast(unit, ent_a, ent_b);
    case 4: /* CCM_LMEP */
        val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, LMEP__SGLPf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, LMEP__SGLPf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, LMEP__VIDf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, LMEP__VIDf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        return 0;
    case 5: /* CCM_RMEP */
        val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, RMEP__SGLPf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, RMEP__SGLPf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, RMEP__VIDf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, RMEP__VIDf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, RMEP__MDLf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, RMEP__MDLf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, RMEP__MEPIDf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, RMEP__MEPIDf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        return 0;
    case 6: /* TRILL */
        val_a = soc_L3_ENTRY_ONLYm_field32_get
            (unit, ent_a, TRILL__INGRESS_RBRIDGE_NICKNAMEf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get
            (unit, ent_b, TRILL__INGRESS_RBRIDGE_NICKNAMEf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        val_a = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_a, TRILL__TREE_IDf);
        val_b = soc_L3_ENTRY_ONLYm_field32_get(unit, ent_b, TRILL__TREE_IDf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        return 0;
    default:
        return 1;
    }
}

int
_soc_mem_cmp_l3x2_ip4ucast(int unit, void *ent_a, void *ent_b)
{
    uint32      type_a, type_b;
    ip_addr_t	ip_a, ip_b;

    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV4_UNICASTm, VRF_IDf)) {
        type_a = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_a, VRF_IDf);
        type_b = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_b, VRF_IDf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV4_UNICASTm, KEY_TYPEf)) {
        type_a = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_a, KEY_TYPEf);
        type_b = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_b, KEY_TYPEf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else {
        type_a = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_a, V6f);
        type_b = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_b, V6f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_a, IPMCf);
        type_b = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_b, IPMCf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    ip_a = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_a, IP_ADDRf);
    ip_b = soc_L3_ENTRY_IPV4_UNICASTm_field32_get(unit, ent_b, IP_ADDRf);
    SOC_MEM_COMPARE_RETURN(ip_a, ip_b);

    return(0);
}

int
_soc_mem_cmp_l3x2_ip4mcast(int unit, void *ent_a, void *ent_b)
{
    uint32      type_a, type_b;
    ip_addr_t	a, b;
    vlan_id_t	vlan_a, vlan_b;

    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV4_MULTICASTm, VRF_IDf)) {
        type_a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, VRF_IDf);
        type_b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, VRF_IDf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }
#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV4_MULTICASTm, KEY_TYPE_0f)) {
        type_a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, KEY_TYPE_0f);
        type_b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, KEY_TYPE_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, KEY_TYPE_1f);
        type_b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, KEY_TYPE_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
        type_a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, V6f);
        type_b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, V6f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, IPMCf);
        type_b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, IPMCf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, SOURCE_IP_ADDRf);
    b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, SOURCE_IP_ADDRf);
    SOC_MEM_COMPARE_RETURN(a, b);

    a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, GROUP_IP_ADDRf);
    b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, GROUP_IP_ADDRf);
    SOC_MEM_COMPARE_RETURN(a, b);

    vlan_a = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_a, VLAN_IDf);
    vlan_b = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, ent_b, VLAN_IDf);
    SOC_MEM_COMPARE_RETURN(vlan_a, vlan_b);

    return(0);
}

int
_soc_mem_cmp_l3x2_ip6ucast(int unit, void *ent_a, void *ent_b)
{
    uint32      type_a, type_b;
    uint32      a[SOC_MAX_MEM_FIELD_WORDS];
    uint32      b[SOC_MAX_MEM_FIELD_WORDS];
    int         i;
    int         ent_words;

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV6_UNICASTm, VRF_IDf)) {
        type_a =
            soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, VRF_IDf);
        type_b =
            soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, VRF_IDf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV6_UNICASTm, VRF_ID_0f)) {
        type_a =
            soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, VRF_ID_0f);
        type_b =
            soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, VRF_ID_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }
#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV6_UNICASTm, KEY_TYPE_0f)) {
        type_a = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, KEY_TYPE_0f);
        type_b = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, KEY_TYPE_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
        type_a = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, V6_0f);
        type_b = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, V6_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, IPMC_0f);
        type_b = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, IPMC_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV6_UNICASTm, VRF_ID_1f)) {
        type_a =
            soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, VRF_ID_1f);
        type_b =
            soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, VRF_ID_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_IPV6_UNICASTm, KEY_TYPE_1f)) {
        type_a = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, KEY_TYPE_1f);
        type_b = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, KEY_TYPE_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
        type_a = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, V6_1f);
        type_b = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, V6_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_a, IPMC_1f);
        type_b = soc_L3_ENTRY_IPV6_UNICASTm_field32_get(unit, ent_b, IPMC_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    soc_mem_field_get(unit, L3_ENTRY_IPV6_UNICASTm, ent_a, IP_ADDR_UPR_64f, a);
    soc_mem_field_get(unit, L3_ENTRY_IPV6_UNICASTm, ent_b, IP_ADDR_UPR_64f, b);
    ent_words = soc_mem_field_length(unit, L3_ENTRY_IPV6_UNICASTm,
                                     IP_ADDR_UPR_64f) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }
    soc_mem_field_get(unit, L3_ENTRY_IPV6_UNICASTm, ent_a, IP_ADDR_LWR_64f, a);
    soc_mem_field_get(unit, L3_ENTRY_IPV6_UNICASTm, ent_b, IP_ADDR_LWR_64f, b);
    ent_words = soc_mem_field_length(unit, L3_ENTRY_IPV6_UNICASTm,
                                     IP_ADDR_LWR_64f) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }

    return(0);
}

int
_soc_mem_cmp_l3x2_ip6mcast(int u, void *e_a, void *e_b)
{
    uint32      type_a, type_b;
    vlan_id_t	vlan_a, vlan_b;
    uint32      a[SOC_MAX_MEM_FIELD_WORDS];
    uint32      b[SOC_MAX_MEM_FIELD_WORDS];
    int         i;
    int         ent_words;

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, VRF_IDf)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, VRF_IDf);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, VRF_IDf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif
    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, VRF_ID_0f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, VRF_ID_0f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, VRF_ID_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }
#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, KEY_TYPE_0f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, KEY_TYPE_0f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, KEY_TYPE_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, V6_0f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, V6_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, IPMC_0f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, IPMC_0f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, VRF_ID_1f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, VRF_ID_1f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, VRF_ID_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, KEY_TYPE_1f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, KEY_TYPE_1f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, KEY_TYPE_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, V6_1f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, V6_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, IPMC_1f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, IPMC_1f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, VRF_ID_2f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, VRF_ID_2f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, VRF_ID_2f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, KEY_TYPE_2f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, KEY_TYPE_2f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, KEY_TYPE_2f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, V6_2f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, V6_2f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, IPMC_2f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, IPMC_2f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, VRF_ID_3f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, VRF_ID_3f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, VRF_ID_3f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

#if defined(BCM_TRX_SUPPORT)
    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, KEY_TYPE_3f)) {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, KEY_TYPE_3f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, KEY_TYPE_3f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    } else
#endif /* BCM_TRX_SUPPORT */
    {
        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, V6_3f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, V6_3f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        type_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, IPMC_3f);
        type_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, IPMC_3f);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);
    }

    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_a, SOURCE_IP_ADDR_UPR_64f, a);
    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_b, SOURCE_IP_ADDR_UPR_64f, b);
    ent_words = soc_mem_field_length(u, L3_ENTRY_IPV6_MULTICASTm,
                                     SOURCE_IP_ADDR_UPR_64f) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }

    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_a, SOURCE_IP_ADDR_LWR_64f, a);
    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_b, SOURCE_IP_ADDR_LWR_64f, b);
    ent_words = soc_mem_field_length(u, L3_ENTRY_IPV6_MULTICASTm,
                                     SOURCE_IP_ADDR_LWR_64f) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }

    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_a, GROUP_IP_ADDR_UPR_56f, a);
    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_b, GROUP_IP_ADDR_UPR_56f, b);
    ent_words = soc_mem_field_length(u, L3_ENTRY_IPV6_MULTICASTm,
                                     GROUP_IP_ADDR_UPR_56f) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }

    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_a, GROUP_IP_ADDR_LWR_64f, a);
    soc_mem_field_get(u, L3_ENTRY_IPV6_MULTICASTm,
                      e_b, GROUP_IP_ADDR_LWR_64f, b);
    ent_words = soc_mem_field_length(u, L3_ENTRY_IPV6_MULTICASTm,
                                     GROUP_IP_ADDR_LWR_64f) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_MEM_FIELD_VALID(u, L3_ENTRY_IPV6_MULTICASTm, VLAN_IDf)) {
        vlan_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, VLAN_IDf);
        vlan_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, VLAN_IDf);
        SOC_MEM_COMPARE_RETURN(vlan_a, vlan_b);
    } else
#endif
    {
        vlan_a = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_a, VLAN_ID_0f);
        vlan_b = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(u, e_b, VLAN_ID_0f);
        SOC_MEM_COMPARE_RETURN(vlan_a, vlan_b);
    }

    return(0);
}

/*
 * Compares both mask and address. Not a masked compare.
 */
int
_soc_mem_cmp_lpm(int u, void *e_a, void *e_b)
{
    uint32      a;
    uint32      b;

    a = soc_mem_field32_get(u, L3_DEFIPm, e_a, VALID1f);
    b = soc_mem_field32_get(u, L3_DEFIPm, e_b, VALID1f);

    if (a && b) {
        a = soc_mem_field32_get(u, L3_DEFIPm, e_a, MASK1f);
        b = soc_mem_field32_get(u, L3_DEFIPm, e_b, MASK1f);
        SOC_MEM_COMPARE_RETURN(a, b);

        a = soc_mem_field32_get(u, L3_DEFIPm, e_a, IP_ADDR1f);
        b = soc_mem_field32_get(u, L3_DEFIPm, e_b, IP_ADDR1f);
        SOC_MEM_COMPARE_RETURN(a, b);

        if (SOC_MEM_FIELD_VALID(u, L3_DEFIPm, VRF_ID_1f)) {
            a = soc_L3_DEFIPm_field32_get(u, e_a, VRF_ID_1f);
            b = soc_L3_DEFIPm_field32_get(u, e_b, VRF_ID_1f);
            SOC_MEM_COMPARE_RETURN(a, b);
        }

        a = soc_mem_field32_get(u, L3_DEFIPm, e_a, MODE1f);
        b = soc_mem_field32_get(u, L3_DEFIPm, e_b, MODE1f);
        SOC_MEM_COMPARE_RETURN(a, b);

        if (a == 0) {
            return(0); /* IPV4 entry */
        }
    }

    a = soc_mem_field32_get(u, L3_DEFIPm, e_a, VALID0f);
    b = soc_mem_field32_get(u, L3_DEFIPm, e_b, VALID0f);

    if (a && b) {
        a = soc_mem_field32_get(u, L3_DEFIPm, e_a, MASK0f);
        b = soc_mem_field32_get(u, L3_DEFIPm, e_b, MASK0f);
        SOC_MEM_COMPARE_RETURN(a, b);

        a = soc_mem_field32_get(u, L3_DEFIPm, e_a, IP_ADDR0f);
        b = soc_mem_field32_get(u, L3_DEFIPm, e_b, IP_ADDR0f);
        SOC_MEM_COMPARE_RETURN(a, b);

        if (SOC_MEM_FIELD_VALID(u, L3_DEFIPm, VRF_ID_0f)) {
            a = soc_L3_DEFIPm_field32_get(u, e_a, VRF_ID_0f);
            b = soc_L3_DEFIPm_field32_get(u, e_b, VRF_ID_0f);
            SOC_MEM_COMPARE_RETURN(a, b);
        }

        a = soc_mem_field32_get(u, L3_DEFIPm, e_a, MODE0f);
        b = soc_mem_field32_get(u, L3_DEFIPm, e_b, MODE0f);
        SOC_MEM_COMPARE_RETURN(a, b);
        if (a == 0) {
            return(0); /* IPV4 entry */
        }
    }

    a = soc_mem_field32_get(u, L3_DEFIPm, e_a, VALID1f);
    b = soc_mem_field32_get(u, L3_DEFIPm, e_b, VALID1f);
    SOC_MEM_COMPARE_RETURN(a, b);
    a = soc_mem_field32_get(u, L3_DEFIPm, e_a, VALID0f);
    b = soc_mem_field32_get(u, L3_DEFIPm, e_b, VALID0f);
    SOC_MEM_COMPARE_RETURN(a, b);

    return (0);
}

#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
int
_soc_mem_cmp_vlan_mac(int unit, void *ent_a, void *ent_b)
{
    sal_mac_addr_t mac_a, mac_b;

    soc_VLAN_MACm_mac_addr_get(unit, ent_a, MAC_ADDRf, mac_a);
    soc_VLAN_MACm_mac_addr_get(unit, ent_b, MAC_ADDRf, mac_b);

    return ENET_CMP_MACADDR(mac_a, mac_b);
}
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_TRX_SUPPORT
int
_soc_mem_cmp_vlan_mac_tr(int unit, void *ent_a, void *ent_b)
{
    sal_mac_addr_t mac_a, mac_b;
    uint32      type_a, type_b;

    type_a = soc_VLAN_MACm_field32_get(unit, ent_a, KEY_TYPEf);
    type_b = soc_VLAN_MACm_field32_get(unit, ent_b, KEY_TYPEf);
    SOC_MEM_COMPARE_RETURN(type_a, type_b);

    switch (type_a) {
    case 3: /* VLAN_MAC */
        soc_VLAN_MACm_mac_addr_get(unit, ent_a, MAC_ADDRf, mac_a);
        soc_VLAN_MACm_mac_addr_get(unit, ent_b, MAC_ADDRf, mac_b);
        return ENET_CMP_MACADDR(mac_a, mac_b);

#ifdef BCM_TRIUMPH2_SUPPORT
    case 7: /* HPAE (MAC_IP_BIND) */
        type_a = soc_VLAN_MACm_field32_get(unit, ent_a, MAC_IP_BIND__SIPf);
        type_b = soc_VLAN_MACm_field32_get(unit, ent_b, MAC_IP_BIND__SIPf);
        SOC_MEM_COMPARE_RETURN(type_a, type_b);

        return 0;
#endif /* BCM_TRIUMPH2_SUPPORT */

    default:
        return 1;
    }
}

int
_soc_mem_cmp_vlan_xlate_tr(int unit, void *ent_a, void *ent_b)
{
    uint32 val_a, val_b;

    val_a = soc_VLAN_XLATEm_field32_get(unit, ent_a, KEY_TYPEf);
    val_b = soc_VLAN_XLATEm_field32_get(unit, ent_b, KEY_TYPEf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    switch (val_a) {
    case 3: /* VLAN_MAC */
    case 7: /* HPAE (MAC_IP_BIND) */
        return _soc_mem_cmp_vlan_mac_tr(unit, ent_a, ent_b);

    case 0: /* IVID_OVID */
    case 1: /* OTAG */
    case 2: /* ITAG */
    case 4: /* OVID */
    case 5: /* IVID */
    case 6: /* PRI_CFI */
        val_a = soc_VLAN_XLATEm_field32_get(unit, ent_a, GLPf);
        val_b = soc_VLAN_XLATEm_field32_get(unit, ent_b, GLPf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        val_a = soc_VLAN_XLATEm_field32_get(unit, ent_a, INCOMING_VIDSf);
        val_b = soc_VLAN_XLATEm_field32_get(unit, ent_b, INCOMING_VIDSf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        return 0;

    case 9: /* VIF_VLAN */
        val_a = soc_VLAN_XLATEm_field32_get(unit, ent_a, VIF__VLANf);
        val_b = soc_VLAN_XLATEm_field32_get(unit, ent_b, VIF__VLANf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);
        /* fall through to case for VIF */

    case 8: /* VIF */
        val_a = soc_VLAN_XLATEm_field32_get(unit, ent_a, VIF__GLPf);
        val_b = soc_VLAN_XLATEm_field32_get(unit, ent_b, VIF__GLPf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        val_a = soc_VLAN_XLATEm_field32_get(unit, ent_a, VIF__SRC_VIFf);
        val_b = soc_VLAN_XLATEm_field32_get(unit, ent_b, VIF__SRC_VIFf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        return 0;

    default:
        return 1;
    }
}

int
_soc_mem_cmp_egr_vlan_xlate_tr(int unit, void *ent_a, void *ent_b)
{
    sal_mac_addr_t mac_a, mac_b;
    uint32 val_a, val_b;

    if (SOC_MEM_FIELD_VALID(unit, EGR_VLAN_XLATEm, ENTRY_TYPEf)) {
        val_a = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_a, ENTRY_TYPEf);
        val_b = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_b, ENTRY_TYPEf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        switch (val_a) {
        case 0: /* VLAN_XLATE */
        case 1: /* VLAN_XLATE_DVP */
        case 2: /* VLAN_XLATE_WLAN */
            if (SOC_IS_TD_TT(unit)) {
                val_a = soc_EGR_VLAN_XLATEm_field32_get
                    (unit, ent_a, DST_MODIDf);
                val_b = soc_EGR_VLAN_XLATEm_field32_get
                    (unit, ent_b, DST_MODIDf);
                SOC_MEM_COMPARE_RETURN(val_a, val_b);

                val_a = soc_EGR_VLAN_XLATEm_field32_get
                    (unit, ent_a, DST_PORTf);
                val_b = soc_EGR_VLAN_XLATEm_field32_get
                    (unit, ent_b, DST_PORTf);
                SOC_MEM_COMPARE_RETURN(val_a, val_b);
            } else if (SOC_MEM_FIELD_VALID(unit, EGR_VLAN_XLATEm, DVPf)) {
                val_a = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_a, DVPf);
                val_b = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_b, DVPf);
                SOC_MEM_COMPARE_RETURN(val_a, val_b);
            } else {
                val_a = soc_EGR_VLAN_XLATEm_field32_get
                    (unit, ent_a, PORT_GROUP_IDf);
                val_b = soc_EGR_VLAN_XLATEm_field32_get
                    (unit, ent_b, PORT_GROUP_IDf);
                SOC_MEM_COMPARE_RETURN(val_a, val_b);
            }

            val_a = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_a, OVIDf);
            val_b = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_b, OVIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_a, IVIDf);
            val_b = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_b, IVIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;

        case 3: /* ISID_XLATE */
        case 4: /* ISID_DVP_XLATE */
            val_a = soc_EGR_VLAN_XLATEm_field32_get
                (unit, ent_a, MIM_ISID__VFIf);
            val_b = soc_EGR_VLAN_XLATEm_field32_get
                (unit, ent_b, MIM_ISID__VFIf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_EGR_VLAN_XLATEm_field32_get
                (unit, ent_a, MIM_ISID__DVPf);
            val_b = soc_EGR_VLAN_XLATEm_field32_get
                (unit, ent_b, MIM_ISID__DVPf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;

        case 5: /* WLAN_SVP_TUNNEL */
        case 6: /* WLAN_SVP_BSSID */
        case 7: /* WLAN_SVP_BSSID_RID */
            val_a = soc_EGR_VLAN_XLATEm_field32_get
                (unit, ent_a, WLAN_SVP__RIDf);
            val_b = soc_EGR_VLAN_XLATEm_field32_get
                (unit, ent_b, WLAN_SVP__RIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            soc_mem_mac_addr_get
                (unit, EGR_VLAN_XLATEm, ent_a, WLAN_SVP__BSSIDf, mac_a);
            soc_mem_mac_addr_get
                (unit, EGR_VLAN_XLATEm, ent_b, WLAN_SVP__BSSIDf, mac_b);
            return ENET_CMP_MACADDR(mac_a, mac_b);

        default:
            return 1;
        }
    }

    val_a = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_a, PORT_GROUP_IDf);
    val_b = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_b, PORT_GROUP_IDf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_a, IVIDf);
    val_b = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_b, IVIDf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_a, OVIDf);
    val_b = soc_EGR_VLAN_XLATEm_field32_get(unit, ent_b, OVIDf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    return 0;
}

#ifdef BCM_TRIUMPH_SUPPORT
int
_soc_mem_cmp_mpls_entry_tr(int unit, void *ent_a, void *ent_b)
{
    sal_mac_addr_t mac_a, mac_b;
    uint32 val_a, val_b;

    if (SOC_MEM_FIELD_VALID(unit, MPLS_ENTRYm, ENTRY_TYPEf)) {
        val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, KEY_TYPEf);
        val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, KEY_TYPEf);
        SOC_MEM_COMPARE_RETURN(val_a, val_b);

        switch (val_a) {
        case 0: /* MPLS */
            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, PORT_NUMf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, PORT_NUMf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MODULE_IDf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MODULE_IDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, Tf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, Tf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MPLS_LABELf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MPLS_LABELf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;
        case 1: /* MIM_NVP */
            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MIM_NVP__BVIDf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MIM_NVP__BVIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            soc_mem_mac_addr_get
                (unit, MPLS_ENTRYm, ent_a, MIM_NVP__BMACSAf, mac_a);
            soc_mem_mac_addr_get
                (unit, MPLS_ENTRYm, ent_b, MIM_NVP__BMACSAf, mac_b);
            return ENET_CMP_MACADDR(mac_a, mac_b);

        case 2: /* MIM_ISID */
            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MIM_ISID__ISIDf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MIM_ISID__ISIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;

        case 3: /* MIM_ISID_SVP */
            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MIM_ISID__ISIDf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MIM_ISID__ISIDf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MIM_ISID__SVPf);
            val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MIM_ISID__SVPf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;

        case 4: /* WLAN_MAC */
            soc_mem_mac_addr_get
                (unit, MPLS_ENTRYm, ent_a, WLAN_MAC__MAC_ADDRf, mac_a);
            soc_mem_mac_addr_get
                (unit, MPLS_ENTRYm, ent_b, WLAN_MAC__MAC_ADDRf, mac_b);
            return ENET_CMP_MACADDR(mac_a, mac_b);

        case 5: /* TRILL */
            val_a = soc_MPLS_ENTRYm_field32_get
                (unit, ent_a, TRILL__RBRIDGE_NICKNAMEf);
            val_b = soc_MPLS_ENTRYm_field32_get
                (unit, ent_b, TRILL__RBRIDGE_NICKNAMEf);
            SOC_MEM_COMPARE_RETURN(val_a, val_b);

            return 0;

        default:
            return 1;
        }
    }

    val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, PORT_NUMf);
    val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, PORT_NUMf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MODULE_IDf);
    val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MODULE_IDf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, Tf);
    val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, Tf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    val_a = soc_MPLS_ENTRYm_field32_get(unit, ent_a, MPLS_LABELf);
    val_b = soc_MPLS_ENTRYm_field32_get(unit, ent_b, MPLS_LABELf);
    SOC_MEM_COMPARE_RETURN(val_a, val_b);

    return 0;
}
#endif /* BCM_TRIUMPH_SUPPORT */
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
int
_soc_mem_cmp_l2er(int unit, void *ent_a, void *ent_b)
{
    sal_mac_addr_t mac_a, mac_b;
    vlan_id_t vlan_a, vlan_b;

    vlan_a = soc_L2_ENTRY_INTERNALm_field32_get(unit, ent_a, VLAN_IDf);
    vlan_b = soc_L2_ENTRY_INTERNALm_field32_get(unit, ent_b, VLAN_IDf);
    SOC_MEM_COMPARE_RETURN(vlan_a, vlan_b);

    soc_L2_ENTRY_INTERNALm_mac_addr_get(unit, ent_a, MAC_ADDRf, mac_a);
    soc_L2_ENTRY_INTERNALm_mac_addr_get(unit, ent_b, MAC_ADDRf, mac_b);

    return ENET_CMP_MACADDR(mac_a, mac_b);
}

int
_soc_mem_cmp_l3v4er(int unit, void *ent_a, void *ent_b)
{
    ip_addr_t	ip_a, ip_b;
    uint32      vrf_a, vrf_b;

    vrf_a = soc_L3_ENTRY_V4m_field32_get(unit, ent_a, VRFf);
    vrf_b = soc_L3_ENTRY_V4m_field32_get(unit, ent_b, VRFf);
    SOC_MEM_COMPARE_RETURN(vrf_a, vrf_b);

    ip_a = soc_L3_ENTRY_V4m_field32_get(unit, ent_a, IP_ADDRf);
    ip_b = soc_L3_ENTRY_V4m_field32_get(unit, ent_b, IP_ADDRf);

    SOC_MEM_COMPARE_RETURN(ip_a, ip_b);

    return(0);
}

int
_soc_mem_cmp_l3v6er(int unit, void *ent_a, void *ent_b)
{
    uint32      a[SOC_MAX_MEM_FIELD_WORDS];
    uint32      b[SOC_MAX_MEM_FIELD_WORDS];
    int         i;
    int         ent_words;

    soc_mem_field_get(unit, L3_ENTRY_V6m, ent_a, IP_ADDRf, a);
    soc_mem_field_get(unit, L3_ENTRY_V6m, ent_b, IP_ADDRf, b);

    ent_words = soc_mem_field_length(unit, L3_ENTRY_V6m, IP_ADDRf) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }

    return(0);
}

int
_soc_mem_cmp_l3_defip_alg(int unit, void *ent_a, void *ent_b)
{
    uint32      a[SOC_MAX_MEM_FIELD_WORDS];
    uint32      b[SOC_MAX_MEM_FIELD_WORDS];
    uint32      pf_a, pf_b;
    int         i;
    int         ent_words;

    soc_mem_field_get(unit, L3_DEFIP_ALGm, ent_a, IP_ADDR_V6f, a);
    soc_mem_field_get(unit, L3_DEFIP_ALGm, ent_b, IP_ADDR_V6f, b);

    ent_words = soc_mem_field_length(unit, L3_DEFIP_ALGm, IP_ADDR_V6f) / 32;
    for (i = ent_words - 1; i >= 0; i--) {
        SOC_MEM_COMPARE_RETURN(a[i], b[i]);
    }

    pf_a = soc_L3_DEFIP_ALGm_field32_get(unit, ent_a, PREFIX_LENGTHf);
    pf_b = soc_L3_DEFIP_ALGm_field32_get(unit, ent_b, PREFIX_LENGTHf);

    SOC_MEM_COMPARE_RETURN(pf_a, pf_b);
    return(0);
}

#endif /* BCM_EASYRIDER_SUPPORT */

/*
 *  This function caches the info needed by the L3 key comparison, below.
 */
int
_soc_mem_cmp_l3x_sync(int unit)
{
    uint32		regval;
    uint32		ipmc_config;
    int			port;
#if defined(BCM_TUCANA_SUPPORT) || defined(BCM_5695)
    port_tab_entry_t	pte;
#endif /* BCM_TUCANA_SUPPORT || BCM_5695 */

    /* get the first enabled port */
    port = -1;
    PBMP_E_ITER(unit, port) {
        break;
    }
    ipmc_config = 0;

    if (port < 0) {
        /* no enable ports, leave everything disabled */
    } else if (SOC_IS_DRACO1(unit)) {
        SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &regval));
        if (soc_reg_field_get(unit, CONFIGr, regval, IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        if (soc_reg_field_get(unit, CONFIGr, regval, SRC_IP_CFGf)) {
            ipmc_config |= L3X_IPMC_SIP0;
        } else {
            ipmc_config |= L3X_IPMC_SIP;
        }
#if defined(BCM_5695)
    } else if (SOC_IS_DRACO15(unit)) {
        SOC_IF_ERROR_RETURN(READ_PORT_TABm(unit, MEM_BLOCK_ANY, port, &pte));
        if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        ipmc_config |= L3X_IPMC_SIP | L3X_IPMC_SIP0;
        if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_DO_VLANf)) {
            ipmc_config |= L3X_IPMC_VLAN;
        }
#endif
#if defined(BCM_LYNX_SUPPORT)
    } else if (SOC_IS_LYNX(unit)) {
        SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &regval));
        if (soc_reg_field_get(unit, CONFIGr, regval, IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));
        if (soc_reg_field_get(unit, ARL_CONTROLr, regval,
                              PIM_DM_SM_ENABLEf)) {
            ipmc_config |= L3X_IPMC_SIP | L3X_IPMC_SIP0;
        } else if (soc_reg_field_get(unit, ARL_CONTROLr, regval,
                                     SRC_IP_CFGf)) {
            ipmc_config |= L3X_IPMC_SIP0;
        } else {
            ipmc_config |= L3X_IPMC_SIP;
        }

        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            if (soc_reg_field_get(unit, ARL_CONTROLr, regval,
                                  IPMC_DO_VLANf)) {
                ipmc_config |= L3X_IPMC_VLAN;
            }
        }

#endif
#ifdef BCM_TUCANA_SUPPORT
    } else if (SOC_IS_TUCANA(unit)) {
        SOC_IF_ERROR_RETURN(READ_PORT_TAB_MOD_CHK(unit,
                                                  MEM_BLOCK_ANY, port, &pte));
        if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        ipmc_config |= L3X_IPMC_SIP | L3X_IPMC_SIP0;

        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            SOC_IF_ERROR_RETURN
                (READ_PORT_TABm(unit, MEM_BLOCK_ANY, port, &pte));
            if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_DO_VLANf)) {
                ipmc_config |= L3X_IPMC_VLAN;
            }
        }
#endif /* BCM_TUCANA_SUPPORT */
    }

    SOC_CONTROL(unit)->hash_key_config = ipmc_config;
    return SOC_E_NONE;
}

int
_soc_mem_cmp_l3x_set(int unit, uint32 ipmc_config)
{
    uint32		regval, oregval;
    int			port;
#if defined(BCM_TUCANA_SUPPORT) || defined(BCM_5695)
    port_tab_entry_t	pte;
#endif /* BCM_TUCANA_SUPPORT || BCM_5695 */

    if (SOC_IS_DRACO1(unit)) {
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &regval));
            oregval = regval;
            soc_reg_field_set(unit, CONFIGr, &regval, IPMC_ENABLEf,
                              (ipmc_config & L3X_IPMC_ENABLE) ? 1 : 0);
            soc_reg_field_set(unit, CONFIGr, &regval, SRC_IP_CFGf,
                              (ipmc_config & L3X_IPMC_SIP0) ? 1 : 0);
            if (regval != oregval) {
                SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, regval));
            }
        }
#if defined(BCM_5695)
    } else if (SOC_IS_DRACO15(unit)) {
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN
                (READ_PORT_TABm(unit, MEM_BLOCK_ANY, port, &pte));
            soc_PORT_TABm_field32_set(unit, &pte, IPMC_ENABLEf,
                                      (ipmc_config & L3X_IPMC_ENABLE) ? 1 : 0);
            soc_PORT_TABm_field32_set(unit, &pte, IPMC_DO_VLANf,
                                      (ipmc_config & L3X_IPMC_VLAN) ? 1 : 0);
            SOC_IF_ERROR_RETURN
                (WRITE_PORT_TABm(unit, MEM_BLOCK_ANY, port, &pte));
        }
#endif
#if defined(BCM_LYNX_SUPPORT)
    } else if (SOC_IS_LYNX(unit)) {
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &regval));
            oregval = regval;
            soc_reg_field_set(unit, CONFIGr, &regval, IPMC_ENABLEf,
                              (ipmc_config & L3X_IPMC_ENABLE) ? 1 : 0);
            if (regval != oregval) {
                SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, regval));
            }
        }
        SOC_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));
        oregval = regval;
        switch (ipmc_config & (L3X_IPMC_SIP | L3X_IPMC_SIP0)) {
        case L3X_IPMC_SIP | L3X_IPMC_SIP0:
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              PIM_DM_SM_ENABLEf, 1);
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              SRC_IP_CFGf, 0);
            break;
        case L3X_IPMC_SIP0:
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              PIM_DM_SM_ENABLEf, 0);
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              SRC_IP_CFGf, 1);
            break;
        case L3X_IPMC_SIP:
        default:
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              PIM_DM_SM_ENABLEf, 0);
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              SRC_IP_CFGf, 0);
            break;
        }
        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            soc_reg_field_set(unit, ARL_CONTROLr, &regval,
                              IPMC_DO_VLANf,
                              (ipmc_config & L3X_IPMC_VLAN) ? 1 : 0);
        }
        if (regval != oregval) {
            SOC_IF_ERROR_RETURN(WRITE_ARL_CONTROLr(unit, regval));
        }
#endif
#ifdef BCM_TUCANA_SUPPORT
    } else if (SOC_IS_TUCANA(unit)) {
        PBMP_E_ITER(unit, port) {
            SOC_IF_ERROR_RETURN
                (READ_PORT_TAB_MOD_CHK(unit, MEM_BLOCK_ANY, port, &pte));
            soc_PORT_TABm_field32_set(unit, &pte, IPMC_ENABLEf,
                                      (ipmc_config & L3X_IPMC_ENABLE) ? 1 : 0);
            if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
                soc_PORT_TABm_field32_set(unit, &pte, IPMC_DO_VLANf,
                                          (ipmc_config & L3X_IPMC_VLAN) ? 1 : 0);
            }
            SOC_IF_ERROR_RETURN
                (WRITE_PORT_TAB_MOD_CHK(unit, MEM_BLOCK_ANY, port, &pte));
        }
#endif /* BCM_TUCANA_SUPPORT */
    }
    return SOC_E_NONE;
}

int
_soc_mem_cmp_l3x(int unit, void *ent_a, void *ent_b)
{
    ip_addr_t	ip_a, ip_b, src_ip_a, src_ip_b;
    vlan_id_t	vlan_a, vlan_b;
    int		ipmc_a, ipmc_b;
    uint32	ipmc_config;

    ipmc_config = SOC_CONTROL(unit)->hash_key_config;

    ip_a = soc_L3Xm_field32_get(unit, ent_a, IP_ADDRf);
    ip_b = soc_L3Xm_field32_get(unit, ent_b, IP_ADDRf);
    SOC_MEM_COMPARE_RETURN(ip_a, ip_b);

    ipmc_a = soc_L3Xm_field32_get(unit, ent_a, IPMCf);
    ipmc_b = soc_L3Xm_field32_get(unit, ent_b, IPMCf);
    SOC_MEM_COMPARE_RETURN(ipmc_a, ipmc_b);

    if ((ipmc_config & L3X_IPMC_ENABLE) && ipmc_a) {
        src_ip_a = soc_L3Xm_field32_get(unit, ent_a, SRC_IP_ADDRf);
        src_ip_b = soc_L3Xm_field32_get(unit, ent_b, SRC_IP_ADDRf);
        SOC_MEM_COMPARE_RETURN(src_ip_a, src_ip_b);

        if (ipmc_config & L3X_IPMC_VLAN) {
            vlan_a = soc_L3Xm_field32_get(unit, ent_a, VLAN_IDf);
            vlan_b = soc_L3Xm_field32_get(unit, ent_b, VLAN_IDf);
            SOC_MEM_COMPARE_RETURN(vlan_a, vlan_b);
        }
    }

    return 0;
}

/*
 * Function:
 *    soc_mem_index_limit
 * Purpose:
 *    Returns the maximum possible index for any dynamically extendable memory
 */
int 
soc_mem_index_limit(int unit, soc_mem_t mem)
{
    
    return soc_mem_index_count(unit, mem);
}

#endif /* BCM_XGS_SWITCH_SUPPORT */

/*
 * Function:
 *	soc_mem_index_last
 * Purpose:
 *	Compute the highest index currently occupied in a sorted table
 */

int
soc_mem_index_last(int unit, soc_mem_t mem, int copyno)
{
    assert(SOC_MEM_IS_VALID(unit, mem));
    assert(soc_attached(unit));
    assert(soc_mem_is_sorted(unit,mem));

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }

    assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));

    return (SOC_MEM_INFO(unit, mem).index_min +
                SOP_MEM_STATE(unit, mem).count[copyno] - 1);
}


/*
 * Function:
 *	soc_mem_entry_dump
 * Purpose:
 *	Debug routine to dump a formatted table entry.
 */

void
soc_mem_entry_dump(int unit, soc_mem_t mem, void *buf)
{
    soc_field_info_t *fieldp;
    soc_mem_info_t *memp;
    int f;
#if !defined(SOC_NO_NAMES)
    uint32 key_type, default_type;
    uint32 fval[SOC_MAX_MEM_FIELD_WORDS];
    char tmp[(SOC_MAX_MEM_FIELD_WORDS * 8) + 3];
#endif
             /* Max nybbles + "0x" + null terminator */

    memp = &SOC_MEM_INFO(unit, mem);

    soc_cm_print("<");

    if (!SOC_MEM_IS_VALID(unit, mem)) {
        soc_cm_print(" Memory not valid for unit ");
    } else {
        for (f = memp->nFields - 1; f >= 0; f--) {
            fieldp = &memp->fields[f];
#if !defined(SOC_NO_NAMES)
            if (memp->flags & SOC_MEM_FLAG_MULTIVIEW) {
                if (soc_mem_field_valid(unit, mem, KEY_TYPEf)) {
                    soc_mem_field_get(unit, mem, buf, KEY_TYPEf, &key_type);
                } else if (soc_mem_field_valid(unit, mem, KEY_TYPE_0f)) {
                    soc_mem_field_get(unit, mem, buf, KEY_TYPE_0f, &key_type);
                } else if (soc_mem_field_valid(unit, mem, VP_TYPEf)) {
                    soc_mem_field_get(unit, mem, buf, VP_TYPEf, &key_type);
                } else {
                    soc_mem_field_get(unit, mem, buf, ENTRY_TYPEf, &key_type);
                }
                default_type = 0;
#if defined(BCM_TRIUMPH2_SUPPORT)
                if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit) ||
                    SOC_IS_HURRICANE(unit) || SOC_IS_TD_TT(unit) ||
                    SOC_IS_KATANA(unit)) {
                    if((mem == VLAN_MACm) ||
                        (mem == L3_ENTRY_IPV4_MULTICASTm) ||
                        (mem == L3_ENTRY_IPV6_UNICASTm) ||
                        (mem == L3_ENTRY_IPV6_MULTICASTm)){
                        default_type = key_type;
                    }
                }
#endif /* BCM_TRIUMPH2_SUPPORT */
                if (strstr(soc_fieldnames[fieldp->field], memp->views[key_type]) ||
                    (strcmp(memp->views[key_type], memp->views[default_type]) == 0 &&
                     !strstr(soc_fieldnames[fieldp->field], ":"))
                    || (fieldp->flags & SOCF_GLOBAL)) {
                    soc_cm_print("%s=", soc_fieldnames[fieldp->field]);
                    sal_memset(fval, 0, sizeof (fval));
                    soc_mem_field_get(unit, mem, buf, fieldp->field, fval);
                    _shr_format_long_integer(tmp, fval, SOC_MAX_MEM_FIELD_WORDS);
                    soc_cm_print("%s%s", tmp, f > 0 ? "," : "");
                }
            } else {
                soc_cm_print("%s=", soc_fieldnames[fieldp->field]);
                sal_memset(fval, 0, sizeof (fval));
                soc_mem_field_get(unit, mem, buf, fieldp->field, fval);
                _shr_format_long_integer(tmp, fval, SOC_MAX_MEM_FIELD_WORDS);
                soc_cm_print("%s%s", tmp, f > 0 ? "," : "");
            }
#endif
        }
    }

    soc_cm_print(">");
}

void
soc_mem_entry_dump_if_changed(int unit, soc_mem_t mem, void *buf, char *prefix)
{
    soc_field_info_t *fieldp;
    soc_mem_info_t *memp;
    int f;
    uint32 fval[SOC_MAX_MEM_FIELD_WORDS];
    uint32 nfval[SOC_MAX_MEM_FIELD_WORDS];
    void *null_entry;
    char tmp[132];
    char *fldpref;
    int nprint;

    memp = &SOC_MEM_INFO(unit, mem);

    if (!SOC_MEM_IS_VALID(unit, mem)) {
        soc_cm_print("%s<Memory not valid for unit>\n", prefix);
        return;
    }

    null_entry = soc_mem_entry_null(unit, mem);
    fldpref = "";
    nprint = 0;
    for (f = memp->nFields - 1; f >= 0; f--) {
        fieldp = &memp->fields[f];
        sal_memset(fval, 0, sizeof (fval));
        sal_memset(nfval, 0, sizeof (nfval));
        soc_mem_field_get(unit, mem, buf, fieldp->field, fval);
	if (null_entry != NULL) {
	    soc_mem_field_get(unit, mem, null_entry, fieldp->field, nfval);
	    if (sal_memcmp(fval, nfval, SOC_MAX_MEM_FIELD_WORDS *
			   sizeof (uint32)) == 0) {
		continue;
	    }
	}
        if (nprint == 0) {
            soc_cm_print("%s<", prefix);
        }
        _shr_format_long_integer(tmp, fval, SOC_MAX_MEM_FIELD_WORDS);
#if !defined(SOC_NO_NAMES)
        soc_cm_print("%s%s=%s", fldpref, soc_fieldnames[fieldp->field], tmp);
#endif
        fldpref = ",";
        nprint += 1;
    }

    if (nprint != 0) {
        soc_cm_print(">\n");
    }
}

/************************************************************************/
/* Routines for configuring the table memory caches			*/
/************************************************************************/

/*
 * Function:
 *	soc_mem_cache_get
 * Purpose:
 *	Return whether or not caching is enabled for a specified memory
 *	or memories
 */

int
soc_mem_cache_get(int unit,
                  soc_mem_t mem,
                  int copyno)
{
    int		rv;

    assert(SOC_UNIT_VALID(unit));
    assert(SOC_MEM_IS_VALID(unit, mem));

    if (!soc_mem_is_cachable(unit, mem)) {
	return FALSE;
    }

    MEM_LOCK(unit, mem);

    if (copyno == SOC_BLOCK_ALL) {
	rv = TRUE;
	SOC_MEM_BLOCK_ITER(unit, mem, copyno) {
	    rv = (rv && (SOC_MEM_STATE(unit, mem).cache[copyno] != NULL));
	}
    } else {
	rv = (SOC_MEM_STATE(unit, mem).cache[copyno] != NULL);
    }

    MEM_UNLOCK(unit, mem);

    return rv;
}

/*
 * Function:
 *	soc_mem_cache_set
 * Purpose:
 *	Enable or disable caching for a specified memory or memories
 * Notes:
 *	Only tables whose contents are not modified by hardware in
 *	any way are eligible for caching.
 */

int
soc_mem_cache_set(int unit,
                  soc_mem_t mem,
                  int copyno,
                  int enable)
{
    int blk;
    soc_memstate_t *memState;
    int entry_dw;
    int index_cnt;
    int cache_size, vmap_size;
    uint8 *vmap;
    uint32 *cache;

    assert(SOC_UNIT_VALID(unit));
    assert(SOC_MEM_IS_VALID(unit, mem));

    if (!soc_mem_is_cachable(unit, mem)) {
	return (enable ? SOC_E_UNAVAIL : SOC_E_NONE);
    }

    memState = &SOC_MEM_STATE(unit, mem);
    entry_dw = soc_mem_entry_words(unit, mem);
    index_cnt = soc_mem_index_count(unit, mem);
    cache_size = index_cnt * entry_dw * 4;
    vmap_size = (index_cnt + 7) / 8;

    soc_cm_debug(DK_SOCMEM,
                 "soc_mem_cache_set: unit %d memory %s.%s %sable\n",
                 unit, SOC_MEM_UFNAME(unit, mem),
                 SOC_BLOCK_NAME(unit, copyno),
                 enable ? "en" : "dis");

    if (copyno != COPYNO_ALL) {
        assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
    }

    MEM_LOCK(unit, mem);

    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }

        /*
         * Turn off caching if currently enabled.
         */

	if (memState->cache[blk] != NULL) {
            sal_free(memState->cache[blk]);
            memState->cache[blk] = NULL;
	}

	if (memState->vmap[blk] != NULL) {
            sal_free(memState->vmap[blk]);
            memState->vmap[blk] = NULL;
        }

        if (!enable) {
            continue;
        }

        /* Allocate new cache */

        if ((cache = sal_alloc(cache_size, "table-cache")) == NULL) {
            MEM_UNLOCK(unit, mem);
            return SOC_E_MEMORY;
        }

        if ((vmap = sal_alloc(vmap_size, "table-vmap")) == NULL) {
            sal_free(cache);
            MEM_UNLOCK(unit, mem);
            return SOC_E_MEMORY;
        }

        sal_memset(vmap, 0, vmap_size);

#ifdef	BCM_XGS_SWITCH_SUPPORT
	/* Pre-load cache from hardware table, if quickly-doable. */

	if (soc_feature(unit, soc_feature_table_dma) &&
	    soc_mem_dmaable(unit, mem, blk)) {
	    int		index_min = soc_mem_index_min(unit, mem);
	    uint32	*table;

	    if ((table = soc_cm_salloc(unit, cache_size, "dma")) != NULL) {
		if (soc_mem_read_range(unit, mem, blk,
				       index_min, index_cnt - 1,
				       &table[index_min * entry_dw]) >= 0) {
		    sal_memcpy(cache, table, cache_size);

		    /* Set all vmap bits as valid */
		    sal_memset(&vmap[index_min / 8], 0xff,
			       (index_cnt + 7 - index_min) / 8);

		    /* Clear invalid bits at the left and right ends */
		    vmap[index_min / 8] &= 0xff00 >> (8 - index_min % 8);
		    vmap[vmap_size - 1] &= 0x00ff >> ((8 - index_cnt % 8) % 8);
		}

		soc_cm_sfree(unit, table);
	    }
	}
#endif	/* BCM_XGS_SWITCH_SUPPORT */

        soc_cm_debug(DK_SOCMEM,
                     "soc_mem_cache_set: cache=%p size=%d vmap=%p\n",
                     (void *)cache, cache_size, (void *)vmap);

        memState->vmap[blk] = vmap;
        memState->cache[blk] = cache;
    }

    MEM_UNLOCK(unit, mem);

    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_mem_cache_invalidate
 * Purpose:
 *	Invalidate a cache entry. Forces read from the device instead
 *      of from cached copy.
 */

int
soc_mem_cache_invalidate(int unit,
			 soc_mem_t mem,
			 int copyno,
			 int index)
{
    int blk;
    uint8 *vmap;

    assert(SOC_UNIT_VALID(unit));
    assert(SOC_MEM_IS_VALID(unit, mem));

    if (!soc_mem_is_cachable(unit, mem)) {
	return SOC_E_UNAVAIL;
    }

    if (index < soc_mem_index_min(unit, mem) ||
	index > soc_mem_index_max(unit, mem)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_cache_invalidate: invalid index %d "
                     "for memory %s\n",
                     index, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    MEM_LOCK(unit, mem);

    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
        if (copyno != SOC_BLOCK_ALL && copyno != blk) {
            continue;
        }

        if (SOC_MEM_STATE(unit, mem).cache[blk] == NULL) {
            continue;
        }

        /* Invalidate the cached entry */

        vmap = SOC_MEM_STATE(unit, mem).vmap[blk];
        CACHE_VMAP_CLR(vmap, index);
    }

    MEM_UNLOCK(unit, mem);

    return SOC_E_NONE;
}

/************************************************************************/
/* Routines for CFAPINIT             					*/
/************************************************************************/

/*
 * Function:
 *	cfapinit_write_cb (internal)
 * Purpose:
 *	Memory test callback routine for cfapinit
 */

static int
cfapinit_write_cb(struct soc_mem_test_s *parm,
                  int copyno,
                  int index,
                  uint32 *entry_data)
{
    return soc_mem_write(parm->unit, parm->mem, copyno, index, entry_data);
}

/*
 * Function:
 *	cfapinit_read_cb (internal)
 * Purpose:
 *	Memory test callback routine for cfapinit
 */

static int
cfapinit_read_cb(struct soc_mem_test_s *parm,
                 int copyno,
                 int index,
                 uint32 *entry_data)
{
    return soc_mem_read(parm->unit, parm->mem, copyno, index, entry_data);
}

/*
 * Function:
 *	cfapinit_miscompare_cb (internal)
 * Purpose:
 *	Memory test callback routine for cfapinit
 */

static int
cfapinit_miscompare_cb(struct soc_mem_test_s *parm,
                       int copyno,
                       int index,
                       uint32 *read_data,
                       uint32 *wrote_data,
                       uint32 *mask_data)
{
    uint8	*bad_list = parm->userdata;

    COMPILER_REFERENCE(copyno);
    COMPILER_REFERENCE(read_data);
    COMPILER_REFERENCE(wrote_data);
    COMPILER_REFERENCE(mask_data);

    bad_list[index - parm->index_start] = 1;

    return 1;
}

/*
 * Function:
 *	soc_mem_cfap_init
 * Purpose:
 *	Routine to diagnose and initialize CFAP pool.
 * Notes:
 *	Ordinarily this routine is not used.  When the chip is reset, it
 *	automatically writes the CFAP table with the correct pointer
 *	data.
 *
 *	This routine does the same thing, except it also runs a memory
 *	test and excludes bad entries in CBPHEADER/CBPDATA[0-3] from the
 *	CFAP pool.
 *
 *	On some devices, errors in the first 63 entries can't be eliminated.
 *	The routine fails if such errors are found.
 *
 *	CFAPPOOLSIZE reset default is 0x1fff (last!) for Draco.
 */

int
soc_mem_cfap_init(int unit)
{
    soc_mem_test_t	mt;
    uint8		*bad_list;
    int			entry_count, bad_count, bad_count_63;
    int			i, ptr, rv;
    uint32		cfappoolsize;
    soc_mem_t           cbpcellheader_m;
    soc_mem_t           cbppktheader_start_m;
    soc_mem_t           cbppktheader_end_m;
    soc_mem_t           cbpdata_start_m;
    soc_mem_t           cbpdata_end_m;
    soc_mem_t           cfap_m;

    cbpcellheader_m      = MMU_MEMORIES1_CBPHEADERm;
    cbppktheader_start_m = MMU_MEMORIES1_CBPHEADERm;
    cbppktheader_end_m   = MMU_MEMORIES1_CBPHEADERm;
    cbpdata_start_m      = MMU_MEMORIES1_CBPDATA0m;
    cbpdata_end_m        = MMU_MEMORIES1_CBPDATA3m;
    cfap_m               = MMU_MEMORIES1_CFAPm;

    if (!SOC_IS_XGS_SWITCH(unit)) {
        return SOC_E_NONE;
    }

    if (!soc_feature(unit, soc_feature_cfap_pool)) {
        return SOC_E_UNAVAIL;
    }

    if (SOC_IS_FB_FX_HX(unit)) {
        cbpcellheader_m      = MMU_CBPCELLHEADERm;
        cbppktheader_start_m = MMU_CBPPKTHEADER0m;
        cbppktheader_end_m   = MMU_CBPPKTHEADER1m;
        cbpdata_start_m      = MMU_CBPDATA0m;
        cbpdata_end_m        = MMU_CBPDATA15m;
        cfap_m               = MMU_CFAPm;
        SOC_IF_ERROR_RETURN(READ_CFAPCONFIGr(unit, &cfappoolsize));
        cfappoolsize = soc_reg_field_get(unit, CFAPCONFIGr,
                                         cfappoolsize, CFAPPOOLSIZEf);
    } else {
        SOC_IF_ERROR_RETURN
            (READ_CFAPPOOLSIZEr(unit, &cfappoolsize));
    }

    mt.unit		= unit;
    mt.patterns		= soc_property_get(unit,
                                           spn_CFAP_TESTS,
                                           MT_PAT_CHECKER | MT_PAT_ICHECKER);
    mt.copyno		= COPYNO_ALL;
    mt.index_start	= soc_mem_index_min(unit, cbpcellheader_m);
    mt.index_end	= soc_mem_index_max(unit, cbpcellheader_m);
    mt.index_step	= 1;
    mt.read_count	= 1;
    mt.status_cb	= 0;			/* No status */
    mt.write_cb		= cfapinit_write_cb;
    mt.read_cb		= cfapinit_read_cb;
    mt.miscompare_cb	= cfapinit_miscompare_cb;

    /*
     * Create array of status per entry, initially with all entries
     * marked good.  The test callback will mark bad entries.
     */

    entry_count = mt.index_end - mt.index_start + 1;

    if ((bad_list = sal_alloc(entry_count, "bad_list")) == NULL) {
        return SOC_E_MEMORY;
    }

    sal_memset(bad_list, 0, entry_count);

    /*
     * Test the five "parallel" memories: CBPHEADER and CBPDATA[0-3].
     */

    mt.userdata = bad_list;
    mt.mem = cbpcellheader_m;
    soc_cm_debug(DK_VERBOSE,
                 "soc_mem_cfap_init: unit %d: testing CBPHEADER\n",
                 unit);

    if ((rv = soc_mem_parity_control(unit, mt.mem,
                                     mt.copyno, FALSE)) < 0) {
        soc_cm_debug(DK_ERR,
                    "Unable to disable parity warnings on %s\n",
                    SOC_MEM_UFNAME(unit, mt.mem));
        goto done;
    }

    (void) soc_mem_test(&mt);

    if ((rv = soc_mem_parity_control(unit, mt.mem,
                                     mt.copyno, TRUE)) < 0) {
        soc_cm_debug(DK_ERR,
                    "Unable to disable parity warnings on %s\n",
                    SOC_MEM_UFNAME(unit, mt.mem));
        goto done;
    }

    if (SOC_IS_FB_FX_HX(unit)) {
        for (mt.mem = cbppktheader_start_m;
             mt.mem <= cbppktheader_end_m;
             mt.mem++) {
            soc_cm_debug(DK_VERBOSE,
                         "soc_mem_cfap_init: unit %d: testing CBPPKTHEADER%d\n",
                         unit, mt.mem - cbppktheader_start_m);

            if ((rv = soc_mem_parity_control(unit, mt.mem,
                                             mt.copyno, FALSE)) < 0) {
                soc_cm_debug(DK_ERR,
                            "Unable to disable parity warnings on %s\n",
                            SOC_MEM_UFNAME(unit, mt.mem));
                goto done;
            }
            (void) soc_mem_test(&mt);

            if ((rv = soc_mem_parity_control(unit, mt.mem,
                                             mt.copyno, TRUE)) < 0) {
                soc_cm_debug(DK_ERR,
                            "Unable to disable parity warnings on %s\n",
                            SOC_MEM_UFNAME(unit, mt.mem));
                goto done;
            }
        }
    }

    for (mt.mem = cbpdata_start_m;
         mt.mem <= cbpdata_end_m;
         mt.mem++) {
        soc_cm_debug(DK_VERBOSE,
                     "soc_mem_cfap_init: unit %d: testing CBPDATA%d\n",
                     unit, mt.mem - cbpdata_start_m);

        if ((rv = soc_mem_parity_control(unit, mt.mem,
                                         mt.copyno, FALSE)) < 0) {
            soc_cm_debug(DK_ERR,
                        "Unable to disable parity warnings on %s\n",
                        SOC_MEM_UFNAME(unit, mt.mem));
            goto done;
        }
        (void) soc_mem_test(&mt);

        if ((rv = soc_mem_parity_control(unit, mt.mem,
                                         mt.copyno, TRUE)) < 0) {
            soc_cm_debug(DK_ERR,
                        "Unable to disable parity warnings on %s\n",
                        SOC_MEM_UFNAME(unit, mt.mem));
            goto done;
        }
    }

    /*
     * Test only
     for (i = 1; i < 512; i += 2) {
         if (i < entry_count) {
             bad_list[i] = 1;
         }
     }
     */

    bad_count = 0;
    bad_count_63 = 0;

    for (i = mt.index_start; i <= mt.index_end; i++) {
        if (bad_list[i - mt.index_start]) {
            bad_count++;
            if (i < mt.index_start + 63) {
                bad_count_63++;
            }
        }
    }

    if ((SOC_IS_FB_FX_HX(unit)) && bad_count_63 > 0) {
        /*
         * On some devices the first 63 entries MUST be good since they are
         * allocated by the hardware on startup.
         */

        soc_cm_print("soc_mem_cfap_init: unit %d: "
                     "Chip unusable, %d error(s) in low entries\n",
                     unit, bad_count_63);
        sal_free(bad_list);
        return SOC_E_FAIL;
    }

    if (bad_count >= entry_count / 2) {
        soc_cm_print("soc_mem_cfap_init: unit %d: "
                     "Chip unusable, too many bad entries (%d)\n",
                     unit, bad_count);
        sal_free(bad_list);
        return SOC_E_FAIL;
    }

    /*
     * For each bad entry, swap it with a good entry taken from the end
     * of the list.  This puts all bad entries at the end of the table
     * where they will not be used.
     */

    ptr = mt.index_end + 1;

    for (i = mt.index_start; i < ptr; i++) {
        uint32		e_good, e_bad;

        if (bad_list[i - mt.index_start]) {
            soc_cm_debug(DK_VERBOSE,
                         "soc_mem_cfap_init: unit %d: "
                         "mapping out CBP index %d\n",
                         unit, i);

            do {
                --ptr;
            } while (bad_list[ptr - mt.index_start]);

            if ((rv = soc_mem_read(unit, cfap_m,
                                   MEM_BLOCK_ANY, i, &e_bad)) < 0) {
                sal_free(bad_list);
                return rv;
            }

            if ((rv = soc_mem_read(unit, cfap_m,
                                   MEM_BLOCK_ANY, ptr, &e_good)) < 0) {
                sal_free(bad_list);
                return rv;
            }

            if ((rv = soc_mem_write(unit, cfap_m,
                                    MEM_BLOCK_ANY, ptr, &e_bad)) < 0) {
                sal_free(bad_list);
                return rv;
            }

            if ((rv = soc_mem_write(unit, cfap_m,
                                    MEM_BLOCK_ANY, i, &e_good)) < 0) {
                sal_free(bad_list);
                return rv;
            }
        }
    }

    /*
     * Write CFAPPOOLSIZE with the number of good entries (minus 1).
     */

    if (bad_count > 0) {
        soc_cm_print("soc_mem_cfap_init: unit %d: "
                     "detected and removed %d bad entries\n",
                     unit, bad_count);

        cfappoolsize -= bad_count;

        if (SOC_IS_FB_FX_HX(unit)) {
            uint32  cfap_config = 0;
            uint32  val;
            uint32  cfap_thresh;

            soc_reg_field_set(unit, CFAPCONFIGr, &cfap_config,
                                CFAPPOOLSIZEf, cfappoolsize);

            if ((rv = WRITE_CFAPCONFIGr(unit, cfappoolsize)) < 0) {
                sal_free(bad_list);
                return rv;
            }

            if ((rv = READ_CFAPFULLTHRESHOLDr(unit, &val)) < 0) {
                sal_free(bad_list);
                return rv;
            }
            cfap_thresh = soc_reg_field_get(unit, CFAPFULLTHRESHOLDr,
                                             val, CFAPFULLRESETPOINTf);
            cfap_thresh -= bad_count;

            soc_reg_field_set(unit, CFAPFULLTHRESHOLDr, &val,
                              CFAPFULLRESETPOINTf, cfap_thresh);

            cfap_thresh = soc_reg_field_get(unit, CFAPFULLTHRESHOLDr,
                                             val, CFAPFULLSETPOINTf);
            cfap_thresh -= bad_count;
            soc_reg_field_set(unit, CFAPFULLTHRESHOLDr, &val,
                              CFAPFULLSETPOINTf, cfap_thresh);
            if ((rv = WRITE_CFAPFULLTHRESHOLDr(unit, val)) < 0) {
                sal_free(bad_list);
                return rv;
            }
        } else {
            if ((rv = WRITE_CFAPPOOLSIZEr(unit, cfappoolsize)) < 0) {
                sal_free(bad_list);
                return rv;
            }
        }
    }

done:

    sal_free(bad_list);

    return SOC_E_NONE;
}


#ifdef BCM_HERCULES_SUPPORT

/************************************************************************/
/* Routines for LLAINIT             					*/
/************************************************************************/

/*
 * Function:
 *	llainit_write_cb (internal)
 * Purpose:
 *	Memory test callback routine for llainit
 */

static int
llainit_write_cb(struct soc_mem_test_s *parm,
                  int copyno,
                  int index,
                  uint32 *entry_data)
{
    return soc_mem_write(parm->unit, parm->mem, copyno, index, entry_data);
}

/*
 * Function:
 *	llainit_read_cb (internal)
 * Purpose:
 *	Memory test callback routine for llainit
 */

static int
llainit_read_cb(struct soc_mem_test_s *parm,
                 int copyno,
                 int index,
                 uint32 *entry_data)
{
    return soc_mem_read(parm->unit, parm->mem, copyno, index, entry_data);
}

#define LLA_INIT_PP_SBE_FLAG    0x8000

/*
 * Function:
 *	llainit_miscompare_cb (internal)
 * Purpose:
 *	Memory test callback routine for llainit
 */

static int
llainit_miscompare_cb(struct soc_mem_test_s *parm,
                       int copyno,
                       int index,
                       uint32 *read_data,
                       uint32 *wrote_data,
                       uint32 *mask_data)
{
    uint16	*bad_list = parm->userdata;

    COMPILER_REFERENCE(copyno);
    COMPILER_REFERENCE(read_data);
    COMPILER_REFERENCE(wrote_data);
    COMPILER_REFERENCE(mask_data);

    if (parm->mem == MEM_PPm) {
        int num_bits, bit_num, dw, badw=0, i;
        uint32 bad_bits;

        num_bits = 0;
        bit_num = 0;
        dw = soc_mem_entry_words(parm->unit, parm->mem);
        for (i = 0; i < dw; i++) {
            bad_bits = (read_data[i] ^ wrote_data[i]) & mask_data[i];
            if (bad_bits) {
                num_bits += _shr_popcount(bad_bits);
                badw = i;
            }
        }

        if (num_bits == 1) {
            /* Exactly one error, which column? */
            uint32 test_word = (read_data[badw] ^ wrote_data[badw]) &
                mask_data[badw];
            bit_num = 0;
            while (test_word) {
                bit_num++;
                test_word >>= 1;
            }
            bit_num += (badw * 32) - 1;
            bad_list[index - parm->index_start] =
                LLA_INIT_PP_SBE_FLAG | (uint16) bit_num;
        } else {
            bad_list[index - parm->index_start] = 1;
        }
    } else {
        bad_list[index - parm->index_start] = 1;
    }
    return 1;
}

#define LLA_PARITY_MASK 0xfff /* LLA parity depends only on first 12 bits */

/*
 * Function:
 *	llainit_miscompare_cb (internal)
 * Purpose:
 *	Memory test callback routine for llainit
 */

static int
lla_write_list(int unit, int port)
{
    int                 index_start, index_end, i, last_good;
    SHR_BITDCL          *lla_cache = SOC_CONTROL(unit)->lla_map[port];
    mem_lla_entry_t     lla_entry;
    int			blk;

    assert(lla_cache != NULL);

    sal_memset(&lla_entry, 0, sizeof (lla_entry));
    index_start	= soc_mem_index_min(unit, MEM_LLAm);
    index_end	= soc_mem_index_max(unit, MEM_LLAm);
    blk = SOC_PORT_BLOCK(unit, port);

    /*
     * For each bad entry, skip over it by linking the next pointer on the
     * preceeding good entry to the next good entry.  This will skip
     * multiple bad entries if necessary.
     */

    last_good = index_start;

    for (i = index_start; i <= index_end; i++) {
        if (SHR_BITGET(lla_cache, i)) {
            continue;
        }

        soc_mem_field32_set(unit, MEM_LLAm, &lla_entry, NEXTPTRf, i);
        soc_mem_field32_set(unit, MEM_LLAm, &lla_entry, PARITYf,
                            _shr_popcount((i ^ last_good) &
                                          LLA_PARITY_MASK) & 1);
        SOC_IF_ERROR_RETURN
            (soc_mem_write(unit, MEM_LLAm, blk, last_good, &lla_entry));
        last_good = i;
    }

    /* Close the loop on the linked list */
    i = index_start;
    soc_mem_field32_set(unit, MEM_LLAm, &lla_entry, NEXTPTRf, i);
    soc_mem_field32_set(unit, MEM_LLAm, &lla_entry, PARITYf,
                        _shr_popcount((i ^ last_good) &
                                      LLA_PARITY_MASK) & 1);
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, MEM_LLAm, blk, last_good, &lla_entry));

    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_mem_lla_port_init
 * Purpose:
 *	Routine to diagnose and initialize LLA pool for a port.
 * Notes:
 *	Ordinarily this routine is not used.  When the chip is reset, the
 *      LLA automatically initializes with the pointers linking to the
 *      next index.
 *
 *      !!!
 *      A1 update:  This must be used for Herc A1 LLA to function properly.
 *      !!!
 *
 *	This routine runs a memory test and excludes bad entries in the LLA
 *      from the LLA pool.
 *
 *	On Hercules, errors in the first entry can't be eliminated.
 *	The routine fails if such errors are found.
 *
 *      MMU_CELLLMTTOTAL is adjusted to reflect the decreased capacity.
 */

int
soc_mem_lla_port_init(int unit, soc_port_t port)
{
    soc_mem_test_t	mt;
    uint16		*bad_list;
    int			i, entry_count, bad_count, bad_count_0, sbe_count;
    SHR_BITDCL		*lla_cache;
    soc_control_t	*soc = SOC_CONTROL(unit);
    int			alloc_size;

    assert(soc->lla_map[port] == NULL);
          /* Should have been flushed before this */

    mt.unit		= unit;
    mt.patterns		= soc_property_get(unit, spn_LLA_TESTS, 0);
    mt.copyno		= port;
    mt.index_start	= soc_mem_index_min(unit, MEM_LLAm);
    mt.index_end	= soc_mem_index_max(unit, MEM_LLAm);
    mt.index_step	= 1;
    mt.read_count	= 1;
    mt.status_cb	= 0;			/* No status */
    mt.write_cb		= llainit_write_cb;
    mt.read_cb		= llainit_read_cb;
    mt.miscompare_cb	= llainit_miscompare_cb;

    /*
     * Create array of status per entry, initially with all entries
     * marked good.  The test callback will mark bad entries.
     */

    entry_count = mt.index_end - mt.index_start + 1;

    if ((bad_list = sal_alloc(entry_count * 2, "bad_list")) == NULL) {
        return SOC_E_MEMORY;
    }

    sal_memset(bad_list, 0, entry_count * 2);

    /*
     * Test the two "parallel" memories: LLA and PP.
     */

    mt.userdata = bad_list;

    soc_cm_debug(DK_VERBOSE,
                 "soc_mem_lla_port_init: unit %d port %s: testing LLA\n",
                 unit, SOC_PORT_NAME(unit, port));

    mt.mem = MEM_LLAm;
    (void) soc_mem_test(&mt);

    soc_cm_debug(DK_VERBOSE,
                 "soc_mem_lla_port_init: unit %d: testing PP\n",
                 unit);

    mt.mem = MEM_PPm;
    (void) soc_mem_test(&mt);

    bad_count = 0;
    bad_count_0 = 0;
    sbe_count = 0;

    alloc_size = SHR_BITALLOCSIZE(mt.index_end);
    if ((lla_cache = sal_alloc(alloc_size, "LLA map")) == NULL) {
        sal_free(bad_list);
        return SOC_E_MEMORY;
    }

    sal_memset(lla_cache, 0, alloc_size);

    for (i = mt.index_start; i <= mt.index_end; i++) {
        if (bad_list[i - mt.index_start]) {
            if (bad_list[i - mt.index_start] & LLA_INIT_PP_SBE_FLAG) {
                soc_cm_debug(DK_VERBOSE,
                             "soc_mem_lla_init: unit %d port %s: "
                             "PP index %d has SBE at bit %d\n",
                             unit, SOC_PORT_NAME(unit, port), i,
                             bad_list[i - mt.index_start] & 0xfff);
                sbe_count++;
            } else {
                bad_count++;
                SHR_BITSET(lla_cache, i);
                soc_cm_debug(DK_VERBOSE,
                             "soc_mem_lla_init: unit %d port %s: "
                             "mapping out LLA index %d\n",
                             unit, SOC_PORT_NAME(unit, port), i);
                if (i == 0) {
                    bad_count_0++;
                    break;
                }
            }
        }
    }

    if (bad_count_0 > 0) {
        /*
         * On Hercules the first entry MUST be good since it is the
         * starting point for the linked list
         */

        soc_cm_print("soc_mem_lla_init: unit %d port %s:"
                     "Port unusable, error in first entry\n",
                     unit, SOC_PORT_NAME(unit, port));
        sal_free(lla_cache);
        sal_free(bad_list);
        return SOC_E_FAIL;
    }

    if (bad_count >= entry_count / 2) {
        soc_cm_print("soc_mem_lla_init: unit %d: port %d:"
                     "Port unusable, too many bad entries (%d)\n",
                     unit, port, bad_count);
        sal_free(lla_cache);
        sal_free(bad_list);
        return SOC_E_FAIL;
    }

    if (sbe_count > 0) {
        soc_cm_print("soc_mem_lla_init: unit %d: port %d: "
                     "detected %d SBE entries\n",
                     unit, port, sbe_count);
        soc->sbe_disable[port] = TRUE;
    }

    if (bad_count > 0) {
        soc_cm_print("soc_mem_lla_init: unit %d: port %d: "
                     "detected and ignoring %d bad entries\n",
                     unit, port, bad_count);

        soc->lla_cells_good[port] = entry_count - bad_count;
    }

    soc->lla_map[port] = lla_cache;
    sal_free(bad_list);
    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_mem_lla_init
 * Purpose:
 *	Routine to diagnose and initialize LLA pool.
 * Notes:
 *      This routine loops through the ports using the above function.
 */
int
soc_mem_lla_init(int unit)
{
    soc_port_t port;
    uint32 mask;
    int rv, enabled = FALSE;

    if (!SOC_IS_HERCULES1(unit)) {
        return SOC_E_NONE;
    }

    /* We don't want to get parity warnings at this time */
    mask = soc_pci_read(unit, CMIC_IRQ_MASK);
    if (mask & IRQ_MMU_IRQ_STAT) {
        enabled = TRUE;
        soc_intr_disable(unit, IRQ_MMU_IRQ_STAT);
    }

    PBMP_ALL_ITER(unit, port) {
        if (SOC_CONTROL(unit)->lla_map[port] == NULL) {
            if ((rv = soc_mem_lla_port_init(unit, port)) < 0) {
                if (rv == SOC_E_FAIL) {
                    /* Port is invalid! */
                    
                } else {
                    /* Fatal error, abort */
                    return rv;
                }
            }
        }

        SOC_IF_ERROR_RETURN(lla_write_list(unit, port));

        /* We don't want to hear about LLA errors produced by this test */
        SOC_IF_ERROR_RETURN(WRITE_MMU_INTCLRr(unit, port,
                                              SOC_ERR_HAND_MMU_LLA_PARITY));
    }

    /* Restore state */
    if (enabled) {
#ifdef BCM_CMICM_SUPPORT
        if(soc_feature(unit, soc_feature_cmicm)) {
            
            /*  soc_cmicm_intr0_enable(unit, IRQ_MMU_IRQ_STAT); */
        } else
#endif
        {
            soc_intr_enable(unit, IRQ_MMU_IRQ_STAT);
        }
    }

    return SOC_E_NONE;
}

#endif /* BCM_HERCULES_SUPPORT */


/************************************************************************/
/* Routines for Filter Mask START/COUNT Maintenance                     */
/************************************************************************/

#if defined(BCM_DRACO_SUPPORT) || defined(BCM_TUCANA_SUPPORT)

/*
 * Function:
 *	_soc_mem_fmask_update
 * Purpose:
 *	Internal routine to change the RULES_START and RULES_COUNT in a
 *	mask entry.
 * Notes:
 *	To avoid deadlock, when both IRULE and IMASK are locked, IRULE
 *	must be locked first.
 *	In the event of an error, filters will be messed up.
 */

STATIC int
_soc_mem_fmask_update(int unit, soc_mem_t mask_mem, int blk,
                      soc_fm_t *fm, int mask)
{
    int rv = SOC_E_NONE;
    soc_cm_debug(DK_SOCMEM,
                 "fmask_update: mask=%d, set start=%d count=%d\n",
                 mask, fm[mask].start, fm[mask].count);

    if (soc_feature(unit, soc_feature_filter_xgs) &&
        SOC_PERSIST(unit)->filter_enable) {

        imask_entry_t		te;

        MEM_LOCK(unit, mask_mem);

        rv = soc_mem_read(unit, mask_mem, blk, mask, &te);
        if (SOC_FAILURE(rv)) {
            MEM_UNLOCK(unit, mask_mem);
            return (rv);
        }
        soc_GFILTER_IMASKm_field32_set
            (unit, &te, RULES_STARTf, fm[mask].start);
        soc_GFILTER_IMASKm_field32_set
            (unit, &te, RULES_SIZEf, fm[mask].count);

        rv = soc_mem_write(unit, mask_mem, blk, mask, &te);
        MEM_UNLOCK(unit, mask_mem);
    }
    return rv;
}

#else /* !BCM_DRACO_SUPPORT */

#define _soc_mem_fmask_update(unit, mem, blk, fm, mask)	0

#endif /* !BCM_DRACO_SUPPORT */

#ifdef BCM_FILTER_SUPPORT
/*
 * Function:
 *	_soc_mem_fmask_init
 * Purpose:
 *	Clear cached filter mask start/count for all masks.
 * Notes:
 *	Call after the mask/rule table are cleared.
 *	Assumes the hardware mask table start/count fields are already zero.
 */

STATIC int
_soc_mem_fmask_init(int unit, soc_mem_t mask_mem, int blk)
{
    soc_fm_t		*fm;
    soc_mem_t		rule_mem;
    int			mask_min, mask_max, mask;
    int			base;

    fm = &SOC_PERSIST(unit)->filter_masks[blk][0];
    switch (mask_mem) {
    case FILTER_IMASKm:
        rule_mem = FILTER_IRULEm;
        break;
    case GFILTER_IMASKm:
        rule_mem = GFILTER_IRULEm;
        break;
    default:
        rule_mem = INVALIDm;
        break;
    }

    assert(rule_mem != INVALIDm);

    base = soc_mem_index_min(unit, rule_mem);

    mask_min = soc_mem_index_min(unit, mask_mem);
    mask_max = soc_mem_index_max(unit, mask_mem);

    for (mask = mask_min; mask <= mask_max; mask++) {
        fm[mask].start = base;
        fm[mask].count = 0;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *	_soc_mem_frule_written
 * Purpose:
 *	Update the appropriate start/count fields in the mask table
 *	following a write to the rule table.
 * Parameters:
 *	unit - StrataSwitch unit #
 *	rule_mem - FILTER_IRULEm or GFILTER_IRULEm
 *	blk - block number of memory
 *	index - index written
 *      entry - data written
 * Notes:
 *	This is a tricky routine that imitates what the hardware does in
 *	5615, but does not do in 5690.  This works well, but only if the
 *	table is kept in perfect sorted order at ALL times by ALL writes.
 */

STATIC int
_soc_mem_frule_written(int unit, soc_mem_t rule_mem, int blk,
                       int index, irule_entry_t *entry)
{
    soc_fm_t            *fm;
    int                 mask_min, mask_max;
    int			mask_cur;	/* Mask currently containing index */
    int			mask_new;	/* New mask to contain index */
    int			mask;
    soc_mem_t           mask_mem;

    soc_cm_debug(DK_SOCMEM,
                 "frule_written: blk=%d index=%d\n",
                 blk, index);

    fm = &SOC_PERSIST(unit)->filter_masks[blk][0];
    switch (rule_mem) {
    case FILTER_IRULEm:
        mask_mem = FILTER_IMASKm;
        break;
    case GFILTER_IRULEm:
        mask_mem = GFILTER_IMASKm;
        break;
    default:
        mask_mem = INVALIDm;
        break;
    }

    assert(mask_mem != INVALIDm);

    mask_min = soc_mem_index_min(unit, mask_mem);
    mask_max = soc_mem_index_max(unit, mask_mem);

    /*
     * If the null entry is written, it must be getting written over the
     * last used entry in the table.
     */

    if (soc_mem_compare_entry(unit, rule_mem, entry,
                              soc_mem_entry_null(unit, rule_mem)) == 0) {
        mask_new = mask_max + 1;
    } else {
        mask_new = soc_GFILTER_IRULEm_field32_get(unit, entry, FSELf);
    }

    /*
     * Find the mask which currently contains the index being written.
     * If not found, mask_cur will equal mask_max + 1.
     */

    for (mask_cur = mask_min; mask_cur <= mask_max; mask_cur++) {
        if (index < fm[mask_cur].start + fm[mask_cur].count) {
            break;
        }
    }

    soc_cm_debug(DK_SOCMEM,
                 "frule_written: mask_cur=%d mask_new=%d\n",
                 mask_cur, mask_new);

    if (mask_cur < mask_new) {
        /* Shift up */

        fm[mask_cur].count--;

        SOC_IF_ERROR_RETURN
            (_soc_mem_fmask_update(unit, mask_mem, blk, fm, mask_cur));

        for (mask = mask_cur + 1;
	     mask <= mask_new && mask <= mask_max; mask++) {
            /* fm[mask].count stays zero; don't need to update HW */
            fm[mask].start--;
        }

        if (mask_new <= mask_max) {
            fm[mask_new].count++;
            SOC_IF_ERROR_RETURN
                (_soc_mem_fmask_update(unit, mask_mem, blk, fm, mask_new));
        }
    } else if (mask_cur > mask_new) {
        /* Shift down */

        if (mask_cur <= mask_max) {
            fm[mask_cur].start++;
            fm[mask_cur].count--;

            SOC_IF_ERROR_RETURN
                (_soc_mem_fmask_update(unit, mask_mem, blk, fm, mask_cur));
        }

        for (mask = mask_cur - 1; mask > mask_new; mask--) {
            /* fm[mask].count stays zero; don't need to update HW */
            fm[mask].start++;
        }

        fm[mask_new].count++;

        SOC_IF_ERROR_RETURN
            (_soc_mem_fmask_update(unit, mask_mem, blk, fm, mask_new));
    }

    return SOC_E_NONE;
}

#if defined(BCM_TUCANA_SUPPORT)
/*
 * Function:
 *	_soc_mem_frule_to_mask
 * Purpose:
 *	Given a rule index, return which mask it belongs to.
 * Parameters:
 *	unit - StrataSwitch unit #
 *	rule_mem - FILTER_IRULEm or GFILTER_IRULEm
 *	blk - block number of memory
 *	index_rule - irule index
 *      index_mask (OUT) - imask index
 * Returns:
 *	SOC_E_XXX
 */

STATIC int
_soc_mem_frule_to_mask(int unit, soc_mem_t rule_mem, int blk,
                       int index_rule, int *index_mask)
{
    soc_fm_t            *fm;
    int                 mask_min, mask_max;
    int			mask_cur;	/* Mask currently containing index */
    soc_mem_t           mask_mem;

    fm = &SOC_PERSIST(unit)->filter_masks[blk][0];
    switch (rule_mem) {
    case FILTER_IRULEm:
        mask_mem = FILTER_IMASKm;
        break;
    case GFILTER_IRULEm:
        mask_mem = GFILTER_IMASKm;
        break;
    default:
        mask_mem = INVALIDm;
        break;
    }

    assert(mask_mem != INVALIDm);

    mask_min = soc_mem_index_min(unit, mask_mem);
    mask_max = soc_mem_index_max(unit, mask_mem);

    /*
     * Find the mask which currently contains the index being written.
     * If not found, mask_cur will equal mask_max + 1.
     */

    for (mask_cur = mask_min; mask_cur <= mask_max; mask_cur++) {
        if (index_rule < fm[mask_cur].start + fm[mask_cur].count) {
            break;
        }
    }

    *index_mask = mask_cur;

    return (mask_cur > mask_max) ? SOC_E_INTERNAL : SOC_E_NONE;
}
#endif /* BCM_TUCANA_SUPPORT */
#endif /* BCM_FILTER_SUPPORT */

#if defined(BCM_XGS_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)

/************************************************************************
 * Table DMA
 *
 *    In hardware, it shares the same buffers and registers used for ARL DMA
 *    In software, it shares with ARL DMA the same semaphore and interrupt
 *        handler
 * Assumption
 *    ARL DMA and Table DMA should not co-exist. The buffer should be
 *        pre-allocated at least (index_max - index_min + 1) * sizeof (entry)
 *
 ************************************************************************/

#define SOC_MEM_DMA_MAX_DATA_BEATS	4

/*
 * Function:
 *	soc_mem_dmaable
 * Purpose:
 *	Determine whether a table is DMA_able
 * Returns:
 *	0 if not, 1 otherwise
 */
int
soc_mem_dmaable(int unit, soc_mem_t mem, int copyno)
{
    assert(SOC_MEM_IS_VALID(unit, mem));

    if (SOC_CONTROL(unit)->tableDmaMutex == 0) {	/* not enabled */
        return FALSE;
    }

    if (SOC_MEM_STATE(unit, mem).cache[copyno] != NULL) {
        return FALSE;		/* Caching is enabled for memory */
    }

#if defined(BCM_FIREBOLT_SUPPORT)
#ifdef SOC_MEM_L3_DEFIP_WAR
    if (soc_feature(unit, soc_feature_l3_defip_map) &&
        (mem == L3_DEFIPm ||
         mem == L3_DEFIP_ONLYm ||
         mem == L3_DEFIP_DATA_ONLYm ||
         mem == L3_DEFIP_HIT_ONLYm)) {
        return FALSE;   /* Could be non-contiguous */
    }
#endif
#endif

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_l3_defip_hole) &&
       (mem == L3_DEFIPm ||
        mem == L3_DEFIP_ONLYm ||
        mem == L3_DEFIP_DATA_ONLYm ||
        mem == L3_DEFIP_HIT_ONLYm)) {
           return FALSE;
    }
    if ((mem == LMEPm) || (mem == LMEP_1m)) {
        return FALSE;
    }
#endif                    

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_MMU) {
            switch (mem) {
            case CTR_FLEX_COUNT_0m: case CTR_FLEX_COUNT_1m: case CTR_FLEX_COUNT_2m:
            case CTR_FLEX_COUNT_3m: case CTR_FLEX_COUNT_4m: case CTR_FLEX_COUNT_5m:
            case CTR_FLEX_COUNT_6m: case CTR_FLEX_COUNT_7m: case CTR_FLEX_COUNT_8m:
            case CTR_FLEX_COUNT_9m: case CTR_FLEX_COUNT_10m: case CTR_FLEX_COUNT_11m: 
                return TRUE;
            default:
                return FALSE;
            }
        }
    }
#endif /* BCM_TRX_SUPPORT */
    if (!soc_feature(unit, soc_feature_flexible_dma_steps) &&
        soc_mem_index_count(unit, mem) > 1) {
#ifdef BCM_TRIUMPH_SUPPORT
        if (soc_feature(unit, soc_feature_esm_support) &&
            SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
            int index0, index1;
            soc_mem_t real_mem;

            /* On BCM56624_A0, don't do DMA for tables whose associated TCAM
             * entry is wider than a single raw TCAM entry */
            soc_tcam_mem_index_to_raw_index(unit, mem, 0, &real_mem, &index0);
            soc_tcam_mem_index_to_raw_index(unit, mem, 1, &real_mem, &index1);
            if (index1 - index0 != 1) {
                return FALSE;
            }
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        /*
         * EGR_VLAN requires to combine entries from both pipe
         * the single entry ISBS_PORT_TO_PIPE_MAPPING and
         * ESBS_PORT_TO_PIPE_MAPPING table need data_beat size adjustment
         */
        if (mem == EGR_VLANm ||
            mem == ISBS_PORT_TO_PIPE_MAPPINGm ||
            mem == ESBS_PORT_TO_PIPE_MAPPINGm) {
            return FALSE;
        }
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        /*
         * IL_STAT_MEM_x counters 
         */
        if (mem == IL_STAT_MEM_3m ) {
            return FALSE;
        }
    }
#endif /* BCM_SHADOW_SUPPORT */


#ifdef  BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        return TRUE;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      return TRUE;
    }
#endif /* BCM_SIRIUS_SUPPORT */

    switch (mem) {
    case MMU_MEMORIES1_CCPm: case MMU_MEMORIES1_CFAPm: case HASH_TRAP_INFOm:
    case MMU_MEMORIES2_XQ0m: case MMU_MEMORIES2_XQ1m: case MMU_MEMORIES2_XQ2m:
    case MMU_MEMORIES2_XQ3m: case MMU_MEMORIES2_XQ4m: case MMU_MEMORIES2_XQ5m:
    case MMU_MEMORIES2_XQ6m: case MMU_MEMORIES2_XQ7m: case MMU_MEMORIES2_XQ8m:
    case MMU_MEMORIES2_XQ9m: case MMU_MEMORIES2_XQ10m:
    case MMU_MEMORIES2_XQ11m: case MMU_MEMORIES2_XQ12m:
    case MMU_MEMORIES2_XQ13m:
        return FALSE; /* gnats 2955 */
    default:
        return (soc_mem_entry_words(unit, mem) <= SOC_MEM_DMA_MAX_DATA_BEATS);
    }
}

#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)

/*
 * Function:
 *	soc_mem_slamable
 * Purpose:
 *	Determine whether a table is SLAMable
 * Returns:
 *	0 if not, 1 otherwise
 */
int
soc_mem_slamable(int unit, soc_mem_t mem, int copyno)
{
    assert(SOC_MEM_IS_VALID(unit, mem));

    if (SOC_CONTROL(unit)->tslamDmaMutex == 0) {        /* not enabled */
        return FALSE;
    }

#if defined(BCM_FIREBOLT_SUPPORT)
#ifdef SOC_MEM_L3_DEFIP_WAR
    if (soc_feature(unit, soc_feature_l3_defip_map) &&
        (mem == L3_DEFIPm ||
         mem == L3_DEFIP_ONLYm ||
         mem == L3_DEFIP_DATA_ONLYm ||
         mem == L3_DEFIP_HIT_ONLYm)) {
        return FALSE;   /* Could be non-contiguous */
    }
#endif
#endif

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_l3_defip_hole) &&
       (mem == L3_DEFIPm ||
        mem == L3_DEFIP_ONLYm ||
        mem == L3_DEFIP_DATA_ONLYm ||
        mem == L3_DEFIP_HIT_ONLYm)) {
           return FALSE;
    }
    if ((mem == LMEPm) || (mem == LMEP_1m)) {
        return FALSE;
    }
#endif                    

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_MMU) {
            return FALSE;
        }
    }
#endif /* BCM_TRX_SUPPORT */

    if (!soc_feature(unit, soc_feature_flexible_dma_steps) &&
        soc_mem_index_count(unit, mem) > 1) {
#ifdef BCM_TRIUMPH_SUPPORT
        if (soc_feature(unit, soc_feature_esm_support) &&
            SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
            int index0, index1;
            soc_mem_t real_mem;

            /* On BCM56624_A0, don't do DMA for tables whose associated TCAM
             * entry is wider than a single raw TCAM entry */
            soc_tcam_mem_index_to_raw_index(unit, mem, 0, &real_mem, &index0);
            soc_tcam_mem_index_to_raw_index(unit, mem, 1, &real_mem, &index1);
            if (index1 - index0 != 1) {
                return FALSE;
            }
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

    return TRUE;
}

/*
 * Function:
 *	_soc_xgs3_mem_dma
 * Purpose:
 *	DMA acceleration for soc_mem_read_range() on FB/ER
 * Parameters:
 *	buffer -- must be pointer to sufficiently large
 *			DMA-able block of memory
 */
STATIC int
_soc_xgs3_mem_dma(int unit, soc_mem_t mem, int copyno,
             int index_min, int index_max, void *buffer)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    uint32        start_addr;
    uint32        count;
    uint32        data_beats;
    uint32        spacing;
    int           rv = SOC_E_NONE;
    uint32        cfg, rval;
    soc_mem_t     cmd_mem = INVALIDm;
    uint8         at;
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif

#ifdef BCM_SBUSDMA_SUPPORT
    if (soc_feature(unit, soc_feature_sbusdma)) {
        return _soc_xgs3_mem_sbus_dma(unit, mem, copyno,
                                      index_min, index_max, buffer);
    }
#endif

    soc_cm_debug(DK_SOCMEM | DK_DMA, "_soc_xgs3_mem_dma: unit %d"
                 " mem %s.%s index %d-%d buffer %p\n",
                 unit, SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno),
                 index_min, index_max, buffer);

#if defined(BCM_EASYRIDER_SUPPORT)
    if (!SOC_IS_SHADOW(unit)) {
        switch (SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem))) {
           case SOC_BLK_BSE:
               cmd_mem = COMMAND_MEMORY_BSEm;
               break;
           case SOC_BLK_CSE:
               cmd_mem = COMMAND_MEMORY_CSEm;
               break;
           case SOC_BLK_HSE:
               cmd_mem = COMMAND_MEMORY_HSEm;
               break;
           default:
               break;
        }
    }
#endif
    data_beats = soc_mem_entry_words(unit, mem);

    assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
#ifndef BCM_WARM_BOOT_SUPPORT
    assert(soc_mem_index_valid(unit, mem, index_min));
    assert(soc_mem_index_valid(unit, mem, index_max));
    assert(index_min <= index_max);
#endif
    count = index_max - index_min + 1;
    if (count < 1) {
        return SOC_E_NONE;
    }

    if (cmd_mem != INVALIDm) {
        MEM_LOCK(unit, cmd_mem);
    }

    TABLE_DMA_LOCK(unit);

    start_addr = soc_mem_addr_get(unit, mem, copyno, index_min, &at);
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
            soc_pci_write(unit, CMIC_CMCx_TABLE_DMA_PCIMEM_START_ADDR_OFFSET(cmc), soc_cm_l2p(unit, buffer));
            soc_pci_write(unit, CMIC_CMCx_TABLE_DMA_SBUS_START_ADDR_OFFSET(cmc), start_addr);
            rval = 0;
            soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_ENTRY_COUNTr, &rval, COUNTf, count);
            soc_pci_write(unit, CMIC_CMCx_TABLE_DMA_ENTRY_COUNT_OFFSET(cmc), rval);
    } else
#endif /* CMICM Support */
    {

        WRITE_CMIC_TABLE_DMA_PCIMEM_START_ADDRr(unit, soc_cm_l2p(unit, buffer));
        WRITE_CMIC_TABLE_DMA_SBUS_START_ADDRr(unit, start_addr);
        rval = 0;
        soc_reg_field_set(unit, CMIC_TABLE_DMA_ENTRY_COUNTr, &rval, COUNTf, count);
        if (soc_feature(unit, soc_feature_flexible_dma_steps) &&
            soc_mem_index_count(unit, mem) > 1) {
#if defined(BCM_TRIUMPH_SUPPORT)
            if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
                int index0, index1, increment;
                soc_mem_t real_mem;

                soc_tcam_mem_index_to_raw_index(unit, mem, 0, &real_mem, &index0);
                soc_tcam_mem_index_to_raw_index(unit, mem, 1, &real_mem, &index1);
                increment = _shr_popcount(index1 - index0 - 1);
                soc_reg_field_set(unit, CMIC_TABLE_DMA_ENTRY_COUNTr, &rval,
                                  SBUS_ADDR_INCREMENT_STEPf, increment);
            }
#endif /* BCM_TRIUMPH_SUPPORT */
        }
        WRITE_CMIC_TABLE_DMA_ENTRY_COUNTr(unit, rval);
    }
    soc_cm_debug(DK_SOCMEM | DK_DMA,
                 "_soc_xgs3_mem_dma: table dma of %d entries "
                 "of %d beats from 0x%x\n",
                 count, data_beats, start_addr);

    /* Set beats. Clear table DMA abort,done and error bit. Start DMA */
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        /* For CMICm retain Endianess etc... */
        cfg = soc_pci_read(unit, CMIC_CMCx_TABLE_DMA_CFG_OFFSET(cmc));
        soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg,
                            BEATSf, data_beats);
        soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg, ABORTf, 0);
        soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg, ENf, 0);
        soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg, 
                          ENABLE_MULTIPLE_SBUS_CMDSf, 0);
        soc_pci_write(unit, CMIC_CMCx_TABLE_DMA_CFG_OFFSET(cmc), cfg);  /* Clearing EN clears stats */
        
#ifdef BCM_EXTND_SBUS_SUPPORT
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            rval = 0;
            /* Use TR3 style sbus access */
            soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_SBUS_CMD_CONFIGr, &rval, 
                              EN_TR3_SBUS_STYLEf, 1);
            soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_SBUS_CMD_CONFIGr, &rval, 
                              SBUS_BLOCKIDf, SOC_BLOCK2SCH(unit, copyno));
            WRITE_CMIC_CMC0_TABLE_DMA_SBUS_CMD_CONFIGr(unit, rval);
        }
#endif
    } else
#endif
    {
        cfg = 0;
        soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg,
                            BEATSf, data_beats);
    }
    if (soc_feature(unit, soc_feature_multi_sbus_cmds)) {
        
        if (soc->sbusCmdSpacing < 0) {
            spacing = data_beats > 7 ? data_beats + 1 : 8;
        } else {
            spacing = soc->sbusCmdSpacing;
        }
        if ((SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_XQPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_GXPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_XLPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_SPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_GPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_QGPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_LLS) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_MMU)) {
            spacing = 0;
        }

#ifdef BCM_KATANA_SUPPORT 
       if (SOC_IS_KATANA(unit)) {
           /* disable MOR for EFP_TCAM and EGR_VLAN_XLATE */
           if (mem == EFP_TCAMm || mem == EGR_VLAN_XLATEm) {
               spacing = 0;
           }  
       } 
#endif 

        if (spacing) {
#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg,
                                  MULTIPLE_SBUS_CMD_SPACINGf, spacing);
                soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg,
                                  ENABLE_MULTIPLE_SBUS_CMDSf, 1);

            } else
#endif
            {
                soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg,
                                  MULTIPLE_SBUS_CMD_SPACINGf, spacing);
                soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg,
                                  ENABLE_MULTIPLE_SBUS_CMDSf, 1);
            }
        }
    }
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg, ENf, 1);
        soc_pci_write(unit, CMIC_CMCx_TABLE_DMA_CFG_OFFSET(cmc), cfg);
    } else
#endif
    {
        soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg, ENf, 1);
        WRITE_CMIC_TABLE_DMA_CFGr(unit, cfg);
    }
    rv = SOC_E_TIMEOUT;
    if (soc->tableDmaIntrEnb) {
#ifdef BCM_CMICM_SUPPORT
        if(soc_feature(unit, soc_feature_cmicm)) {
            soc_cmicm_intr0_enable(unit, IRQ_CMCx_TDMA_DONE);
            if (sal_sem_take(soc->tableDmaIntr, soc->tableDmaTimeout) < 0) {
                rv = SOC_E_TIMEOUT;
            }
            soc_cmicm_intr0_disable(unit, IRQ_CMCx_TDMA_DONE);

            cfg = soc_pci_read(unit, CMIC_CMCx_TABLE_DMA_STAT_OFFSET(cmc));
            if (soc_reg_field_get(unit, CMIC_CMC0_TABLE_DMA_STATr,
                                                        cfg, DONEf)) {
                rv = SOC_E_NONE;
                if (soc_reg_field_get(unit, CMIC_CMC0_TABLE_DMA_STATr,
                                                        cfg, ERRORf)) {
                    rv = SOC_E_FAIL;
                }
            }
        } else
#endif
        {
            soc_intr_enable(unit, IRQ_TDMA_DONE);
            if (sal_sem_take(soc->tableDmaIntr, soc->tableDmaTimeout) < 0) {
                rv = SOC_E_TIMEOUT;
            }
            soc_intr_disable(unit, IRQ_TDMA_DONE);

            READ_CMIC_TABLE_DMA_CFGr(unit, &cfg);
            if (soc_reg_field_get(unit, CMIC_TABLE_DMA_CFGr,
                    cfg, DONEf)) {
                rv = SOC_E_NONE;
                if (soc_reg_field_get(unit, CMIC_TABLE_DMA_CFGr,
                        cfg, ERRORf)) {
                    rv = SOC_E_FAIL;
                }
            }
        }
    } else {
        soc_timeout_t to;

        soc_timeout_init(&to, soc->tableDmaTimeout, 10000);

        do {
#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                cfg = soc_pci_read(unit, CMIC_CMCx_TABLE_DMA_STAT_OFFSET(cmc));
                if (soc_reg_field_get(unit, CMIC_CMC0_TABLE_DMA_STATr,
                                                        cfg, DONEf)) {
                    rv = SOC_E_NONE;
                    if (soc_reg_field_get(unit, CMIC_CMC0_TABLE_DMA_STATr,
                                                        cfg, ERRORf)) {
                        rv = SOC_E_FAIL;
                    }
                    break;
                }
            } else
#endif
            {
                READ_CMIC_TABLE_DMA_CFGr(unit, &cfg);
                if (soc_reg_field_get(unit, CMIC_TABLE_DMA_CFGr,
                        cfg, DONEf)) {
                    rv = SOC_E_NONE;
                    if (soc_reg_field_get(unit, CMIC_TABLE_DMA_CFGr,
                            cfg, ERRORf)) {
                        rv = SOC_E_FAIL;
                    }
                    break;
                }
             }
        } while(!(soc_timeout_check(&to)));
    }

    if (rv < 0) {
        if (rv != SOC_E_TIMEOUT) {
            soc_cm_debug(DK_ERR, "%s: %s.%s failed(NAK)\n",
                         __FUNCTION__, SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno));
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit)) {
                /* SCHAN nack */
                soc_triumph2_mem_nack(INT_TO_PTR(unit),
                        INT_TO_PTR(start_addr), 0, 0, 0);
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
        } else {
            soc_timeout_t to;

            soc_cm_debug(DK_ERR, "%s: %s.%s %s timeout\n",
                         __FUNCTION__, SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno),
                         soc->tableDmaIntrEnb ? "interrupt" : "polling");

#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                /* Abort Table DMA */
                cfg = soc_pci_read(unit, CMIC_CMCx_TABLE_DMA_CFG_OFFSET(cmc));
                soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg, ENf, 0);
                soc_reg_field_set(unit, CMIC_CMC0_TABLE_DMA_CFGr, &cfg, ABORTf, 1);
                soc_pci_write(unit, CMIC_CMCx_TABLE_DMA_CFG_OFFSET(cmc), cfg);

                /* Check the done bit to confirm */
                soc_timeout_init(&to, soc->tableDmaTimeout, 0);
                while (1) {
                    cfg = soc_pci_read(unit, CMIC_CMCx_TABLE_DMA_STAT_OFFSET(cmc));
                    if (soc_reg_field_get(unit, CMIC_CMC0_TABLE_DMA_STATr, cfg, DONEf)) {
                        break;
                    }
                    if (soc_timeout_check(&to)) {
                        soc_cm_debug(DK_ERR,
                                     "_soc_xgs3_mem_dma: Abort Failed\n");
                        break;
                    }
                }
            } else
#endif
            {
                /* Abort Table DMA */
                READ_CMIC_TABLE_DMA_CFGr(unit, &cfg);
                soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg, ENf, 0);
                soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg, ABORTf, 1);
                soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg, DONEf, 0);
                soc_reg_field_set(unit, CMIC_TABLE_DMA_CFGr, &cfg, ERRORf, 0);
                WRITE_CMIC_TABLE_DMA_CFGr(unit, cfg);
                
                /* Check the done bit to confirm */
                soc_timeout_init(&to, soc->tableDmaTimeout, 0);
                while (1) {
                    READ_CMIC_TABLE_DMA_CFGr(unit, &cfg);
                    if (soc_reg_field_get(unit, CMIC_TABLE_DMA_CFGr, cfg, DONEf)) {
                        break;
                    }
                    if (soc_timeout_check(&to)) {
                        soc_cm_debug(DK_ERR,
                                     "_soc_xgs3_mem_dma: Abort Failed\n");
                        break;
                    }
                }
            }
        }
    }

    soc_cm_sinval(unit, (void *)buffer, WORDS2BYTES(data_beats) * count);

    TABLE_DMA_UNLOCK(unit);

    if (cmd_mem != INVALIDm) {
        MEM_UNLOCK(unit, cmd_mem);
    }

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_KATANA_SUPPORT)
    if (soc_feature(unit, soc_feature_xy_tcam_direct) &&
        (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_CAM) &&
        (!(SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_EXT_CAM))) {
        uint32 *entry, index;
        uint32 key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
        soc_field_t key_field[4], mask_field[4];
        int field_count, bit_length[4], word_length[4], i, word;
        soc_pbmp_t pbmp;

        if (mem == L3_DEFIPm || mem == L3_DEFIP_Xm || mem == L3_DEFIP_Ym ||
            mem == L3_DEFIP_ONLYm) {
            key_field[0] = KEY0f;
            key_field[1] = KEY1f;
            mask_field[0] = MASK0f;
            mask_field[1] = MASK1f;
            field_count = 2;
#if defined(BCM_KATANA_SUPPORT)
        } else if (mem == L3_DEFIP_PAIR_128m || mem == L3_DEFIP_PAIR_128_ONLYm) {
            key_field[0] = KEY0_UPRf;
            key_field[1] = KEY1_UPRf;
            key_field[2] = KEY0_LWRf;
            key_field[3] = KEY1_LWRf;
            mask_field[0] = MASK0_UPRf;
            mask_field[1] = MASK1_UPRf;
            mask_field[2] = MASK0_LWRf;
            mask_field[3] = MASK1_LWRf;
            field_count = 4;
#endif
        } else {
            if (soc_mem_field_valid(unit, mem, FULL_KEYf)) {
                key_field[0] = FULL_KEYf;
                mask_field[0] = FULL_MASKf;
            } else {
                key_field[0] = KEYf;
                mask_field[0] = MASKf;
            }
            field_count = 1;
        }
        for (i = 0; i < field_count; i++) {
            bit_length[i] = soc_mem_field_length(unit, mem, key_field[i]);
            word_length[i] = (bit_length[i] + 31) / 32;
        }
        entry = buffer;
        for (index = 0; index < count; index++) {
            for (i = 0; i < field_count; i++) {
                soc_mem_field_get(unit, mem, entry, key_field[i], key);
                soc_mem_field_get(unit, mem, entry, mask_field[i], mask);
                for (word = 0; word < word_length[i]; word++) {
                    mask[word] = key[word] | ~mask[word];
                }
                if ((bit_length[i] & 0x1f) != 0) {
                    mask[word - 1] &= (1 << (bit_length[i] & 0x1f)) - 1;
                }
                soc_mem_field_set(unit, mem, entry, mask_field[i], mask);
            }
            if (SOC_IS_TD_TT(unit) &&
                (mem == FP_GLOBAL_MASK_TCAM_Xm ||
                 mem == FP_GLOBAL_MASK_TCAM_Ym)) {
                soc_mem_pbmp_field_get(unit, mem, entry, IPBM_MASKf, &pbmp);
                if (mem == FP_GLOBAL_MASK_TCAM_Xm) {
                    SOC_PBMP_AND(pbmp, PBMP_XPIPE(unit));
                } else {
                    SOC_PBMP_AND(pbmp, PBMP_YPIPE(unit));
                }
                soc_mem_pbmp_field_set(unit, mem, entry, IPBM_MASKf, &pbmp);
            }
            entry += data_beats;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT, BCM_KATANA_SUPPORT */

    return rv;
}

/*
 * Function:
 *	_soc_xgs3_mem_slam
 * Purpose:
 *	DMA acceleration for soc_mem_write_range() on FB/ER
 * Parameters:
 *	buffer -- must be pointer to sufficiently large
 *			DMA-able block of memory
 *      index_begin <= index_end - Forward direction
 *      index_begin >  index_end - Reverse direction
 */

STATIC int
_soc_xgs3_mem_slam(int unit, soc_mem_t mem, int copyno,
             int index_begin, int index_end, void *buffer)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    uint32        start_addr;
    uint32        count;
    uint32        data_beats;
    uint32        spacing;
    int           rv = SOC_E_NONE;
    uint32        cfg, rval;
    void          *buffer_ptr;
    uint8         at;
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif

    if (SOC_HW_ACCESS_DISABLE(unit)) {
        return SOC_E_NONE;
    }
#ifdef BCM_SBUSDMA_SUPPORT
    if (soc_feature(unit, soc_feature_sbusdma)) {
        return _soc_xgs3_mem_sbus_slam(unit, mem, copyno,
                                       index_begin, index_end, 
                                       buffer, FALSE);
    }
#endif
    soc_cm_debug(DK_SOCMEM | DK_DMA, "_soc_xgs3_mem_slam: unit %d"
                 " mem %s.%s index %d-%d buffer %p\n",
                 unit, SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno),
                 index_begin, index_end, buffer);

    data_beats = soc_mem_entry_words(unit, mem);

    assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
#ifndef BCM_WARM_BOOT_SUPPORT
    assert(soc_mem_index_valid(unit, mem, index_begin));
    assert(soc_mem_index_valid(unit, mem, index_end));
#endif

    if (index_begin > index_end) {
        start_addr = soc_mem_addr_get(unit, mem, copyno, index_end, &at);
        count = index_begin - index_end + 1;
    } else {
        start_addr = soc_mem_addr_get(unit, mem, copyno, index_begin, &at);
        count = index_end - index_begin + 1;
    }

    if (count < 1) {
        return SOC_E_NONE;
    }

    buffer_ptr = buffer;
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_KATANA_SUPPORT) \
    || defined(BCM_SHADOW_SUPPORT)
    if (soc_feature(unit, soc_feature_xy_tcam) &&
        (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_CAM) &&
        (!(SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_EXT_CAM))) {
        uint32 *entry, *converted_entry;
        uint32 key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
        uint32 converted_key, converted_mask, index;
        soc_field_t key_field[4], mask_field[4];
        int field_count, bit_length[4], word_length[4], i, word;
        uint8 no_trans = FALSE;  

        if (!soc_feature(unit, soc_feature_xy_tcam_direct)) {
            /* Only clear the "don't care" key bits */
            no_trans = TRUE;
        }
        if (mem == L3_DEFIPm || mem == L3_DEFIP_Xm || mem == L3_DEFIP_Ym ||
            mem == L3_DEFIP_ONLYm) {
            key_field[0] = KEY0f;
            key_field[1] = KEY1f;
            mask_field[0] = MASK0f;
            mask_field[1] = MASK1f;
            field_count = 2;
#if defined(BCM_KATANA_SUPPORT)
        } else if (mem == L3_DEFIP_PAIR_128m || mem == L3_DEFIP_PAIR_128_ONLYm) {
            key_field[0] = KEY0_UPRf;
            key_field[1] = KEY1_UPRf;
            key_field[2] = KEY0_LWRf;
            key_field[3] = KEY1_LWRf;
            mask_field[0] = MASK0_UPRf;
            mask_field[1] = MASK1_UPRf;
            mask_field[2] = MASK0_LWRf;
            mask_field[3] = MASK1_LWRf;
            field_count = 4;
#endif
        } else {
            if (soc_mem_field_valid(unit, mem, FULL_KEYf)) {
                key_field[0] = FULL_KEYf;
                mask_field[0] = FULL_MASKf;
            } else {
                key_field[0] = KEYf;
                mask_field[0] = MASKf;
            }
            field_count = 1;
        }
        for (i = 0; i < field_count; i++) {
            bit_length[i] = soc_mem_field_length(unit, mem, key_field[i]);
            word_length[i] = (bit_length[i] + 31) / 32;
        }
        buffer_ptr = soc_cm_salloc(unit, count * data_beats * sizeof(uint32),
                                   "converted buffer");
        if (buffer_ptr == NULL) {
            return SOC_E_MEMORY;
        }
        entry = buffer;
        converted_entry = buffer_ptr;
        for (index = 0; index < count; index++) {
            sal_memcpy(converted_entry, entry, data_beats * sizeof(uint32));
            for (i = 0; i < field_count; i++) {
                soc_mem_field_get(unit, mem, entry, key_field[i], key);
                soc_mem_field_get(unit, mem, entry, mask_field[i], mask);
                for (word = 0; word < word_length[i]; word++) {
                    converted_key = key[word] & mask[word];
                    if (!no_trans) {
                        converted_mask = key[word] | ~mask[word];
                        mask[word] = converted_mask;
                    }
                    key[word] = converted_key;
                }
                if ((bit_length[i] & 0x1f) != 0) {
                    mask[word - 1] &= (1 << (bit_length[i] & 0x1f)) - 1;
                }
                soc_mem_field_set(unit, mem, converted_entry, key_field[i],
                                  key);
                soc_mem_field_set(unit, mem, converted_entry, mask_field[i],
                                  mask);
            }
            entry += data_beats;
            converted_entry += data_beats;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT, BCM_KATANA_SUPPORT */
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        /* For CMICm retain Endianess etc... */
        cfg = soc_pci_read(unit, CMIC_CMCx_SLAM_DMA_CFG_OFFSET(cmc));
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, ABORTf, 0);
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, ENf, 0);
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, 
                          ENABLE_MULTIPLE_SBUS_CMDSf, 0);
        soc_pci_write(unit, CMIC_CMCx_SLAM_DMA_CFG_OFFSET(cmc), cfg);  /* Clearing EN clears the stats */

        /* Set beats. Clear tslam DMA abort,done and error bit. Start DMA */
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, BEATSf, data_beats);
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, REV_MODULO_COUNTf,
                          (count % (64 / data_beats)));
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg,
                            ORDERf, (index_begin > index_end) ? 1:0);

#ifdef BCM_EXTND_SBUS_SUPPORT
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            rval = 0;
            /* Use TR3 style sbus access */
            soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_SBUS_CMD_CONFIGr, &rval, 
                              EN_TR3_SBUS_STYLEf, 1);
            soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_SBUS_CMD_CONFIGr, &rval, 
                              SBUS_BLOCKIDf, SOC_BLOCK2SCH(unit, copyno));
            WRITE_CMIC_CMC0_SLAM_DMA_SBUS_CMD_CONFIGr(unit, rval);
        }
#endif
    } else
#endif
    {
        /* Set beats. Clear tslam DMA abort,done and error bit. Start DMA */
        cfg = 0;
        soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg, REV_MODULO_COUNTf,
                          (count % (64 / data_beats)));
        soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg, BEATSf, data_beats);
        soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg,
                            ORDERf, (index_begin > index_end) ? 1:0);
    }
    if (soc_feature(unit, soc_feature_multi_sbus_cmds)) {
        
        if (soc->sbusCmdSpacing < 0) {
            spacing = data_beats > 7 ? data_beats + 1 : 8;
        } else {
            spacing = soc->sbusCmdSpacing;
        }
        if ((SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_XQPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_GXPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_XLPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_SPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_GPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_QGPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_LLS) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_MMU)) {
            spacing = 0;
        }

#ifdef BCM_KATANA_SUPPORT 
       if (SOC_IS_KATANA(unit)) {
           /* disable MOR for EFP_TCAM and EGR_VLAN_XLATE */
           if (mem == EFP_TCAMm || mem == EGR_VLAN_XLATEm) {
               spacing = 0;
           }  
       } 
#endif 

        if (spacing) {
#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg,
                                  MULTIPLE_SBUS_CMD_SPACINGf, spacing);
                soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg,
                                  ENABLE_MULTIPLE_SBUS_CMDSf, 1);
            } else
#endif
            {
                soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg,
                                  MULTIPLE_SBUS_CMD_SPACINGf, spacing);
                soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg,
                                  ENABLE_MULTIPLE_SBUS_CMDSf, 1);
            }
        }
    }
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, ENf, 1);
    } else
#endif
    {
        soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg,
                            ENf, 1);
    }

    soc_cm_debug(DK_SOCMEM | DK_DMA,
                 "_soc_xgs3_mem_slam: tslam dma of %d entries "
                 "of %d beats from 0x%x to index %d-%d\n",
                 count, data_beats, start_addr, index_begin, index_end);

    TSLAM_DMA_LOCK(unit);

    soc_cm_sflush(unit, (void *)buffer_ptr, WORDS2BYTES(data_beats) * count);
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        soc_pci_write(unit, CMIC_CMCx_SLAM_DMA_PCIMEM_START_ADDR_OFFSET(cmc), soc_cm_l2p(unit, buffer_ptr));
        soc_pci_write(unit, CMIC_CMCx_SLAM_DMA_SBUS_START_ADDR_OFFSET(cmc), start_addr);
        rval = 0;
        soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_ENTRY_COUNTr, &rval, COUNTf, count);
        soc_pci_write(unit, CMIC_CMCx_SLAM_DMA_ENTRY_COUNT_OFFSET(cmc), rval);
        soc_pci_write(unit, CMIC_CMCx_SLAM_DMA_CFG_OFFSET(cmc), cfg);
    } else
#endif
    {
        WRITE_CMIC_SLAM_DMA_PCIMEM_START_ADDRr(unit, soc_cm_l2p(unit, buffer_ptr));
        WRITE_CMIC_SLAM_DMA_SBUS_START_ADDRr(unit, start_addr);
        rval = 0;
        soc_reg_field_set(unit, CMIC_SLAM_DMA_ENTRY_COUNTr, &rval, COUNTf, count);
        if (soc_feature(unit, soc_feature_flexible_dma_steps) &&
            soc_mem_index_count(unit, mem) > 1) {
#if defined(BCM_TRIUMPH_SUPPORT)
            if (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) {
                int index0, index1, increment;
                soc_mem_t real_mem;

                soc_tcam_mem_index_to_raw_index(unit, mem, 0, &real_mem, &index0);
                soc_tcam_mem_index_to_raw_index(unit, mem, 1, &real_mem, &index1);
                increment = _shr_popcount(index1 - index0 - 1);
                soc_reg_field_set(unit, CMIC_SLAM_DMA_ENTRY_COUNTr, &rval,
                                  SBUS_ADDR_INCREMENT_STEPf, increment);
            }
#endif /* BCM_TRIUMPH_SUPPORT */
        }
        WRITE_CMIC_SLAM_DMA_ENTRY_COUNTr(unit, rval);
        WRITE_CMIC_SLAM_DMA_CFGr(unit, cfg);
    }
    rv = SOC_E_TIMEOUT;
    if (soc->tslamDmaIntrEnb) {

#ifdef BCM_CMICM_SUPPORT
        if(soc_feature(unit, soc_feature_cmicm)) {
            soc_cmicm_intr0_enable(unit, IRQ_CMCx_TSLAM_DONE);
            if (sal_sem_take(soc->tslamDmaIntr, soc->tslamDmaTimeout) < 0) {
                rv = SOC_E_TIMEOUT;
            }
            soc_cmicm_intr0_disable(unit, IRQ_CMCx_TSLAM_DONE);

            cfg = soc_pci_read(unit, CMIC_CMCx_SLAM_DMA_STAT_OFFSET(cmc));
            if (soc_reg_field_get(unit, CMIC_CMC0_SLAM_DMA_STATr, cfg, DONEf)) {
                rv = SOC_E_NONE;
            }
            if (soc_reg_field_get(unit, CMIC_CMC0_SLAM_DMA_STATr, cfg, ERRORf)) {
                rv = SOC_E_FAIL;
            }
        } else
#endif
        {
            soc_intr_enable(unit, IRQ_TSLAM_DONE);
            if (sal_sem_take(soc->tslamDmaIntr, soc->tslamDmaTimeout) < 0) {
                rv = SOC_E_TIMEOUT;
            }
            soc_intr_disable(unit, IRQ_TSLAM_DONE);

            READ_CMIC_SLAM_DMA_CFGr(unit, &cfg);
            if (soc_reg_field_get(unit, CMIC_SLAM_DMA_CFGr, cfg, DONEf)) {
                rv = SOC_E_NONE;
            }
            if (soc_reg_field_get(unit, CMIC_SLAM_DMA_CFGr, cfg, ERRORf)) {
                rv = SOC_E_FAIL;
            }
        }
    } else {
        soc_timeout_t to;

        soc_timeout_init(&to, soc->tslamDmaTimeout, 10000);

        do {
#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                cfg = soc_pci_read(unit, CMIC_CMCx_SLAM_DMA_STAT_OFFSET(cmc));
                if (soc_reg_field_get(unit, CMIC_CMC0_SLAM_DMA_STATr, cfg, ERRORf)) {
                    rv = SOC_E_FAIL;
                    break;
                }
                if (soc_reg_field_get(unit, CMIC_CMC0_SLAM_DMA_STATr, cfg, DONEf)) {
                    rv = SOC_E_NONE;
                    break;
                }
            } else
#endif 
            {
                READ_CMIC_SLAM_DMA_CFGr(unit, &cfg);
                if (soc_reg_field_get(unit, CMIC_SLAM_DMA_CFGr, cfg, ERRORf)) {
                    rv = SOC_E_FAIL;
                    break;
                }
                if (soc_reg_field_get(unit, CMIC_SLAM_DMA_CFGr, cfg, DONEf)) {
                    rv = SOC_E_NONE;
                    break;
                }
            }
        } while (!(soc_timeout_check(&to)));
    }

    if (rv < 0) {
        if (rv != SOC_E_TIMEOUT) {
            soc_cm_debug(DK_ERR, "%s: %s.%s failed(NAK)\n",
                         __FUNCTION__, SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno));
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit)) {
                /* SCHAN nack */
                soc_triumph2_mem_nack(INT_TO_PTR(unit),
                        INT_TO_PTR(start_addr), 0, 0, 0);
            }
#endif /* BCM_TRIUMPH2_SUPPORT */
        } else {
            soc_timeout_t to;

            soc_cm_debug(DK_ERR, "%s: %s.%s %s timeout\n",
                         __FUNCTION__, SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno),
                         soc->tslamDmaIntrEnb ? "interrupt" : "polling");
            
#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                /* Abort Tslam DMA */
                cfg = soc_pci_read(unit, CMIC_CMCx_SLAM_DMA_CFG_OFFSET(cmc));
                soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, ENf, 0);
                soc_reg_field_set(unit, CMIC_CMC0_SLAM_DMA_CFGr, &cfg, ABORTf, 1);
                soc_pci_write(unit, CMIC_CMCx_SLAM_DMA_CFG_OFFSET(cmc), cfg);

                /* Check the done bit to confirm */
                soc_timeout_init(&to, soc->tslamDmaTimeout, 10000);
                while (1) {
                    cfg = soc_pci_read(unit, CMIC_CMCx_SLAM_DMA_STAT_OFFSET(cmc));
                    if (soc_reg_field_get(unit, CMIC_CMC0_SLAM_DMA_STATr, cfg, DONEf)) {
                        break;
                    }
                    if (soc_timeout_check(&to)) {
                        soc_cm_debug(DK_ERR,
                                     "_soc_xgs3_mem_slam: Abort Failed\n");
                        break;
                    }
                }
            } else
#endif
            {
                /* Abort Tslam DMA */
                READ_CMIC_SLAM_DMA_CFGr(unit, &cfg);
                soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg, ENf, 0);
                soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg, ABORTf, 1);
                soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg, DONEf, 0);
                soc_reg_field_set(unit, CMIC_SLAM_DMA_CFGr, &cfg, ERRORf, 0);
                WRITE_CMIC_SLAM_DMA_CFGr(unit, cfg);

                /* Check the done bit to confirm */
                soc_timeout_init(&to, soc->tslamDmaTimeout, 10000);
                while (1) {
                    READ_CMIC_SLAM_DMA_CFGr(unit, &cfg);
                    if (soc_reg_field_get(unit, CMIC_SLAM_DMA_CFGr, cfg, DONEf)) {
                        break;
                    }
                    if (soc_timeout_check(&to)) {
                        soc_cm_debug(DK_ERR,
                                     "_soc_xgs3_mem_slam: Abort Failed\n");
                        break;
                    }
                }
            }
        }
    }

    TSLAM_DMA_UNLOCK(unit);

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_KATANA_SUPPORT) \
    || defined(BCM_SHADOW_SUPPORT)
    if (soc_feature(unit, soc_feature_xy_tcam) &&
        (SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_CAM) &&
        (!(SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_EXT_CAM))) {
        soc_cm_sfree(unit, buffer_ptr);
    }
#endif /* BCM_TRIDENT_SUPPORT, BCM_KATANA_SUPPORT */

    return rv;
}

/*
 * Function:
 *      _soc_xgs3_mem_clear_slam
 * Purpose:
 *	soc_mem_clear acceleration using table DMA write (slam)
 */
STATIC int
_soc_xgs3_mem_clear_slam(int unit, soc_mem_t mem, 
                         int copyno, void *null_entry)
{
    int       rv, chunk_size, chunk_entries, mem_size, entry_words;
    int       index, index_end, index_min, index_max;
    uint32    *buf;

    if (SOC_WARM_BOOT(unit) || SOC_IS_RELOADING(unit)) {
        return SOC_E_NONE;
    }
#ifdef BCM_SBUSDMA_SUPPORT
    if (soc_feature(unit, soc_feature_sbusdma)) {
        return _soc_xgs3_mem_clear_sbus_slam(unit, mem, 
                                             copyno, null_entry);
    }
#endif
    chunk_size = SOC_MEM_CLEAR_CHUNK_SIZE_GET(unit);  
    index_min = soc_mem_index_min(unit, mem);
    index_max = soc_mem_index_max(unit, mem);
    entry_words = soc_mem_entry_words(unit, mem);
    mem_size = (index_max - index_min + 1) * entry_words * 4;
    if (mem_size < chunk_size) {
	chunk_size = mem_size;
    }

    buf = soc_cm_salloc(unit, chunk_size, "mem_clear_buf");
    if (buf == NULL) {
	return SOC_E_MEMORY;
    }

    chunk_entries = chunk_size / (entry_words * 4);

    if (null_entry == _soc_mem_entry_null_zeroes) {
	sal_memset(buf, 0, chunk_size);
    } else {
	for (index = 0; index < chunk_entries; index++) {
	    sal_memcpy(buf + (index * entry_words),
		       null_entry, entry_words * 4);
	}
    }

    rv = SOC_E_NONE;
    for (index = index_min; index <= index_max; index += chunk_entries) {
        index_end = index + chunk_entries - 1;
        if (index_end > index_max) {
            index_end = index_max;
        }

        rv = soc_mem_write_range(unit, mem, copyno, index, index_end, buf);
        if (rv < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_write_range: "
                         "write %s.%s[%d-%d] failed: %s\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno),
                         index, index_end, soc_errmsg(rv));
            break;
        }
    }

    soc_cm_sfree(unit, buf);
    return rv;
}

/*
 * Function:
 *      _soc_xgs3_mem_clear_pipe
 * Purpose:
 *	soc_mem_clear acceleration using pipeline table clear logic.
 *	Much faster than table slam.  Exists on non-easyrider XGS3.
 */
STATIC int
_soc_xgs3_mem_clear_pipe(int unit, soc_mem_t mem, int blk, void *null_entry)
{
    int		rv, mementries, to_usec, to_rv;
    uint32	hwreset1, hwreset2, membase, memoffset, memstage;
    soc_timeout_t to;

    if (SOC_WARM_BOOT(unit) || SOC_IS_RELOADING(unit) || SOC_HW_ACCESS_DISABLE(unit)) {
        return SOC_E_NONE;
    }
    if (SAL_BOOT_PLISIM) {
	return SOC_E_UNAVAIL;
    }
    if (!SOC_REG_IS_VALID(unit, ING_HW_RESET_CONTROL_1r)) {
	return SOC_E_UNAVAIL;
    }
    if (null_entry != _soc_mem_entry_null_zeroes) {
	return SOC_E_UNAVAIL;
    }

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TRIUMPH(unit) || SOC_IS_VALKYRIE(unit)) {
        switch (mem) {
        case ING_PW_TERM_COUNTERSm:
        case EGR_IPFIX_SESSION_TABLEm:
        case EGR_IPFIX_EXPORT_FIFOm:
            return SOC_E_UNAVAIL;
        default:
            break;
        }
    }
#if defined(BCM_TRIUMPH2_SUPPORT)
    else if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
             SOC_IS_VALKYRIE2(unit)) {
        if (mem == LMEPm) {
            return SOC_E_UNAVAIL;
        }
        if (soc_feature(unit, soc_feature_ser_parity)) {
            SOC_IF_ERROR_RETURN
                (soc_triumph2_ser_mem_clear(unit, mem));
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */
#if defined(BCM_ENDURO_SUPPORT)
    else if (SOC_IS_ENDURO(unit)) {
        if (mem == LMEPm || mem == LMEP_1m) {
            return SOC_E_UNAVAIL;
        }
    }
#endif /* BCM_ENDURO_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT)
    else if (SOC_IS_TD_TT(unit)) {
        if (soc_feature(unit, soc_feature_ser_parity)) {
            SOC_IF_ERROR_RETURN
                (soc_trident_ser_mem_clear(unit, mem));
        }
        if ((SOC_MEM_INFO(unit, mem).flags & SOC_MEM_FLAG_CAM) ||
            (SOC_MEM_INFO(unit, mem).index_max == 0)) {
            return SOC_E_UNAVAIL;
        }
        switch (mem) {
            case EGR_VLANm:
            case VLAN_TABm:
            case VLAN_SUBNETm:
            case VLAN_SUBNET_ONLYm:
            case L3_TUNNELm:
                return SOC_E_UNAVAIL;
            default:
            break;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */
#endif /* BCM_TRIUMPH_SUPPORT */

    if (SAL_BOOT_SIMULATION) {
	to_usec = 10000000;
    } else {
	to_usec = 50000;
    }
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit) && (mem == ING_VLAN_TAG_ACTION_PROFILEm)) {
       return SOC_E_UNAVAIL;
    }

 if (SOC_IS_SHADOW(unit)) {
        /* Reset HW Reset Control */
        SOC_IF_ERROR_RETURN(WRITE_ING_HW_RESET_CONTROL_2r(unit, 0));
        SOC_IF_ERROR_RETURN(WRITE_ING_HW_RESET_CONTROL_1r(unit, 0));
    }
#endif


    rv = SOC_E_NONE;
    to_rv = SOC_E_NONE;

#define	SOC_MEM_ADDR_OFFSET(_addr) ((_addr) & 0x000fffff)
#define	SOC_MEM_ADDR_STAGE(_addr) (((_addr) >> 24) & 0x3f)
#ifdef BCM_EXTND_SBUS_SUPPORT
#define	SOC_MEM_ADDR_OFFSET_EXTENDED(_addr) ((_addr) & 0x03ffffff)
#define	SOC_MEM_ADDR_STAGE_EXTENDED(_addr) (((_addr) >> 26) & 0x3f)
#endif

    membase = SOC_MEM_BASE(unit, mem);
    mementries = SOC_MEM_SIZE(unit, mem);
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        memoffset = SOC_MEM_ADDR_OFFSET_EXTENDED(membase);
        memstage = SOC_MEM_ADDR_STAGE_EXTENDED(membase);
    } else
#endif
    {
        memoffset = SOC_MEM_ADDR_OFFSET(membase);
        memstage = SOC_MEM_ADDR_STAGE(membase);
    }

    switch (SOC_BLOCK_TYPE(unit, blk)) {
    case SOC_BLK_IPIPE:
    case SOC_BLK_IPIPE_HI:
	hwreset1 = 0;
	soc_reg_field_set(unit, ING_HW_RESET_CONTROL_1r, &hwreset1,
			  OFFSETf, memoffset);
	soc_reg_field_set(unit, ING_HW_RESET_CONTROL_1r, &hwreset1,
			  STAGE_NUMBERf, memstage);
	hwreset2 = 0;
	soc_reg_field_set(unit, ING_HW_RESET_CONTROL_2r, &hwreset2,
			  COUNTf, mementries);
	soc_reg_field_set(unit, ING_HW_RESET_CONTROL_2r, &hwreset2,
			  VALIDf, 1);

	rv = WRITE_ING_HW_RESET_CONTROL_1r(unit, hwreset1);
	if (rv < 0) {
	    break;
	}
	rv = WRITE_ING_HW_RESET_CONTROL_2r(unit, hwreset2);
	if (rv < 0) {
	    break;
	}

	soc_timeout_init(&to, to_usec, 100);
	for (;;) {
	    rv = READ_ING_HW_RESET_CONTROL_2r(unit, &hwreset2);
	    if (rv < 0) {
		break;
	    }
	    if (soc_reg_field_get(unit, ING_HW_RESET_CONTROL_2r, hwreset2,
				  DONEf)) {
		break;
	    }
	    if (soc_timeout_check(&to)) {
		to_rv = SOC_E_TIMEOUT;
		break;
	    }
	}

        hwreset2 = 0;
        if (soc_reg_field_valid(unit, ING_HW_RESET_CONTROL_2r, CMIC_REQ_ENABLEf)) {
            soc_reg_field_set(unit, ING_HW_RESET_CONTROL_2r, &hwreset2, CMIC_REQ_ENABLEf, 1);
        }
        rv = WRITE_ING_HW_RESET_CONTROL_2r(unit, hwreset2);
        if (rv < 0) {
            break;
        }
        rv = WRITE_ING_HW_RESET_CONTROL_1r(unit, 0);
        break;

    case SOC_BLK_EPIPE:
    case SOC_BLK_EPIPE_HI:
	hwreset1 = 0;
	soc_reg_field_set(unit, EGR_HW_RESET_CONTROL_0r, &hwreset1,
			  START_ADDRESSf, memoffset);
	soc_reg_field_set(unit, EGR_HW_RESET_CONTROL_0r, &hwreset1,
			  STAGE_NUMBERf, memstage);
	hwreset2 = 0;
	soc_reg_field_set(unit, EGR_HW_RESET_CONTROL_1r, &hwreset2,
			  COUNTf, mementries);
	soc_reg_field_set(unit, EGR_HW_RESET_CONTROL_1r, &hwreset2,
			  VALIDf, 1);

	rv = WRITE_EGR_HW_RESET_CONTROL_0r(unit, hwreset1);
	if (rv < 0) {
	    break;
	}
	rv = WRITE_EGR_HW_RESET_CONTROL_1r(unit, hwreset2);
	if (rv < 0) {
	    break;
	}

	soc_timeout_init(&to, to_usec, 100);
	for (;;) {
	    rv = READ_EGR_HW_RESET_CONTROL_1r(unit, &hwreset2);
	    if (rv < 0) {
		break;
	    }
	    if (soc_reg_field_get(unit, EGR_HW_RESET_CONTROL_1r, hwreset2,
				  DONEf)) {
		break;
	    }
	    if (soc_timeout_check(&to)) {
		to_rv = SOC_E_TIMEOUT;
		break;
	    }
	}

	rv = WRITE_EGR_HW_RESET_CONTROL_1r(unit, 0);
	if (rv < 0) {
	    break;
	}
	rv = WRITE_EGR_HW_RESET_CONTROL_0r(unit, 0);
	break;

    default:
	rv = SOC_E_UNAVAIL;
	break;
    }

    return rv == SOC_E_NONE ? to_rv : rv;
}
#endif /* BCM_XGS3_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */

#if defined(BCM_XGS12_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
/*
 * Function:
 *	_soc_mem_dma
 * Purpose:
 *	DMA acceleration for soc_mem_read_range()
 * Parameters:
 *	buffer -- must be pointer to sufficiently large
 *			DMA-able block of memory
 */
STATIC int
_soc_mem_dma(int unit, soc_mem_t mem, int copyno,
             int index_min, int index_max, void *buffer)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    uint32        start_addr;
    uint32        count, block_count;
    uint32        data_beats;
    int           rv = SOC_E_NONE;
    uint32        cmic_config;
    uint8         at;

    soc_cm_debug(DK_SOCMEM | DK_DMA, "_soc_mem_dma: unit %d"
                 " mem %s.%s index %d-%d buffer %p\n",
                 unit,
                 SOC_MEM_UFNAME(unit, mem), SOC_BLOCK_NAME(unit, copyno),
                 index_min, index_max, buffer);

    data_beats = soc_mem_entry_words(unit, mem);

    assert(data_beats <= SOC_MEM_DMA_MAX_DATA_BEATS);
    assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
    assert(soc_mem_index_valid(unit, mem, index_min));
    assert(soc_mem_index_valid(unit, mem, index_max));
    assert(index_min <= index_max);

    count = index_max - index_min + 1;

    TABLE_DMA_LOCK(unit);

    block_count = 0;

    do {
        /* Compute the starting addresses in hardware */
        /* based upon the previous block DMA, if any. */
        start_addr = soc_mem_addr_get(unit, mem, copyno,
                                      (index_min+=block_count), &at);
        buffer = (uint8 *)buffer + (block_count * data_beats * 4);

        if (count > CMIC_ARL_DMA_CNT_MAX) {
            /* Remaining count exceeds the DMA engine's count limit. */
            /* Do the largest count possible that's divisible by 4.  */
            block_count = CMIC_ARL_DMA_CNT_MAX & (~3);
            count -= block_count;
        }
        else {
            /* Final DMA block transfer */
            block_count = count;
            count = 0;
        }

        if (data_beats == 1) {
            assert(block_count % 4 == 0);
        }
        if (data_beats == 2) {
            assert(block_count % 2 == 0);
        }

        /* Table DMA shares two registers with ARL DMA */
        soc_pci_write(unit, CMIC_ARL_DMA_ADDR, soc_cm_l2p(unit, buffer));
        soc_pci_write(unit, CMIC_ARL_DMA_CNT,
                      block_count | (data_beats << 29));

        soc_pci_write(unit, CMIC_TABLE_DMA_START, start_addr);

        soc_cm_debug(DK_SOCMEM | DK_DMA,
                     "_soc_mem_dma: table dma of %d entries "
                     "of %d beats from 0x%x\n",
                     block_count, data_beats, start_addr);

        /* Clear table DMA abort bit */
        cmic_config = soc_pci_read(unit, CMIC_CONFIG);
        cmic_config &= ~CC_ABORT_TBL_DMA;
        soc_pci_write(unit, CMIC_CONFIG, cmic_config);

        soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_DMA_EN_SET);

        if (soc->tableDmaIntrEnb) {
            /* Table DMA shares an interrupt handler with ARL DMA */

            soc->arl_dma_cnt0 = 0;
            soc->arl_notified = 0;

            soc_intr_enable(unit, IRQ_ARL_DMA_CNT0);

            if (sal_sem_take(soc->arl_notify, soc->tableDmaTimeout) < 0) {
                rv = SOC_E_TIMEOUT;
            }

            soc_intr_disable(unit, IRQ_ARL_DMA_CNT0);
        } else {
            soc_timeout_t to;

            soc_timeout_init(&to, soc->tableDmaTimeout, 10000);

            while ((soc_pci_read(unit, CMIC_SCHAN_CTRL) &
                    SC_ARL_DMA_DONE_TST) == 0) {
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_TIMEOUT;
                    break;
                }
            }
        }

        if (rv < 0) {
            soc_cm_debug(DK_ERR, "_soc_mem_dma: timeout\n");

            /* Abort Table DMA */
            cmic_config = soc_pci_read(unit, CMIC_CONFIG);
            cmic_config |= CC_ABORT_TBL_DMA;
            soc_pci_write(unit, CMIC_CONFIG, cmic_config);

            /* Dummy read to allow abort to finish */
            cmic_config = soc_pci_read(unit, CMIC_CONFIG);

            /* Disable table DMA */
            soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_DMA_EN_CLR);

            /* Clear table DMA abort bit */
            cmic_config &= ~CC_ABORT_TBL_DMA;
            soc_pci_write(unit, CMIC_CONFIG, cmic_config);
            count = 0; /* Skip any subsequent DMA blocks */
        } else {
            soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_DMA_EN_CLR);
        }
    } while (count);

    soc_pci_write(unit, CMIC_SCHAN_CTRL, SC_ARL_DMA_DONE_CLR);

    soc_cm_sinval(unit, (void *)buffer, WORDS2BYTES(data_beats) * block_count);

    TABLE_DMA_UNLOCK(unit);

    return rv;
}
#endif /* BCM_XGS12_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */

#endif /* BCM_XGS_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */

/************************************************************************/
/* Routines for reading/writing all tables (except no write to ARL)	*/
/************************************************************************/

/*
 * Function:
 *	soc_mem_read
 * Purpose:
 *	Read a memory internal to the SOC.
 * Notes:
 *	GBP/CBP memory should only accessed when MMU is in DEBUG mode.
 */

STATIC int
_soc_mem_read(int unit,
              soc_mem_t mem,
              int copyno,
              int index,
              void *entry_data)
{
    schan_msg_t schan_msg;
    int entry_dw = soc_mem_entry_words(unit, mem);
    soc_mem_info_t *meminfo; 
    int index_valid;
    uint32 *cache;
    uint8 at, *vmap;
    int index2;
    uint32 maddr;
    int rv;
    int resp_word = 0;

    if (!soc_mem_is_valid(unit, mem)) {
        return SOC_E_MEMORY;
    }
    meminfo = &SOC_MEM_INFO(unit, mem);

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }

    if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_read: invalid block %d for memory %s\n",
                     copyno, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

#ifdef BCM_DRACO15_SUPPORT
    /* 5695 Rev B specific memories */
    if ((mem == MMU_MEMORIES1_MSTP_TBLm ||
	 mem == MMU_MEMORIES1_ST_PORT_TBLm) &&
	!soc_feature(unit, soc_feature_mstp_lookup)) {
        return SOC_E_PARAM;
    }
    if ((mem == MMU_MEMORIES2_SRC_TRUNK_MAPm ||
	 mem == MMU_MEMORIES2_EGR_TRUNK_MAPm) &&
	!soc_feature(unit, soc_feature_ipmc_lookup)) {
        return SOC_E_PARAM;
    }
#endif /* BCM_DRACO15_SUPPORT */

    /*
     * When checking index, check for 0 instead of soc_mem_index_min.
     * Diagnostics need to read/write index 0 of Strata ARL and GIRULE.
     */
    index_valid = (index >= 0 &&
                   index <= soc_mem_index_max(unit, mem));

    if (!index_valid) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_read: invalid index %d for memory %s\n",
                     index, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

#ifdef BCM_HERCULES_SUPPORT
    /* Handle Hercules' word read tables separately */
    if (SOC_IS_HERCULES(unit) &&
        (soc_mem_flags(unit, mem) & SOC_MEM_FLAG_WORDADR)) {
        return soc_hercules_mem_read(unit, mem, copyno, index, entry_data);
    }
#endif /* BCM_HERCULES_SUPPORT */

    rv = SOC_E_NONE;

    MEM_LOCK(unit, mem);

    /* Return data from cache if active */

    cache = SOC_MEM_STATE(unit, mem).cache[copyno];
    vmap = SOC_MEM_STATE(unit, mem).vmap[copyno];

    if (index_valid && cache != NULL && CACHE_VMAP_TST(vmap, index)) {
        sal_memcpy(entry_data, cache + index * entry_dw, entry_dw * 4);
        goto done;
    }

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_always_drive_dbus) &&
        SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM &&
        soc_mem_is_cam(unit, mem)) {
        static const soc_field_t field_mask[] =
            {TMW0f, TMW1f, TMW2f, TMW3f, TMW4f, TMW5f};
        static const soc_field_t field_data[] =
            {TDW0f, TDW1f, TDW2f, TDW3f, TDW4f, TDW5f};
        uint32 tmp_entry[SOC_MAX_MEM_WORDS];
        uint32  sdata[SOC_MAX_MEM_WORDS], src_hit, dst_hit;
        soc_mem_t real_mem, defip_data_mem, src_hit_mem, dst_hit_mem, mask_mem;
        int raw_index, valid;
        int tcam_width, i;
        uint32 mask[4], data[4];

        rv = soc_tcam_mem_index_to_raw_index(unit, mem, index, &real_mem,
                                             &raw_index);
        if (SOC_FAILURE(rv)) {
            goto done;
        }
        switch (mem) {
        case EXT_ACL360_TCAM_DATAm:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
            mask_mem = EXT_ACL360_TCAM_MASKm;
            break;
        case EXT_ACL432_TCAM_DATAm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:
            mask_mem = EXT_ACL432_TCAM_MASKm;
            break;
        default:
            mask_mem = INVALIDm;
            break;
        }

        defip_data_mem = INVALIDm;
        src_hit_mem = INVALIDm;
        dst_hit_mem = INVALIDm;
        switch (real_mem) {
        case EXT_IPV4_DEFIPm:
            defip_data_mem = EXT_DEFIP_DATA_IPV4m;
            src_hit_mem = EXT_SRC_HIT_BITS_IPV4m;
            dst_hit_mem = EXT_DST_HIT_BITS_IPV4m;
            tcam_width = 1;
            break;
        case EXT_IPV6_64_DEFIPm:
            defip_data_mem = EXT_DEFIP_DATA_IPV6_64m;
            src_hit_mem = EXT_SRC_HIT_BITS_IPV6_64m;
            dst_hit_mem = EXT_DST_HIT_BITS_IPV6_64m;
            tcam_width = 1;
            break;
        case EXT_IPV6_128_DEFIPm:
            defip_data_mem = EXT_DEFIP_DATA_IPV6_128m;
            src_hit_mem = EXT_SRC_HIT_BITS_IPV6_128m;
            dst_hit_mem = EXT_DST_HIT_BITS_IPV6_128m;
            tcam_width = 2;
            break;
        case EXT_L2_ENTRY_TCAMm:
        case EXT_IPV4_DEFIP_TCAMm:
        case EXT_IPV6_64_DEFIP_TCAMm:
            tcam_width = 1;
            break;
        case EXT_IPV6_128_DEFIP_TCAMm:
        case EXT_ACL144_TCAMm:
            tcam_width = 2;
            break;
        case EXT_ACL288_TCAMm:
            tcam_width = 4;
            break;
        case EXT_ACL360_TCAM_DATAm:
            tcam_width = 5;
            break;
        case EXT_ACL432_TCAM_DATAm:
            tcam_width = 6;
            break;
        case EXT_L2_ENTRYm:
        default:
            tcam_width = 0;
            break;
        }

        sal_memcpy(entry_data, soc_mem_entry_null(unit, real_mem),
                   soc_mem_entry_words(unit, real_mem) * sizeof(uint32));

        if (defip_data_mem != INVALIDm) {
            rv = soc_mem_read(unit, defip_data_mem, copyno, index, tmp_entry);
            if (SOC_FAILURE(rv)) {
                goto done;
            }
            soc_mem_field_get(unit, defip_data_mem, tmp_entry, SDATAf, sdata);
            soc_mem_field_set(unit, real_mem, entry_data, SDATAf, sdata);
        }
        if (src_hit_mem != INVALIDm) {
            rv = soc_mem_read(unit, src_hit_mem, copyno, index >> 5,
                              tmp_entry);
            if (SOC_FAILURE(rv)) {
                goto done;
            }
            src_hit = (soc_mem_field32_get(unit, src_hit_mem, tmp_entry,
                                           SRC_HITf) >> (index & 0x1f)) & 1;
            soc_mem_field32_set(unit, real_mem, entry_data, SRC_HITf, src_hit);
        }
        if (dst_hit_mem != INVALIDm) {
            rv = soc_mem_read(unit, dst_hit_mem, copyno, index >> 5,
                              tmp_entry);
            if (SOC_FAILURE(rv)) {
                goto done;
            }
            dst_hit = (soc_mem_field32_get(unit, dst_hit_mem, tmp_entry,
                                           DST_HITf) >> (index & 0x1f)) & 1;
            soc_mem_field32_set(unit, real_mem, entry_data, DST_HITf, dst_hit);
        }
        if (mask_mem != INVALIDm) {
            sal_memcpy(tmp_entry, soc_mem_entry_null(unit, mask_mem),
                       soc_mem_entry_words(unit, mask_mem) * sizeof(uint32));
        }
        if (tcam_width) {
            for (i = 0; i < tcam_width; i++) {
                rv = soc_tcam_read_entry(unit, TCAM_PARTITION_RAW,
                                         raw_index + i, mask, data, &valid);
                if (SOC_FAILURE(rv)) {
                    goto done;
                }
                mask[0] = mask[3];
                mask[3] = mask[1];
                mask[1] = mask[2];
                mask[2] = mask[3];
                if (mask_mem == INVALIDm) {
                    soc_mem_field_set(unit, real_mem, entry_data,
                                      field_mask[i], mask);
                } else {
                    soc_mem_field_set(unit, mask_mem, tmp_entry,
                                      field_mask[i], mask);
                }
                data[0] = data[3];
                data[3] = data[1];
                data[1] = data[2];
                data[2] = data[3];
                soc_mem_field_set(unit, real_mem, entry_data, field_data[i],
                                  data);
            }
            if (mask_mem != INVALIDm) {
                rv = soc_mem_write(unit, mask_mem, copyno, 0, tmp_entry);
            }
            goto done;
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    /* Setup S-Channel command packet */
    schan_msg_clear(&schan_msg);
    schan_msg.readcmd.header.opcode = READ_MEMORY_CMD_MSG;
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.readcmd.header.srcblk = 0;
    } else 
#endif
    {
        schan_msg.readcmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
    /* schan_msg.readcmd.header.srcblk = 0;*/
#endif
    }
    schan_msg.readcmd.header.datalen = 4;	/* Hercules needs it */
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        if (mem == ESBS_PORT_TO_PIPE_MAPPINGm ||
            mem == ISBS_PORT_TO_PIPE_MAPPINGm) {
            schan_msg.readcmd.header.datalen = entry_dw * 4;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    index2 = index;

#if defined(BCM_FIREBOLT_SUPPORT)
#ifdef SOC_MEM_L3_DEFIP_WAR
    if (soc_feature(unit, soc_feature_l3_defip_map) &&
        (mem == L3_DEFIPm ||
         mem == L3_DEFIP_ONLYm ||
         mem == L3_DEFIP_DATA_ONLYm ||
         mem == L3_DEFIP_HIT_ONLYm)) {
        index2 = soc_fb_l3_defip_index_map(unit, index);
    }
#endif
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
    if (soc_feature(unit, soc_feature_l3_defip_hole) &&
       (mem == L3_DEFIPm ||
        mem == L3_DEFIP_ONLYm ||
        mem == L3_DEFIP_DATA_ONLYm ||
        mem == L3_DEFIP_HIT_ONLYm)) {
              index2 = soc_tr2_l3_defip_index_map(unit, index);
    }
#endif                    
    maddr = soc_mem_addr_get(unit, mem, copyno, index2, &at);
    schan_msg.readcmd.address = maddr;
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.readcmd.header.dstblk = SOC_BLOCK2SCH(unit, copyno);
    } else 
#endif
    {
#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
        /* required on XGS3. Optional on other devices */
        schan_msg.readcmd.header.dstblk = ((maddr >> SOC_BLOCK_BP) & 0xf) |
                                  (((maddr >> SOC_BLOCK_MSB_BP) & 0x1) << 4);
#endif /* BCM_XGS3_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT*/
    }

#ifdef BCM_TRIUMPH_SUPPORT
    if ((!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM) &&
        soc_feature(unit, soc_feature_esm_support)) {
        /* Following tables have response word in the s-chan message.
           we probably should invent SOC_MEM_FLAG_HAS_RESP_WORD in regsfile
           for such tables (or define a bit in S-chan message header if
           hardware can do that) */
        switch (mem) {
        case EXT_L2_ENTRYm:
        case EXT_L2_ENTRY_TCAMm:
        case EXT_L2_ENTRY_DATAm:
        case EXT_DEFIP_DATAm:
        case EXT_IPV4_DEFIP_TCAMm:
        case EXT_IPV4_DEFIPm:
        case EXT_IPV6_64_DEFIP_TCAMm:
        case EXT_IPV6_64_DEFIPm:
        case EXT_IPV6_128_DEFIP_TCAMm:
        case EXT_IPV6_128_DEFIPm:
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
        case EXT_FP_CNTR8m:
        case EXT_ACL144_TCAMm:
        case EXT_ACL288_TCAMm:
        case EXT_ACL360_TCAM_DATAm: /* not for MASK portion */
        case EXT_ACL432_TCAM_DATAm: /* not for MASK portion */
        /* and following software invented tables as well */
        case EXT_DEFIP_DATA_IPV4m:
        case EXT_DEFIP_DATA_IPV6_64m:
        case EXT_DEFIP_DATA_IPV6_128m:
        case EXT_FP_POLICY_ACL288_L2m:
        case EXT_FP_POLICY_ACL288_IPV4m:
        case EXT_FP_POLICY_ACL360_IPV6_SHORTm:
        case EXT_FP_POLICY_ACL432_IPV6_LONGm:
        case EXT_FP_POLICY_ACL144_L2m:
        case EXT_FP_POLICY_ACL144_IPV4m:
        case EXT_FP_POLICY_ACL144_IPV6m:
        case EXT_FP_POLICY_ACL432_L2_IPV4m:
        case EXT_FP_POLICY_ACL432_L2_IPV6m:
        case EXT_FP_CNTR_ACL288_L2m:
        case EXT_FP_CNTR_ACL288_IPV4m:
        case EXT_FP_CNTR_ACL360_IPV6_SHORTm:
        case EXT_FP_CNTR_ACL432_IPV6_LONGm:
        case EXT_FP_CNTR_ACL144_L2m:
        case EXT_FP_CNTR_ACL144_IPV4m:
        case EXT_FP_CNTR_ACL144_IPV6m:
        case EXT_FP_CNTR_ACL432_L2_IPV4m:
        case EXT_FP_CNTR_ACL432_L2_IPV6m:
        case EXT_FP_CNTR8_ACL288_L2m:
        case EXT_FP_CNTR8_ACL288_IPV4m:
        case EXT_FP_CNTR8_ACL360_IPV6_SHORTm:
        case EXT_FP_CNTR8_ACL432_IPV6_LONGm:
        case EXT_FP_CNTR8_ACL144_L2m:
        case EXT_FP_CNTR8_ACL144_IPV4m:
        case EXT_FP_CNTR8_ACL144_IPV6m:
        case EXT_FP_CNTR8_ACL432_L2_IPV4m:
        case EXT_FP_CNTR8_ACL432_L2_IPV6m:
        case EXT_ACL288_TCAM_L2m:
        case EXT_ACL288_TCAM_IPV4m:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_ACL144_TCAM_L2m:
        case EXT_ACL144_TCAM_IPV4m:
        case EXT_ACL144_TCAM_IPV6m:
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:

            resp_word = 1;
            break;
        default:
            break;
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit) && (!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM)) {
	schan_msg.readcmd.header.srcblk = 0;
	schan_msg.readcmd.header.datalen = 0;
	/* mask off the block field */
	schan_msg.readcmd.address &= 0x3F0FFFFF;
    }
#endif

    /*
     * Write onto S-Channel "memory read" command packet consisting of header
     * word + address word, and read back header word + entry_dw data words.
     */

    rv = soc_schan_op(unit, &schan_msg, 2, 1 + entry_dw + resp_word, 0);
    if (SOC_FAILURE(rv)) {
        goto done;
    }

    if (schan_msg.readresp.header.opcode != READ_MEMORY_ACK_MSG) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_read: "
                     "invalid S-Channel reply, expected READ_MEMORY_ACK:\n");
        soc_schan_dump(unit, &schan_msg, 1 + entry_dw + resp_word);
        rv = SOC_E_INTERNAL;
	goto done;
    }

    sal_memcpy(entry_data,
               resp_word ? schan_msg.genresp.data : schan_msg.readresp.data,
               entry_dw * sizeof (uint32));

    /*
     * Reconstruct the FSEL field for Tucana.  This field is in the
     * regsfile to help with software, but does not exist in the chip.
     */

#if defined(BCM_TUCANA_SUPPORT)
    if (SOC_IS_TUCANA(unit) &&
	SOC_PERSIST(unit)->filter_enable &&
	(mem == FILTER_IRULEm || mem == GFILTER_IRULEm)) {
	int fsel;
	if (_soc_mem_frule_to_mask(unit, mem, copyno, index, &fsel) >= 0) {
	    soc_mem_field32_set(unit, mem, entry_data, FSELf, fsel);
	}
    }
#endif /* BCM_TUCANA_SUPPORT */

#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_KATANA_SUPPORT)
    if (soc_feature(unit, soc_feature_xy_tcam_direct) &&
        (meminfo->flags & SOC_MEM_FLAG_CAM) &&
        (!(meminfo->flags & SOC_MEM_FLAG_EXT_CAM))) {
        uint32 key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
        soc_field_t key_field[4], mask_field[4];
        int field_count, bit_length, word_length, i, word;
        soc_pbmp_t pbmp;

        if (mem == L3_DEFIPm || mem == L3_DEFIP_Xm || mem == L3_DEFIP_Ym ||
            mem == L3_DEFIP_ONLYm) {
            key_field[0] = KEY0f;
            key_field[1] = KEY1f;
            mask_field[0] = MASK0f;
            mask_field[1] = MASK1f;
            field_count = 2;
#if defined(BCM_KATANA_SUPPORT)
        } else if (mem == L3_DEFIP_PAIR_128m || mem == L3_DEFIP_PAIR_128_ONLYm) {
            key_field[0] = KEY0_UPRf;
            key_field[1] = KEY1_UPRf;
            key_field[2] = KEY0_LWRf;
            key_field[3] = KEY1_LWRf;
            mask_field[0] = MASK0_UPRf;
            mask_field[1] = MASK1_UPRf;
            mask_field[2] = MASK0_LWRf;
            mask_field[3] = MASK1_LWRf;
            field_count = 4;
#endif
        } else {
            if (soc_mem_field_valid(unit, mem, FULL_KEYf)) {
                key_field[0] = FULL_KEYf;
                mask_field[0] = FULL_MASKf;
            } else {
                key_field[0] = KEYf;
                mask_field[0] = MASKf;
            }
            field_count = 1;
        }
        for (i = 0; i < field_count; i++) {
            soc_mem_field_get(unit, mem, entry_data, key_field[i], key);
            soc_mem_field_get(unit, mem, entry_data, mask_field[i], mask);
            bit_length = soc_mem_field_length(unit, mem, key_field[i]);
            word_length = (bit_length + 31) / 32;
            for (word = 0; word < word_length; word++) {
                mask[word] = key[word] | ~mask[word];
            }
            if ((bit_length & 0x1f) != 0) {
                mask[word - 1] &= (1 << (bit_length & 0x1f)) - 1;
            }
            soc_mem_field_set(unit, mem, entry_data, mask_field[i], mask);
        }
        if (SOC_IS_TD_TT(unit) &&
            (mem == FP_GLOBAL_MASK_TCAM_Xm || mem == FP_GLOBAL_MASK_TCAM_Ym)) {
            soc_mem_pbmp_field_get(unit, mem, entry_data, IPBM_MASKf, &pbmp);
            if (mem == FP_GLOBAL_MASK_TCAM_Xm) {
                SOC_PBMP_AND(pbmp, PBMP_XPIPE(unit));
            } else {
                SOC_PBMP_AND(pbmp, PBMP_YPIPE(unit));
            }
            soc_mem_pbmp_field_set(unit, mem, entry_data, IPBM_MASKf, &pbmp);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT, BCM_KATANA_SUPPORT */

    /*
     * Add read data to memory cache if it is enabled.
     */

    if (index_valid && cache != NULL) {
	sal_memcpy(cache + index * entry_dw, entry_data, entry_dw * 4);
	CACHE_VMAP_SET(vmap, index);
    }

 done:

    MEM_UNLOCK(unit, mem);


    if (NULL != meminfo->snoop_cb && 
         (SOC_MEM_SNOOP_READ & meminfo->snoop_flags)) {
         meminfo->snoop_cb(unit, mem, SOC_MEM_SNOOP_READ, copyno, index, index, 
                           entry_data, meminfo->snoop_user_data);
    }
    if (soc_cm_debug_check(DK_MEM)) {
        soc_cm_print("soc_mem_read unit %d: %s.%s[%d]: ",
                     unit, SOC_MEM_NAME(unit, mem),
                     SOC_BLOCK_NAME(unit, copyno), index);
        soc_mem_entry_dump(unit, mem, entry_data);
        soc_cm_print("\n");
    }

    return rv;
}

int
soc_mem_read(int unit,
             soc_mem_t mem,
             int copyno,
             int index,
             void *entry_data)
{
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        uint32 entry_data_y[SOC_MAX_MEM_WORDS];
        soc_pbmp_t pbmp_x, pbmp_y;

        if (mem == EGR_VLANm) {
            soc_trident_pipe_select(unit, TRUE, 1); /* Y pipe */
            SOC_IF_ERROR_RETURN
                (_soc_mem_read(unit, mem, copyno, index, entry_data_y));
            soc_trident_pipe_select(unit, TRUE, 0); /* X pipe */
            SOC_IF_ERROR_RETURN
                (_soc_mem_read(unit, mem, copyno, index, entry_data));
            soc_mem_pbmp_field_get(unit, mem, entry_data, PORT_BITMAPf,
                                   &pbmp_x);
            soc_mem_pbmp_field_get(unit, mem, entry_data_y, PORT_BITMAPf,
                                   &pbmp_y);
            BCM_PBMP_OR(pbmp_x, pbmp_y);
            soc_mem_pbmp_field_set(unit, mem, entry_data, PORT_BITMAPf,
                                   &pbmp_x);
            soc_mem_pbmp_field_get(unit, mem, entry_data, UT_PORT_BITMAPf,
                                   &pbmp_x);
            soc_mem_pbmp_field_get(unit, mem, entry_data_y, UT_PORT_BITMAPf,
                                   &pbmp_y);
            BCM_PBMP_OR(pbmp_x, pbmp_y);
            soc_mem_pbmp_field_set(unit, mem, entry_data, UT_PORT_BITMAPf,
                                   &pbmp_x);
            return SOC_E_NONE;
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    return _soc_mem_read(unit, mem, copyno, index, entry_data);
}

/*
 * Function:
 *	soc_mem_read_range
 * Purpose:
 *	Read a range of chip's memory
 * Parameters:
 *	buffer -- Pointer to block of sufficiently large memory.
 *                  For Draco DMA operations, this memory must be
 *                  DMA-able
 * Notes:
 *	Table DMA only works on tables whose entry is less than
 *	SOC_MEM_DMA_MAX_DATA_BEATS words long.
 *
 *	Table DMA has a minimum transaction size of 4 words, so if the
 *	table entry is 1 or 2 words, then the count of words is modified
 *	to keep this alignment.  For the remainder of the entries, this
 *	function reads in the remainder of the data through mem_read
 *	without using DMA.
 */

int
soc_mem_read_range(int unit, soc_mem_t mem, int copyno,
                   int index_min, int index_max, void *buffer)
{
    uint32 dw;
    int index;
    int count;
    void *buf;
    soc_mem_info_t  *meminfo;
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    int index_max1;
#endif /* BCM_XGS12_SWITCH_SUPPORT */

    if (!soc_mem_is_valid(unit, mem)) {
        return SOC_E_MEMORY;
    }

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }


    assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
    assert(soc_mem_index_valid(unit, mem, index_min));
    assert(soc_mem_index_valid(unit, mem, index_max));
    assert(index_min <= index_max);
    assert(buffer != NULL);

    meminfo = &SOC_MEM_INFO(unit, mem);
    dw = soc_mem_entry_words(unit, mem);
    count = 0;

    soc_cm_debug(DK_SOCMEM,
                 "soc_mem_read_range: unit %d memory %s.%s [%d:%d]\n",
                 unit, SOC_MEM_UFNAME(unit, mem),
                 SOC_BLOCK_NAME(unit, copyno),
                 index_min, index_max);

    /* If device is gone fill buffer with null entries. */
    if (SOC_IS_DETACHING(unit)) {
        buf = buffer;
        for (index = index_min; index <= index_max; index++, count++) {
            buf = soc_mem_table_idx_to_pointer(unit, mem, void *,
                                               buffer, count);
            sal_memcpy(buf, soc_mem_entry_null(unit, mem),
                       soc_mem_entry_bytes(unit, mem));
        }
        return (SOC_E_NONE);
    }

#if defined(BCM_XGS_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (soc_mem_dmaable(unit, mem, copyno)) {
#if defined(BCM_SIRIUS_SUPPORT) || defined (BCM_XGS3_SWITCH_SUPPORT)

        if (SOC_IS_SIRIUS(unit) || SOC_IS_XGS3_SWITCH(unit)) {

            SOC_IF_ERROR_RETURN(
                _soc_xgs3_mem_dma(unit, mem, copyno, index_min, index_max,
                                  buffer));
            if (NULL != meminfo->snoop_cb && 
                (SOC_MEM_SNOOP_READ & meminfo->snoop_flags)) {
                meminfo->snoop_cb(unit, mem, SOC_MEM_SNOOP_READ, copyno,
                                  index_min, index_max, buffer,
                                  meminfo->snoop_user_data);
            }
            return SOC_E_NONE;
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT BCM_SIRIUS_SUPPORT */
#if defined(BCM_XGS12_SWITCH_SUPPORT)
        count = index_max - index_min + 1;

        if (dw == 1) {
            /* dma entry count should be divisible by 4 */
            count -= (count % 4);
        }

        if (dw == 2) {
            /* dma entry count should be divisible by 2 */
            count -= (count % 2);
        }

        if (count != 0) {
            index_max1 = index_min + count - 1;

            SOC_IF_ERROR_RETURN
                (_soc_mem_dma(unit, mem, copyno,
                              index_min, index_max1, buffer));

            if (NULL != meminfo->snoop_cb && 
                (SOC_MEM_SNOOP_READ & meminfo->snoop_flags)) {
                meminfo->snoop_cb(unit, mem, SOC_MEM_SNOOP_READ, copyno,
                                  index_min, index_max1, buffer,
                                  meminfo->snoop_user_data);
            }

            index_min = index_max1 + 1;
        }
#endif /* BCM_XGS12_SWITCH_SUPPORT */
    }
#endif /* (BCM_XGS_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */
    /* For the rest of entries, do direct read */
    for (index = index_min; index <= index_max; index++, count++) {
        buf = soc_mem_table_idx_to_pointer(unit, mem, void *,
                                           buffer, count);
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, copyno, index, buf));
    }

    return SOC_E_NONE;
}

#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *	soc_scache_mem_read_range
 * Purpose:
 *	Read a range of chip's memory used for L2 Warm Boot
 * Parameters:
 *	buffer -- Pointer to block of sufficiently large memory.
 *                  For DMA operations, this memory must be
 *                  DMA-able
 * Notes:
 *	This function is only used for DMA'ing the scache from the
 *      chip's internal memory.
 */

STATIC int
soc_scache_mem_read_range(int unit, soc_mem_t mem, int copyno,
                          int index_min, int index_max, void *buffer)
{
    uint32 dw;
    int index;
    int count;
    void *buf;
    soc_mem_info_t  *meminfo;

    if (!soc_mem_is_valid(unit, mem)) {
        return SOC_E_MEMORY;
    }

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }

    assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
    assert(buffer != NULL);

    meminfo = &SOC_MEM_INFO(unit, mem);
    dw = soc_mem_entry_words(unit, mem);
    count = 0;

    soc_cm_debug(DK_SOCMEM,
                 "soc_scache_mem_read_range: unit %d memory %s.%s [%d:%d]\n",
                 unit, SOC_MEM_UFNAME(unit, mem),
                 SOC_BLOCK_NAME(unit, copyno),
                 index_min, index_max);

    /* If device is gone fill buffer with null entries. */
    if (SOC_IS_DETACHING(unit)) {
        buf = buffer;
        for (index = index_min; index <= index_max; index++, count++) {
            buf = soc_mem_table_idx_to_pointer(unit, mem, void *,
                                               buffer, count);
            sal_memcpy(buf, soc_mem_entry_null(unit, mem),
                       soc_mem_entry_bytes(unit, mem));
        }
        return (SOC_E_NONE);
    }

    if (soc_mem_dmaable(unit, mem, copyno)) {
        if (SOC_IS_XGS3_SWITCH(unit)) {
            SOC_IF_ERROR_RETURN(
                _soc_xgs3_mem_dma(unit, mem, copyno, index_min, index_max,
                                  buffer));
            if (NULL != meminfo->snoop_cb && 
                (SOC_MEM_SNOOP_READ & meminfo->snoop_flags)) {
                meminfo->snoop_cb(unit, mem, SOC_MEM_SNOOP_READ, copyno,
                                  index_min, index_max, buffer,
                                  meminfo->snoop_user_data);
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_scache_mem_write_range
 * Purpose:
 *	Write a range of chip's memory used for L2 Warm Boot
 * Parameters:
 *	buffer -- Pointer to block of sufficiently large memory.
 *                For slam operations, this memory must be
 *                slammable
 * Notes:
 *	This function is only used for slamming the scache to the
 *      chip's internal memory.
 */
STATIC int
soc_scache_mem_write_range(int unit, soc_mem_t mem, int copyno,
                           int index_min, int index_max, void *buffer)
{
    uint32          entry_dw;
    int             i;
    int             rv = SOC_E_NONE;
    soc_mem_info_t  *meminfo;

    if (!soc_mem_is_valid(unit, mem)) {
        return SOC_E_MEMORY;
    }

    meminfo = &SOC_MEM_INFO(unit, mem);
    entry_dw = soc_mem_entry_words(unit, mem);

    soc_cm_debug(DK_SOCMEM,
                 "soc_scache_mem_write_range: unit %d memory %s.%s [%d:%d]\n",
                 unit, SOC_MEM_UFNAME(unit, mem),
                 SOC_BLOCK_NAME(unit, copyno),
                 index_min, index_max);

    /*    coverity[negative_returns : FALSE]    */
    if (SOC_IS_XGS3_SWITCH(unit) && soc_mem_slamable(unit, mem, copyno)) {
        int blk;

        if (copyno == COPYNO_ALL) {
            SOC_MEM_BLOCK_ITER(unit, mem, blk) {
                copyno = blk;
                break;
            }
        }

	MEM_LOCK(unit, mem);
	if ((rv = _soc_xgs3_mem_slam(unit, mem, copyno,
				     index_min, index_max, buffer)) >= 0) {
	    uint32 *cache;
	    uint8 *vmap;
           /* coverity[negative_returns : FALSE]    */
           /* coverity[negative_returns] */
	    cache = SOC_MEM_STATE(unit, mem).cache[copyno];
	    vmap = SOC_MEM_STATE(unit, mem).vmap[copyno];

	    if (cache != NULL) {
		sal_memcpy(cache + index_min * entry_dw, buffer,
			   (index_max - index_min + 1) * entry_dw * 4);

		for (i = index_min; i <= index_max; i++) {
		    CACHE_VMAP_SET(vmap, i);
		}
	    }
	}

	MEM_UNLOCK(unit, mem);
        if (NULL != meminfo->snoop_cb && 
            (SOC_MEM_SNOOP_WRITE & meminfo->snoop_flags)) {
            meminfo->snoop_cb(unit, mem, SOC_MEM_SNOOP_WRITE, copyno, index_min, 
                              index_max, buffer, meminfo->snoop_user_data);
        }
    }
    return rv;
}

int
soc_scache_esw_nh_mem_write(int unit, uint8 *buf, int offset, int nbytes) 
{
    int num_entries, bytes2,  rv = SOC_E_NONE;
    int stable_size;
    uint32 stable_index_max, stable_index_min;
    int i, j, bufptr;
    uint8 *buf2; 
    int start, end;
    int scache_offset;
    uint8 mask[SOC_MAX_MEM_BYTES];

    SOC_IF_ERROR_RETURN(
        soc_stable_tmp_access(unit, sf_index_min, &stable_index_min, TRUE));
    SOC_IF_ERROR_RETURN(
        soc_stable_tmp_access(unit, sf_index_max, &stable_index_max, TRUE));
    SOC_IF_ERROR_RETURN(
        soc_stable_size_get(unit, &stable_size));

    bytes2 = soc_mem_entry_words(unit, EGR_L3_NEXT_HOPm) * sizeof(uint32);
    num_entries = stable_index_max - stable_index_min + 1;

    start = stable_index_min + offset / (bytes2);

    scache_offset = offset % (bytes2);
    end = start + (nbytes + scache_offset + bytes2 - 1) / (bytes2);

    buf2 = NULL;
    /* Allocate slammable memory */
    buf2 = soc_cm_salloc(unit, bytes2 * num_entries, "EGR_L3_NEXT_HOP buffer");
    if (NULL == buf2) {
        rv = SOC_E_MEMORY;
        goto cleanup;
    }
    sal_memset(buf2, 0, bytes2 * num_entries);

    /* Copy the buffer to the individual memories */
    bufptr = 0;
    sal_memset(mask, 0, SOC_MAX_MEM_BYTES); 
    soc_mem_datamask_get(unit, EGR_L3_NEXT_HOPm, (uint32 *)mask);

    SOC_IF_ERROR_RETURN
        (soc_scache_mem_read_range(unit, EGR_L3_NEXT_HOPm, MEM_BLOCK_ANY,
                                   start, start, buf2));
    if (start != end) {
        SOC_IF_ERROR_RETURN
            (soc_scache_mem_read_range(unit, EGR_L3_NEXT_HOPm, MEM_BLOCK_ANY,
                                       end, end, buf2 + end * bytes2));
    }

    for (i = 0; i <= (end - start); i++) {
        if (bufptr < stable_size) {
            for (j = (i == 0) ? scache_offset : 0; j < SOC_MAX_MEM_BYTES; j++) {
                if (mask[j] != 0xff) {
                    continue;
                }
                /*
                sal_memcpy(buf2 + (i - stable_index_min) * bytes2 + j, 
                           (buf + bufptr), 1);
                */
                *(buf2 + i * bytes2 + j) = *(buf + bufptr);
                bufptr++;
            }
        }
    }

    rv = soc_scache_mem_write_range(unit, EGR_L3_NEXT_HOPm, MEM_BLOCK_ALL,  
                                    start, end, buf2);
cleanup:
    if (buf2 != NULL) {
        soc_cm_sfree(unit, buf2);
    }
    return rv;
}

int
soc_scache_esw_nh_mem_read(int unit, uint8 *buf, int offset, int nbytes)
{
    int i, num_entries, bytes2, rv = SOC_E_NONE;
    int j, bufptr;
    int stable_size;
    uint32 stable_index_min, stable_index_max;
    uint8 *buf2; 
    int start, end;
    int scache_offset;
    uint8 mask[SOC_MAX_MEM_BYTES];

    SOC_IF_ERROR_RETURN(
        soc_stable_tmp_access(unit, sf_index_min, &stable_index_min, TRUE));
    SOC_IF_ERROR_RETURN(
        soc_stable_tmp_access(unit, sf_index_max, &stable_index_max, TRUE));
    SOC_IF_ERROR_RETURN(
        soc_stable_size_get(unit, &stable_size));

    bytes2 = soc_mem_entry_words(unit, EGR_L3_NEXT_HOPm) * sizeof(uint32);
    num_entries = stable_index_max - stable_index_min + 1;

    buf2 = NULL;
    start = stable_index_min + offset / (bytes2);

    scache_offset = offset % (bytes2);
    end = start + (nbytes + scache_offset + bytes2 - 1) / (bytes2);

    /* Allocate DMA'able memory */
    buf2 = soc_cm_salloc(unit, bytes2 * num_entries, "EGR_L3_NEXT_HOP buffer");
    if (NULL == buf2) {
        rv = SOC_E_MEMORY;
        goto cleanup;
    }
    sal_memset(buf2, 0, bytes2 * num_entries);

    /* DMA the tables */
    rv = soc_scache_mem_read_range(unit, EGR_L3_NEXT_HOPm, MEM_BLOCK_ANY,  
                                   stable_index_min, stable_index_max, buf2);

    /* Copy out the individual buffers to the scache buffer */
    bufptr = 0;
    sal_memset(mask, 0, SOC_MAX_MEM_BYTES);
    soc_mem_datamask_get(unit, EGR_L3_NEXT_HOPm, (uint32 *)mask);
    for (i = 0; i <= (end - start); i++) {
        if (bufptr < stable_size) {
            for (j = (i == 0) ? scache_offset : 0; j < SOC_MAX_MEM_BYTES; j++) {
                if (mask[j] != 0xff) {
                    continue;
                }
                /*
                sal_memcpy((buf + offset + bufptr), 
                            buf2 + (i - stable_index_min) * bytes2 + j, 1);
                */
                *(buf + bufptr) = *(buf2 + i * bytes2 + j);
                bufptr++;
            }
        }
    }
cleanup:
    if (buf2 != NULL) {
        soc_cm_sfree(unit, buf2);
    }
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
#endif /* defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */

/*
 * Function:
 *	_soc_mem_write
 * Purpose:
 *	Write a memory internal to the SOC.
 * Notes:
 *	GBP/CBP memory should only accessed when MMU is in DEBUG mode.
 */

STATIC int
_soc_mem_write(int unit,
               soc_mem_t mem,
               int copyno,	/* Use COPYNO_ALL for all */
               int index,
               void *entry_data)
{
    schan_msg_t schan_msg;
    int blk;
    soc_mem_info_t *meminfo; 
    int entry_dw = soc_mem_entry_words(unit, mem);
    uint32 *cache;
    uint32 maddr;
    uint8 at, *vmap;
    int no_cache;
    int index_valid, index2;
    int rv;
    void *entry_data_ptr;
#ifdef BCM_TRIDENT_SUPPORT
    uint32 converted_entry_data[SOC_MAX_MEM_WORDS];
#endif /* BCM_TRIDENT_SUPPORT */

    if (!soc_mem_is_valid(unit, mem)) {
        return SOC_E_MEMORY;
    }
    meminfo = &SOC_MEM_INFO(unit, mem);

    if (index < 0) {
	index = -index;		/* Negative index bypasses cache for debug */
	no_cache = 1;
    } else {
	no_cache = 0;
    }

    if (meminfo->flags & SOC_MEM_FLAG_READONLY) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_write: attempt to write R/O memory %s\n",
                     SOC_MEM_NAME(unit, mem));
        return SOC_E_INTERNAL;
    }

#ifdef BCM_HERCULES_SUPPORT
    /* Handle Hercules' word read tables separately */
    if (meminfo->flags & SOC_MEM_FLAG_WORDADR) {
        return soc_hercules_mem_write(unit, mem, copyno, index, entry_data);
    }
#endif /* BCM_HERCULES_SUPPORT */

#ifdef BCM_DRACO15_SUPPORT
    /* 5695 Rev B specific memories */
    if ((mem == MMU_MEMORIES1_MSTP_TBLm ||
	 mem == MMU_MEMORIES1_ST_PORT_TBLm) &&
	!soc_feature(unit, soc_feature_mstp_lookup)) {
        return SOC_E_PARAM;
    }

    if ((mem == MMU_MEMORIES2_SRC_TRUNK_MAPm ||
	 mem == MMU_MEMORIES2_EGR_TRUNK_MAPm) &&
	!soc_feature(unit, soc_feature_ipmc_lookup)) {
        return SOC_E_PARAM;
    }
#endif /* BCM_DRACO15_SUPPORT */

    entry_data_ptr = entry_data;
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_KATANA_SUPPORT) \
    || defined(BCM_SHADOW_SUPPORT)
    if (soc_feature(unit, soc_feature_xy_tcam) &&
        (meminfo->flags & SOC_MEM_FLAG_CAM) &&
        (!(meminfo->flags & SOC_MEM_FLAG_EXT_CAM))) {
        uint32 key[SOC_MAX_MEM_FIELD_WORDS], mask[SOC_MAX_MEM_FIELD_WORDS];
        uint32 converted_key, converted_mask;
        soc_field_t key_field[4], mask_field[4];
        int field_count, bit_length, word_length, i, word;
        uint8 no_trans = FALSE;  

        if (!soc_feature(unit, soc_feature_xy_tcam_direct)) {
            /* Only clear the "don't care" key bits */
            no_trans = TRUE;
        }
        if (mem == L3_DEFIPm || mem == L3_DEFIP_Xm || mem == L3_DEFIP_Ym ||
            mem == L3_DEFIP_ONLYm) {
            key_field[0] = KEY0f;
            key_field[1] = KEY1f;
            mask_field[0] = MASK0f;
            mask_field[1] = MASK1f;
            field_count = 2;
#if defined(BCM_KATANA_SUPPORT)
        } else if (mem == L3_DEFIP_PAIR_128m || mem == L3_DEFIP_PAIR_128_ONLYm) {
            key_field[0] = KEY0_UPRf;
            key_field[1] = KEY1_UPRf;
            key_field[2] = KEY0_LWRf;
            key_field[3] = KEY1_LWRf;
            mask_field[0] = MASK0_UPRf;
            mask_field[1] = MASK1_UPRf;
            mask_field[2] = MASK0_LWRf;
            mask_field[3] = MASK1_LWRf;
            field_count = 4;
#endif
        } else {
            if (soc_mem_field_valid(unit, mem, FULL_KEYf)) {
                key_field[0] = FULL_KEYf;
                mask_field[0] = FULL_MASKf;
            } else {
                key_field[0] = KEYf;
                mask_field[0] = MASKf;
            }
            field_count = 1;
        }
#ifdef BCM_TRIDENT_SUPPORT
        sal_memcpy(converted_entry_data, entry_data,
                   entry_dw * sizeof(uint32));
        entry_data_ptr = converted_entry_data;
#endif /* BCM_TRIDENT_SUPPORT */

        for (i = 0; i < field_count; i++) {
            soc_mem_field_get(unit, mem, entry_data, key_field[i], key);
            soc_mem_field_get(unit, mem, entry_data, mask_field[i], mask);
            bit_length = soc_mem_field_length(unit, mem, key_field[i]);
            word_length = (bit_length + 31) / 32;
            for (word = 0; word < word_length; word++) {
                converted_key = key[word] & mask[word];
                if (!no_trans) {
                    converted_mask = key[word] | ~mask[word];
                    mask[word] = converted_mask;
                } 
                key[word] = converted_key;
            }
            if ((bit_length & 0x1f) != 0) {
                mask[word - 1] &= (1 << (bit_length & 0x1f)) - 1;
            }
            soc_mem_field_set(unit, mem, entry_data_ptr, key_field[i], key);
            soc_mem_field_set(unit, mem, entry_data_ptr, mask_field[i], mask);
        }
    }
#endif /* BCM_TRIDENT_SUPPORT, BCM_KATANA_SUPPORT */
    /*
     * Setup S-Channel command packet
     *
     * NOTE: the datalen field matters only for the Write Memory and
     * Write Register commands, where it is used only by the CMIC to
     * determine how much data to send, and is in units of bytes.
     */

    schan_msg_clear(&schan_msg);
    schan_msg.writecmd.header.opcode = WRITE_MEMORY_CMD_MSG;
    schan_msg.writecmd.header.datalen = entry_dw * sizeof (uint32);
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.writecmd.header.srcblk = 0;
    } else 
#endif
    {
        schan_msg.writecmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
    /* schan_msg.writecmd.header.srcblk = 0;*/
#endif

#ifdef BCM_SIRIUS_SUPPORT
        schan_msg.writecmd.header.srcblk = (SOC_IS_SIRIUS(unit) ? 0 : \
                                            schan_msg.writecmd.header.srcblk);
#endif
    }
    sal_memcpy(schan_msg.writecmd.data,
               entry_data_ptr,
               entry_dw * sizeof (uint32));

    if (copyno != COPYNO_ALL) {
        if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
            soc_cm_debug(DK_WARN,
                         "soc_mem_write: invalid block %d for memory %s\n",
                         copyno, SOC_MEM_NAME(unit, mem));
            return SOC_E_PARAM;
        }
    }

    /*
     * When checking index, check for 0 instead of soc_mem_index_min.
     * Diagnostics need to read/write index 0 of Strata ARL and GIRULE.
     */

    index_valid = (index >= 0 &&
                   index <= soc_mem_index_max(unit, mem));

    if (soc_cm_debug_check(DK_MEM)) {
        soc_cm_print("soc_mem_write unit %d: %s.%s[%d]: ",
                     unit, SOC_MEM_NAME(unit, mem),
                     SOC_BLOCK_NAME(unit, copyno), index);
        soc_mem_entry_dump(unit, mem, entry_data_ptr);
        soc_cm_print("\n");
    }

    /* Write to one or all copies of the memory */

    rv = SOC_E_NONE;

    MEM_LOCK(unit, mem);

    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }

        index2 = index;

#if defined(BCM_FIREBOLT_SUPPORT)
#ifdef SOC_MEM_L3_DEFIP_WAR
        if (soc_feature(unit, soc_feature_l3_defip_map) &&
            (mem == L3_DEFIPm ||
             mem == L3_DEFIP_ONLYm ||
             mem == L3_DEFIP_DATA_ONLYm ||
             mem == L3_DEFIP_HIT_ONLYm)) {
            index2 = soc_fb_l3_defip_index_map(unit, index);
        }
#endif
#endif

#ifdef BCM_TRIUMPH2_SUPPORT
        if (soc_feature(unit, soc_feature_l3_defip_hole) &&
           (mem == L3_DEFIPm ||
            mem == L3_DEFIP_ONLYm ||
            mem == L3_DEFIP_DATA_ONLYm ||
            mem == L3_DEFIP_HIT_ONLYm)) {
                   index2 = soc_tr2_l3_defip_index_map(unit, index);
        }
#endif
        maddr = soc_mem_addr_get(unit, mem, blk, index2, &at);
        schan_msg.writecmd.address = maddr;
#ifdef BCM_EXTND_SBUS_SUPPORT
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            schan_msg.writecmd.header.dstblk = SOC_BLOCK2SCH(unit, blk);
        } else 
#endif
        {
#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
            /* required on XGS3. Optional on other devices */
            schan_msg.writecmd.header.dstblk = ((maddr >> SOC_BLOCK_BP)
                 & 0xf) | (((maddr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
#endif /* BCM_XGS3_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT*/
        }

#ifdef BCM_SIRIUS_SUPPORT
	if (SOC_IS_SIRIUS(unit) && (!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM)) {
	    /* mask off the block field */
	    schan_msg.writecmd.address &= 0x3F0FFFFF;
	}
#endif

        /* Write header + address + entry_dw data DWORDs */
        /* Note: The hardware does not send WRITE_MEMORY_ACK_MSG. */

        if ((rv = soc_schan_op(unit, &schan_msg,
			       2 + entry_dw, 0, 0)) < 0) {
	    goto done;
	}

        /* Write back to cache if active */

        cache = SOC_MEM_STATE(unit, mem).cache[blk];
        vmap = SOC_MEM_STATE(unit, mem).vmap[blk];

        if (index_valid && cache != NULL && !no_cache) {
            sal_memcpy(cache + index * entry_dw, entry_data_ptr, entry_dw * 4);
            CACHE_VMAP_SET(vmap, index);
	}

#ifdef BCM_FILTER_SUPPORT
        if ((mem == FILTER_IRULEm || mem == GFILTER_IRULEm) &&
            SOC_PERSIST(unit)->filter_enable &&
            SOC_PERSIST(unit)->filter_update &&
            !SOC_IS_LYNX(unit)) {
	    if ((rv =
		 _soc_mem_frule_written(unit, mem, blk, index,
					(irule_entry_t *)entry_data_ptr)) < 0) {
		goto done;
	    }
        }
#endif /* BCM_FILTER_SUPPORT */
    }

 done:

    MEM_UNLOCK(unit, mem);
    if (NULL != meminfo->snoop_cb && 
        (SOC_MEM_SNOOP_WRITE & meminfo->snoop_flags)) {
        meminfo->snoop_cb(unit, mem, SOC_MEM_SNOOP_WRITE, copyno, index, index, 
                          entry_data_ptr, meminfo->snoop_user_data);
    }

    return rv;
}

#ifdef BCM_TRIUMPH2_SUPPORT
STATIC int
_soc_mem_op_cpu_tdm(int unit, int enable)
{
    if (enable) {
        soc_IARB_TDM_TABLEm_field32_set(unit, 
                                       &(SOC_CONTROL(unit)->iarb_tdm), 
                                       PORT_NUMf, 0);
    } else {
        soc_IARB_TDM_TABLEm_field32_set(unit, 
                                       &(SOC_CONTROL(unit)->iarb_tdm), 
                                       PORT_NUMf, 63);
    }
    return _soc_mem_write(unit, IARB_TDM_TABLEm, SOC_BLOCK_ALL, 
                          SOC_CONTROL(unit)->iarb_tdm_idx, 
                          &(SOC_CONTROL(unit)->iarb_tdm));
}
#endif /* BCM_TRIUMPH2_SUPPORT */

/*
 * Function:
 *	soc_mem_write
 * Purpose:
 *	Write a memory internal to the SOC.
 * Notes:
 *	GBP/CBP memory should only accessed when MMU is in DEBUG mode.
 */

int
soc_mem_write(int unit,
              soc_mem_t mem,
              int copyno,	/* Use COPYNO_ALL for all */
              int index,
              void *entry_data)
{
    int rv;

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit)) {
        if ((mem == LMEPm) || (mem == LMEP_1m)) {
            /* Disable CPU slot from TDM */
            SOC_IF_ERROR_RETURN(_soc_mem_op_cpu_tdm(unit, 0));
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    rv = _soc_mem_write(unit, mem, copyno, index, entry_data);
    SOC_IF_ERROR_RETURN(rv);

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit) || SOC_IS_ENDURO(unit)) {
        if ((mem == LMEPm) || (mem == LMEP_1m)) {
            /* Enable CPU slot from TDM */
            SOC_IF_ERROR_RETURN(_soc_mem_op_cpu_tdm(unit, 1));
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    return rv;
}

/*
 * Function:
 *	soc_mem_write_range
 * Purpose:
 *	Write a range of chip's memory with multiple entries
 */

int
soc_mem_write_range(int unit, soc_mem_t mem, int copyno,
                    int index_min, int index_max, void *buffer)
{
    uint32          entry_dw;
    int             i;
    soc_mem_info_t  *meminfo;

    if (!soc_mem_is_valid(unit, mem)) {
        return SOC_E_MEMORY;
    }

    meminfo = &SOC_MEM_INFO(unit, mem);
    entry_dw = soc_mem_entry_words(unit, mem);

    soc_cm_debug(DK_SOCMEM,
                 "soc_mem_write_range: unit %d memory %s.%s [%d:%d]\n",
                 unit, SOC_MEM_UFNAME(unit, mem),
                 SOC_BLOCK_NAME(unit, copyno),
                 index_min, index_max);

#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    /*    coverity[negative_returns : FALSE]    */
    if ((SOC_IS_XGS3_SWITCH(unit) || SOC_IS_SIRIUS(unit)) && soc_mem_slamable(unit, mem, copyno)) {
	int		rv;
    int     blk;

    if (copyno == COPYNO_ALL) {
        SOC_MEM_BLOCK_ITER(unit, mem, blk) {
            copyno = blk;
            break;
        }
    }

	MEM_LOCK(unit, mem);
	if ((rv = _soc_xgs3_mem_slam(unit, mem, copyno,
				     index_min, index_max, buffer)) >= 0) {
	    uint32 *cache;
	    uint8 *vmap;
    /*    coverity[negative_returns : FALSE]    */

    /* coverity[negative_returns] */
	    cache = SOC_MEM_STATE(unit, mem).cache[copyno];
	    vmap = SOC_MEM_STATE(unit, mem).vmap[copyno];

	    if (cache != NULL) {
		sal_memcpy(cache + index_min * entry_dw, buffer,
			   (index_max - index_min + 1) * entry_dw * 4);

		for (i = index_min; i <= index_max; i++) {
		    CACHE_VMAP_SET(vmap, i);
		}
	    }
	}

	MEM_UNLOCK(unit, mem);
    if (NULL != meminfo->snoop_cb && 
        (SOC_MEM_SNOOP_WRITE & meminfo->snoop_flags)) {
        meminfo->snoop_cb(unit, mem, SOC_MEM_SNOOP_WRITE, copyno, index_min, 
                          index_max, buffer, meminfo->snoop_user_data);
    }

	return rv;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */

    for (i = index_min; i <= index_max; i++) {
	SOC_IF_ERROR_RETURN(soc_mem_write(unit, mem, copyno, i, buffer));
	buffer = ((uint32 *)buffer + entry_dw);
    }

    return SOC_E_NONE;
}

/************************************************************************
 * Routines for reading/writing sorted tables except ARL,		*
 * and searching sorted tables including the ARL.			*
 * Sorted tables are:  IRULE, L3, MCAST, PVLAN				*
 ************************************************************************/

/*
 * Function:
 *	soc_mem_clear
 * Purpose:
 *	Clears a memory.
 *	Operates on all copies of the table if copyno is COPYNO_ALL.
 * Notes:
 *	For non-sorted tables, all entries are cleared.  For sorted
 *	tables, only the entries known to contain data are cleared,
 *	unless the force_all flag is on, in which case all entries
 *	are cleared.  The force_all flag is mainly intended for the first
 *	initialization after a chip reset.
 */

int
soc_mem_clear(int unit,
              soc_mem_t mem,
              int copyno,
              int force_all)
{
#if defined(BCM_XGS12_SWITCH_SUPPORT) || defined(BCM_XGS12_FABRIC_SUPPORT)
    soc_persist_t	*sop = SOC_PERSIST(unit);
#endif
    int			blk, index_min, index_max, index;
    int			rv = SOC_E_NONE;

    if (soc_mem_index_count(unit, mem) == 0) {
	return SOC_E_NONE;
    }

#if defined(BCM_FIREBOLT_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_FBX(unit) || SOC_IS_SIRIUS(unit)) {
        void *null_entry = soc_mem_entry_null(unit, mem);
        MEM_LOCK(unit, mem);
        SOC_MEM_BLOCK_ITER(unit, mem, blk) {
            if (copyno != COPYNO_ALL && copyno != blk) {
                continue;
            }
            if (SOC_MEM_CLEAR_USE_DMA(unit)) {
                rv = _soc_xgs3_mem_clear_pipe(unit, mem, blk, null_entry);
                if (rv == SOC_E_UNAVAIL) {
                    rv = _soc_xgs3_mem_clear_slam(unit, mem, blk, null_entry);
                }
                if (rv < 0) {
                    goto done;
                }
            } else {
                index_min = soc_mem_index_min(unit, mem);
                index_max = soc_mem_index_max(unit, mem);
                for (index = index_min; index <= index_max; index++) {
                    if ((rv = soc_mem_write(unit, mem, blk, index,
                                            null_entry)) < 0) {
                        soc_cm_debug(DK_ERR,
                                     "soc_mem_clear: "
                                     "write %s.%s[%d] failed: %s\n",
                                     SOC_MEM_UFNAME(unit, mem),
                                     SOC_BLOCK_NAME(unit, blk),
                                     index, soc_errmsg(rv));
                        goto done;
                    }
                }
            }
        }
        rv = SOC_E_NONE;
        goto done;
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    /* Temporary mem inits for ER */
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        void *null_entry = soc_mem_entry_null(unit, mem);
        MEM_LOCK(unit, mem);
        SOC_MEM_BLOCK_ITER(unit, mem, blk) {
            if (copyno != COPYNO_ALL && copyno != blk) {
                continue;
            }
            index_min = soc_mem_index_min(unit, mem);
            index_max = soc_mem_index_max(unit, mem);
            switch (mem) {
            case IPMC_GROUP_V4m:
                rv = WRITE_IPMC_ENTRY_VLDr(unit, 0);
		if (rv < 0) {
		    goto done;
		}
                rv = WRITE_IPMC_ENTRY_V4_AVAILr(unit, 0);
		if (rv < 0) {
		    goto done;
		}
                rv = WRITE_IPMC_ENTRY_V4_BLKCNTr(unit, 0);
		if (rv < 0) {
		    goto done;
		}
                rv = WRITE_IPMC_V4_MAPPING_0r(unit, 0);
		if (rv < 0) {
		    goto done;
		}
                break;
            case IPMC_GROUP_V6m:
                rv = WRITE_IPMC_ENTRY_V6r(unit, 0);
		if (rv < 0) {
		    goto done;
		}
                rv = WRITE_IPMC_ENTRY_V6_AVAILr(unit, 0);
		if (rv < 0) {
		    goto done;
		}
                rv = WRITE_IPMC_ENTRY_V6_BLKCNTr(unit, 0);
		if (rv < 0) {
		    goto done;
		}
		rv = WRITE_IPMC_V6_MAPPING_0r(unit, 0);
		if (rv < 0) {
		    goto done;
		}
                break;
            case FP_METER_TABLE_EXTm:
                for (index = index_min; index <= index_max; index++) {
                    if ((rv = soc_er_fp_meter_table_ext_write(unit, index,
                             (fp_meter_table_ext_entry_t *)null_entry)) < 0) {
                        soc_cm_debug(DK_ERR,
                                     "soc_mem_clear: "
                                     "write %s.%s[%d] failed: %s\n",
                                     SOC_MEM_UFNAME(unit, mem),
                                     SOC_BLOCK_NAME(unit, blk),
                                     index, soc_errmsg(rv));
                        goto done;
                    }
                }
                break;
            default:
                break;
            }

            if (SOC_MEM_CLEAR_USE_DMA(unit)) {
                if ((rv = _soc_xgs3_mem_clear_slam(unit, mem,
                                                   blk, null_entry)) < 0) {
                    goto done;
                }
            } else {
                for (index = index_min; index <= index_max; index++) {
                    if ((rv = soc_mem_write(unit, mem, blk, index,
                                            null_entry)) < 0) {
                        soc_cm_debug(DK_ERR,
                                     "soc_mem_clear: "
                                     "write %s.%s[%d] failed: %s\n",
                                     SOC_MEM_UFNAME(unit, mem),
                                     SOC_BLOCK_NAME(unit, blk),
                                     index, soc_errmsg(rv));
                        goto done;
                    }
                }
            }
        }
        rv = SOC_E_NONE;
        goto done;
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#if defined(BCM_XGS12_SWITCH_SUPPORT) || defined(BCM_XGS12_FABRIC_SUPPORT)
    if (copyno == COPYNO_ALL) {
#ifdef BCM_XGS_SWITCH_SUPPORT
#if defined(BCM_XGS12_SWITCH_SUPPORT)
        if (mem == L2Xm) {
            return soc_l2x_delete_all(unit, 1);
        }
#endif /* BCM_XGS12_SWITCH_SUPPORT */
#if defined(INCLUDE_L3)
        if (mem == L3Xm) {
            return soc_l3x_delete_all(unit);
        }
#endif /* INCLUDE_L3 */
#ifdef BCM_DRACO15_SUPPORT
        /* 5695 Rev B specific memories */
        if ((mem == MMU_MEMORIES1_MSTP_TBLm ||
	     mem == MMU_MEMORIES1_ST_PORT_TBLm) &&
	    !soc_feature(unit, soc_feature_mstp_lookup)) {
            return SOC_E_PARAM;
        }
        if ((mem == MMU_MEMORIES2_SRC_TRUNK_MAPm ||
	     mem == MMU_MEMORIES2_EGR_TRUNK_MAPm) &&
	    !soc_feature(unit, soc_feature_ipmc_lookup)) {
            return SOC_E_PARAM;
        }
#endif /* BCM_DRACO15_SUPPORT */
#endif /* BCM_XGS_SWITCH_SUPPORT */
    } else {
        assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
    }

    MEM_LOCK(unit, mem);

    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }

        index_min = soc_mem_index_min(unit, mem);
        index_max = soc_mem_index_max(unit, mem);

        switch (mem) {
#if defined(BCM_XGS_SWITCH_SUPPORT)
        case L3_IPMCm:
#endif
        case FILTER_IRULEm:
        case GFILTER_IRULEm:
            if (mem == FILTER_IRULEm || mem == GFILTER_IRULEm) {
                /* Disable updating of masks while clearing rules */
                sop->filter_update = FALSE;
            }
            /*
             * Sorted tables: if force_all is true, overwrite all
             * entries.  Otherwise, overwrite only the currently
             * occupied entries.
             */
            if (!force_all && soc_mem_is_sorted(unit, mem)) {
                index_max = soc_mem_index_last(unit, mem, blk);
            }
            sop->memState[mem].count[blk] = 0;
            /* Fall through */
        case FILTER_IMASKm:
        case GFILTER_IMASKm:
#ifdef BCM_XGS12_FABRIC_SUPPORT
        case MEM_UCm:
        case MEM_MCm:
        case MEM_VIDm:
        case MEM_IPMCm:
#ifdef BCM_HERCULES15_SUPPORT
    case MEM_EGR_MODMAPm:
    case MEM_ING_MODMAPm:
    case MEM_ING_SRCMODBLKm:
    case MEM_TRUNK_PORT_POOLm:
#endif /* BCM_HERCULES15_SUPPORT */
#endif /* BCM_XGS12_FABRIC_SUPPORT */
#ifdef BCM_XGS_SWITCH_SUPPORT
        case DEFIP_HIm:
        case DEFIP_LOm:
        case DEFIP_HIT_HIm:
        case DEFIP_HIT_LOm:
        case EGR_MASKm:
        case L2X_BASEm:
        case L2X_HITm:
        case L2X_MCm:
        case L2X_STATICm:
        case L2X_VALIDm:
        case L3X_BASEm:
        case L3X_HITm:
        case L3X_VALIDm:
        case MAC_BLOCKm:
        case PORT_TABm:
        case STG_TABm:
        case TRUNK_GROUPm:
        case TRUNK_BITMAPm:
        case TRUNK_EGR_MASKm:
        case VLAN_TABm:
#endif /* BCM_XGS_SWITCH_SUPPORT */
#ifdef BCM_LYNX_SUPPORT
        case DEFIPm:
        case DEFIP_HITm:
        case EGRESS_IPMC_MSm:
        case EGRESS_IPMC_LSm:
        case TRNK_DSTm:
        case EGRESS_VLAN_STGm:
#endif /* BCM_LYNX_SUPPORT */
#ifdef BCM_TUCANA_SUPPORT
        case FE_IPMC_VLANm:
        case FE_IPMC_VECm:
        case GE_IPMC_VLANm:
        case GE_IPMC_VECm:
        case MMU_MEMORIES1_IPMCREPm:
        case VPLS_BITMAP_TABLEm:
	case DSCP_PRIORITY_TABLEm:
#endif /* BCM_TUCANA_SUPPORT */
#ifdef BCM_DRACO15_SUPPORT
        case VLAN_MACm:
        case VLAN_MAC_VALIDm:
        case MMU_MEMORIES1_IPMC_GROUP_TBLm:
        case MMU_MEMORIES1_IPMC_VLAN_TBLm:
        case MMU_MEMORIES1_MSTP_TBLm:
        case MMU_MEMORIES1_ST_PORT_TBLm:
        case MMU_MEMORIES2_SRC_TRUNK_MAPm:
        case MMU_MEMORIES2_EGR_TRUNK_MAPm:
#endif /* BCM_DRACO15_SUPPORT */
            /* NOTE: fall through from above */
            /* Tables where all entries should be cleared to null value */
            for (index = index_min; index <= index_max; index++) {
                if ((rv = soc_mem_write(unit, mem, blk, index,
                                        soc_mem_entry_null(unit, mem))) < 0) {
                    soc_cm_debug(DK_ERR,
                                 "soc_mem_clear: "
                                 "write %s.%s[%d] failed: %s\n",
                                 SOC_MEM_UFNAME(unit, mem),
                                 SOC_BLOCK_NAME(unit, blk),
                                 index, soc_errmsg(rv));
                    goto done;
                }
            }

#ifdef BCM_FILTER_SUPPORT
            if ((mem == FILTER_IMASKm || mem == GFILTER_IMASKm) &&
                sop->filter_enable) {
                _soc_mem_fmask_init(unit, mem, blk);
            }

            if (mem == FILTER_IRULEm || mem == GFILTER_IRULEm) {
                if (SOC_IS_LYNX(unit)) {	/* Flush any parity errors */
                    irule_entry_t		irule;

                    sal_memset(&irule, 0, sizeof (irule));

                    if ((rv = soc_mem_read(unit, mem, blk, 0, &irule)) < 0) {
                        soc_cm_debug(DK_ERR,
                                     "soc_mem_clear: "
                                     "read %s.%s[%d] failed: %s\n",
                                     SOC_MEM_UFNAME(unit, mem),
                                     SOC_BLOCK_NAME(unit, blk),
                                     0, soc_errmsg(rv));
                        goto done;
                    }
                }

                /* Restore updating of masks */
                sop->filter_update = TRUE;
            }
#endif /* BCM_FILTER_SUPPORT */
            break;
#ifdef BCM_DRACO15_SUPPORT
        case VLAN_MAC_ENTRYm:
            if ( ((rv = soc_mem_clear(unit, VLAN_MAC_VALIDm, 0, 0)) < 0) ||
                 ((rv = soc_mem_clear(unit, VLAN_MACm, 0, 0)) < 0) )
                continue;
            goto done;
#endif /* BCM_DRACO15_SUPPORT */
#if defined(BCM_5690) || defined(BCM_5665)
        case GFILTER_FFPCOUNTERSm:
        case GFILTER_FFPPACKETCOUNTERSm:
#if defined(BCM_5695)
        case GFILTER_FFP_IN_PROFILE_COUNTERSm:
        case GFILTER_FFP_OUT_PROFILE_COUNTERSm:
#endif
        case GFILTER_METERINGm:
#endif
#if defined(BCM_5673) || defined(BCM_5674)
        case XFILTER_FFPCOUNTERSm:
        case XFILTER_FFPOPPACKETCOUNTERSm:
        case XFILTER_FFPIPPACKETCOUNTERSm:
        case XFILTER_FFPOPBYTECOUNTERSm:
        case XFILTER_FFPIPBYTECOUNTERSm:
        case XFILTER_METERINGm:
#endif
#if defined(BCM_5690) || defined(BCM_5665) || \
    defined(BCM_5673) || defined(BCM_5674)
            if (soc_feature(unit, soc_feature_filter_metering)) {
                /* NOTE: fall through from above */
                /* Tables where all entries should be cleared to null value */
                for (index = index_min; index <= index_max; index++) {
                    if ((rv = soc_mem_write(unit, mem, blk, index,
                                            soc_mem_entry_null(unit,
                                                               mem))) < 0) {
                        soc_cm_debug(DK_ERR,
                                     "soc_mem_clear: "
                                     "write %s.%s[%d] failed: %s\n",
                                     SOC_MEM_UFNAME(unit, mem),
                                     SOC_BLOCK_NAME(unit, blk),
                                     index, soc_errmsg(rv));
                        goto done;
                    }
                }
                break;
            } /* else fall thru to default */
#endif /* BCM_XGS_SWITCH_SUPPORT */
        default:
            soc_cm_debug(DK_ERR,
                         "soc_mem_clear: don't know how to clear %s\n",
                         SOC_MEM_UFNAME(unit, mem));
            rv = SOC_E_PARAM;
            goto done;
        }
    }
#endif /* XGS12_SWITCH  || XGS12_FABRIC */

    rv = SOC_E_NONE;

 done:

    MEM_UNLOCK(unit, mem);

    soc_cm_debug(DK_SOCMEM,
                 "soc_mem_clear: unit %d memory %s.%s returns %s\n",
                 unit, SOC_MEM_UFNAME(unit, mem),
                 SOC_BLOCK_NAME(unit, copyno), soc_errmsg(rv));

    return rv;
}

#ifdef BCM_LYNX_SUPPORT
/*
 * Function:
 *      _soc_mem_cam_search
 * Purpose:
 *      Search a CAM memory for a key.
 * Parameters:
 *      mem  		     CAM Memory to search
 *      blk                  Which block contains the CAM (if multiple copies)
 *      index_ptr            OUT:
 *                           If entry found gets the location of the entry.
 *                           CAN NO LONGER BE NULL.
 *      key_data       	     IN:  Data to search for
 *      entry_data,    	     OUT: Data if found
 *                           CAN NO LONGER BE NULL.  Must be big enough
 *                           to handle the appropriate data.
 * Returns:
 *	SOC_E_ENTRY_NOT_FOUND
 *			     Entry not found.
 *	SOC_E_NONE	     Entry is found:  index_ptr gets location
 *	SOC_E_XXX	     If internal error occurs
 * Notes:
 *      Uses CAM lookup ability on Lynx to find a matching entry if it exists.
 *
 */

int
_soc_mem_cam_search(int unit,
                    soc_mem_t mem,
                    int blk,
                    int *index_ptr,
                    void *key_data,
                    void *entry_data)
{
    int idx, port;
    int rv;
    uint32 irule_result;
    soc_mem_t search_mem;
    soc_timeout_t to;
    uint32              lookup_data[SOC_MAX_MEM_WORDS];
    uint32              field_val[SOC_MAX_MEM_WORDS];

    assert(entry_data);
    assert(index_ptr);

    if (blk == MEM_BLOCK_ANY) {
        blk = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    assert(SOC_MEM_BLOCK_VALID(unit, mem, blk));

    if (mem != GFILTER_IRULEm || !SOC_IS_LYNX(unit)) {
        return SOC_E_INTERNAL; /* Illegal use */
    }

    port = SOC_BLOCK_PORT(unit, blk);

    /* Write to search "memory" */
    search_mem = GFILTER_IRULELOOKUPm;
    idx = 0;
    sal_memset(lookup_data, 0, sizeof (lookup_data));

    soc_mem_field_get(unit, mem, key_data, FILTERf, field_val);
    soc_mem_field_set(unit, search_mem, lookup_data, FILTERf, field_val);

    soc_mem_field_get(unit, mem, key_data, PKTFORMATf, field_val);
    soc_mem_field_set(unit, search_mem, lookup_data, PKTFORMATf, field_val);

    soc_mem_field_get(unit, mem, key_data, EPORTf, field_val);
    soc_mem_field_set(unit, search_mem, lookup_data, EPORTf, field_val);

    soc_mem_field_get(unit, mem, key_data, EGRSMODf, field_val);
    soc_mem_field_set(unit, search_mem, lookup_data, EGRSMODf, field_val);

    soc_mem_field_get(unit, mem, key_data, FSELf, field_val);
    soc_mem_field_set(unit, search_mem, lookup_data, FSELf, field_val);

    if ((rv = soc_mem_write(unit, search_mem, blk, idx, lookup_data)) < 0) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_cam_search: write %s.%s[%d] failed\n",
                     SOC_MEM_UFNAME(unit, search_mem),
                     SOC_BLOCK_NAME(unit, blk), idx);
        return rv;
    }

    /* Poll return until value appears or timeout */
    soc_timeout_init(&to, SOC_CONTROL(unit)->tableDmaTimeout, 100);

    while (1) {
        SOC_IF_ERROR_RETURN(READ_FFP_IRULERESULTr(unit, port, &irule_result));
        if (soc_reg_field_get(unit, FFP_IRULERESULTr, irule_result,
                              VALIDf)) {
            break;
        }

        if (soc_timeout_check(&to)) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_cam_search: timeout on result polling\n");
            return SOC_E_BUSY;
        }
    }

    if (!soc_reg_field_get(unit, FFP_IRULERESULTr, irule_result,
                              MATCHf)) {
        return SOC_E_NOT_FOUND;
    } else {
        *index_ptr = soc_reg_field_get(unit, FFP_IRULERESULTr, irule_result,
                                       ADDRESSf);

        if (soc_feature(unit, soc_feature_filter_128_rules)) {
            if (*index_ptr != ((*index_ptr / 8) * 8)) {
                /* Lynx A1 CAM, match in wrong location! */
                soc_cm_debug(DK_ERR,
                             "soc_mem_cam_search: index %d found in %s.%s\n",
                             *index_ptr, SOC_MEM_UFNAME(unit, search_mem),
                             SOC_BLOCK_NAME(unit, blk));
                return SOC_E_INTERNAL;
            }
        } else if (!soc_feature(unit, soc_feature_filter_krules)) {
            int	min, max, min_eff, max_eff;

            min = soc_mem_index_min(unit, mem);
            max = soc_mem_index_max(unit, mem);
            /* GNATS 4404: odd parity */
            min_eff = (min % 2) ? min : (min + 1);
            max_eff = max / 2; /* GNATS 4403: Lynx A0 CAM, lower half only */

            if ((*index_ptr < min_eff) || (*index_ptr > max_eff) ||
                ((*index_ptr % 2) == 0)) {
                /* GNATS 4403, 4403:  This entry should not be used! */
                soc_cm_debug(DK_ERR,
                             "soc_mem_cam_search: index %d found in %s.%s\n",
                             *index_ptr, SOC_MEM_UFNAME(unit, search_mem),
                             SOC_BLOCK_NAME(unit, blk));
                return SOC_E_INTERNAL;
            }
        }

        /* Read out the line of the CAM indicated */
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, blk,
                                         *index_ptr, entry_data));
    }

    return SOC_E_NONE;
}
#endif /* BCM_LYNX_SUPPORT */

/*
 * Function:
 *      _soc_mem_search
 * Purpose:
 *      Helper function for soc_mem_search.  Does the raw search
 *      without worrying about special memory cases.
 * Parameters:
 *      See soc_mem_search, below.
 * Returns:
 *	SOC_E_NOT_FOUND
 *			     Entry not found:  in this case, if
 *                           table is sorted, then index_ptr gets the
 *                           location in which the entry should be
 *                           inserted.
 *	SOC_E_NONE	     Entry is found:  index_ptr gets location
 *	SOC_E_XXX	     If internal error occurs
 * Notes:
 *	See soc_mem_search, below.
 */

STATIC int
_soc_mem_search(int unit,
               soc_mem_t mem,
               int copyno,
               int *index_ptr,
               void *key_data,
               void *entry_data,
               int lowest_match)
{
    int base, count, mid, mask, last_read = -1;
    int rv, r;

    /*
     * Establish search range within table.  For filter, optimize using
     * cached start/count of rule set for mask specified by FSEL.  Note
     * that all rules using a given FSEL are necessarily contiguous.
     */

    switch (mem) {
    case FILTER_IRULEm:
        mask = soc_FILTER_IRULEm_field32_get(unit, key_data, FSELf);
        base = SOC_PERSIST(unit)->filter_masks[copyno][mask].start;
        count = SOC_PERSIST(unit)->filter_masks[copyno][mask].count;
        break;
    case GFILTER_IRULEm:
        mask = soc_GFILTER_IRULEm_field32_get(unit, key_data, FSELf);
        base = SOC_PERSIST(unit)->filter_masks[copyno][mask].start;
        count = SOC_PERSIST(unit)->filter_masks[copyno][mask].count;
        break;
    default:
        base = soc_mem_index_min(unit, mem);
        count = soc_mem_entries(unit, mem, copyno);
        break;
    }

    while (count > 0) {
        mid = base + count / 2;

        if ((rv = soc_mem_read(unit, mem, copyno, mid, entry_data)) < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_search: read %s.%s[%d] failed\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno), mid);
            return rv;
        }

        last_read = mid;

        r = soc_mem_compare_key(unit, mem, key_data, entry_data);

        if (r == 0) {
            if (lowest_match) {
                count = count / 2;      /* Must treat same as < 0 */
            } else {
                if (index_ptr != NULL) {
                    *index_ptr = mid;   /* Don't care if it's lowest match */
                }
                return SOC_E_NONE;              /* Found */
            }
        } else if (r < 0) {
            count = count / 2;
        } else {
            base = mid + 1;
            count = (count - 1) / 2;
        }
    }

    *index_ptr = base;

    /*
     * base is the expected position of the entry (insertion point).
     * If lowest_match is true, we may have actually found it, and
     * we have to check again here.
     */

    if (lowest_match && base <= soc_mem_index_max(unit, mem)) {
        if (last_read != base &&        /* Optimize out read if possible */
            (rv = soc_mem_read(unit, mem, copyno, base, entry_data)) < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_search: read %s.%s[%d] failed\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno), base);
            return rv;
        }

        if (soc_mem_compare_key(unit, mem, key_data, entry_data) == 0) {
            return SOC_E_NONE;                  /* Found */
        }
    }

    /* If bitmap conflict found, return SOC_E_CONFIG */
    return SOC_E_NOT_FOUND;       /* No error; not found */
}

/****************************************************************
 *
 * TUCANA (5665) SUPPORT FUNCTIONS
 *
 ****************************************************************/

#if defined(BCM_TUCANA_SUPPORT)

/*
 * Function:
 *      _soc_5665_irule_iport_best_match
 * Purpose:
 *      Search an IRULE table for a matching entry
 * Parameters:
 *      unit - Strata-XGS unit number
 *      mem - which memory
 *      copyno - Actual copy number, can't be "ANY" or "ALL"
 *      index_ptr - (IN/OUT) Start search here; holds index of best match
 *      key_data - Data to search for
 *      entry_data - (IN/OUT) Start data; Gets data if found; May be NULL.
 * Returns:
 *      SOC_E_XXX or SOC_MEM_PARTIAL_MATCH
 * Notes:
 *      Should only be called when the rule uses IPORT_BITMAPs,
 *      Returns SOC_E_CONFIG if it is not possible to add the entry
 *      without creating a table conflict (two matching rules with
 *      intersecting bitmap).
 *
 *      If an entry is found containing the search key's bitmap,
 *      SOC_E_NONE is returned.
 *
 *      If exactly one entry is found which intersects, but does not
 *      contain the search key's bitmap, "PARTIAL_MATCH" > 0 is returned.
 *
 *      If no entry has intersecting bitmap, return SOC_E_NOT_FOUND.
 */

STATIC int
_soc_5665_irule_iport_best_match(int unit, soc_mem_t mem, int copyno,
				 int *index_ptr, void *key_data,
				 void *entry_data)
{
    int some_intersect = FALSE;  /* Have we found overlap for pbmp? */
    int overlap_idx;
    int save_idx;
    int entry_dw = soc_mem_entry_words(unit, mem);
    uint32 save_entry[SOC_MAX_MEM_WORDS];
    uint32 ent_bitmap, blk_bitmap;

    overlap_idx = -1;
    save_idx = *index_ptr;
    sal_memcpy(save_entry, entry_data, entry_dw * sizeof (uint32));
    blk_bitmap = soc_mem_field32_get(unit, FILTER_IRULEm, key_data,
                                     IPORT_BITMAPf);

    while (1) {
        ent_bitmap = soc_mem_field32_get(unit, FILTER_IRULEm, entry_data,
                                         IPORT_BITMAPf);
        if ((blk_bitmap & ent_bitmap) == blk_bitmap) {
            /* Entry contains all ports of search entry.  Good match */
            return SOC_E_NONE;
        } else if (ent_bitmap & blk_bitmap) {
            if (some_intersect) {
                /*
                 * Already found an entry with some intersection;
                 * can't add to table without conflict.
                 */
                soc_cm_debug(DK_WARN, "soc_mem_search 5665 irule: bitmap "
                             "0x%x conflicts with entries %d and %d\n",
                             blk_bitmap, overlap_idx, *index_ptr);
                return SOC_E_CONFIG;
            }
            some_intersect = TRUE;
            overlap_idx = *index_ptr;
        }

        /* Look at the next entry */
        if ((*index_ptr)++ >= soc_mem_index_max(unit, mem)) {
            break;
        }
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, copyno,
                                         *index_ptr, entry_data));
        if (_soc_mem_cmp_rule_5665(unit, key_data, entry_data)) {
            /* Table entry no longer matches non-iport data */
            break;
        }
    }

    if (overlap_idx >= 0) { /* overlap found */
        *index_ptr = overlap_idx;
        SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, copyno,
                                         overlap_idx, entry_data));
        return SOC_MEM_PARTIAL_MATCH;
    } else if (*index_ptr != save_idx) { /* no overlap, leave unchanged */
        *index_ptr = save_idx;
        sal_memcpy(entry_data, save_entry, entry_dw * sizeof (uint32));
    }

    return SOC_E_NOT_FOUND;
}

/*
 * Function:
 *      _soc_5665_irule_search
 * Purpose:
 *      Search an IRULE table for a matching entry
 * Parameters:
 *      unit - Strata-XGS unit number
 *      copyno - Actual copy number, can't be "ANY" or "ALL"
 *      index_ptr - (OUT) Gets index of entry in table if found
 *      key_data - Data to search for
 *      entry_data - (OUT) Gets data if found; May be NULL.
 * Returns:
 *      SOC_E_XXX; may return SOC_MEM_PARTIAL_MATCH > 0
 * Notes:
 *      When the rule uses IPORT_BITMAPs, this will return SOC_E_CONFIG
 *      if it is not possible to add the entry without creating a table
 *      conflict.  A table conflict means that there are two rules in
 *      the table already both of which intersect the port bitmap of
 *      this entry.
 */

STATIC int
_soc_5665_irule_search(int unit, soc_mem_t mem, int copyno,
                       int *index_ptr, void *key_data, void *entry_data)
{
    int mask;
    soc_fm_t            *fm;
    int rv;

    mask = soc_mem_field32_get(unit, mem, key_data, FSELf);
    fm = &SOC_PERSIST(unit)->filter_masks[copyno][mask];

    if (fm->count == 0) { /* No rules for this filter */
        *index_ptr = fm->start;
        return SOC_E_NOT_FOUND;
    }

    /* get the IPORT mode from the mask */
    if (soc_mem_field32_get(unit, mem, key_data, IPORT_MODEf) ==
           SOC_IPORT_MODE_SPECIFIC) { /* Normal memory search */
        return _soc_mem_search(unit, mem, copyno, index_ptr, key_data,
                             entry_data, 0);
    }

    rv = _soc_mem_search(unit, mem, copyno, index_ptr, key_data,
                         entry_data, 1);

    if (rv == SOC_E_NONE) {
        /* We've found a matching entry.  Look for bitmap match */

        return _soc_5665_irule_iport_best_match(unit, mem, copyno, index_ptr,
                                                key_data, entry_data);
    }

    return rv;
}

#endif /* BCM_TUCANA_SUPPORT */

/*
 * Function:
 *      soc_mem_generic_lookup
 * Purpose:
 *      Send a lookup message over the S-Channel and receive the response.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      banks - For dual hashing, which halves are selected (inverted)
 *              Note: used to create a bitmap for ISM mems. (All banks: -1)
 *      key - entry to look up
 *      result - entry to receive entire found entry
 *      index_ptr (OUT) - If found, receives table index where found
 * Returns:
 *      SOC_E_INTERNAL if retries exceeded or other internal error
 *      SOC_E_NOT_FOUND if the entry is not found.
 *      SOC_E_NONE (0) on success (entry found):
 */

int
soc_mem_generic_lookup(int unit, soc_mem_t mem, int copyno, uint8 banks,
                       void *key, void *result, int *index_ptr)
{
    schan_msg_t schan_msg;
    int         rv, entry_dw;
    uint8       at;

    assert(SOC_MEM_IS_VALID(unit, mem));
    assert(soc_attached(unit));

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_generic_lookup: not supported on %s\n",
                     SOC_CHIP_STRING(unit));
	return SOC_E_UNAVAIL;
    }
#endif

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_generic_lookup: invalid block %d for memory %s\n",
                     copyno, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    schan_msg_clear(&schan_msg);
    schan_msg.gencmd.header.opcode = TABLE_LOOKUP_CMD_MSG;
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.gencmd.header.srcblk = 0;
    } else 
#endif
    {
        schan_msg.gencmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
    /* schan_msg.gencmd.header.srcblk = 0;*/
#endif
    }
    schan_msg.gencmd.header.dstblk = SOC_BLOCK2SCH(unit, copyno);
    schan_msg.gencmd.header.cos = banks & 0x3;
    schan_msg.gencmd.header.datalen = entry_dw * 4;

    schan_msg.gencmd.address = soc_mem_addr_get(unit, mem, copyno, 0, &at);

    /* Fill in entry data */
    sal_memcpy(schan_msg.gencmd.data, key, entry_dw * 4);

    /*
     * Execute S-Channel "lookup insert" command packet consisting of
     * (header word + address word + entry_dw), and read back
     * (header word + response word + entry_dw) data words.
     */

    rv = soc_schan_op(unit, &schan_msg, entry_dw + 2, entry_dw + 2, 0);

    /* Check result */

    if (schan_msg.genresp.header.opcode != TABLE_LOOKUP_DONE_MSG) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_generic_lookup: "
                     "invalid S-Channel reply, expected TABLE_LOOKUP_DONE_MSG:\n");
        soc_schan_dump(unit, &schan_msg, entry_dw + 2);
        return SOC_E_INTERNAL;
    }

    if ((schan_msg.genresp.header.cpu) || (rv == SOC_E_FAIL)) {
        if (index_ptr) {
            *index_ptr = -1;
        }
        if (schan_msg.genresp.response.type == SCHAN_GEN_RESP_TYPE_NOT_FOUND) {
            rv = SOC_E_NOT_FOUND;
        } else {
            rv = SOC_E_FAIL;
        }
    } else {
        if (result != NULL) {
            sal_memcpy(result, schan_msg.genresp.data,
                       entry_dw * sizeof(uint32));
        }
        if (index_ptr != NULL) {
            *index_ptr = schan_msg.genresp.response.index;
        }
    }

    if (soc_cm_debug_check(DK_SOCMEM)) {
        soc_cm_print("Lookup table[%s]: banks=%d", SOC_MEM_NAME(unit, mem),
                     banks);
        if (soc_cm_debug_check(DK_VERBOSE)) {
            soc_mem_entry_dump(unit, mem, result);
        }
        if (SOC_FAILURE(rv)) {
            if (schan_msg.genresp.response.type == SCHAN_GEN_RESP_TYPE_NOT_FOUND) {
                soc_cm_print(" Not found\n");
            } else {
                soc_cm_print(" Fail\n");
            }
        } else {
            soc_cm_print(" (index=%d)\n", schan_msg.genresp.response.index);
        }
    }

    return rv;
}

/*
 * Function:
 *      soc_mem_generic_insert
 * Purpose:
 *      Insert an entry
 * Parameters:
 *      unit - StrataSwitch unit #
 *      banks - For dual hashing, which halves are selected (inverted)
 *              Note: used to create a bitmap for ISM mems. (All banks: -1)
 *      entry - entry to insert
 *      old_entry - old entry if existing entry was replaced
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_EXISTS - existing entry was replaced
 *      SOC_E_FULL - full
 *      SOC_E_BUSY - modfifo full
 * Notes:
 *      Uses hardware insertion; sends an INSERT message over the
 *      S-Channel.
 */

int
soc_mem_generic_insert(int unit, soc_mem_t mem, int copyno, uint8 banks,
                       void *entry, void *old_entry, int *index_ptr)
{
    schan_msg_t     schan_msg;
    int             rv, entry_dw;
    schan_genresp_t *response;
    uint8           at;

    assert(SOC_MEM_IS_VALID(unit, mem));
    assert(soc_attached(unit));

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_generic_insert: not supported on %s\n",
                     SOC_CHIP_STRING(unit));
	return SOC_E_UNAVAIL;
    }
#endif

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_generic_insert: invalid block %d for memory %s\n",
                     copyno, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    if (soc_cm_debug_check(DK_SOCMEM)) {
        soc_cm_print("Insert table[%s]: banks=%d", SOC_MEM_NAME(unit, mem),
                     banks);
        if (soc_cm_debug_check(DK_VERBOSE)) {
            soc_mem_entry_dump(unit, mem, entry);
        }
        soc_cm_print("\n");
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    schan_msg_clear(&schan_msg);
    schan_msg.gencmd.header.opcode = TABLE_INSERT_CMD_MSG;
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.readcmd.header.srcblk = 0;
    } else 
#endif
    {
        schan_msg.gencmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
        /* schan_msg.gencmd.header.srcblk = 0; */
#endif
    }
    schan_msg.gencmd.header.dstblk = SOC_BLOCK2SCH(unit, copyno);
    schan_msg.gencmd.header.cos = banks & 0x3;
    schan_msg.gencmd.header.datalen = entry_dw * 4;

    schan_msg.gencmd.address = soc_mem_addr_get(unit, mem, copyno, 0, &at);

    /* Fill in packet data */
    sal_memcpy(schan_msg.gencmd.data, entry, entry_dw * 4);

    /*
     * Execute S-Channel "table insert" command packet consisting of
     * (header word + address word + entry_dw), and read back
     * (header word + response word + entry_dw) data words.
     */

    rv = soc_schan_op(unit, &schan_msg, entry_dw + 2, entry_dw + 2, 0);

    if (schan_msg.gencmd.header.opcode != TABLE_INSERT_DONE_MSG) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_generic_insert: "
                     "invalid S-Channel reply, expected TABLE_INSERT_DONE_MSG:\n");
        soc_schan_dump(unit, &schan_msg, 1);
        return SOC_E_INTERNAL;
    }

    response = &schan_msg.genresp.response;
    if ((schan_msg.genresp.header.cpu) || (rv == SOC_E_FAIL)) {
        if (index_ptr) {
            *index_ptr = -1;
        }
        if (response->type == SCHAN_GEN_RESP_TYPE_FULL) {
            soc_cm_debug(DK_SOCMEM,
                "Insert table[%s]: hash bucket full\n", SOC_MEM_NAME(unit, mem));
            rv = SOC_E_FULL;
        } else if (response->type == SCHAN_GEN_RESP_TYPE_ERROR) {
            if (response->err_info == SCHAN_GEN_RESP_ERROR_BUSY) {
                soc_cm_debug(DK_SOCMEM,
                    "Insert table[%s]: Modfifo full\n", SOC_MEM_NAME(unit, mem));
                rv = SOC_E_BUSY;
            } else if (response->err_info == SCHAN_GEN_RESP_ERROR_PARITY) {
                soc_cm_debug(DK_ERR,
                    "Insert table[%s]: Parity Error Index %d\n", SOC_MEM_NAME(unit, mem),
                     (int) response->index);
                rv = SOC_E_INTERNAL;
            }
        } else {
            rv = SOC_E_FAIL;
        }
    } else {
        if (index_ptr != NULL) {
            *index_ptr = response->index;
        }
        if (response->type == SCHAN_GEN_RESP_TYPE_REPLACED) {
            if (old_entry != NULL) {
                sal_memcpy(old_entry, schan_msg.genresp.data,
                           entry_dw * sizeof(uint32));
            }
            rv = SOC_E_EXISTS;
        }
#if defined(BCM_TRIUMPH_SUPPORT)
        if (mem == EXT_L2_ENTRYm) {
            SOC_IF_ERROR_RETURN
                (soc_triumph_ext_l2_entry_update(unit, response->index,
                                                 entry));
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

    return rv;
}

/*
 * Function:
 *      soc_mem_generic_delete
 * Purpose:
 *      Delete an entry
 * Parameters:
 *      unit - StrataSwitch unit #
 *      banks - For dual hashing, which halves are selected (inverted)
 *              Note: used to create a bitmap for ISM mems. (All banks: -1)
 *      entry - entry to delete
 *      old_entry - old entry if entry was found and deleted
 * Returns:
 *      SOC_E_NONE - success
 *      SOC_E_NOT_FOUND - full
 *      SOC_E_BUSY - modfifo full
 * Notes:
 *      Uses hardware deletion; sends an DELETE message over the
 *      S-Channel.
 */

int
soc_mem_generic_delete(int unit, soc_mem_t mem, int copyno, uint8 banks,
                       void *entry, void *old_entry, int *index_ptr)
{
    schan_msg_t         schan_msg;
    int                 rv, entry_dw;
    schan_genresp_t     *response;
    uint8               at;

    assert(SOC_MEM_IS_VALID(unit, mem));
    assert(soc_attached(unit));

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_generic_delete: not supported on %s\n",
                     SOC_CHIP_STRING(unit));
	return SOC_E_UNAVAIL;
    }
#endif

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_generic_delete: invalid block %d for memory %s\n",
                     copyno, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    if (soc_cm_debug_check(DK_SOCMEM)) {
        soc_cm_print("Delete table[%s]: banks=%d", SOC_MEM_NAME(unit, mem),
                     banks);
        if (soc_cm_debug_check(DK_VERBOSE)) {
            soc_mem_entry_dump(unit, mem, entry);
        }
        soc_cm_print("\n");
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    schan_msg_clear(&schan_msg);
    schan_msg.gencmd.header.opcode = TABLE_DELETE_CMD_MSG;
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.readcmd.header.srcblk = 0;
    } else 
#endif
    {
        schan_msg.gencmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
        /* schan_msg.gencmd.header.srcblk = 0; */
#endif
    }
    schan_msg.gencmd.header.dstblk = SOC_BLOCK2SCH(unit, copyno);
    schan_msg.gencmd.header.cos = banks & 0x3;
    schan_msg.gencmd.header.datalen = entry_dw * 4;
    schan_msg.gencmd.address = soc_mem_addr_get(unit, mem, copyno, 0, &at);

    /* Fill in packet data */
    sal_memcpy(schan_msg.gencmd.data, entry, entry_dw * 4);

    /*
     * Execute S-Channel "table delete" command packet consisting of
     * (header word + address word + entry_dw), and read back
     * (header word + response word + entry_dw) data words.
     */
    rv = soc_schan_op(unit, &schan_msg, entry_dw + 2, entry_dw + 2, 0);

    if (schan_msg.readresp.header.opcode != TABLE_DELETE_DONE_MSG) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_generic_delete: "
                     "invalid S-Channel reply, expected TABLE_DELETE_DONE_MSG:\n");
        soc_schan_dump(unit, &schan_msg, 1);
        return SOC_E_INTERNAL;
    }

    response = &schan_msg.genresp.response;
    if ((schan_msg.genresp.header.cpu) || (rv == SOC_E_FAIL)) {
        if (index_ptr) {
            *index_ptr = -1;
        }
        if (response->type == SCHAN_GEN_RESP_TYPE_NOT_FOUND) {
            soc_cm_debug(DK_SOCMEM,
                "Delete table[%s]: Not found\n", SOC_MEM_NAME(unit, mem));
            rv = SOC_E_NOT_FOUND;
        } else if (response->type == SCHAN_GEN_RESP_TYPE_ERROR) {
            if (response->err_info == SCHAN_GEN_RESP_ERROR_BUSY) {
                soc_cm_debug(DK_SOCMEM,
                    "Delete table[%s]: Modfifo full\n", SOC_MEM_NAME(unit, mem));
                rv = SOC_E_BUSY;
            } else if (response->err_info == SCHAN_GEN_RESP_ERROR_PARITY) {
                soc_cm_debug(DK_ERR,
                    "Delete table[%s]: Parity Error Index %d\n", SOC_MEM_NAME(unit, mem),
                     (int) response->index);
                rv = SOC_E_INTERNAL;
            }
        } else {
            rv = SOC_E_FAIL;
        }
    } else {
        if (old_entry != NULL) {
            sal_memcpy(old_entry, schan_msg.genresp.data,
                       entry_dw * sizeof(uint32));
        }
        if (index_ptr != NULL) {
            *index_ptr = response->index;
        }
#if defined(BCM_TRIUMPH_SUPPORT)
        if (mem == EXT_L2_ENTRYm) {
            SOC_IF_ERROR_RETURN
                (soc_triumph_ext_l2_entry_update(unit, response->index, NULL));
        }
#endif /* BCM_TRIUMPH_SUPPORT */
    }

    return rv;
}

/*
 * Function:
 *      soc_mem_search
 * Purpose:
 *      Search a memory for a key.
 * Parameters:
 *      mem  		     Memory to search
 *      copyno               Which copy to search (if multiple copies)
 *      index_ptr            OUT:
 *                           If entry found gets the location of the entry.
 *                           If not found, and table is sorted, gets the
 *                           location of the insertion point.
 *                           CAN NO LONGER BE NULL.
 *      key_data       	     IN:  Data to search for
 *      entry_data,    	     OUT: Data if found
 *                           CAN NO LONGER BE NULL.  Must be big enough
 *                           to handle the appropriate data.
 *      lowest_match	     IN:  For sorted tables only.
 *                           If there are duplicate entries in the
 *                           table, and lowest_match is 1, then the
 *                           matching entry at the lowest index is
 *                           returned.  If lowest_match is 0, then any
 *                           of the matching entries may be picked at
 *                           random, which can be faster.
 * Returns:
 *	SOC_E_NOT_FOUND
 *			     Entry not found:  in this case, if
 *                           table is sorted, then index_ptr gets the
 *                           location in which the entry should be
 *                           inserted.
 *	SOC_E_NONE	     Entry is found:  index_ptr gets location
 *	SOC_E_XXX	     If internal error occurs
 *      SOC_MEM_PARTIAL_MATCH ( > 0) for Tucana IRULE memories.  See above.
 * Notes:
 *	A binary search is performed for a matching table entry.
 *		The appropriate field(s) of entry_data are used for the key.
 *		All other fields in entry_data are ignored.
 *
 *	If found, the index is stored in index_ptr, and if entry_data is
 *		non-NULL, the contents of the found entry are written into it,
 *		and 1 is returned.
 *	If not found, the index of a correct insertion point for the
 *		entry is stored in index_ptr and 0 is returned (unless this
 *		is a hashed table).
 *	If a table read error occurs, SOC_E_XXX is returned.
 *
 *	For the ARL table, all entries are searched.  On revision GSL and
 *		higher, a hardware lookup feature is used.  For earlier
 *		revisions, a software search is used, which is not
 *		recommended since the ARL table may be under constant
 *		update by the hardware.  For hardware lookup, the index_ptr
 *		is written with -1.
 *	For non-ARL sorted tables, only the entries known to contain data
 *		are searched:
 *		(index_min <= search_index < index_min + memInfo.count).
 */

int
soc_mem_search(int unit,
               soc_mem_t mem,
               int copyno,
               int *index_ptr,
               void *key_data,
               void *entry_data,
               int lowest_match)
{
    int rv = 0;

    COMPILER_REFERENCE(rv);
    assert(soc_mem_is_sorted(unit, mem) ||
           soc_mem_is_hashed(unit, mem) ||
           soc_mem_is_cam(unit, mem) ||
           soc_mem_is_cmd(unit, mem));
    assert(entry_data);
    assert(index_ptr);

#ifdef BCM_LYNX_SUPPORT
    if (mem == GFILTER_IRULEm && SOC_IS_LYNX(unit)) {
        return _soc_mem_cam_search(unit, mem, copyno, index_ptr,
                                   key_data, entry_data);
    }
#endif

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));

#ifdef BCM_TUCANA_SUPPORT
    if ((mem == GFILTER_IRULEm || mem == FILTER_IRULEm)
             && SOC_IS_TUCANA(unit)) {
        return _soc_5665_irule_search(unit, mem, copyno, index_ptr,
                                      key_data, entry_data);
    }
#endif

#ifdef BCM_DRACO15_SUPPORT
    if ((mem == VLAN_MAC_ENTRYm) && SOC_IS_DRACO15(unit)) {
        return soc_mem_draco15_vlan_mac_search(unit, index_ptr,
                                               entry_data);
    }
#endif

#if defined(BCM_EASYRIDER_SUPPORT)
    if (soc_mem_is_cmd(unit, mem)) {
        soc_mem_t rmem = INVALIDm;
        switch (mem) {
	case L2_ENTRY_EXTERNALm:
	case L2_ENTRY_INTERNALm:
	case L2_ENTRY_OVERFLOWm:
            rv = soc_er_l2x_lookup(unit, key_data, entry_data,
                                   index_ptr, &rmem);
            if ((rv >= 0) && (mem != rmem)) {
                rv = SOC_E_NOT_FOUND;
                *index_ptr = -1;
            }
            return rv;
#if defined(INCLUDE_L3)
	case L3_ENTRY_V4m:
            return soc_er_l3v4_lookup(unit, key_data,
                                      entry_data, index_ptr);
	case L3_ENTRY_V6m:
            return soc_er_l3v6_lookup(unit, key_data,
                                      entry_data, index_ptr);
#endif /* INCLUDE_L3 */
	case MY_STATIONm:
            rv = soc_mem_er_search(unit, mem, copyno, index_ptr,
                                   key_data, entry_data, lowest_match);
            if (rv >= 0) {
                *index_ptr -=
                    SOC_PERSIST(unit)->er_memcfg.mystation_search_offset;
            }
            return rv;

	case L3_DEFIP_ALGm:
            return soc_er_defip_lookup(unit, index_ptr, key_data,
                                       entry_data);

	case FP_EXTERNALm:
            return soc_er_fp_ext_lookup(unit, index_ptr, key_data,
                                        entry_data);

        default:
            return soc_mem_er_search(unit, mem, copyno, index_ptr,
                                     key_data, entry_data, lowest_match);
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        switch (mem) {
            case L2Xm:
            case L3_ENTRY_IPV4_UNICASTm:
            case L3_ENTRY_IPV4_MULTICASTm:
            case L3_ENTRY_IPV6_UNICASTm:
            case L3_ENTRY_IPV6_MULTICASTm:
            case EGR_VLAN_XLATEm:
            case VLAN_XLATEm:
            case VLAN_MACm:
            case MPLS_ENTRYm:
            case EXT_L2_ENTRYm:
                return soc_mem_generic_lookup(unit, mem, copyno, 0,
                                              key_data, entry_data, index_ptr);
            default:
                break;
        }
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        switch (mem) {
	case L2Xm:
	    return soc_fb_l2x_lookup(unit, key_data,
				     entry_data, index_ptr);
#if defined(INCLUDE_L3)
	case L3_DEFIPm:
	    return soc_fb_lpm_match(unit, key_data,
				    entry_data, index_ptr);
	case L3_ENTRY_IPV4_UNICASTm:
	case L3_ENTRY_IPV4_MULTICASTm:
	case L3_ENTRY_IPV6_UNICASTm:
	case L3_ENTRY_IPV6_MULTICASTm:
	    return soc_fb_l3x_lookup(unit, key_data,
				     entry_data, index_ptr);
#endif /* INCLUDE_L3 */
	case VLAN_MACm:
	    return soc_fb_vlanmac_entry_lkup(unit, key_data,
					     entry_data, index_ptr);
	default:
	    break;
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_XGS_SWITCH_SUPPORT
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    if (mem == L2Xm) {
        return soc_l2x_lookup(unit, key_data, entry_data, index_ptr);
    }
#endif /* BCM_XGS12_SWITCH_SUPPORT */

#if defined(INCLUDE_L3)
    if (mem == L3Xm) {
        return soc_l3x_lookup(unit, key_data, entry_data, index_ptr);
    }
#endif /* INCLUDE_L3 */
#endif /* BCM_XGS_SWITCH_SUPPORT */

    return _soc_mem_search(unit, mem, copyno, index_ptr, key_data,
                           entry_data, lowest_match);
}


#if defined(BCM_FIREBOLT2_SUPPORT) || defined(BCM_TRX_SUPPORT) \
         || defined(BCM_RAVEN_SUPPORT) || defined(BCM_HAWKEYE_SUPPORT)

/*
 * Function:
 *	soc_mem_bank_insert
 * Purpose:
 *	Do a single insert to a banked hash table with bank selection
 */
int
soc_mem_bank_insert(int unit,
                    soc_mem_t mem,
                    uint8 banks,
                    int copyno,
                    void *entry_data,
                    void *old_entry_data)
{
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        return soc_mem_generic_insert(unit, mem, MEM_BLOCK_ANY,
                                      banks, entry_data, old_entry_data, 0);
    }

    switch (mem) {
    case L2Xm:
        return soc_fb_l2x_bank_insert(unit, banks,
                                      (l2x_entry_t *)entry_data);
#if defined(INCLUDE_L3)
    case L3_ENTRY_IPV4_UNICASTm:
    case L3_ENTRY_IPV4_MULTICASTm:
    case L3_ENTRY_IPV6_UNICASTm:
    case L3_ENTRY_IPV6_MULTICASTm:
        return soc_fb_l3x_bank_insert(unit, banks,
                        (l3_entry_ipv6_multicast_entry_t *)entry_data);
#endif
#if defined(BCM_RAVEN_SUPPORT)
    case VLAN_MACm:
	if (SOC_IS_RAVEN(unit) || SOC_IS_HAWKEYE(unit)) {
	    return soc_fb_vlanmac_entry_bank_ins(unit, banks,
			(vlan_mac_entry_t *)entry_data);
	} else {
	    return SOC_E_UNAVAIL;
	}
#endif
    default:
        break;
    }

    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *	soc_mem_bank_search
 * Purpose:
 *	Search a banked hash table with bank selection
 */
int
soc_mem_bank_search(int unit,
                    soc_mem_t mem,
                    uint8 banks,
                    int copyno,
                    int *index_ptr,
                    void *key_data,
                    void *entry_data)
{
    switch (mem) {
    case L2Xm:
        return soc_fb_l2x_bank_lookup(unit, banks, (l2x_entry_t *)key_data,
                                      (l2x_entry_t *)entry_data, index_ptr);
#if defined(INCLUDE_L3)
    case L3_ENTRY_IPV4_UNICASTm:
    case L3_ENTRY_IPV4_MULTICASTm:
    case L3_ENTRY_IPV6_UNICASTm:
    case L3_ENTRY_IPV6_MULTICASTm:
        return soc_fb_l3x_bank_lookup(unit, banks,
                        (l3_entry_ipv6_multicast_entry_t *)key_data,
                        (l3_entry_ipv6_multicast_entry_t *)entry_data,
                                        index_ptr);
#endif
    default:
        break;
    }

    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *	_soc_mem_dual_hash_get
 * Purpose:
 *	Calcualte hash for dual hashed memories
 */

STATIC int
_soc_mem_dual_hash_get(int unit, soc_mem_t mem, int hash_sel,
                       void *entry_data)
{
    switch (mem) {
    case L2Xm:
#if defined(BCM_TRX_SUPPORT)
    if (SOC_IS_TRX(unit)) {
        return soc_tr_l2x_entry_hash(unit, hash_sel, entry_data);
    } else
#endif 
    {
        return soc_fb_l2x_entry_hash(unit, hash_sel, entry_data);
    }
#if defined(INCLUDE_L3)
    case L3_ENTRY_IPV4_UNICASTm:
    case L3_ENTRY_IPV4_MULTICASTm:
    case L3_ENTRY_IPV6_UNICASTm:
    case L3_ENTRY_IPV6_MULTICASTm:
        return soc_fb_l3x_entry_hash(unit, hash_sel, entry_data);
#endif /* INCLUDE_L3 */
#if defined(BCM_RAVEN_SUPPORT)
    case VLAN_MACm:
        if (SOC_IS_RAVEN(unit)  || SOC_IS_HAWKEYE(unit)) {
            return soc_fb_vlan_mac_entry_hash(unit, hash_sel, entry_data);
        } else {
            return -1;
        }
#endif

#if defined(BCM_TRX_SUPPORT)
    case VLAN_XLATEm:
        if (SOC_IS_TRX(unit)) {
            return soc_tr_vlan_xlate_entry_hash( unit, hash_sel, entry_data);
        }
        return -1;

    case MPLS_ENTRYm:
        if (SOC_IS_TR_VL(unit)) {
            return soc_tr_mpls_entry_hash( unit, hash_sel, entry_data);
        }
        return -1;

    case EGR_VLAN_XLATEm:
        if (SOC_IS_TRX(unit)) {
            return soc_tr_egr_vlan_xlate_entry_hash( unit, hash_sel, entry_data );
        }
        return -1;
#endif

    default:
        return -1;
    }
}

/*
 * Function:
 *	_soc_mem_dual_hash_move
 * Purpose:
 *	Recursive move routine for dual hash auto-move inserts
 *      Assumes feature already checked, mem locks taken.
 */

STATIC int
_soc_mem_dual_hash_move(int unit,
                        soc_mem_t mem,     /* Assumes memory locked */
                        uint8 banks,
                        int copyno,
                        void *entry_data,
                        dual_hash_info_t *hash_info,
                        SHR_BITDCL *bucket_trace,
                        int recurse_depth)
{
    int hash_base, cur_index = -1, base_index, bucket_index;
    int dest_hash_base, dest_bucket_index;
    int half_bucket, trace_size;
    SHR_BITDCL *trace;               /* Buckets involved in recursion.   */
    int rv = SOC_E_NONE, bix, i, found = FALSE;
    uint8 this_bank_bit, that_bank_only, this_hash, that_hash;
    uint32 move_entry[SOC_MAX_MEM_WORDS];

    if (recurse_depth < 0) {
        return SOC_E_FULL;
    }

    /* Stack variables initialization & memory allocations.*/
    half_bucket = hash_info->bucket_size / 2;

    /* Keep back trace of all buckets affected by recursion. */
    trace_size =
        SHR_BITALLOCSIZE(soc_mem_index_count(unit, hash_info->base_mem));
    if (NULL == bucket_trace) {
        trace =  sal_alloc(trace_size, "Dual hash");
        if (NULL == trace) {
            return (SOC_E_MEMORY);
        }
    } else {
        trace = bucket_trace;
    }

    /* Iterate over banks. */
    for (bix = 0; bix < 2; bix++) {
        this_bank_bit =
            (bix == 0) ? SOC_MEM_HASH_BANK0_BIT : SOC_MEM_HASH_BANK1_BIT;
        that_bank_only =
            (bix == 0) ? SOC_MEM_HASH_BANK1_ONLY : SOC_MEM_HASH_BANK0_ONLY;
        this_hash = (bix == 0) ? hash_info->hash_sel0 : hash_info->hash_sel1;
        that_hash = (bix == 0) ? hash_info->hash_sel1 : hash_info->hash_sel0;

        if (banks & this_bank_bit) {
            /* Not this bank */
            continue;
        }
        hash_base = _soc_mem_dual_hash_get(unit, mem, this_hash, entry_data);
        if (hash_base == -1) {
            rv = SOC_E_INTERNAL;
            break;
        }

        /* Bucket half is based on bank id.      */
        bucket_index = hash_base * hash_info->bucket_size + bix * half_bucket;

        /* Recursion trace initialization. */
        if (NULL == bucket_trace) {
            sal_memset(trace, 0, trace_size);
        }
        SHR_BITSET(trace, bucket_index);

        /* Iterate over entries in the half-bucket.. */
        for (i = 0; i < (hash_info->bucket_size / 2); i++) {
            cur_index = base_index = bucket_index + i;
            /* Read bucket entry. */
            rv = soc_mem_read(unit, hash_info->base_mem, copyno,
                              base_index, move_entry);
            if (SOC_FAILURE(rv)) {
                rv = SOC_E_MEMORY;
                break;
            }

            /* Calculate destination entry hash value. */
            dest_hash_base = _soc_mem_dual_hash_get(unit, mem, that_hash, move_entry);
            if (dest_hash_base == -1) {
                rv = SOC_E_INTERNAL;
                break;
            }
            dest_bucket_index =
                dest_hash_base * hash_info->bucket_size + (!bix) * half_bucket;

            /* Make sure we are not touching buckets in bucket trace. */
            if(SHR_BITGET(trace, dest_bucket_index)) {
                continue;
            }

            /* Attempt to insert it into the other bank. */
            rv = soc_mem_bank_insert(unit, mem, that_bank_only,
                                     copyno, move_entry, NULL);
            if (SOC_FAILURE(rv)) {
                if (rv != SOC_E_FULL) {
                    break;
                }
                /* Recursive call - attempt to create a slot   */
                /* in other bank bucket.                       */
                rv = _soc_mem_dual_hash_move(unit, mem, that_bank_only,
                                             copyno, move_entry, hash_info,
                                             trace, recurse_depth - 1);
                if (SOC_FAILURE(rv)) {
                    if (rv != SOC_E_FULL) {
                        break;
                    }
                    continue;
                }
            }
            /* Entry was moved successfully. */
            found = TRUE;
            break;
        }  /* Bucket iteration loop. */

        if (found || ((rv < 0) && (rv != SOC_E_FULL))) {
            break;
        }
    } /* Bank iteration loop. */

    if ((rv < 0) && (rv != SOC_E_FULL)) {
        if (NULL == bucket_trace) sal_free(trace);
        return rv;
    }

    if (!found) {
        if (NULL == bucket_trace) sal_free(trace);
        return SOC_E_FULL;
    }
    rv = soc_mem_write(unit, mem, copyno, cur_index, entry_data);
    if (NULL == bucket_trace) sal_free(trace);
    return (rv);
}

/*
 * Function:
 *	_soc_mem_dual_hash_insert
 * Purpose:
 *	Dual hash auto-move inserts
 *      Assumes feature already checked
 */

int
_soc_mem_dual_hash_insert(int unit,
                          soc_mem_t mem,     /* Assumes memory locked */
                          int copyno,
                          void *entry_data,
                          void *old_entry_data,
                          int recurse_depth)
{
    int rv = SOC_E_NONE;
    dual_hash_info_t hash_info;

    switch (mem) {
    case L2Xm:
        rv = soc_mem_bank_insert(unit, mem, 0, copyno,
                                 entry_data, old_entry_data);
        if (rv != SOC_E_FULL) {
            return rv;
        }
        SOC_IF_ERROR_RETURN
            (soc_fb_l2x_entry_bank_hash_sel_get(unit, 0,
                                                &(hash_info.hash_sel0)));
        SOC_IF_ERROR_RETURN
            (soc_fb_l2x_entry_bank_hash_sel_get(unit, 1,
                                                &(hash_info.hash_sel1)));
        if ((hash_info.hash_sel0 == hash_info.hash_sel1) || 
            (recurse_depth == 0)) {
            /* Can't juggle the entries */
            return SOC_E_FULL;
        }
        hash_info.bucket_size = SOC_L2X_BUCKET_SIZE;
        hash_info.base_mem = mem;

        /* Time to shuffle the entries */
        SOC_IF_ERROR_RETURN(soc_l2x_freeze(unit));
        rv = _soc_mem_dual_hash_move(unit, mem, SOC_MEM_HASH_BANK_BOTH,
                                     copyno, entry_data, &hash_info,
                                     NULL, recurse_depth - 1);
        SOC_IF_ERROR_RETURN(soc_l2x_thaw(unit));
        break;
#if defined(BCM_TRIUMPH_SUPPORT)
    case MPLS_ENTRYm:
        if (SOC_IS_TR_VL(unit)) {
	    rv = soc_mem_bank_insert(unit, mem, 0, copyno,
                                       entry_data, old_entry_data);
            if (rv != SOC_E_FULL) {
                return rv;
            }
             
            SOC_IF_ERROR_RETURN
                (soc_tr_mpls_hash_sel_get(unit, 0, &(hash_info.hash_sel0)));

            SOC_IF_ERROR_RETURN
                (soc_tr_mpls_hash_sel_get(unit, 1, &(hash_info.hash_sel1)));

            if ((hash_info.hash_sel0 == hash_info.hash_sel1) || 
                (recurse_depth == 0)) {
                /* Can't juggle the entries */
                return SOC_E_FULL;
            }
            hash_info.bucket_size = SOC_MPLS_ENTRY_BUCKET_SIZE;
            hash_info.base_mem = mem;
            /* Time to shuffle the entries */
            soc_mem_lock(unit, MPLS_ENTRYm);
    	    rv = _soc_mem_dual_hash_move(unit, mem, SOC_MEM_HASH_BANK_BOTH,
                                         copyno, entry_data, &hash_info,
                                         NULL, recurse_depth - 1);
            soc_mem_unlock(unit, MPLS_ENTRYm);
        }
        break;
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_TRX_SUPPORT
    case EGR_VLAN_XLATEm:
        if (SOC_IS_TRX(unit)) {
	    rv = soc_mem_bank_insert(unit, mem, 0, copyno,
                                       entry_data, old_entry_data);
            if (rv != SOC_E_FULL) {
                return rv;
            }
             
            SOC_IF_ERROR_RETURN
                (soc_tr_egr_vlan_xlate_hash_sel_get(unit, 0, &(hash_info.hash_sel0)));

            SOC_IF_ERROR_RETURN
                (soc_tr_egr_vlan_xlate_hash_sel_get(unit, 1, &(hash_info.hash_sel1)));

            if ((hash_info.hash_sel0 == hash_info.hash_sel1) || 
                (recurse_depth == 0)) {
                /* Can't juggle the entries */
                return SOC_E_FULL;
            }
            hash_info.bucket_size = SOC_EGR_VLAN_XLATE_BUCKET_SIZE;
            hash_info.base_mem = mem;
            /* Time to shuffle the entries */
            soc_mem_lock(unit, EGR_VLAN_XLATEm);
    	    rv = _soc_mem_dual_hash_move(unit, mem, SOC_MEM_HASH_BANK_BOTH,
                                         copyno, entry_data, &hash_info,
                                         NULL, recurse_depth - 1);
            soc_mem_unlock(unit, EGR_VLAN_XLATEm);
        }
        break;
    case VLAN_XLATEm:
        if (!SOC_IS_TRX(unit)) {
            break;
        }
        /* Fall thru for TRX (VLAN_XLATE & VLAN_MAC are a shared table) */
#endif /* BCM_TRX_SUPPORT */
#if defined(BCM_RAVEN_SUPPORT) || defined(BCM_TRX_SUPPORT)
    case VLAN_MACm:
        if (SOC_IS_FIREBOLT2(unit)) {
            break;
        }
	rv = soc_mem_bank_insert(unit, mem, 0, copyno,
                                 entry_data, old_entry_data);
        if (rv != SOC_E_FULL) {
            return rv;
        }
#ifdef BCM_TRX_SUPPORT
        if (mem == VLAN_XLATEm) {
            SOC_IF_ERROR_RETURN
                (soc_tr_vlan_xlate_hash_sel_get(unit, 0, &(hash_info.hash_sel0)));

            SOC_IF_ERROR_RETURN
                (soc_tr_vlan_xlate_hash_sel_get(unit, 1, &(hash_info.hash_sel1)));
            hash_info.bucket_size = SOC_VLAN_XLATE_BUCKET_SIZE;
        } else
#endif /* BCM_TRX_SUPPORT */
        {
            SOC_IF_ERROR_RETURN
                (soc_fb_rv_vlanmac_hash_sel_get(unit, 0, &(hash_info.hash_sel0)));

            SOC_IF_ERROR_RETURN
                (soc_fb_rv_vlanmac_hash_sel_get(unit, 1, &(hash_info.hash_sel1)));
            hash_info.bucket_size = SOC_VLAN_MAC_BUCKET_SIZE;
        }
        if ((hash_info.hash_sel0 == hash_info.hash_sel1) || 
            (recurse_depth == 0)) {
            /* Can't juggle the entries */
            return SOC_E_FULL;
        }
        hash_info.base_mem = mem;
        /* Time to shuffle the entries */
        if (SOC_IS_TRX(unit)) {
            soc_mem_lock(unit, VLAN_XLATEm);
        }
        soc_mem_lock(unit, VLAN_MACm);
    	rv = _soc_mem_dual_hash_move(unit, mem, SOC_MEM_HASH_BANK_BOTH,
                                     copyno, entry_data, &hash_info,
                                     NULL, recurse_depth - 1);
        soc_mem_unlock(unit, VLAN_MACm);
        if (SOC_IS_TRX(unit)) {
            soc_mem_unlock(unit, VLAN_XLATEm);
        }
        break;
#endif /* BCM_RAVEN_SUPPORT || BCM_TRX_SUPPORT */
#if defined(INCLUDE_L3)
    case L3_ENTRY_ONLYm:
    case L3_ENTRY_IPV4_UNICASTm:
    case L3_ENTRY_IPV4_MULTICASTm:
    case L3_ENTRY_IPV6_UNICASTm:
    case L3_ENTRY_IPV6_MULTICASTm:
        return soc_fb_l3x_insert(unit, entry_data);
#endif /* INCLUDE_L3 */
    default:
        return SOC_E_UNAVAIL;
    }

    return rv;
}
#endif /* BCM_FIREBOLT2_SUPPORT || BCM_TRX_SUPPORT \
          || BCM_RAVEN_SUPPORT */

#ifdef BCM_LYNX_SUPPORT
/*
 * Function:
 *	_soc_mem_cam_insert
 * Purpose:
 *	Lynx FFP helper routine for _soc_mem_insert
 */

STATIC int
_soc_mem_cam_insert(int unit,
                    soc_mem_t mem,		/* Assumes memory locked */
                    int copyno,
                    void *entry_data)
{
    uint32		entry_tmp[SOC_MAX_MEM_WORDS];
    uint32		valid_ret;
    int			min, max, ins_index, index, rv;
    int                 min_eff, max_eff, inc_eff;
    int                 all_rules = soc_feature(unit,
                                                soc_feature_filter_krules);
    int                 rules_128 = soc_feature(unit,
                                                soc_feature_filter_128_rules);

    min = soc_mem_index_min(unit, mem);
    max = soc_mem_index_max(unit, mem);
    if (all_rules) {
        min_eff = min;
        max_eff = max;
        inc_eff = 1;
    } else if (rules_128) {
        min_eff = min;
        max_eff = max;
        inc_eff = 8;
    } else {
        min_eff = (min % 2) ? min : (min + 1); /* GNATS 4404: odd parity */
        max_eff = max / 2; /* GNATS 4403: Lynx A0 CAM, lower half only */
        inc_eff = 2;
    }

    rv = _soc_mem_cam_search(unit, mem, copyno,
                            &ins_index, entry_data, entry_tmp);

    if (rv == SOC_E_NONE) {
        /* If entry with matching key already exists, overwrite it. */
        if ((rv = soc_mem_write(unit, mem, copyno,
                                ins_index, entry_data)) < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_cam_insert: write %s.%s[%d] failed\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno), ins_index);
            return rv;
        }

        if (rules_128) { /* Lynx A1 CAM:  All filters are blocks of 8 */
            assert((ins_index % 8) == 0);

            while ((++ins_index) % 8) {
                if ((rv = soc_mem_write(unit, mem, copyno,
                                        ins_index, entry_data)) < 0) {
                    soc_cm_debug(DK_ERR,
                                 "soc_mem_cam_insert: "
				 "write %s.%s[%d] failed\n",
                                 SOC_MEM_UFNAME(unit, mem),
                                 SOC_BLOCK_NAME(unit, copyno), ins_index);
                    return rv;
                }
            }
        }

        return SOC_E_NONE;
    }

    if (rv != SOC_E_NOT_FOUND) {
        return rv;
    }

    /* Point to first unused entry; fail if table is already full. */

    index = min_eff;

    while (index <= max_eff) {
        /* Look for valid entry */
        if ((rv = soc_mem_read(unit, mem, copyno,
                               index, entry_tmp)) < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_insert: read %s.%s[%d] failed\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno), index);
            return rv;
        }

        soc_mem_field_get(unit, mem, entry_tmp, VALIDf, &valid_ret);
        if (!valid_ret) {
            /* Write entry data in blank non-valid entry */
            if ((rv = soc_mem_write(unit, mem, copyno,
                                    index, entry_data) < 0)) {
                soc_cm_debug(DK_ERR,
                             "soc_mem_cam_insert: write %s.%s[%d] failed\n",
                             SOC_MEM_UFNAME(unit, mem),
                             SOC_BLOCK_NAME(unit, copyno), index);
                return rv;
            }

            if (rules_128) { /* Lynx A1 CAM:  All filters are blocks of 8 */
                assert((index % 8) == 0);

                while ((++index) % 8) {
                    if ((rv = soc_mem_write(unit, mem, copyno,
                                            index, entry_data)) < 0) {
                        soc_cm_debug(DK_ERR,
                                     "soc_mem_cam_insert: "
				     "write %s.%s[%d] failed\n",
                                     SOC_MEM_UFNAME(unit, mem),
                                     SOC_BLOCK_NAME(unit, copyno), index);
                        return rv;
                    }
                }
            }

            SOP_MEM_STATE(unit, mem).count[copyno]++;
            return SOC_E_NONE;
        }

        index += inc_eff; /* GNATS 4404: Lynx A0 CAM, odd parity */
    }

    return SOC_E_FULL;
}
#endif /* BCM_LYNX_SUPPORT */

/*
 * Function:
 *	_soc_mem_insert
 * Purpose:
 *	Helper routine for soc_mem_insert
 */

STATIC int
_soc_mem_insert(int unit,
                soc_mem_t mem,		/* Assumes memory locked */
                int copyno,
                void *entry_data)
{
    uint32		entry_tmp[SOC_MAX_MEM_WORDS];
    int			max, ins_index, index, last, rv;

#ifdef BCM_LYNX_SUPPORT
    if (mem == GFILTER_IRULEm && SOC_IS_LYNX(unit)) {
        return _soc_mem_cam_insert(unit, mem, copyno, entry_data);
    }
#endif

#ifdef BCM_DRACO15_SUPPORT
    if (mem == VLAN_MAC_ENTRYm && SOC_IS_DRACO15(unit)) {
        return soc_mem_draco15_vlan_mac_insert(unit, entry_data);
    }
#endif

    max = soc_mem_index_max(unit, mem);
    last = soc_mem_index_last(unit, mem, copyno);

    rv = soc_mem_search(unit, mem, copyno,
                        &ins_index, entry_data, entry_tmp, 0);

#if defined(BCM_TUCANA_SUPPORT)
    /* Check if rule memory on Tucana, in IPORT mode */
    if (SOC_IS_TUCANA(unit) &&
        (mem == GFILTER_IRULEm || mem == FILTER_IRULEm) &&
        (soc_mem_field32_get(unit, mem, entry_data, IPORT_MODEf) ==
         SOC_IPORT_MODE_ARBITRARY_BMAP)) {
        uint32 key_bmp;

        /* Get the bitmap for the entry. */
        key_bmp = soc_mem_field32_get(unit, FILTER_IRULEm, entry_data,
                                      IPORT_BITMAPf);
        if (!key_bmp) {
            /* Blank bitmap, don't waste time */
            return SOC_E_PARAM;
        }

        if (rv == SOC_MEM_PARTIAL_MATCH) {
            /* We need to OR the current bitmap with the entries bitmap */
            /* Or the bitmaps together and continue */
            key_bmp |= soc_mem_field32_get(unit, FILTER_IRULEm, entry_tmp,
                                           IPORT_BITMAPf);
            soc_mem_field32_set(unit, FILTER_IRULEm, entry_data,
                                IPORT_BITMAPf, key_bmp);
            rv = SOC_E_NONE;
        } else if (rv == SOC_E_NOT_FOUND) {
            uint8 bitmap_sofar = 0; /* Bits marked by rules in this group */
            /* It's a new entry. Set up direction vectors for entries in
	     * group.  */
            while (!_soc_mem_cmp_rule_5665(unit, entry_data, entry_tmp)) {
                /* Update other entry's direction vectors. */
                soc_mem_field32_force(unit, mem, entry_tmp, IPORT_DIRECTIONf,
                                      bitmap_sofar);
                rv = soc_mem_write(unit, mem, copyno, ins_index, entry_tmp);
                if (rv <  0) {
                    soc_cm_debug(DK_ERR,
                                 "soc_mem_insert: write %s.%s[%d] failed\n",
                                 SOC_MEM_UFNAME(unit, mem),
                                 SOC_BLOCK_NAME(unit, copyno), ins_index);
                    return rv;
                }
                rv = SOC_E_NOT_FOUND;
                bitmap_sofar |= soc_mem_field32_get(unit, mem, entry_tmp,
                                                    IPORT_BITMAPf);

                if (ins_index++ >= soc_mem_index_max(unit, mem)) {
                    return SOC_E_MEMORY;
                }
                SOC_IF_ERROR_RETURN(
                    soc_mem_read(unit, mem, copyno, ins_index, entry_tmp));
            }
            /* Dir vec is 1 to ports already in group */
            soc_mem_field32_force(unit, mem, entry_data, IPORT_DIRECTIONf,
                                  bitmap_sofar);
        }
    }
#endif

    if (rv == SOC_E_NONE) {
        /* If entry with matching key already exists, overwrite it. */

        if ((rv = soc_mem_write(unit, mem, copyno,
                                ins_index, entry_data)) < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_insert: write %s.%s[%d] failed\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno), ins_index);
            return rv;
        }

        return SOC_E_NONE;
    }

    if (rv != SOC_E_NOT_FOUND) {
        return rv;
    }

    /* Point to first unused entry; fail if table is already full. */

    if ((index = last + 1) > max) {
        return SOC_E_FULL;
    }

    while (index > ins_index) {
        /* Move up one entry */

        if ((rv = soc_mem_read(unit, mem, copyno,
                               index - 1, entry_tmp)) < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_insert: read %s.%s[%d] failed\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno), index - 1);
            return rv;
        }

        if ((rv = soc_mem_write(unit, mem, copyno,
                                index, entry_tmp)) < 0) {
            soc_cm_debug(DK_ERR,
                         "soc_mem_insert: write %s.%s[%d] failed\n",
                         SOC_MEM_UFNAME(unit, mem),
                         SOC_BLOCK_NAME(unit, copyno), index);
            return rv;
        }

        index--;
    }

    /* Write entry data at insertion point */

    if ((rv = soc_mem_write(unit, mem, copyno,
                            ins_index, entry_data)) < 0) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_insert: write %s.%s[%d] failed\n",
                     SOC_MEM_UFNAME(unit, mem),
                     SOC_BLOCK_NAME(unit, copyno), ins_index);
        return rv;
    }

    SOP_MEM_STATE(unit, mem).count[copyno]++;

    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_mem_insert
 * Purpose:
 *	Insert an entry into a structured table based on key.
 * Returns:
 *	SOC_E_NONE - insertion succeeded
 *	SOC_E_FULL - table full
 *	SOC_E_XXX - other error
 * Notes:
 *      Tables may be binary, hashed, CAM, or command.
 *
 *      Binary tables:
 *	A binary search is made for the entry, and if a matching key is
 *	found, the entry is overwritten with new data, and 0 is returned.
 *	If key is not found:
 *	Starting with the last entry (kept in soc_control_t structure),
 *	entries are moved down until a space is made for the required
 *	index.   The moving process results in temporary duplicates but not
 *	temporarily-out-of-order entries.
 *	Fails if the table is already full before the insert.
 *
 *      Hashed tables:
 *      The entry is passed to the hardware, which performs a hash
 *      calculation on the key.  The entry wil be added to the
 *      corresponding hash bucket, if an available slot exists.
 *      Failes if the hash bucket for that key is full.
 *
 *      CAM tables:
 *      Entries are added to the CAM based on the device's
 *      implementation-specific algorithm.
 *      Fails if table resources are exhausted.
 *
 *      Command supported tables:
 *      Uses memory command hardware support to handle table state,
 *      based on the entry key.
 *      Fails if hardware insert operation is unsuccessful.
 *
 *	If COPYNO_ALL is specified, the insertion is repeated for each
 *	copy of the table.
 */

int
soc_mem_insert(int unit,
               soc_mem_t mem,
               int copyno,
               void *entry_data)
{
    int rv = SOC_E_NONE;

    assert(soc_mem_is_sorted(unit, mem) ||
           soc_mem_is_hashed(unit, mem) ||
           soc_mem_is_cam(unit, mem) ||
           soc_mem_is_cmd(unit, mem));
    assert(entry_data);

    if (copyno == COPYNO_ALL) {

#if defined(BCM_EASYRIDER_SUPPORT)
    if (soc_mem_is_cmd(unit, mem)) {
        if (mem == L3_DEFIP_ALGm) {
            rv = soc_er_defip_insert(unit, entry_data);
        } else {
            soc_mem_cmd_t cmd_info;
            sal_memset(&cmd_info, 0, sizeof(cmd_info));
            cmd_info.command = SOC_MEM_CMD_LEARN;
            cmd_info.entry_data = entry_data;
            rv = soc_mem_cmd_op(unit, mem, &cmd_info, FALSE);
        }

        return (rv == SOC_E_NOT_FOUND) ? SOC_E_NONE : rv;
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        switch (mem) {
        case MPLS_ENTRYm:
            if (SOC_IS_SC_CQ(unit)) {
                break;
            }
        case EGR_VLAN_XLATEm:
        case VLAN_XLATEm:
	case VLAN_MACm:
            return _soc_mem_dual_hash_insert(unit, mem, copyno,
                                             entry_data, NULL,
                                             soc_dual_hash_recurse_depth_get(unit, mem));
            break;
        }
    }
#endif

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        switch (mem) {
	case L2Xm:
            return soc_fb_l2x_insert(unit, entry_data);
#if defined(INCLUDE_L3)
	case L3_DEFIPm:
	    return soc_fb_lpm_insert(unit, entry_data);
	case L3_ENTRY_ONLYm:
	case L3_ENTRY_IPV4_UNICASTm:
	case L3_ENTRY_IPV4_MULTICASTm:
	case L3_ENTRY_IPV6_UNICASTm:
	case L3_ENTRY_IPV6_MULTICASTm:
             return soc_fb_l3x_insert(unit, entry_data);
#endif
	case VLAN_MACm:
#if defined(BCM_RAVEN_SUPPORT)
            if (soc_feature(unit, soc_feature_dual_hash) &&
                (SOC_IS_RAVEN(unit) || SOC_IS_HAWKEYE(unit))) {
               return _soc_mem_dual_hash_insert(unit, mem, copyno,
                                                entry_data, NULL,
                                                soc_dual_hash_recurse_depth_get(unit, mem));
            } else
#endif
	    {
	        return soc_fb_vlanmac_entry_ins(unit, entry_data);
	    }
	default:
	    break;
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_XGS_SWITCH_SUPPORT
#if defined(BCM_XGS12_SWITCH_SUPPORT)
        if (mem == L2Xm) {
            return soc_l2x_insert(unit, entry_data);
        }
#endif /* BCM_XGS12_SWITCH_SUPPORT */
#if defined(INCLUDE_L3)
        if (mem == L3Xm) {
            return soc_l3x_insert(unit, entry_data);
        }
#endif /* INCLUDE_L3 */
#endif

        MEM_LOCK(unit, mem);
        SOC_MEM_BLOCK_ITER(unit, mem, copyno) {
            if ((rv = _soc_mem_insert(unit, mem, copyno, entry_data)) < 0) {
                break;
            }
        }
    } else {
        assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
        MEM_LOCK(unit, mem);
        rv = _soc_mem_insert(unit, mem, copyno, entry_data);
    }

    MEM_UNLOCK(unit, mem);

    return rv;
}

/*
 * Function:
 *	soc_mem_insert_return_old
 * Purpose:
 *      Same as soc_mem_insert() with the addtional feature
 *      that if an exiting entry was replaced, the old entry data
 *      is returned back.
 * Returns:
 *	SOC_E_NONE - insertion succeeded
 *	SOC_E_EXISTS - insertion succeeded, old_entry_data is valid
 *	SOC_E_FULL - table full
 *	SOC_E_UNAVAIL - not supported on the specified unit, mem
 *	SOC_E_XXX - other error
 */
int
soc_mem_insert_return_old(int unit,
                          soc_mem_t mem,
                          int copyno,
                          void *entry_data,
                          void *old_entry_data)
{
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        switch (mem) {
        case MPLS_ENTRYm:
            if (SOC_IS_SC_CQ(unit)) {
                break;
            }
        case EGR_VLAN_XLATEm:
        case VLAN_XLATEm:
	case VLAN_MACm:
	case L3_ENTRY_ONLYm:
	case L3_ENTRY_IPV4_UNICASTm:
	case L3_ENTRY_IPV4_MULTICASTm:
	case L3_ENTRY_IPV6_UNICASTm:
	case L3_ENTRY_IPV6_MULTICASTm:
	case L2Xm:
            return _soc_mem_dual_hash_insert(unit, mem, copyno,
                                             entry_data, old_entry_data,
                                             soc_dual_hash_recurse_depth_get(unit, mem));
            break;
        }
    }
#endif
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *	_soc_mem_delete_index
 * Purpose:
 *	Helper function for soc_mem_delete and soc_mem_delete_index.
 */

STATIC int
_soc_mem_delete_index(int unit,
                      soc_mem_t mem,		/* Assumes memory locked */
                      int copyno,
                      int del_index)
{
    uint32		entry_tmp[SOC_MAX_MEM_WORDS];
    int			min, index, eot_index, rv;
#if defined(BCM_TUCANA_SUPPORT)
    int                 iport_bitmap_mode = FALSE;
    uint32              entry_bmp, key_bmp = 0;
    uint32		del_entry[SOC_MAX_MEM_WORDS];
#endif

    /* eot_index points one past last entry in use */

    if (soc_mem_is_cam(unit, mem)) {
        index = del_index;
    } else {
        min = soc_mem_index_min(unit, mem);
        eot_index = soc_mem_index_last(unit, mem, copyno) + 1;

        if (del_index < min || del_index >= eot_index) {
            return SOC_E_NOT_FOUND;
        }

#if defined(BCM_TUCANA_SUPPORT)
        /* Are we in iport bitmap mode?  If so, prep */
        if (SOC_IS_TUCANA(unit) &&
            (mem == GFILTER_IRULEm || mem == FILTER_IRULEm)) {
            if ((rv = soc_mem_read(unit, mem, copyno,
                                   del_index, del_entry)) < 0) {
                soc_cm_debug(DK_ERR,
                             "soc_mem_delete_index: "
                             "read %s.%s[%d] failed\n",
                             SOC_MEM_UFNAME(unit, mem),
                             SOC_BLOCK_NAME(unit, copyno), del_index);
                return rv;
            }

            if ((soc_mem_field32_get(unit, mem, del_entry, IPORT_MODEf) ==
                 SOC_IPORT_MODE_ARBITRARY_BMAP)) {
                /* Get the bitmap for the entry. */
                key_bmp = soc_mem_field32_get(unit, FILTER_IRULEm,
                                              del_entry, IPORT_BITMAPf);
                iport_bitmap_mode = TRUE;
            }
        }
#endif
        for (index = del_index; index + 1 < eot_index; index++) {
            /* Move down one entry */

            if ((rv = soc_mem_read(unit, mem, copyno,
                                   index + 1, entry_tmp)) < 0) {
                soc_cm_debug(DK_ERR,
                             "soc_mem_delete_index: "
                             "read %s.%s[%d] failed\n",
                             SOC_MEM_UFNAME(unit, mem),
                             SOC_BLOCK_NAME(unit, copyno), index + 1);
                return rv;
            }

#if defined(BCM_TUCANA_SUPPORT)
            if (iport_bitmap_mode) {
                /* Is this entry part of the same group? */
                if (!_soc_mem_cmp_rule_5665(unit, del_entry, entry_tmp)) {
                    /* Fix the iport direction vector for this entry */
                    entry_bmp = soc_mem_field32_get(unit, mem, entry_tmp,
                                                    IPORT_DIRECTIONf);
                    entry_bmp &= ~key_bmp; /* Dir 1 to previous ports */
                    soc_mem_field32_force(unit, mem, entry_tmp,
                                          IPORT_DIRECTIONf, entry_bmp);
                } else {
                    /* No, done, so stop wasting time */
                    iport_bitmap_mode = FALSE;
                }
            }
#endif

            if ((rv = soc_mem_write(unit, mem, copyno,
                                    index, entry_tmp)) < 0) {
                soc_cm_debug(DK_ERR,
                             "soc_mem_delete_index: "
                             "write %s.%s[%d] failed\n",
                             SOC_MEM_UFNAME(unit, mem),
                             SOC_BLOCK_NAME(unit, copyno), index);
                return rv;
            }
        }
    }

    if ((rv = soc_mem_write(unit, mem, copyno, index,
                            soc_mem_entry_null(unit, mem))) < 0) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_delete_index: "
                     "write %s.%s[%d] failed\n",
                     SOC_MEM_UFNAME(unit, mem),
                     SOC_BLOCK_NAME(unit, copyno), index + 1);
        return rv;
    }

#ifdef BCM_LYNX_SUPPORT
    if (soc_feature(unit, soc_feature_filter_128_rules)) {
        assert((index % 8) == 0);

        while ((++index) % 8) {
            if ((rv = soc_mem_write(unit, mem, copyno, index,
                                    soc_mem_entry_null(unit, mem))) < 0) {
                soc_cm_debug(DK_ERR,
                             "soc_mem_delete_index: "
                             "write %s.%s[%d] failed\n",
                             SOC_MEM_UFNAME(unit, mem),
                             SOC_BLOCK_NAME(unit, copyno), index + 1);
                return rv;
            }
        }
    }
#endif

    SOP_MEM_STATE(unit, mem).count[copyno]--;

    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_mem_delete_index
 * Purpose:
 *	Delete an entry from a sorted table based on index,
 *	moving up all successive entries.
 * Notes:
 *	If COPYNO_ALL is specified, the deletion is repeated for each
 *	copy of the table.
 *
 *	For deletes from the ARL, reads the entry and requests hardware
 *	deletion by key.  This shouldn't generally be done, because the
 *	hardware may change the ARL table at any time.
 */

int
soc_mem_delete_index(int unit,
                     soc_mem_t mem,
                     int copyno,
                     int index)
{
    int rv = SOC_E_NONE;

    assert(soc_mem_is_sorted(unit, mem) ||
           soc_mem_is_hashed(unit, mem) ||
           soc_mem_is_cam(unit, mem) ||
           soc_mem_is_cmd(unit, mem));

#if defined(BCM_EASYRIDER_SUPPORT)
    if (soc_mem_is_cmd(unit, mem)) {
        uint32 entry_tmp[SOC_MAX_MEM_WORDS];

        SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                         index, entry_tmp));
        return soc_mem_delete(unit, mem, copyno, entry_tmp);
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        uint32 entry_tmp[SOC_MAX_MEM_WORDS];

        switch (mem) {
            case L2Xm:
            case L3_ENTRY_IPV4_UNICASTm:
            case L3_ENTRY_IPV4_MULTICASTm:
            case L3_ENTRY_IPV6_UNICASTm:
            case L3_ENTRY_IPV6_MULTICASTm:
            case EGR_VLAN_XLATEm:
            case VLAN_XLATEm:
            case VLAN_MACm:
            case MPLS_ENTRYm:
            case EXT_L2_ENTRYm:
                SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                                 index, entry_tmp));
                return soc_mem_delete(unit, mem, copyno, entry_tmp);
            default:
                break;
        }
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        uint32 entry_tmp[SOC_MAX_MEM_WORDS];

        switch (mem) {
            case L3_ENTRY_IPV4_UNICASTm:
            case L3_ENTRY_IPV4_MULTICASTm:
            case L3_ENTRY_IPV6_UNICASTm:
            case L3_ENTRY_IPV6_MULTICASTm:
            case L2Xm:
            case VLAN_MACm:
                SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                                 index, entry_tmp));
                return soc_mem_delete(unit, mem, copyno, entry_tmp);
            case L3_DEFIPm:
            default:
                return (SOC_E_UNAVAIL);
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */

    if (copyno == COPYNO_ALL){

#ifdef BCM_XGS_SWITCH_SUPPORT
        uint32 entry_tmp[SOC_MAX_MEM_WORDS];

        if (mem == L2Xm) {
            SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                             index, entry_tmp));
            if (soc_L2Xm_field32_get(unit,
                                     (void *) entry_tmp, VALID_BITf)) {
                return soc_mem_delete(unit, mem, copyno, entry_tmp);
            }
            return SOC_E_NONE;
        }

        if (mem == L3Xm) {
            SOC_IF_ERROR_RETURN(soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                                             index, entry_tmp));
            if (soc_L3Xm_field32_get(unit,
                                     (void *) entry_tmp, L3_VALIDf)) {
                return soc_mem_delete(unit, mem, copyno, entry_tmp);
            }
            return SOC_E_NONE;
        }
#endif /* BCM_XGS_SWITCH_SUPPORT */

        MEM_LOCK(unit, mem);
        SOC_MEM_BLOCK_ITER(unit, mem, copyno) {
            if ((rv = _soc_mem_delete_index(unit, mem, copyno, index)) < 0) {
                break;
            }
        }
    } else {
        assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
        MEM_LOCK(unit, mem);
        rv = _soc_mem_delete_index(unit, mem, copyno, index);
    }

    MEM_UNLOCK(unit, mem);

    return rv;
}

/*
 * Function:
 *	soc_mem_delete
 * Purpose:
 *	Delete an entry from a sorted table based on key,
 *	moving up all successive entries.
 * Returns:
 *	0 on success (whether or not entry was found),
 *	SOC_E_XXX on read or write error.
 * Notes:
 *	A binary search for the key is performed.  If the key is found,
 *	the entry is deleted by moving up successive entries until
 *	the last entry is moved up.  The previously last entry is
 *	overwritten with the null key.
 *
 *	If COPYNO_ALL is specified, the deletion is repeated for each copy
 *	of the table.
 */

int
soc_mem_delete(int unit,
               soc_mem_t mem,
               int copyno,
               void *key_data)
{
    int			index, rv;
    uint32		entry_tmp[SOC_MAX_MEM_WORDS];

    assert(soc_mem_is_sorted(unit, mem) ||
           soc_mem_is_hashed(unit, mem) ||
           soc_mem_is_cam(unit, mem) ||
           soc_mem_is_cmd(unit, mem));
    assert(key_data);

#if defined(BCM_EASYRIDER_SUPPORT)
    if (soc_mem_is_cmd(unit, mem)) {
        if (mem == L3_DEFIP_ALGm) {
            return soc_er_defip_delete(unit, key_data);
        } else {
            soc_mem_cmd_t cmd_info;
            sal_memset(&cmd_info, 0, sizeof(cmd_info));
            cmd_info.command = SOC_MEM_CMD_DELETE;
            cmd_info.entry_data = key_data;
            rv = soc_mem_cmd_op(unit, mem, &cmd_info, FALSE);
            if (((mem == L2_ENTRY_INTERNALm) ||
                 (mem == L2_ENTRY_EXTERNALm) ||
                 (mem == L2_ENTRY_OVERFLOWm)) && (rv >= 0)) {
                int i;
                for (i = 0; i < 50; i++) {
                    rv = soc_mem_search(unit, mem, copyno, &index,
                                        key_data, entry_tmp, 0);
                    if (rv == SOC_E_NOT_FOUND) {
                        rv = SOC_E_NONE;
                        break;
                    }
                    rv = soc_mem_cmd_op(unit, mem, &cmd_info, FALSE);
                    if (rv < 0) {
                        break;
                    }
                }
                if (i == 50) {
                    rv = SOC_E_INTERNAL;
                }
            }
            return (rv);
        }
    }
#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        switch (mem) {
            case L2Xm:
            case L3_ENTRY_IPV4_UNICASTm:
            case L3_ENTRY_IPV4_MULTICASTm:
            case L3_ENTRY_IPV6_UNICASTm:
            case L3_ENTRY_IPV6_MULTICASTm:
            case EGR_VLAN_XLATEm:
            case VLAN_XLATEm:
            case VLAN_MACm:
            case MPLS_ENTRYm:
            case EXT_L2_ENTRYm:
                return soc_mem_generic_delete(unit, mem, MEM_BLOCK_ANY,
                                              0, key_data, NULL, 0);
            default:
                break;
        }
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        switch (mem) {
            case L2Xm:
                return soc_fb_l2x_delete(unit, key_data);
#if defined(INCLUDE_L3)
            case L3_DEFIPm:
                return soc_fb_lpm_delete(unit, key_data);
            case L3_ENTRY_IPV4_UNICASTm:
            case L3_ENTRY_IPV4_MULTICASTm:
            case L3_ENTRY_IPV6_UNICASTm:
            case L3_ENTRY_IPV6_MULTICASTm:
                return soc_fb_l3x_delete(unit, key_data);
#endif
            case VLAN_MACm:
                return soc_fb_vlanmac_entry_del(unit, key_data);
            default:
                break;
        }
    }
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_XGS_SWITCH_SUPPORT
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    if (mem == L2Xm) {
        return soc_l2x_delete(unit, key_data);
    }
#endif /* BCM_XGS12_SWITCH_SUPPORT */
#if defined(INCLUDE_L3)
    if (mem == L3Xm) {
        return soc_l3x_delete(unit, key_data);
    }
#endif /* INCLUDE_L3 */
#endif

#ifdef BCM_DRACO15_SUPPORT
    if ((mem == VLAN_MAC_ENTRYm) && SOC_IS_DRACO15(unit)) {
        return soc_mem_draco15_vlan_mac_delete(unit, key_data);
    }
#endif

    rv = SOC_E_NONE;
    if (copyno == COPYNO_ALL) {
        MEM_LOCK(unit, mem);
        SOC_MEM_BLOCK_ITER(unit, mem, copyno) {
            if ((rv = soc_mem_search(unit, mem, copyno,
                                     &index, key_data, entry_tmp, 0)) < 0) {
                break;
            }
            if ((rv = soc_mem_delete_index(unit, mem, copyno, index)) < 0) {
                break;
            }
        }
    } else {
        assert(SOC_MEM_BLOCK_VALID(unit, mem, copyno));
        MEM_LOCK(unit, mem);
        rv = soc_mem_search(unit, mem, copyno, &index, key_data, entry_tmp, 0);
#if defined(BCM_TUCANA_SUPPORT)
        if (SOC_IS_TUCANA(unit) &&
                (mem == GFILTER_IRULEm || mem == FILTER_IRULEm)) {

            if (rv == SOC_MEM_PARTIAL_MATCH) {
                /* Need to delete the rule for these ports, not others */
                uint32 key_bmp;
                /* Or the bitmaps together and continue */
                key_bmp = soc_mem_field32_get(unit, FILTER_IRULEm, key_data,
                                              IPORT_BITMAPf);
                key_bmp &= ~(soc_mem_field32_get(unit, FILTER_IRULEm,
                                                 entry_tmp, IPORT_BITMAPf));
                soc_mem_field32_set(unit, FILTER_IRULEm, entry_tmp,
                                    IPORT_BITMAPf, key_bmp);
                SOC_IF_ERROR_RETURN(
                    soc_mem_write(unit, mem, copyno, index, entry_tmp));
                return SOC_E_NONE;
            }
        }
#endif
        if (rv >= 0) {
            rv = soc_mem_delete_index(unit, mem, copyno, index);
        }
    }

    MEM_UNLOCK(unit, mem);

    return rv;
}

/*
 * Function:
 *      soc_mem_delete_return_old
 * Purpose:
 *      Same as soc_mem_delete() with the addtional feature
 *      that if an exiting entry was deleted, the old entry data
 *      is returned back.
 * Returns:
 *      SOC_E_NONE - delete succeeded
 *      SOC_E_UNAVAIL - not supported on the specified unit, mem
 *      SOC_E_XXX - other error
 */
int
soc_mem_delete_return_old(int unit,
                          soc_mem_t mem,
                          int copyno,
                          void *key_data,
                          void *old_entry_data)
{
#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit)) {
        switch (mem) {
        case MPLS_ENTRYm:
            if (SOC_IS_SC_CQ(unit)) {
                break;
            }
        case EGR_VLAN_XLATEm:
        case VLAN_XLATEm:
        case VLAN_MACm:
        case L3_ENTRY_ONLYm:
        case L3_ENTRY_IPV4_UNICASTm:
        case L3_ENTRY_IPV4_MULTICASTm:
        case L3_ENTRY_IPV6_UNICASTm:
        case L3_ENTRY_IPV6_MULTICASTm:
        case L2Xm:
            return soc_mem_generic_delete(unit, mem, MEM_BLOCK_ANY,
                                          0, key_data, old_entry_data, 0);
            break;
        }
    }
#endif
    return SOC_E_UNAVAIL;

}

#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
/*
 * Function:
 *	_soc_mem_pop
 * Purpose:
 *	Helper function for soc_mem_pop.
 */
STATIC int
_soc_mem_pop(int unit, soc_mem_t mem, int copyno, void *entry_data)
{
    schan_msg_t schan_msg;
    int         rv, entry_dw;
    uint8       at;

    assert(SOC_MEM_IS_VALID(unit, mem));
    assert(soc_attached(unit));

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_pop: invalid block %d for memory %s\n",
                     copyno, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    schan_msg_clear(&schan_msg);
    schan_msg.popcmd.header.opcode = FIFO_POP_CMD_MSG;
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.readcmd.header.srcblk = 0;
    } else 
#endif
    {
        schan_msg.popcmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
    /* schan_msg.gencmd.header.srcblk = 0;*/
#endif
    }
    schan_msg.popcmd.header.dstblk = SOC_BLOCK2SCH(unit, copyno);
    schan_msg.popcmd.header.cos = 0;
    schan_msg.popcmd.header.datalen = 0;

    schan_msg.gencmd.address = soc_mem_addr_get(unit, mem, copyno, 0, &at);

#ifdef BCM_SIRIUS_SUPPORT
    schan_msg.popcmd.header.srcblk = (SOC_IS_SIRIUS(unit) ? 0 : schan_msg.popcmd.header.srcblk);
    if (SOC_IS_SIRIUS(unit) && (!SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM)) {
	/* mask off the block field */
	schan_msg.popcmd.address &= 0x3F0FFFFF;
    }
#endif

    /*
     * Execute S-Channel "fifo pop" command packet consisting of
     * (header word + address word), and read back
     * (header word + entry_dw) data words.
     */

    rv = soc_schan_op(unit, &schan_msg, 2, entry_dw + 1, 0);

    /* Check result */

    if (schan_msg.popresp.header.opcode != FIFO_POP_DONE_MSG) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_pop: "
                     "invalid S-Channel reply, expected FIFO_POP_DONE_MSG:\n");
        soc_schan_dump(unit, &schan_msg, entry_dw + 2);
        return SOC_E_INTERNAL;
    }

    if (rv == SOC_E_NONE) {
        if (schan_msg.popresp.header.cpu) {
            rv = SOC_E_NOT_FOUND;
        } else {
            sal_memcpy(entry_data, schan_msg.popresp.data, entry_dw * 4);
        }
    }

    if (soc_cm_debug_check(DK_SOCMEM)) {
        soc_cm_debug(DK_SOCMEM, "Fifo pop: ");
        soc_mem_entry_dump(unit, mem, entry_data);
    }

    return rv;
}


/*
 * Function:
 *	_soc_mem_push
 * Purpose:
 *	Helper function for soc_mem_push.
 */

STATIC int
_soc_mem_push(int unit, soc_mem_t mem, int copyno, void *entry_data)
{
    schan_msg_t schan_msg;
    int         rv, entry_dw;
    uint8       at;

    assert(SOC_MEM_IS_VALID(unit, mem));
    assert(soc_attached(unit));

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_push: invalid block %d for memory %s\n",
                     copyno, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    schan_msg_clear(&schan_msg);
    schan_msg.pushcmd.header.opcode = FIFO_PUSH_CMD_MSG;
#ifdef BCM_EXTND_SBUS_SUPPORT
    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        schan_msg.readcmd.header.srcblk = 0;
    } else 
#endif
    {
        schan_msg.pushcmd.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
    /* schan_msg.gencmd.header.srcblk = 0;*/
#endif
    }
    schan_msg.pushcmd.header.dstblk = SOC_BLOCK2SCH(unit, copyno);
    schan_msg.pushcmd.header.cos = 0;
    schan_msg.pushcmd.header.datalen = entry_dw * 4;

    schan_msg.gencmd.address = soc_mem_addr_get(unit, mem, copyno, 0, &at);
 
    /* Fill in packet data */
    sal_memcpy(schan_msg.pushcmd.data, entry_data, entry_dw * 4);

    /*
     * Execute S-Channel "fifo push" command packet consisting of
     * (header word + address word + entry_dw), and read back
     * (header word + entry_dw) data words.
     */

    rv = soc_schan_op(unit, &schan_msg, entry_dw + 2, entry_dw + 1, 0);

    /* Check result */

    if (schan_msg.pushresp.header.opcode != FIFO_PUSH_DONE_MSG) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_push: "
                     "invalid S-Channel reply, expected FIFO_PUSH_DONE_MSG:\n");
        soc_schan_dump(unit, &schan_msg, entry_dw + 2);
        return SOC_E_INTERNAL;
    }

    if ((rv == SOC_E_NONE) && (schan_msg.pushresp.header.cpu)) {
        rv = SOC_E_FULL;
    }

    if (soc_cm_debug_check(DK_SOCMEM)) {
        soc_cm_debug(DK_SOCMEM, "Fifo push: ");
        soc_mem_entry_dump(unit, mem, entry_data);
    }

    return rv;
}

static const struct {
    soc_reg_t   cfg;
    soc_reg_t   sbus_addr;
    soc_reg_t   hostmem_addr;
    soc_reg_t   read_ptr;
    soc_reg_t   write_ptr;
    soc_reg_t   threshold;
    soc_field_t overflow_fld;
    int         overflow_bit;
} _soc_mem_fifo_dma[] = {
    {
        CMIC_FIFO_CH0_RD_DMA_CFGr,
        CMIC_FIFO_CH0_RD_DMA_SBUS_START_ADDRESSr,
        CMIC_FIFO_CH0_RD_DMA_HOSTMEM_START_ADDRESSr,
        CMIC_FIFO_CH0_RD_DMA_HOSTMEM_READ_PTRr,
        CMIC_FIFO_CH0_RD_DMA_HOSTMEM_WRITE_PTRr,
        CMIC_FIFO_CH0_RD_DMA_HOSTMEM_THRESHOLDr,
        FIFO_CH0_DMA_HOSTMEM_OVERFLOWf,
        0,
    },
    {
        CMIC_FIFO_CH1_RD_DMA_CFGr,
        CMIC_FIFO_CH1_RD_DMA_SBUS_START_ADDRESSr,
        CMIC_FIFO_CH1_RD_DMA_HOSTMEM_START_ADDRESSr,
        CMIC_FIFO_CH1_RD_DMA_HOSTMEM_READ_PTRr,
        CMIC_FIFO_CH1_RD_DMA_HOSTMEM_WRITE_PTRr,
        CMIC_FIFO_CH1_RD_DMA_HOSTMEM_THRESHOLDr,
        FIFO_CH1_DMA_HOSTMEM_OVERFLOWf,
        2,
    },
    {
        CMIC_FIFO_CH2_RD_DMA_CFGr,
        CMIC_FIFO_CH2_RD_DMA_SBUS_START_ADDRESSr,
        CMIC_FIFO_CH2_RD_DMA_HOSTMEM_START_ADDRESSr,
        CMIC_FIFO_CH2_RD_DMA_HOSTMEM_READ_PTRr,
        CMIC_FIFO_CH2_RD_DMA_HOSTMEM_WRITE_PTRr,
        CMIC_FIFO_CH2_RD_DMA_HOSTMEM_THRESHOLDr,
        FIFO_CH2_DMA_HOSTMEM_OVERFLOWf,
        4,
    },
    {
        CMIC_FIFO_CH3_RD_DMA_CFGr,
        CMIC_FIFO_CH3_RD_DMA_SBUS_START_ADDRESSr,
        CMIC_FIFO_CH3_RD_DMA_HOSTMEM_START_ADDRESSr,
        CMIC_FIFO_CH3_RD_DMA_HOSTMEM_READ_PTRr,
        CMIC_FIFO_CH3_RD_DMA_HOSTMEM_WRITE_PTRr,
        CMIC_FIFO_CH3_RD_DMA_HOSTMEM_THRESHOLDr,
        FIFO_CH3_DMA_HOSTMEM_OVERFLOWf,
        6,
    },
};
STATIC int
_soc_mem_fifo_dma_start(int unit, int chan, soc_mem_t mem, int copyno,
                        int host_entries, void *host_buf)
{
    soc_control_t  *soc = SOC_CONTROL(unit);
    soc_reg_t cfg_reg;
    uint32 addr, rval, data_beats, sel, spacing;
    uint8 at;

    if (chan < 0 || chan > 3 || host_buf == NULL) {
        return SOC_E_PARAM;
    }

    cfg_reg = _soc_mem_fifo_dma[chan].cfg;

    switch (host_entries) {
    case 64:    sel = 0; break;
    case 128:   sel = 1; break;
    case 256:   sel = 2; break;
    case 512:   sel = 3; break;
    case 1024:  sel = 4; break;
    case 2048:  sel = 5; break;
    case 4096:  sel = 6; break;
    case 8192:  sel = 7; break;
    case 16384: sel = 8; break;
    case 32768: sel = 9; break;
    case 65536: sel = 10; break;
    default:
        return SOC_E_PARAM;
    }

    if (mem != ING_IPFIX_EXPORT_FIFOm && mem != EGR_IPFIX_EXPORT_FIFOm &&
        mem != EXT_L2_MOD_FIFOm && mem != L2_MOD_FIFOm &&
        mem != CS_EJECTION_MESSAGE_TABLEm) {
        return SOC_E_BADID;
    }

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }

    data_beats = soc_mem_entry_words(unit, mem);

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].sbus_addr, REG_PORT_ANY,
                        0);
    rval = soc_mem_addr_get(unit, mem, copyno, 0, &at);
    soc_pci_write(unit, addr, rval);

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].hostmem_addr,
                        REG_PORT_ANY, 0);
    rval = soc_cm_l2p(unit, host_buf);
    soc_pci_write(unit, addr, rval);

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].read_ptr, REG_PORT_ANY,
                        0);
    soc_pci_write(unit, addr, rval);

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].threshold, REG_PORT_ANY,
                        0);
    rval = 0;
    soc_reg_field_set(unit, _soc_mem_fifo_dma[chan].threshold, &rval, ADDRESSf,
                      host_entries / 16 * data_beats * sizeof(uint32));
    soc_pci_write(unit, addr, rval);

    addr = soc_reg_addr(unit, cfg_reg, REG_PORT_ANY, 0);
    rval = 0;
    soc_reg_field_set(unit, cfg_reg, &rval, BEAT_COUNTf, data_beats);
    soc_reg_field_set(unit, cfg_reg, &rval, HOST_NUM_ENTRIES_SELf, sel);
    soc_reg_field_set(unit, cfg_reg, &rval, TIMEOUT_COUNTf, 200);
    if (soc_feature(unit, soc_feature_multi_sbus_cmds)) {
        
        if (soc->sbusCmdSpacing < 0) {
            spacing = data_beats > 7 ? data_beats + 1 : 8;
        } else {
            spacing = soc->sbusCmdSpacing;
        }
        if ((SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_ESM) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_XQPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_GXPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_XLPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_SPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_GPORT) ||
            (SOC_BLOCK_TYPE(unit, copyno) == SOC_BLK_QGPORT)) {
            spacing = 0;
        }
        if (spacing) {
            soc_reg_field_set(unit, cfg_reg, &rval,
                              MULTIPLE_SBUS_CMD_SPACINGf, spacing);
            soc_reg_field_set(unit, cfg_reg, &rval,
                              ENABLE_MULTIPLE_SBUS_CMDSf, 1);
        }
    }
    soc_pci_write(unit, addr, rval);

    soc_reg_field_set(unit, cfg_reg, &rval, ENABLEf, 1);
    soc_reg_field_set(unit, cfg_reg, &rval, ENABLE_VALf, 1);
    soc_pci_write(unit, addr, rval);

    return SOC_E_NONE;
}

STATIC int
_soc_mem_fifo_dma_stop(int unit, int chan)
{
    uint32 addr, rval;

    if (chan < 0 || chan > 3) {
        return SOC_E_PARAM;
    }

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].cfg, REG_PORT_ANY, 0);
    rval = 0;
    soc_reg_field_set(unit, _soc_mem_fifo_dma[chan].cfg, &rval, ENABLEf, 1);
    soc_pci_write(unit, addr, rval);

    return SOC_E_NONE;
}

STATIC int
_soc_mem_fifo_dma_get_read_ptr(int unit, int chan, void **host_ptr, int *count)
{
    soc_reg_t cfg_reg;
    int host_entries, data_beats;
    uint32 addr, rval, debug, hostmem_addr, read_ptr, write_ptr;

    if (chan < 0 || chan > 3 || host_ptr == NULL) {
        return SOC_E_PARAM;
    }

    cfg_reg = _soc_mem_fifo_dma[chan].cfg;

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].read_ptr, REG_PORT_ANY,
                        0);
    soc_pci_getreg(unit, addr, &read_ptr);

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].write_ptr, REG_PORT_ANY,
                        0);
    soc_pci_getreg(unit, addr, &write_ptr);

    if (write_ptr == 0) {
        return SOC_E_EMPTY;
    }

    if (read_ptr == write_ptr) {
        addr = soc_reg_addr(unit, CMIC_FIFO_RD_DMA_DEBUGr, REG_PORT_ANY, 0);
        soc_pci_getreg(unit, addr, &debug);
        if (!soc_reg_field_get(unit, CMIC_FIFO_RD_DMA_DEBUGr,
                               debug, _soc_mem_fifo_dma[chan].overflow_fld)) {
            return SOC_E_EMPTY;
        }
        /* Re-read write pointer */
        addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].write_ptr,
                            REG_PORT_ANY, 0);
        soc_pci_getreg(unit, addr, &write_ptr);
    }

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].hostmem_addr,
                        REG_PORT_ANY, 0);
    soc_pci_getreg(unit, addr, &hostmem_addr);

    addr = soc_reg_addr(unit, cfg_reg, REG_PORT_ANY, 0);
    soc_pci_getreg(unit, addr, &rval);
    data_beats = soc_reg_field_get(unit, cfg_reg, rval, BEAT_COUNTf);
    if (data_beats <= 0) {
        soc_cm_debug(DK_ERR, "Invalid BEAT_COUNT (%d) in "
                     "CMIC_FIFO_CH%d_RD_DMA_CFG \n", data_beats, chan);
        return SOC_E_CONFIG;
    }

    switch (soc_reg_field_get(unit, cfg_reg, rval, HOST_NUM_ENTRIES_SELf)) {
    case 0:  host_entries = 64;    break;
    case 1:  host_entries = 128;   break;
    case 2:  host_entries = 256;   break;
    case 3:  host_entries = 512;   break;
    case 4:  host_entries = 1024;  break;
    case 5:  host_entries = 2048;  break;
    case 6:  host_entries = 4096;  break;
    case 7:  host_entries = 8192;  break;
    case 8:  host_entries = 16384; break;
    case 9:  host_entries = 32768; break;
    case 10: host_entries = 65536; break;
    default: return SOC_E_CONFIG;
    }

    *host_ptr = soc_cm_p2l(unit, read_ptr);
    if (read_ptr >= write_ptr) {
        *count = host_entries -
            (read_ptr - hostmem_addr) / data_beats / sizeof(uint32);
    } else {
        *count = (write_ptr - read_ptr) / data_beats / sizeof(uint32);
    }

    if (SAL_BOOT_QUICKTURN) {
        /* Delay to ensure PCI DMA tranfer to host memory completes */
        sal_usleep(soc_mem_fifo_delay_value);
    }
    return (*count) ? SOC_E_NONE : SOC_E_EMPTY;
}

STATIC int
_soc_mem_fifo_dma_advance_read_ptr(int unit, int chan, int count)
{
    soc_reg_t cfg_reg;
    int host_entries, data_beats;
    uint32 addr, rval, debug;
    uint32 *host_buf, *read_ptr;

    if (chan < 0 || chan > 3) {
        return SOC_E_PARAM;
    }

    cfg_reg = _soc_mem_fifo_dma[chan].cfg;

    addr = soc_reg_addr(unit, cfg_reg, REG_PORT_ANY, 0);
    soc_pci_getreg(unit, addr, &rval);
    data_beats = soc_reg_field_get(unit, cfg_reg, rval, BEAT_COUNTf);

    switch (soc_reg_field_get(unit, cfg_reg, rval, HOST_NUM_ENTRIES_SELf)) {
    case 0:  host_entries = 64;    break;
    case 1:  host_entries = 128;   break;
    case 2:  host_entries = 256;   break;
    case 3:  host_entries = 512;   break;
    case 4:  host_entries = 1024;  break;
    case 5:  host_entries = 2048;  break;
    case 6:  host_entries = 4096;  break;
    case 7:  host_entries = 8192;  break;
    case 8:  host_entries = 16384; break;
    case 9:  host_entries = 32768; break;
    case 10: host_entries = 65536; break;
    default: return SOC_E_CONFIG;
    }

    if (count < 0 || count >= host_entries) {
        return SOC_E_PARAM;
    }

    /* Clear threshold overflow bit */
    addr = soc_reg_addr(unit, CMIC_FIFO_RD_DMA_DEBUGr, REG_PORT_ANY, 0);
    debug = 0;
    soc_reg_field_set(unit, CMIC_FIFO_RD_DMA_DEBUGr,
                      &debug, BIT_POSf, _soc_mem_fifo_dma[chan].overflow_bit);
    soc_pci_write(unit, addr, debug);

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].hostmem_addr,
                        REG_PORT_ANY, 0);
    soc_pci_getreg(unit, addr, &rval);
    host_buf = soc_cm_p2l(unit, rval);

    addr = soc_reg_addr(unit, _soc_mem_fifo_dma[chan].read_ptr, REG_PORT_ANY,
                        0);
    soc_pci_getreg(unit, addr, &rval);
    read_ptr = soc_cm_p2l(unit, rval);

    read_ptr += count * data_beats;
    if (read_ptr >= &host_buf[host_entries * data_beats]) {
        read_ptr -= host_entries * data_beats;
    }
    rval = soc_cm_l2p(unit, read_ptr);
    soc_pci_write(unit, addr, rval);

    return SOC_E_NONE;
}

#endif /* BCM_TRX_SUPPORT || BCM_SIRIUS_SUPPORT*/

/*
 * Function:
 *      soc_mem_pop
 * Purpose:
 *      Pop an entry from a FIFO.
 * Parameters:
 *      mem  		     Memory to search
 *      copyno               Which copy to search (if multiple copies)
 *      entry_data     	     OUT: Data if found
 *                           CAN NO LONGER BE NULL.  Must be big enough
 *                           to handle the appropriate data.
 * Returns:
 *	SOC_E_NOT_FOUND      Entry not found: Fifo is empty
 *	SOC_E_NONE	     Entry is found
 *	SOC_E_XXX	     If internal error occurs
 */

int
soc_mem_pop(int unit,
            soc_mem_t mem,
            int copyno,
            void *entry_data)
{
#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_mem_push_pop)) {
        switch (mem) {
            case ING_IPFIX_EXPORT_FIFOm:
            case EGR_IPFIX_EXPORT_FIFOm:
            case EXT_L2_MOD_FIFOm:
            case L2_MOD_FIFOm:
                return _soc_mem_pop(unit, mem, copyno, entry_data);
            default:
                return SOC_E_BADID;
                break;
        }
    }
#endif /* BCM_TRX_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    
    if (soc_feature(unit, soc_feature_mem_push_pop)) {
        switch (mem) {
	    /* return _soc_mem_pop(unit, mem, copyno, entry_data); */
            default:
		soc_cm_debug(DK_ERR,
			     "soc_mem_pop: implement mem POP\n");
                return SOC_E_BADID;
                break;
        }
    }
#endif /* BCM_SIRIUS_SUPPORT */

    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *      soc_mem_push
 * Purpose:
 *      Push an entry to a FIFO.
 * Parameters:
 *      mem                  Memory to search
 *      copyno               Which copy to search (if multiple copies)
 *      entry_data           In: Data to be pushed
 *
 * Returns:
 *      SOC_E_FULL           Fifo is full
 *      SOC_E_NONE           Entry is pushed successfully
 *      SOC_E_XXX            If internal error occurs
 */

int
soc_mem_push(int unit,
             soc_mem_t mem,
             int copyno,
             void *entry_data)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        soc_cm_debug(DK_ERR,
                     "soc_mem_push: not supported on %s\n",
                     SOC_CHIP_STRING(unit));
	return SOC_E_UNAVAIL;
    }
#endif

#ifdef BCM_TRX_SUPPORT
    if (soc_feature(unit, soc_feature_mem_push_pop)) {
        switch (mem) {
            case ING_IPFIX_EXPORT_FIFOm:
            case EGR_IPFIX_EXPORT_FIFOm:
            case EXT_L2_MOD_FIFOm:
            case L2_MOD_FIFOm:
                return _soc_mem_push(unit, mem, copyno, entry_data);
            default:
                return SOC_E_BADID;
                break;
        }
    }
#endif /* BCM_TRX_SUPPORT */

    return SOC_E_UNAVAIL;
}

int
soc_mem_fifo_dma_start(int unit, int chan, soc_mem_t mem, int copyno,
                       int host_entries, void *host_buf)
{
#if defined(BCM_KATANA_SUPPORT) && defined(BCM_CMICM_SUPPORT)
    if (soc_feature(unit, soc_feature_cmicm)) {
        return _soc_mem_kt_fifo_dma_start(unit, chan, mem, copyno,
                 host_entries, host_buf);
    }
#endif
#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (soc_feature(unit, soc_feature_fifo_dma)) {
        return _soc_mem_fifo_dma_start(unit, chan, mem, copyno,
                host_entries, host_buf);
    }
#endif /* BCM_TRX_SUPPORT || BCM_SIRIUS_SUPPORT */
    return SOC_E_UNAVAIL;
}

int
soc_mem_fifo_dma_stop(int unit, int chan)
{
#if defined(BCM_KATANA_SUPPORT) && defined(BCM_CMICM_SUPPORT)
    if (soc_feature(unit, soc_feature_cmicm)) {
        return _soc_mem_kt_fifo_dma_stop(unit, chan);
    }
#endif
#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (soc_feature(unit, soc_feature_fifo_dma)) {
        return _soc_mem_fifo_dma_stop(unit, chan);
    }
#endif /* BCM_TRX_SUPPORT || BCM_SIRIUS_SUPPORT */
    return SOC_E_UNAVAIL;
}

int
soc_mem_fifo_dma_get_read_ptr(int unit, int chan, void **host_ptr, int *count)
{
#if defined(BCM_KATANA_SUPPORT) && defined(BCM_CMICM_SUPPORT)
    if (soc_feature(unit, soc_feature_cmicm)) {
        return _soc_mem_kt_fifo_dma_get_read_ptr(unit, chan, host_ptr, count);
    }
#endif
#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (soc_feature(unit, soc_feature_fifo_dma)) {
        return _soc_mem_fifo_dma_get_read_ptr(unit, chan, host_ptr, count);
    }
#endif /* BCM_TRX_SUPPORT || BCM_SIRIUS_SUPPORT */

    return SOC_E_UNAVAIL;
}

int
soc_mem_fifo_dma_advance_read_ptr(int unit, int chan, int count)
{
#if defined(BCM_KATANA_SUPPORT) && defined(BCM_CMICM_SUPPORT)
    if (soc_feature(unit, soc_feature_cmicm)) {
        return _soc_mem_kt_fifo_dma_advance_read_ptr(unit, chan, count);
    }
#endif
#if defined(BCM_TRX_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (soc_feature(unit, soc_feature_fifo_dma)) {
        return _soc_mem_fifo_dma_advance_read_ptr(unit, chan, count);
    }
#endif /* BCM_TRX_SUPPORT || BCM_SIRIUS_SUPPORT */

    return SOC_E_UNAVAIL;
}

#ifdef BCM_XGS_SWITCH_SUPPORT

/*
 * Function:
 *	soc_vlan_entries
 * Purpose:
 *	Return the number of entries in VLAN_TAB with the VALID bit set.
 */

STATIC int
soc_vlan_entries(int unit)
{
    int			index_min, index_max, index;
    vlan_tab_entry_t	ve;
    int			total;

    index_min = soc_mem_index_min(unit, VLAN_TABm);
    index_max = soc_mem_index_max(unit, VLAN_TABm);

    total = 0;

    for (index = index_min; index <= index_max; index++) {
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, VLAN_TABm, MEM_BLOCK_ANY, index, &ve));

        total += soc_VLAN_TABm_field32_get(unit, &ve, VALIDf);
    }

    return total;
}

#endif /* BCM_XGS_SWITCH_SUPPORT */

/*
 * Function:
 *	soc_mem_entries
 * Purpose:
 *	Return the number of entries in a sorted table.
 * Notes:
 *	Performs special functions for some tables.
 */

int
soc_mem_entries(int unit,
                soc_mem_t mem,
                int copyno)
{
    int			entries = 0;
#ifdef BCM_EASYRIDER_SUPPORT
    uint32              rval;
    uint32              fval;
#endif

    assert(SOC_MEM_IS_VALID(unit, mem));
    assert(soc_attached(unit));
    assert(soc_mem_is_sorted(unit, mem) ||
           soc_mem_is_hashed(unit, mem) ||
           soc_mem_is_cam(unit, mem) ||
           soc_mem_is_cmd(unit, mem) ||
           mem == VLAN_TABm);

    if (copyno == MEM_BLOCK_ANY) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        soc_cm_debug(DK_WARN,
                     "soc_mem_entries: invalid block %d for memory %s\n",
                     copyno, SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    switch (mem) {
#ifdef BCM_XGS_SWITCH_SUPPORT
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    case L2Xm:
        entries = soc_l2x_entries(unit);	/* Warning: very slow */
        break;
#endif /* BCM_XGS12_SWITCH_SUPPORT */
#if defined(INCLUDE_L3)
    case L3Xm:
        entries = soc_l3x_entries(unit);	/* Warning: very slow */
        break;
#endif /* INCLUDE_L3 */
    case VLAN_TABm:
        entries = soc_vlan_entries(unit);	/* Warning: very slow */
        break;
#endif

    case EGR_VLAN_XLATEm:
#ifdef BCM_EASYRIDER_SUPPORT
        if (SOC_IS_EASYRIDER(unit)) {
            SOC_IF_ERROR_RETURN
                (READ_EVTX_ENTRY_SRCH_AVAILr(unit, &rval));
            fval = soc_reg_field_get(unit, EVTX_ENTRY_SRCH_AVAILr, rval,
                                     EVTX_ENTRY_SRCH_AVAIL_BITSf);
            entries = soc_mem_index_count(unit, mem) - fval;
        } else
#endif
        {
            entries = SOP_MEM_STATE(unit, mem).count[copyno];
        }
        break;

#ifdef BCM_EASYRIDER_SUPPORT
    case IGR_VLAN_XLATEm:
        SOC_IF_ERROR_RETURN
            (READ_IVTX_ENTRY_SRCH_AVAILr(unit, &rval));
        fval = soc_reg_field_get(unit, IVTX_ENTRY_SRCH_AVAILr, rval,
                                   IVTX_ENTRY_SRCH_AVAIL_BITSf);
        entries = soc_mem_index_count(unit, mem) - fval;
        break;
    case VRF_VFI_INTFm:
        SOC_IF_ERROR_RETURN
            (READ_IIF_ENTRY_SRCH_AVAILr(unit, &rval));
        fval = soc_reg_field_get(unit, IIF_ENTRY_SRCH_AVAILr, rval,
                                   IIF_ENTRY_SRCH_AVAIL_BITSf);
        entries = soc_mem_index_count(unit, mem) - fval;
        break;
    case IPMC_GROUP_V4m:
        SOC_IF_ERROR_RETURN
            (READ_IPMC_ENTRY_V4_BLKCNTr(unit, &rval));
        fval = soc_reg_field_get(unit, IPMC_ENTRY_V4_BLKCNTr, rval,
                                   IPMC_ENTRY_V4_BLKCNT_BITSf);
        entries = (soc_mem_index_count(unit, mem) /
                   SOC_ER_IPMC_GROUP_TBL_BLK_NUM) * fval;
        SOC_IF_ERROR_RETURN
            (READ_IPMC_ENTRY_V4_AVAILr(unit, &rval));
        fval = soc_reg_field_get(unit, IPMC_ENTRY_V4_AVAILr, rval,
                                   IPMC_ENTRY_V4_AVAIL_BITSf);
        entries -= fval;
        break;
    case IPMC_GROUP_V6m:
        SOC_IF_ERROR_RETURN
            (READ_IPMC_ENTRY_V6_BLKCNTr(unit, &rval));
        fval = soc_reg_field_get(unit, IPMC_ENTRY_V6_BLKCNTr, rval,
                                   IPMC_ENTRY_V6_BLKCNT_BITSf);
        entries = (soc_mem_index_count(unit, mem) /
                   SOC_ER_IPMC_GROUP_TBL_BLK_NUM) * fval;
        SOC_IF_ERROR_RETURN
            (READ_IPMC_ENTRY_V6_AVAILr(unit, &rval));
        fval = soc_reg_field_get(unit, IPMC_ENTRY_V6_AVAILr, rval,
                                   IPMC_ENTRY_V6_AVAIL_BITSf);
        entries -= fval;
        break;
    case L2_ENTRY_EXTERNALm:
        entries = soc_er_l2x_entries_external(unit); /* Warning: very slow */
        break;
    case L2_ENTRY_INTERNALm:
        entries = soc_er_l2x_entries_internal(unit);
        break;
    case L2_ENTRY_OVERFLOWm:
        entries = soc_er_l2x_entries_overflow(unit);
        break;
#endif /* BCM_EASYRIDER_SUPPORT */
    default:
        entries = SOP_MEM_STATE(unit, mem).count[copyno];
        break;
    }

    return entries;
}

/*
 * Function:
 *	soc_mem_debug_set
 * Purpose:
 *	Enable or disable MMU debug mode.
 * Returns:
 *	Previous enable state, or SOC_E_XXX on error.
 */

int
soc_mem_debug_set(int unit, int enable)
{
    schan_msg_t schan_msg;
    int	msg, rv = SOC_E_NONE, old_enable;

    old_enable = SOC_PERSIST(unit)->debugMode;

    if (enable && !old_enable) {
        msg = ENTER_DEBUG_MODE_MSG;
    } else if (!enable && old_enable) {
        msg = EXIT_DEBUG_MODE_MSG;
    } else {
        msg = -1;
    }

    if (msg >= 0) {
        schan_msg_clear(&schan_msg);
        schan_msg.header.opcode = msg;
        schan_msg.header.dstblk = SOC_BLOCK2SCH(unit, MMU_BLOCK(unit));
        schan_msg.header.srcblk = SOC_BLOCK2SCH(unit, CMIC_BLOCK(unit));
#if defined(BCM_SHADOW_SUPPORT)
    /* schan_msg.gencmd.header.srcblk = 0;*/
#endif

#ifdef BCM_SIRIUS_SUPPORT
	schan_msg.header.srcblk = (SOC_IS_SIRIUS(unit) ? 0 : schan_msg.header.srcblk);
#endif

        if ((rv = soc_schan_op(unit, &schan_msg, 1, 0, 0)) >= 0) {
            SOC_PERSIST(unit)->debugMode = enable;
        }

        if (!enable) {
            /* Allow packet transfers in progress to drain */
            sal_usleep(100000);
        }
    }

    return (rv < 0) ? rv : old_enable;
}

/*
 * Function:
 *	soc_mem_debug_get
 * Purpose:
 *	Return current MMU debug mode status
 */

int
soc_mem_debug_get(int unit, int *enable)
{
    *enable = SOC_PERSIST(unit)->debugMode;
    return SOC_E_NONE;
}

/*
 * Function:
 *	soc_mem_iterate
 * Purpose:
 *	Iterate over all the valid memories and call user
 *	passed callback function (do_it) for each valid memory.
 */
int
soc_mem_iterate(int unit, soc_mem_iter_f do_it, void *data)
{
    soc_mem_t		mem;
    int                 rv = SOC_E_NONE;

    if (!do_it) {
        SOC_ERROR_PRINT((DK_ERR,
                         "soc_mem_iterate: Callback function is NULL"));
        return SOC_E_PARAM;
    }

    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
	if (!SOC_MEM_IS_VALID(unit, mem)) {
	    continue;
        }

#if defined(BCM_HAWKEYE_SUPPORT)
        if (SOC_IS_HAWKEYE(unit) && (soc_mem_index_max(unit, mem) <= 0)) {
            continue;
        }
#endif /* BCM_HAWKEYE_SUPPORT */

        /*
         * Call user provided callback function.
         */
        if ((rv = do_it(unit, mem, data)) < 0) {
            SOC_ERROR_PRINT((DK_ERR,
                         "soc_mem_iterate: Failed on memory (%s)\n",
                         SOC_MEM_NAME(unit, mem)));
            break;
        }
    }
    return rv;
}


/*
 * Function:   soc_mem_fields32_modify
 * Purpose:    Modify the value of a fields in a memory.
 * Parameters:
 *       unit         - (IN) SOC unit number.
 *       mem          - (IN) Memory.
 *       index        - (IN) Memory entry index.
 *       field_count  - (IN) Number of fields to modify.
 *       fields       - (IN) Modified fields array.
 *       values       - (IN) New value for each member of fields array.
 * Returns:
 *       SOC_E_XXX
 */
int
soc_mem_fields32_modify(int unit, soc_mem_t mem, int index,
                        int field_count, soc_field_t *fields, uint32 *values)
{
    uint32 buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read memory entry. */
    int idx;                             /* Iteration index.             */
    int rv;                              /* Operation return status.     */

    /* Check that memory is a valid one for this unit. */
    if (!SOC_MEM_IS_VALID(unit, mem)) {
        return (SOC_E_UNAVAIL);
    }

    /* Check entry index range. */
    if ((index > soc_mem_index_max(unit, mem)) ||
        (index < soc_mem_index_min(unit, mem))) {
        return (SOC_E_PARAM);
    }

    /*  Fields & values sanity check. */
    for (idx = 0; idx < field_count; idx++) {
        if ((NULL == fields + idx) || (NULL == values + idx)) {
            return (SOC_E_PARAM);
        }

        /* Make sure value fits into memory field. */
        SOC_IF_ERROR_RETURN
            (soc_mem_field32_fit(unit, mem, fields[idx], values[idx]));
    }

    /* Lock the memory. */
    soc_mem_lock(unit, mem);
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, index, buf);
    if (SOC_FAILURE(rv)) {
        soc_mem_unlock(unit, mem);
        return (rv);
    }

    /* Set updated values in the buffer. */
    for (idx = 0; idx < field_count; idx ++) {
        soc_mem_field32_set(unit, mem, buf, fields[idx], values[idx]);
    }

    /* Write buffer back to memory. */
    rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL, index, buf);

    soc_mem_unlock(unit, mem);
    return (rv);
}

/*
 * Function:   soc_mem_field32_modify
 * Purpose:    Modify the value of a field in a memory entry.
 * Parameters:
 *       unit      - (IN) SOC unit number.
 *       mem       - (IN) Memory.
 *       index     - (IN) Memory entry index number.
 *       field     - (IN) Modified field.
 *       value     - (IN) New field value.
 * Returns:
 *       SOC_E_XXX
 */
int
soc_mem_field32_modify(int unit, soc_mem_t mem, int index,
                       soc_field_t field, uint32 value)
{
    return soc_mem_fields32_modify(unit, mem, index, 1, &field, &value);
}

#ifdef BCM_CMICM_SUPPORT
/*
 * Function:   soc_host_ccm_copy
 * Purpose:    Copy a range of memory from one host location to aother
 *                 Primarily for Cross-Coupled Memories.
 * Parameters:
 *       unit      - (IN) SOC unit number.
 *       srcbuf   - (IN) Source Buffer.
 *       dstbuf   - (IN) Destiation Buffer.
 *       count    - (IN) Number of dwords to transfer.
 *       flags    - (IN) 
 *                  0x01  Source and destination endian to be changed. 
 *                  0x02  Source is PCI
 * Returns:
 *       SOC_E_XXX
 */
int
soc_host_ccm_copy(int unit, void *srcbuf, void *dstbuf,
                  int count, int flags)
{
    int i, rv;
    soc_control_t *soc = SOC_CONTROL(unit);
    uint32 *srcptr = (uint32 *)srcbuf;
    uint32 *dstptr = (uint32 *)dstbuf;
    int cmc = SOC_PCI_CMC(unit);
    uint32 reg, reg2;
    assert (srcptr && dstptr);

    if (SOC_CONTROL(unit)->ccmDmaMutex == 0) {
        /* If DMA not enabled, or short length use PIO to copy */
        /*soc_cm_debug(DK_WARN, "using PIO mode for CCM copy\n");*/
        for (i = 0; i < count; i ++ ) {
            if (flags & 2) {            /* Read from PCI */
                reg = soc_pci_mcs_read(unit, PTR_TO_INT(srcptr));
            } else {
                reg = *srcptr;
            }
            
            if (flags & 1) {
                reg = ((reg & 0xff000000) >> 24) |
                    ((reg & 0x00ff0000) >> 8) |
                    ((reg & 0x0000ff00) << 8) |
                    ((reg & 0x000000ff) << 24);
            }

            if (flags & 2) {
                *dstptr = reg;
            } else {
                soc_pci_mcs_write(unit, PTR_TO_INT(dstptr), reg);
                reg2 = soc_pci_mcs_read(unit, PTR_TO_INT(dstptr));
                if (reg2 != reg) {
                    soc_cm_debug(DK_ERR, "ccm_dma: compare error %x (%x %x)\n",
                                 PTR_TO_INT(dstptr), reg, reg2);
                }
            }
            
            dstptr++;
            srcptr++;
        }
        return SOC_E_NONE;
    }

    CCM_DMA_LOCK(unit);
    
    soc_pci_write(unit, CMIC_CMCx_CCM_DMA_HOST0_MEM_START_ADDR_OFFSET(cmc), soc_cm_l2p(unit, srcbuf));
    soc_pci_write(unit, CMIC_CMCx_CCM_DMA_HOST1_MEM_START_ADDR_OFFSET(cmc), soc_cm_l2p(unit, dstbuf));
    soc_pci_write(unit, CMIC_CMCx_CCM_DMA_ENTRY_COUNT_OFFSET(cmc), count);
    /* Keep endianess default... */
    reg = soc_pci_read(unit, CMIC_CMCx_CCM_DMA_CFG_OFFSET(cmc));
    soc_reg_field_set(unit, CMIC_CMC0_CCM_DMA_CFGr, &reg, ABORTf, 0);
    soc_reg_field_set(unit, CMIC_CMC0_CCM_DMA_CFGr, &reg, ENf, 0);
    soc_pci_write(unit, CMIC_CMCx_CCM_DMA_CFG_OFFSET(cmc), reg);  /* Clearing EN clears stats */
    /* Start DMA */
    soc_reg_field_set(unit, CMIC_CMC0_CCM_DMA_CFGr, &reg, ENf, 1);
    soc_pci_write(unit, CMIC_CMCx_CCM_DMA_CFG_OFFSET(cmc), reg);

    rv = SOC_E_TIMEOUT;
    if (soc->ccmDmaIntrEnb) {
            soc_cmicm_intr0_enable(unit, IRQ_CMCx_CCMDMA_DONE);
            if (sal_sem_take(soc->ccmDmaIntr, soc->ccmDmaTimeout) < 0) {
                rv = SOC_E_TIMEOUT;
            }
            soc_cmicm_intr0_disable(unit, IRQ_CMCx_CCMDMA_DONE);

            reg = soc_pci_read(unit, CMIC_CMCx_CCM_DMA_STAT_OFFSET(cmc));
            if (soc_reg_field_get(unit, CMIC_CMC0_CCM_DMA_STATr,
                                                        reg, DONEf)) {
                rv = SOC_E_NONE;
                if (soc_reg_field_get(unit, CMIC_CMC0_CCM_DMA_STATr,
                                                        reg, ERRORf)) {
                    rv = SOC_E_FAIL;
                }
            }
    } else {
        soc_timeout_t to;
        soc_cm_debug(DK_WARN, "using Polling mode for CCM DMA\n");
        soc_timeout_init(&to, soc->ccmDmaTimeout, 10000);
        do {
            reg = soc_pci_read(unit, CMIC_CMCx_CCM_DMA_STAT_OFFSET(cmc));
            if (soc_reg_field_get(unit, CMIC_CMC0_CCM_DMA_STATr,
                                                    reg, DONEf)) {
                rv = SOC_E_NONE;
                if (soc_reg_field_get(unit, CMIC_CMC0_CCM_DMA_STATr,
                                                    reg, ERRORf)) {
                    rv = SOC_E_FAIL;
                }
                break;
            }
        } while(!(soc_timeout_check(&to)));
    }

    if (rv == SOC_E_TIMEOUT) {
        soc_cm_debug(DK_ERR, "ccm_dma: timeout\n");

        /* Abort CCM DMA */

        /* Dummy read to allow abort to finish */

        /* Disable CCM DMA */

        /* Clear ccm DMA abort bit */
    }

    CCM_DMA_UNLOCK(unit);
    return rv;
}
#endif

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */
