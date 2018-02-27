/*
 * $Id: devids.h 1.217.2.15 Broadcom SDK $
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

#ifndef _SOC_DEVIDS_H
#define _SOC_DEVIDS_H

/*
 * Defines PCI device and revision ID for every recognized device.
 * All driver routines refer to this ID only.
 */

#define BROADCOM_VENDOR_ID      0x14e4

/* Draco */
#define BCM5690_DEVICE_ID       0x5690
#define BCM5690_A0_REV_ID       1
#define BCM5690_A1_REV_ID       2
#define BCM5690_A2_REV_ID       3

/* Draco without HG (Medusa) */
#define BCM5691_DEVICE_ID       0x5691
#define BCM5691_A0_REV_ID       1
#define BCM5691_A1_REV_ID       2
#define BCM5691_A2_REV_ID       3

/* Draco without L3 */
#define BCM5692_DEVICE_ID       0x5692
#define BCM5692_A0_REV_ID       1
#define BCM5692_A1_REV_ID       2
#define BCM5692_A2_REV_ID       3

/* Draco without HG or L3 */
#define BCM5693_DEVICE_ID       0x5693
#define BCM5693_A0_REV_ID       1
#define BCM5693_A1_REV_ID       2
#define BCM5693_A2_REV_ID       3

/* Draco 1.5 */
#define BCM5695_DEVICE_ID       0x5695
#define BCM5695_A0_REV_ID       1
#define BCM5695_A1_REV_ID       2
#define BCM5695_B0_REV_ID       0x11

/* Draco 1.5 without HG */
#define BCM5696_DEVICE_ID       0x5696
#define BCM5696_A0_REV_ID       1
#define BCM5696_A1_REV_ID       2
#define BCM5696_B0_REV_ID       0x11

/* Draco 1.5 without L3 */
#define BCM5697_DEVICE_ID       0x5697
#define BCM5697_A0_REV_ID       1
#define BCM5697_A1_REV_ID       2
#define BCM5697_B0_REV_ID       0x11

/* Draco 1.5 without HG or L3 */
#define BCM5698_DEVICE_ID       0x5698
#define BCM5698_A0_REV_ID       1
#define BCM5698_A1_REV_ID       2
#define BCM5698_B0_REV_ID       0x11

/* Hercules with 8 ports */
#define BCM5670_DEVICE_ID       0x5670
#define BCM5670_A0_REV_ID       1
#define BCM5670_A1_REV_ID       2

/* Hercules with 4 ports */
#define BCM5671_DEVICE_ID       0x5671
#define BCM5671_A0_REV_ID       1
#define BCM5671_A1_REV_ID       2
#define BCM5671_A2_REV_ID       3       /* Maxxus */

/* Hercules 1.5 with 8 ports */
#define BCM5675_DEVICE_ID       0x5675
#define BCM5675_A0_REV_ID       1
#define BCM5675_A1_REV_ID       2

/* Hercules 1.5 with 4 ports */
#define BCM5676_DEVICE_ID       0x5676
#define BCM5676_A0_REV_ID       1
#define BCM5676_A1_REV_ID       2

/* Lynx */
#define BCM5673_DEVICE_ID       0x5673
#define BCM5673_A0_REV_ID       1
#define BCM5673_A1_REV_ID       2
#define BCM5673_A2_REV_ID       3

/* Lynx 1.5 */
#define BCM5674_DEVICE_ID       0x5674
#define BCM5674_A0_REV_ID       1

/* Felix */
#define BCM56100_DEVICE_ID      0xb100
#define BCM56100_A0_REV_ID      1
#define BCM56100_A1_REV_ID      2
#define BCM56101_DEVICE_ID      0xb101
#define BCM56101_A0_REV_ID      1
#define BCM56101_A1_REV_ID      2
#define BCM56102_DEVICE_ID      0xb102
#define BCM56102_A0_REV_ID      1
#define BCM56102_A1_REV_ID      2
#define BCM56105_DEVICE_ID      0xb105
#define BCM56105_A0_REV_ID      1
#define BCM56105_A1_REV_ID      2
#define BCM56106_DEVICE_ID      0xb106
#define BCM56106_A0_REV_ID      1
#define BCM56106_A1_REV_ID      2
#define BCM56107_DEVICE_ID      0xb107
#define BCM56107_A0_REV_ID      1
#define BCM56107_A1_REV_ID      2

/* Felix 1.5 */
#define BCM56110_DEVICE_ID      0xb110
#define BCM56110_A0_REV_ID      1
#define BCM56111_DEVICE_ID      0xb111
#define BCM56111_A0_REV_ID      1
#define BCM56112_DEVICE_ID      0xb112
#define BCM56112_A0_REV_ID      1
#define BCM56115_DEVICE_ID      0xb115
#define BCM56115_A0_REV_ID      1
#define BCM56116_DEVICE_ID      0xb116
#define BCM56116_A0_REV_ID      1
#define BCM56117_DEVICE_ID      0xb117
#define BCM56117_A0_REV_ID      1

/* Helix */
#define BCM56300_DEVICE_ID      0xb300
#define BCM56300_A0_REV_ID      1
#define BCM56300_A1_REV_ID      2
#define BCM56300_B0_REV_ID      0x11
#define BCM56300_B1_REV_ID      0x12
#define BCM56301_DEVICE_ID      0xb301
#define BCM56301_A0_REV_ID      1
#define BCM56301_A1_REV_ID      2
#define BCM56301_B0_REV_ID      0x11
#define BCM56301_B1_REV_ID      0x12
#define BCM56302_DEVICE_ID      0xb302
#define BCM56302_A0_REV_ID      1
#define BCM56302_A1_REV_ID      2
#define BCM56302_B0_REV_ID      0x11
#define BCM56302_B1_REV_ID      0x12
#define BCM56303_DEVICE_ID      0xb303
#define BCM56303_A1_REV_ID      2
#define BCM56303_A0_REV_ID      1
#define BCM56303_B0_REV_ID      0x11
#define BCM56303_B1_REV_ID      0x12
#define BCM56304_DEVICE_ID      0xb304
#define BCM56304_A0_REV_ID      1
#define BCM56304_A1_REV_ID      2
#define BCM56304_B0_REV_ID      0x11
#define BCM56304_B1_REV_ID      0x12
#define BCM56404_DEVICE_ID      0xb404
#define BCM56404_A0_REV_ID      1
#define BCM56404_A1_REV_ID      2
#define BCM56305_DEVICE_ID      0xb305
#define BCM56305_A0_REV_ID      1
#define BCM56305_A1_REV_ID      2
#define BCM56305_B0_REV_ID      0x11
#define BCM56305_B1_REV_ID      0x12
#define BCM56306_DEVICE_ID      0xb306
#define BCM56306_A0_REV_ID      1
#define BCM56306_A1_REV_ID      2
#define BCM56306_B0_REV_ID      0x11
#define BCM56306_B1_REV_ID      0x12
#define BCM56307_DEVICE_ID      0xb307
#define BCM56307_A0_REV_ID      1
#define BCM56307_A1_REV_ID      2
#define BCM56307_B0_REV_ID      0x11
#define BCM56307_B1_REV_ID      0x12
#define BCM56308_DEVICE_ID      0xb308
#define BCM56308_A0_REV_ID      1
#define BCM56308_A1_REV_ID      2
#define BCM56308_B0_REV_ID      0x11
#define BCM56308_B1_REV_ID      0x12
#define BCM56309_DEVICE_ID      0xb309
#define BCM56309_A0_REV_ID      1
#define BCM56309_A1_REV_ID      2
#define BCM56309_B0_REV_ID      0x11
#define BCM56309_B1_REV_ID      0x12

/* Helix 1.5 */
#define BCM56310_DEVICE_ID      0xb310
#define BCM56310_A0_REV_ID      1
#define BCM56311_DEVICE_ID      0xb311
#define BCM56311_A0_REV_ID      1
#define BCM56312_DEVICE_ID      0xb312
#define BCM56312_A0_REV_ID      1
#define BCM56313_DEVICE_ID      0xb313
#define BCM56313_A0_REV_ID      1
#define BCM56314_DEVICE_ID      0xb314
#define BCM56314_A0_REV_ID      1
#define BCM56315_DEVICE_ID      0xb315
#define BCM56315_A0_REV_ID      1
#define BCM56316_DEVICE_ID      0xb316
#define BCM56316_A0_REV_ID      1
#define BCM56317_DEVICE_ID      0xb317
#define BCM56317_A0_REV_ID      1
#define BCM56318_DEVICE_ID      0xb318
#define BCM56318_A0_REV_ID      1
#define BCM56319_DEVICE_ID      0xb319
#define BCM56319_A0_REV_ID      1

#ifndef EXCLUDE_BCM56324
/* Helix 2 */
#define BCM56322_DEVICE_ID      0xb322
#define BCM56322_A0_REV_ID      1
#define BCM56324_DEVICE_ID      0xb324
#define BCM56324_A0_REV_ID      1
#endif /* EXCLUDE_BCM56324 */

#define BCM53300_DEVICE_ID      0xb006
#define BCM53300_A0_REV_ID      0x11
#define BCM53300_A1_REV_ID      0x12
#define BCM53301_DEVICE_ID      0xb206
#define BCM53301_A0_REV_ID      0x11
#define BCM53301_A1_REV_ID      0x12
#define BCM53302_DEVICE_ID      0xb008
#define BCM53302_A0_REV_ID      0x11
#define BCM53302_A1_REV_ID      0x12

/* Firebolt */
#define BCM56500_DEVICE_ID      0xb500
#define BCM56500_A0_REV_ID      1
#define BCM56500_A1_REV_ID      2
#define BCM56500_B0_REV_ID      0x11
#define BCM56500_B1_REV_ID      0x12
#define BCM56500_B2_REV_ID      0x13
#define BCM56501_DEVICE_ID      0xb501
#define BCM56501_A0_REV_ID      1
#define BCM56501_A1_REV_ID      2
#define BCM56501_B0_REV_ID      0x11
#define BCM56501_B1_REV_ID      0x12
#define BCM56501_B2_REV_ID      0x13
#define BCM56502_DEVICE_ID      0xb502
#define BCM56502_A0_REV_ID      1
#define BCM56502_A1_REV_ID      2
#define BCM56502_B0_REV_ID      0x11
#define BCM56502_B1_REV_ID      0x12
#define BCM56502_B2_REV_ID      0x13
#define BCM56503_DEVICE_ID      0xb503
#define BCM56503_A0_REV_ID      1
#define BCM56503_A1_REV_ID      2
#define BCM56503_B0_REV_ID      0x11
#define BCM56503_B1_REV_ID      0x12
#define BCM56503_B2_REV_ID      0x13
#define BCM56504_DEVICE_ID      0xb504
#define BCM56504_A0_REV_ID      1
#define BCM56504_A1_REV_ID      2
#define BCM56504_B0_REV_ID      0x11
#define BCM56504_B1_REV_ID      0x12
#define BCM56504_B2_REV_ID      0x13
#define BCM56505_DEVICE_ID      0xb505
#define BCM56505_A0_REV_ID      1
#define BCM56505_A1_REV_ID      2
#define BCM56505_B0_REV_ID      0x11
#define BCM56505_B1_REV_ID      0x12
#define BCM56505_B2_REV_ID      0x13
#define BCM56506_DEVICE_ID      0xb506
#define BCM56506_A0_REV_ID      1
#define BCM56506_A1_REV_ID      2
#define BCM56506_B0_REV_ID      0x11
#define BCM56506_B1_REV_ID      0x12
#define BCM56506_B2_REV_ID      0x13
#define BCM56507_DEVICE_ID      0xb507
#define BCM56507_A0_REV_ID      1
#define BCM56507_A1_REV_ID      2
#define BCM56507_B0_REV_ID      0x11
#define BCM56507_B1_REV_ID      0x12
#define BCM56507_B2_REV_ID      0x13
#define BCM56508_DEVICE_ID      0xb508
#define BCM56508_A0_REV_ID      1
#define BCM56508_A1_REV_ID      2
#define BCM56508_B0_REV_ID      0x11
#define BCM56508_B1_REV_ID      0x12
#define BCM56508_B2_REV_ID      0x13
#define BCM56509_DEVICE_ID      0xb509
#define BCM56509_A0_REV_ID      1
#define BCM56509_A1_REV_ID      2
#define BCM56509_B0_REV_ID      0x11
#define BCM56509_B1_REV_ID      0x12
#define BCM56509_B2_REV_ID      0x13

/* Easyrider */
#define BCM56600_DEVICE_ID      0xb600
#define BCM56600_A0_REV_ID      1
#define BCM56600_B0_REV_ID      0x11
#define BCM56600_C0_REV_ID      0x21
#define BCM56601_DEVICE_ID      0xb601
#define BCM56601_A0_REV_ID      1
#define BCM56601_B0_REV_ID      0x11
#define BCM56601_C0_REV_ID      0x21
#define BCM56602_DEVICE_ID      0xb602
#define BCM56602_A0_REV_ID      1
#define BCM56602_B0_REV_ID      0x11
#define BCM56602_C0_REV_ID      0x21
#define BCM56603_DEVICE_ID      0xb603
#define BCM56603_A0_REV_ID      1
#define BCM56603_B0_REV_ID      0x11
#define BCM56603_C0_REV_ID      0x21
#define BCM56605_DEVICE_ID      0xb605
#define BCM56605_A0_REV_ID      1
#define BCM56605_B0_REV_ID      0x11
#define BCM56605_C0_REV_ID      0x21
#define BCM56606_DEVICE_ID      0xb606
#define BCM56606_A0_REV_ID      1
#define BCM56606_B0_REV_ID      0x11
#define BCM56606_C0_REV_ID      0x21
#define BCM56607_DEVICE_ID      0xb607
#define BCM56607_A0_REV_ID      1
#define BCM56607_B0_REV_ID      0x11
#define BCM56607_C0_REV_ID      0x21
#define BCM56608_DEVICE_ID      0xb608
#define BCM56608_A0_REV_ID      1
#define BCM56608_B0_REV_ID      0x11
#define BCM56608_C0_REV_ID      0x21

/* Goldwing */
#define BCM56580_DEVICE_ID      0xb580
#define BCM56580_A0_REV_ID      1

/* HUMV */
#define BCM56700_DEVICE_ID      0xb700
#define BCM56700_A0_REV_ID      1
#define BCM56701_DEVICE_ID      0xb701
#define BCM56701_A0_REV_ID      1

/* Bradley */
#define BCM56800_DEVICE_ID      0xb800
#define BCM56800_A0_REV_ID      1
#define BCM56801_DEVICE_ID      0xb801
#define BCM56801_A0_REV_ID      1
#define BCM56802_DEVICE_ID      0xb802
#define BCM56802_A0_REV_ID      1
#define BCM56803_DEVICE_ID      0xb803
#define BCM56803_A0_REV_ID      1

/* Raven */
#define BCM56224_DEVICE_ID      0xb224
#define BCM56224_A0_REV_ID      1
#define BCM56224_B0_REV_ID      0x11
#define BCM56225_DEVICE_ID      0xb225
#define BCM56225_A0_REV_ID      1
#define BCM56225_B0_REV_ID      0x11
#define BCM56226_DEVICE_ID      0xb226
#define BCM56226_A0_REV_ID      1
#define BCM56226_B0_REV_ID      0x11
#define BCM56227_DEVICE_ID      0xb227
#define BCM56227_A0_REV_ID      1
#define BCM56227_B0_REV_ID      0x11
#define BCM56228_DEVICE_ID      0xb228
#define BCM56228_A0_REV_ID      1
#define BCM56228_B0_REV_ID      0x11
#define BCM56229_DEVICE_ID      0xb229
#define BCM56229_A0_REV_ID      1
#define BCM56229_B0_REV_ID      0x11
#define BCM56024_DEVICE_ID      0xb024
#define BCM56024_A0_REV_ID      1
#define BCM56024_B0_REV_ID      0x11
#define BCM56025_DEVICE_ID      0xb025
#define BCM56025_A0_REV_ID      1
#define BCM56025_B0_REV_ID      0x11
#define BCM53724_DEVICE_ID      0xc724
#define BCM53724_A0_REV_ID      1
#define BCM53724_B0_REV_ID      0x11
#define BCM53726_DEVICE_ID      0xc726
#define BCM53726_A0_REV_ID      1
#define BCM53726_B0_REV_ID      0x11

/* Hawkeye */
#define BCM53312_DEVICE_ID      0xc312
#define BCM53312_A0_REV_ID      1
#define BCM53312_B0_REV_ID      0x11
#define BCM53313_DEVICE_ID      0xc313
#define BCM53313_A0_REV_ID      1
#define BCM53313_B0_REV_ID      0x11
#define BCM53314_DEVICE_ID      0xc314
#define BCM53314_A0_REV_ID      1
#define BCM53314_B0_REV_ID      0x11

/* Hawkeye EEE */
#define BCM53322_DEVICE_ID      0xc322
#define BCM53322_A0_REV_ID      1
#define BCM53323_DEVICE_ID      0xc323
#define BCM53323_A0_REV_ID      1
#define BCM53324_DEVICE_ID      0xc324
#define BCM53324_A0_REV_ID      1

/* VENDOR BROADCOM [ */
/* Stag's Leap */
#define BCM53610_DEVICE_ID      0xc610
#define BCM53610_A0_REV_ID      1
#define BCM53610_A1_REV_ID      2
/* VENDOR BROADCOM ] */

/* Raptor */
#define BCM56218_DEVICE_ID              0xB218
#define BCM56218_A0_REV_ID              1
#define BCM56218_A1_REV_ID              2
#define BCM56218_A2_REV_ID              3
#define BCM56218X_DEVICE_ID             0xc710
#define BCM56218X_A0_REV_ID             1
#define BCM56218X_A1_REV_ID             2
#define BCM56218X_A2_REV_ID             3
#define BCM56219_DEVICE_ID              0xB219
#define BCM56219_A0_REV_ID              1
#define BCM56219_A1_REV_ID              2
#define BCM56219_A2_REV_ID              3
#define BCM56218R_DEVICE_ID             0xB21A
#define BCM56218R_A0_REV_ID             1
#define BCM56218R_A1_REV_ID             2
#define BCM56218R_A2_REV_ID             3
#define BCM56219R_DEVICE_ID             0xB21B
#define BCM56219R_A0_REV_ID             1
#define BCM56219R_A1_REV_ID             2
#define BCM56219R_A2_REV_ID             3
#define BCM56214_DEVICE_ID              0xB214
#define BCM56214_A0_REV_ID              1
#define BCM56214_A1_REV_ID              2
#define BCM56214_A2_REV_ID              3
#define BCM56215_DEVICE_ID              0xB215
#define BCM56215_A0_REV_ID              1
#define BCM56215_A1_REV_ID              2
#define BCM56215_A2_REV_ID              3
#define BCM56214R_DEVICE_ID             0xB21C
#define BCM56214R_A0_REV_ID             1
#define BCM56214R_A1_REV_ID             2
#define BCM56214R_A2_REV_ID             3
#define BCM56215R_DEVICE_ID             0xB21D
#define BCM56215R_A0_REV_ID             1
#define BCM56215R_A1_REV_ID             2
#define BCM56215R_A2_REV_ID             3
#define BCM56216_DEVICE_ID              0xB216
#define BCM56216_A0_REV_ID              1
#define BCM56216_A1_REV_ID              2
#define BCM56216_A2_REV_ID              3
#define BCM56217_DEVICE_ID              0xB217
#define BCM56217_A0_REV_ID              1
#define BCM56217_A1_REV_ID              2
#define BCM56217_A2_REV_ID              3
#define BCM56212_DEVICE_ID              0xB212
#define BCM56212_A0_REV_ID              1
#define BCM56212_A1_REV_ID              2
#define BCM56212_A2_REV_ID              3
#define BCM56213_DEVICE_ID              0xB213
#define BCM56213_A0_REV_ID              1
#define BCM56213_A1_REV_ID              2
#define BCM56213_A2_REV_ID              3
#define BCM53718_DEVICE_ID              0xC71A
#define BCM53718_A0_REV_ID              1
#define BCM53718_A1_REV_ID              2
#define BCM53718_A2_REV_ID              3
#define BCM53714_DEVICE_ID              0xC71B
#define BCM53714_A0_REV_ID              1
#define BCM53714_A1_REV_ID              2
#define BCM53714_A2_REV_ID              3
#define BCM53716_DEVICE_ID              0xC716
#define BCM53716_A0_REV_ID              1
#define BCM53716_A1_REV_ID              2
#define BCM53716_A2_REV_ID              3
#define BCM56018_DEVICE_ID              0xB018
#define BCM56018_A0_REV_ID              1
#define BCM56018_A1_REV_ID              2
#define BCM56018_A2_REV_ID              3
#define BCM56014_DEVICE_ID              0xB014
#define BCM56014_A0_REV_ID              1
#define BCM56014_A1_REV_ID              2
#define BCM56014_A2_REV_ID              3

/* Firebolt2 */
#define BCM56510_DEVICE_ID      0xb510
#define BCM56510_A0_REV_ID      1
#define BCM56511_DEVICE_ID      0xb511
#define BCM56511_A0_REV_ID      1
#define BCM56512_DEVICE_ID      0xb512
#define BCM56512_A0_REV_ID      1
#define BCM56513_DEVICE_ID      0xb513
#define BCM56513_A0_REV_ID      1
#define BCM56514_DEVICE_ID      0xb514
#define BCM56514_A0_REV_ID      1
#define BCM56516_DEVICE_ID      0xb516
#define BCM56516_A0_REV_ID      1
#define BCM56517_DEVICE_ID      0xb517
#define BCM56517_A0_REV_ID      1
#define BCM56518_DEVICE_ID      0xb518
#define BCM56518_A0_REV_ID      1
#define BCM56519_DEVICE_ID      0xb519
#define BCM56519_A0_REV_ID      1

/* Triumph */
#define BCM56620_DEVICE_ID      0xb620
#define BCM56620_A0_REV_ID      1
#define BCM56620_A1_REV_ID      2
#define BCM56620_B0_REV_ID      0x11
#define BCM56620_B1_REV_ID      0x12
#define BCM56620_B2_REV_ID      0x13
#define BCM56624_DEVICE_ID      0xb624
#define BCM56624_A0_REV_ID      1
#define BCM56624_A1_REV_ID      2
#define BCM56624_B0_REV_ID      0x11
#define BCM56624_B1_REV_ID      0x12
#define BCM56624_B2_REV_ID      0x13
#define BCM56626_DEVICE_ID      0xb626
#define BCM56626_A0_REV_ID      1
#define BCM56626_A1_REV_ID      2
#define BCM56626_B0_REV_ID      0x11
#define BCM56626_B1_REV_ID      0x12
#define BCM56626_B2_REV_ID      0x13
#define BCM56628_DEVICE_ID      0xb628
#define BCM56628_A0_REV_ID      1
#define BCM56628_A1_REV_ID      2
#define BCM56628_B0_REV_ID      0x11
#define BCM56628_B1_REV_ID      0x12
#define BCM56628_B2_REV_ID      0x13
#define BCM56629_DEVICE_ID      0xb629
#define BCM56629_A0_REV_ID      1
#define BCM56629_A1_REV_ID      2
#define BCM56629_B0_REV_ID      0x11
#define BCM56629_B1_REV_ID      0x12
#define BCM56629_B2_REV_ID      0x13

/* Valkyrie */
#define BCM56680_DEVICE_ID      0xb680
#define BCM56680_A0_REV_ID      1
#define BCM56680_A1_REV_ID      2
#define BCM56680_B0_REV_ID      0x11
#define BCM56680_B1_REV_ID      0x12
#define BCM56680_B2_REV_ID      0x13
#define BCM56684_DEVICE_ID      0xb684
#define BCM56684_A0_REV_ID      1
#define BCM56684_A1_REV_ID      2
#define BCM56684_B0_REV_ID      0x11
#define BCM56684_B1_REV_ID      0x12
#define BCM56684_B2_REV_ID      0x13
#define BCM56686_DEVICE_ID      0xb686
#define BCM56686_B0_REV_ID      0x11
#define BCM56686_B1_REV_ID      0x12
#define BCM56686_B2_REV_ID      0x13

/* Scorpion */
#define BCM56820_DEVICE_ID      0xb820
#define BCM56820_A0_REV_ID      1
#define BCM56820_B0_REV_ID      0x11
#define BCM56821_DEVICE_ID      0xb821
#define BCM56821_A0_REV_ID      1
#define BCM56821_B0_REV_ID      0x11
#define BCM56822_DEVICE_ID      0xb822
#define BCM56822_A0_REV_ID      1
#define BCM56822_B0_REV_ID      0x11
#define BCM56823_DEVICE_ID      0xb823
#define BCM56823_A0_REV_ID      1
#define BCM56823_B0_REV_ID      0x11
#define BCM56825_DEVICE_ID      0xb825
#define BCM56825_B0_REV_ID      0x11

/* HUMV Plus */
#define BCM56720_DEVICE_ID      0xb720
#define BCM56720_A0_REV_ID      1
#define BCM56720_B0_REV_ID      0x11
#define BCM56721_DEVICE_ID      0xb721
#define BCM56721_A0_REV_ID      1
#define BCM56721_B0_REV_ID      0x11

/* Conqueror */
#define BCM56725_DEVICE_ID      0xb725
#define BCM56725_A0_REV_ID      1
#define BCM56725_B0_REV_ID      0x11

/* Triumph2 */
#define BCM56630_DEVICE_ID      0xb630
#define BCM56630_A0_REV_ID      1
#define BCM56630_B0_REV_ID      0x11
#define BCM56634_DEVICE_ID      0xb634
#define BCM56634_A0_REV_ID      1
#define BCM56634_B0_REV_ID      0x11
#define BCM56636_DEVICE_ID      0xb636
#define BCM56636_A0_REV_ID      1
#define BCM56636_B0_REV_ID      0x11
#define BCM56638_DEVICE_ID      0xb638
#define BCM56638_A0_REV_ID      1
#define BCM56638_B0_REV_ID      0x11
#define BCM56639_DEVICE_ID      0xb639
#define BCM56639_A0_REV_ID      1
#define BCM56639_B0_REV_ID      0x11

/* Valkyrie2 */
#define BCM56685_DEVICE_ID      0xb685
#define BCM56685_A0_REV_ID      1
#define BCM56685_B0_REV_ID      0x11
#define BCM56689_DEVICE_ID      0xb689
#define BCM56689_A0_REV_ID      1
#define BCM56689_B0_REV_ID      0x11

/* Apollo */
#define BCM56520_DEVICE_ID      0xb520
#define BCM56520_A0_REV_ID      1
#define BCM56520_B0_REV_ID      0x11
#define BCM56521_DEVICE_ID      0xb521
#define BCM56521_A0_REV_ID      1
#define BCM56521_B0_REV_ID      0x11
#define BCM56522_DEVICE_ID      0xb522
#define BCM56522_A0_REV_ID      1
#define BCM56522_B0_REV_ID      0x11
#define BCM56524_DEVICE_ID      0xb524
#define BCM56524_A0_REV_ID      1
#define BCM56524_B0_REV_ID      0x11
#define BCM56526_DEVICE_ID      0xb526
#define BCM56526_A0_REV_ID      1
#define BCM56526_B0_REV_ID      0x11

/* Firebolt 3 */
#define BCM56534_DEVICE_ID      0xb534
#define BCM56534_B0_REV_ID      0x11
#define BCM56538_DEVICE_ID      0xb538
#define BCM56538_B0_REV_ID      0x11

/* Enduro */
#define BCM56331_DEVICE_ID      0xb331
#define BCM56331_A0_REV_ID      1
#define BCM56331_B0_REV_ID      0x11
#define BCM56331_B1_REV_ID      0x12
#define BCM56333_DEVICE_ID      0xb333
#define BCM56333_A0_REV_ID      1
#define BCM56333_B0_REV_ID      0x11
#define BCM56333_B1_REV_ID      0x12
#define BCM56334_DEVICE_ID      0xb334
#define BCM56334_A0_REV_ID      1
#define BCM56334_B0_REV_ID      0x11
#define BCM56334_B1_REV_ID      0x12
#define BCM56338_DEVICE_ID      0xb338
#define BCM56338_A0_REV_ID      1
#define BCM56338_B0_REV_ID      0x11
#define BCM56338_B1_REV_ID      0x12

/* Helix 3 */
#define BCM56320_DEVICE_ID      0xb320
#define BCM56320_A0_REV_ID      1
#define BCM56320_B0_REV_ID      0x11
#define BCM56320_B1_REV_ID      0x12
#define BCM56321_DEVICE_ID      0xb321
#define BCM56321_A0_REV_ID      1
#define BCM56321_B0_REV_ID      0x11
#define BCM56321_B1_REV_ID      0x12

/* Stardust */
#define BCM56132_DEVICE_ID      0xb132
#define BCM56132_A0_REV_ID      1
#define BCM56132_B0_REV_ID      0x11
#define BCM56132_B1_REV_ID      0x12
#define BCM56134_DEVICE_ID      0xb134
#define BCM56134_A0_REV_ID      1
#define BCM56134_B0_REV_ID      0x11
#define BCM56134_B1_REV_ID      0x12

/* Hurricane */
#define BCM56140_DEVICE_ID      0xb140
#define BCM56140_A0_REV_ID      1
#define BCM56142_DEVICE_ID      0xb142
#define BCM56142_A0_REV_ID      1
#define BCM56143_DEVICE_ID      0xb143
#define BCM56143_A0_REV_ID      1
#define BCM56144_DEVICE_ID      0xb144
#define BCM56144_A0_REV_ID      1
#define BCM56146_DEVICE_ID      0xb146
#define BCM56146_A0_REV_ID      1
#define BCM56147_DEVICE_ID      0xb147
#define BCM56147_A0_REV_ID      1

/* Trident */
#define BCM56840_DEVICE_ID      0xb840
#define BCM56840_A0_REV_ID      1
#define BCM56840_A1_REV_ID      2
#define BCM56840_A2_REV_ID      3
#define BCM56840_A3_REV_ID      4
#define BCM56840_A4_REV_ID      5
#define BCM56840_B0_REV_ID      0x11
#define BCM56840_B1_REV_ID      0x12
#define BCM56841_DEVICE_ID      0xb841
#define BCM56841_A0_REV_ID      1
#define BCM56841_A1_REV_ID      2
#define BCM56841_A2_REV_ID      3
#define BCM56841_A3_REV_ID      4
#define BCM56841_A4_REV_ID      5
#define BCM56841_B0_REV_ID      0x11
#define BCM56841_B1_REV_ID      0x12
#define BCM56843_DEVICE_ID      0xb843
#define BCM56843_A0_REV_ID      1
#define BCM56843_A1_REV_ID      2
#define BCM56843_A2_REV_ID      3
#define BCM56843_A3_REV_ID      4
#define BCM56843_A4_REV_ID      5
#define BCM56843_B0_REV_ID      0x11
#define BCM56843_B1_REV_ID      0x12
#define BCM56845_DEVICE_ID      0xb845
#define BCM56845_A0_REV_ID      1
#define BCM56845_A1_REV_ID      2
#define BCM56845_A2_REV_ID      3
#define BCM56845_A3_REV_ID      4
#define BCM56845_A4_REV_ID      5
#define BCM56845_B0_REV_ID      0x11
#define BCM56845_B1_REV_ID      0x12

/* Titan */
#define BCM56743_DEVICE_ID      0xb743
#define BCM56743_A0_REV_ID      1
#define BCM56743_A1_REV_ID      2
#define BCM56743_A2_REV_ID      3
#define BCM56743_A3_REV_ID      4
#define BCM56743_A4_REV_ID      5
#define BCM56743_B0_REV_ID      0x11
#define BCM56743_B1_REV_ID      0x12
#define BCM56745_DEVICE_ID      0xb745
#define BCM56745_A0_REV_ID      1
#define BCM56745_A1_REV_ID      2
#define BCM56745_A2_REV_ID      3
#define BCM56745_A3_REV_ID      4
#define BCM56745_A4_REV_ID      5
#define BCM56745_B0_REV_ID      0x11
#define BCM56745_B1_REV_ID      0x12

/* Trident Plus */
#define BCM56842_DEVICE_ID      0xb842
#define BCM56842_A0_REV_ID      1
#define BCM56842_A1_REV_ID      2
#define BCM56844_DEVICE_ID      0xb844
#define BCM56844_A0_REV_ID      1
#define BCM56844_A1_REV_ID      2
#define BCM56846_DEVICE_ID      0xb846
#define BCM56846_A0_REV_ID      1
#define BCM56846_A1_REV_ID      2
#define BCM56549_DEVICE_ID      0xb549
#define BCM56549_A0_REV_ID      1
#define BCM56549_A1_REV_ID      2

/* Titan Plus */
#define BCM56744_DEVICE_ID      0xb744
#define BCM56744_A0_REV_ID      1
#define BCM56744_A1_REV_ID      2
#define BCM56746_DEVICE_ID      0xb746
#define BCM56746_A0_REV_ID      1
#define BCM56746_A1_REV_ID      2

/* Sirius */
#define BCM88230_DEVICE_ID      0x0230
#define BCM88230_A0_REV_ID      1
#define BCM88230_B0_REV_ID      0x11
#define BCM88230_C0_REV_ID      0x21
#define BCM88231_DEVICE_ID      0x0231
#define BCM88231_A0_REV_ID      1
#define BCM88231_B0_REV_ID      0x11
#define BCM88231_C0_REV_ID      0x21
#define BCM88235_DEVICE_ID      0x0235
#define BCM88235_A0_REV_ID      1
#define BCM88235_B0_REV_ID      0x11
#define BCM88235_C0_REV_ID      0x21
#define BCM88236_DEVICE_ID      0x0236
#define BCM88236_A0_REV_ID      1
#define BCM88236_B0_REV_ID      0x11
#define BCM88236_C0_REV_ID      0x21
#define BCM88239_DEVICE_ID      0x0239
#define BCM88239_A0_REV_ID      1
#define BCM88239_B0_REV_ID      0x11
#define BCM88239_C0_REV_ID      0x21
#define BCM56613_DEVICE_ID      0xb613
#define BCM56613_A0_REV_ID      1
#define BCM56613_B0_REV_ID      0x11
#define BCM56613_C0_REV_ID      0x21
#define BCM56930_DEVICE_ID      0xb930
#define BCM56930_A0_REV_ID      1
#define BCM56930_B0_REV_ID      0x11
#define BCM56930_C0_REV_ID      0x21
#define BCM56931_DEVICE_ID      0xb931
#define BCM56931_A0_REV_ID      1
#define BCM56931_B0_REV_ID      0x11
#define BCM56931_C0_REV_ID      0x21
#define BCM56935_DEVICE_ID      0xb935
#define BCM56935_A0_REV_ID      1
#define BCM56935_B0_REV_ID      0x11
#define BCM56935_C0_REV_ID      0x21
#define BCM56936_DEVICE_ID      0xb936
#define BCM56936_A0_REV_ID      1
#define BCM56936_B0_REV_ID      0x11
#define BCM56936_C0_REV_ID      0x21
#define BCM56939_DEVICE_ID      0xb939
#define BCM56939_A0_REV_ID      1
#define BCM56939_B0_REV_ID      0x11
#define BCM56939_C0_REV_ID      0x21

/* Shadow */
#define BCM88732_DEVICE_ID      0x0732
#define BCM88732_A0_REV_ID      1
#define BCM88732_A1_REV_ID      2
#define BCM88732_A2_REV_ID      4
#define BCM88732_B0_REV_ID      0x11
#define BCM88732_B1_REV_ID      0x12


/* Katana */
#define BCM56440_DEVICE_ID      0xb440
#define BCM56440_A0_REV_ID      1
#define BCM56441_DEVICE_ID      0xb441
#define BCM56441_A0_REV_ID      1
#define BCM56442_DEVICE_ID      0xb442
#define BCM56442_A0_REV_ID      1
#define BCM56443_DEVICE_ID      0xb443
#define BCM56443_A0_REV_ID      1
#define BCM56444_DEVICE_ID      0xb444
#define BCM56444_A0_REV_ID      1
#define BCM56445_DEVICE_ID      0xb445
#define BCM56445_A0_REV_ID      1
#define BCM56446_DEVICE_ID      0xb446
#define BCM56446_A0_REV_ID      1
#define BCM56447_DEVICE_ID      0xb447
#define BCM56447_A0_REV_ID      1
#define BCM56448_DEVICE_ID      0xb448
#define BCM56448_A0_REV_ID      1

/*
 * BCM5665: Tucana48 (48+4+1)
 * BCM5665L: Tucana24 (24+4+1)
 * BCM5666: Tucana48 (48+4+1) without L3
 * BCM5666L: Tucana24 (24+4+1) without L3
 *
 * The device ID is 0x5665 for all of these parts.  For BCM5665L and
 * BCM5666L, the pbmp_valid property must be set to invalidate fe24-fe47
 * (see $SDK/rc/config.bcm).
 */
#define BCM5665_DEVICE_ID       0x5665
#define BCM5665_A0_REV_ID       1
#define BCM5665_B0_REV_ID       0x11

/*
 * BCM5655: Titanium48 (48+4)
 * BCM5656: Titanium48 (48+4) without L3
 *
 * The device ID is 0x5655 for both parts.
 */
#define BCM5655_DEVICE_ID       0x5655
#define BCM5655_A0_REV_ID       1
#define BCM5655_B0_REV_ID       0x11

/*
 * BCM5650: Titanium-II (24+4)
 * BCM5651: Titanium-II (24+4) without L3
 *
 * The device ID is 0x5650 for both parts.
 * BCM5650C0 is the first spin of a real 24+4 cost-reduced chip.
 */
#define BCM5650_DEVICE_ID       0x5650
#define BCM5650_A0_REV_ID       1
#define BCM5650_B0_REV_ID       0x11
#define BCM5650_C0_REV_ID       0x21

#define BROADCOM_PHYID_HIGH 0x0040

/* robo devices */
#define BCM5338_PHYID_LOW       0x62b0
#define BCM5338_A0_REV_ID       0
#define BCM5338_A1_REV_ID       1
#define BCM5338_B0_REV_ID       3 

#define BCM5324_PHYID_LOW       0xbc20
#define BCM5324_PHYID_HIGH      0x143
#define BCM5324_A1_PHYID_HIGH   0x153
#define BCM5324_DEVICE_ID       0xbc20
#define BCM5324_A0_REV_ID       0
#define BCM5324_A1_REV_ID       1
#define BCM5324_A2_REV_ID       2

#define BCM5380_PHYID_LOW       0x6250
#define BCM5380_A0_REV_ID       0

#define BCM5388_PHYID_LOW       0x6288
#define BCM5388_A0_REV_ID       0

#define BCM5396_PHYID_LOW       0xbd70
#define BCM5396_PHYID_HIGH      0x143
#define BCM5396_DEVICE_ID       0x96
#define BCM5396_A0_REV_ID       0

#define BCM5389_PHYID_LOW       0xbd70
#define BCM5389_PHYID_HIGH      0x143
#define BCM5389_DEVICE_ID       0x89
#define BCM5389_A0_REV_ID       0
#define BCM5389_A1_DEVICE_ID    0x86
#define BCM5389_A1_REV_ID       1

#define BCM5398_PHYID_LOW       0xbcd0
#define BCM5398_PHYID_HIGH      0x0143
#define BCM5398_DEVICE_ID       0x98
#define BCM5398_A0_REV_ID       0

#define BCM5325_PHYID_LOW       0xbc30
#define BCM5325_PHYID_HIGH      0x143
#define BCM5325_DEVICE_ID       0xbc30
#define BCM5325_A0_REV_ID       0
#define BCM5325_A1_REV_ID       1

#define BCM5348_PHYID_LOW   0xbe40
#define BCM5348_PHYID_HIGH      0x0143
#define BCM5348_DEVICE_ID       0x48
#define BCM5348_A0_REV_ID       0
#define BCM5348_A1_REV_ID       1

#define BCM5397_PHYID_LOW       0xbcd0
#define BCM5397_PHYID_HIGH      0x0143
#define BCM5397_DEVICE_ID       0x97
#define BCM5397_A0_REV_ID       0

#define BCM5347_PHYID_LOW       0xbe40
#define BCM5347_PHYID_HIGH      0x0143
#define BCM5347_DEVICE_ID       0x47
#define BCM5347_A0_REV_ID       0

#define BCM5395_PHYID_LOW       0xbcf0
#define BCM5395_PHYID_HIGH      0x0143
#define BCM5395_DEVICE_ID       0xbcf0
#define BCM5395_A0_REV_ID       0

#define BCM53242_PHYID_LOW      0xbf10
#define BCM53242_PHYID_HIGH     0x0143 
#define BCM53242_DEVICE_ID      0xbf10 
#define BCM53242_A0_REV_ID  0
#define BCM53242_B0_REV_ID  4
#define BCM53242_B1_REV_ID  5

#define BCM53262_PHYID_LOW      0xbf20
#define BCM53262_PHYID_HIGH     0x0143 
#define BCM53262_DEVICE_ID      0xbf20 
#define BCM53262_A0_REV_ID  0
#define BCM53262_B0_REV_ID  4
#define BCM53262_B1_REV_ID  5

#define BCM53115_PHYID_LOW       0xbf80
#define BCM53115_PHYID_HIGH      0x0143
#define BCM53115_DEVICE_ID       0xbf80  
#define BCM53115_A0_REV_ID       0
#define BCM53115_A1_REV_ID       1
/* 53115_b0 is a exception on rev_id 
 *  - Normally, the B0 id definition should be 4
 */
#define BCM53115_B0_REV_ID       2
#define BCM53115_B1_REV_ID       3
#define BCM53115_C0_REV_ID       8

#define BCM53118_PHYID_LOW       0xbfe0
#define BCM53118_PHYID_HIGH      0x0143
#define BCM53118_DEVICE_ID       0xbfe0  
#define BCM53118_A0_REV_ID       0

#define BCM53118_B0_REV_ID       4
#define BCM53118_B1_REV_ID       5

#define BCM53280_PHYID_LOW       0x5e90
#define BCM53280_PHYID_HIGH      0x0362
#define BCM53280_DEVICE_ID       (0x4 | BCM53280_PHYID_LOW)
#define BCM53280_A0_REV_ID       0
#define BCM53280_B0_REV_ID	 0x4
#define BCM53280_B1_REV_ID	 0x5
#define BCM53280_B2_REV_ID	 0x6
#define BCM53286_DEVICE_ID       (0x4 | BCM53280_PHYID_LOW)
#define BCM53288_DEVICE_ID       (0xc | BCM53280_PHYID_LOW)
#define BCM53284_DEVICE_ID       (0x7 | BCM53280_PHYID_LOW)
#define BCM53283_DEVICE_ID       (0x6 | BCM53280_PHYID_LOW)
#define BCM53282_DEVICE_ID       (0x5 | BCM53280_PHYID_LOW)
#define BCM53101_PHYID_LOW       0x5ed0
#define BCM53101_PHYID_HIGH      0x0362
#define BCM53101_DEVICE_ID       0x5ed0
#define BCM53101_A0_REV_ID       0
#define BCM53101_B0_REV_ID       4

#define BCM53125_PHYID_LOW       0x5f20
#define BCM53125_PHYID_HIGH      0x0362
#define BCM53125_DEVICE_ID       0x5f20  
#define BCM53125_A0_REV_ID       0
#define BCM53125_B0_REV_ID       0x4
#define BCM53125_MODEL_ID       0x53125

#define BCM53128_PHYID_LOW       0x5e10
#define BCM53128_PHYID_HIGH      0x0362
#define BCM53128_DEVICE_ID       0x5e10  
#define BCM53128_A0_REV_ID       0
#define BCM53128_B0_REV_ID       0x4
#define BCM53128_MODEL_ID       0x53128

#define BCM53600_PHYID_LOW	0x5f40
#define BCM53600_PHYID_HIGH	0x0362
#define BCM53600_DEVICE_ID	(0x3 | BCM53600_PHYID_LOW)
#define BCM53600_A0_REV_ID	0
#define BCM53602_DEVICE_ID	(0x1 | BCM53600_PHYID_LOW)
#define BCM53603_DEVICE_ID	(0x2 | BCM53600_PHYID_LOW)
#define BCM53604_DEVICE_ID	(0x3 | BCM53600_PHYID_LOW)
#define BCM53606_DEVICE_ID	(0x7 | BCM53600_PHYID_LOW)

/* (out of band) ethernet devices */
#define BCM4713_DEVICE_ID       0x4713
#define BCM4713_A0_REV_ID       0
#define BCM4713_A9_REV_ID       9

/* (out of band) ethernet devices */
#define BCM53000_GMAC_DEVICE_ID       0x4715
#define BCM53000_A0_REV_ID       0

/* CPU devices */
#define BCM53000PCIE_DEVICE_ID  0x5300

/* Define SBX device IDs */
#define SANDBURST_VENDOR_ID     0x17ba
#define BME3200_DEVICE_ID       0x0280
#define BME3200_A0_REV_ID       0x0000
#define BME3200_B0_REV_ID       0x0001
#define BM9600_DEVICE_ID        0x0480
#define BM9600_A0_REV_ID        0x0000
#define BM9600_B0_REV_ID        0x0010
#define QE2000_DEVICE_ID        0x0300
#define QE2000_A1_REV_ID        0x0001
#define QE2000_A2_REV_ID        0x0002
#define QE2000_A3_REV_ID        0x0003
#define QE2000_A4_REV_ID        0x0004
#define BCM88020_DEVICE_ID      0x0380
#define BCM88020_A0_REV_ID      0x0000
#define BCM88020_A1_REV_ID      0x0001
#define BCM88020_A2_REV_ID      0x0002
#define BCM88025_DEVICE_ID      0x0580
#define BCM88025_A0_REV_ID      0x0000
#define BCM88130_DEVICE_ID      0x0480
#define BCM88130_A0_REV_ID      0x0000
#define BCM88130_A1_REV_ID      0x0001
#define BCM88130_B0_REV_ID      0x0010
#define PLX_VENDOR_ID           0x10b5
#define PLX9656_DEVICE_ID       0x9656
#define PLX9656_REV_ID          0x0000
#define PLX9056_DEVICE_ID       0x9056
#define PLX9056_REV_ID          0x0000

/* Define EA device IDs */
#define TK371X_DEVICE_ID 0x8600
#define TK371X_A0_REV_ID 0x0

#endif  /* !_SOC_DEVIDS_H */
