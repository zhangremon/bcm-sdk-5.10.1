/*
 * $Id: sample.c 1.1.54.1 Broadcom SDK $
 *
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
 */

int rc;
int i;
int unit = 1;
int vftbase = 0;
int femod = 0;
int native_vid = 17;
int pci = 31;
int pcisid = 42;

rc = soc_sbx_g2p3_vlan_ft_base_get(unit, &vftbase);

rc = soc_sbx_g2p3_age_set(unit, 0);

rc = soc_sbx_g2p3_node_set(unit, femod);

soc_sbx_g2p3_p2e_t p2e;
soc_sbx_g2p3_p2e_t_init(p2e);

p2e.nativevid = native_vid;
p2e.provider = 1;
rc = soc_sbx_g2p3_p2e_set(unit, pci, p2e);

/* clear QOS mapping 0 (default) */
soc_sbx_g2p3_qos_t qos;
soc_sbx_g2p3_qos_t_init(qos);

for (i=0; i < 16; i++) {
  rc = soc_sbx_g2p3_qos_set(unit, (i % 2), (i / 2), 0, qos);
}

rc = soc_sbx_g2p3_mc_ft_offset_set(unit, 0);
   
soc_sbx_g2p3_lp_t lp;
soc_sbx_g2p3_lp_t_init(lp);

lp.pid = pcisid;
rc = soc_sbx_g2p3_lp_set(unit, pci, lp);

print rc;

cint_reset();


/* the end */

/* 
# Untagged packet, miss
proc AT_g2p3i_001 {chip unit args} {
    set p [i_bridge_pkt]
    set ip $p
    set ep [i_erh_pkt $p $::native_vlan $::native_qid 1 ]
    return [g2p3_unit_txrx $ip $ep 0 "untagged miss"]
}

proc g2p3_unit_txrx {txp rxp {egress 0} {rxpn "g2p3 unit test packet"} } {
    set rxunits [list $::fe]
    if {$egress == 2} {
        bcm shell "1:nicconfig free"
        lappend rxunits ethernet
    } elseif {$egress == 1} {
        bcm shell "1:nicconfig egress"
        lappend rxunits ethernet
    } else {
        bcm shell "1:nicconfig ingress"
        if {$::sirius} {
            lappend rxunits gu2irh_sirius
        } elseif {$::qess} {
            lappend rxunits gu2erh_qess
        } else {
            lappend rxunits sbx
        }
    }
    set txs [list $::fe $txp]
    set rxs {}
    if {[llength $rxp] > 0} {
        lappend rxs $::fe $rxpn $rxp 0
    }
    if {![sbx_pkt_test $txs $rxs $rxunits]} {
        return FAIL
    }
    return PASS
}

*/
