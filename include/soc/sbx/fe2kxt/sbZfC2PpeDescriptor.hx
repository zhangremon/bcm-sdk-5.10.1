/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfC2PpeDescriptor.hx 1.3.16.4 Broadcom SDK $
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


#ifndef SB_ZF_ZFC2PPEDESCRIPTOR_H
#define SB_ZF_ZFC2PPEDESCRIPTOR_H

#define SB_ZF_ZFC2PPEDESCRIPTOR_SIZE_IN_BYTES 192
#define SB_ZF_ZFC2PPEDESCRIPTOR_SIZE 192
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UCONTINUEBYTE_BITS "1535:1528"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE0_BITS "1527:1524"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE1_BITS "1523:1520"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE2_BITS "1519:1516"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE3_BITS "1515:1512"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE4_BITS "1511:1508"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE5_BITS "1507:1504"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE6_BITS "1503:1500"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE7_BITS "1499:1496"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE8_BITS "1495:1492"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UHEADERTYPE9_BITS "1491:1488"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION0_BITS "1487:1480"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION1_BITS "1479:1472"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION2_BITS "1471:1464"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION3_BITS "1463:1456"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION4_BITS "1455:1448"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION5_BITS "1447:1440"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION6_BITS "1439:1432"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION7_BITS "1431:1424"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION8_BITS "1423:1416"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_ULOCATION9_BITS "1415:1408"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_USTREAMSELECTOR_BITS "1407:1405"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UBUFFER_BITS "1404:1391"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UEXCEPTINDEX_BITS "1390:1384"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_USOURCEQUEUE_BITS "1383:1376"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_URXPORTDATA0_BITS "1375:1368"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UREPLICANTCNT_BITS "1367:1356"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_BREPLICANTBIT_BITS "1355:1355"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_URESV0_BITS "1354:1354"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UDROP_BITS "1353:1353"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UENQUEUE_BITS "1352:1352"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UDQUEUE_BITS "1351:1344"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_URXPORTDATA1_BITS "1343:1326"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UFRAMELENGTH_BITS "1325:1312"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UAGGRHASH_BITS "1311:1280"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UVARIABLE_BITS "1279:1248"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE0_BITS "1247:1240"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE1_BITS "1239:1232"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE2_BITS "1231:1224"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE3_BITS "1223:1216"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE4_BITS "1215:1208"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE5_BITS "1207:1200"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE6_BITS "1199:1192"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE7_BITS "1191:1184"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE8_BITS "1183:1176"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE9_BITS "1175:1168"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE10_BITS "1167:1160"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE11_BITS "1159:1152"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE12_BITS "1151:1144"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE13_BITS "1143:1136"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE14_BITS "1135:1128"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE15_BITS "1127:1120"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE16_BITS "1119:1112"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE17_BITS "1111:1104"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE18_BITS "1103:1096"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE19_BITS "1095:1088"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE20_BITS "1087:1080"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE21_BITS "1079:1072"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE22_BITS "1071:1064"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE23_BITS "1063:1056"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE24_BITS "1055:1048"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE25_BITS "1047:1040"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE26_BITS "1039:1032"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE27_BITS "1031:1024"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE28_BITS "1023:1016"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE29_BITS "1015:1008"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE30_BITS "1007:1000"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE31_BITS "999:992"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE32_BITS "991:984"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE33_BITS "983:976"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE34_BITS "975:968"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE35_BITS "967:960"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE36_BITS "959:952"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE37_BITS "951:944"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE38_BITS "943:936"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE39_BITS "935:928"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE40_BITS "927:920"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE41_BITS "919:912"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE42_BITS "911:904"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE43_BITS "903:896"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE44_BITS "895:888"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE45_BITS "887:880"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE46_BITS "879:872"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE47_BITS "871:864"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE48_BITS "863:856"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE49_BITS "855:848"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE50_BITS "847:840"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE51_BITS "839:832"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE52_BITS "831:824"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE53_BITS "823:816"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE54_BITS "815:808"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE55_BITS "807:800"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE56_BITS "799:792"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE57_BITS "791:784"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE58_BITS "783:776"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE59_BITS "775:768"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE60_BITS "767:760"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE61_BITS "759:752"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE62_BITS "751:744"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE63_BITS "743:736"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE64_BITS "735:728"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE65_BITS "727:720"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE66_BITS "719:712"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE67_BITS "711:704"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE68_BITS "703:696"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE69_BITS "695:688"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE70_BITS "687:680"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE71_BITS "679:672"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE72_BITS "671:664"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE73_BITS "663:656"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE74_BITS "655:648"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE75_BITS "647:640"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE76_BITS "639:632"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE77_BITS "631:624"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE78_BITS "623:616"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE79_BITS "615:608"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE80_BITS "607:600"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE81_BITS "599:592"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE82_BITS "591:584"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE83_BITS "583:576"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE84_BITS "575:568"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE85_BITS "567:560"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE86_BITS "559:552"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE87_BITS "551:544"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE88_BITS "543:536"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE89_BITS "535:528"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE90_BITS "527:520"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE91_BITS "519:512"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE92_BITS "511:504"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE93_BITS "503:496"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE94_BITS "495:488"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE95_BITS "487:480"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE96_BITS "479:472"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE97_BITS "471:464"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE98_BITS "463:456"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE99_BITS "455:448"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE100_BITS "447:440"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE101_BITS "439:432"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE102_BITS "431:424"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE103_BITS "423:416"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE104_BITS "415:408"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE105_BITS "407:400"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE106_BITS "399:392"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE107_BITS "391:384"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE108_BITS "383:376"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE109_BITS "375:368"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE110_BITS "367:360"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE111_BITS "359:352"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE112_BITS "351:344"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE113_BITS "343:336"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE114_BITS "335:328"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE115_BITS "327:320"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE116_BITS "319:312"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE117_BITS "311:304"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE118_BITS "303:296"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE119_BITS "295:288"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE120_BITS "287:280"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE121_BITS "279:272"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE122_BITS "271:264"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE123_BITS "263:256"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE124_BITS "255:248"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE125_BITS "247:240"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE126_BITS "239:232"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE127_BITS "231:224"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE128_BITS "223:216"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE129_BITS "215:208"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE130_BITS "207:200"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE131_BITS "199:192"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE132_BITS "191:184"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE133_BITS "183:176"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE134_BITS "175:168"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE135_BITS "167:160"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE136_BITS "159:152"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE137_BITS "151:144"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE138_BITS "143:136"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE139_BITS "135:128"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE140_BITS "127:120"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE141_BITS "119:112"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE142_BITS "111:104"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE143_BITS "103:96"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE144_BITS "95:88"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE145_BITS "87:80"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE146_BITS "79:72"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE147_BITS "71:64"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE148_BITS "63:56"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE149_BITS "55:48"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE150_BITS "47:40"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE151_BITS "39:32"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE152_BITS "31:24"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE153_BITS "23:16"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE154_BITS "15:8"
#define SB_ZF_ZFC2PPEDESCRIPTOR_M_UPACKETDATABYTE155_BITS "7:0"


typedef struct _sbZfC2PpeDescriptor {
  uint32_t m_uContinueByte;
  uint32_t m_uHeaderType0;
  uint32_t m_uHeaderType1;
  uint32_t m_uHeaderType2;
  uint32_t m_uHeaderType3;
  uint32_t m_uHeaderType4;
  uint32_t m_uHeaderType5;
  uint32_t m_uHeaderType6;
  uint32_t m_uHeaderType7;
  uint32_t m_uHeaderType8;
  uint32_t m_uHeaderType9;
  uint32_t m_uLocation0;
  uint32_t m_uLocation1;
  uint32_t m_uLocation2;
  uint32_t m_uLocation3;
  uint32_t m_uLocation4;
  uint32_t m_uLocation5;
  uint32_t m_uLocation6;
  uint32_t m_uLocation7;
  uint32_t m_uLocation8;
  uint32_t m_uLocation9;
  uint32_t m_uStreamSelector;
  uint32_t m_uBuffer;
  uint32_t m_uExceptIndex;
  uint32_t m_uSourceQueue;
  uint32_t m_uRxPortData0;
  uint32_t m_uReplicantCnt;
  uint32_t m_bReplicantBit;
  uint32_t m_uResv0;
  uint32_t m_uDrop;
  uint32_t m_uEnqueue;
  uint32_t m_uDqueue;
  uint32_t m_uRxPortData1;
  uint32_t m_uFrameLength;
  uint32_t m_uAggrHash;
  uint32_t m_uVariable;
  uint8_t m_uPacketDataByte0;
  uint8_t m_uPacketDataByte1;
  uint8_t m_uPacketDataByte2;
  uint8_t m_uPacketDataByte3;
  uint8_t m_uPacketDataByte4;
  uint8_t m_uPacketDataByte5;
  uint8_t m_uPacketDataByte6;
  uint8_t m_uPacketDataByte7;
  uint8_t m_uPacketDataByte8;
  uint8_t m_uPacketDataByte9;
  uint8_t m_uPacketDataByte10;
  uint8_t m_uPacketDataByte11;
  uint8_t m_uPacketDataByte12;
  uint8_t m_uPacketDataByte13;
  uint8_t m_uPacketDataByte14;
  uint8_t m_uPacketDataByte15;
  uint8_t m_uPacketDataByte16;
  uint8_t m_uPacketDataByte17;
  uint8_t m_uPacketDataByte18;
  uint8_t m_uPacketDataByte19;
  uint8_t m_uPacketDataByte20;
  uint8_t m_uPacketDataByte21;
  uint8_t m_uPacketDataByte22;
  uint8_t m_uPacketDataByte23;
  uint8_t m_uPacketDataByte24;
  uint8_t m_uPacketDataByte25;
  uint8_t m_uPacketDataByte26;
  uint8_t m_uPacketDataByte27;
  uint8_t m_uPacketDataByte28;
  uint8_t m_uPacketDataByte29;
  uint8_t m_uPacketDataByte30;
  uint8_t m_uPacketDataByte31;
  uint8_t m_uPacketDataByte32;
  uint8_t m_uPacketDataByte33;
  uint8_t m_uPacketDataByte34;
  uint8_t m_uPacketDataByte35;
  uint8_t m_uPacketDataByte36;
  uint8_t m_uPacketDataByte37;
  uint8_t m_uPacketDataByte38;
  uint8_t m_uPacketDataByte39;
  uint8_t m_uPacketDataByte40;
  uint8_t m_uPacketDataByte41;
  uint8_t m_uPacketDataByte42;
  uint8_t m_uPacketDataByte43;
  uint8_t m_uPacketDataByte44;
  uint8_t m_uPacketDataByte45;
  uint8_t m_uPacketDataByte46;
  uint8_t m_uPacketDataByte47;
  uint8_t m_uPacketDataByte48;
  uint8_t m_uPacketDataByte49;
  uint8_t m_uPacketDataByte50;
  uint8_t m_uPacketDataByte51;
  uint8_t m_uPacketDataByte52;
  uint8_t m_uPacketDataByte53;
  uint8_t m_uPacketDataByte54;
  uint8_t m_uPacketDataByte55;
  uint8_t m_uPacketDataByte56;
  uint8_t m_uPacketDataByte57;
  uint8_t m_uPacketDataByte58;
  uint8_t m_uPacketDataByte59;
  uint8_t m_uPacketDataByte60;
  uint8_t m_uPacketDataByte61;
  uint8_t m_uPacketDataByte62;
  uint8_t m_uPacketDataByte63;
  uint8_t m_uPacketDataByte64;
  uint8_t m_uPacketDataByte65;
  uint8_t m_uPacketDataByte66;
  uint8_t m_uPacketDataByte67;
  uint8_t m_uPacketDataByte68;
  uint8_t m_uPacketDataByte69;
  uint8_t m_uPacketDataByte70;
  uint8_t m_uPacketDataByte71;
  uint8_t m_uPacketDataByte72;
  uint8_t m_uPacketDataByte73;
  uint8_t m_uPacketDataByte74;
  uint8_t m_uPacketDataByte75;
  uint8_t m_uPacketDataByte76;
  uint8_t m_uPacketDataByte77;
  uint8_t m_uPacketDataByte78;
  uint8_t m_uPacketDataByte79;
  uint8_t m_uPacketDataByte80;
  uint8_t m_uPacketDataByte81;
  uint8_t m_uPacketDataByte82;
  uint8_t m_uPacketDataByte83;
  uint8_t m_uPacketDataByte84;
  uint8_t m_uPacketDataByte85;
  uint8_t m_uPacketDataByte86;
  uint8_t m_uPacketDataByte87;
  uint8_t m_uPacketDataByte88;
  uint8_t m_uPacketDataByte89;
  uint8_t m_uPacketDataByte90;
  uint8_t m_uPacketDataByte91;
  uint8_t m_uPacketDataByte92;
  uint8_t m_uPacketDataByte93;
  uint8_t m_uPacketDataByte94;
  uint8_t m_uPacketDataByte95;
  uint8_t m_uPacketDataByte96;
  uint8_t m_uPacketDataByte97;
  uint8_t m_uPacketDataByte98;
  uint8_t m_uPacketDataByte99;
  uint8_t m_uPacketDataByte100;
  uint8_t m_uPacketDataByte101;
  uint8_t m_uPacketDataByte102;
  uint8_t m_uPacketDataByte103;
  uint8_t m_uPacketDataByte104;
  uint8_t m_uPacketDataByte105;
  uint8_t m_uPacketDataByte106;
  uint8_t m_uPacketDataByte107;
  uint8_t m_uPacketDataByte108;
  uint8_t m_uPacketDataByte109;
  uint8_t m_uPacketDataByte110;
  uint8_t m_uPacketDataByte111;
  uint8_t m_uPacketDataByte112;
  uint8_t m_uPacketDataByte113;
  uint8_t m_uPacketDataByte114;
  uint8_t m_uPacketDataByte115;
  uint8_t m_uPacketDataByte116;
  uint8_t m_uPacketDataByte117;
  uint8_t m_uPacketDataByte118;
  uint8_t m_uPacketDataByte119;
  uint8_t m_uPacketDataByte120;
  uint8_t m_uPacketDataByte121;
  uint8_t m_uPacketDataByte122;
  uint8_t m_uPacketDataByte123;
  uint8_t m_uPacketDataByte124;
  uint8_t m_uPacketDataByte125;
  uint8_t m_uPacketDataByte126;
  uint8_t m_uPacketDataByte127;
  uint8_t m_uPacketDataByte128;
  uint8_t m_uPacketDataByte129;
  uint8_t m_uPacketDataByte130;
  uint8_t m_uPacketDataByte131;
  uint8_t m_uPacketDataByte132;
  uint8_t m_uPacketDataByte133;
  uint8_t m_uPacketDataByte134;
  uint8_t m_uPacketDataByte135;
  uint8_t m_uPacketDataByte136;
  uint8_t m_uPacketDataByte137;
  uint8_t m_uPacketDataByte138;
  uint8_t m_uPacketDataByte139;
  uint8_t m_uPacketDataByte140;
  uint8_t m_uPacketDataByte141;
  uint8_t m_uPacketDataByte142;
  uint8_t m_uPacketDataByte143;
  uint8_t m_uPacketDataByte144;
  uint8_t m_uPacketDataByte145;
  uint8_t m_uPacketDataByte146;
  uint8_t m_uPacketDataByte147;
  uint8_t m_uPacketDataByte148;
  uint8_t m_uPacketDataByte149;
  uint8_t m_uPacketDataByte150;
  uint8_t m_uPacketDataByte151;
  uint8_t m_uPacketDataByte152;
  uint8_t m_uPacketDataByte153;
  uint8_t m_uPacketDataByte154;
  uint8_t m_uPacketDataByte155;
} sbZfC2PpeDescriptor_t;

uint32_t
sbZfC2PpeDescriptor_Pack(sbZfC2PpeDescriptor_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex);
void
sbZfC2PpeDescriptor_Unpack(sbZfC2PpeDescriptor_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex);
void
sbZfC2PpeDescriptor_InitInstance(sbZfC2PpeDescriptor_t *pFrame);

#define SB_ZF_C2PPEDESCRIPTOR_SET_CONTBYTE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[191] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[190] = ((pToData)[190] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[190] = ((pToData)[190] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[189] = ((pToData)[189] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[189] = ((pToData)[189] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[188] = ((pToData)[188] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[188] = ((pToData)[188] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[187] = ((pToData)[187] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[187] = ((pToData)[187] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[186] = ((pToData)[186] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_HDR9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[186] = ((pToData)[186] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[185] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[184] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[183] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[182] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[181] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[180] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[179] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[178] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[177] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_LOCN9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[176] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_STR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[175] = ((pToData)[175] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_BUFFER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[173] = ((pToData)[173] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[174] = ((pToData)[174] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[175] = ((pToData)[175] & ~ 0x1f) | (((nFromData) >> 9) & 0x1f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_EXPIDX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[173] = ((pToData)[173] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_SQUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[172] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_RXPORT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[171] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_REPCNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[169] = ((pToData)[169] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[170] = ((pToData)[170] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_REPBIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[169] = ((pToData)[169] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[169] = ((pToData)[169] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_DROP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[169] = ((pToData)[169] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_ENQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[169] = ((pToData)[169] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_DQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[168] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_RXPORT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[165] = ((pToData)[165] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[166] = ((pToData)[166] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[167] = ((pToData)[167] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_FRMLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[164] = ((nFromData)) & 0xFF; \
           (pToData)[165] = ((pToData)[165] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_AGGRHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[160] = ((nFromData)) & 0xFF; \
           (pToData)[161] = ((pToData)[161] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[162] = ((pToData)[162] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[163] = ((pToData)[163] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_VARIABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[156] = ((nFromData)) & 0xFF; \
           (pToData)[157] = ((pToData)[157] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[158] = ((pToData)[158] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[159] = ((pToData)[159] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[155] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[154] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[153] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[152] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[151] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[150] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[149] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[148] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[147] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[146] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[145] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[144] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[143] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[142] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[141] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[140] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[139] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[138] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[137] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[136] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[135] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[134] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[133] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[132] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[131] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA25(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[130] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA26(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[129] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA27(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[128] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA28(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[127] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA29(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[126] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA30(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[125] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA31(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[124] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA32(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[123] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA33(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[122] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA34(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[121] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA35(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[120] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA36(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[119] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA37(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[118] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA38(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[117] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA39(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[116] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA40(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[115] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA41(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[114] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA42(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[113] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA43(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[112] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA44(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[111] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA45(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[110] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA46(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[109] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA47(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[108] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA48(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[107] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA49(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[106] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA50(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[105] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA51(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[104] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA52(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[103] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA53(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[102] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA54(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[101] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA55(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[100] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA56(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[99] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA57(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[98] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA58(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[97] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA59(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[96] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA60(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[95] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA61(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[94] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA62(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[93] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA63(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[92] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA64(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[91] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA65(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[90] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA66(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[89] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA67(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[88] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA68(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[87] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA69(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[86] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA70(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[85] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA71(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[84] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA72(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[83] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA73(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[82] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA74(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[81] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA75(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[80] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA76(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[79] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA77(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[78] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA78(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[77] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA79(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[76] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA80(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[75] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA81(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[74] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA82(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[73] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA83(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[72] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA84(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[71] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA85(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[70] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA86(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[69] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA87(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[68] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA88(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[67] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA89(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[66] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA90(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[65] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA91(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[64] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA92(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[63] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA93(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[62] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA94(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[61] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA95(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[60] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA96(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[59] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA97(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[58] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA98(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[57] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA99(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[56] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA100(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[55] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA101(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[54] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA102(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[53] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA103(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[52] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA104(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[51] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA105(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[50] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA106(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[49] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA107(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[48] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA108(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[47] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA109(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[46] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA110(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[45] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA111(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[44] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA112(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[43] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA113(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[42] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA114(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[41] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA115(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[40] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA116(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[39] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA117(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[38] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA118(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[37] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA119(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[36] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA120(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[35] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA121(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[34] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA122(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[33] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA123(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[32] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA124(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA125(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[30] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA126(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[29] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA127(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA128(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA129(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA130(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA131(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA132(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA133(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA134(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA135(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA136(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA137(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA138(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA139(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA140(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA141(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA142(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA143(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA144(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA145(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA146(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA147(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA148(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA149(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA150(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA151(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA152(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA153(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA154(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_SET_PDATA155(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_CONTBYTE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[191] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[190] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[190]) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[189] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[189]) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[188] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[188]) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[187] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[187]) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[186] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_HDR9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[186]) & 0x0f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[185] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[184] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[183] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[182] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[181] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[180] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[179] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[178] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[177] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_LOCN9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[176] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_STR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[175] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_BUFFER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[173] >> 7) & 0x01; \
           (nToData) |= (uint32_t) (pFromData)[174] << 1; \
           (nToData) |= (uint32_t) ((pFromData)[175] & 0x1f) << 9; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_EXPIDX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[173]) & 0x7f; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_SQUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[172] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_RXPORT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[171] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_REPCNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[169] >> 4) & 0x0f; \
           (nToData) |= (uint32_t) (pFromData)[170] << 4; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_REPBIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[169] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[169] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_DROP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[169] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_ENQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[169]) & 0x01; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_DQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[168] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_RXPORT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) ((pFromData)[165] >> 6) & 0x03; \
           (nToData) |= (uint32_t) (pFromData)[166] << 2; \
           (nToData) |= (uint32_t) (pFromData)[167] << 10; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_FRMLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[164] ; \
           (nToData) |= (uint32_t) ((pFromData)[165] & 0x3f) << 8; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_AGGRHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[160] ; \
           (nToData) |= (uint32_t) (pFromData)[161] << 8; \
           (nToData) |= (uint32_t) (pFromData)[162] << 16; \
           (nToData) |= (uint32_t) (pFromData)[163] << 24; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_VARIABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32_t) (pFromData)[156] ; \
           (nToData) |= (uint32_t) (pFromData)[157] << 8; \
           (nToData) |= (uint32_t) (pFromData)[158] << 16; \
           (nToData) |= (uint32_t) (pFromData)[159] << 24; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[155] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[154] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[153] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[152] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[151] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[150] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[149] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[148] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[147] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[146] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[145] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[144] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[143] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[142] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[141] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[140] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[139] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[138] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[137] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[136] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[135] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[134] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[133] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[132] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[131] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA25(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[130] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA26(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[129] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA27(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[128] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA28(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[127] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA29(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[126] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA30(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[125] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA31(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[124] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA32(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[123] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA33(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[122] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA34(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[121] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA35(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[120] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA36(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[119] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA37(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[118] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA38(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[117] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA39(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[116] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA40(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[115] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA41(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[114] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA42(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[113] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA43(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[112] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA44(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[111] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA45(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[110] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA46(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[109] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA47(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[108] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA48(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[107] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA49(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[106] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA50(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[105] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA51(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[104] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA52(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[103] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA53(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[102] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA54(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[101] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA55(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[100] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA56(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[99] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA57(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[98] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA58(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[97] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA59(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[96] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA60(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[95] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA61(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[94] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA62(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[93] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA63(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[92] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA64(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[91] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA65(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[90] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA66(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[89] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA67(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[88] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA68(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[87] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA69(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[86] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA70(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[85] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA71(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[84] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA72(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[83] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA73(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[82] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA74(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[81] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA75(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[80] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA76(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[79] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA77(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[78] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA78(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[77] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA79(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[76] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA80(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[75] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA81(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[74] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA82(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[73] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA83(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[72] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA84(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[71] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA85(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[70] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA86(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[69] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA87(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[68] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA88(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[67] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA89(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[66] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA90(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[65] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA91(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[64] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA92(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[63] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA93(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[62] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA94(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[61] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA95(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[60] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA96(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[59] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA97(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[58] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA98(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[57] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA99(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[56] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA100(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[55] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA101(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[54] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA102(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[53] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA103(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[52] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA104(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[51] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA105(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[50] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA106(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[49] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA107(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[48] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA108(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[47] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA109(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[46] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA110(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[45] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA111(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[44] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA112(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[43] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA113(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[42] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA114(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[41] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA115(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[40] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA116(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[39] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA117(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[38] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA118(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[37] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA119(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[36] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA120(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[35] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA121(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[34] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA122(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[33] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA123(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[32] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA124(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[31] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA125(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[30] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA126(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[29] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA127(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[28] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA128(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[27] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA129(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[26] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA130(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[25] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA131(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[24] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA132(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[23] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA133(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[22] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA134(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[21] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA135(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[20] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA136(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[19] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA137(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[18] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA138(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[17] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA139(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[16] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA140(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[15] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA141(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[14] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA142(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[13] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA143(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[12] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA144(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[11] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA145(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[10] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA146(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[9] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA147(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[8] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA148(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[7] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA149(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[6] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA150(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[5] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA151(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA152(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA153(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA154(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_C2PPEDESCRIPTOR_GET_PDATA155(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8_t) (pFromData)[0] ; \
          } while(0)

#endif
