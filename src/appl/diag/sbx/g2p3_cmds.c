/*
 * $Id: g2p3_cmds.c 1.37 Broadcom SDK $
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
 * File:        gu2_set_get.c
 * Purpose:     sbx commands for ucode tables
 * Requires:
 *
 */

#include <sal/core/libc.h>
#include <sal/appl/sal.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#ifdef BCM_FE2000_SUPPORT

#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g2p3/g2p3.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/g2p3/g2p3_ace.h>

#include <bcm/mcast.h>
#include <bcm/multicast.h>
#include <bcm/vlan.h>

cmd_result_t
cmd_sbx_g2p3_get (int unit, args_t *args)
{
    int rv;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2P3) {
        printk("ERROR: not running gu2 v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_g2p3_shell_print(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_g2p3_set (int unit, args_t *args)
{
    int rv;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2P3) {
        printk("ERROR: not running gu2 v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_g2p3_shell_set(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_g2p3_delete (int unit, args_t *args)
{
    int rv;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2P3) {
        printk("ERROR: not running gu2 v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_g2p3_shell_delete(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_g2p3_svid_cap(int unit, args_t *args)
{
  uint ovid, ivid, vlan, i, added;
  soc_sbx_g2p3_pvv2e_t p;
  sbStatus_t s;
  sal_usecs_t start;

#if 0
  ovid = ivid = 1;
  vlan = 0;
  added = 0;

  soc_sbx_g2p3_pvv2e_t_init (&p);
  start = sal_time_usecs();
  do {
    ivid = 1;
    do {
      vlan++;
      if (vlan > 0xfff)
        vlan = 1;

      s = soc_sbx_g2p3_pvv2e_add (unit, ivid, ovid, 0x1f, &p);
      if (s != SB_OK) {
        printk ("%d\n", added);
        goto rem_entries_1;
      }
      added++;
      ivid++; 
    } while (ivid < 0xfff);
    ovid++;
  } while (ovid < 0xfff);

rem_entries_1:
  printk ("adding %d entries using soc_sbx_g2p3_pvv2e_add took %dus\n", added, SAL_USECS_SUB(sal_time_usecs(), start));

  printk ("Removing entries ...\n");
  ovid = ivid = 1;

  for (i = 0; i < added; i++) {
    s = soc_sbx_g2p3_pvv2e_remove (unit, ivid, ovid, 0x1f);
    ivid++;
    if (ivid >= 0xfff) {
      ivid = 1;
      ovid++;
    }
  }
#endif

  ovid = ivid = 1;
  vlan = 0;
  added = 0;
  start = sal_time_usecs();
  do {
    ivid = 1;
    do {
      vlan++;
      if (vlan > 0xfff)
        vlan = 1;

      s = soc_sbx_g2p3_pvv2e_set (unit, ivid, ovid, 0x1f, &p);
      if (s != SB_OK) {
        printk ("%d\n", added);
        goto rem_entries_2;
      }
      added++;
      ivid++; 
    } while (ivid < 0xfff);
    ovid++;
  } while (ovid < 0xfff);

rem_entries_2:
  printk ("adding %d entries using soc_sbx_g2p3_pvv2e_set took %dus\n", added, SAL_USECS_SUB(sal_time_usecs(), start));

  printk ("Removing entries ...\n");
  ovid = ivid = 1;

  for (i = 0; i < added; i++) {
    s = soc_sbx_g2p3_pvv2e_remove (unit, ivid, ovid, 0x1f);
    ivid++;
    if (ivid >= 0xfff) {
      ivid = 1;
      ovid++;
    }
  }

  return CMD_OK;
}

#define TEST_ACL_ADD_DELETE_SPEED 0
#if TEST_ACL_ADD_DELETE_SPEED
int bcm_acl_dma_add_thread(int feUnit, bcm_field_group_t group, uint32_t entryStart, uint32_t entryEnd);
int bcm_acl_dma_del_thread(int feUnit, bcm_field_group_t gid, uint32_t entryStart, uint32_t entryEnd);
void acl_test_loop();
uint32_t gTick = 5;
uint32_t entryCur = 0;
uint32_t ifShutdown = 0;
static int acl_test_count = 0;
sal_thread_t acl_test_thread_id;

/* 
  sp bcm_acl_dma_add_thread,0,0,62,7061
 */
int bcm_acl_dma_add_thread(int feUnit, bcm_field_group_t group, uint32_t entryStart, uint32_t entryEnd)
{
    int rv = BCM_E_NONE;
    int prio = group;
    uint32_t entryCnt = 0;
    bcm_field_qset_t  qset;

    /* Attach the ACL rules to the ingress. */
    BCM_FIELD_QSET_INIT(qset);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageIngressQoS);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOuterVlan);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifySrcMac);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstMac);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifySrcIp);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDSCP);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyRangeCheck);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyTcpControl);

    /* Create ACL group, priority=0. */
    rv = bcm_field_group_create_id(feUnit, qset, prio, group);
    if ((BCM_E_NONE != rv) && (BCM_E_EXISTS != rv))
    {
        printf("<%s>: Line=%d, bcm_field_group_create_id error, rv=%d.\n",
                FUNCTION_NAME(), __LINE__, rv);
        return BCM_E_FAIL;
    }

    for (entryCnt = entryStart; entryCnt < entryEnd; entryCnt++)
    {
        bcm_mac_t   macData;
        bcm_mac_t   macMask;
        bcm_pbmp_t  portData;
        bcm_pbmp_t  portMask;

        memset(&macData, 0, sizeof(bcm_pbmp_t));
        memset(&macMask, 0, sizeof(bcm_pbmp_t));

        macData[0] = 0x0;
        macData[1] = 0x0;
        macData[2] = ((entryCnt>>8) & 0xff);
        macData[3] = ((entryCnt>>4) & 0xff);
        macData[4] = (entryCnt & 0xff);
        macData[5] = 0x1;

        macMask[0] = 0xFF;
        macMask[1] = 0xFF;
        macMask[2] = 0xFF;
        macMask[3] = 0xFF;
        macMask[4] = 0xFF;
        macMask[5] = 0xFF;

        memset(&portData, 0, sizeof(bcm_pbmp_t));
        memset(&portMask, 0, sizeof(bcm_pbmp_t));

        portData.pbits[0] = 0x1;
        portData.pbits[1] = 0x0;
        portData.pbits[2] = 0x0;
        portMask = PBMP_ALL(feUnit);

        entryCur = entryCnt;

        rv = bcm_field_entry_create_id(feUnit, group, entryCnt);
        if (BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_entry_create_id (group=%d,cnt=%d) error, rv=%d(%s).\n",
                    FUNCTION_NAME(), __LINE__, group, entryCnt, rv, bcm_errmsg(rv));
            break;
        }

        rv = bcm_field_qualify_InPorts(feUnit, entryCnt, portData, portMask);
        if (BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_qualify_InPorts error, rv=%d.\n",
                    FUNCTION_NAME(), __LINE__, rv);
            break;
        }

        rv = bcm_field_qualify_SrcMac(feUnit, entryCnt, macData, macMask);
        if (BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_qualify_SrcMac error, rv=%d.\n",
                    FUNCTION_NAME(), __LINE__, rv);
            break;
        }

        rv = bcm_field_action_add(feUnit, entryCnt, bcmFieldActionDrop, 0, 0);
        if (BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_action_add error, rv=%d.\n",
                    FUNCTION_NAME(), __LINE__, rv);
           break;
        }
        
#if 0
        /* Install the ACL rule group. */
        rv = bcm_field_group_install(feUnit, group);
        if(BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_group_install error, rv=%d.\n",
                    FUNCTION_NAME(), __LINE__, rv);
        }
#endif

#if 1
        /* Install the ACL rule group. */
        rv = bcm_field_entry_install(feUnit, entryCnt);
        if (BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_group_install error, rv=%d.\n",
                    FUNCTION_NAME(), __LINE__, rv);
            return BCM_E_FAIL;
        }
#endif

	acl_test_count++;
        /*taskDelay(5);*/
	/*        printf(" ## entry id %d\n", entryCnt);*/
    }

    return BCM_E_NONE;
}




int bcm_acl_dma_del_thread(int feUnit, bcm_field_group_t gid, uint32_t entryStart, uint32_t entryEnd)
{
    int rv = 0;
    int eid = 0;

    if (entryStart > entryEnd)
    {
      printk("entryStart > entryEnd\n");
        return BCM_E_PARAM;
    }

    for (eid = entryStart; eid < entryEnd; eid++)
    {
        bcm_field_entry_remove(feUnit, eid);
        if (BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_entry_remove error, rv=%d. eid=%d\n",
                    FUNCTION_NAME(), __LINE__, rv, eid);
            continue;
        }

        rv = bcm_field_entry_destroy(feUnit, eid);
        if (BCM_E_NONE != rv)
        {
            printf("<%s>: Line=%d, bcm_field_entry_destroy error, rv=%d. eid=%d\n", 
                    FUNCTION_NAME(), __LINE__, rv, eid);
            break;
        }

	acl_test_count--;
	/*        printf(" ## entry id %d\n", eid);*/
    }

    rv = bcm_field_group_destroy(feUnit, gid);
    if (BCM_E_NONE != rv)
    {
        printf("<%s>: Line=%d, bcm_field_group_destroy error, rv=%d. gid=%d\n", 
                FUNCTION_NAME(), __LINE__, rv, gid);
        return rv;
    }

    return BCM_E_NONE;
}

void acl_test_loop()
{
  int i=0;
  for (i=0; i<5; i++) {
    if (bcm_acl_dma_add_thread(1, 0, 0, 7678) != BCM_E_NONE)
      break;
    if (bcm_acl_dma_del_thread(1, 0, 0,7678) != BCM_E_NONE)
      break;
  }
  acl_test_thread_id = NULL;
  printk("ACL test thread finished.\n");
}
#endif /*TEST_ACL_ADD_DELETE_SPEED*/

cmd_result_t 
_cmd_g2p3_util_constants(int unit, args_t *args)
{
    extern int soc_sbx_g2p3_constant_shell_print(int unit, int cid);
    extern int soc_sbx_g2p3_constant_get(int unit, char *sym, uint32 *vp);

    int i;

    if (ARG_CNT(args)) {
        int rv, value, nFound = 0;
        soc_sbx_g2p3_state_t *fe =
            (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
        soc_sbx_g2p3_table_manager_t *tm = fe->tableinfo;
        
        value = parse_integer(ARG_GET(args));

        for (i = 0; i < SOC_SBX_G2P3_CONSTANT_MAX_ID; i++) {
            uint32 constVal;

            rv = soc_sbx_g2p3_constant_get(unit, tm->constants[i].name,
                                           &constVal);
            if (SOC_SUCCESS(rv)) {
                if (value == constVal) {
                    soc_sbx_g2p3_constant_shell_print(unit, i);
                    nFound++;
                }
            }
        }
        printk("Found %d constants with value %d\n", nFound, value);
        return CMD_OK;
    }

    for (i = 0; i < SOC_SBX_G2P3_CONSTANT_MAX_ID; i++) {
        soc_sbx_g2p3_constant_shell_print(unit, i);
    }

    return CMD_OK;
}

char cmd_sbx_g2p3_util_usage[] =
"Usage:\n"
"  g2p3util help               - displays this messge\n"
"  g2p3util pvv2ecap           - pvv2e capacity test\n"
"  g2p3util pvv2ekeys ivid=<iv> ovid=<ov> port=<p> - displays pvv2e keys\n"
"  g2p3util faststp [svid=<sv>] [evid=<ev>] - test fast STP\n"
"  g2p3util testmcast vid=<VID> [pass=<count>] [group=<count>] [addr=<count>] - test mcast\n"
"  g2p3util aceget <ingress || egress> <index>  - retrieved a classifier ACE entry and associated rule action\n"
"  g2p3util reloaducodebin [cfile] - disable lrp, load ucode from binary, then enable it\n"
"  g2p3util constants [val]        - displays G2P3 constant names matching an integer value, all if no value supplied\n"
"  g2p3util allocator              - displays G2P3 SDK table allocations\n";

void sal_alloc_stat(void *param);

extern int soc_sbx_g2p3_allocator_shell_print(int unit);
cmd_result_t
cmd_sbx_g2p3_util(int unit, args_t *a)
{
    char *cmd, *subcmd;
    cmd_result_t rv = CMD_OK;
    int ivid, ovid, port, b;
    uint32 left, right, seed;
    parse_table_t pt;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    cmd = ARG_CMD(a);
    if (!sh_check_attached(cmd, unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2P3) {
        printk("ERROR: not running gu2 v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "help") == 0) {
      rv = CMD_USAGE;
    } else if (sal_strcasecmp(subcmd, "constants") == 0) {
        return _cmd_g2p3_util_constants(unit, a);
    } else if (sal_strcasecmp(subcmd, "svidcap") == 0) {
      cmd_sbx_g2p3_svid_cap(unit, NULL);
    } else if (sal_strcasecmp(subcmd, "pvv2ekeys") == 0) {
        ivid = 0xfff;
        ovid = 0xfff;
        parse_table_init(0, &pt);

        port = CMIC_PORT(unit);
        parse_table_add(&pt, "ivid", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &ivid, NULL);
        parse_table_add(&pt, "ovid", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &ovid, NULL);
        parse_table_add(&pt, "port", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &port, NULL);
        if (parseEndOk(a, &pt, &rv)) {
            soc_sbx_g2p3_pvv2e_keys_get(unit, ivid, ovid, port,
                                        &left, &right, &seed, &b);
            printk("pvv2e left key = 0x%x, ", left);
            printk("right key = 0x%x, ", right);
            printk("seed = 0x%x, ", seed);
            printk(" bank: %s\n", b ? "b" : "a");
        }
    } else if (sal_strcasecmp(subcmd, "faststp") == 0) {
        sal_usecs_t st, et;
        sal_usecs_t igetusec = 0;
        sal_usecs_t iget10usec = 0;
        sal_usecs_t isetusec = 0;
        sal_usecs_t iset10usec = 0;
        sal_usecs_t egetusec = 0;
        sal_usecs_t eget10usec = 0;
        sal_usecs_t esetusec = 0;
        sal_usecs_t eset10usec = 0;
        sal_usecs_t ifastusec, ifast10usec, efastusec, efast10usec;
        soc_sbx_g2p3_pv2e_t *pv2es;
        soc_sbx_g2p3_epv2e_t *epv2es;
        uint32 *istates, *edrops;
        int  svid, evid, vids, s, i, fastonly;
        int pass;
        int passes = 64; /* enough so timer grain is insignificant */
        int factor1 = 32; /* keep long ones tolerable; make short ones longer */
        int factor2 = 128; /* keep long ones tolerable; make short ones longer */
        int64 count;
        int64 fcount1;
        int64 fcount2;

        svid = 0;
        evid = 0xfff;
        fastonly = 0;
        parse_table_init(0, &pt);

        port = CMIC_PORT(unit);
        parse_table_add(&pt, "svid", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &svid, NULL);
        parse_table_add(&pt, "evid", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &evid, NULL);
        parse_table_add(&pt, "fastonly", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &fastonly, NULL);
        parse_table_add(&pt,
                        "passes",
                        PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0,
                        &passes,
                        NULL);
        parse_table_add(&pt,
                        "factor1",
                        PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0,
                        &factor1,
                        NULL);
        parse_table_add(&pt,
                        "factor2",
                        PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0,
                        &factor2,
                        NULL);
        if (parseEndOk(a, &pt, &rv)) {
            port = CMIC_PORT(unit);
            vids = evid - svid + 1;
            pv2es = sal_alloc(vids * sizeof(soc_sbx_g2p3_pv2e_t), "pv2es");
            epv2es = sal_alloc(vids * sizeof(soc_sbx_g2p3_epv2e_t), "epv2es");
            istates = sal_alloc(vids * sizeof(uint32), "istates");
            edrops = sal_alloc(vids * sizeof(uint32), "edrops");
            count = vids;
            count *= 100000;
            count *= passes;
            fcount1 = count * factor1;
            fcount2 = count * factor2;

            if (!fastonly) {
                st = sal_time_usecs();
                for (pass = 0; pass < passes; pass++) {
                    for (i = svid; i <= evid; i++) {
                        s = soc_sbx_g2p3_pv2e_get(unit, i, port, &pv2es[i - svid]);
                        if (s != SOC_E_NONE) {
                            printk("ERROR: pv2e get port %d, vid %d failed %d: "
                                   "%s\n",
                                   port, i, s, bcm_errmsg(s));
                            sal_free(pv2es);
                            sal_free(epv2es);
                            sal_free(istates);
                            sal_free(edrops);
                            return CMD_FAIL;
                        }
                        istates[i - svid] = pv2es[i - svid].stpstate;
                    }
                }
                et = sal_time_usecs();
                igetusec = SAL_USECS_SUB(et, st);
                iget10usec = igetusec / 10;
                printk("Got %d pv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       igetusec,
                       iget10usec ? (int)(count / iget10usec) : -1);

                st = sal_time_usecs();
                for (pass = 0; pass < passes; pass++) {
                    for (i = svid; i <= evid; i++) {
                        s = soc_sbx_g2p3_epv2e_get(unit, i, port,
                                                   &epv2es[i - svid]);
                        if (s != SOC_E_NONE) {
                            printk("ERROR: epv2e get port %d, vid %d failed %d: "
                                   "%s\n",
                                   port, i, s, bcm_errmsg(s));
                            sal_free(pv2es);
                            sal_free(epv2es);
                            sal_free(istates);
                            sal_free(edrops);
                            return CMD_FAIL;
                        }
                        edrops[i - svid] = epv2es[i - svid].drop;
                    }
                }
                et = sal_time_usecs();
                egetusec = SAL_USECS_SUB(et, st);
                eget10usec = egetusec / 10;
                printk("Got %d epv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       egetusec,
                       eget10usec ? (int)(count / eget10usec) : -1);
                printk("Got %d pv2es & epv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       igetusec + egetusec,
                       iget10usec + eget10usec ? (int)(count / (iget10usec + eget10usec)) : -1);

                st = sal_time_usecs();
                for (pass = 0; pass < passes; pass++) {
                    for (i = svid; i <= evid; i++) {
                        s = soc_sbx_g2p3_pv2e_set(unit, i, port, &pv2es[i - svid]);
                        if (s != SOC_E_NONE) {
                            printk("ERROR: pv2e set port %d, vid %d failed %d: "
                                   "%s\n",
                                   port, i, s, bcm_errmsg(s));
                            sal_free(pv2es);
                            sal_free(epv2es);
                            sal_free(istates);
                            sal_free(edrops);
                            return CMD_FAIL;
                        }
                    }
                }
                et = sal_time_usecs();
                isetusec = SAL_USECS_SUB(et, st);
                iset10usec = isetusec / 10;
                printk("Set %d pv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       isetusec,
                       iset10usec ? (int)(count / iset10usec) : -1);
                printk("Got/set %d pv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       igetusec + isetusec,
                       iget10usec + iset10usec ? (int)(count / (iget10usec + iset10usec)) : -1);
                st = sal_time_usecs();
                for (pass = 0; pass < passes; pass++) {
                    for (i = svid; i <= evid; i++) {
                        s = soc_sbx_g2p3_epv2e_set(unit, i, port,
                                                   &epv2es[i - svid]);
                        if (s != SOC_E_NONE) {
                            printk("ERROR: epv2e set port %d, vid %d failed %d: "
                                  "%s\n",
                                   port, i, s, bcm_errmsg(s));
                            sal_free(pv2es);
                            sal_free(epv2es);
                            sal_free(istates);
                            sal_free(edrops);
                            return CMD_FAIL;
                        }
                    }
                }
                et = sal_time_usecs();
                esetusec = SAL_USECS_SUB(et, st);
                eset10usec = esetusec / 10;
                printk("Set %d epv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       esetusec,
                       eset10usec ? (int)(count / eset10usec) : -1);
                printk("Got/set %d epv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       egetusec + esetusec,
                       eget10usec + eset10usec ? (int)(count / (eget10usec + eset10usec)) : -1);
                printk("Set %d pv2es & epv2es in %u microseconds (%d/sec)\n",
                       vids * passes,
                       isetusec + esetusec,
                       iset10usec + eset10usec ? (int)(count / (iset10usec + eset10usec)) : -1);
            }

            st = sal_time_usecs();
            for (pass = 0; pass < passes * factor1; pass++) {
                s = soc_sbx_g2p3_pv2e_stpstate_fast_set(unit, svid, port,
                                                       evid, port, NULL,
                                                        istates, vids);
                if (s != SOC_E_NONE) {
                    printk("ERROR: pv2e stpstate fast set failed %d: %s\n",
                           s, bcm_errmsg(s));
                            sal_free(pv2es);
                            sal_free(epv2es);
                            sal_free(istates);
                            sal_free(edrops);
                    return CMD_FAIL;
                }
            }
            et = sal_time_usecs();
            ifastusec = SAL_USECS_SUB(et, st);
            ifast10usec = ifastusec / 10;
            printk("Fast set %d pv2e.stpstates in %u microseconds (%d/sec)\n",
                   vids * passes * factor1,
                   ifastusec,
                   ifast10usec ? (int)(fcount1 / ifast10usec) : -1);

            if (!fastonly) {
                printk("Fast pv2e.stpstate set gives %dx original performance\n",
                       ifastusec ? (igetusec + isetusec)  / (ifastusec / factor1) : -1);
            }

            st = sal_time_usecs();
            for (pass = 0; pass < passes * factor2; pass++) {
                s = soc_sbx_g2p3_epv2e_drop_fast_set(unit, svid, port,
                                                     evid, port, NULL,
                                                     edrops, vids);
                if (s != SOC_E_NONE) {
                    printk("ERROR: epv2e stpdrop fast set failed %d: %s\n",
                           s, bcm_errmsg(s));
                            sal_free(pv2es);
                            sal_free(epv2es);
                            sal_free(istates);
                            sal_free(edrops);
                    return CMD_FAIL;
                }
            }
            et = sal_time_usecs();
            efastusec = SAL_USECS_SUB(et, st);
            efast10usec = efastusec / 10;
            printk("Fast set %d epv2e.stpdrops in %u microseconds (%d/sec)\n",
                   vids * passes * factor2,
                   efastusec,
                   efast10usec ? (int)(fcount2 / efast10usec) : -1);

            if (!fastonly) {
                printk("Fast set epv2e.stpdrops gives %dx original performance\n\n",
                       efastusec ? (egetusec + esetusec) / (efastusec / factor2) : -1);

                printk("Normal update %d in %u microseconds (%d/sec)\n",
                       vids * passes,
                       igetusec + igetusec + egetusec + esetusec,
                       iget10usec + iget10usec + eget10usec + eset10usec ? (int)(count / (iget10usec + iget10usec + eget10usec + eset10usec)) : -1);
            }

            printk("Fast update %d in %u microseconds (%d/sec)\n",
                   vids * passes * factor1,
                   ifastusec + (int)(((int64)efastusec * factor1) / factor2),
                   ifast10usec + efast10usec ? (int)(fcount1 / (ifast10usec + (int)(((int64)efast10usec * factor1) / factor2))) : -1);

            if (!fastonly) {
                printk("Fast update provides %dx original performance\n",
                       ifastusec + efastusec ? (igetusec + igetusec + egetusec + esetusec) / ((ifastusec / factor1) + (efastusec / factor2)) : -1);
            }

            sal_free(edrops);
            sal_free(istates);
            sal_free(epv2es);
            sal_free(pv2es);
        }
    } else if (0 == sal_strcasecmp(subcmd, "testmcast")) {
        bcm_vlan_t vid;
        int vidTemp;
        static int passDefault = 4;
        static int passes = 4;
        int pass;
        static int groupDefault = 8;
        static int groups = 8;
        int group;
        static int addrDefault = 4096;
        static int addresses = 4096;
        int addr;
        int result;
        bcm_mac_t mac;
        bcm_mcast_addr_t mca;
        bcm_multicast_t groupId;
        bcm_port_t port;
        int qeUnit = -1;
        bcm_pbmp_t pbmp;
        bcm_pbmp_t ubmp;

        parse_table_init(0, &pt);
        parse_table_add(&pt, "vid", PQ_INT | PQ_NO_EQ_OPT, NULL, &vidTemp, NULL);
        parse_table_add(&pt, "pass", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT, &passDefault, &passes, NULL);
        parse_table_add(&pt, "group", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT, &groupDefault, &groups, NULL);
        parse_table_add(&pt, "addr", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT, &addrDefault, &addresses, NULL);
        if (parseEndOk(a, &pt, &rv)) {
            vid = vidTemp;
            result = bcm_vlan_port_get(unit, vid, &pbmp, &ubmp);
            if (BCM_E_NONE == result) {
                PBMP_ITER(pbmp, port) {
                    if ((0 <= port) && (SBX_MAX_PORTS > port)) {
                        qeUnit = SOC_SBX_QE_FROM_FE(unit, port);
                        break;
                    }
                }
                if (-1 == qeUnit) {
                    /* there are no ports in this VSI */
                    printk("there must be at least one port in the vid!\n");
                    return CMD_FAIL;
                }
                for (pass = 0; pass < passes; pass++) {
                    printk("pass %d of %d\n", pass, passes);
                    for (group = 0; group < groups; group++) {
                        if ((groups < 64) || (0 == (group & 0x3F))) {
                            printk("  group %d of %d\n", group, groups);
                        }
                        groupId = -1;
                        result = bcm_multicast_create(qeUnit, BCM_MULTICAST_TYPE_VPLS, &groupId);
                        if (BCM_E_NONE == result) {
                            for (addr = 0;
                                 (BCM_E_NONE == result) && (addr < addresses);
                                 addr++) {
                                if (0 == (addr & 0x7FF)) {
                                    printk("    address %d of %d - add\n", addr, addresses);
                                }
                                mac[0] = 0x01;
                                mac[1] = group & 0xFF;
                                mac[2] = 0x5E;
                                mac[3] = (addr >> 16) & 0xFF;
                                mac[4] = (addr >>  8) & 0xFF;
                                mac[5] =  addr        & 0xFF;
                                bcm_mcast_addr_t_init(&mca, mac, vid);
                                mca.distribution_class = 1;
                                mca.flags = BCM_MCAST_USE_FABRIC_DISTRIBUTION;
                                mca.l2mc_index = groupId;
                                mca.pbmp = pbmp;
                                mca.ubmp = ubmp;
                                result = bcm_mcast_addr_add_w_l2mcindex(unit, &mca);
                                if (BCM_E_NONE != result) {
                                    printk("unable to add multicast group %d address %02X:%06X to unit %d: %d (%s)\n",
                                           groupId,
                                           group,
                                           addr,
                                           unit,
                                           result,
                                           _SHR_ERRMSG(result));
                                }
                            }
                            while (addr > 0) {
                                addr--;
                                if (0 == (addr & 0x7FF)) {
                                    printk("    address %d of %d - remove\n", addr, addresses);
                                }
                                mac[0] = 0x01;
                                mac[1] = group & 0xFF;
                                mac[2] = 0x5E;
                                mac[3] = (addr >> 16) & 0xFF;
                                mac[4] = (addr >>  8) & 0xFF;
                                mac[5] =  addr        & 0xFF;
                                bcm_mcast_addr_t_init(&mca, mac, vid);
                                mca.distribution_class = 1;
                                mca.flags = BCM_MCAST_USE_FABRIC_DISTRIBUTION;
                                mca.l2mc_index = groupId;
                                mca.pbmp = pbmp;
                                mca.ubmp = ubmp;
                                result = bcm_mcast_addr_remove_w_l2mcindex(unit, &mca);
                                if (BCM_E_NONE != result) {
                                    printk("unable to remove multicast group %d address %02X:%06X from unit %d: %d (%s)\n",
                                           groupId,
                                           group,
                                           addr,
                                           unit,
                                           result,
                                           _SHR_ERRMSG(result));
                                }
                            }
                            result = bcm_multicast_destroy(qeUnit, groupId);
                            if (BCM_E_NONE != result) {
                                printk("unable to destroy multicast group %d on QE unit %d: %d (%s)\n",
                                       groupId,
                                       unit,
                                       result,
                                       _SHR_ERRMSG(result));
                            }
                        } else {
                            printk("unable to create multicast group on QE unit %d: %d (%s)\n",
                                   qeUnit,
                                   result,
                                   _SHR_ERRMSG(result));
                        }
                    }
                }
            } else {
                printk("unable to get VLAN %03X ports: %d (%s)\n",
                       vid,
                       result,
                       _SHR_ERRMSG(result));
            }
            printk("done.\n");
        }
    } else if (sal_strcasecmp(subcmd, "aceget") == 0) {
      int index;
      int db;
      char *pArg;
      int rv;
      soc_sbx_g2p3_ace_rule_handle_t h;
      soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

      if ((pArg = ARG_GET(a)) == NULL) {
          printk("Missing Db\n");
          return CMD_USAGE;
      }
      if (!sal_strncasecmp(pArg, "ingress", 7)){
          db = SB_G2P3_CLS_QOS;
      }else if (!sal_strncasecmp(pArg, "egress", 6)){
          db = SB_G2P3_CLS_EGRESS;
      }else{
          printk("Unrecognized Rule Database\n");
          return CMD_USAGE;
      }
      
      if ((pArg = ARG_GET(a)) == NULL) {
          printk("Missing index\n");
          return CMD_USAGE;
      }
      index = sal_ctoi(pArg, 0);

      rv = soc_sbx_g2p3_ace_nth_handle(unit, db, 0, index, &h);
      if ( rv != SOC_E_NONE ) {
          if (rv == SOC_E_NOT_FOUND) {
              printk("Entry 0x%x not found\n", index);
          }else{
              printk("Get failed: 0x%x \n", rv);
          }
          return rv;
      }
      /* print rule handle */
      if (db == SB_G2P3_CLS_QOS) {
        soc_sbx_g2p3_ace_rule_handle_print(sbx->drv, h);
      }else{
        soc_sbx_g2p3_ace_rule_handle_print(sbx->drv, h);
      }
    } else if (sal_strcasecmp(subcmd, "reloaducodebin") == 0) {
#ifndef NO_FILEIO
      char *ucodeFile, *subcmd;
      unsigned int rlen=0;
      unsigned int rcount=0;
      unsigned int rvalue;
      FILE *rf;
      unsigned char *rbuf=NULL, *tmpbuf;
      soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
      int c_file=0;
      int err;

      if ((ucodeFile = ARG_GET(a)) == NULL) {
	printk("Missing ucodeFile\n");
	return CMD_USAGE;
      }

      if ((subcmd = ARG_GET(a)) != NULL) {
        if (sal_strcasecmp(subcmd, "cfile") == 0) {
           c_file = 1;
        }
      }

      rf = fopen(ucodeFile, "rb");
      if(rf == 0) {
	  printf("didn't find the binary ucode image.\n");
	  return SB_UCODE_BAD_IMAGE_ERR_CODE;
      }

      if (c_file) {
        char ch;
        int  size_found=0;
        while((ch = (char)fgetc(rf))!='}') {
          if ((ch=='[') && (!size_found)) {
              fscanf(rf,"%d", &rlen);
              rbuf = (unsigned char*)sal_alloc(rlen, "ucode_buffer");
              if(!rbuf) {
                  printf("system error: malloc (...) \n");
                  return SOC_E_RESOURCE;
              }
              rcount = 0;
              size_found = 1;
          }
          if (((ch=='{') || (ch==',')) && (size_found)) {
              fscanf(rf,"%x", &rvalue);
              rbuf[rcount] = (unsigned char)rvalue;
              rcount++;
          }
        }
        printf("Load ucode from C file %s for %d bytes\n", ucodeFile, rcount);
      } else {
        err = fseek(rf, 0, SEEK_END);
        if (err == -1) {
            printf ("fseek returned error\n");
            fclose(rf);
            return err;
        }
        rlen = ftell(rf);
        if (rlen == -1) {
            printf ("ftell returned error\n");
            fclose(rf);
            return rlen;
        }
        rewind(rf);
        rbuf = (unsigned char*)sal_alloc(rlen, "ucode_buffer");
        if(!rbuf) {
            printf("system error: malloc (...) \n");
            rv = SOC_E_RESOURCE;
            fclose(rf);
            return rv;
        }else{
            tmpbuf = rbuf;
            fread (tmpbuf, rlen, 1, rf);
        }
      }

      fclose(rf);

      err = soc_sbx_fe2000_ucode_load_from_buffer(unit, rbuf, rlen);
      if(err != SB_OK)
      {
	printf("Load ucode from buffer failed with error code: %d\n", err);
	sal_free(rbuf);
        return err;
      }

      if (sbFe2000UcodeLoad(SOC_SBX_SBHANDLE(unit), sbx->ucode) ) {
          printf("soc_sbx_fe2000_ucode_load failed\n");
      }

      sal_free(rbuf);
#endif
    }else if (!sal_strcasecmp(subcmd, "allocator")) {
        return soc_sbx_g2p3_allocator_shell_print(unit);
    } else if (sal_strcasecmp(subcmd, "pvv2e-test") == 0) {
      soc_sbx_g2p3_pvv2e_t pvv2e;
      sal_usecs_t start;
      sal_memset(&pvv2e, 0, sizeof(pvv2e));
      pvv2e.vlan = 2;
      start = sal_time_usecs();
      soc_sbx_g2p3_pvv2e_set(1, 2, 2, 1, &pvv2e);
      printk("pvv2e took %u us\n", SAL_USECS_SUB(sal_time_usecs(), start));
#if TEST_ACL_ADD_DELETE_SPEED
    } else if (sal_strcasecmp(subcmd, "acl-test") == 0) {
      if (acl_test_thread_id == NULL) {
	acl_test_thread_id = sal_thread_create("acl-test", SAL_THREAD_STKSZ*10, 128, acl_test_loop, NULL);
	printk("ACL test thread pointer: %u\n", (uint32)acl_test_thread_id);
      } else {
	printk("ACLs added: %d\n", acl_test_count);
      }
#endif
    } else {
        return CMD_USAGE;
    }

    return rv;
}

#endif /* BCM_FE2000_SUPPORT */
