/*
 * $Id: testlist.c 1.73.4.3 Broadcom SDK $
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
 * File:        testlist.c
 * Purpose:     Defines list of tests and their associated
 *              functions.
 *
 * Notes:       Fields defined as follows:
 *
 *      Test # - Indicates the test number, all commands hat operate on tests 
 *              can use either the test name or #. The # is intended to be the
 *              DV Test number, but is arbitrary. If the test number is 
 *              negative, then the test is not selected by default; otherwise 
 *              it is.
 *      Flags - TSEL - Test is selected by default on supported chips.
 *              T56xx- Test is supported on the specified chip.
 *              TALL - Simple form to include all chips.
 *              TRC - rc.soc is to be loaded before running test.
 *      Test Name - The name of the test.
 *      Init Func - A routine called before the test is run to perform any 
 *              setup required, may be NULL if no initialization required.
 *      Cleanup Func - Function called after test has run to reset any state 
 *              etc, may be NULL if not required.
 *      Test Function - actual test function, called "Loop Count" times.
 *      Loop Count - Number of times to call "Test Function".
 *      Default Args - Default argument string passed into test.
 *
 * Order of a normal test execution is:
 *
 *      [1] Load TEST_RC file (if required)
 *      [2] Call "Init Func"
 *      [3] Call "Test Func" Loop times
 *      [4] Call "Cleanup Func"
 *
 * All extern declarations for functions are found in testlist.h.
 */

#include <appl/diag/system.h>
#include <appl/diag/test.h>

#include "testlist.h"

test_t test_list[] = { /*

Test #      "Test Name"                 Flags/Chips             Init Func               Cleanup Func            Test Func          Loop Cnt  Default Args
----------------------------------------------------------------------------------------------------------------------------------------------------------*/
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT)
TEST(1,     "Register reset defaults",  TALL+T88230,            0,                      0,                      rval_test,              1,      0)
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT */
#ifdef BCM_ESW_SUPPORT
TEST(2,     "PCI Compliance",           TSEL+TALL,              pci_test_init,          pci_test_done,          pci_test,               100,    0)
#endif /* BCM_ESW_SUPPORT */
#if defined (BCM_ESW_SUPPORT) || defined (BCM_SIRIUS_SUPPORT)
TEST(3,     "Register read/write",      TALL+T88230,            0,                      0,                      reg_test,               1,      0)
TEST(4,     "PCI S-Channel Buf",        TSEL+TALL+T88230,       0,                      0,                      pci_sch_test,           100,    0)
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT */
#ifdef BCM_ESW_SUPPORT
TEST(5,     "BIST",                     TSEL+T5690,             bist_test_init,         bist_test_done,         bist_test,              1,      "ALL")
TEST(17,    "CPU Loopback",             TRC+TSEL+TXGSSW-T56700, lb_dma_init,            lb_dma_done,            lb_dma_test,            1,      0)
TEST(18,    "MAC Loopback",             TRC+TALL,               lb_mac_init,            lb_mac_done,            lb_mac_test,            1,      0)
TEST(19,    "PHY Loopback",             TRC+TALL,               lb_mii_init,            lb_mii_done,            lb_mii_test,            1,      0)
TEST(20,    "EXT Loopback",             TRC+TALL,               lb_ext_init,            lb_ext_done,            lb_ext_test,            1,      0)
#endif /* BCM_ESW_SUPPORT */
TEST(21,    "CPU Benchmarks",           TALL+TROBO,                   benchmark_init,         benchmark_done,         benchmark_test,         1,      0)
#ifdef BCM_ESW_SUPPORT
TEST(22,    "CPU S/G, Reload",          TRC+TSEL+TALL, lb_reload_init,   lb_reload_done,         lb_reload_test,         1,      0)
TEST(23,    "CPU S/G, Simple",          TRC+TSEL+TALL,  lb_sg_dma_init,  lb_sg_dma_done,         lb_sg_dma_test,         1,      0)
TEST(24,    "CPU S/G, Random",          TRC+TSEL+TALL,  lb_random_init,  lb_random_done,         lb_random_test,         1,      0)
#endif /* BCM_ESW_SUPPORT */
TEST(30,    "Counter widths",           TRC+TSEL+TALL+T88230+TROBO,   0,                      0,                      ctr_test_width,         1,      0)
TEST(31,    "Counter read/write",       TRC+TSEL+TALL+T88230+TROBO,   0,                      0,                      ctr_test_rw,            1,      0)
#ifdef BCM_ESW_SUPPORT
#ifdef BCM_XGS_SWITCH_SUPPORT
TEST(32,    "XGS L2 Ins/Lookup/Del",    TSEL+TXGSSW,                   draco_l2_lu_test_init,  draco_l2_test_done,     draco_l2_test_lu,       1,      0)
TEST(33,    "XGS L2 Overflow Ins",      TSEL+TXGSSW,                   draco_l2_ov_test_init,  draco_l2_test_done,     draco_l2_test_ov,       1,      0)
TEST(34,    "XGS L2 Hashing",           TSEL+TXGSSW,                   draco_l2_hash_test_init,        draco_l2_test_done,     draco_l2_test_hash,     1,      0)
TEST(35,    "XGS L2 Delete by Port",    TSEL+TXGSSW,                   draco_l2_dp_test_init,  draco_l2_test_done,     draco_l2_test_dp,       1,      0)
TEST(36,    "XGS L2 Delete by VLAN",    TSEL+TXGSSW,                   draco_l2_dv_test_init,  draco_l2_test_done,     draco_l2_test_dv,       1,      0)
#ifdef INCLUDE_L3
TEST(37,    "XGS L3 Overflow Ins",      TSEL+TXGS12SW,             draco_l3_ov_test_init,  draco_l3_test_done,     draco_l3_test_ov,       1,      0)
TEST(38,    "XGS L3 Hashing",           TSEL+TXGS12SW,             draco_l3_hash_test_init,        draco_l3_test_done,     draco_l3_test_hash,     1,      0)
#endif /* INCLUDE_L3 */
#endif /* BCM_XGS_SWITCH_SUPPORT */
#endif /* BCM_ESW_SUPPORT */
TEST(39,    "New Snake Test",   TRC+TALL+TROBO,                lb2_snake_init,         lb2_done,               lb2_snake_test,         1,      0)
TEST(40,    "BCM Packet Send",          TALL+TROBO,                    tpacket_tx_init,        tpacket_tx_done,        tpacket_tx_test,        1,      0)
#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT) || defined(BCM_FE2000_SUPPORT) || defined(BCM_ROBO_SUPPORT)
TEST(41,    "BCM Packet Receive",       TRC+TXGSSW+TROBO+T88020, rpacket_init,  rpacket_done,           rpacket_test,           1,      0)
#endif /* BCM_FILTER_SUPPORT ||BCM_FIELD_SUPPORT || BCM_FE2000_SUPPORT ||  BCM_ROBO_SUPPORT */
#ifdef BCM_ESW_SUPPORT
TEST(48,    "MAC Loopback - Mark 2",    TRC+TALL,               lb2_mac_init,           lb2_done,               lb2_port_test,          1,      0)
#endif /* BCM_ESW_SUPPORT */
TEST(49,    "PHY Loopback - Mark 2",    TRC+TALL+TROBO,               lb2_phy_init,           lb2_done,               lb2_port_test,          1,      0)
#if defined BCM_ESW_SUPPORT || defined (BCM_SIRIUS_SUPPORT)
TEST(50,    "Memory Fill/Verify",       TALL+T88230,            mem_test_init,          mem_test_done,          mem_test,               1,      0)
TEST(51,    "Memory Random Addr/Data",  TALL+T88230,            mem_rand_init,          mem_rand_done,          mem_rand,               1,      0)
TEST(52,    "Rand Mem Addr, write all", TALL+T88230,            addr_rand_init,         addr_rand_done,         addr_rand,              1,      0)
#endif /* BCM_ESW_SUPPORT || BCM_SIRIUS_SUPPORT */
#ifdef BCM_DDR3_SUPPORT
TEST(53,   "DDR Memory Fill/Verify",    TRC+TALL,               ddr_test_init,           ddr_test_done,          ddr_test,           1,      0)
#endif
#ifdef BCM_ESW_SUPPORT
TEST(60,    "Linkscan MDIO",            TRC+TSEL+TXGSSW,     ls_test_init,           ls_test_done,           ls_test,                1,      0)
#ifdef BCM_FILTER_SUPPORT
TEST(70,    "FFP IRULE search",         TSEL+TXGS12SW,     ffp_test_init,          ffp_test_done,          ffp_test,               1,      0)
#endif /* BCM_FILTER_SUPPORT */
#endif /* BCM_ESW_SUPPORT */
#if defined (BCM_XGS_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
TEST(71,    "Table DMA",                TSEL+TXGSSW+T88230,            td_test_init,           td_test_done,           td_test_test,           1,      0)
#endif /* BCM_XGS_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */
TEST(72,    "Traffic Test",             TRC+TALL+TROBO,                traffic_test_init,      traffic_test_done,      traffic_test,           1,      0)
#ifdef BCM_ESW_SUPPORT
TEST(73,    "SNMP MIB Object Test",     TALL,                    snmp_test_init,         snmp_test_done,         snmp_test_test,         1,      0)
#ifdef BCM_FIREBOLT_SUPPORT
#ifdef INCLUDE_L3
TEST(85,    "L3 IPV6 Overflow Ins",     TSEL+TXGS3SW-T56218,      fb_l3ip6_ov_test_init,  fb_l3_test_done,        fb_l3_test_ov,  1,      0)
TEST(86,    "L3 IPV6 Hashing",          TSEL+TXGS3SW-T56218,      fb_l3ip6_hash_test_init,        fb_l3_test_done,        fb_l3_test_hash,        1,      0)
TEST(87,    "L3 IPV4 Overflow Ins",     TSEL+TXGS3SW-T56218,      fb_l3_ov_test_init,     fb_l3_test_done,        fb_l3_test_ov,  1,      0)
TEST(88,    "L3 IPV4 Hashing",          TSEL+TXGS3SW-T56218,      fb_l3_hash_test_init,   fb_l3_test_done,        fb_l3_test_hash,        1,      0)
#endif /* INCLUDE_L3 */
#endif
TEST(90,    "TX Reload Test",        TRC+TALL,                   pktspeed_test_init,   pktspeed_test_done,        pktspeed_test_tx,        1,      0)
TEST(91,    "RX Reload Test",        TRC+TALL,                   pktspeed_test_init,   pktspeed_test_done,        pktspeed_test_rx,        1,      0)
#ifdef BCM_TRX_SUPPORT
TEST(100,   "VLAN xlate Overflow Ins",  T56624 | T56820 | T56680,    tr_vlan_xlate_ov_test_init,   tr_vlan_xlate_test_done,  tr_vlan_xlate_test_ov,          1,      0)
TEST(101,   "VLAN xlate Hashing",       T56624 | T56820 | T56680,    tr_vlan_xlate_hash_test_init, tr_vlan_xlate_test_done,  tr_vlan_xlate_test_hash,        1,      0)
TEST(102,   "Egr VLAN xlate Ovrflw Ins",  T56624 | T56820 | T56680,    tr_egr_vlan_xlate_ov_test_init,   tr_egr_vlan_xlate_test_done,  tr_egr_vlan_xlate_test_ov,          1,      0)
TEST(103,   "Egr VLAN xlate Hashing",       T56624 | T56820 | T56680,    tr_egr_vlan_xlate_hash_test_init, tr_egr_vlan_xlate_test_done,  tr_egr_vlan_xlate_test_hash,        1,      0)
#ifdef BCM_TRIUMPH_SUPPORT
TEST(104,   "MPLS Overflow Ins",  T56624 | T56680,    tr_mpls_ov_test_init,   tr_mpls_test_done,  tr_mpls_test_ov,          1,      0)
TEST(105,   "MPLS Hashing",       T56624 | T56680,    tr_mpls_hash_test_init, tr_mpls_test_done,  tr_mpls_test_hash,        1,      0)
#endif /* BCM_TRIUMPH_SUPPORT */
#endif /* BCM_TRX_SUPPORT */
#endif  /* BCM_ESW_SUPPORT */
#ifdef BCM_SBX_SUPPORT
TEST(107,   "SBX BIST Test", TSEL+T88200+T88020+T88025,        sbx_bist_init,             sbx_test_done,                   sbx_bist_test,                        1,       0)
#if defined(BCM_FE2000_SUPPORT) 
TEST(108,   "SBX XE0 Loopback",TSEL+T88020,                   sbx_10g_loopback_init,      sbx_test_done,                   sbx_10G_XE0_loopback_test,       1,      "")
TEST(109,   "SBX XE1 Loopback",TSEL+T88020,                   sbx_10g_loopback_init,      sbx_test_done,                   sbx_10G_XE1_loopback_test,       1,      "")
TEST(118,   "SBX MAC Loopback",TSEL+T88020+T88025,            sbx_loopback_init,          sbx_test_done,                   sbx_1G_Int_loopback_test,        1,      "")
TEST(119,   "SBX PHY Loopback",TSEL+T88020+T88025,            sbx_loopback_init,          sbx_test_done,                   sbx_1G_Phy_loopback_test,        1,      "")
TEST(120,   "SBX EXT Loopback",TSEL+T88020+T88025,            sbx_loopback_init,          sbx_test_done,                   sbx_1G_Ext_loopback_test,        1,      "")
TEST(121,   "SBX Memory Fill/Verify",TSEL+T88020+T88025,      sbx_mem_init,               sbx_test_done,                   sbx_mem_test,                    1,      "")
#endif
#ifdef BCM_BM9600_SUPPORT
TEST(110,   "SBX Fabric PRBS Tests",T88130,                   sbx_prbs_init,              sbx_test_done,                   sbx_prbs_test,                   1,      "")
TEST(111,   "SBX Simple Register Test",T88130,                sbx_reg_init,               sbx_test_done,                   sbx_reg_test,                    1,      "")
#endif
#if defined BCM_BM9600_SUPPORT || defined BCM_SIRIUS_SUPPORT || defined BCM_QE2000_SUPPORT
TEST(113,   "SBX Unit PRBS test", T88130+T88230+T88200,       sbx_unit_prbs_init,         sbx_unit_prbs_test_done,         sbx_unit_prbs_test,              1,      0)
#endif
#ifdef BCM_QE2000_SUPPORT
TEST(112,   "SBX QE Traffic Test", TSEL+T88200,               sbx_qe_traffic_init,        sbx_qe_traffic_done,             sbx_qe_traffic_test,             1,      0)
#endif
#if defined(BCM_FE2000_SUPPORT)
TEST(114,   "SBX SPI PRBS Tests", TSEL+T88020+T88025,                sbx_prbs_init,              sbx_test_done,                   sbx_prbs_test,                   1,       0)
#ifdef BCM_QE2000_SUPPORT
TEST(115,   "SBX 1G FE<-->QE SNAKE Test", TSEL+T88020+T88025,        sbx_snake_init,             sbx_test_done,                   sbx_snake_test,                  1,       0)
#endif
TEST(116,   "SBX Line Rate Mem Test", TSEL+T88020+T88025,            sbx_linerate_mem_init,      sbx_test_done,                   sbx_linerate_mem_test,           1,       0)
#endif
#if defined(BCM_FE2000_SUPPORT)
TEST(117,   "SBX XE Loopback",T88025,                               sbx_10g_loopback_init,       sbx_test_done,                   sbx_10G_XE_loopback_test,        1,      "")
#endif
#endif /* BCM_SBX_SUPPORT */
#if defined (BCM_KATANA_SUPPORT) && defined(INCLUDE_CES)
TEST(118,  "CES Unstructured Test", T56440,                         ces_test_init,               ces_test_done,                   ces_test_unstruct, 1, 0)
TEST(119,  "CES Structured Test",   T56440,                         ces_test_init,               ces_test_done,                   ces_test_struct, 1, 0)
#endif
};

int test_cnt = COUNTOF(test_list);      /* # entries. */
