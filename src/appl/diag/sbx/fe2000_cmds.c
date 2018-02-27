/*
 * $Id: fe2000_cmds.c 1.72 Broadcom SDK $
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
 * File:        fe2000_cmds.c
 * Purpose:     FE-2000-specific diagnostic shell commands
 * Requires:
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <soc/defs.h>

#ifdef BCM_FE2000_SUPPORT

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/sbx_util.h>
#include <soc/sbx/fe2000.h>

#include <soc/sbx/fe2k/sbFe2000Util.h>
#include <soc/sbx/fe2k/sbFe2000InitUtils.h>
#include <soc/sbx/fe2kxt/sbFe2000XtInitUtils.h>
#include <soc/sbx/fe2k/sbZfFe2000QmQueueConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000PtQueueToPortEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000QmQueueState0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000QmQueueState1EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000PpInitialQueueStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000PpQueuePriorityGroupEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000PpeDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000LrpDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000PedDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000PpRxPortDataEntryConsole.hx>

#include <soc/sbx/fe2kxt/sbZfC2QmQueueConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtQueueToPortEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpInitialQueueStateEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpQueuePriorityGroupEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpeDescriptorConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrpDescriptorConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PedDescriptorConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpRxPortDataEntryConsole.hx>

#include <appl/diag/parse.h>
#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/register.h>
#include <appl/diag/sbx/field.h>
#include <appl/diag/sbx/gu2.h>
#include <appl/test/sbx_pkt.h>
#include <bcm/error.h>
#include <bcm/init.h>
#include <bcm/rx.h>
#include <bcm/vlan.h>
#include <bcm/stg.h>
#include <bcm/mcast.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/policer.h>
#include <bcm/oam.h>
#include <bcm/multicast.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#include <soc/sbx/g2p3/g2p3_int.h>
#endif

#include <bcm_int/sbx/fe2000/policer.h>

#include <shared/idxres_fl.h>
#include <shared/idxres_afl.h>

#define MAX_ILIB_STR    4096

#ifdef INCLUDE_TCL
extern cmd_result_t cmd_tcl(int unit, args_t *a);
#endif

static char pBuf[MAX_ILIB_STR];

#define SOCK_LOG_ZF(zFrameName, pFrame) \
do{ \
  zFrameName##_SPrint(pFrame, pBuf, MAX_ILIB_STR); \
  printk("%s", pBuf); \
}while(0)

static int
_fe2000_rx_port_data_read(sbhandle sbh, uint32_t uQueueId,
                   sbZfFe2000PpRxPortDataEntry_t *pRxPortData)
{
  /* Note that this zframe is identical for CA/C2 */
  uint32_t uBuffer[SB_ZF_FE2000_PP_RX_PORT_DATA_ENTRY_SIZE_IN_WORDS];

  if (sbFe2000UtilReadIndir(sbh, TRUE, TRUE,
                            SAND_HAL_FE2000_REG_OFFSET(sbh, PP_RX_PORT_DATA_MEM_ACC_CTRL),
                            uQueueId,
                            SB_ZF_FE2000_PP_RX_PORT_DATA_ENTRY_SIZE_IN_WORDS,
                            uBuffer)) {
      return TRUE;
  }

  sbZfFe2000PpRxPortDataEntry_Unpack(pRxPortData, (uint8_t *) &uBuffer[0],
                             SB_ZF_FE2000_PP_RX_PORT_DATA_ENTRY_SIZE_IN_BYTES);
  return FALSE;
}

static int
_fe2000xt_rx_port_data_read(sbhandle sbh, uint32_t uQueueId,
                   sbZfC2PpRxPortDataEntry_t *pRxPortData)
{
  /* Note that this zframe is identical for CA/C2 */
  uint32_t uBuffer[(SB_ZF_C2PPRXPORTDATAENTRY_SIZE_IN_BYTES+3)/4];

  if (sbFe2000UtilReadIndir(sbh, TRUE, TRUE,
                            SAND_HAL_FE2000_REG_OFFSET(sbh, PP_RX_PORT_DATA_MEM_ACC_CTRL),
                            uQueueId,
                            (SB_ZF_C2PPRXPORTDATAENTRY_SIZE_IN_BYTES+3)/4,
                            uBuffer)) {
      return TRUE;
  }

  sbZfC2PpRxPortDataEntry_Unpack(pRxPortData, (uint8_t *) &uBuffer[0],
                             SB_ZF_C2PPRXPORTDATAENTRY_SIZE_IN_BYTES);
  return FALSE;
}

static int
_fe2000_queue_priority_group_read(sbhandle sbh, uint32_t uQueueId,
                           sbZfFe2000PpQueuePriorityGroupEntry_t *qpg)
{
  uint32_t uBuffer[1];

  if (sbFe2000UtilReadIndir(sbh, TRUE, FALSE,
                            SAND_HAL_FE2000_REG_OFFSET(sbh, PP_QUEUE_PRI_GROUP_MEM_ACC_CTRL),
                            uQueueId, 1, uBuffer)) {
      return TRUE;
  }

  sbZfFe2000PpQueuePriorityGroupEntry_Unpack(qpg, (uint8_t *) &uBuffer[0], 4);
  return FALSE;
}

static int
_fe2000xt_queue_priority_group_read(sbhandle sbh, uint32_t uQueueId,
                           sbZfC2PpQueuePriorityGroupEntry_t *qpg)
{
  uint32_t uBuffer[1];

  if (sbFe2000UtilReadIndir(sbh, TRUE, FALSE,
                            SAND_HAL_FE2000_REG_OFFSET(sbh, PP_QUEUE_PRI_GROUP_MEM_ACC_CTRL),
                            uQueueId, 1, uBuffer)) {
      return TRUE;
  }

  sbZfC2PpQueuePriorityGroupEntry_Unpack(qpg, (uint8_t *) &uBuffer[0], 4);
  return FALSE;
}

static int _ports_per_iface[SB_FE2000_IF_PTYPE_MAX] =
    {64, 64, 12, 12, 1, 1, 1};
static char *_iface_names[SB_FE2000_IF_PTYPE_MAX] =
    {"spi0", "spi1", "agm0", "agm1", "xgm0", "xgm1", "pci"};

void
cmd_sbx_fe2000_print_queue_info(int unit, int queue)
{
  sbhandle sbh = SOC_SBX_CONTROL(unit)->sbhdl;
  unsigned int qn;
  int i, j;
  sbZfFe2000QmQueueConfigEntry_t qc;
  sbZfFe2000PtQueueToPortEntry_t q2p;
  sbZfFe2000QmQueueState0Entry_t qs0;
  sbZfFe2000QmQueueState1Entry_t qs1;
  sbZfFe2000PpInitialQueueStateEntry_t pqs;
  sbZfFe2000PpQueuePriorityGroupEntry_t qpg;
  sbZfFe2000PpRxPortDataEntry_t rxp;
  sbZfC2QmQueueConfigEntry_t xt_qc;
  sbZfC2PtQueueToPortEntry_t xt_q2p;
  sbZfC2QmQueueState0Entry_t xt_qs0;
  sbZfC2QmQueueState1Entry_t xt_qs1;
  sbZfC2PpInitialQueueStateEntry_t xt_pqs;
  sbZfC2PpQueuePriorityGroupEntry_t xt_qpg;
  sbZfC2PpRxPortDataEntry_t xt_rxp;


  if (!SOC_IS_SBX_FE2KXT(unit)){
      CaPtQueueToPortRead(sbh, queue, &q2p);
      CaQmQueueConfigRead(sbh, queue, &qc);
      CaQmQueueState0Read(sbh, queue, &qs0);
      CaQmQueueState1Read(sbh, queue, &qs1);
      CaPpInitialQueueStateRead(sbh, queue, &pqs);
      if (queue < 128) {
          _fe2000_queue_priority_group_read(sbh, queue, &qpg);
          _fe2000_rx_port_data_read(sbh, queue, &rxp);
      }
      printk("queue %d:  ", queue);
      for (i = SB_FE2000_IF_PTYPE_SPI0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
        for (j = 0; j < _ports_per_iface[i]; j++) {
          CaPrPortToQueueRead(sbh, i, j, &qn);
          if (qn == queue) {
            printk("is fed by %s.%d ", _iface_names[i], j);
          }
        }
      }
      for (i = SB_FE2000_IF_PTYPE_SPI0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
        for (j = 0; j < _ports_per_iface[i]; j++) {
          CaPtPortToQueueRead(sbh, i, j, &qn);
          if (qn == queue) {
            printk("feeds %s.%d ", _iface_names[i], j);
          }
        }
      }
      printk("\n");
      sbZfFe2000QmQueueConfigEntry_Print(&qc);
      sbZfFe2000PtQueueToPortEntry_Print(&q2p);
      sbZfFe2000QmQueueState0Entry_Print(&qs0);
      sbZfFe2000QmQueueState1Entry_Print(&qs1);
      sbZfFe2000PpInitialQueueStateEntry_Print(&pqs);
      if (queue < 128) {
          sbZfFe2000PpQueuePriorityGroupEntry_Print(&qpg);
          sbZfFe2000PpRxPortDataEntry_Print(&rxp);
      }
  }else{
      C2PtQueueToPortRead(sbh, queue, &xt_q2p);
      C2QmQueueConfigRead(sbh, queue, &xt_qc);
      C2QmQueueState0Read(sbh, queue, &xt_qs0);
      C2QmQueueState1Read(sbh, queue, &xt_qs1);
      C2PpInitialQueueStateRead(sbh, queue, &xt_pqs);
      if (queue < 128) {
          _fe2000xt_queue_priority_group_read(sbh, queue, &xt_qpg);
          _fe2000xt_rx_port_data_read(sbh, queue, &xt_rxp);
      }
      printk("queue %d:  ", queue);
      for (i = SB_FE2000_IF_PTYPE_SPI0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
        for (j = 0; j < _ports_per_iface[i]; j++) {
          C2PrPortToQueueRead(sbh, i, j, &qn);
          if (qn == queue) {
            printk("is fed by %s.%d ", _iface_names[i], j);
          }
        }
      }
      for (i = SB_FE2000_IF_PTYPE_SPI0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
        for (j = 0; j < _ports_per_iface[i]; j++) {
          C2PtPortToQueueRead(sbh, i, j, &qn);
          if (qn == queue) {
            printk("feeds %s.%d ", _iface_names[i], j);
          }
        }
      }
      printk("\n");
      sbZfC2QmQueueConfigEntry_Print(&xt_qc);
      sbZfC2PtQueueToPortEntry_Print(&xt_q2p);
      sbZfC2QmQueueState0Entry_Print(&xt_qs0);
      sbZfC2QmQueueState1Entry_Print(&xt_qs1);
      sbZfC2PpInitialQueueStateEntry_Print(&xt_pqs);
      if (queue < 128) {
          sbZfC2PpQueuePriorityGroupEntry_Print(&xt_qpg);
          sbZfC2PpRxPortDataEntry_Print(&xt_rxp);
      }
  }
}

char cmd_sbx_fe2000_sep_usage[] =
"Usage: SetEgressPages port rate [mtu]\n";
cmd_result_t
cmd_sbx_fe2000_sep(int unit, args_t *a)
{
    soc_sbx_control_t *sbx;
    sbhandle sbh;
    char *arg;
    int port, mbps, mtu = 9000;
    cmd_result_t rv = CMD_OK;
    int err;

    arg = ARG_CMD(a);
    if (!sh_check_attached(arg, unit)) {
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);
    sbh = sbx->sbhdl;

    if ((arg = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    port = sal_ctoi(arg, 0);

    if ((arg = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    mbps = sal_ctoi(arg, 0);

    if ((arg = ARG_GET(a)) != NULL) {
        mtu = sal_ctoi(arg, 0);
    }

    err = soc_sbx_fe2000_egress_pages_set(unit, port, mbps, mtu);
    if (err != SOC_E_NONE) {
        printk("soc_sbx_fe2000_egress_pages_set failed: %d\n", err);
        rv = CMD_FAIL;
    }
    return rv;
}

static int sbFeQueue = -1;

char cmd_sbx_g2k_util_usage[] =
"Usage:\n"
"  g2util ppearm [queue]     - arms ppe cam capture\n"
"  g2util ppedump            - dumps ppe cam information\n"
"  g2util getinst stream pc  - dump an instruction from memory\n"
"  g2util swsdump            - dump SWS queue mapping\n"
"  g2util ccdump             - dump CC one-to-many queue mapping for HiGig interfaces (FE2KXT only)\n"
"  g2util ccmod <iface> <entry> <port> <qid> - change SWS QID mapping for specifed CC entry (FE2KXT only)\n"
"                                                   - set port == -1 to match on all ports\n"
"                                                   - set qid==-1 to drop packet\n"
"  g2util help               - displays this messge\n";

cmd_result_t
cmd_sbx_g2k_util(int unit, args_t *a)
{
    soc_sbx_control_t *sbx;
    sbhandle sbh;
    char *cmd, *subcmd, *arg;

    sbG2P3FePpCamEntry_t  camConfig;
    sbG2P3FePpRamEntry_t  ramConfig;
    soc_sbx_g2p3_state_t       *pFe;

    cmd_result_t rv = CMD_OK;

    cmd = ARG_CMD(a);
    if (!sh_check_attached(cmd, unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    sbx = SOC_SBX_CONTROL(unit);
    sbh = sbx->sbhdl;

    if (sal_strcasecmp(subcmd, "ppearm") == 0) {
      if (SOC_IS_SBX_FE(unit)) {
          if ((subcmd = ARG_GET(a)) == NULL) {
              printk("Missing Queue Id for ppearm\n");
              return CMD_USAGE;
          }

          sbFeQueue = sal_ctoi(subcmd, 0);

          SAND_HAL_FE2000_RMW_FIELD(sbh, PP_DEBUG_PARSING_CAM_CTRL, QUEUE, sbFeQueue);
          SAND_HAL_FE2000_RMW_FIELD(sbh, PP_DEBUG_PARSING_CAM_CTRL, TRACE_ARM, 1);
          printk ("PPE Armed to capture on queue %d\n", sbFeQueue);
          rv = CMD_OK;
      } else {
          printk("PPE Arm command valid only on FE\n");
          rv = CMD_FAIL;
      }
    } else if (sal_strcasecmp(subcmd, "ppedump") == 0) {
      if (SOC_IS_SBX_FE(unit)) {
          if (sbFeQueue == -1) {
              printk("Run this command after arming PPE with a valid source queue\n");
              return CMD_FAIL;
          }

          printk ("PPE Dump \n");
          if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_CTRL, TRACE_CAPTURED,
                           SAND_HAL_FE2000_READ(sbh, PP_DEBUG_PARSING_CAM_CTRL))) {
              int cam_entry_id[] = {-1, -1, -1, -1, -1, -1, -1, -1};
              int cam_id;

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_0, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_0))) {
                  cam_entry_id[0] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_0,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_0));
                  printk ("CAM 0 %x \n", cam_entry_id[0]);
              }

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_1, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_1))) {
                  cam_entry_id[1] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_1,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_1));
                  printk ("CAM 1 %x \n", cam_entry_id[1]);
              }

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_2, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_2))) {
                  cam_entry_id[2] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_2,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_2));
                  printk ("CAM 2 %x \n", cam_entry_id[2]);
              }

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_3, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_3))) {
                  cam_entry_id[3] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_3,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_3));
                  printk ("CAM 3 %x \n", cam_entry_id[3]);
              }

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_4, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_4))) {
                  cam_entry_id[4] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_4,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_4));
                  printk ("CAM 4 %x \n", cam_entry_id[4]);
              }

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_5, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_5))) {
                  cam_entry_id[5] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_5,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_5));
                  printk ("CAM 5 %x \n", cam_entry_id[5]);
              }

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_6, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_6))) {
                  cam_entry_id[6] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_6,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_6));
                  printk ("CAM 6 %x \n", cam_entry_id[6]);
              }

              if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_DEBUG_PARSING_CAM_7, CAM_HIT,
                                     SAND_HAL_FE2000_READ(sbh,
                                                   PP_DEBUG_PARSING_CAM_7))) {
                  cam_entry_id[7] = SAND_HAL_FE2000_GET_FIELD(sbh,
                                         PP_DEBUG_PARSING_CAM_7,
                                         CAM_ENTRY,
                                         SAND_HAL_FE2000_READ(sbh,
                                                       PP_DEBUG_PARSING_CAM_7));
                  printk ("CAM 7 %x \n", cam_entry_id[7]);
              }

              printk("\n");

	      pFe = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;      
              for (cam_id = 0; cam_id < 8; cam_id++) {                        
                   if (cam_entry_id[cam_id] == -1) {                          
                       continue;                                              
                   }                                                          
                                                                              
                   sbG2P3FePpCamEntry_Init(&camConfig);            
                   sbG2P3FePpRamEntry_Init(&ramConfig);         
                /* if (soc_sbx_g2p3_pp_cam_rule_read(unit,                     
                                           cam_id,                            
                                           cam_entry_id[cam_id],              
                                           &camConfig,                        
                                           &ramConfig) < 0) {                 
                       continue;                                              
                   }                                                          
                */                                                            
                   printk("CAM %d Entry %d\n", cam_id, cam_entry_id[cam_id]); 
                   printk("-----------------\n");                             
                /* sbG2P3FePpCamEntry_Print(&camConfig);                */
                /* sbG2P3FePpRamEntry_Print(&ramConfig);                */
                   printk("\n");                                              
              }
              return CMD_OK;
          } else {
              printk("No PPE CAM trace captured\n");
              return CMD_OK;
          }
      } else {
          printk("PPE Dump command valid only on FE\n");
          rv = CMD_USAGE;
      }

    } else if (sal_strcasecmp(subcmd, "getinst") == 0) {
        int i, stream, startpc, endpc, task;
        uint32 data0, data1, data2, thread, opcode, rega, regb, tar;
        char *s;
        sbStatus_t sts;
        if ((arg = ARG_GET(a)) == NULL) {
            printk("Missing stream for get instruction\n");
            return CMD_USAGE;
        }
        stream = sal_ctoi(arg, 0);
        if ((arg = ARG_GET(a)) == NULL) {
            printk("Missing instruction for get instruction\n");
            return CMD_USAGE;
        }
        startpc = sal_ctoi(arg, 0);
        for (s = arg; *s && *s != '-'; s++);
        if (*s == '-') {
            s++;
            endpc = sal_ctoi(s, 0);
        } else if ((arg = ARG_GET(a))) {
            endpc = startpc + sal_ctoi(arg, 0) - 1;
        } else {
            endpc = startpc;
        }
        for (i = startpc; i <= endpc; i++) {
            sts = sbFe2000LrpGetInstruction(sbh, stream, i,
                                            &data0, &data1, &data2, &task);
            if (sts == SB_BAD_ARGUMENT_ERR_CODE) {
                printk("illegal stream (%d) or pc (%d)\n", stream, i);
                return CMD_USAGE;
            }
            if (sts != SB_OK) {
                printk("get instruction %d:%d failed: %d\n", stream, i, sts);
                return CMD_FAIL;
            }
            thread = (data2 >> 29) & 0x7;
            opcode = (data2 >> 20) & 0x1ff;
            rega = (data2 >> 19) & 1;
            regb = (data2 >> 18) & 1;
            tar = data2 & 0x3ffff;
            printk("%d:%d %s %d. op=0x%x, tar=0x%x, srca=0x%x%s, "
                   "srcb=0x%x%s\n",
                   stream, i, task ? "<" : ">", thread, opcode,
                   tar,
                   data1, rega ? "r" : "",
                   data0 >> 16, regb ? "r" : "");
            printk("          0x%08x %08x %08x\n", data2, data1, data0);
        }

    } else if (sal_strcasecmp(subcmd, "swsdump") == 0) {
      if (SOC_IS_SBX_FE(unit)) {
        sbFe2000InitParams_t *ip = &SOC_SBX_CFG_FE2000(unit)->init_params;
        sbFe2000XtInitParams_t *xtip = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;
        sbFe2000Queues_t *queues;
        sbFe2000Connection_t *c;
        int i;
        char many2one, one2many;
        if (!SOC_IS_SBX_FE2KXT(unit)){
            queues = &ip->queues;
        }else{
            queues = &xtip->queues;
        }
        printk("IFACE   (uPort)---> QID  ---> QID  --->   IFACE\n");
        for (i=0; i < queues->n; i++) {
          c = &queues->connections[i];
          many2one = (queues->many2onecon[i] != SB_FE2000_SWS_UNINITIALIZED_CON) ? '*' : ' ';
          one2many = (queues->one2manycon[i] != SB_FE2000_SWS_UNINITIALIZED_CON) ? '*' : ' ';
          if (!c->bEgress){
              printk("%c%4s.%-2d(0x%02x) ---> 0x%02x ---> 0x%02x ---> %c%4s.%-2d\n",
              one2many, _iface_names[c->from.ulInterfacePortType], c->from.ulPortId,
              c->ulUcodePort, queues->fromqid[i], queues->toqid[i],
              many2one, _iface_names[c->to.ulInterfacePortType], c->to.ulPortId);
          }else{
              printk("%c%4s.%-2d(0x%02x) <--- 0x%02x <--- 0x%02x <--- %c%4s.%-2d\n",
              many2one, _iface_names[c->to.ulInterfacePortType], c->to.ulPortId,
              c->ulUcodePort, queues->toqid[i], queues->fromqid[i],
              one2many, _iface_names[c->from.ulInterfacePortType], c->from.ulPortId);
          }
        }
      }else{
          printk("swsdump command valid only on FE\n");
          rv = CMD_USAGE;
      }
    } else if (sal_strcasecmp(subcmd, "ccdump") == 0) {
      if (SOC_IS_SBX_FE(unit)) {
          int i, e;
          int32_t destport, qid;
          if (!SOC_IS_SBX_FE2KXT(unit)){
              printk("CC block mapping only on FE2KXT\n");
          }else{
              for (i=SB_FE2000_IF_PTYPE_AGM0; i<=SB_FE2000_IF_PTYPE_XGM1; i++) {
                if (!SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, i) ) {
                    continue;
                }
                e=0;
                while ( sbFe2000XtCcPort2QidMapGet((sbhandle)unit, i, e, &destport, &qid) == SB_OK) {
                    printk("Iface: 0x%x (%4s) Entry: 0x%x ERHDestPort: 0x%02x ---> qid: 0x%02x\n", i, _iface_names[i], e, destport, qid);
                    e++;
                }
              }
          }
      }else{
          printk("ccdump command valid only on FE\n");
          rv = CMD_USAGE;
      }
    } else if (sal_strcasecmp(subcmd, "ccmod") == 0) {
      if (SOC_IS_SBX_FE(unit)) {
          uint32_t iface, entry;
          uint32_t port, qid;
          if ((arg = ARG_GET(a)) == NULL) {
              printk("Missing iface info for ccmod command\n");
              return CMD_USAGE;
          }
          iface = sal_ctoi(arg, 0);
          if ((arg = ARG_GET(a)) == NULL) {
              printk("Missing entry info for ccmod command\n");
              return CMD_USAGE;
          }
          entry = sal_ctoi(arg, 0);
          if ((arg = ARG_GET(a)) == NULL) {
              printk("Missing port info for ccmod command\n");
              return CMD_USAGE;
          }
          port = sal_ctoi(arg, 0);
          if ((arg = ARG_GET(a)) == NULL) {
              printk("Missing qid info for ccmod command\n");
              return CMD_USAGE;
          }
          qid = sal_ctoi(arg, 0);
          if (!SOC_IS_SBX_FE2KXT(unit)){
              printk("CC block mapping only on FE2KXT\n");
          }else{
              if ( sbFe2000XtCcPort2QidMapSet((sbhandle)unit, iface, entry, port, qid) == SB_OK) {
                rv = CMD_OK;
              }else{
                rv = CMD_FAIL;
              }
          }
      }else{
          printk("ccmod command valid only on FE\n");
          rv = CMD_USAGE;
      }
    } else if (sal_strcasecmp(subcmd, "help") == 0) {
      rv = CMD_USAGE;
    }

    return rv;
}


char cmd_sbx_fe2000_hc_usage[] =
"Usage:\n"
"  with no argument prints the current state of header capture\n"
"  on              - enable header capture\n"
"  off             - disable header capture\n"
"  queue=n         - enables PPE capture for the specified queue\n"
;

cmd_result_t
cmd_sbx_fe2000_hc(int unit, args_t *a)
{
    sbhandle sbh;
    int on = 0;
    int off = 0;
    int queue = -1;

    if( !SOC_SBX_INIT(unit) ){
        printk("Unit %d, not initialized - call 'init soc' first!\n", unit);
        return CMD_FAIL;
    }
    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("only supported on FE-2000\n");
        return CMD_FAIL;
    }

    sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "on", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &on, NULL);
        parse_table_add(&pt, "off", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &off, NULL);
        parse_table_add(&pt, "queue", PQ_DFL | PQ_INT,
                        0, &queue, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (on && off) {
      return CMD_USAGE;
    }
    if (on) {
        if (queue != -1) {
            SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, QUEUE, queue);
            SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, CAPTURE_ALL, 0);
        } else {
            SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, CAPTURE_ALL, 1);
        }
        SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, INTERRUPT, 1);
        SAND_HAL_FE2000_RMW_FIELD(sbh, PD_DEBUG, COPY_ALL, 1);
    } else if (off) {
        SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, CAPTURE_ALL, 0);
        SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, QUEUE, 0xff);
        SAND_HAL_FE2000_RMW_FIELD(sbh, PD_DEBUG, COPY_ALL, 0);
    }
    printk("PPE capture is on");
    if (!SAND_HAL_FE2000_GET_FIELD(sbh, PP_COPY_OUT, CAPTURE_ALL,
                           SAND_HAL_FE2000_READ(sbh, PP_COPY_OUT))) {
        printk(" for queue 0x%02x",
               SAND_HAL_FE2000_GET_FIELD(sbh, PP_COPY_OUT, QUEUE,
                                  SAND_HAL_FE2000_READ(sbh, PP_COPY_OUT)));
    }
    printk(", PED capture is %s\n",
           SAND_HAL_FE2000_GET_FIELD(sbh, PD_DEBUG, COPY_ALL,
                              SAND_HAL_FE2000_READ(sbh, PD_DEBUG))
           ? "on" : "off");

    return CMD_OK;
}

static void
ppe_print(sbZfFe2000PpeDescriptor_t *pFromStruct) {
  printk("Fe2000PpeDescriptor:: contbyte=0x%02x", (unsigned int)  pFromStruct->m_uContinueByte);
  printk(" hdr0=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType0);
  printk(" hdr1=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType1);
  printk(" hdr2=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType2);
  printk(" hdr3=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType3);
  printk("\n");

  printk("Fe2000PpeDescriptor:: hdr4=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType4);
  printk(" hdr5=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType5);
  printk(" hdr6=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType6);
  printk(" hdr7=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType7);
  printk(" hdr8=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType8);
  printk(" hdr9=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType9);
  printk("\n");

  printk("Fe2000PpeDescriptor:: locn0=0x%02x", (unsigned int)  pFromStruct->m_uLocation0);
  printk(" locn1=0x%02x", (unsigned int)  pFromStruct->m_uLocation1);
  printk(" locn2=0x%02x", (unsigned int)  pFromStruct->m_uLocation2);
  printk(" locn3=0x%02x", (unsigned int)  pFromStruct->m_uLocation3);
  printk("\n");

  printk("Fe2000PpeDescriptor:: locn4=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation4);
  printk(" locn5=0x%02x", (unsigned int)  pFromStruct->m_uLocation5);
  printk(" locn6=0x%02x", (unsigned int)  pFromStruct->m_uLocation6);
  printk(" locn7=0x%02x", (unsigned int)  pFromStruct->m_uLocation7);
  printk("\n");

  printk("Fe2000PpeDescriptor:: locn8=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation8);
  printk(" locn9=0x%02x", (unsigned int)  pFromStruct->m_uLocation9);
  printk(" str=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  printk(" buffer=0x%01x", (unsigned int)  pFromStruct->m_uBuffer);
  printk(" expidx=0x%02x", (unsigned int)  pFromStruct->m_uExceptIndex);
  printk("\n");

  printk("Fe2000PpeDescriptor:: sque=0x%02x",
         (unsigned int)  pFromStruct->m_uSourceQueue);
  printk(" rxport0=0x%02x", (unsigned int)  pFromStruct->m_uRxPortData0);
  printk(" repcnt=0x%03x", (unsigned int)  pFromStruct->m_uReplicantCnt);
  printk(" repbit=0x%01x", (unsigned int)  pFromStruct->m_bReplicantBit);
  printk("\n");

  printk("Fe2000PpeDescriptor:: resv0=0x%01x",
         (unsigned int)  pFromStruct->m_uResv0);
  printk(" drop=0x%01x", (unsigned int)  pFromStruct->m_uDrop);
  printk(" enq=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  printk(" dqueue=0x%02x", (unsigned int)  pFromStruct->m_uDqueue);
  printk("\n");

  printk("Fe2000PpeDescriptor:: rxport1=0x%05x",
         (unsigned int)  pFromStruct->m_uRxPortData1);
  printk(" frmlen=0x%04x", (unsigned int)  pFromStruct->m_uFrameLength);
  printk(" aggrhash=0x%08x", (unsigned int)  pFromStruct->m_uAggrHash);
  printk("\n");

  printk("Fe2000PpeDescriptor:: variable=0x%08x",
         (unsigned int)  pFromStruct->m_uVariable);
  printk("\n");
}
static void
ppe_c2_print(sbZfC2PpeDescriptor_t *pFromStruct) {
  printk("C2PpeDescriptor:: contbyte=0x%02x", (unsigned int)  pFromStruct->m_uContinueByte);
  printk(" hdr0=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType0);
  printk(" hdr1=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType1);
  printk(" hdr2=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType2);
  printk(" hdr3=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType3);
  printk("\n");

  printk("C2PpeDescriptor:: hdr4=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType4);
  printk(" hdr5=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType5);
  printk(" hdr6=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType6);
  printk(" hdr7=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType7);
  printk(" hdr8=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType8);
  printk(" hdr9=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType9);
  printk("\n");

  printk("C2PpeDescriptor:: locn0=0x%02x", (unsigned int)  pFromStruct->m_uLocation0);
  printk(" locn1=0x%02x", (unsigned int)  pFromStruct->m_uLocation1);
  printk(" locn2=0x%02x", (unsigned int)  pFromStruct->m_uLocation2);
  printk(" locn3=0x%02x", (unsigned int)  pFromStruct->m_uLocation3);
  printk("\n");

  printk("C2PpeDescriptor:: locn4=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation4);
  printk(" locn5=0x%02x", (unsigned int)  pFromStruct->m_uLocation5);
  printk(" locn6=0x%02x", (unsigned int)  pFromStruct->m_uLocation6);
  printk(" locn7=0x%02x", (unsigned int)  pFromStruct->m_uLocation7);
  printk("\n");

  printk("C2PpeDescriptor:: locn8=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation8);
  printk(" locn9=0x%02x", (unsigned int)  pFromStruct->m_uLocation9);
  printk(" str=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  printk(" buffer=0x%01x", (unsigned int)  pFromStruct->m_uBuffer);
  printk(" expidx=0x%02x", (unsigned int)  pFromStruct->m_uExceptIndex);
  printk("\n");

  printk("C2PpeDescriptor:: sque=0x%02x",
         (unsigned int)  pFromStruct->m_uSourceQueue);
  printk(" rxport0=0x%02x", (unsigned int)  pFromStruct->m_uRxPortData0);
  printk(" repcnt=0x%03x", (unsigned int)  pFromStruct->m_uReplicantCnt);
  printk(" repbit=0x%01x", (unsigned int)  pFromStruct->m_bReplicantBit);
  printk("\n");

  printk("C2PpeDescriptor:: resv0=0x%01x",
         (unsigned int)  pFromStruct->m_uResv0);
  printk(" drop=0x%01x", (unsigned int)  pFromStruct->m_uDrop);
  printk(" enq=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  printk(" dqueue=0x%02x", (unsigned int)  pFromStruct->m_uDqueue);
  printk("\n");

  printk("C2PpeDescriptor:: rxport1=0x%05x",
         (unsigned int)  pFromStruct->m_uRxPortData1);
  printk(" frmlen=0x%04x", (unsigned int)  pFromStruct->m_uFrameLength);
  printk(" aggrhash=0x%08x", (unsigned int)  pFromStruct->m_uAggrHash);
  printk("\n");

  printk("C2PpeDescriptor:: variable=0x%08x",
         (unsigned int)  pFromStruct->m_uVariable);
  printk("\n");
}

static void
pedin_print(sbZfFe2000LrpDescriptor_t *pFromStruct) {
  printk("Fe2000LrpDescriptor:: contbyte=0x%02x",
         (unsigned int)  pFromStruct->m_uContinueByte);
  printk(" hdr0=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType0);
  printk(" hdr1=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType1);
  printk(" hdr2=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType2);
  printk(" hdr3=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType3);
  printk("\n");

  printk("Fe2000LrpDescriptor:: hdr4=0x%01x",
         (unsigned int)  pFromStruct->m_uHeaderType4);
  printk(" hdr5=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType5);
  printk(" hdr6=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType6);
  printk(" hdr7=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType7);
  printk(" hdr8=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType8);
  printk(" hdr9=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType9);
  printk("\n");

  printk("Fe2000LrpDescriptor:: locn0=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation0);
  printk(" locn1=0x%02x", (unsigned int)  pFromStruct->m_uLocation1);
  printk(" locn2=0x%02x", (unsigned int)  pFromStruct->m_uLocation2);
  printk(" locn3=0x%02x", (unsigned int)  pFromStruct->m_uLocation3);
  printk("\n");

  printk("Fe2000LrpDescriptor:: locn4=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation4);
  printk(" locn5=0x%02x", (unsigned int)  pFromStruct->m_uLocation5);
  printk(" locn6=0x%02x", (unsigned int)  pFromStruct->m_uLocation6);
  printk(" locn7=0x%02x", (unsigned int)  pFromStruct->m_uLocation7);
  printk("\n");

  printk("Fe2000LrpDescriptor:: locn8=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation8);
  printk(" locn9=0x%02x", (unsigned int)  pFromStruct->m_uLocation9);
  printk(" str=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  printk(" buffer=0x%04x", (unsigned int)  pFromStruct->m_uBuffer);
  printk("\n");

  printk("Fe2000LrpDescriptor:: expidx=0x%02x",
         (unsigned int)  pFromStruct->m_uExceptIndex);
  printk(" sque=0x%02x", (unsigned int)  pFromStruct->m_uSourceQueue);
  printk(" tc=0x%01x", (unsigned int)  pFromStruct->m_uTruncationCommand);
  printk(" hccpy=0x%01x", (unsigned int)  pFromStruct->m_uHeaderCopy);
  printk(" mirridx=0x%01x", (unsigned int)  pFromStruct->m_uMirrorIndex);
  printk("\n");

  printk("Fe2000LrpDescriptor:: cpycnt=0x%03x",
         (unsigned int)  pFromStruct->m_uCopyCount);
  printk(" crcmode=0x%01x", (unsigned int)  pFromStruct->m_uCrcMode);
  printk(" dropbit=0x%01x", (unsigned int)  pFromStruct->m_uDropBit);
  printk(" enq=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  printk("\n");

  printk("Fe2000LrpDescriptor:: dqueue=0x%02x",
         (unsigned int)  pFromStruct->m_uDestinationQueue);
  printk(" xferlen=0x%04x", (unsigned int)  pFromStruct->m_uXferLength);
  printk(" packmark=0x%01x", (unsigned int)  pFromStruct->m_uPacketMarking);
  printk(" resv1=0x%01x", (unsigned int)  pFromStruct->m_uResv1);
  printk("\n");

  printk("Fe2000LrpDescriptor:: frmlen=0x%04x",
         (unsigned int)  pFromStruct->m_uFrameLength);
  printk(" resv2=0x%08x", (unsigned int)  pFromStruct->m_uResv2);
  printk(" resv3=0x%08x", (unsigned int)  pFromStruct->m_uResv3);
  printk("\n");
}

static void
pedin_c2_print(sbZfC2LrpDescriptor_t *pFromStruct) {
  printk("C2LrpDescriptor:: contbyte=0x%02x",
         (unsigned int)  pFromStruct->m_uContinueByte);
  printk(" hdr0=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType0);
  printk(" hdr1=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType1);
  printk(" hdr2=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType2);
  printk(" hdr3=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType3);
  printk("\n");

  printk("C2LrpDescriptor:: hdr4=0x%01x",
         (unsigned int)  pFromStruct->m_uHeaderType4);
  printk(" hdr5=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType5);
  printk(" hdr6=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType6);
  printk(" hdr7=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType7);
  printk(" hdr8=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType8);
  printk(" hdr9=0x%01x", (unsigned int)  pFromStruct->m_uHeaderType9);
  printk("\n");

  printk("C2LrpDescriptor:: locn0=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation0);
  printk(" locn1=0x%02x", (unsigned int)  pFromStruct->m_uLocation1);
  printk(" locn2=0x%02x", (unsigned int)  pFromStruct->m_uLocation2);
  printk(" locn3=0x%02x", (unsigned int)  pFromStruct->m_uLocation3);
  printk("\n");

  printk("C2LrpDescriptor:: locn4=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation4);
  printk(" locn5=0x%02x", (unsigned int)  pFromStruct->m_uLocation5);
  printk(" locn6=0x%02x", (unsigned int)  pFromStruct->m_uLocation6);
  printk(" locn7=0x%02x", (unsigned int)  pFromStruct->m_uLocation7);
  printk("\n");

  printk("C2LrpDescriptor:: locn8=0x%02x",
         (unsigned int)  pFromStruct->m_uLocation8);
  printk(" locn9=0x%02x", (unsigned int)  pFromStruct->m_uLocation9);
  printk(" str=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  printk(" buffer=0x%04x", (unsigned int)  pFromStruct->m_uBuffer);
  printk("\n");

  printk("C2LrpDescriptor:: expidx=0x%02x",
         (unsigned int)  pFromStruct->m_uExceptIndex);
  printk(" sque=0x%02x", (unsigned int)  pFromStruct->m_uSourceQueue);
  printk(" tc=0x%01x", (unsigned int)  pFromStruct->m_uTruncationCommand);
  printk(" hccpy=0x%01x", (unsigned int)  pFromStruct->m_uHeaderCopy);
  printk(" mirridx=0x%01x", (unsigned int)  pFromStruct->m_uMirrorIndex);
  printk("\n");

  printk("C2LrpDescriptor:: cpycnt=0x%03x",
         (unsigned int)  pFromStruct->m_uCopyCount);
  printk(" crcmode=0x%01x", (unsigned int)  pFromStruct->m_uCrcMode);
  printk(" dropbit=0x%01x", (unsigned int)  pFromStruct->m_uDropBit);
  printk(" enq=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  printk("\n");

  printk("C2LrpDescriptor:: dqueue=0x%02x",
         (unsigned int)  pFromStruct->m_uDestinationQueue);
  printk(" xferlen=0x%04x", (unsigned int)  pFromStruct->m_uXferLength);
  printk(" packmark=0x%01x", (unsigned int)  pFromStruct->m_uPacketMarking);
  printk(" resv1=0x%01x", (unsigned int)  pFromStruct->m_uResv1);
  printk("\n");

  printk("C2LrpDescriptor:: frmlen=0x%04x",
         (unsigned int)  pFromStruct->m_uFrameLength);
  printk(" resv2=0x%08x", (unsigned int)  pFromStruct->m_uResv2);
  printk(" resv3=0x%08x", (unsigned int)  pFromStruct->m_uResv3);
  printk("\n");
}

static void
pedout_print(sbZfFe2000PedDescriptor_t *pFromStruct) {
  printk("Fe2000PedDescriptor:: cntbyte=0x%02x",
         (unsigned int)  pFromStruct->m_uContinueByte);
  printk(" hdrlen=0x%02x", (unsigned int)  pFromStruct->m_uHeaderLength);
  printk(" resv0=0x%04x", (unsigned int)  pFromStruct->m_uResv0);
  printk(" strsel=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  printk("\n");

  printk("Fe2000PedDescriptor:: buffer=0x%04x",
         (unsigned int)  pFromStruct->m_uBuffer);
  printk(" excpidx=0x%02x", (unsigned int)  pFromStruct->m_uExceptionIndex);
  printk(" squeue=0x%02x", (unsigned int)  pFromStruct->m_uSourceQueue);
  printk(" resv1=0x%01x", (unsigned int)  pFromStruct->m_uResv1);
  printk("\n");

  printk("Fe2000PedDescriptor:: mridx=0x%01x",
         (unsigned int)  pFromStruct->m_uMirrorIndex);
  printk(" cpycnt=0x%03x", (unsigned int)  pFromStruct->m_uCopyCount);
  printk(" crcmode=0x%01x", (unsigned int)  pFromStruct->m_uCrcMode);
  printk(" dbit=0x%01x", (unsigned int)  pFromStruct->m_uDropBit);
  printk(" enque=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  printk("\n");

  printk("Fe2000PedDescriptor:: dque=0x%02x",
         (unsigned int)  pFromStruct->m_uDestinationQueue);
  printk(" xferlen=0x%04x", (unsigned int)  pFromStruct->m_uXferLength);
  printk(" packmark=0x%01x", (unsigned int)  pFromStruct->m_uPacketMarking);
  printk(" resv3=0x%01x", (unsigned int)  pFromStruct->m_uResv3);
  printk("\n");

  printk("Fe2000PedDescriptor:: srcfrmlen=0x%04x",
         (unsigned int)  pFromStruct->m_uSourceFrameLength);
  printk("\n");
}
static void
pedout_c2_print(sbZfC2PedDescriptor_t *pFromStruct) {
  printk("C2PedDescriptor:: cntbyte=0x%02x",
         (unsigned int)  pFromStruct->m_uContinueByte);
  printk(" hdrlen=0x%02x", (unsigned int)  pFromStruct->m_uHeaderLength);
  printk(" resv0=0x%04x", (unsigned int)  pFromStruct->m_uResv0);
  printk(" strsel=0x%01x", (unsigned int)  pFromStruct->m_uStreamSelector);
  printk("\n");

  printk("C2PedDescriptor:: buffer=0x%04x",
         (unsigned int)  pFromStruct->m_uBuffer);
  printk(" excpidx=0x%02x", (unsigned int)  pFromStruct->m_uExceptionIndex);
  printk(" squeue=0x%02x", (unsigned int)  pFromStruct->m_uSourceQueue);
  printk(" resv1=0x%01x", (unsigned int)  pFromStruct->m_uResv1);
  printk("\n");

  printk("C2PedDescriptor:: mridx=0x%01x",
         (unsigned int)  pFromStruct->m_uMirrorIndex);
  printk(" cpycnt=0x%03x", (unsigned int)  pFromStruct->m_uCopyCount);
  printk(" crcmode=0x%01x", (unsigned int)  pFromStruct->m_uCrcMode);
  printk(" dbit=0x%01x", (unsigned int)  pFromStruct->m_uDropBit);
  printk(" enque=0x%01x", (unsigned int)  pFromStruct->m_uEnqueue);
  printk("\n");

  printk("C2PedDescriptor:: dque=0x%02x",
         (unsigned int)  pFromStruct->m_uDestinationQueue);
  printk(" xferlen=0x%04x", (unsigned int)  pFromStruct->m_uXferLength);
  printk(" packmark=0x%01x", (unsigned int)  pFromStruct->m_uPacketMarking);
  printk(" resv3=0x%01x", (unsigned int)  pFromStruct->m_uResv3);
  printk("\n");

  printk("C2PedDescriptor:: srcfrmlen=0x%04x",
         (unsigned int)  pFromStruct->m_uSourceFrameLength);
  printk("\n");
}

typedef enum {
    HEADER_DUMP_PPE_OUT,
    HEADER_DUMP_PED_IN,
    HEADER_DUMP_PED_OUT,
} header_dump_type_t;

static cmd_result_t
_cmd_sbx_fe2000_dump_header(int unit, header_dump_type_t type,
                            int raw, int parsed)
{
    int hdr_words = SB_FE2000_HEADER_CAPTURE_SIZE_IN_BYTES/sizeof(uint32_t);
    uint32_t h[SB_FE2000_HEADER_CAPTURE_SIZE_IN_BYTES/sizeof(uint32_t)];
    uint32_t hr[SB_FE2000_HEADER_CAPTURE_SIZE_IN_BYTES/sizeof(uint32_t)];
    sbhandle sbh = SOC_SBX_CONTROL(unit)->sbhdl;
    sbZfFe2000PpeDescriptor_t ppd;
    sbZfFe2000LrpDescriptor_t lrd;
    sbZfFe2000PedDescriptor_t ped;
    sbZfC2PpeDescriptor_t ppd_c2;
    sbZfC2LrpDescriptor_t lrd_c2;
    sbZfC2PedDescriptor_t ped_c2;
    int i, j;
    int reg = 0;
    int descending = FALSE;
    char *name = NULL;

    switch (type) {
    case HEADER_DUMP_PPE_OUT:
        reg = SAND_HAL_FE2000_REG_OFFSET(sbh, PP_COPY_OUT_MEM_ACC_CTRL);
        name = "PPE_OUT";
        descending = TRUE;
        break;
    case HEADER_DUMP_PED_IN:
        reg = SAND_HAL_FE2000_REG_OFFSET(sbh, PD_COPY_IN_MEM_ACC_CTRL);
        name = "PED_IN";
        descending = FALSE;
        break;
    case HEADER_DUMP_PED_OUT:
        reg = SAND_HAL_FE2000_REG_OFFSET(sbh, PD_COPY_OUT_MEM_ACC_CTRL);
        name = "PED_OUT";
        descending = FALSE;
        break;
    }
    if ((0 == reg) || (NULL == name)) {
        printk("Unknown header dump type=%d", type);
        return CMD_FAIL;
    }


    j = descending ?  0 : hdr_words / 8 - 1;
    for (i = 0; i < hdr_words / 8; i++) {
        if (sbFe2000UtilReadIndir(sbh, FALSE, descending, reg, i, 8,
                                  &h[j * 8])) {
            printk("%s header read failed at %d\n", name, i);
            return CMD_FAIL;
        }
        if (sbFe2000UtilReadIndir(sbh, TRUE, descending, reg, i, 8,
                                  &hr[j * 8])) {
            printk("%s header read failed at %d\n", name, i);
            return CMD_FAIL;
        }
        j = descending ? j + 1 : j - 1;
    }

    if ( parsed ) {
        printk("%s descriptor:\n", name);
        switch (type) {
        case HEADER_DUMP_PPE_OUT:
            if (!SOC_IS_SBX_FE2KXT(unit)){
                sbZfFe2000PpeDescriptor_Unpack(&ppd, (uint8_t *) hr,
                                               hdr_words * sizeof(uint32_t));
                ppe_print(&ppd);
            }else{
                sbZfC2PpeDescriptor_Unpack(&ppd_c2, (uint8_t *) hr,
                                               hdr_words * sizeof(uint32_t));
                ppe_c2_print(&ppd_c2);
            }
            break;
        case HEADER_DUMP_PED_IN:
            if (!SOC_IS_SBX_FE2KXT(unit)){
                sbZfFe2000LrpDescriptor_Unpack(&lrd, (uint8_t *) hr,
                                               hdr_words * sizeof(uint32_t));
                pedin_print(&lrd);
            }else{
                sbZfC2LrpDescriptor_Unpack(&lrd_c2, (uint8_t *) hr,
                                               hdr_words * sizeof(uint32_t));
                pedin_c2_print(&lrd_c2);
            }
            break;
        case HEADER_DUMP_PED_OUT:
            /* PED_OUT is only 172 bits, adjust before unpack */
            for (i = 0; i < hdr_words - 5; i++) {
                hr[i] = hr[i + 5];
            }
            if (!SOC_IS_SBX_FE2KXT(unit)){
                sbZfFe2000PedDescriptor_Unpack(&ped, (uint8_t *) hr,
                                               hdr_words * sizeof(uint32_t));
                pedout_print(&ped);
            }else{
                sbZfC2PedDescriptor_Unpack(&ped_c2, (uint8_t *) hr,
                                               hdr_words * sizeof(uint32_t));
                pedout_c2_print(&ped_c2);
            }
            break;
        }
    }

    if ( raw ) {
        printk("%s descriptor dump:", name);
        for (i = 0; i < hdr_words; i++) {
            if (i % 8 == 0) {
                printk("\n%02d ", i);
            }
            j = hdr_words - i - 1;
            printk("%08x ", h[j]);
        }
        printk("\n");
    }

    return CMD_OK;
}

char cmd_sbx_fe2000_hd_usage[] =
"Usage:\n"
"  dump captured headers & clear capture buffer\n"
"  ppe             - dump captured header from PPE\n"
"  pedin           - dump captured header from PED input\n"
"  pedout          - dump captured header from ped output\n"
"  raw             - dump raw captured headers\n"
"  parsed          - dump parsed captured headers\n\n"
"if neither raw nor parsed are specified, both are dumped\n"
"if none of ppe, pedin and pedout are specified, all are dumped\n"
;

cmd_result_t
cmd_sbx_fe2000_hd(int unit, args_t *a)
{
    sbhandle sbh;
    int rv, i, in, out, n;
    int ppe = -1;
    int pedin = -1;
    int pedout = -1;
    int raw = -1;
    int parsed = -1;

    if( !SOC_SBX_INIT(unit) ){
        printk("unit %d, not initialized - call 'init soc' first!\n", unit);
        return CMD_FAIL;
    }
    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("only supported on FE-2000\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "ppe", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ppe, NULL);
        parse_table_add(&pt, "pedin", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &pedin, NULL);
        parse_table_add(&pt, "pedout", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &pedout, NULL);
        parse_table_add(&pt, "raw", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &raw, NULL);
        parse_table_add(&pt, "parsed", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &parsed, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (ppe == -1 && pedin == -1 && pedout == -1) {
        ppe = 1;
        pedin = 1;
        pedout = 1;
    }
    ppe = (ppe == -1) ? 0 : ppe;
    pedin = (pedin == -1) ? 0 : pedin;
    pedout = (pedout == -1) ? 0 : pedout;

    if (raw == -1 && parsed == -1) {
        raw = 1;
        parsed = 1;
    }
    raw = (raw == -1) ? 0 : raw;
    parsed = (parsed == -1) ? 0 : parsed;

    sbh = SOC_SBX_CONTROL(unit)->sbhdl;

    if (ppe) {
        if (SAND_HAL_FE2000_GET_FIELD(sbh, PP_COPY_OUT, INTERRUPT,
                               SAND_HAL_FE2000_READ(sbh, PP_COPY_OUT))) {
            printk("dumping captured PPE header\n");
            rv = _cmd_sbx_fe2000_dump_header(unit, HEADER_DUMP_PPE_OUT,
                                             raw, parsed);
            if (rv) {
                return rv;
            }
            SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, INTERRUPT, 1);
        }
    }

    in = pedin
        ? SAND_HAL_FE2000_GET_FIELD(sbh, PD_COPY_IN_EVENT, NUM_ENTRIES,
                             SAND_HAL_FE2000_READ(sbh, PD_COPY_IN_EVENT))
        : 0;
    out = pedout
        ? SAND_HAL_FE2000_GET_FIELD(sbh, PD_COPY_OUT_EVENT, NUM_ENTRIES,
                             SAND_HAL_FE2000_READ(sbh, PD_COPY_OUT_EVENT))
        : 0;

    n = in > out ? in : out;

    if (n) {
        printk("dumping %d captured PED headers\n", n);
    }
    for (i = 0; i < n; i++) {
        printk("PED header %d ---------------------------------------------\n",
               i);
        if (i < in) {
            rv = _cmd_sbx_fe2000_dump_header(unit, HEADER_DUMP_PED_IN,
                                             raw, parsed);
            if (rv) {
                return rv;
            }
        }
        SAND_HAL_FE2000_WRITE(sbh, PD_COPY_IN_EVENT,
                       SAND_HAL_FE2000_SET_FIELD(sbh, PD_COPY_IN_EVENT, POP, 1));
        if (i < out) {
            rv = _cmd_sbx_fe2000_dump_header(unit, HEADER_DUMP_PED_OUT,
                                             raw, parsed);
            if (rv) {
                return rv;
            }
        }
        SAND_HAL_FE2000_WRITE(sbh, PD_COPY_OUT_EVENT,
                       SAND_HAL_FE2000_SET_FIELD(sbh, PD_COPY_OUT_EVENT, POP, 1));
    }

    if (pedin
        && SAND_HAL_FE2000_GET_FIELD(sbh, PD_COPY_IN_EVENT, HDR_REC_OVERFLOW,
                                    SAND_HAL_FE2000_READ(sbh, PD_COPY_IN_EVENT))) {
        printk("PED_IN header capture buffer overflowed\n");
        SAND_HAL_FE2000_RMW_FIELD(sbh, PD_COPY_IN_EVENT, HDR_REC_OVERFLOW, 0x1);
    }

    if (pedout
        && SAND_HAL_FE2000_GET_FIELD(sbh, PD_COPY_OUT_EVENT, HDR_REC_OVERFLOW,
                              SAND_HAL_FE2000_READ(sbh, PD_COPY_OUT_EVENT))) {
        printk("PED_OUT header capture buffer overflowed\n");
        SAND_HAL_FE2000_RMW_FIELD(sbh, PD_COPY_OUT_EVENT, HDR_REC_OVERFLOW, 0x1);
    }

    return CMD_OK;
}

#ifdef INCLUDE_TEST
char cmd_sbx_fe2000_hg_usage[] =
"Usage:\n"
"  clear PPE capture, grab from specifed queue or port, and dump result\n"
"  queue           - grab and dump PPE header for specified SWS queue\n"
"  delay           - delay between PPE capture buffer clear and dump\n"
"  fabric          - parse the packet starting with Fabric header\n"
"  dumpold         - clear capture buffer\n"
;
cmd_result_t
cmd_sbx_fe2000_hg(int unit, args_t *a)
{
    sbhandle sbh;
    int queue = -1;
    int delay = -1;
    int dumpold = -1;
    int fabric = -1;
    int rv = 0;
    args_t *tcl_args;
    int hdr_words = SB_FE2000_HEADER_CAPTURE_SIZE_IN_BYTES/sizeof(uint32_t);
    uint32_t hr[SB_FE2000_HEADER_CAPTURE_SIZE_IN_BYTES/sizeof(uint32_t)];
    int i, j, reg, descending;
    sbZfFe2000PpeDescriptor_t ppd;
    char buf[SB_FE2000_HEADER_CAPTURE_SIZE_IN_BYTES*6] = {0};
    char *tcl_path;
    char script_path[250];
    sbxpkt_t *pkt;

    if( !SOC_SBX_INIT(unit) ){
        printk("unit %d, not initialized - call 'init soc' first!\n", unit);
        return CMD_FAIL;
    }
    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("only supported on FE-2000\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "queue", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &queue, NULL);
        parse_table_add(&pt, "delay", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &delay, NULL);
        parse_table_add(&pt, "dumpold", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &dumpold, NULL);
        parse_table_add(&pt, "fabric", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &fabric, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return CMD_USAGE;
        }
    }

    if (queue >= 128) {
        printk("Bad queue number\n");
    }

    if (delay == -1) {
        delay = 10;
    }

    if (fabric == -1) {
        fabric = 0;
    }

    if (dumpold == -1) {
        dumpold = 0;
    }

    sbh = SOC_SBX_CONTROL(unit)->sbhdl;

    /* configure capture */
    if (queue != -1) {
        SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, QUEUE, queue);
        SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, CAPTURE_ALL, 0);
    } else {
        SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, CAPTURE_ALL, 1);
    }
    SAND_HAL_FE2000_RMW_FIELD(sbh, PP_COPY_OUT, INTERRUPT, 1);

    if (dumpold) {
        /* clear buffer */
        rv = _cmd_sbx_fe2000_dump_header(unit, HEADER_DUMP_PPE_OUT, 0, 0);

        /* delay */
        sal_usleep(delay * 1000);

        if (!(SAND_HAL_FE2000_GET_FIELD(sbh, PP_COPY_OUT, INTERRUPT,
                           SAND_HAL_FE2000_READ(sbh, PP_COPY_OUT)))) {
            printk("No packet header captured on port %d\n", queue);
            return CMD_OK;
        }

    }

    /* dump */
    reg = SAND_HAL_FE2000_REG_OFFSET(sbh, PP_COPY_OUT_MEM_ACC_CTRL);
    descending = TRUE;
    j = 0;
    for (i = 0; i < hdr_words / 8; i++) {
        if (sbFe2000UtilReadIndir(sbh, TRUE, descending, reg, i, 8,
                                  &hr[j * 8])) {
            printk("PPE header read failed at %d\n", i);
            return CMD_FAIL;
        }
        j = j + 1;
    }
    sbZfFe2000PpeDescriptor_Unpack(&ppd, (uint8_t *) hr,
                                   hdr_words * sizeof(uint32_t));

    /* Yuck */
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte0);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte1);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte2);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte3);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte4);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte5);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte6);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte7);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte8);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte9);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte10);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte11);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte12);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte13);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte14);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte15);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte16);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte17);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte18);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte19);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte20);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte21);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte22);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte23);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte24);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte25);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte26);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte27);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte28);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte29);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte30);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte31);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte32);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte33);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte34);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte35);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte36);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte37);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte38);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte39);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte40);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte41);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte42);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte43);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte44);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte45);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte46);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte47);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte48);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte49);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte50);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte51);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte52);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte53);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte54);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte55);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte56);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte57);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte58);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte59);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte60);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte61);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte62);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte63);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte64);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte65);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte66);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte67);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte68);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte69);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte70);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte71);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte72);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte73);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte74);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte75);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte76);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte77);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte78);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte79);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte80);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte81);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte82);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte83);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte84);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte85);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte86);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte87);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte88);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte89);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte90);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte91);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte92);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte93);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte94);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte95);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte96);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte97);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte98);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte99);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte100);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte101);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte102);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte103);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte104);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte105);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte106);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte107);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte108);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte109);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte110);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte111);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte112);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte113);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte114);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte115);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte116);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte117);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte118);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte119);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte120);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte121);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte122);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte123);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte124);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte125);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte126);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte127);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte128);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte129);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte130);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte131);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte132);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte133);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte134);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte135);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte136);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte137);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte138);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte139);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte140);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte141);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte142);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte143);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte144);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte145);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte146);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte147);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte148);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte149);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte150);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte151);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte152);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte153);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte154);
    sal_sprintf(&buf[sal_strlen(buf)],"0x%.2x ", ppd.m_uPacketDataByte155);
    /* truncate at continue byte */
    buf[ppd.m_uContinueByte*5] = '\0';

#ifndef __KERNEL__
    /* everything is relative to the TCL_LIBRARY */
    tcl_path = getenv("TCL_LIBRARY");
    if ((tcl_path == NULL) || (tcl_path[0] == '\0')) {
        printk("TCL_LIBRARY environment variable not set\n");
        return CMD_FAIL;
    }
#endif

    printk("%d byte packet on port %d, captured header ",
           ppd.m_uFrameLength, queue);
    if ((sal_strlen(tcl_path)) > 150) {
        printk("TCL_LIBRARY environment variable too long: %s\n", tcl_path);
        return CMD_FAIL;
    }
    sprintf(script_path, "%s/../../regress/bcm/pkg/sbx/utils/header_grab.tcl",
            tcl_path);
    /* TCL start */
    /* invoke TCL packet parser */
    tcl_args = sal_alloc(sizeof(args_t), "tcl_args");
    sal_memset(tcl_args,0x0,sizeof(args_t));
    tcl_args->a_cmd="HEADER_GRAB";
    tcl_args->a_argc = 5;
    tcl_args->a_arg=1;
    tcl_args->a_argv[0] = ".tcl";
    tcl_args->a_argv[1] = script_path;
    tcl_args->a_argv[2] = tcl_path;
    tcl_args->a_argv[3] = fabric ? "pkt_gu2erh_parse" : "pkt_enet_parse";
    tcl_args->a_argv[4] = buf;

#ifdef INCLUDE_TCL
    cmd_tcl(unit, tcl_args);
#endif
    sal_free(tcl_args);

    printk("\n\n Format with sbx packet class:\n");
    /* Format with sbx packet class */
    pkt = sbxpkt_alloc();
    pkt->entry.type = PACKET;
    if (fabric) {
      from_byte (ERH_QE, &ppd.m_uPacketDataByte0, ppd.m_uFrameLength, pkt);
    }else {
      from_byte (MAC, &ppd.m_uPacketDataByte0, ppd.m_uFrameLength, pkt);
    }
    sbxpkt_print(pkt);
    debug_print(&ppd.m_uPacketDataByte0, pkt->entry.length);
    sbxpkt_free(pkt);

    return rv;
}



static const char *cmd_pkt_dump_parse_type[] = {
    "INVALID-packet",
    "erh_qe",
    "erh_ss",
    "erh_qess",
    "mac",
/*      VLAN,
      STAG,
      ETYPE,
      LLC,
      SNAP,
      IPV4,
      ITAG,
      MPLS,
      UDP,
      TCP,
      IGMP,
      RAW_DATA,
      UNKNOWN,*/
    NULL
};


char cmd_sbx_fe2000_pkt_dump_usage[] =
"\n PacketDump [-buffer_max=n] [-type=mac|erh_qe|erh_qess|erh_ss] \n"
  "            packet_hex_string [packet_hex_string]*\n"
"  -buffer_max       - maximum internal buffer size for the parsed packet, \n"
"                      in bytes.  Default=256\n"
"  -type             - packet type to be parsed - hint to parse the packet. \n"
"                       Valid values: erh_qe - sbx ERH for QE2k (Default)\n"
"                                     erh_ss - sbx ERH for Sirius\n"
"                                     erh_qess - sbx ERH for Qe/Sirius interop\n"
"                                     mac    - L2 packet\n"
"  packet_hex_string - one or more strings of assumed hex digits of the \n"
"                      form:  (0[xX])?[0-9a-fA-F]{1,\"buffer_max-1\"}\n"
"\n"
" PacketDump will continue to consume all arguments as hex strings. \n"
;

cmd_result_t
cmd_sbx_fe2000_pkt_dump(int unit, args_t *args)
{
    int       rv;
    uint8    *buffer;
    char     *token;
    char      hex_byte_str[5];
    int       is_odd, byte_pos;
    int       buffer_max = 256;
    int       pkt_type = 1;
    sbxpkt_t *pkt;

    if (!SOC_SBX_INIT(unit)) {
        printk("unit %d, not initialized - call 'init soc' first!\n", unit);
        return CMD_FAIL;
    }
        
    if (ARG_CNT(args)) {
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "buffer_max", PQ_DFL | PQ_INT, 
                        0, &buffer_max, NULL);
        parse_table_add(&pt, "type", PQ_MULTI, 
                        (void*)1, &pkt_type, cmd_pkt_dump_parse_type);

        rv = parse_arg_eq(args, &pt);
        parse_arg_eq_done(&pt);

        if (rv < 0) {
            return CMD_USAGE;
        }
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    if (pkt_type > COUNTOF(cmd_pkt_dump_parse_type) || pkt_type == 0) {
        printk("Uknown packet type: %d\n", pkt_type);
        return CMD_USAGE;
    }

    buffer = sal_alloc(buffer_max, "pkt_buffer");
    if (buffer == NULL) {
        printk("Failed to alloc temp buffer\n");
        return CMD_FAIL;
    }

    byte_pos = 0;
    sal_snprintf(hex_byte_str, 5, "0x00");

    /* Accepting repeating forms:  (0[xX])?[0-9a-fA-F]{1,"buffer_max-1"} */
    while (ARG_CNT(args)) {

        token = ARG_GET(args);

        /* skip command options */
        if (*token == '-')     { continue; }
           
        if (sal_strlen(token) > 2) {
            if (*token == '0' && (*(token+1) == 'x' || *(token+1) == 'X')) {
                token += 2;
            }
        }
        
        is_odd = (sal_strlen(token) & 1);
        if (is_odd) {
            hex_byte_str[3] = *token++;
            buffer[byte_pos++] = _shr_ctoi(hex_byte_str);
        }

        while (*token) {
            hex_byte_str[2] = *token++;
            hex_byte_str[3] = *token++;
            buffer[byte_pos++] = _shr_ctoi(hex_byte_str);

            if (byte_pos >= (buffer_max - 1)) {
                printk("increase buffer_max: %d\n", buffer_max);
                sal_free(buffer);
                return CMD_FAIL;
            }
        }
    }

    if (byte_pos == 0) {
        sal_free(buffer);
        return CMD_USAGE;
    }

    printk("\n\n Format with sbx packet class:\n");

    /* Format with sbx packet class */
    pkt = sbxpkt_alloc();
    from_byte(pkt_type, buffer, byte_pos, pkt);

    sbxpkt_print(pkt);
    sbxpkt_free(pkt);
    sal_free(buffer);

    return CMD_OK;
}
#endif /* INCLUDE_TEST */


char cmd_sbx_fe2000_nic_config_usage[] =
"Usage:\n"
"nicconfig <options>\n"
"  Configure the `steering' of the FE-2000 PCI CPU packet port\n"
"  ingress  - loop PCI packets through the ingress microcode"
"  engress  - loop PCI packets through the eg2ress microcode"
"  free     - send packets from PCI into the system\n"
;

cmd_result_t
cmd_sbx_fe2000_nic_config(int unit, args_t *a)
{
    sbhandle sbh;
    int ingress = -1;
    int egress = -1;
    int free = -1;
    int found, ihppq, isysq, ehppq, efrontq;
    int i, j;
    uint32_t qn;
    sbZfFe2000PtQueueToPortEntry_t q2p;
    sbZfC2PtQueueToPortEntry_t q2p_c2;

    if (!SOC_IS_SBX_FE2000(unit)){
        printk("only supported on FE-2000\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "ingress", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ingress, NULL);
        parse_table_add(&pt, "egress", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &egress, NULL);
        parse_table_add(&pt, "free", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &free, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (ingress + egress + free > -1) {
        return CMD_USAGE;
    }

    sbh = SOC_SBX_CONTROL(unit)->sbhdl;

    if (!SOC_IS_SBX_FE2KXT(unit)){
        ihppq = SOC_SBX_CFG_FE2000(unit)
            ->init_params.queues.port2iqid[CMIC_PORT(unit)];
        isysq = ihppq + 128;
        ehppq = SOC_SBX_CFG_FE2000(unit)
            ->init_params.queues.port2eqid[CMIC_PORT(unit)];
        efrontq = ehppq + 128;

        if (ingress == 1) {
            CaPtQueueToPortRead(sbh, ihppq, &q2p);
            q2p.m_uDestInterface = SB_FE2000_IF_PTYPE_PCI;
            q2p.m_uDestPort = 0;
            CaPtQueueToPortWrite(sbh, ihppq, &q2p);
            CaPtQueueToPortRead(sbh, isysq, &q2p);
            q2p.m_uDestInterface = SB_FE2000_IF_PTYPE_PCI;
            q2p.m_uDestPort = 0;
            CaPtQueueToPortWrite(sbh, isysq, &q2p);
            CaPrPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, ihppq);
            CaPtPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, isysq);
        } else if (egress == 1) {
            CaPrPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, ehppq);
            CaPtPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, efrontq);
        } else if (free == 1) {
            found = 0;
            for (i = SB_FE2000_IF_PTYPE_SPI0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
                for (j = 0; j < _ports_per_iface[i] && !found; j++) {
                    CaPtPortToQueueRead(sbh, i, j, &qn);
                    if (qn == isysq) {
                        found = 1;
                        break;
                    }
                }
                if (found) {
                    break;
                }
            }
            if (!found) {
                printk("couldn't find system-side queue for PCI\n");
                return CMD_FAIL;
            }
            CaPtQueueToPortRead(sbh, ihppq, &q2p);
            q2p.m_uDestInterface = i;
            q2p.m_uDestPort = j;
            CaPtQueueToPortWrite(sbh, ihppq, &q2p);
            CaPtQueueToPortRead(sbh, isysq, &q2p);
            q2p.m_uDestInterface = i;
            q2p.m_uDestPort = j;
            CaPtQueueToPortWrite(sbh, isysq, &q2p);
            CaPrPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, ihppq);
            CaPtPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, efrontq);
        } else {
            CaPrPortToQueueRead(sbh, SB_FE2000_IF_PTYPE_PCI, 0, &qn);
            if (qn == ehppq) {
                printk("PCI NIC is currently captive to egress\n");
            } else {
                CaPtPortToQueueRead(sbh, SB_FE2000_IF_PTYPE_PCI, 0, &qn);
                if (qn == isysq) {
                    printk("PCI NIC is currently captive to ingress\n");
                } else {
                    printk("PCI NIC currently connects to the system\n");
                }
            }
        }
    }else{ /* FE2KXT */
        ihppq = SOC_SBX_CFG_FE2000(unit)
            ->xt_init_params.queues.port2iqid[CMIC_PORT(unit)];
        isysq = ihppq + 128;
        ehppq = SOC_SBX_CFG_FE2000(unit)
            ->xt_init_params.queues.port2eqid[CMIC_PORT(unit)];
        efrontq = ehppq + 128;

        if (ingress == 1) {
            C2PtQueueToPortRead(sbh, ihppq, &q2p_c2);
            q2p_c2.m_uDestInterface = SB_FE2000_IF_PTYPE_PCI;
            q2p_c2.m_uDestPort = 0;
            C2PtQueueToPortWrite(sbh, ihppq, &q2p_c2);
            C2PtQueueToPortRead(sbh, isysq, &q2p_c2);
            q2p_c2.m_uDestInterface = SB_FE2000_IF_PTYPE_PCI;
            q2p_c2.m_uDestPort = 0;
            C2PtQueueToPortWrite(sbh, isysq, &q2p_c2);
            C2PrPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, ihppq);
            C2PtPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, isysq);
        } else if (egress == 1) {
            C2PrPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, ehppq);
            C2PtPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, efrontq);
        } else if (free == 1) {
            found = 0;
            for (i = SB_FE2000_IF_PTYPE_SPI0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
                for (j = 0; j < _ports_per_iface[i] && !found; j++) {
                    C2PtPortToQueueRead(sbh, i, j, &qn);
                    if (qn == isysq) {
                        found = 1;
                        break;
                    }
                }
                if (found) {
                    break;
                }
            }
            if (!found) {
                printk("couldn't find system-side queue for PCI\n");
                return CMD_FAIL;
            }
            C2PtQueueToPortRead(sbh, ihppq, &q2p_c2);
            q2p_c2.m_uDestInterface = i;
            q2p_c2.m_uDestPort = j;
            C2PtQueueToPortWrite(sbh, ihppq, &q2p_c2);
            C2PtQueueToPortRead(sbh, isysq, &q2p_c2);
            q2p_c2.m_uDestInterface = i;
            q2p_c2.m_uDestPort = j;
            C2PtQueueToPortWrite(sbh, isysq, &q2p_c2);
            C2PrPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, ihppq);
            C2PtPortToQueueWrite(sbh, SB_FE2000_IF_PTYPE_PCI, 0, efrontq);
        } else {
            C2PrPortToQueueRead(sbh, SB_FE2000_IF_PTYPE_PCI, 0, &qn);
            if (qn == ehppq) {
                printk("PCI NIC is currently captive to egress\n");
            } else {
                C2PtPortToQueueRead(sbh, SB_FE2000_IF_PTYPE_PCI, 0, &qn);
                if (qn == isysq) {
                    printk("PCI NIC is currently captive to ingress\n");
                } else {
                    printk("PCI NIC currently connects to the system\n");
                }
            }
        }
    }

    return CMD_OK;
}

char cmd_sbx_fe2000_check_mem_usage[] =
"Usage:\n"
"checkmem\n"
"  check & clean memory error flags\n"
;

cmd_result_t
cmd_sbx_fe2000_check_mem(int unit, args_t *a)
{
    sbhandle sbh = SOC_SBX_CONTROL(unit)->sbhdl;

    if (soc_sbx_fe2000_check_mem(unit)) {
        SAND_HAL_FE2000_WRITE(sbh, MM0_ERROR, SAND_HAL_FE2000_READ(sbh, MM0_ERROR));
        SAND_HAL_FE2000_WRITE(sbh, MM1_ERROR, SAND_HAL_FE2000_READ(sbh, MM1_ERROR));
    }
    return CMD_OK;
}


#define BCM_FE2000_MAX_POLICERS         0x10000 /* 64K */
#define KBITS_TO_BYTES(kbits) (kbits*125)

#ifndef SB_G2_FE_MAX_POLICER_IDX
#define SB_G2_FE_MAX_POLICER_IDX  (65536)
#endif

int
_bcm_policer_print(int unit, bcm_policer_t id,
                   bcm_policer_config_t *info, void *cookie_unused)
{
    int rv = BCM_E_NONE;

    if ((id < 0) || (id > SB_G2_FE_MAX_POLICER_IDX) || (info == NULL)) {
        rv = BCM_E_PARAM;
    }

    if (rv == BCM_E_NONE) {
        /* now display the policer config */
        printk("BCM Policer id - %d \n", id);
        printk("    flags:      0x%x -> ", info->flags);
        if (info->flags) {
            if (info->flags & BCM_POLICER_WITH_ID)
                printk("BCM_POLICER_WITH_ID ");
            if (info->flags & BCM_POLICER_REPLACE)
                printk("BCM_POLICER_REPLACE ");
            if (info->flags & BCM_POLICER_COLOR_BLIND)
                printk("BCM_POLICER_COLOR_BLIND ");
            if (info->flags & BCM_POLICER_DROP_RED)
                printk("BCM_POLICER_DROP_RED ");
            printk(" \n");
        } else {
            printk("EMPTY\n");
        }
        printk("    Mode:       ");
        switch (info->mode) {
            case bcmPolicerModeSrTcm: printk("SrTcm \n"); break;
            case bcmPolicerModeTrTcm: printk("TrTcm \n"); break;
            case bcmPolicerModeTrTcmDs: printk("TrTcmDs \n"); break;
            case bcmPolicerModePassThrough: printk("PassThrough \n"); break;
            default: printk("Invalid \n"); break;
        }
        printk("    Committed:  Burst - %dB, CIR - %dkbps \n",
               KBITS_TO_BYTES(info->ckbits_burst), info->ckbits_sec);
        printk("    Excess:     Burst - %dB, EIR - %dkbps \n",
               KBITS_TO_BYTES(info->pkbits_burst), info->pkbits_sec);
        printk("    Current:    Rate - %dkbps \n", info->kbits_current);
    }
    return rv;
}

#define CMD_POL_INIT_USAGE \
    "\n" \
    "   policer init\n"
    
#define CMD_POL_LIST_USAGE \
    "   policer list id=<policerid> \n" \
    "   policer list all \n"

#define CMD_POL_GROUP_USAGE \
    "   policer group rate get group=<0-8>\n" \
    "   policer group rate set group=<0-8> rate=<[1/10/100/250/500]KBPS, [1/10/100]MBPS, [1/10]GBPS>\n"
    
#define CMD_POL_ADD_USAGE \
    "   policer add id=<policerid> flags=<hex value bitmap> mode=<mode> cbs=<cbs> cir=<cir> ebs=<ebs> eir=<eir>\n" \
    "           flags bits  : 0x4 (BCM_POLICER_COLOR_BLIND), 0x8 (BCM_POLICER_DROP_RED) \n" \
    "           modes       : 0 (bcmPolicerModeSrTcm), 3 (bcmPolicerModeTrTcm), \n" \
    "                         4 (bcmPolicerModeTrTcmDs), 6 (bcmPolicerModePassThrough) \n" \
    "           cbs & ebs   : kbits \n" \
    "           cir & eir   : kbits/sec \n"
    
#define CMD_POL_REM_USAGE \
    "   policer remove id=<policerid>\n" \
    "   policer remove all\n"

#define CMD_POL_STAT_USAGE \
    "   policer stat enable  id=<policerid>\n" \
    "   policer stat disable id=<policerid>\n" \
    "   policer stat get     id=<policerid> [cos=<level>] [type=<stat type>]\n" \
    "   policer stat set     id=<policerid> [cos=<level>] [type=<stat type>] value=<stat value> \n" \
    "       stat type - if not specified, applies to all valid types for this policer \n" \
    "       stat type - 1-49  1-bcmPolicerStatGreenToGreenBytes\n" \
    "                         2-bcmPolicerStatGreenToYellowPackets\n" \
    "                         ...look at bcm_policer_stat_t enum for full list>\n"
    
#define CMD_POL_OAM_USAGE \
    "   policer oam timer add    id=<timerid> deadline=<time> started=<0|1> strict=<0|1> reset=<0|1> \n" \
    "                                         interrupt=<0|1> rate=<[1/10/100/250]KBPS, [1/10/100]MBPS, [1/10]GBPS>\n" \
    "   policer oam timer remove id=<timerid> \n" \
    "   policer oam timer remove all \n" \
    "   policer oam timer list   id=<timerid> \n" \
    "   policer oam timer list   all \n" \
    "   policer oam sequencegenerator add     id=<seq gen id> rate=<[1/10/100/250]KBPS, [1/10/100]MBPS, [1/10]GBPS>\n" \
    "   policer oam sequencegenerator remove  id=<seq gen id> \n" \
    "   policer oam sequencegenerator remove  all \n"

#define CMD_POL_MON_USAGE \
    "   policer monitor attach id=<policerid>\n" \
    "   policer monitor detach id=<policerid>\n" \
    "   policer monitor display id=<policerid>\n"
    
char cmd_sbx_policer_usage [] = 
    CMD_POL_INIT_USAGE
    CMD_POL_LIST_USAGE
    CMD_POL_GROUP_USAGE
    CMD_POL_ADD_USAGE
    CMD_POL_REM_USAGE
    CMD_POL_STAT_USAGE
    CMD_POL_OAM_USAGE
    CMD_POL_MON_USAGE
    ;

int
_bcm_sbx_policer_cos_level_get(int unit, bcm_policer_t pol_id, int *cos)
{
    int                         rv = BCM_E_NONE;
    bcm_policer_group_mode_t    grp_mode;

    *cos = 1;
    if (SOC_IS_SBX_G2P3(unit)) {
        rv = _bcm_fe2000_policer_group_mode_get(unit, pol_id, &grp_mode); 
        if ((rv == BCM_E_NONE) && 
            (grp_mode == bcmPolicerGroupModeTypedIntPri)) {
            BCM_IF_ERROR_RETURN(bcm_cosq_config_get(unit, cos));
        }
    }
    return rv;
}

int 
_bcm_sbx_policer_stat_valid(int unit, bcm_policer_t pol_id, int stat_type)
{
    int                         rv = BCM_E_NONE;
    bcm_policer_group_mode_t    grp_mode;
    int                         temp;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if ((stat_type < bcmPolicerStatPackets) || 
            (stat_type > bcmPolicerStatRedBytes)) {
            rv = BCM_E_PARAM;
        } else {
            rv = _bcm_fe2000_policer_group_mode_get(unit, pol_id, &grp_mode); 
            if (rv == BCM_E_NONE) {
                rv = _bcm_fe2000_g2p3_policer_stat_mem_get(unit, grp_mode, 
                                                           stat_type, 0, &temp, &temp);
            }
        }
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        rv = BCM_E_PARAM;
    }

    return rv;
}

int
_bcm_sbx_policer_stat_display(int unit, int stat_type, uint64_t stat_value)
{
    int rv = CMD_OK;

    switch (stat_type) {
    case bcmPolicerStatGreenToGreenPackets: 
        printk("Green To Green Packets:     0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenToGreenBytes: 
        printk("Green To Green Bytes:       0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenToYellowPackets: 
        printk("Green To Yellow Packets:    0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenToYellowBytes: 
        printk("Green To Yellow Bytes:      0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenToRedPackets: 
        printk("Green To Red Packets:       0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenToRedBytes: 
        printk("Green To Red Bytes:         0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenToDropPackets: 
        printk("Green To Drop Packets:      0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenToDropBytes: 
        printk("Green To Drop Bytes:        0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToGreenPackets: 
        printk("Yellow To Green Packets:    0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToGreenBytes: 
        printk("Yellow To Green Bytes:      0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToYellowPackets: 
        printk("Yellow To Yellow Packets:   0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToYellowBytes: 
        printk("Yellow To Yellow Bytes:     0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToRedPackets: 
        printk("Yellow To Red Packets:      0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToRedBytes: 
        printk("Yellow To Red Bytes:        0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToDropPackets: 
        printk("Yellow To Drop Packets:     0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowToDropBytes: 
        printk("Yellow To Drop Bytes:       0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToGreenPackets: 
        printk("Red To Green Packets:       0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToGreenBytes: 
        printk("Red To Green Bytes:         0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToYellowPackets: 
        printk("Red To Yellow Packets:      0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToYellowBytes: 
        printk("Red To Yellow Bytes:        0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToRedPackets: 
        printk("Red To Red Packets:         0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToRedBytes: 
        printk("Red To Red Bytes:           0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToDropPackets: 
        printk("Red To Drop Packets:        0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedToDropBytes: 
        printk("Red To Drop Bytes:          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatPackets:
        printk("Policed Packets             0x%llx \n", stat_value);
        break;
    case bcmPolicerStatBytes:
        printk("Policed Bytes               0x%llx \n", stat_value);
        break;
    case bcmPolicerStatUnknownUnicastPackets:
        printk("Policed Unknown UC Packets  0x%llx \n", stat_value);
        break;
    case bcmPolicerStatUnknownUnicastBytes:
        printk("Policed Unknown UC Bytes    0x%llx \n", stat_value);
        break;
    case bcmPolicerStatUnicastPackets:
        printk("Policed UC Packets          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatUnicastBytes:
        printk("Policed UC Bytes            0x%llx \n", stat_value);
        break;
    case bcmPolicerStatMulticastPackets:
        printk("Policed MC Packets          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatMulticastBytes:
        printk("Policed MC Bytes            0x%llx \n", stat_value);
        break;
    case bcmPolicerStatBroadcastPackets:
        printk("Policed BC Packets          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatBroadcastBytes:
        printk("Policed BC Bytes            0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropPackets:
        printk("Dropped Packets             0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropBytes:
        printk("Dropped Bytes               0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropUnknownUnicastPackets:
        printk("Dropped Unknown UC Packets  0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropUnknownUnicastBytes:
        printk("Dropped Unknown UC Bytes    0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropUnicastPackets:
        printk("Dropped UC Packets          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropUnicastBytes:
        printk("Dropped UC Bytes            0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropMulticastPackets:
        printk("Dropped MC Packets          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropMulticastBytes:
        printk("Dropped MC Bytes            0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropBroadcastPackets:
        printk("Dropped BC Packets          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatDropBroadcastBytes:
        printk("Dropped BC Bytes            0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenPackets:
        printk("Green marked Packets        0x%llx \n", stat_value);
        break;
    case bcmPolicerStatGreenBytes:
        printk("Green marked Bytes          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowPackets:
        printk("Yellow marked Packets       0x%llx \n", stat_value);
        break;
    case bcmPolicerStatYellowBytes:
        printk("Yellow marked Bytes         0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedPackets:
        printk("Red marked Packets          0x%llx \n", stat_value);
        break;
    case bcmPolicerStatRedBytes:
        printk("Red marked Bytes            0x%llx \n", stat_value);
        break;
    default:
        rv = CMD_FAIL;
        break;
    }
    return rv;
}

int
_cmd_sbx_pm_get(int unit, sbFe2000Pm_t **ppPm)
{
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_state_t *g2p3_st;
#endif

    if (!ppPm) {
        return BCM_E_PARAM;
    }
    *ppPm = NULL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        g2p3_st = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
        if (!g2p3_st) {
            return BCM_E_INTERNAL;
        }
        *ppPm = g2p3_st->pm;
        return BCM_E_NONE;
        break;
#endif   /* BCM_FE2000_P3_SUPPORT */
    default:
        return BCM_E_INTERNAL;
    }

}

cmd_result_t
_cmd_pol_init(int unit, args_t *args)
{
    if (bcm_policer_init(unit) != BCM_E_NONE) {
        return CMD_FAIL;
    }

    return CMD_OK;
}

cmd_result_t
_cmd_pol_list(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    bcm_policer_config_t    _pol_cfg;
    bcm_policer_t           _polid = -1;

    bcm_policer_config_t_init(&_pol_cfg);

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    if (!sal_strncasecmp(param, "all", 3)) {
        param = ARG_GET(args);
        rv = bcm_policer_traverse(unit, _bcm_policer_print, NULL);
        if (rv != BCM_E_NONE) {
            printk("Internal Error displaying policer info. \n");
            return CMD_FAIL;
        }
    } else {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "id", PQ_DFL|PQ_INT, 0, &_polid, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        if (_polid < 0) {
            printk("Invalid policer id specified\n");
            return CMD_FAIL;
        }
        if (bcm_policer_get(unit, _polid, &_pol_cfg) != BCM_E_NONE) {
            printk("Invalid policer id (%d) specified. \n", _polid);
            return CMD_FAIL;
        } else if (_bcm_policer_print(unit, _polid, &_pol_cfg, NULL) 
                   != BCM_E_NONE) {
            printk("Internal Error displaying policer info. \n");
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}

cmd_result_t
_cmd_pol_group_rate_get(int unit, args_t *args)
{
    int                     group = -1;
    parse_table_t           pt;
    cmd_result_t            retCode;
    sbStatus_t              sbsts;
    sbFe2000PmGroupConfig_t zGroupConfig;
    sbFe2000Pm_t            *pPm;

    if (_cmd_sbx_pm_get(unit, &pPm) != BCM_E_NONE) {
        printk("Internal error. \n");
        return CMD_FAIL;
    }
    sal_memset(&zGroupConfig, 0, sizeof(sbFe2000PmGroupConfig_t));

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "group", PQ_DFL|PQ_INT, 0, &group, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if ((group < 0) || (group >= SB_FE_2000_PM_NUM_OF_GROUPS)) {
        printk("Invalid group specified. \n");
        return CMD_FAIL;
    }

    sbsts = sbFe2000PmPolicerGroupRead(pPm, group, &zGroupConfig);
    if(sbsts != SB_OK) {
        printk("%s: sbFe2000PmPolicerGroupRead failed. \n",FUNCTION_NAME());
        return CMD_FAIL;
    }
    printk("\nGroup %d  Base rate is ", group);
    switch (zGroupConfig.uRate) {
    case SB_FE_2000_PM_PRATE_1KBPS:     printk("1KBPS");    break;
    case SB_FE_2000_PM_PRATE_10KBPS:    printk("10KBPS");   break;
    case SB_FE_2000_PM_PRATE_100KBPS:   printk("100KBPS");  break;
    case SB_FE_2000_PM_PRATE_250KBPS:   printk("250KBPS");  break;
    case SB_FE_2000_PM_PRATE_500KBPS:   printk("500KBPS");  break;
    case SB_FE_2000_PM_PRATE_1MBPS:     printk("1MBPS");    break;
    case SB_FE_2000_PM_PRATE_10MBPS:    printk("10MBPS");   break;
    case SB_FE_2000_PM_PRATE_100MBPS:   printk("100MBPS");  break;
    case SB_FE_2000_PM_PRATE_1GBPS:     printk("1GBPS");    break;
    case SB_FE_2000_PM_PRATE_10GBPS:    printk("10GBPS");   break;
    default:                            printk("INVALID");  break;
    }
    printk(" \n");
    /* SOCK_LOG_ZF(sbZfFe2000PmGroupConfig, &(zGroupConfig.groupConfig)); */
    return CMD_OK;
}

int
_cmd_pol_rate_valid(char *rate, int *sb_rate)
{
    int r_len;

    if (!rate || !sb_rate) {
        return BCM_E_PARAM;
    }
    r_len = strlen(rate);
    if (!r_len) {
        return BCM_E_PARAM;
    }

    if (!sal_strncasecmp("1KBPS", rate, (r_len > 5 ? 5 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_1KBPS;
    } else if (!sal_strncasecmp("10KBPS", rate, (r_len > 6 ? 6 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_10KBPS;
    } else if (!sal_strncasecmp("100KBPS", rate, (r_len > 7 ? 7 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_100KBPS;
    } else if (!sal_strncasecmp("250KBPS", rate, (r_len > 7 ? 7 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_250KBPS;
    } else if (!sal_strncasecmp("500KBPS", rate, (r_len > 7 ? 7 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_500KBPS;
    } else if (!sal_strncasecmp("1MBPS", rate, (r_len > 5 ? 5 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_1MBPS;
    } else if (!sal_strncasecmp("10MBPS", rate, (r_len > 6 ? 6 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_10MBPS;
    } else if (!sal_strncasecmp("100MBPS", rate, (r_len > 7 ? 7 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_100MBPS;
    } else if (!sal_strncasecmp("1GBPS", rate, (r_len > 5 ? 5 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_1GBPS;
    } else if (!sal_strncasecmp("10GBPS", rate, (r_len > 6 ? 6 : r_len))) {
        *sb_rate = SB_FE_2000_PM_PRATE_10GBPS;
    } else {
        return BCM_E_PARAM;
    }


    return BCM_E_NONE;
}

cmd_result_t
_cmd_pol_group_rate_set(int unit, args_t *args)
{
    int                     group = -1, sb_rate;
    char                    *rate;
    parse_table_t           pt;
    sbStatus_t              sbsts;
    sbFe2000Pm_t            *pPm;
    
    if (_cmd_sbx_pm_get(unit, &pPm) != BCM_E_NONE) {
        printk("Internal error. \n");
        return CMD_FAIL;
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "group", PQ_DFL|PQ_INT, 0, &group, NULL);
    parse_table_add(&pt, "rate", PQ_STRING, 0, &rate, NULL);
    if (!parse_arg_eq(args, &pt)) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    if ((group < 0) || (group >= SB_FE_2000_PM_NUM_OF_GROUPS)) {
        printk("Invalid group specified. \n");
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    if (_cmd_pol_rate_valid(rate, &sb_rate) != BCM_E_NONE) {
        printk("Invalid rate (%s) specified. \n", rate);
        parse_arg_eq_done(&pt);
        return CMD_USAGE;
    }

    sbsts = sbFe2000PmPolicerGroupRateChange(pPm, group, sb_rate);
    if (sbsts != SB_OK) {
        printk("%s: sbFe2000PmPolicerGroupRateChange failed. \n",
               FUNCTION_NAME());
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    printk("Group %d Base rate changed to %s \n", group, rate);
    parse_arg_eq_done(&pt);
    return CMD_OK;
}

cmd_result_t
_cmd_pol_group(int unit, args_t *args)
{
    char                    *subcmd;
    
    subcmd = ARG_GET(args);
    if ((!subcmd) || (sal_strcasecmp(subcmd, "rate"))) {
        return CMD_USAGE;
    }
    subcmd = ARG_GET(args);
    if (!subcmd || !ARG_CUR(args)) {
        return CMD_USAGE;
    }

     if (!sal_strcasecmp(subcmd, "set")) {
         return _cmd_pol_group_rate_set(unit, args);
     } else if (!sal_strcasecmp(subcmd, "get")) {
         return _cmd_pol_group_rate_get(unit, args);
     } 
     
     return CMD_USAGE;
}

cmd_result_t
_cmd_pol_add(int unit, args_t *args)
{
    parse_table_t           pt;
    bcm_policer_config_t    pol_cfg;
    bcm_policer_t           polid = -1;
    cmd_result_t            retCode;
    int                     rv;

    bcm_policer_config_t_init(&pol_cfg);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "id", PQ_DFL|PQ_INT, 0, &polid, NULL);
    parse_table_add(&pt, "flags", PQ_DFL|PQ_INT, 0, &pol_cfg.flags, NULL);
    parse_table_add(&pt, "mode", PQ_DFL|PQ_INT, 0, &pol_cfg.mode, NULL);
    parse_table_add(&pt, "cbs", PQ_DFL|PQ_INT, 0, &pol_cfg.ckbits_burst, NULL);
    parse_table_add(&pt, "cir", PQ_DFL|PQ_INT, 0, &pol_cfg.ckbits_sec, NULL);
    parse_table_add(&pt, "ebs", PQ_DFL|PQ_INT, 0, &pol_cfg.pkbits_burst, NULL);
    parse_table_add(&pt, "eir", PQ_DFL|PQ_INT, 0, &pol_cfg.pkbits_sec, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (polid >= 0) {
        pol_cfg.flags |= BCM_POLICER_WITH_ID;
    }

    if ((rv = bcm_policer_create(unit, &pol_cfg, &polid)) != BCM_E_NONE) {
        printk("Policer add failed. (%s) \n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (!(pol_cfg.flags & BCM_POLICER_WITH_ID)) {
        printk("Policer created with id: %d \n", polid);
    }
    
    return CMD_OK;
}

cmd_result_t
_cmd_pol_rem(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    bcm_policer_t           polid = -1;

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    if (!sal_strncasecmp(param, "all", 3)) {
        param = ARG_GET(args);
        if ((rv = bcm_policer_destroy_all(unit)) != BCM_E_NONE) {
            printk("ERROR: bcm_policer_destroy_all(unit=%d) failed.(%s) \n",
                   unit, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "id", PQ_DFL|PQ_INT, 0, &polid, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        if (polid < 0) {
            printk("Invalid policer id specified\n");
            return CMD_FAIL;
        }
        if ((rv = bcm_policer_destroy(unit, polid)) != BCM_E_NONE) {
            printk("ERROR: bcm_policer_destroy(unit=%d, id=%d) failed (%s) \n",
                   unit, polid, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}

cmd_result_t
_cmd_pol_stat(int unit, args_t *args)
{
    char            *subcmd;
    int             rv;
    parse_table_t   pt;
    cmd_result_t    retCode;
    bcm_policer_t   polid = -1;
    int             cos_cfg = -1, type = -1, value = -1;
    int             stat_type_min, stat_type_max, cos_min, cos_max, idx, cos;
    int             sc_len;
    uint64_t        stat_value;

    subcmd = ARG_GET(args);
    if (!subcmd || !ARG_CUR(args)) {
        return CMD_USAGE;
    }
    sc_len = strlen(subcmd);
    COMPILER_64_SET(stat_value, 0, 0);

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "id", PQ_DFL|PQ_INT, 0, &polid, NULL);
    parse_table_add(&pt, "cos", PQ_DFL|PQ_INT, 0, &cos_cfg, NULL);
    parse_table_add(&pt, "type", PQ_DFL|PQ_INT, 0, &type, NULL);
    parse_table_add(&pt, "value", PQ_DFL|PQ_INT, 0, &value, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (polid < 0) {
        return CMD_USAGE;
    }
    
    if (!sal_strncasecmp(subcmd, "enable", (sc_len>6 ? 6:sc_len))) {
        if ((rv = bcm_policer_stat_enable_set(unit, polid, 1)) != BCM_E_NONE) {
            printk("Stat enable for policer id %d failed (rv=%d: %s) \n",
                   polid, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (!sal_strncasecmp(subcmd, "disable", (sc_len>7 ? 7:sc_len))) {
        if ((rv = bcm_policer_stat_enable_set(unit, polid, 0)) != BCM_E_NONE) {
            printk("Stat disable for policer id %d failed (rv=%d: %s) \n",
                   polid, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (!sal_strncasecmp(subcmd, "get", (sc_len>3 ? 3:sc_len))) {
        if (type < 0) {
            stat_type_min = 1;
            stat_type_max = bcmPolicerStatCount - 1;
        } else {
            stat_type_min = stat_type_max = type;
        }

        if ((stat_type_min < 1) || 
            (stat_type_max >= bcmPolicerStatCount)) {
            printk("Invalid stat_type specified (%d) \n", type);
            return CMD_USAGE;
        }

        for (idx=stat_type_min; idx<=stat_type_max; idx++) {
            cos_min = 0;
            cos_max = 1;
            if ((idx >= bcmPolicerStatGreenPackets) && 
                (idx <= bcmPolicerStatRedBytes)) {
                if (_bcm_sbx_policer_cos_level_get(unit, polid, &cos_max) 
                    != BCM_E_NONE) {
                    return CMD_FAIL;
                }
            } 
            if (cos_cfg >= 0) {
                /* if provided, use the cos level */
                cos_min = cos_cfg;
                cos_max = cos_cfg + 1;
            }
            for (cos = cos_min; cos < cos_max; cos++) {
                if (_bcm_sbx_policer_stat_valid(unit, polid, idx) 
                    == BCM_E_NONE) {
                    if (bcm_policer_stat_get(unit, polid, cos, idx, &stat_value) 
                        != BCM_E_NONE) {
                        return CMD_FAIL;
                    }
                    if (((cos_max > 1) || (cos_cfg >= 0)) &&
                        ((idx >= bcmPolicerStatGreenPackets) && 
                         (idx <= bcmPolicerStatRedBytes))) {
                        printk("Cos %d: ", cos);
                    }
                    if (_bcm_sbx_policer_stat_display(unit, idx, stat_value) 
                        != CMD_OK) {
                        return CMD_FAIL;
                    }
                }
            }
        }
    } else if (!sal_strncasecmp(subcmd, "set", (sc_len>3 ? 3:sc_len))) {
        if (value) {
            printk("stat value can only be 0 \n");
            return CMD_FAIL;
        }
        if (type < 0) {
            stat_type_min = 1;
            stat_type_max = bcmPolicerStatCount - 1;
        } else {
            stat_type_min = stat_type_max = type;
        }
        if ((stat_type_min < 1) || 
            (stat_type_max >= bcmPolicerStatCount)) {
            printk("Invalid stat_type specified (%d) \n", type);
            return CMD_USAGE;
        }

        for (idx=stat_type_min; idx<=stat_type_max; idx++) {
            cos_min = 0;
            cos_max = 1;
            if ((idx >= bcmPolicerStatGreenPackets) && 
                (idx <= bcmPolicerStatRedBytes)) {
                if (_bcm_sbx_policer_cos_level_get(unit, polid, &cos_max) 
                    != BCM_E_NONE) {
                    return CMD_FAIL;
                }
            } 
            if (cos_cfg >= 0) {
                /* if provided, use the cos level */
                cos_min = cos_cfg;
                cos_max = cos_cfg + 1;
            }
            for (cos = cos_min; cos < cos_max; cos++) {
                if (_bcm_sbx_policer_stat_valid(unit, polid, idx) 
                    == BCM_E_NONE) {
                    if (bcm_policer_stat_set(unit, polid, cos, idx, value) 
                        != BCM_E_NONE) {
                        return CMD_FAIL;
                    }
                }
            }
        }
    } else {
        printk("Sub-Command (%s) invalid. \n", subcmd);
        return CMD_USAGE;
    }

    return CMD_OK;
}

cmd_result_t
_cmd_pol_oam_timer(int unit, args_t *args)
{
    char    *subcmd, *rate, *temp;
    int     sc_len, id = -1, deadline = -1, started = -1, strict = -1;
    int     reset = -1, intr = -1, sb_rate, sb_sts;
    int     min, max, idx, parsed = 1;
    uint32_t                        id_ret;
    parse_table_t                   pt;
    sbZfFe2000PmOamTimerConfig_t    zTimer;
    sbFe2000Pm_t                    *pPm;

    sbZfFe2000PmOamTimerConfig_InitInstance(&zTimer);

    if (_cmd_sbx_pm_get(unit, &pPm) != BCM_E_NONE) {
        printk("Internal error. \n");
        return CMD_FAIL;
    }

    subcmd = ARG_GET(args);
    if (!subcmd || !ARG_CUR(args)) {
        return CMD_USAGE;
    }
    sc_len = strlen(subcmd);
    if (!sc_len) {
        return CMD_USAGE;
    }

    if (!sal_strncasecmp(subcmd, "add", (sc_len > 3? 3: sc_len))) {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "id",          PQ_DFL|PQ_INT, 0, &id,       NULL);
        parse_table_add(&pt, "deadline",    PQ_DFL|PQ_INT, 0, &deadline, NULL);
        parse_table_add(&pt, "started",     PQ_DFL|PQ_INT, 0, &started,  NULL);
        parse_table_add(&pt, "strict",      PQ_DFL|PQ_INT, 0, &strict,   NULL);
        parse_table_add(&pt, "reset",       PQ_DFL|PQ_INT, 0, &reset,    NULL);
        parse_table_add(&pt, "interrupt",   PQ_DFL|PQ_INT, 0, &intr,     NULL);
        parse_table_add(&pt, "rate",        PQ_STRING,     0, &rate,     NULL);
        if (!parse_arg_eq(args, &pt)) {
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        if (_cmd_pol_rate_valid(rate, &sb_rate) != BCM_E_NONE) {
            if (rate && (strlen(rate) != 0)) {
                printk("Invalid rate (%s) specified. \n", rate);
            } else {
                printk("Rate not specified. \n");
            }
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        if ((id < 0) || (deadline < 0) || (started < 0) || (strict < 0) ||
            (reset < 0) || (intr < 0)) {
            printk("One or more input params missing for \"time add\" OAM "
                   "sub-command\n");
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        sbZfFe2000PmOamTimerConfig_InitInstance(&zTimer);

        zTimer.uDeadline = deadline;
        zTimer.uRate = sb_rate;
        zTimer.bStrict = strict;
        zTimer.bReset = reset;
        zTimer.bStarted = started;
        zTimer.bInterrupt = intr;

        sb_sts = sbFe2000PmOamTimerCreate(pPm, (uint32_t)id, &zTimer, &id_ret);
        if (sb_sts != SB_OK) {
            printk("sbFe2000PmOamTimerCreate failed. \n");
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        printk("Timer Id - %d created successfully\n", id);
    } else if (!sal_strncasecmp(subcmd, "remove", (sc_len > 6? 6: sc_len))) {
        temp = ARG_CUR(args);
        sc_len = strlen(temp);
        if (!sal_strncasecmp(temp, "all", (sc_len > 3? 3: sc_len))) {
            printk("TODO: remove all not supported yet. \n");
            return CMD_FAIL;
        } else {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "id",          PQ_DFL|PQ_INT, 0, &id,       NULL);
            if (!parse_arg_eq(args, &pt)) {
                parse_arg_eq_done(&pt);
                return CMD_FAIL;
            }
            if (id < 0) {
                printk("Timer id not specified. \n");
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
            min = id;
            max = id + 1;
        }
        for(idx = min; idx < max; idx++) {
            sb_sts = sbFe2000PmOamTimerDelete(pPm, (uint32_t)idx);
            if(sb_sts != SB_OK) {
                printk("sbFe2000PmOamTimerDelete failed. \n");
                parse_arg_eq_done(&pt);
                return CMD_FAIL;
            }
            printk("Timer %d Deleted\n", idx);
        }
    } else if (!sal_strncasecmp(subcmd, "list", (sc_len > 4? 4: sc_len))) {
        temp = ARG_CUR(args);
        sc_len = strlen(temp);
        if (!sal_strncasecmp(temp, "all", (sc_len > 3? 3: sc_len))) {
            min = 0;
            max = SB_FE_2000_MAX_POLICER_IDX;
            parsed = 0;
        } else {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "id",          PQ_DFL|PQ_INT, 0, &id,       NULL);
            if (!parse_arg_eq(args, &pt)) {
                parse_arg_eq_done(&pt);
                return CMD_FAIL;
            }
            if (id < 0) {
                printk("Timer id not specified. \n");
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
            min = id;
            max = id + 1;
        }
        for(idx = min; idx < max; idx++) {
            sb_sts = sbFe2000PmOamTimerRead(pPm, (uint32_t)idx, &zTimer);
            if(sb_sts != SB_OK) {
                printk("sbFe2000PmOamTimerRead: idx %d failed. \n", idx);
                if (parsed) {
                    parse_arg_eq_done(&pt);
                }
                return CMD_FAIL;
            }
            SOCK_LOG_ZF(sbZfFe2000PmOamTimerConfig, &zTimer);
        }
    } else {
        printk("Invalid OAM sub-command (%s) \n", subcmd);
        return CMD_USAGE;
    }

    if (parsed) {
        parse_arg_eq_done(&pt);
    }
    return CMD_OK;
}

cmd_result_t
_cmd_pol_oam_seq_gen(int unit, args_t *args)
{
    char            *subcmd, *rate, *temp;
    int             id = -1, min, max, idx, sb_sts, sc_len, sb_rate;
    uint32_t        id_ret, parsed = 1;
    parse_table_t   pt;
    sbFe2000Pm_t    *pPm;

    if (_cmd_sbx_pm_get(unit, &pPm) != BCM_E_NONE) {
        printk("Internal error. \n");
        return CMD_FAIL;
    }

    subcmd = ARG_GET(args);
    if (!subcmd || !ARG_CUR(args)) {
        return CMD_USAGE;
    }
    sc_len = strlen(subcmd);
    if (!sc_len) {
        return CMD_USAGE;
    }
    
    if (!sal_strncasecmp(subcmd, "add", (sc_len > 3? 3: sc_len))) {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "id",          PQ_DFL|PQ_INT, 0, &id,       NULL);
        parse_table_add(&pt, "rate",        PQ_STRING,     0, &rate,     NULL);
        if (!parse_arg_eq(args, &pt)) {
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        if (_cmd_pol_rate_valid(rate, &sb_rate) != BCM_E_NONE) {
            if (rate && (strlen(rate) != 0)) {
                printk("Invalid rate (%s) specified. \n", rate);
            } else {
                printk("Rate not specified. \n");
            }
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        if (id < 0) {
            printk("Id not specified. \n");
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        sb_sts = sbFe2000PmOamSeqGenCreate(pPm, (uint32_t)id, &id_ret);
        if (sb_sts != SB_OK) {
            printk("sbFe2000PmOamSeqGenCreate failed. \n");
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        printk("SeqGen Id - %d created successfully\n", id);
    } else if (!sal_strncasecmp(subcmd, "remove", (sc_len > 6 ? 6: sc_len))) {
        temp = ARG_CUR(args);
        sc_len = strlen(temp);
        if (!sal_strncasecmp(temp, "all", (sc_len > 3? 3: sc_len))) {
            printk("TODO: remove all not supported yet. \n");
            parsed = 0;
            return CMD_FAIL;
        } else {
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "id",          PQ_DFL|PQ_INT, 0, &id,       NULL);
            if (!parse_arg_eq(args, &pt)) {
                parse_arg_eq_done(&pt);
                return CMD_FAIL;
            }
            if (id < 0) {
                printk("SeqGen id not specified. \n");
                parse_arg_eq_done(&pt);
                return CMD_USAGE;
            }
            min = id;
            max = id + 1;
        }
        for(idx = min; idx < max; idx++) {
            sb_sts = sbFe2000PmOamSeqGenDelete(pPm, (uint32_t)idx);
            if(sb_sts != SB_OK) {
                printk("sbFe2000PmOamSeqGenDelete failed. \n");
                if (parsed) {
                    parse_arg_eq_done(&pt);
                }
                return CMD_FAIL;
            }
            printk("SeqGen %d Deleted\n", idx);
        }
    } else {
        printk("Invalid SequenceGenerator sub-command (%s) \n", subcmd);
        return CMD_FAIL;
    }

    if (parsed) {
        parse_arg_eq_done(&pt);
    }
    return CMD_OK;
}

cmd_result_t
_cmd_pol_oam(int unit, args_t *args) 
{
    char    *subcmd;
    int     sc_len;

    subcmd = ARG_GET(args);
    if (!subcmd || !ARG_CUR(args)) {
        return CMD_USAGE;
    }
    sc_len = strlen(subcmd);

    if (!sal_strncasecmp(subcmd, "timer", (sc_len > 5? 5: sc_len))) {
        return _cmd_pol_oam_timer(unit, args);
    } else if (!sal_strncasecmp(subcmd, "sequencegenerator", (sc_len > 17? 17: sc_len))) {
        return _cmd_pol_oam_seq_gen(unit, args);
    } else {
        printk("Invalid OAM Sub-Command. \n");
        return CMD_USAGE;
    }

    return CMD_OK;
}

cmd_result_t
_cmd_pol_monitor(int unit, args_t *args)
{
    char            *subcmd;
    int             rv, sc_len, stat_type, monid, stat_min, stat_max;
    parse_table_t   pt;
    cmd_result_t    retCode;
    bcm_policer_t   polid = -1;    
    uint64_t        stat_value;
    uint64_t        zero_value;

    COMPILER_64_ZERO(zero_value);

    subcmd = ARG_GET(args);
    if (!subcmd || !ARG_CUR(args)) {
        return CMD_USAGE;
    }
    sc_len = strlen(subcmd);
    COMPILER_64_SET(stat_value, 0, 0);

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "id", PQ_DFL|PQ_INT, 0, &polid, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }
    if (polid < 0) {
        return CMD_USAGE;
    }

    if (!sal_strncasecmp(subcmd, "attach", (sc_len>6 ? 6:sc_len))) {
        rv = _bcm_fe2000_policer_unit_lock(unit);
        if (rv == BCM_E_NONE) {
          rv = _bcm_fe2000_attach_monitor(unit, polid);
          _bcm_fe2000_policer_unit_unlock(unit);
        }
        if (rv != BCM_E_NONE) {
          printk("Internal error attaching monitor to policer: %d\n", polid);
          return CMD_FAIL;
        }
    } else if (!sal_strncasecmp(subcmd, "detach", (sc_len>6 ? 6:sc_len))) {
        rv = _bcm_fe2000_policer_unit_lock(unit);
        if (rv == BCM_E_NONE) {
          rv = _bcm_fe2000_detach_monitor(unit, polid);
          _bcm_fe2000_policer_unit_unlock(unit);
        }
        if (rv != BCM_E_NONE) {
          printk("Internal error detaching monitor to policer: %d\n", polid);
          return CMD_FAIL;
        }
    } else if (!sal_strncasecmp(subcmd, "display", (sc_len>7 ? 7:sc_len))) {
        rv = _bcm_fe2000_policer_unit_lock(unit);
        if (rv == BCM_E_NONE) {
            if (_bcm_fe2000_is_monitor_stat_enabled(unit, polid, &monid)){
                stat_min = ((SOC_INFO(unit).chip_type == SOC_INFO_FE2000) ?
                            bcmPolicerStatGreenToGreenBytes :
                            bcmPolicerStatGreenToGreenPackets);
                stat_max = bcmPolicerStatRedToDropBytes;
                for (stat_type = stat_min; stat_type <= stat_max; stat_type++){
                    rv = _bcm_fe2000_monitor_stat_get(unit, monid, 
                                                       stat_type, &stat_value);
                    _bcm_fe2000_monitor_stat_set(unit, monid, 
                                                 stat_type, zero_value);
                    if (rv == BCM_E_NONE) {
                        rv = _bcm_sbx_policer_stat_display(unit, 
                                                           stat_type, stat_value);
                    }
                    if (rv != BCM_E_NONE) {
                        break;
                    }
                }
            } else {
                printk("Monitor stats not enabled on policer %d \n",
                 polid);
            }
            _bcm_fe2000_policer_unit_unlock(unit);
        }
        if (rv != BCM_E_NONE) {
            printk("Internal error displaying monitor stats for "
                   "policer: %d \n", polid);
            return CMD_FAIL;
        }
    } else {
        printk("Invalid Monitor sub-command (%s) \n", subcmd);
        return CMD_USAGE;
    }

    return CMD_OK;
}


static cmd_t _cmd_sbx_policer_list[] = {
    {"init",    _cmd_pol_init,          "\n" CMD_POL_INIT_USAGE,        NULL},
    {"list",    _cmd_pol_list,          "\n" CMD_POL_LIST_USAGE,        NULL},
    {"group",   _cmd_pol_group,         "\n" CMD_POL_GROUP_USAGE,       NULL},
    {"add",     _cmd_pol_add,           "\n" CMD_POL_ADD_USAGE,         NULL},
    {"remove",  _cmd_pol_rem,           "\n" CMD_POL_REM_USAGE,         NULL},
    {"stat",    _cmd_pol_stat,          "\n" CMD_POL_STAT_USAGE,        NULL},
    {"oam",     _cmd_pol_oam,           "\n" CMD_POL_OAM_USAGE,         NULL},
    {"monitor", _cmd_pol_monitor,       "\n" CMD_POL_MON_USAGE,         NULL}
};

cmd_result_t
cmd_sbx_policer(int unit, args_t *args) 
{
    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("Policer commands only supported on FE-2000 \n");
        return CMD_FAIL;
    }

    return subcommand_execute(unit, args, _cmd_sbx_policer_list, 
                              COUNTOF(_cmd_sbx_policer_list));
}

char cmd_sbx_ddr_train_usage[] =
"DDRtrain\n"
;
cmd_result_t
cmd_sbx_ddr_train(int unit, args_t *args)
{

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("only supported on FE-2000\n");
        return CMD_FAIL;
    }

    soc_sbx_fe2000_ddr_train(unit);

    return CMD_OK;
}

char cmd_sbx_ddr_tune_usage[] =
"External memory automatic tuning\n"
"exttsbx [derate_margin] - (optional) derate setup margin.  Default is 4.\n"
"Use debugmod soc +verbose for more debug msgs\n"
;
cmd_result_t
cmd_sbx_ddr_tune(int unit, args_t *args)
{
   char *deratemargin_s;
   int deratemargin;

  /* parse_table_t pt; */

  if (!SOC_IS_SBX_FE2KXT(unit)){
    printk("only supported on BCM88025\n");
    return CMD_FAIL;
  }

  if ((deratemargin_s = ARG_GET(args)) == NULL) {
    deratemargin = 4;
  }else{
    deratemargin = sal_ctoi(deratemargin_s,0);
  }


#if 0
  if (!SOC_SBX_INIT(unit)) {
    printk("call init soc first\n");
    return CMD_FAIL;
  }

  parse_table_init(unit, &pt);
  /* consume valid arguments */
  if (parse_arg_eq(args, &pt) < 0) {
    printk("%s: Invalid option: %s\n",
	   ARG_CMD(args), ARG_CUR(args));
    parse_arg_eq_done(&pt);
    return CMD_USAGE;
  }

  if (ARG_CNT(args) != 0) {
    printk("%s: extra options starting with \"%s\" \n",
	   ARG_CMD(args), ARG_CUR(args));
    parse_arg_eq_done(&pt);
    return CMD_USAGE;
  }
#endif

  soc_sbx_fe2000_ddr_tune(unit, deratemargin);

  return CMD_OK;

}


#ifdef BROADCOM_SVK
char cmd_sbx_fe2000_sync_ethernet_usage[] =
"\nSyncE <geport>  - use <geport> to recover clock from \n"
"SyncE   off     - turn sync mode off, turns on free-run mode\n"
;

cmd_result_t
cmd_sbx_fe2000_sync_ethernet(int unit, args_t *a)
{
  sbhandle sbh;
  char *cmd, *subcmd;
  int ge_port=0;

  if( !SOC_SBX_INIT(unit) ){
    printk("Unit %d, not initialized - call 'init soc' first!\n", unit);
    return CMD_FAIL;
  }

  if (!SOC_IS_SBX_FE2000(unit)) {
    printk("only supported on FE-2000\n");
    return CMD_FAIL;
  }

  sbh = SOC_SBX_CONTROL(unit)->sbhdl;

  cmd = ARG_CMD(a);
  if (!sh_check_attached(cmd, unit)) {
    return CMD_FAIL;
  }

  if ((subcmd = ARG_GET(a)) == NULL) {
    return CMD_USAGE;
  }

  if (sal_strcasecmp(subcmd,"off") == 0) {
    printk("Turning off sync mode\n");
    SAND_HAL_FE2000_WRITE(sbh,AG0_RXCLK_MUX,0xf);
    SAND_HAL_FE2000_WRITE(sbh,AG1_RXCLK_MUX,0xf);
    SAND_HAL_FE2000_WRITE(sbh,PC_GPIO_DIR,0xfffffc);
    SAND_HAL_FE2000_WRITE(sbh,PC_GPIO_OUT,0x0);
    return CMD_OK;
  }
  else {
    ge_port = sal_ctoi(subcmd,0);
    if (ge_port < 0 || ge_port > 23 ) {
      printk("%s:Invalid ge port (%d)\n",SOC_CHIP_STRING(unit),ge_port);
      return CMD_FAIL;
    }
    printk("Using %s for recovered clock\n",SOC_PORT_NAME(unit,ge_port));
    if (ge_port <= 11) {
      SAND_HAL_FE2000_WRITE(sbh,AG0_RXCLK_MUX,ge_port);
    } else {
      ge_port -= 12;
      SAND_HAL_FE2000_WRITE(sbh,AG1_RXCLK_MUX,ge_port);
    }
    /* put cleanup PLL in jitter cleanup mode */
    SAND_HAL_FE2000_WRITE(sbh,PC_GPIO_OUT,0x3);
  }

  return CMD_OK;
}

static
void _cmd_sbx_fe2kxt_ddrconfig_dump(int unit, sbFe2000XtInitParamsMmDdrConfig_t* ddr)
{
  int i;

  for (i=0; i<SB_FE2000_NUM_MM_INSTANCES; i++) {
      printk("bcm88025_mmu%i_narrow0_config.%d=%d,%d,%d,%d,%d\n",i,unit,ddr[i].uValidDelay[0],ddr[i].uPhaseSelect[0],ddr[i].uQkMidRange[0],ddr[i].uQknMidRange[0],ddr[i].uXMidRange[0]);
      printk("bcm88025_mmu%i_narrow1_config.%d=%d,%d,%d,%d,%d\n",i,unit,ddr[i].uValidDelay[1],ddr[i].uPhaseSelect[1],ddr[i].uQkMidRange[1],ddr[i].uQknMidRange[1],ddr[i].uXMidRange[1]);
      printk("bcm88025_mmu%i_wide_config.%d=%d,%d,%d,%d,%d\n",i,unit,ddr[i].uValidDelay[2],ddr[i].uPhaseSelect[2],ddr[i].uQkMidRange[2],ddr[i].uQknMidRange[2],ddr[i].uXMidRange[2]);
  }

}

char cmd_sbx_fe2kxt_ddrconfig_usage[] =
"\nddrconfig load      - load ddr tuning parameters from EEPROM\n"
"ddrconfig show      - show current ddr tuning parameters (not from EEPROM)\n"
"ddrconfig store       - store ddr tuning parameters from EEPROM\n"
"ddrconfig help        - displays this messge\n";

cmd_result_t
cmd_sbx_fe2kxt_ddrconfig(int unit, args_t *a)
{
    soc_sbx_control_t *sbx;
    sbhandle sbh;
    char *cmd, *subcmd;
    sbFe2000XtInitParams_t *pIP;
    int ddr_rv;

    cmd_result_t rv = CMD_OK;

    cmd = ARG_CMD(a);
    if (!sh_check_attached(cmd, unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    sbx = SOC_SBX_CONTROL(unit);
    sbh = sbx->sbhdl;

    if (!SOC_IS_SBX_FE2KXT(unit)) {
        printk("ddrconfig command valid only on FE2KXT\n");
        rv = CMD_FAIL;
    }
    if (sal_strcasecmp(subcmd, "load") == 0) {
        printk ("loading DDR tuning config from EEPROM\n");
  
        pIP = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;
  
        if (pIP->ddrConfigRw) {
            ddr_rv = pIP->ddrConfigRw(unit, 0, pIP->ddrconfig);
            if (ddr_rv){
                rv = CMD_FAIL;
            }
        }else{
            rv = CMD_FAIL;
        }
  
        _cmd_sbx_fe2kxt_ddrconfig_dump(unit, pIP->ddrconfig);
  
        rv = CMD_OK;
    } else if (sal_strcasecmp(subcmd, "store") == 0) {
        printk ("storing DDR tuning config to EEPROM\n");

        pIP = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;

        _cmd_sbx_fe2kxt_ddrconfig_dump(unit, pIP->ddrconfig);

        if (pIP->ddrConfigRw) {
            ddr_rv = pIP->ddrConfigRw(unit, 1, pIP->ddrconfig);
            if (ddr_rv){
                rv = CMD_FAIL;
            }
        }else{
            rv = CMD_FAIL;
        }

        return CMD_OK;
    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        printk ("current DDR tuning configuration \n");
  
        pIP = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;
  
        _cmd_sbx_fe2kxt_ddrconfig_dump(unit, pIP->ddrconfig);
  
        rv = CMD_OK;
    } else if (sal_strcasecmp(subcmd, "help") == 0) {
        rv = CMD_USAGE;
    }

    return rv;
}

#endif /* BROADCOM_SVK */

#endif /* BCM_FE2000_SUPPORT */
